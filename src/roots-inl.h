// Copyright 2018 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_ROOTS_INL_H_
#define V8_ROOTS_INL_H_

#include "src/roots.h"

#include "src/feedback-vector.h"
#include "src/handles.h"
#include "src/heap/heap-inl.h"
#include "src/objects/api-callbacks.h"
#include "src/objects/descriptor-array.h"
#include "src/objects/literal-objects.h"
#include "src/objects/map.h"
#include "src/objects/scope-info.h"
#include "src/objects/slots.h"

namespace v8 {
namespace internal {

V8_INLINE constexpr bool operator<(RootIndex lhs, RootIndex rhs) {
  typedef typename std::underlying_type<RootIndex>::type type;
  return static_cast<type>(lhs) < static_cast<type>(rhs);
}

V8_INLINE RootIndex operator++(RootIndex& index) {
  typedef typename std::underlying_type<RootIndex>::type type;
  index = static_cast<RootIndex>(static_cast<type>(index) + 1);
  return index;
}

bool RootsTable::IsRootHandleLocation(Address* handle_location,
                                      RootIndex* index) const {
  ObjectSlot location(handle_location);
  ObjectSlot first_root(&roots_[0]);
  ObjectSlot last_root(&roots_[kEntriesCount]);
  if (location >= last_root) return false;
  if (location < first_root) return false;
  *index = static_cast<RootIndex>(location - first_root);
  return true;
}

template <typename T>
bool RootsTable::IsRootHandle(Handle<T> handle, RootIndex* index) const {
  // This can't use handle.location() because it is called from places
  // where handle dereferencing is disallowed. Comparing the handle's
  // location against the root handle list is safe though.
  Address* handle_location = reinterpret_cast<Address*>(handle.address());
  return IsRootHandleLocation(handle_location, index);
}

ReadOnlyRoots::ReadOnlyRoots(Heap* heap)
    : roots_table_(heap->isolate()->roots_table()) {}

ReadOnlyRoots::ReadOnlyRoots(Isolate* isolate)
    : roots_table_(isolate->roots_table()) {}

// TODO(jkummerow): Drop std::remove_pointer after the migration to ObjectPtr.
#define ROOT_ACCESSOR(Type, name, CamelName)                             \
  Type ReadOnlyRoots::name() const {                                     \
    return std::remove_pointer<Type>::type::cast(                        \
        roots_table_[RootIndex::k##CamelName]);                          \
  }                                                                      \
  Handle<std::remove_pointer<Type>::type> ReadOnlyRoots::name##_handle() \
      const {                                                            \
    return Handle<std::remove_pointer<Type>::type>(                      \
        bit_cast<Address*>(&roots_table_[RootIndex::k##CamelName]));     \
  }

READ_ONLY_ROOT_LIST(ROOT_ACCESSOR)
#undef ROOT_ACCESSOR

Map ReadOnlyRoots::MapForFixedTypedArray(ExternalArrayType array_type) {
  RootIndex root_index = RootsTable::RootIndexForFixedTypedArray(array_type);
  return Map::cast(roots_table_[root_index]);
}

Map ReadOnlyRoots::MapForFixedTypedArray(ElementsKind elements_kind) {
  RootIndex root_index = RootsTable::RootIndexForFixedTypedArray(elements_kind);
  return Map::cast(roots_table_[root_index]);
}

FixedTypedArrayBase ReadOnlyRoots::EmptyFixedTypedArrayForMap(const Map map) {
  RootIndex root_index =
      RootsTable::RootIndexForEmptyFixedTypedArray(map->elements_kind());
  return FixedTypedArrayBase::cast(roots_table_[root_index]);
}

}  // namespace internal
}  // namespace v8

#endif  // V8_ROOTS_INL_H_
