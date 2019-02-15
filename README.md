# qt-video-wlr

Proof of concept pip-mode-like video player for wlroots based wayland compositor

To compile:

```sh
wayland-scanner client-protocol wlr-layer-shell-unstable-v1.xml wlr-layer-shell-unstable-v1-client-protocol.h
wayland-scanner private-code wlr-layer-shell-unstable-v1.xml wlr-layer-shell-unstable-v1-protocol.c
wayland-scanner private-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-protocol.c
gcc xdg-shell-protocol.c wlr-layer-shell-unstable-v1-protocol.c -c
g++ qt-video-wlr-bg.cpp wlr-layer-shell-unstable-v1-protocol.o xdg-shell-protocol.o -I /usr/include/qt/ -fpic -lQt5Widgets -lQt5Core -lQt5Multimedia -lQt5MultimediaWidgets -lQt5Gui -lwayland-client -o qt-video-wlr
```

To run:

```
Usage: ./qt-video-wlr [options] FILE...
Qt pip-mode-like video player for wlroots based wayland compositor

Options:
  -h, --help             Displays this help.
  -v, --version          Displays version information.
  -l, --layer <layer>    Layer to render on, background, bottom, top or
                         overlay. Defaults to top.
  -w, --width <width>    Widget width. 0 to use max width. Defaults to 320.
  -e, --height <height>  Widget height. 0 to use max height. Defaults to 240.

Arguments:
  FILE                   Files to play.
```

Paths are based on Archlinux and might be different on other distro.

You might need to change the variable name `namespace` in wlr-layer-shell-unstable-v1-client-protocol.h to make it work with c++ compilers.
