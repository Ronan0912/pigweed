// Copyright 2023 The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.

#include "pw_bluetooth_sapphire/internal/host/hci/advertising_handle_map.h"

#include <pw_assert/check.h>

namespace bt::hci {

std::optional<hci_spec::AdvertisingHandle> AdvertisingHandleMap::MapHandle(
    const DeviceAddress& address) {
  if (Size() == capacity_) {
    return std::nullopt;
  }

  auto next_handle = NextHandle();
  PW_CHECK(next_handle);

  Value value;
  value.address = address;
  value.node = node_.CreateChild(node_.UniqueName("advertising_set_"));
  value.node.RecordString("address", address.ToString());
  value.node.RecordUint("handle", next_handle.value());

  auto [_, success] = map_.try_emplace(next_handle.value(), std::move(value));
  PW_CHECK(success);
  return next_handle;
}
std::optional<DeviceAddress> AdvertisingHandleMap::GetAddress(
    hci_spec::AdvertisingHandle handle) const {
  auto iter = map_.find(handle);
  if (iter == map_.end()) {
    return std::nullopt;
  }
  return iter->second.address;
}

std::optional<hci_spec::AdvertisingHandle>
AdvertisingHandleMap::LastUsedHandleForTesting() const {
  if (last_handle_ > hci_spec::kMaxAdvertisingHandle) {
    return std::nullopt;
  }

  return last_handle_;
}

std::optional<hci_spec::AdvertisingHandle> AdvertisingHandleMap::NextHandle() {
  if (Size() >= capacity_) {
    return std::nullopt;
  }

  hci_spec::AdvertisingHandle handle = last_handle_;
  do {
    handle = static_cast<uint8_t>(handle + 1) % capacity_;
  } while (map_.count(handle) != 0);

  last_handle_ = handle;
  return handle;
}

void AdvertisingHandleMap::AttachInspect(inspect::Node& parent) {
  node_ = parent.CreateChild("advertising_handle_map");
}

}  // namespace bt::hci
