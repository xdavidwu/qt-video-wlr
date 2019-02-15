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

```sh
QT_WAYLAND_SHELL_INTEGRATION=foo ./qt-video-wlr FILES...
```

`QT_WAYLAND_SHELL_INTEGRATION` needs to be set to shell integration that doesn't exist to prevent qt from assigning role to window.

Paths are based on Archlinux and might be different on other distro.

You might need to change the variable name `namespace` in wlr-layer-shell-unstable-v1-client-protocol.h to make it work with c++ compilers.
