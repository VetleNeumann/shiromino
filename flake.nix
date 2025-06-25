{
  description = "Shiromino C++ dev shell";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};

      sdl3_mixer = pkgs.stdenv.mkDerivation {
        pname = "sdl3-mixer";
        version = "dev";

        src = pkgs.fetchFromGitHub {
          owner = "libsdl-org";
          repo = "SDL_mixer";
          rev = "main";
          sha256 = "sha256-JaeDKCPCSBvLmXucw9emiqNXa7t2jX6zWR41WHGwkc4="; # Fill in later
        };

        nativeBuildInputs = [
          pkgs.cmake
          pkgs.pkg-config
        ];
        buildInputs = [
          pkgs.sdl3
          pkgs.alsa-lib
        ];

        cmakeFlags = [ "-DSDL3MIXER_VENDORED=ON" ];
      };

      # Shared inputs for both CI and dev
      commonInputs = [
        pkgs.stdenv.cc
        pkgs.cmake
        pkgs.ninja
        pkgs.pkg-config

        pkgs.sdl3
        pkgs.sdl3-image
        sdl3_mixer
        pkgs.lerc
        pkgs.libvorbis
        pkgs.libavif
        pkgs.sqlite
        pkgs.libwebp
        pkgs.libtiff
      ];
    in
    {
      devShells.${system} = {
        # Minimal CI shell
        ci = pkgs.mkShell { buildInputs = commonInputs; };

        # Dev shell adds tools on top of CI
        default = pkgs.mkShell {
          buildInputs = commonInputs ++ [
            pkgs.llvmPackages_20.clang-tools
            pkgs.gdb
            pkgs.nixfmt-rfc-style
          ];
        };

        # shellHook = ''
        #   # This is a workaround for WSL2, as ld does not support non-posix paths
        #   export PATH=$(echo "$PATH" | tr ':' '\n' | grep -v '/mnt/c/' | paste -sd:)
        # '';
      };
    };
}
