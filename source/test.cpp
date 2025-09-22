#include <gtest/gtest.h>
#include <cstddef>
#include <vector>

#include "cache2Q.hpp"
#include "cacheLRU.hpp"
#include "cachePerfect.hpp"

struct Page {
    void* data;
};

typedef Page (*get_page_func)(int);
Page get_page_long(int id) {
    return Page{nullptr}; // not so long =)
}

template <typename CacheType = caches::CacheLRU<Page, int>>
size_t CheckHitrate(size_t cache_size, size_t n_inputs, const std::vector<int>& input) {
    assert(n_inputs == input.size());

    CacheType lru_cache(cache_size);

    size_t hit_count = 0;
    for (size_t i = 0; i < input.size(); i++) {
        if (lru_cache.template Lookup<get_page_func>(input[i], get_page_long).first) {
            hit_count += 1;
        }
    }

    return hit_count;
}

TEST(LRUCache, General) {
    EXPECT_EQ((CheckHitrate<caches::CacheLRU<Page, int>>(3,  6,  std::vector{1, 2, 3, 2, 1, 3})),                   3);
    EXPECT_EQ((CheckHitrate<caches::CacheLRU<Page, int>>(2,  6,  std::vector{1, 2, 1, 2, 1, 2})),                   4);
    EXPECT_EQ((CheckHitrate<caches::CacheLRU<Page, int>>(3,  8,  std::vector{1, 2, 3, 1, 4, 1, 2, 3})),             2);
    EXPECT_EQ((CheckHitrate<caches::CacheLRU<Page, int>>(3,  12, std::vector{1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5})), 2);
    EXPECT_EQ((CheckHitrate<caches::CacheLRU<Page, int>>(4,  7,  std::vector{1, 2, 3, 4, 3, 2, 1})),                3);
    EXPECT_EQ((CheckHitrate<caches::CacheLRU<Page, int>>(2,  6,  std::vector{1, 2, 3, 1, 2, 3})),                   0);
    EXPECT_EQ((CheckHitrate<caches::CacheLRU<Page, int>>(10, 5,  std::vector{1, 2, 1, 2, 3})),                      2);
    EXPECT_EQ((CheckHitrate<caches::CacheLRU<Page, int>>(0,  6,  std::vector{1, 2, 3, 1, 2, 3})),                   0);
    EXPECT_EQ((CheckHitrate<caches::CacheLRU<Page, int>>(1,  5,  std::vector{7, 7, 7, 7, 7})),                      4);
    EXPECT_EQ((CheckHitrate<caches::CacheLRU<Page, int>>(1,  6,  std::vector{1, 2, 1, 2, 1, 2})),                   0);
}

TEST(Cache2Q, General) {
    EXPECT_EQ((CheckHitrate<caches::Cache2Q<Page, int>>(8, 15, std::vector<int>{1, 2, 3, 4, 3, 4, 1, 2, 5, 6, 5, 6, 7, 7, 3})),       5);
    EXPECT_EQ((CheckHitrate<caches::Cache2Q<Page, int>>(8, 17, std::vector<int>{1, 2, 3, 4, 3, 4, 1, 2, 5, 6, 5, 6, 3, 7, 7, 3, 4})), 7);
    EXPECT_EQ((CheckHitrate<caches::Cache2Q<Page, int>>(8, 6,  std::vector<int>{1, 2, 3, 2, 1, 3})),             2);
    EXPECT_EQ((CheckHitrate<caches::Cache2Q<Page, int>>(8, 6,  std::vector<int>{1, 2, 3, 4, 1, 1})),             1);
    EXPECT_EQ((CheckHitrate<caches::Cache2Q<Page, int>>(8, 6,  std::vector<int>{1, 2, 3, 2, 2, 2})),             3);
    EXPECT_EQ((CheckHitrate<caches::Cache2Q<Page, int>>(8, 10, std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 1})), 0);
    EXPECT_EQ((CheckHitrate<caches::Cache2Q<Page, int>>(8, 7,  std::vector<int>{1, 2, 3, 2, 2, 2, 2})),          4);
    EXPECT_EQ((CheckHitrate<caches::Cache2Q<Page, int>>(8, 9,  std::vector<int>{1, 2, 3, 4, 1, 2, 5,1,2})),      2);
    EXPECT_EQ((CheckHitrate<caches::Cache2Q<Page, int>>(8, 4,  std::vector<int>{1, 2, 3, 1})),                   0);
    EXPECT_EQ((CheckHitrate<caches::Cache2Q<Page, int>>(8, 6,  std::vector<int>{1, 2, 3, 4,5,6})),               0);
}

template <typename CacheType = caches::CacheIdeal<Page, int>>
size_t CheckHitrateIdeal(size_t cache_size, size_t /*n_inputs*/, const std::vector<int>& input) {
    CacheType ideal_cache(cache_size, input);
    size_t hit_count = 0;
    for (size_t i = 0; i < input.size(); i++) {
        if (ideal_cache.template Lookup<get_page_func>(get_page_long).first) {
            hit_count += 1;
        }
    }
    return hit_count;
}

TEST(CacheIdealGeneral, WarmUpThenHits) {
    EXPECT_EQ((CheckHitrateIdeal(3, 12, std::vector{1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5})), 5);
    EXPECT_EQ((CheckHitrateIdeal(3, 6,  std::vector{1, 2, 3, 2, 1, 3})),                   3);
    EXPECT_EQ((CheckHitrateIdeal(2, 6,  std::vector{1, 2, 1, 2, 1, 2})),                   4);
    EXPECT_EQ((CheckHitrateIdeal(3, 8,  std::vector{1, 2, 3, 1, 4, 1, 2, 3})),             3);
    EXPECT_EQ((CheckHitrateIdeal(4, 7,  std::vector{1, 2, 3, 4, 3, 2, 1})),                3);
    EXPECT_EQ((CheckHitrateIdeal(2, 6,  std::vector{1, 2, 3, 1, 2, 3})),                   2);
    EXPECT_EQ((CheckHitrateIdeal(10, 5, std::vector{1, 2, 1, 2, 3})),                      2);
    EXPECT_EQ((CheckHitrateIdeal(0, 6,  std::vector{1, 2, 3, 1, 2, 3})),                   0);
    EXPECT_EQ((CheckHitrateIdeal(1, 5,  std::vector{7, 7, 7, 7, 7})),                      4);
    EXPECT_EQ((CheckHitrateIdeal(1, 6,  std::vector{1, 2, 1, 2, 1, 2})),                   0);
}