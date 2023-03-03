#include "lizard_ui.h"
#include "purple.h"

#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gi18n-lib.h>

#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>

#include <signal.h>
#include <string.h>
#include <unistd.h>

/*** ConnectionsUi ops ***/
static void network_disconnected(PurpleConnection *gc)
{

	printf("This machine has been disconnected from the internet\n");

}

static void report_disconnect(PurpleConnection *gc, PurpleConnectionError reason, const char* text)
{

	PurpleAccount *account = purple_connection_get_account(gc);
	printf("Connection disconnected: \"%s\" (%s)\n  >Error: %d\n  >Reason: %s\n", purple_account_get_username(account), purple_account_get_protocol_id(account), reason, text);

}

/*** ConversationUi ops ***/
static void lizard_write_conv(PurpleConversation *conv, PurpleMessage *msg)
{
	gchar *timestamp = purple_message_format_timestamp(msg, "(%H:%M:%S)");

	printf("(%s) %s %s: %s\n",
		purple_conversation_get_name(conv),
		timestamp,
		purple_message_get_author_alias(msg),
		purple_message_get_contents(msg));

	g_free(timestamp);
}

static PurpleConnectionUiOps connection_uiops =
{
    NULL, /* void (* connected)(PurpleConnection* gc);*/
    .disconnected = network_disconnected, /* void (* disconnected)(PurpleConnection* gc);*/ 
    .report_disconnect = report_disconnect, /* void (* report_disconnect)(PurpleConnection* gc,PurpleConnectionError reason,const char* text);*/
    //=================================//
    NULL, /*reserved*/
    NULL, /*reserved*/
    NULL, /*reserved*/
    NULL, /*reserved*/
};

static PurpleConversationUiOps conversation_uiops =
{
    NULL, /* void (* create_conversation)(PurpleConversation* conv);*/
    NULL, /* void (* destroy_conversation)(PurpleConversation* conv);*/
    NULL, /* void (* write_chat)(PurpleChatConversation* chat,PurpleMessage* msg);*/
    NULL, /* void (* write_im)(PurpleIMConversation* im,PurpleMessage* msg);*/
   .write_conv = lizard_write_conv, /* void (* write_conv)(PurpleConversation* conv,PurpleMessage* msg);*/
    NULL, /* void (* chat_add_users)(PurpleChatConversation* chat,GList* cbuddies,gboolean new_arrivals);*/
    NULL, /* void (* chat_rename_user)(PurpleChatConversation* chat,const char* old_name,const char* new_name,const char* new_alias);*/
    NULL, /* void (* chat_remove_users)(PurpleChatConversation* chat,GList* users);*/
    NULL, /* void (* chat_update_user)(PurpleChatUser* cb);*/
    NULL, /* void (* present)(PurpleConversation* conv);*/
    NULL, /* gboolean (* has_focus)(PurpleConversation* conv);*/
    NULL, /* void (* send_confirm) (PurpleConversation* conv,const char* message);*/
    NULL, /*reserved*/
    NULL, /*reserved*/
    NULL, /*reserved*/
    NULL, /*reserved*/
};

struct _LizardUi
{
    PurpleUi parent;
};

G_DECLARE_FINAL_TYPE (LizardUi, lizard_ui, LIZARD, UI, PurpleUi)

G_DEFINE_TYPE(LizardUi, lizard_ui, PURPLE_TYPE_UI)

static gboolean lizard_ui_start(G_GNUC_UNUSED PurpleUi *ui, G_GNUC_UNUSED GError **error) 
{
	purple_conversations_set_ui_ops(&conversation_uiops);
    purple_connections_set_ui_ops(&connection_uiops);
	return TRUE;
}

static gpointer lizard_ui_get_settings_backend(G_GNUC_UNUSED PurpleUi *ui) 
{
	return g_memory_settings_backend_new();
}

static void lizard_ui_init(G_GNUC_UNUSED LizardUi *ui)
{}

static void lizard_ui_class_init(LizardUiClass *klass)
{
	PurpleUiClass *ui_class = PURPLE_UI_CLASS(klass);

	ui_class->start = lizard_ui_start;
	ui_class->get_settings_backend = lizard_ui_get_settings_backend;
}

static PurpleUi* lizard_ui_new(void) 
{
	return g_object_new(
		lizard_ui_get_type(),
		"id", "lizard",       //change all this
		"name", "Lizard-UI",
		"version", "0.02312",
		"website", PURPLE_WEBSITE,
		"support-website", PURPLE_WEBSITE,
		"client-type", "test",
		NULL);
}

static gboolean ui_init_history(GError **error)
{
	PurpleHistoryManager *manager = NULL;
	PurpleHistoryAdapter *adapter = NULL;
	const gchar *id = NULL;

	manager = purple_history_manager_get_default();

	adapter = purple_sqlite_history_adapter_new(":memory:");
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

void ui_lizard_init()
{
    PurpleUi *ui = NULL;
	GError *error = NULL;
	
    /* libpurple's built-in DNS resolution forks processes to perform
	 * blocking lookups without blocking the main process.  It does not
	 * handle SIGCHLD itself, so if the UI does not you quickly get an army
	 * of zombie subprocesses marching around.
	 */
	signal(SIGCHLD, SIG_IGN); //SHOULD THIS BE HERE???

    /* set the magic PURPLE_PLUGINS_SKIP environment variable */
	g_setenv("PURPLE_PLUGINS_SKIP", "1", TRUE); //SHOULD THIS BE HERE???

    ui = lizard_ui_new();

	g_print("Hello World!");

    if (!purple_core_init(ui, &error)) {
		/* Initializing the core failed. Terminate. */
		fprintf(stderr,
		        _("Initialization of the libpurple core failed. %s\n"
		          "Aborting!\nPlease report this!\n"),
		        (error != NULL) ? error->message : "unknown error");
		g_clear_error(&error);
		abort();
	}
	
	

	if(!ui_init_history(&error)) {
		g_critical("failed to initialize the history api: %s",
		           error ? error->message : "unknown");
		g_clear_error(&error);
	}

	
    // CHANGE THIS FOR THE PLUGINS FOR FACEBOOK, SLACK AND MICROSOFT TEAMS
	/* Load the preferences. */
	purple_prefs_load();

	purple_prefs_add_none("~/devProjects/libpurple_subproj/libpurple");
	purple_prefs_add_path_list("~/devProjects/libpurple_subproj/libpurple/plugins", NULL);

	/* Load the desired plugins. The client should save the list of loaded plugins in
	 * the preferences using purple_plugins_save_loaded() */
	purple_plugins_load_saved("~/devProjects/libpurple_subproj/libpurple/plugins");
}