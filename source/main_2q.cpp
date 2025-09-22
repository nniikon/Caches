#include <cstddef>
#include <iostream>
#include <vector>
#include "cache2Q.hpp"

struct Page {
    void* ptr;
};

Page GetPage(int key) {
    return {nullptr};
}

using CacheType = caches::Cache2Q<Page, int>;

int main() {
    size_t cache_size = 0;
    std::cin >> cache_size;

    size_t input_size = 0;
    std::cin >> input_size;

    std::vector<int> input_vector(input_size);
    for (size_t i = 0; i < input_size; i++) {
        std::cin >> input_vector[i];
    }

    CacheType cache(cache_size);

    size_t hit_count = 0;
    for (size_t i = 0; i < input_size; i++) {
        std::pair<bool, Page&> lookup_res = cache.Lookup(input_vector[i], GetPage);
        hit_count += lookup_res.first;
    }

    std::cout << hit_count << '\n';
}