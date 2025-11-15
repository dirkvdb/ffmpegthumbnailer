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
            pkgs = import inputs.nixpkgs { inherit system; };
            pkgsStatic = import inputs.nixpkgs {
              inherit system;
            };
          }
        );
    in
    {
      packages = forEachSupportedSystem (
        { pkgs, pkgsStatic }:
        let
          mkPackage =
            pkgsForBuild: pkgsForHost: isStatic:
            pkgsForHost.stdenv.mkDerivation {
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
                    if isStatic then
                      (ffmpeg-headless.override {
                        withGPL = true;
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
                        # Disable gnutls to avoid nettle linking issues in static builds
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
                        # These deps are only neede for encoding
                        withWebp = false;
                        withTheora = false;
                        withX264 = false;
                        withX265 = false;
                        withXvid = false;
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
                  libjpeg
                  libpng
                ]
                ++ pkgsForHost.lib.optionals (pkgsForHost.stdenv.isLinux && !isStatic) [
                  glib
                ];

              cmakeFlags = [
                "-DCMAKE_BUILD_TYPE=Release"
                "-DENABLE_THUMBNAILER=ON"
              ]
              ++ pkgsForHost.lib.optionals (!isStatic) [
                "-DBUILD_SHARED_LIBS=ON"
              ]
              ++ pkgsForHost.lib.optionals isStatic [
                "-DENABLE_TESTS=OFF"
                "-DENABLE_SHARED=OFF"
                "-DENABLE_STATIC=ON"
                "-DCMAKE_EXE_LINKER_FLAGS=-static"
              ]
              ++ pkgsForHost.lib.optionals (pkgsForHost.stdenv.isLinux && !isStatic) [
                "-DENABLE_GIO=ON"
              ];

              # Explicitly strip binaries completely (including static builds)
              stripAllList = [ "bin" ];

              meta = {
                description = "Lightweight video thumbnailer";
                homepage = "https://github.com/dirkvdb/ffmpegthumbnailer";
                platforms = pkgsForHost.lib.platforms.unix;
              };
            };
        in
        {
          default = mkPackage pkgs pkgs false;
          static = mkPackage pkgs pkgsStatic.pkgsStatic true;
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
