// Copyright 2016 Husky Team
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

#include <list>
#include <unordered_map>
#include <utility>

#include "boost/optional.hpp"

namespace husky {

template <typename KeyT, typename ValueT>
class CacheBase {
   public:
    explicit CacheBase(size_t size) { size_ = size; }

    size_t get_max_size() { return size_; }

    virtual void put(KeyT&& key, ValueT&& val) = 0;

    virtual void put(const KeyT& key, const ValueT& val) = 0;

    virtual boost::optional<std::pair<KeyT, ValueT>> del() = 0;

    virtual boost::optional<std::pair<KeyT, ValueT>> poll() = 0;

    virtual size_t get_size() = 0;

    virtual bool exists(KeyT&& key) = 0;

    virtual bool exists(const KeyT& key) = 0;

   protected:
    size_t size_;
};

template <typename KeyT, typename ValueT>
class LRUCache : public CacheBase<KeyT, ValueT> {
   public:
    using ListIterator = typename std::list<std::pair<KeyT, ValueT>>::iterator;

    explicit LRUCache(size_t size) : CacheBase<KeyT, ValueT>(size) {}

    ~LRUCache() = default;

    void put(KeyT&& key, ValueT&& val) override {
        auto it_cache = cache_map_.find(key);
        cache_list_.push_front(std::make_pair(key, std::move(val)));
        if (it_cache != cache_map_.end()) {
            cache_list_.erase(it_cache->second);
            cache_map_.erase(it_cache);
        }
        cache_map_[std::move(key)] = cache_list_.begin();

        if (cache_map_.size() > this->size_) {
            del();
        }
    }

    void put(const KeyT& key, const ValueT& val) override {
        auto it_cache = cache_map_.find(key);
        cache_list_.push_front(std::make_pair(key, val));
        if (it_cache != cache_map_.end()) {
            cache_list_.erase(it_cache->second);
            cache_map_.erase(it_cache);
        }
        cache_map_[key] = cache_list_.begin();

        if (cache_map_.size() > this->size_) {
            del();
        }
    }

    boost::optional<std::pair<KeyT, ValueT>> del() override {
        if (cache_map_.size() == 0) {
            return boost::none;
        }
        auto last = cache_list_.end();
        last--;
        KeyT key = last->first;
        ValueT objlist = last->second;
        cache_map_.erase(key);
        cache_list_.pop_back();
        return *last;
    }

    boost::optional<std::pair<KeyT, ValueT>> poll() override {
        if (cache_map_.size() == 0) {
            return boost::none;
        }
        return cache_list_.back();
    }

    size_t get_size() override { return cache_map_.size(); }

    bool exists(KeyT&& key) override { return cache_map_.find(std::move(key)) != cache_map_.end(); }

    bool exists(const KeyT& key) override { return cache_map_.find(key) != cache_map_.end(); }

   private:
    std::unordered_map<KeyT, ListIterator> cache_map_;
    std::list<std::pair<KeyT, ValueT>> cache_list_;
};

template <typename KeyT, typename ValueT>
class FIFOCache : public CacheBase<KeyT, ValueT> {
   public:
    using ListIterator = typename std::list<std::pair<KeyT, ValueT>>::iterator;

    explicit FIFOCache(size_t size) : CacheBase<KeyT, ValueT>(size) {}

    ~FIFOCache() = default;

    void put(KeyT&& key, ValueT&& val) override {
        auto it_cache = cache_map_.find(key);
        if (it_cache != cache_map_.end() && it_cache->second->second != val) {
            cache_list_.erase(it_cache->second);
            cache_map_.erase(it_cache);
        } else if (it_cache != cache_map_.end() && it_cache->second->second == val) {
            return;
        }
        cache_list_.push_front(std::make_pair(key, std::move(val)));
        cache_map_[std::move(key)] = cache_list_.begin();

        if (cache_map_.size() > this->size_) {
            del();
        }
    }

    void put(const KeyT& key, const ValueT& val) override {
        auto it_cache = cache_map_.find(key);
        if (it_cache != cache_map_.end() && it_cache->second->second != val) {
            cache_list_.erase(it_cache->second);
            cache_map_.erase(it_cache);
        } else if (it_cache != cache_map_.end() && it_cache->second->second == val) {
            return;
        }
        cache_list_.push_front(std::make_pair(key, val));
        cache_map_[key] = cache_list_.begin();

        if (cache_map_.size() > this->size_) {
            del();
        }
    }

    boost::optional<std::pair<KeyT, ValueT>> del() override {
        if (cache_map_.size() == 0) {
            return boost::none;
        }
        auto last = cache_list_.end();
        last--;
        KeyT key = last->first;
        ValueT objlist = last->second;
        cache_map_.erase(key);
        cache_list_.pop_back();
        return *last;
    }

    boost::optional<std::pair<KeyT, ValueT>> poll() override {
        if (cache_map_.size() == 0) {
            return boost::none;
        }
        return cache_list_.back();
    }

    bool exists(KeyT&& key) override { return cache_map_.find(std::move(key)) != cache_map_.end(); }

    bool exists(const KeyT& key) override { return cache_map_.find(key) != cache_map_.end(); }

    size_t get_size() override { return cache_map_.size(); }

   private:
    std::unordered_map<KeyT, ListIterator> cache_map_;
    std::list<std::pair<KeyT, ValueT>> cache_list_;
};
}  // namespace husky