#pragma once

#include <assert.h>
#include <concepts>
#include <iostream>
#include <list>
#include <unordered_map>
#include <utility>
#include <variant>

#define LOGURU_WITH_STREAMS 1
#include "loguru.hpp"

namespace caches {

const float kDefaultCapacityFactorAm    = 0.75f;
const float kDefaultCapacityFactorA1out = 0.50f;

template <typename T, typename KeyT = int>
class Cache2Q {
public:
    explicit Cache2Q(size_t capacity_,
                     float capacity_factor_am     = kDefaultCapacityFactorAm,
                     float capacity_factor_a1_out = kDefaultCapacityFactorA1out);

    template <typename Func>
        requires std::convertible_to<std::invoke_result_t<Func, KeyT>, T>
    std::pair<bool, T> Lookup(KeyT key, Func getter);

private:
    size_t capacity_ = 0ull;

    size_t capacity_am_     = 0ull;
    size_t capacity_a1_in_  = 0ull;
    size_t capacity_a1_out_ = 0ull;

    struct Node {
        T    value;
        KeyT key;
    };

    using ListReal  = std::list<Node>;
    using ListGhost = std::list<KeyT>;

    ListReal  am_;
    ListReal  a1_in_;
    ListGhost a1_out_;

    using ListRealIt  = typename ListReal ::iterator;
    using ListGhostIt = typename ListGhost::iterator;

    struct AmIt    { ListRealIt  it; };
    struct A1InIt  { ListRealIt  it; };
    struct A1OutIt { ListGhostIt it; };

    using HashMapInfo = std::variant<std::monostate, AmIt, A1InIt, A1OutIt>;

    std::unordered_map<KeyT, HashMapInfo> map_;

private:
    HashMapInfo GetKeyInfo(KeyT key) const;

    ListRealIt AmMakeMRU       (ListRealIt iter);
    ListRealIt MoveToAmFromA1In(ListRealIt iter);

    template <typename Func>
    ListRealIt MoveToAmFromA1Out(ListGhostIt iter, Func getter);

    template <typename Func>
    ListRealIt InsertNewToA1In(KeyT key, Func getter);

    void Dump() const;
};

inline size_t get_am_capacity    (size_t capacity, float am_capacity_factor);
inline size_t get_a1_in_capacity (size_t capacity, float am_capacity_factor);
inline size_t get_a1_out_capacity(size_t capacity, float a1_out_capacity_factor);

template <typename T, typename KeyT>
Cache2Q<T, KeyT>::Cache2Q(size_t capacity, float capacity_factor_am,
                                           float capacity_factor_a1_out)
    : capacity_(capacity),
      capacity_am_    (get_am_capacity    (capacity, capacity_factor_am)),
      capacity_a1_in_ (get_a1_in_capacity (capacity, capacity_factor_am)),
      capacity_a1_out_(get_a1_out_capacity(capacity, capacity_factor_a1_out)),
      am_(),
      a1_in_(),
      a1_out_(),
      map_() {

    if (capacity_ < 3ull) {
        LOG_F(WARNING, "Cache2Q capacity cannot be less then 3");
        capacity_        = 3ull;
        capacity_am_     = get_am_capacity    (capacity_, capacity_factor_am);
        capacity_a1_in_  = get_a1_in_capacity (capacity_, capacity_factor_am);
        capacity_a1_out_ = get_a1_out_capacity(capacity,  capacity_factor_a1_out);
    }
    LOG_F(INFO, "===| Create Cache2Q with capacity %zu |===", capacity_);
}

inline size_t get_am_capacity(size_t capacity, float am_capacity_factor) {
    float am_capacity_f = static_cast<float>(capacity) * am_capacity_factor;
    return static_cast<size_t>(am_capacity_f);
}

inline size_t get_a1_in_capacity(size_t capacity, float am_capacity_factor) {
    size_t am_capacity = get_am_capacity(capacity, am_capacity_factor);
    return capacity - am_capacity;
}

inline size_t get_a1_out_capacity(size_t capacity, float a1_out_capacity_factor) {
    float a1_out_capacity_f = static_cast<float>(capacity) * a1_out_capacity_factor;
    return static_cast<size_t>(a1_out_capacity_f);
}

template <typename T, typename KeyT>
template <typename Func>
    requires std::convertible_to<std::invoke_result_t<Func, KeyT>, T>
std::pair<bool, T> Cache2Q<T, KeyT>::Lookup(KeyT key, Func getter) {
    assert(capacity_ > 0ull);

#ifndef NDEBUG
    Dump();
#endif

    HashMapInfo info = GetKeyInfo(key);

    LOG_S(INFO) << "Cache2Q: Looking up: <" << key << ">";

    if (std::holds_alternative<std::monostate>(info)) {
        LOG_F(INFO, "Cache2Q: Miss");
        auto it = InsertNewToA1In(key, getter);
        return {false, it->value};
    }

    if (auto p = std::get_if<AmIt>(&info)) {
        LOG_F(INFO, "Cache2Q: Found in Am");
        auto it = AmMakeMRU(p->it);
        return {true, it->value};
    }

    if (auto p = std::get_if<A1InIt>(&info)) {
        LOG_F(INFO, "Cache2Q: Found in A1_In");
        auto it = MoveToAmFromA1In(p->it);
        return {true, it->value};
    }

    if (auto p = std::get_if<A1OutIt>(&info)) {
        LOG_F(INFO, "Cache2Q: Found in A1_Out");
        auto it = MoveToAmFromA1Out(p->it, getter);
        return {false, it->value};
    }

    auto it = InsertNewToA1In(key, getter);
    return {false, it->value};
}

template <typename T, typename KeyT>
typename Cache2Q<T, KeyT>::HashMapInfo Cache2Q<T, KeyT>::GetKeyInfo(KeyT key) const {
    auto it = map_.find(key);
    if (it == map_.end()) return HashMapInfo{};
    return it->second;
}

template <typename T, typename KeyT>
typename Cache2Q<T, KeyT>::ListRealIt Cache2Q<T, KeyT>::AmMakeMRU(ListRealIt iter) {
    am_.splice(am_.begin(), am_, iter);
    return am_.begin();
}

template <typename T, typename KeyT>
template <typename Func>
typename Cache2Q<T, KeyT>::ListRealIt Cache2Q<T, KeyT>::InsertNewToA1In(KeyT key, Func getter) {
    auto new_elem = getter(key);

    if (a1_in_.size() >= capacity_a1_in_) {
        auto pushed_out = a1_in_.back().key;
        map_.erase(a1_in_.back().key);
        a1_in_.pop_back();

        if (a1_out_.size() >= capacity_a1_out_) {
            map_.erase(a1_in_.back().key);
            a1_out_.pop_back();
        }
        a1_out_.push_front(pushed_out);
        map_[pushed_out] = A1OutIt{a1_out_.begin()};
    }

    a1_in_.push_front(Node{std::move(new_elem), key});
    map_[key] = A1InIt{a1_in_.begin()};
    return a1_in_.begin();
}

template <typename T, typename KeyT>
typename Cache2Q<T, KeyT>::ListRealIt Cache2Q<T, KeyT>::MoveToAmFromA1In(ListRealIt a1_iter) {
    KeyT key = a1_iter->key;
    if (am_.size() >= capacity_am_) {
        map_.erase(am_.back().key);
        am_.pop_back();
    }

    am_.splice(am_.begin(), a1_in_, a1_iter);
    map_[key] = AmIt{am_.begin()};
    return am_.begin();
}

template <typename T, typename KeyT>
template <typename Func>
typename Cache2Q<T, KeyT>::ListRealIt Cache2Q<T, KeyT>::MoveToAmFromA1Out(ListGhostIt a1_out_iter, Func getter) {
    KeyT key = *a1_out_iter;

    a1_out_.erase(a1_out_iter);
    map_.erase(key);

    if (am_.size() >= capacity_am_) {
        map_.erase(am_.back().key);
        am_.pop_back();
    }

    auto val = getter(key);
    am_.push_front(Node{std::move(val), key});
    map_[key] = AmIt{am_.begin()};
    return am_.begin();
}

template <typename T, typename KeyT>
void Cache2Q<T, KeyT>::Dump() const {
    std::cerr << "\n" "am: ";
    for (const auto& a : am_) {
        std::cerr << a.key << " ";
    }
    std::cerr << "\n" "a1_in: ";
    for (const auto& a : a1_in_) {
        std::cerr << a.key << " ";
    }
    std::cerr << "\n" "a1_out: ";
    for (const auto& a : a1_out_) {
        std::cerr << a << " ";
    }
    std::cerr << '\n';
}

}  // namespace caches