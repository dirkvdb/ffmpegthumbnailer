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
          }
        );
    in
    {
      packages = forEachSupportedSystem (
        { pkgs }:
        {
          default = pkgs.stdenv.mkDerivation {
            pname = "ffmpegthumbnailer";
            version = "dev";

            src = ./.;

            nativeBuildInputs = with pkgs; [
              cmake
              ninja
              pkg-config
            ];

            buildInputs =
              with pkgs;
              [
                ffmpeg-headless
                libjpeg
                libpng
              ]
              ++ pkgs.lib.optionals pkgs.stdenv.isLinux [
                glib
              ];

            cmakeFlags = [
              "-DCMAKE_BUILD_TYPE=Release"
              "-DENABLE_THUMBNAILER=ON"
            ]
            ++ pkgs.lib.optionals pkgs.stdenv.isLinux [
              "-DENABLE_GIO=ON"
            ];

            doCheck = true;

            meta = {
              description = "Lightweight video thumbnailer";
              homepage = "https://github.com/dirkvdb/ffmpegthumbnailer";
              platforms = pkgs.lib.platforms.unix;
            };
          };
        }
      );

      checks = forEachSupportedSystem (
        { pkgs }:
        {
          default = self.packages.${pkgs.system}.default;
        }
      );

      devShells = forEachSupportedSystem (
        { pkgs }:
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
