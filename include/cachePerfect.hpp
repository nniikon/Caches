#pragma once

#include <assert.h>
#include <map>
#include <stdexcept>
#include <vector>
#include <list>

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
    std::pair<bool, T&> Lookup(Func getter);

    ListIt GetFartherUsedKey();
};

template <typename T, typename KeyT>
CacheIdeal<T, KeyT>::CacheIdeal(size_t capacity,
                                const std::vector<KeyT>& access_patern)
    : capacity_(capacity),
      access_patern_(access_patern),
      access_patern_index_(0ull) {
    if (capacity_ == 0ull) {
        capacity_ = 1ull;
    }
}

template <typename T, typename KeyT>
template <typename Func>
    requires std::convertible_to<std::invoke_result_t<Func, KeyT>, T>
std::pair<bool, T&> CacheIdeal<T, KeyT>::Lookup(Func getter) {
    if (access_patern_index_ >= access_patern_.size()) {
        throw std::runtime_error("Access patern ran out");
    }

    const auto key = access_patern_[access_patern_index_++];

    auto map_it = map_.find(key);
    if (map_it != map_.end()) {
        return {true, map_it->second->value};
    }

    if (map_.size() == capacity_) {
        auto further_used_key = GetFartherUsedKey();
        map_.erase(further_used_key->key);
        list_.erase(further_used_key);
    }

    auto page = getter(key);
    list_.push_front(Node{std::move(page), key});
    map_[key] = list_.begin();
    return {false, list_.begin()->value};
}

template <typename T, typename KeyT>
CacheIdeal<T, KeyT>::ListIt CacheIdeal<T, KeyT>::GetFartherUsedKey() {
    assert(!list_.empty());
    assert(list_.size() == map_.size());

    size_t total_farthest = 0;
    ListIt total_farthest_it = list_.begin();

    const size_t access_patern_size = access_patern_.size();

    for (auto list_it = list_.begin(); list_it != list_.end(); ++list_it) {
        size_t next_use = access_patern_size;

        for (size_t i = access_patern_index_; i < access_patern_size; ++i) {
            if (access_patern_[i] == list_it->key) {
                next_use = i;
                break;
            }
        }

        if (next_use >= total_farthest) {
            total_farthest = next_use;
            total_farthest_it = list_it;
        }

        if (total_farthest == access_patern_size) {
            break;
        }
    }

    return total_farthest_it;
}

} // namespace