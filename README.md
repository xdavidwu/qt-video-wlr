# qt-video-wlr

Qt pip-mode-like video player for wlroots based wayland compositors.

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
  -h, --help                 Displays this help.
  -v, --version              Displays version information.
  -l, --layer <layer>        Layer to render on, background, bottom, top or
                             overlay. Defaults to top.
  -w, --width <width>        Widget width. 0 to use max width. Defaults to 320.
  -e, --height <height>      Widget height. 0 to use max height. Defaults to
                             240.
  -c, --color <color>        Background color, QColor::setNamedColor() format.
  -s, --volume <volume>      Linear sound volume, [0,100]. Defaults to 100.
  -p, --position <position>  Widget position, center, top, top-left, top-right,
                             bottom, bottom-left, bottom-right, left ro right.
                             Defaults to bottom-right.
  -n, --no-loop              Do not loop. Defaults to loop.
  -m, --margin <margin>      Widget margin. Edge-specific options take
                             precedence if specified. No effect when not
                             sticking to the edge. Defaults to 0.
  --top-margin <margin>      Widget top margin. Defaults to 0.
  --right-margin <margin>    Widget right margin. Defaults to 0.
  --bottom-margin <margin>   Widget bottom margin. Defaults to 0.
  --left-margin <margin>     Widget left margin. Defaults to 0.

Arguments:
  FILE                       Files to play.
```

Paths are based on Archlinux and might be different on other distro.
