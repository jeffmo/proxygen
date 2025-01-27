/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>

#include "proxygen/lib/http/sink/HTTPSink.h"

namespace proxygen {

/**
 * A HTTPTransactionSink forwards events to the client txn.
 */
class HTTPTransactionSink : public HTTPSink {
 public:
  explicit HTTPTransactionSink(HTTPTransaction* clientTxn)
      : httpTransaction_{clientTxn} {
    XCHECK(clientTxn)
        << "HTTPTransactionSink must be created with a valid clientTxn.";
  }
  ~HTTPTransactionSink() override = default;
  [[nodiscard]] HTTPTransaction* FOLLY_NULLABLE getHTTPTxn() const override {
    return httpTransaction_;
  }
  void detachHandler() override {
    httpTransaction_->setHandler(nullptr);
  }

  // Sending data
  void sendHeaders(const HTTPMessage& headers) override {
    httpTransaction_->sendHeaders(headers);
  }
  bool sendHeadersWithDelegate(
      const HTTPMessage& headers,
      std::unique_ptr<DSRRequestSender> sender) override {
    return httpTransaction_->sendHeadersWithDelegate(headers,
                                                     std::move(sender));
  }
  void sendHeadersWithEOM(const HTTPMessage& headers) override {
    httpTransaction_->sendHeadersWithEOM(headers);
  }
  void sendHeadersWithOptionalEOM(const HTTPMessage& headers,
                                  bool eom) override {
    httpTransaction_->sendHeadersWithOptionalEOM(headers, eom);
  }
  void sendBody(std::unique_ptr<folly::IOBuf> body) override {
    httpTransaction_->sendBody(std::move(body));
  }
  void sendChunkHeader(size_t length) override {
    httpTransaction_->sendChunkHeader(length);
  }
  void sendChunkTerminator() override {
    httpTransaction_->sendChunkTerminator();
  }
  void sendTrailers(const HTTPHeaders& trailers) override {
    httpTransaction_->sendTrailers(trailers);
  }
  void sendEOM() override {
    httpTransaction_->sendEOM();
  }
  void sendAbort() override {
    httpTransaction_->sendAbort();
  }
  [[nodiscard]] bool canSendHeaders() const override {
    return httpTransaction_->canSendHeaders();
  }
  [[nodiscard]] bool extraResponseExpected() const override {
    return httpTransaction_->extraResponseExpected();
  }
  // Flow control
  void pauseIngress() override {
    httpTransaction_->pauseIngress();
  }
  void pauseEgress() override {
    httpTransaction_->pauseEgress();
  }
  void resumeIngress() override {
    httpTransaction_->resumeIngress();
  }
  void resumeEgress() override {
    httpTransaction_->resumeEgress();
  }
  [[nodiscard]] bool isIngressPaused() const override {
    return httpTransaction_->isIngressPaused();
  }
  [[nodiscard]] bool isEgressPaused() const override {
    return httpTransaction_->isEgressPaused();
  }
  void setEgressRateLimit(uint64_t bitsPerSecond) override {
    httpTransaction_->setEgressRateLimit(bitsPerSecond);
  }
  // Client timeout
  void timeoutExpired() override {
    httpTransaction_->timeoutExpired();
  }
  // Capabilities
  bool safeToUpgrade(HTTPMessage* req) const override;
  [[nodiscard]] bool supportsPush() const override {
    return true;
  }

 private:
  HTTPTransaction* httpTransaction_;
};

} // namespace proxygen
