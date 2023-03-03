#include "purple.h"
#include "lizard_ui.h"
#include <glib.h>

#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>

#define CUSTOM_USER_DIRECTORY  "~/libpurple/"

static void init_libpurple()
{
	//g_print("Hello World!");
    /* Set a custom user directory (optional) */
	purple_util_set_user_dir(CUSTOM_USER_DIRECTORY);

	/*debug init, look at the debug functions at https://docs.imfreedom.org/purple3/index.html#functions*/
	purple_debug_init();
    
    /*set the ui for libpurple*/
    ui_lizard_init();
	
	/*Initalizing subsystms*/
    purple_connections_init();
	
    purple_conversations_init();

}
int main(int argc, char *argv[])
{
	//GMainLoop *loop = g_main_loop_new(NULL, FALSE);
	//g_print("Hello World!");
	init_libpurple();

	printf("libpurple initialized. Running version %s.\n", purple_core_get_version()); //I like to see the version number

	PurpleAccount *account = purple_account_new("YOUR_IM_ACCOUNTS_USERNAME_HERE", "prpl-facebook"); //this could be prpl-aim, prpl-yahoo, prpl-msn, prpl-icq, etc.
    //purple_account_set_password(account, "YOUR_IM_ACCOUNTS_PASSWORD_HERE"); DEPRECATED FUNCTION

	//purple_accounts_add(account); DEPRECATED FUNCTION
	//purple_account_set_enabled(account, UI_ID, TRUE); DEPRECATED FUNCTION

    purple_account_connect(account);

	//g_main_loop_run(loop);

	return 0;

}