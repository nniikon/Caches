#pragma once

#include <assert.h>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <list>
#include <unordered_map>
#include <utility>

#define LOGURU_WITH_STREAMS 1
#include "loguru.hpp"

namespace caches {

template <typename T, typename KeyT = int>
class CacheLRU {
public:
    explicit CacheLRU(size_t capacity_);

    template <typename Func>
        requires std::convertible_to<std::invoke_result_t<Func, KeyT>, T>
    std::pair<bool, T&> Lookup(KeyT key, Func getter);

private:
    struct Node {
        T    value;
        KeyT key;
    };

    size_t capacity_ = 0ull;

    using List  = std::list<Node>;
    using ListIt = typename List::iterator;

    List list_;
    std::unordered_map<KeyT, ListIt> map_;

private:
    ListIt MakeMRU(ListIt it);

    template <typename Func>
    ListIt InsertNew(KeyT key, Func getter);

    void Dump() const;
};

template <typename T, typename KeyT>
CacheLRU<T, KeyT>::CacheLRU(size_t capacity)
    : capacity_(capacity), list_(), map_() {
    if (capacity_ == 0ull) {
        capacity_ = 1ull;
    }
    LOG_F(INFO, "===| Create CacheLRU with capacity %zu |===", capacity_);
}

template <typename T, typename KeyT>
template <typename Func>
    requires std::convertible_to<std::invoke_result_t<Func, KeyT>, T>
std::pair<bool, T&> CacheLRU<T, KeyT>::Lookup(KeyT key, Func getter) {
    assert(capacity_ > 0ull);

#ifndef NDEBUG
    Dump();
#endif

    LOG_S(INFO) << "CacheLRU: Looking up: <" << key << ">";

    auto hit = map_.find(key);
    if (hit == map_.end()) {
        LOG_F(INFO, "CacheLRU: Miss");
        auto it = InsertNew(key, getter);
        return {false, it->value};
    }

    LOG_F(INFO, "CacheLRU: Hit");
    auto it = MakeMRU(hit->second);
    return {true, it->value};
}

template <typename T, typename KeyT>
typename CacheLRU<T, KeyT>::ListIt
CacheLRU<T, KeyT>::MakeMRU(ListIt it) {
    if (it != list_.begin()) {
        list_.splice(list_.begin(), list_, it);
    }
    return list_.begin();
}

template <typename T, typename KeyT>
template <typename Func>
typename CacheLRU<T, KeyT>::ListIt
CacheLRU<T, KeyT>::InsertNew(KeyT key, Func getter) {
    auto page = getter(key);

    if (list_.size() >= capacity_) {
        map_.erase(list_.back().key);
        list_.pop_back();
    }

    list_.push_front(Node{std::move(page), key});
    map_[key] = list_.begin();
    return list_.begin();
}

template <typename T, typename KeyT>
void CacheLRU<T, KeyT>::Dump() const {
    std::cerr << "\n" "lru (MRU->LRU): ";
    for (const auto& n : list_) {
        std::cerr << n.key << " ";
    }
    std::cerr << "\n";
}

} // namespace caches