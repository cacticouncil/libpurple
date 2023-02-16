/*
 * pidgin
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>
#include <glib.h>
#include <locale.h>

#include <purple.h>

// #include "gtkaccount.h"
// #include "gtkblist.h"
// #include "gtkconn.h"
// #include "gtkxfer.h"
// #include "gtkidle.h"
// #include "gtkmedia.h"
// #include "gtknotify.h"
// #include "gtkprivacy.h"
// #include "gtkrequest.h"
// #include "gtkroomlist.h"
// #include "gtkutils.h"
// #include "gtkwhiteboard.h"
// #include "pidginapplication.h"
// #include "pidgincore.h"
// #include "pidgindebug.h"
// #include "pidginplugininfo.h"
// #include "pidginprefs.h"
// #include "pidginprivate.h"


static void purple_ui_add_protocol_theme_paths(PurpleProtocol *protocol) {
	GdkDisplay *display = NULL;
	GtkIconTheme *theme = NULL;
	const gchar *path = NULL;

	display = gdk_display_get_default();

	theme = gtk_icon_theme_get_for_display(display);

	path = purple_protocol_get_icon_search_path(protocol);
	if(path != NULL) {
		gtk_icon_theme_add_search_path(theme, path);
	}

	path = purple_protocol_get_icon_resource_path(protocol);
	if(path != NULL) {
		gtk_icon_theme_add_resource_path(theme, path);
	}
}

static void purple_ui_protocol_foreach_theme_cb(PurpleProtocol *protocol, gpointer data) {
	purple_ui_add_protocol_theme_paths(protocol);
}

static void purple_ui_protocol_registered_cb(PurpleProtocolManager *manager,
                                 PurpleProtocol *protocol)
{
	purple_ui_add_protocol_theme_paths(protocol);
}

static gboolean pidgin_history_init(GError **error) {
	PurpleHistoryManager *manager = NULL;
	PurpleHistoryAdapter *adapter = NULL;
	gchar *filename = NULL;
	const gchar *id = NULL;

	manager = purple_history_manager_get_default();

	/* Attempt to create the config_dir. We don't care about the result as the
	 * logging adapter will fail with a better error than us failing to create
	 * the directory.
	 */
	g_mkdir_with_parents(purple_config_dir(), 0700);

	filename = g_build_filename(purple_config_dir(), "history.db", NULL);
	adapter = purple_sqlite_history_adapter_new(filename);
	g_free(filename);

	id = purple_history_adapter_get_id(adapter);
	if(!purple_history_manager_register(manager, adapter, error)) {
		g_clear_object(&adapter);

		return FALSE;
	}

	/* The manager adds a ref to the adapter on registration, so we can remove
	 * our reference.
	 */
	g_clear_object(&adapter);

	return purple_history_manager_set_active(manager, id, error);
}

static void pidgin_ui_init(void)
{
	PurpleProtocolManager *protocol_manager = NULL;
	GdkDisplay *display = NULL;
	GtkIconTheme *theme = NULL;
	GError *error = NULL;
	gchar *path;

	pidgin_debug_init();

	display = gdk_display_get_default();
	theme = gtk_icon_theme_get_for_display(display);

	path = g_build_filename(PURPLE_DATADIR, "pidgin", "icons", NULL);
	gtk_icon_theme_add_search_path(theme, path);
	g_free(path);

	/* Add a callback for when a protocol is registered to add its icon paths
	 * if it was found after initial startup.
	 */
	protocol_manager = purple_protocol_manager_get_default();
	g_signal_connect(protocol_manager, "registered",
	                 G_CALLBACK(purple_ui_protocol_registered_cb), NULL);

	/* Add the icon paths for all the protocols that libpurple found at start
	 * up.
	 */
	purple_protocol_manager_foreach(protocol_manager,
	                                purple_ui_protocol_foreach_theme_cb, NULL);

	if(!pidgin_history_init(&error)) {
		g_critical("failed to initialize the history api: %s",
		           error != NULL ? error->message : "unknown");
		g_clear_error(&error);
	}

	/* Set the UI operation structures. */
	//purple_xfers_set_ui_ops(pidgin_xfers_get_ui_ops());
	//purple_blist_set_ui(PIDGIN_TYPE_BUDDY_LIST);
	//purple_notify_set_ui_ops(pidgin_notify_get_ui_ops());
	//purple_request_set_ui_ops(pidgin_request_get_ui_ops());
	purple_connections_set_ui_ops(lizard_connections_get_ui_ops());
	//purple_whiteboard_set_ui_ops(pidgin_whiteboard_get_ui_ops());
	//purple_idle_set_ui(pidgin_idle_new());

	purple_accounts_init();//pidgin_accounts_init();
	purple_connection_init();//pidgin_connection_init();
	//pidgin_request_init();
	//pidgin_blist_init();
	purple_conversations_init();//pidgin_conversations_init();
	//pidgin_commands_init();
	//idgin_privacy_init();
	//pidgin_xfers_init();
	//pidgin_roomlist_init();
	//pidgin_medias_init();
	//pidgin_notify_init();
}

// static void pidgin_quit(void)
// {
// 	/* Uninit */

// 	/* Be sure to close all windows that are not attached to anything
// 	 * (e.g., the debug window), or they may access things after they are
// 	 * shut down. */
// 	pidgin_notify_uninit();
// 	pidgin_commands_uninit();
// 	pidgin_conversations_uninit();
// 	pidgin_blist_uninit();
// 	pidgin_request_uninit();
// 	pidgin_connection_uninit();
// 	pidgin_accounts_uninit();
// 	pidgin_xfers_uninit();
// 	pidgin_debug_window_hide();
// 	pidgin_debug_uninit();

// 	/* and end it all... */
// 	g_application_quit(g_application_get_default());
// }

static PurpleCoreUiOps core_ops = {
	//.ui_prefs_init = pidgin_prefs_init,
	.ui_prefs_init = purple_prefs_init,
	.ui_init = pidgin_ui_init,
	.quit = NULL,//.quit = pidgin_quit,
};

PurpleCoreUiOps* lizard_core_get_ui_ops(void)
{
	return &core_ops;
}

int lizard_start(int argc, char *argv[])
{
	GApplication *app;
	int ret;

	bindtextdomain(PACKAGE, PURPLE_LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE);

	/* Locale initialization is not complete here.  See gtk_init_check() */
	setlocale(LC_ALL, "");

	app = pidgin_application_new();
	g_application_set_default(app);

	ret = g_application_run(app, argc, argv);

	/* Make sure purple has quit in case something in GApplication
	 * has caused g_application_run() to finish on its own. This can
	 * happen, for example, if the desktop session is ending.
	 */
	if (purple_get_core() != NULL) {
		purple_core_quit();
	}

	if (g_application_get_is_registered(app) &&
			g_application_get_is_remote(app)) {
		g_printerr("%s\n", _("Exiting because another libpurple client is "
		                     "already running."));
	}

	/* Now that we're sure purple_core_quit() has been called,
	 * this can be freed.
	 */
	g_object_unref(app);

	return ret;
}
