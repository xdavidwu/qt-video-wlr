#include <QUrl>
#include <QWidget>
#include <QFileInfo>
#include <QApplication>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QMediaPlaylist>
#include <QCommandLineParser>
#include <qpa/qplatformnativeinterface.h>
#include <iostream>
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

struct qvw_state {
	struct zwlr_layer_shell_v1 *layer_shell;
	uint32_t width, height;
	QApplication *app;
	QWidget *root;
	QVideoWidget *videoWidget;
};

int main(int argc,char *argv[]){
	struct wl_surface *wl_surface;
	struct zwlr_layer_surface_v1 *layer_surface;
	struct qvw_state state {
		.app = new QApplication(argc, argv),
		.root = new QWidget,
	};

	// hack: disable shell integration
	setenv("QT_WAYLAND_SHELL_INTEGRATION","nonexistance",1);
	QApplication::setApplicationName("qt-video-wlr");
	QApplication::setApplicationVersion("dev");

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
	parser.addOptions({ layerOption, widthOption, heightOption, colorOption,
		volumeOption, positionOption, noLoopOption, marginOption,
		topMarginOption, rightMarginOption, bottomMarginOption,
		leftMarginOption });
	parser.process(*state.app);

	uint32_t layer, anchor;
	int32_t top_margin, right_margin, bottom_margin, left_margin;
	bool ok;
	QString layerStr = parser.value(layerOption);
	if (layerStr == "background")
		layer = ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND;
	else if (layerStr == "bottom")
		layer = ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM;
	else if (layerStr == "top")
		layer = ZWLR_LAYER_SHELL_V1_LAYER_TOP;
	else if (layerStr == "overlay")
		layer = ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY;
	else
		parser.showHelp(1);

	state.width = parser.value(widthOption).toInt(&ok);
	if (!ok) parser.showHelp(1);
	state.height = parser.value(heightOption).toInt(&ok);
	if (!ok) parser.showHelp(1);

	QString positionStr = parser.value(positionOption);
	if (positionStr == "center")
		anchor = 0;
	else if (positionStr == "top")
		anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
	else if (positionStr == "top-left")
		anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
			ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
	else if (positionStr == "top-right")
		anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
			ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
	else if (positionStr == "bottom")
		anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
	else if (positionStr == "bottom-left")
		anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
			ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
	else if (positionStr == "bottom-right")
		anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
			ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
	else if (positionStr == "left")
		anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
	else if (positionStr == "right")
		anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
	else
		parser.showHelp(1);
	if (state.width == 0)
		anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
			ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
	if (state.height == 0)
		anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
			ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;

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

	QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
	struct wl_display *display = (struct wl_display *)
		native->nativeResourceForWindow("display", NULL);
	struct wl_registry *registry = wl_display_get_registry(display);
	const struct wl_registry_listener registry_listener = {
		.global = [](void *data, struct wl_registry *registry,
				uint32_t name, const char *interface, uint32_t version) {
			struct qvw_state *state = static_cast<struct qvw_state *>(data);
			if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
				state->layer_shell = (zwlr_layer_shell_v1*)wl_registry_bind(
						registry, name, &zwlr_layer_shell_v1_interface, 1);
			}
		},
		.global_remove = [](void *data, struct wl_registry *registry, uint32_t name) {},
	};
	wl_registry_add_listener(registry, &registry_listener, &state);
	wl_display_roundtrip(display);
	if (state.layer_shell == NULL) {
		fprintf(stderr, "layer_shell not available\n");
		state.app->quit();
		return 1;
	}

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

	state.videoWidget = new QVideoWidget(state.root);
	state.root->resize(state.width ? state.width : 1, state.height ? state.height : 1);
	state.root->show();

	wl_surface = static_cast<struct wl_surface *>(
		native->nativeResourceForWindow("surface", state.root->windowHandle()));
	layer_surface = zwlr_layer_shell_v1_get_layer_surface(state.layer_shell,
		wl_surface, NULL, layer, "video-player");

	const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
		.configure = [](void *data,
				struct zwlr_layer_surface_v1 *surface,
				uint32_t serial, uint32_t w, uint32_t h) {
			struct qvw_state *state = static_cast<struct qvw_state *>(data);
			printf("resize %d %d\n", w, h);
			state->root->resize(w, h);
			state->videoWidget->resize(w ,h);
			zwlr_layer_surface_v1_ack_configure(surface, serial);
		},
		.closed = [](void *data,
				struct zwlr_layer_surface_v1 *surface) {
			struct qvw_state *state = static_cast<struct qvw_state *>(data);
			zwlr_layer_surface_v1_destroy(surface);
			state->app->quit();
		},
	};
	zwlr_layer_surface_v1_set_margin(layer_surface, top_margin, right_margin,
		bottom_margin, left_margin);
	zwlr_layer_surface_v1_set_size(layer_surface, state.width, state.height);
	zwlr_layer_surface_v1_set_anchor(layer_surface, anchor);
	zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener, &state);
	wl_surface_commit(wl_surface);
	wl_display_roundtrip(display);

	player.setVideoOutput(state.videoWidget);
	state.videoWidget->setMinimumSize(state.width ? state.width : 1, state.height ? state.height : 1);
	if (parser.isSet(colorOption)) {
		QColor qcolor(parser.value(colorOption));
		if(!qcolor.isValid()) parser.showHelp(1);
		QPalette pal;
		pal.setColor(QPalette::Window, qcolor);
		state.videoWidget->setAutoFillBackground(true);
		state.videoWidget->setPalette(pal);
		QPalette pal2;
		pal2.setColor(QPalette::Window, "#00000000");
		state.root->setAutoFillBackground(true);
		state.root->setPalette(pal2);
	}
	state.videoWidget->show();
	player.play();
	QObject::connect(&player, &QMediaPlayer::stateChanged,
		[&layer_surface, &state](QMediaPlayer::State playerState) {
			if(playerState == QMediaPlayer::StoppedState){
				zwlr_layer_surface_v1_destroy(layer_surface);
				state.app->quit();
			}
		});

	return state.app->exec();
}
