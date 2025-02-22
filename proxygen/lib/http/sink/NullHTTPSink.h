/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/sink/HTTPSink.h"
#include <folly/logging/xlog.h>

namespace proxygen {

/**
 * A NullHTTPSink provides a dummy HTTPSink to Revproxy Handler with no
 * associated client transaction or Async Request generated by RevProxy.
 * It does not expect any events to be invoked by HTTPRevproxyHandler.
 *
 * Its main use case is to support Async RevproxyHandler associated with
 * no active transaction.
 *
 * Life Cycle
 * ==========
 * A NullHTTPSink is owned by a unqiue_ptr in the caller creating Async
 * HTTPRevProxyHandler and set through setHTTPSink API.
 *
 * It is destructed when the AsycRevProxyHandler is destroyed.
 */
class NullHTTPSink : public HTTPSink {
 public:
  NullHTTPSink() = default;
  ~NullHTTPSink() override = default;
  [[nodiscard]] HTTPTransaction* FOLLY_NULLABLE getHTTPTxn() const override {
    return nullptr;
  }
  void detachHandler() override {
  }
  // Sending data
  void sendHeaders(const HTTPMessage& /*headers*/) override {
    XLOG(ERR) << "sendHeaders event is not expected for NullHTTPSink";
  }
  bool sendHeadersWithDelegate(
      const HTTPMessage& /*headers*/,
      std::unique_ptr<DSRRequestSender> /*sender*/) override {
    XLOG(FATAL)
        << "sendHeadersWithDelegate event is not expected for NullHTTPSink";
  }
  void sendHeadersWithEOM(const HTTPMessage& /*headers*/) override {
    XLOG(ERR) << "sendHeadersWithEOM event is not expected for NullHTTPSink";
  }
  void sendHeadersWithOptionalEOM(const HTTPMessage& /*headers*/,
                                  bool /*eom*/) override {
    XLOG(ERR) << "sendHeadersWithOptionalEOM event is not expected for "
                 "NullHTTPSink";
  }
  void sendBody(std::unique_ptr<folly::IOBuf> /*body*/) override {
    XLOG(ERR) << "sendBody event is not expected for NullHTTPSink";
  }
  void sendChunkHeader(size_t /*length*/) override {
    XLOG(ERR) << "sendChunkHeader event is not expected for NullHTTPSink";
  }
  void sendChunkTerminator() override {
    XLOG(ERR) << "sendChunkTerminator event is not expected for NullHTTPSink";
  }
  void sendTrailers(const HTTPHeaders& /*trailers*/) override {
    XLOG(ERR) << "sendTrailers event is not expected for NullHTTPSink";
  }
  void sendEOM() override {
    XLOG(ERR) << "sendEOM event is not expected for NullHTTPSink";
  }
  void sendAbort() override {
    XLOG(ERR) << "sendAbort event is not expected for NullHTTPSink";
  }
  [[nodiscard]] bool canSendHeaders() const override {
    XLOG(ERR) << "canSendHeaders event is not expected for NullHTTPSink";
    return false;
  }
  [[nodiscard]] bool extraResponseExpected() const override {
    XLOG(ERR) << "extraResponseExpected event is not expected for NullHTTPSink";
    return false;
  }
  // Flow control (no-op)
  void pauseIngress() override {
    XLOG(ERR) << "pauseIngress event is not expected for NullHTTPSink";
  }
  void pauseEgress() override {
    XLOG(ERR) << "pauseEgress event is not expected for NullHTTPSink";
  }
  void resumeIngress() override {
    XLOG(ERR) << "resumeIngress event is not expected for NullHTTPSink";
  }
  void resumeEgress() override {
    XLOG(ERR) << "resumeEgress event is not expected for NullHTTPSink";
  }
  [[nodiscard]] bool isIngressPaused() const override {
    XLOG(ERR) << "isIngressPaused not expected for NullHTTPSink";
    return false;
  }
  [[nodiscard]] bool isEgressPaused() const override {
    XLOG(ERR) << "isEgressPaused not expected for NullHTTPSink";
    return false;
  }
  void setEgressRateLimit(uint64_t /*bitsPerSecond*/) override {
    XLOG(ERR) << "setEgressRateLimit is not expected for NullHTTPSink";
  }

  // Client timeout
  void timeoutExpired() override {
    XLOG(ERR) << "timeoutExpired is not expected for NullHTTPSink";
  }

  // Capabilities
  bool safeToUpgrade(HTTPMessage* /*req*/) const override {
    return true;
  }
  [[nodiscard]] bool supportsPush() const override {
    return false;
  }

 private:
};

} // namespace proxygen
