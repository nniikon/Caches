# Caches

Small C++ project implementing several cache algorithms (LRU, 2Q, and an ideal/perfect cache)

## Build

Clone the repository:
   ```bash
   git clone https://github.com/nniikon/Caches.git
   cd Caches
   git submodule update --init --recursive
   ```
Configure and build:
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build -j
   ```

## Tests

To run tests:
```bash
ctest --test-dir build --output-on-failure
```

## Run

Executables are placed in `build/`:
- `build/Caches2Q`
- `build/CachesPerfect`

Both read from stdin in the format:
```
<cache_size>
<input_size>
<vector of input_size integers>
```

Example:
```bash
echo "3 7 1 2 3 4 2 1 3" | build/Caches2Q
echo "3 7 1 2 3 4 2 1 3" | build/CachesPerfect
```
