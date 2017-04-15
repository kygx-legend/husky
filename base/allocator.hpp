// Copyright 2017 Husky Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <memory>

#include "boost/align/aligned_allocator.hpp"

namespace husky {

template <typename T>
using StdAllocator = std::allocator<T>;

template <typename T, size_t aligned_size = 32>
using AlignedAllocator = boost::alignment::aligned_allocator<T, aligned_size>;

}  // namespace husky
