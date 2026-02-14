configure:
    cmake -S . -B build -G Ninja -DCMAKe_BUILT_TYPE=Release

build: configure
    cmake --build ./build        

test: build
    ctest --test-dir ./build
