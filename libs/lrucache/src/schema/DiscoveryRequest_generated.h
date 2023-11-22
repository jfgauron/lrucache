// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_DISCOVERYREQUEST_LRUCACHE_SCHEMA_H_
#define FLATBUFFERS_GENERATED_DISCOVERYREQUEST_LRUCACHE_SCHEMA_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 5 &&
              FLATBUFFERS_VERSION_REVISION == 26,
             "Non-compatible flatbuffers version included");

namespace lrucache {
namespace schema {

struct DiscoveryRequest;
struct DiscoveryRequestBuilder;

struct DiscoveryRequest FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef DiscoveryRequestBuilder Builder;
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           verifier.EndTable();
  }
};

struct DiscoveryRequestBuilder {
  typedef DiscoveryRequest Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  explicit DiscoveryRequestBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<DiscoveryRequest> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<DiscoveryRequest>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<DiscoveryRequest> CreateDiscoveryRequest(
    ::flatbuffers::FlatBufferBuilder &_fbb) {
  DiscoveryRequestBuilder builder_(_fbb);
  return builder_.Finish();
}

inline const lrucache::schema::DiscoveryRequest *GetDiscoveryRequest(const void *buf) {
  return ::flatbuffers::GetRoot<lrucache::schema::DiscoveryRequest>(buf);
}

inline const lrucache::schema::DiscoveryRequest *GetSizePrefixedDiscoveryRequest(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<lrucache::schema::DiscoveryRequest>(buf);
}

inline bool VerifyDiscoveryRequestBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<lrucache::schema::DiscoveryRequest>(nullptr);
}

inline bool VerifySizePrefixedDiscoveryRequestBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<lrucache::schema::DiscoveryRequest>(nullptr);
}

inline void FinishDiscoveryRequestBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<lrucache::schema::DiscoveryRequest> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedDiscoveryRequestBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<lrucache::schema::DiscoveryRequest> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace schema
}  // namespace lrucache

#endif  // FLATBUFFERS_GENERATED_DISCOVERYREQUEST_LRUCACHE_SCHEMA_H_
