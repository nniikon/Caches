#include <cstddef>
#include <iostream>
#include <vector>
#include "cachePerfect.hpp"

struct Page {
    void* ptr;
};

Page GetPage(int key) {
    return {nullptr};
}

using CachePerfectType = caches::CacheIdeal<Page, int>;

int main() {
    size_t cache_size = 0;
    std::cin >> cache_size;

    size_t input_size = 0;
    std::cin >> input_size;

    std::vector<int> input_vector(input_size);
    for (size_t i = 0; i < input_size; i++) {
        std::cin >> input_vector[i];
    }

    CachePerfectType perfect_cache(cache_size, input_vector);

    size_t perfect_hit_count = 0;
    for (size_t i = 0; i < input_size; i++) {
        std::pair<bool, Page&> perfect_lookup_res = perfect_cache.Lookup(GetPage);
        perfect_hit_count += perfect_lookup_res.first;
    }

    std::cout << perfect_hit_count << '\n';
}