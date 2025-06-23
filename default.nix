{ pkgs ? import <nixpkgs> {} }:

let
  sdl3-mixer = pkgs.stdenv.mkDerivation {
    pname = "sdl3-mixer";
    version = "dev";
    src = pkgs.fetchFromGitHub {
      owner = "libsdl-org";
      repo = "SDL_mixer";
      rev = "main";
      # Replace this with actual hash after first run
      sha256 = "sha256-JaeDKCPCSBvLmXucw9emiqNXa7t2jX6zWR41WHGwkc4=";
    };

    nativeBuildInputs = with pkgs; [ cmake pkg-config ];
    buildInputs = with pkgs; [ sdl3 libvorbis pkgs.alsa-lib ];

    cmakeFlags = [ "-DSDL3MIXER_VENDORED=ON" ]; # use vendored libs if you like
  };
in

pkgs.stdenv.mkDerivation {
  pname = "shiromino";
  version = "0.2.1";
  src = ./.;

  nativeBuildInputs = [
    pkgs.cmake
    pkgs.ninja
    pkgs.gdb
    pkgs.pkg-config
    pkgs.gcc
    pkgs.llvmPackages_20.clang-tools
  ];

  buildInputs = [
    pkgs.sdl3
    pkgs.sdl3-image
    sdl3-mixer
    pkgs.lerc
    pkgs.libvorbis
    pkgs.libavif
    pkgs.sqlite
    pkgs.libwebp
    pkgs.libtiff
  ];
}