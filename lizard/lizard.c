#include <glib/gi18n.h>
#include "lizardcore.h"


int main(int argc, char *argv[])
{
     g_set_application_name("Lounge Lizard");
     return lizard_start(argc, argv);
}
