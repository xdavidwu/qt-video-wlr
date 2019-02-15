#include <QtCore/QUrl>
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
	zwlr_layer_surface_v1_ack_configure(surface, serial);
}

static void layer_surface_closed(void *data,
		struct zwlr_layer_surface_v1 *surface) {
	zwlr_layer_surface_v1_destroy(surface);
	wl_surface_destroy(wl_surface);
}

struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = layer_surface_configure,
	.closed = layer_surface_closed,
};

int main(int argc,char *argv[]){
	// hack: disable shell integration
	setenv("QT_WAYLAND_SHELL_INTEGRATION","nonexistance",1);
	QApplication app(argc, argv);
	QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
	struct wl_display *display = (struct wl_display *)
		native->nativeResourceForWindow("display", NULL);
	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);
	if (layer_shell == NULL) {
		fprintf(stderr, "layer_shell not available\n");
		return 1;
	}	

	auto player = new QMediaPlayer;
	auto playlist = new QMediaPlaylist;
	QStringList cmdline_args = QCoreApplication::arguments();
	if(cmdline_args.size()<2){
		std::cerr<<"Usage: "<<cmdline_args.at(0).toLocal8Bit().constData()
			<<" FILES..."<<std::endl;
		return 1;
	}
	for(int a=1;a<cmdline_args.size();a++)
		playlist->addMedia(QUrl::fromLocalFile(cmdline_args.at(a)));
	playlist->setPlaybackMode(QMediaPlaylist::Loop);
	player->setPlaylist(playlist);

	QWidget root;
	root.resize(320,240);
	root.show();
	wl_surface = static_cast<struct wl_surface *>(
		native->nativeResourceForWindow("surface", root.windowHandle()));
	layer_surface = zwlr_layer_shell_v1_get_layer_surface(layer_shell,
			wl_surface, NULL, ZWLR_LAYER_SHELL_V1_LAYER_TOP, "foo");
	zwlr_layer_surface_v1_set_size(layer_surface, 320, 240);
	zwlr_layer_surface_v1_set_anchor(layer_surface,ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT|
			ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM);
	zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener, layer_surface);
	wl_surface_commit(wl_surface);
	wl_display_roundtrip(display);

	auto videoWidget = new QVideoWidget(&root);
	player->setVideoOutput(videoWidget);
 	videoWidget->setMinimumSize(320,240);
	videoWidget->show();
	player->play();
	
	return app.exec();
}
