project('qt-video-wlr', ['cpp'])

qt5_dep = dependency('qt5', modules: ['Widgets', 'Core', 'Multimedia', 'MultimediaWidgets', 'Gui'])
layer_shell_qt_dep = dependency('LayerShellQt', modules: ['LayerShellQt::Interface'])
executable('qt-video-wlr',
  ['qt-video-wlr-bg.cpp'],
  dependencies: [qt5_dep, layer_shell_qt_dep],
  install: true)
