
#ifndef LIZARD_CORE_H
#define LIZARD_CORE_H

#include <glib.h>

#include <purple.h>


/* change this only when we have a sane upgrade path for old prefs */
#define PIDGIN_PREFS_ROOT "/pidgin"

int lizard_start(int argc, char *argv[]);

PurpleCoreUiOps* lizard_core_get_ui_ops(void);

#endif /* LIZARD_CORE_H */