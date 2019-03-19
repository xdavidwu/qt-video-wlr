project('qt-video-wlr',['c','cpp'])

wl_client=dependency('wayland-client')
wl_protocols=dependency('wayland-protocols')
scanner=find_program('wayland-scanner')
scanner_private_code=generator(scanner,output: '@BASENAME@-protocol.c',arguments: ['private-code','@INPUT@','@OUTPUT@'])
scanner_client_header=generator(scanner,output: '@BASENAME@-client-protocol.h',arguments: ['client-header','@INPUT@','@OUTPUT@'])

protocols_src=[
  scanner_private_code.process('wlr-layer-shell-unstable-v1.xml'),
  scanner_private_code.process(wl_protocols.get_pkgconfig_variable('pkgdatadir')+'/stable/xdg-shell/xdg-shell.xml')
]
protocols_headers=[
  scanner_client_header.process('wlr-layer-shell-unstable-v1.xml')
]
lib_protocols=static_library('protocols',protocols_src+protocols_headers,dependencies: wl_client)
protocols_dep=declare_dependency(link_with: lib_protocols,sources: protocols_headers)

qt5=import('qt5')
qt5_dep=dependency('qt5',modules: ['Widgets','Core','Multimedia','MultimediaWidgets','Gui'])
executable('qt-video-wlr',['qt-video-wlr-bg.cpp'],dependencies: [qt5_dep,protocols_dep],install: true)
