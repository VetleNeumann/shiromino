
{ pkgs ? import <nixpkgs> {
    # ↓ the overlay is applied *inside the same import* that defines pkgs
overlays = [
  (final: prev: {
    SDL2 = prev.libsdl2.override {
      x11Support = false;
      waylandSupport = false;
      pulseaudioSupport = false;
    };

    SDL2_image = prev.SDL2_image.override { sdl = final.SDL2; };
    SDL2_mixer = prev.SDL2_mixer.override { sdl = final.SDL2; };
  })
];
  } }:

let
  cross = pkgs.pkgsCross.mingwW64;   # shorter alias
in
pkgs.mkShell {
  name = "shiromino-cross-shell";

  nativeBuildInputs = [
    pkgs.cmake
    pkgs.ninja
    pkgs.gdb
    pkgs.pkg-config
    cross.buildPackages.gcc   # the cross GCC
  ];

  buildInputs = with cross; [
    SDL2        SDL2_image  SDL2_mixer   # ← now evaluate cleanly
    libvorbis   sqlite      libwebp      libtiff
  ];
}