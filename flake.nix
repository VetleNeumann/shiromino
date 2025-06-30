{
  description = "Shiromino C++ dev shell";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};

      llvm = pkgs.llvmPackages_20;

      sdl3_mixer = pkgs.stdenv.mkDerivation {
        pname = "sdl3-mixer";
        version = "dev";

        src = pkgs.fetchFromGitHub {
          owner = "libsdl-org";
          repo = "SDL_mixer";
          rev = "main";
          sha256 = "sha256-JaeDKCPCSBvLmXucw9emiqNXa7t2jX6zWR41WHGwkc4=";
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

      # Shared buildInputs for both CI and dev
      commonBuildInputs = [
        pkgs.stdenv.cc
        llvm.libcxxClang
        llvm.clang-tools
        pkgs.lld
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
      commonShellHooks = ''
        export LIBCXX_INCLUDE="${llvm.libcxx.dev}/include/c++/v1"
        export LIBCXX_LIBDIR="${llvm.libcxx.out}/lib"
        export GLIBC_INCLUDE="${pkgs.glibc.dev}/include"
      '';
    in
    {
      devShells.${system} = {
        # Minimal CI shell
        ci = pkgs.mkShell {
          buildInputs = commonBuildInputs;
          shellHook = commonShellHooks;
        };

        # Dev shell adds tools on top of CI
        default = pkgs.mkShell {
          buildInputs = commonBuildInputs ++ [
            pkgs.gdb
            pkgs.nixfmt-rfc-style
            pkgs.cmake-format
          ];
          shellHook = commonShellHooks;
        };
      };
    };
}
