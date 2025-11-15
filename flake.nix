{
  description = "ffmpegthumbnailer";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-25.05";
  };

  outputs =
    { self, ... }@inputs:

    let
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      forEachSupportedSystem =
        f:
        inputs.nixpkgs.lib.genAttrs supportedSystems (
          system:
          f {
            pkgs = import inputs.nixpkgs {
              inherit system;
            };

            pkgsStatic = import inputs.nixpkgs {
              inherit system;
            };

            pkgsWindows = (
              import inputs.nixpkgs {
                inherit system;
                crossSystem = {
                  config = "x86_64-w64-mingw32";
                };
                config = {
                  allowBroken = true;
                };
              }
            );
          }
        );
    in
    {
      packages = forEachSupportedSystem (
        {
          pkgs,
          pkgsStatic,
          pkgsWindows,
        }:
        let
          mkPackage =
            pkgsForBuild: pkgsForHost: isStatic: isWindows:
            let
              baseStdenv = pkgsForHost.stdenv;

              # On Windows, use win32 threads to get a fully static binary
              stdenv' =
                if isWindows && baseStdenv.cc.isGNU && baseStdenv.targetPlatform.isWindows then
                  let
                    buildPkgs = pkgsForHost.buildPackages;
                    gccWin32 = buildPkgs.wrapCC (
                      buildPkgs.gcc-unwrapped.override {
                        threadsCross = {
                          model = "win32";
                          package = null;
                        };
                      }
                    );
                  in
                  pkgsForHost.overrideCC baseStdenv gccWin32
                else
                  baseStdenv;

              # zlib-ng in zlib-compatible mode
              zlibNgCompat = pkgsForHost.zlib-ng.override {
                withZlibCompat = true;
              };

              # Use zlib-ng and force it to be static-only (we mainly care on Windows)
              zlibNgStatic = zlibNgCompat.overrideAttrs (old: {
                dontDisableStatic = true;
                cmakeFlags = (old.cmakeFlags or [ ]) ++ [
                  "-DBUILD_SHARED_LIBS=OFF"
                ];
              });

              # libpng that uses zlib-ng instead of plain zlib
              libpngWithZlibNg = pkgsForHost.libpng.override {
                zlib = zlibNgStatic;
              };

            in
            stdenv'.mkDerivation {
              pname = "ffmpegthumbnailer";
              version = "dev";

              src = ./.;

              nativeBuildInputs = with pkgsForBuild; [
                cmake
                ninja
                pkg-config
              ];

              buildInputs =
                with pkgsForHost;
                [
                  (
                    if isStatic || isWindows then
                      (ffmpeg-headless.override {
                        withGPL = true;
                        withShared = false;
                        withStatic = true;
                        zlib = zlibNgStatic;

                        # Disable CUDA/LLVM to avoid compiler-rt dependency
                        withCuda = false;
                        withCudaLLVM = false;
                        withCudaNVCC = false;
                        buildAvdevice = false;
                        buildSwresample = false;
                        buildFfmpeg = false;
                        buildFfplay = false;
                        buildFfprobe = false;
                        buildPostproc = false;
                        withBin = false;
                        withDocumentation = false;
                        withHtmlDoc = false;
                        withPodDoc = false;
                        withTxtDoc = false;
                        withManPages = false;
                        # Disable all audio dependencies (not needed for thumbnailing)
                        withPulse = false;
                        withAlsa = false;
                        withJack = false;
                        withSdl2 = false;
                        withOpenal = false;
                        withOpus = false;
                        withVorbis = false;
                        withMp3lame = false;
                        withSpeex = false;
                        withSoxr = false;
                        withAmf = false;
                        withCelt = false;
                        # Disable X11 for headless
                        withXcb = false;
                        withFontconfig = false;
                        withFreetype = false;
                        withAss = false;
                        withFribidi = false;
                        withHarfbuzz = false;
                        # Disable other unnecessary features
                        withSsh = false;
                        withSrt = false;
                        withBluray = false;
                        withOpencl = false;
                        withOpenmpt = false;
                        withVulkan = false;
                        withVaapi = false;
                        withVdpau = false;
                        # Disable gnutls to avoid gettext/libevent/unbound deps
                        withGnutls = false;
                        withVmaf = false;
                        withVidStab = false;
                        withRist = false;
                        withZvbi = false;
                        withAom = false;
                        withZimg = false;
                        withOpenjpeg = false;
                        withV4l2 = false;
                        withDrm = false;
                        # only needed for dash demuxing
                        withXml2 = false;
                        # These deps are only needed for encoding
                        withWebp = false;
                        withTheora = false;
                        withX264 = false;
                        withX265 = false;
                        withXvid = false;
                        withSvtav1 = false;
                        # Disable dav1d for static macOS builds
                        withDav1d = if (isStatic && stdenv'.isDarwin) then false else true;
                      }).overrideAttrs
                        (old: {
                          doCheck = false;
                          configureFlags = old.configureFlags ++ [
                            "--disable-encoders"
                            "--disable-muxers"
                            "--disable-devices"
                            "--disable-hwaccels"
                          ];
                        })
                    else
                      ffmpeg-headless
                  )
                  # Use static versions of libpng and libjpeg for Windows
                  (
                    if isWindows then
                      libjpeg.override {
                        enableStatic = true;
                        enableShared = false;
                      }
                    else
                      libjpeg
                  )
                  (
                    if isWindows then
                      libpngWithZlibNg.overrideAttrs (old: {
                        dontDisableStatic = true;
                        configureFlags = (old.configureFlags or [ ]) ++ [
                          "--enable-static"
                          "--disable-shared"
                        ];
                      })
                    else
                      libpng
                  )

                ]
                ++ pkgsForHost.lib.optionals isWindows [
                  # ffmpeg transitive dependencies needed for linking on Windows
                  zlibNgStatic
                  (xz.overrideAttrs (old: {
                    dontDisableStatic = true;
                    configureFlags = (old.configureFlags or [ ]) ++ [
                      "--enable-static"
                      "--disable-shared"
                    ];
                  }))
                  (bzip2.overrideAttrs (old: {
                    dontDisableStatic = true;
                    configureFlags = (old.configureFlags or [ ]) ++ [
                      "--enable-static"
                      "--disable-shared"
                    ];
                  }))
                  (libiconv.overrideAttrs (old: {
                    dontDisableStatic = true;
                    configureFlags = (old.configureFlags or [ ]) ++ [
                      "--enable-static"
                      "--disable-shared"
                    ];
                  }))
                  (dav1d.overrideAttrs (old: {
                    mesonFlags = (old.mesonFlags or [ ]) ++ [
                      "-Ddefault_library=static"
                      "-Denable_tools=false"
                      "-Denable_tests=false"
                    ];
                  }))
                ]
                ++ pkgsForHost.lib.optionals (pkgsForHost.stdenv.isLinux && !isStatic) [
                  glib
                ];

              cmakeFlags = [
                "-DCMAKE_BUILD_TYPE=Release"
              ]
              ++ pkgsForHost.lib.optionals (!isWindows) [
                "-DENABLE_THUMBNAILER=ON"
              ]
              ++ pkgsForHost.lib.optionals (!isStatic && !isWindows) [
                "-DBUILD_SHARED_LIBS=ON"
              ]
              ++ pkgsForHost.lib.optionals (isStatic || isWindows) [
                "-DENABLE_TESTS=OFF"
                "-DENABLE_SHARED=OFF"
                "-DENABLE_STATIC=ON"
              ]
              ++ pkgsForHost.lib.optionals (isStatic && !isWindows && pkgsForHost.stdenv.isLinux) [
                "-DCMAKE_EXE_LINKER_FLAGS=-static"
              ]
              ++ pkgsForHost.lib.optionals isStatic [
                "-DENABLE_FULL_STATIC=ON"
              ]
              ++ pkgsForHost.lib.optionals (pkgsForHost.stdenv.isLinux && !isStatic && !isWindows) [
                "-DENABLE_GIO=ON"
              ];

              # Explicitly strip binaries completely (including static builds)
              stripAllList = [ "bin" ];

              meta = {
                description = "Lightweight video thumbnailer";
                homepage = "https://github.com/dirkvdb/ffmpegthumbnailer";
                platforms = pkgsForHost.lib.platforms.unix ++ pkgsForHost.lib.platforms.windows;
              };
            };
        in
        {
          default = mkPackage pkgs pkgs false false;
          static = mkPackage pkgs pkgsStatic.pkgsStatic true false;
          windows = mkPackage pkgs pkgsWindows true true;
        }
      );

      checks = forEachSupportedSystem (
        { pkgs, ... }:
        {
          default = self.packages.${pkgs.system}.default;
          static = self.packages.${pkgs.system}.static;
        }
      );

      devShells = forEachSupportedSystem (
        { pkgs, ... }:
        let
          pkg = self.packages.${pkgs.system}.default;
        in
        {
          default =
            pkgs.mkShell.override
              {
                # Override stdenv in order to change compiler:
                # stdenv = pkgs.clangStdenv;
              }
              {
                inputsFrom = [ pkg ];
                packages =
                  with pkgs;
                  [
                    # development tools
                    clang-tools
                  ]
                  ++ (if pkgs.system == "aarch64-darwin" then [ ] else [ gdb ]);
              };
        }
      );
    };
}
