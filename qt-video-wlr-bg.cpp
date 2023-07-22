#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QUrl>
#include <QVBoxLayout>
#include <QVideoWidget>
#include <QWidget>
#include <QWindow>

#include <LayerShellQt/Window>
#include <LayerShellQt/Shell>

int main(int argc,char *argv[]){
	LayerShellQt::Shell::useLayerShell();
	QApplication::setApplicationName("qt-video-wlr");
	QApplication::setApplicationVersion("dev");
	// XXX: hack to make QWidget::windowHandle available before show
	// removed @ Qt6
	QApplication::setAttribute(Qt::AA_ImmediateWidgetCreation);

	QApplication app(argc, argv);
	QWidget root;

	QCommandLineParser parser;
	parser.setApplicationDescription("Qt pip-mode-like video player "
		"for wlroots based wayland compositor");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("FILE", "Files to play.", "FILE...");
	QCommandLineOption layerOption({"l", "layer"},
		"Layer to render on, background, bottom, top or overlay."
		" Defaults to top.", "layer", "top");
	QCommandLineOption widthOption({"w", "width"},
		"Widget width. 0 for max width. Defaults to 320.", "width", "320");
	QCommandLineOption heightOption({"e", "height"},
		"Widget height. 0 for max height. Defaults to 240.", "height", "240");
	QCommandLineOption colorOption({"c", "color"},
		"Background color, QColor::setNamedColor() format.", "color");
	QCommandLineOption volumeOption({"s", "volume"},
		"Linear sound volume, [0,100]. Defaults to 100.", "volume");
	QCommandLineOption positionOption({"p", "position"},
		"Widget position: center, top, top-left, top-right, "
		"bottom, bottom-left, bottom-right, left or right. "
		"Defaults to bottom-right.", "position", "bottom-right");
	QCommandLineOption noLoopOption({"n", "no-loop"},
		"Do not loop. Defaults to loop.");
	QCommandLineOption marginOption({"m", "margin"},
		"Widget margin. Edge-specific options take precedence if specified. "
		"No effect when not sticking to the edge. Defaults to 0.", "margin", "0");
	QCommandLineOption topMarginOption("top-margin",
		"Widget top margin. Defaults to 0.", "margin", "0");
	QCommandLineOption rightMarginOption("right-margin",
		"Widget right margin. Defaults to 0.", "margin", "0");
	QCommandLineOption bottomMarginOption("bottom-margin",
		"Widget bottom margin. Defaults to 0.", "margin", "0");
	QCommandLineOption leftMarginOption("left-margin",
		"Widget left margin. Defaults to 0.", "margin", "0");
	QCommandLineOption exclusiveZoneOption({"x", "exclusive-zone"},
		"Exclusive zone distance, -1 to draw over panels. Defaults to 0.", "distance", "0");
	parser.addOptions({ layerOption, widthOption, heightOption, colorOption,
		volumeOption, positionOption, noLoopOption, marginOption,
		topMarginOption, rightMarginOption, bottomMarginOption,
		leftMarginOption, exclusiveZoneOption });
	parser.process(app);

	bool ok;

	uint32_t width, height;
	width = parser.value(widthOption).toInt(&ok);
	if (!ok) parser.showHelp(1);
	height = parser.value(heightOption).toInt(&ok);
	if (!ok) parser.showHelp(1);

	auto layerShell = LayerShellQt::Window::get(root.windowHandle());
	LayerShellQt::Window::Layer layer;
	QString layerStr = parser.value(layerOption);
	if (layerStr == "background")
		layer = LayerShellQt::Window::LayerBackground;
	else if (layerStr == "bottom")
		layer = LayerShellQt::Window::LayerBottom;
	else if (layerStr == "top")
		layer = LayerShellQt::Window::LayerTop;
	else if (layerStr == "overlay")
		layer = LayerShellQt::Window::LayerOverlay;
	else
		parser.showHelp(1);
	layerShell->setLayer(layer);

	LayerShellQt::Window::Anchors anchor;
	QString positionStr = parser.value(positionOption);
	if (positionStr == "center")
		anchor = {};
	else if (positionStr == "top")
		anchor = LayerShellQt::Window::Anchor::AnchorTop;
	else if (positionStr == "top-left")
		anchor = {LayerShellQt::Window::Anchor::AnchorTop,
			LayerShellQt::Window::Anchor::AnchorLeft};
	else if (positionStr == "top-right")
		anchor = {LayerShellQt::Window::Anchor::AnchorTop,
			LayerShellQt::Window::Anchor::AnchorRight};
	else if (positionStr == "bottom")
		anchor = LayerShellQt::Window::Anchor::AnchorBottom;
	else if (positionStr == "bottom-left")
		anchor = {LayerShellQt::Window::Anchor::AnchorBottom,
			LayerShellQt::Window::Anchor::AnchorLeft};
	else if (positionStr == "bottom-right")
		anchor = {LayerShellQt::Window::Anchor::AnchorBottom,
			LayerShellQt::Window::Anchor::AnchorRight};
	else if (positionStr == "left")
		anchor = LayerShellQt::Window::Anchor::AnchorLeft;
	else if (positionStr == "right")
		anchor = LayerShellQt::Window::Anchor::AnchorRight;
	else
		parser.showHelp(1);
	if (width == 0)
		anchor |= {LayerShellQt::Window::Anchor::AnchorLeft,
			LayerShellQt::Window::Anchor::AnchorRight};
	if (height == 0)
		anchor |= {LayerShellQt::Window::Anchor::AnchorTop,
			LayerShellQt::Window::Anchor::AnchorBottom};
	layerShell->setAnchors(anchor);

	int top_margin, right_margin, bottom_margin, left_margin;
	top_margin = right_margin = bottom_margin = left_margin =
		parser.value(marginOption).toInt(&ok);
	if (!ok) parser.showHelp(1);
	top_margin = parser.value(topMarginOption).toInt(&ok);
	if (!ok) parser.showHelp(1);
	right_margin = parser.value(rightMarginOption).toInt(&ok);
	if (!ok) parser.showHelp(1);
	bottom_margin = parser.value(bottomMarginOption).toInt(&ok);
	if (!ok) parser.showHelp(1);
	left_margin = parser.value(leftMarginOption).toInt(&ok);
	if (!ok) parser.showHelp(1);
	layerShell->setMargins({left_margin, top_margin, right_margin, bottom_margin});

	int32_t exclusive_zone = parser.value(exclusiveZoneOption).toInt(&ok);
	if (!ok || exclusive_zone < -1) parser.showHelp(1);
	layerShell->setExclusiveZone(exclusive_zone);

	layerShell->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);

	layerShell->setDesiredOutput(nullptr);

	QVBoxLayout layout;
	QVideoWidget videoWidget;
	layout.addWidget(&videoWidget);
	layout.setContentsMargins({});
	root.setLayout(&layout);
	root.resize(width ? width : 1, height ? height : 1);
	root.show();

	QMediaPlayer player;
	QMediaPlaylist playlist;
	QStringList pos_args = parser.positionalArguments();
	if (pos_args.size() == 0)
		parser.showHelp(1);
	for (auto path: pos_args)
		playlist.addMedia(
			QUrl::fromLocalFile(QFileInfo(path).absoluteFilePath()));
	if (!parser.isSet(noLoopOption))
		playlist.setPlaybackMode(QMediaPlaylist::Loop);
	player.setPlaylist(&playlist);
	if (parser.isSet(volumeOption)) {
		player.setVolume(parser.value(volumeOption).toInt(&ok));
		if (!ok) parser.showHelp(1);
	}

	player.setVideoOutput(&videoWidget);
	if (parser.isSet(colorOption)) {
		QColor qcolor(parser.value(colorOption));
		if(!qcolor.isValid()) parser.showHelp(1);
		QPalette pal;
		pal.setColor(QPalette::Window, qcolor);
		videoWidget.setAutoFillBackground(true);
		videoWidget.setPalette(pal);
		QPalette pal2;
		pal2.setColor(QPalette::Window, "#00000000");
		root.setAutoFillBackground(true);
		root.setPalette(pal2);
	}
	videoWidget.show();
	player.play();
	QObject::connect(&player, &QMediaPlayer::stateChanged,
		[&app](QMediaPlayer::State playerState) {
			if(playerState == QMediaPlayer::StoppedState){
				app.quit();
			}
		});

	return app.exec();
}
