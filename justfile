configure:
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

build: configure
    cmake --build ./build        

test: build
    ctest --test-dir ./build

cache output="default":
    nix build --accept-flake-config --no-link '.#{{output}}'
    nix copy --accept-flake-config --no-check-sigs --to 'https://nix.cloudsmith.io/open-source/ffmpegthumbnailer?compression=zstd' '.#{{output}}'
