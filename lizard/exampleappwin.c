#include <gtk/gtk.h>

#include "exampleapp.h"
#include "exampleappwin.h"

int expanderSize = 1;
struct _ExampleAppWindow
{
  GtkApplicationWindow parent;

  GtkWidget *send_button;
  GtkWidget *channels;
};

G_DEFINE_TYPE(ExampleAppWindow, example_app_window, GTK_TYPE_APPLICATION_WINDOW);
void
generateExpander ()
{
  g_print ("Expander Generated\n");
  GtkWidget *expanderWidget;
  for (int i = 0; i < expanderSize; i++)
    {
      expanderWidget = gtk_expander_new_with_mnemonic ("More Options");
      gtk_widget_set_halign (expanderWidget, GTK_ALIGN_START);
      gtk_widget_set_valign (expanderWidget, GTK_ALIGN_START);
    }
}
static void
send_clicked ()
{
  g_print ("Message Sent\n");
}
/* Function must be declared above this line*/
static void
example_app_window_init (ExampleAppWindow *win)
{
  gtk_widget_init_template (GTK_WIDGET (win));
}

static void
example_app_window_class_init (ExampleAppWindowClass *class)
{
  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
                                               "/org/gtk/exampleapp/window.ui");
  //Creates a GtkWidget object that points to the object ID in window.ui
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), ExampleAppWindow, send_button);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), ExampleAppWindow, channels); //Figure out how to get children
  gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), send_clicked);

   generateExpander ();
}

ExampleAppWindow *
example_app_window_new (ExampleApp *app)
{
  return g_object_new (EXAMPLE_APP_WINDOW_TYPE, "application", app, NULL);
}

void
example_app_window_open (ExampleAppWindow *win,
                         GFile            *file)
{
}
