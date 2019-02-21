#include <QtCore/QUrl>
#include <QtCore/QFileInfo>
#include <QtCore/QCommandLineParser>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtGui/5.12.1/QtGui/qpa/qplatformnativeinterface.h>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaPlaylist>
#include <QtMultimediaWidgets/QVideoWidget>
#include <iostream>
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

static struct zwlr_layer_shell_v1 *layer_shell;
struct zwlr_layer_surface_v1 *layer_surface;
static struct wl_output *wl_output;
struct wl_surface *wl_surface;

uint32_t width = 320, height = 240;

QApplication *app;
QWidget *root;
QVideoWidget *videoWidget;

static void handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		layer_shell = (zwlr_layer_shell_v1*)wl_registry_bind(
				registry, name, &zwlr_layer_shell_v1_interface, 1);
	}
}

static void handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t name) {
	// who cares
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = handle_global_remove,
};

static void layer_surface_configure(void *data,
		struct zwlr_layer_surface_v1 *surface,
		uint32_t serial, uint32_t w, uint32_t h) {
	printf("resize %d  %d\n",w,h);
	root->resize(w, h);
	videoWidget->resize(w ,h);
	zwlr_layer_surface_v1_ack_configure(surface, serial);
}

static void layer_surface_closed(void *data,
		struct zwlr_layer_surface_v1 *surface) {
	zwlr_layer_surface_v1_destroy(surface);
	app->quit();
}

struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = layer_surface_configure,
	.closed = layer_surface_closed,
};

void playerStateSlot(QMediaPlayer::State state){
	if(state == QMediaPlayer::StoppedState){
		zwlr_layer_surface_v1_destroy(layer_surface);
		app->quit();
	}
}

int main(int argc,char *argv[]){
	// hack: disable shell integration
	setenv("QT_WAYLAND_SHELL_INTEGRATION","nonexistance",1);
	app = new QApplication(argc, argv);
	QApplication::setApplicationName("qt-video-wlr");
	QApplication::setApplicationVersion("dev");

	QCommandLineParser parser;
	parser.setApplicationDescription("Qt pip-mode-like video player "
		"for wlroots based wayland compositor");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("FILE","Files to play.","FILE...");
	QCommandLineOption layerOption(QStringList() << "l" << "layer",
		"Layer to render on, background, bottom, top or overlay."
		" Defaults to top.", "layer");
	QCommandLineOption widthOption(QStringList() << "w" << "width",
		"Widget width. 0 to use max width. Defaults to 320.", "width");
	QCommandLineOption heightOption(QStringList() << "e" << "height",
		"Widget height. 0 to use max height. Defaults to 240.", "height");
	QCommandLineOption colorOption(QStringList() << "c" << "color",
		"Background color, QColor::setNamedColor() format.", "color");
	QCommandLineOption volumeOption(QStringList() << "s" << "volume",
		"Linear sound volume, [0,100]. Defaults to 100.", "volume");
	QCommandLineOption positionOption(QStringList() << "p" << "position",
		"Widget position, top, top-left, top-right, bottom, "
		"bottom-left, bottom-right, left or right. Defaults "
		"to bottom-right.", "position");
	QCommandLineOption loopOption(QStringList() << "n" << "no-loop",
		"Do not loop. Defaults to loop.");
	parser.addOption(layerOption);
	parser.addOption(widthOption);
	parser.addOption(heightOption);
	parser.addOption(colorOption);
	parser.addOption(volumeOption);
	parser.addOption(positionOption);
	parser.addOption(loopOption);
	parser.process(*app);

	uint32_t layer = ZWLR_LAYER_SHELL_V1_LAYER_TOP;
	uint32_t anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
		ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
	if(parser.isSet(layerOption)){
		QString str = parser.value(layerOption);
		if(str == "background")
			layer = ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND;
		else if(str == "bottom")
			layer = ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM;
		else if(str == "overlay")
			layer = ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY;
		else if(str != "top")
			parser.showHelp(1);
	}
	if(parser.isSet(widthOption)){
		bool ok;
		width = parser.value(widthOption).toInt(&ok, 10);
		if(!ok||width < 0) parser.showHelp(1);
	}
	if(parser.isSet(heightOption)){
		bool ok;
		height = parser.value(heightOption).toInt(&ok, 10);
		if(!ok||height < 0) parser.showHelp(1);
	}
	if(parser.isSet(positionOption)){
		QString str = parser.value(positionOption);
		if(str == "top")
			anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
		else if(str == "top-left")
			anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
				ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
		else if(str == "top-right")
			anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
				ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
		else if(str == "bottom")
			anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
		else if(str == "bottom-left")
			anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
				ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
		else if(str == "left")
			anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
		else if(str == "right")
			anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
		else if(str != "bottom-right")
			parser.showHelp(1);
		if(width == 0)
			anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
		if(height == 0)
			anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
	}

	QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
	struct wl_display *display = (struct wl_display *)
		native->nativeResourceForWindow("display", NULL);
	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);
	if(layer_shell == NULL){
		fprintf(stderr, "layer_shell not available\n");
		return 1;
	}

	auto player = new QMediaPlayer;
	auto playlist = new QMediaPlaylist;
	QStringList pos_args = parser.positionalArguments();
	if(pos_args.size() == 0){
		parser.showHelp(1);
	}
	for(int a = 0;a < pos_args.size();a++)
		playlist->addMedia(QUrl::fromLocalFile(QFileInfo(pos_args.at(a))
			.absoluteFilePath()));
	if(!parser.isSet(loopOption))
		playlist->setPlaybackMode(QMediaPlaylist::Loop);
	player->setPlaylist(playlist);
	if(parser.isSet(volumeOption)){
		bool ok;
		player->setVolume(parser.value(volumeOption).toInt(&ok, 10));
		if(!ok||height < 0) parser.showHelp(1);
	}

	root = new QWidget;
	videoWidget = new QVideoWidget(root);
	root->resize(width ? width : 1, height ? height : 1);
	root->show();
	wl_surface = static_cast<struct wl_surface *>(
		native->nativeResourceForWindow("surface", root->windowHandle()));
	layer_surface = zwlr_layer_shell_v1_get_layer_surface(layer_shell,
		wl_surface, NULL, layer, "foo");
	zwlr_layer_surface_v1_set_size(layer_surface, width, height);
	zwlr_layer_surface_v1_set_anchor(layer_surface, anchor);
	zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener, layer_surface);
	wl_surface_commit(wl_surface);
	wl_display_roundtrip(display);

	player->setVideoOutput(videoWidget);
	videoWidget->setMinimumSize(width ? width : 1, height ? height : 1);
	if(parser.isSet(colorOption)){
		QColor qcolor(parser.value(colorOption));
		if(!qcolor.isValid()) parser.showHelp(1);
		QPalette pal;
		pal.setColor(QPalette::Background, qcolor);
		videoWidget->setAutoFillBackground(true);
		videoWidget->setPalette(pal);
		QPalette pal2;
		pal2.setColor(QPalette::Background, "#00000000");
		root->setAutoFillBackground(true);
		root->setPalette(pal2);
	}
	videoWidget->show();
	player->play();
	QObject::connect(player, &QMediaPlayer::stateChanged, playerStateSlot);

	return app->exec();
}
