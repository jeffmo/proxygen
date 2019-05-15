/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include <proxygen/lib/http/session/test/HQSessionTestCommon.h>
#include <quic/codec/QuicInteger.h>

#include <folly/Random.h>
#include <folly/String.h>

using namespace proxygen;
using namespace proxygen::hq;

size_t encodeQuicIntegerWithAtLeast(uint64_t value,
                                    uint8_t atLeast,
                                    folly::io::QueueAppender& appender) {
  CHECK(atLeast == 1 || atLeast == 2 || atLeast == 4 || atLeast == 8);

  CHECK_LE(value, quic::kEightByteLimit);
  uint8_t numBytes = 0;
  if (value <= quic::kOneByteLimit) {
    numBytes = 1;
  } else if (value <= quic::kTwoByteLimit) {
    numBytes = 2;
  } else if (value <= quic::kFourByteLimit) {
    numBytes = 4;
  } else if (value <= quic::kEightByteLimit) {
    numBytes = 8;
  }
  CHECK_NE(numBytes, 0);
  numBytes = std::max(numBytes, atLeast);
  CHECK(numBytes == 1 || numBytes == 2 || numBytes == 4 || numBytes == 8);
  if (numBytes == 1) {
    uint8_t modified = static_cast<uint8_t>(value);
    appender.writeBE(modified);
    return sizeof(modified);
  } else if (numBytes == 2) {
    uint16_t reduced = static_cast<uint16_t>(value);
    uint16_t modified = reduced | 0x4000;
    appender.writeBE(modified);
    return sizeof(modified);
  } else if (numBytes == 4) {
    uint32_t reduced = static_cast<uint32_t>(value);
    uint32_t modified = reduced | 0x80000000;
    appender.writeBE(modified);
    return sizeof(modified);
  } else if (numBytes == 8) {
    uint64_t modified = value | 0xC000000000000000;
    appender.writeBE(modified);
    return sizeof(modified);
  }
  CHECK(false);
}

size_t generateStreamPreface(folly::IOBufQueue& writeBuf,
                             UnidirectionalStreamType type) {
  folly::io::QueueAppender appender(&writeBuf, 8);
  uint8_t size = 1 << (folly::Random::rand32() % 4);
  auto bytesWritten = encodeQuicIntegerWithAtLeast(
      static_cast<hq::StreamTypeType>(type), size, appender);
  CHECK_GE(bytesWritten, size);
  return bytesWritten;
}

std::string paramsToTestName(const testing::TestParamInfo<TestParams>& info) {
  std::vector<std::string> paramsV;
  folly::split("-", info.param.alpn_, paramsV);
  return folly::join("", paramsV);
}

folly::Optional<std::pair<UnidirectionalStreamType, size_t>> parseStreamPreface(
    folly::io::Cursor cursor, std::string alpn) {
  CHECK(!ALPN_H1Q_FB_V1);
  auto res = quic::decodeQuicInteger(cursor);
  if (!res) {
    return folly::none;
  }
  auto prefaceEnum = UnidirectionalStreamType(res->first);
  switch (prefaceEnum) {
    case UnidirectionalStreamType::H1Q_CONTROL:
      if (ALPN_H1Q_FB_V2) {
        return std::make_pair(prefaceEnum, res->second);
      } else {
        return folly::none;
      }
      break;
    case UnidirectionalStreamType::CONTROL:
    case UnidirectionalStreamType::QPACK_ENCODER:
    case UnidirectionalStreamType::QPACK_DECODER:
      if (ALPN_HQ) {
        return std::make_pair(prefaceEnum, res->second);
      } else {
        return folly::none;
      }
      break;
    default:
      break;
  }
  return folly::none;
}

void parseReadData(HQUnidirectionalCodec* codec,
                   folly::IOBufQueue& readBuf,
                   std::unique_ptr<folly::IOBuf> buf) {
  readBuf.append(std::move(buf));
  auto ret = codec->onUnidirectionalIngress(readBuf.move());
  readBuf.append(std::move(ret));
}

void createControlStream(quic::MockQuicSocketDriver* socketDriver,
                         quic::StreamId id,
                         UnidirectionalStreamType streamType) {
  folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
  auto length = generateStreamPreface(writeBuf, streamType);
  CHECK_EQ(length, writeBuf.chainLength());
  socketDriver->sock_->setControlStream(id);
  for (size_t i = 0; i < length; i++) {
    socketDriver->addReadEvent(
        id, writeBuf.splitAtMost(1), std::chrono::milliseconds(0));
  }
}
