#pragma once

#include <assert.h>
#include <pthread.h>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>
#include <list>
#include <type_traits>

#define LOGURU_WITH_STREAMS 1
#include "loguru.hpp"

namespace caches {

template <typename T, typename KeyT = int>
class CacheIdeal {
private:
    struct Node {
        T    value;
        KeyT key;
    };

    size_t capacity_ = 0ull;

    const std::vector<KeyT> access_patern_;
    size_t access_patern_index_ = 0ull;

    std::list<Node> list_;
    using ListIt  = typename std::list<Node>::iterator;

    std::map<KeyT, ListIt> map_;

public:
    explicit CacheIdeal(size_t capacity,
                        const std::vector<KeyT>& access_patern);

    template <typename Func>
        requires std::convertible_to<std::invoke_result_t<Func, KeyT>, T>
    std::pair<bool, T> Lookup(Func getter);

    std::pair<ListIt, size_t> GetFartherUsedKey()      const;
    size_t                    GetNextUsedKey(KeyT key) const;
};

template <typename T, typename KeyT>
CacheIdeal<T, KeyT>::CacheIdeal(size_t capacity,
                                const std::vector<KeyT>& access_patern)
    : capacity_(capacity),
      access_patern_(access_patern),
      access_patern_index_(0ull) {
}

template <typename T, typename KeyT>
template <typename Func>
    requires std::convertible_to<std::invoke_result_t<Func, KeyT>, T>
std::pair<bool, T> CacheIdeal<T, KeyT>::Lookup(Func getter) {
    if (access_patern_index_ >= access_patern_.size()) {
        throw std::runtime_error("Access patern ran out");
    }

    const auto key = access_patern_[access_patern_index_++];

    if (capacity_ == 0ull) {
        T page = getter(key);
        return {false, std::move(page)};
    }

    auto map_it = map_.find(key);
    if (map_it != map_.end()) {
        return {true, map_it->second->value};
    }

    size_t key_next_use = GetNextUsedKey(key);

    if (map_.size() == capacity_) {
        auto [farthest_it, farthest_idx] = GetFartherUsedKey();
        if (key_next_use > farthest_idx) {
            T page = getter(key);
            return {false, std::move(page)};
        }
        map_.erase(farthest_it->key);
        list_.erase(farthest_it);
    }

    T page = getter(key);
    list_.push_front(Node{std::move(page), key});
    map_[key] = list_.begin();
    return {false, list_.begin()->value};
}

template <typename T, typename KeyT>
size_t CacheIdeal<T, KeyT>::GetNextUsedKey(KeyT key) const {
    size_t access_patern_size = access_patern_.size();
    for (size_t i = access_patern_index_; i < access_patern_size; i++) {
        if (access_patern_[i] == key) {
            return i;
        }
    }
    return SIZE_MAX;
}

template <typename T, typename KeyT>
std::pair<typename CacheIdeal<T, KeyT>::ListIt, size_t>
CacheIdeal<T, KeyT>::GetFartherUsedKey() const {
    assert(!list_.empty());
    assert(list_.size() == map_.size());

    size_t farthest_idx = access_patern_index_;
    ListIt farthest_it = const_cast<std::list<Node>&>(list_).begin();

    const size_t access_patern_size = access_patern_.size();

    for (auto it = const_cast<std::list<Node>&>(list_).begin();
         it != const_cast<std::list<Node>&>(list_).end(); ++it) {
        size_t next_use = access_patern_size;
        for (size_t i = access_patern_index_; i < access_patern_size; ++i) {
            if (access_patern_[i] == it->key) {
                next_use = i;
                break;
            }
        }
        if (next_use >= farthest_idx) {
            farthest_idx = next_use;
            farthest_it = it;
        }
        if (farthest_idx == access_patern_size) break;
    }

    return {farthest_it, farthest_idx};
}

} // namespace caches