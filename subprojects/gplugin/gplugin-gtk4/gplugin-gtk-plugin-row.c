/*
 * Copyright (C) 2021 Elliott Sales de Andrade <quantum.analyst@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <glib/gi18n-lib.h>

#include <gplugin.h>

#include <gplugin-gtk-plugin-row.h>

/**
 * GPluginGtkPluginRow:
 *
 * A widget that displays a [iface@GPlugin.Plugin] in a user friendly way,
 * intended to be placed in a [class@Gtk.ListBox].
 */

/******************************************************************************
 * Structs
 *****************************************************************************/
struct _GPluginGtkPluginRow {
	GtkListBoxRow parent;

	GPluginPlugin *plugin;
	gulong signal_id;

	/* Header */
	GtkWidget *enable;
	GtkWidget *title;
	GtkWidget *summary;
	GtkWidget *version;
	GtkWidget *config;
	GtkWidget *revealer;

	/* Details */
	GtkWidget *description;
	GtkWidget *authors_box;
	GtkWidget *website;
	GtkWidget *dependencies_box;
	GtkWidget *error;
	GtkWidget *id;
	GtkWidget *filename;
	GtkWidget *abi_version;
	GtkWidget *loader;
	GtkWidget *internal;
	GtkWidget *load_on_query;
};

/******************************************************************************
 * Enums
 *****************************************************************************/
enum {
	PROP_ZERO,
	PROP_PLUGIN,
	PROP_EXPANDED,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {
	NULL,
};

enum {
	SIG_PLUGIN_STATE_SET,
	N_SIGNALS,
};
static guint signals[N_SIGNALS] = {
	0,
};

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
_gplugin_gtk_plugin_row_refresh(GPluginGtkPluginRow *row)
{
	GtkWidget *widget = NULL;
	GPluginPluginState state = GPLUGIN_PLUGIN_STATE_UNKNOWN;
	GError *error = NULL;
	gchar *name = NULL, *version = NULL, *website = NULL;
	gchar *summary = NULL, *description = NULL, *id = NULL, *abi_version = NULL;
	gchar *loader = NULL;
	gchar **authors = NULL;
	gchar **dependencies = NULL;
	guint32 abi_version_uint;
	gboolean loq = FALSE, internal = FALSE;
	const gchar *filename = NULL;

	/* Remove all the children from the authors box. */
	while((widget = gtk_widget_get_first_child(row->authors_box)) != NULL) {
		gtk_box_remove(GTK_BOX(row->authors_box), widget);
	}

	/* Remove all the children from the dependencies box. */
	while((widget = gtk_widget_get_first_child(row->dependencies_box)) !=
		  NULL) {
		gtk_box_remove(GTK_BOX(row->dependencies_box), widget);
	}

	/* Now get the info if we can. */
	if(GPLUGIN_IS_PLUGIN(row->plugin)) {
		GPluginPluginInfo *plugin_info = gplugin_plugin_get_info(row->plugin);
		GPluginLoader *plugin_loader = gplugin_plugin_get_loader(row->plugin);

		filename = gplugin_plugin_get_filename(row->plugin);
		error = gplugin_plugin_get_error(row->plugin);
		state = gplugin_plugin_get_state(row->plugin);

		if(GPLUGIN_IS_LOADER(plugin_loader)) {
			loader = g_strdup(G_OBJECT_TYPE_NAME(plugin_loader));
		}

		/* clang-format off */
		g_object_get(
			G_OBJECT(plugin_info),
			"abi_version", &abi_version_uint,
			"authors", &authors,
			"summary", &summary,
			"description", &description,
			"dependencies", &dependencies,
			"id", &id,
			"internal", &internal,
			"load-on-query", &loq,
			"name", &name,
			"version", &version,
			"website", &website,
			NULL);
		/* clang-format on */

		/* Finagle the website. */
		if(website) {
			gchar *markup = g_markup_printf_escaped(
				"<a href=\"%s\">%s</a>",
				website,
				website);
			g_free(website);
			website = markup;
		}

		/* Finagle the abi_version. */
		abi_version = g_strdup_printf("%08x", abi_version_uint);

		g_clear_object(&plugin_loader);
		g_clear_object(&plugin_info);
	}

	/* Add a default name if unavailable. */
	if(name == NULL) {
		gchar *basename = g_path_get_basename(filename);
		name = g_strdup_printf(_("Unnamed Plugin: %s"), basename);
		g_free(basename);
	}

	/* Set state of enable switch. */
	switch(state) {
		case GPLUGIN_PLUGIN_STATE_QUERIED:
		case GPLUGIN_PLUGIN_STATE_REQUERY:
			gtk_switch_set_state(GTK_SWITCH(row->enable), FALSE);
			gtk_widget_set_sensitive(row->enable, TRUE);
			break;

		case GPLUGIN_PLUGIN_STATE_LOADED:
			gtk_switch_set_state(GTK_SWITCH(row->enable), TRUE);
			gtk_widget_set_sensitive(row->enable, TRUE);
			break;

		case GPLUGIN_PLUGIN_STATE_UNLOAD_FAILED:
			gtk_switch_set_state(GTK_SWITCH(row->enable), TRUE);
			gtk_widget_set_sensitive(row->enable, FALSE);
			break;

		case GPLUGIN_PLUGIN_STATE_ERROR:
		case GPLUGIN_PLUGIN_STATE_LOAD_FAILED:
		case GPLUGIN_PLUGIN_STATE_UNKNOWN:
		default:
			gtk_switch_set_state(GTK_SWITCH(row->enable), FALSE);
			gtk_widget_set_sensitive(row->enable, FALSE);
			break;
	}

	gtk_label_set_text(GTK_LABEL(row->title), name);
	gtk_label_set_text(GTK_LABEL(row->version), version);
	gtk_label_set_markup(GTK_LABEL(row->website), website);
	gtk_label_set_text(GTK_LABEL(row->summary), summary);
	gtk_label_set_text(GTK_LABEL(row->description), description);
	gtk_widget_set_visible(row->description, description != NULL);
	gtk_label_set_text(GTK_LABEL(row->id), id);
	gtk_label_set_text(
		GTK_LABEL(row->error),
		(error != NULL) ? error->message : "(none)");
	gtk_label_set_text(GTK_LABEL(row->filename), filename);
	gtk_label_set_text(GTK_LABEL(row->abi_version), abi_version);
	gtk_label_set_text(
		GTK_LABEL(row->loader),
		(loader != NULL) ? loader : _("Unknown"));
	gtk_label_set_text(GTK_LABEL(row->internal), internal ? _("Yes") : _("No"));
	gtk_label_set_text(GTK_LABEL(row->load_on_query), loq ? _("Yes") : _("No"));
	/* FIXME: Set this correctly when plugins get proper configuration. */
	gtk_widget_set_sensitive(row->config, FALSE);

	/* Set the authors. */
	if(authors) {
		gint i = 0;

		for(i = 0; authors[i]; i++) {
			widget = gtk_label_new(authors[i]);
			gtk_widget_set_halign(widget, GTK_ALIGN_START);
			gtk_widget_set_valign(widget, GTK_ALIGN_START);
			gtk_box_append(GTK_BOX(row->authors_box), widget);
		}
	} else {
		widget = gtk_label_new(_("(none)"));
		gtk_box_append(GTK_BOX(row->authors_box), widget);
	}

	/* Set the dependencies. */
	if(dependencies) {
		gint i = 0;

		for(i = 0; dependencies[i]; i++) {
			widget = gtk_label_new(dependencies[i]);
			gtk_widget_set_halign(widget, GTK_ALIGN_START);
			gtk_widget_set_valign(widget, GTK_ALIGN_START);
			gtk_box_append(GTK_BOX(row->dependencies_box), widget);
		}
	} else {
		widget = gtk_label_new(_("(none)"));
		gtk_box_append(GTK_BOX(row->dependencies_box), widget);
	}

	g_clear_error(&error);
	g_free(loader);
	g_free(abi_version);
	g_strfreev(authors);
	g_free(summary);
	g_free(description);
	g_strfreev(dependencies);
	g_free(id);
	g_free(name);
	g_free(version);
	g_free(website);

	gtk_list_box_row_changed(GTK_LIST_BOX_ROW(row));
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
gplugin_gtk_plugin_row_state_cb(
	G_GNUC_UNUSED GObject *obj,
	G_GNUC_UNUSED GParamSpec *pspec,
	gpointer data)
{
	_gplugin_gtk_plugin_row_refresh(GPLUGIN_GTK_PLUGIN_ROW(data));
}

static gboolean
gplugin_gtk_plugin_row_enable_state_set_cb(
	G_GNUC_UNUSED GtkSwitch *widget,
	gboolean state,
	gpointer data)
{
	GPluginGtkPluginRow *row = GPLUGIN_GTK_PLUGIN_ROW(data);

	g_signal_emit(G_OBJECT(row), signals[SIG_PLUGIN_STATE_SET], 0, state);

	return TRUE;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(
	GPluginGtkPluginRow,
	gplugin_gtk_plugin_row,
	GTK_TYPE_LIST_BOX_ROW)

static void
gplugin_gtk_plugin_row_set_property(
	GObject *obj,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec)
{
	GPluginGtkPluginRow *row = GPLUGIN_GTK_PLUGIN_ROW(obj);

	switch(prop_id) {
		case PROP_PLUGIN:
			gplugin_gtk_plugin_row_set_plugin(row, g_value_get_object(value));
			break;
		case PROP_EXPANDED:
			gplugin_gtk_plugin_row_set_expanded(
				row,
				g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
gplugin_gtk_plugin_row_get_property(
	GObject *obj,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec)
{
	GPluginGtkPluginRow *row = GPLUGIN_GTK_PLUGIN_ROW(obj);

	switch(prop_id) {
		case PROP_PLUGIN:
			g_value_set_object(value, gplugin_gtk_plugin_row_get_plugin(row));
			break;
		case PROP_EXPANDED:
			g_value_set_boolean(
				value,
				gplugin_gtk_plugin_row_get_expanded(row));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void
gplugin_gtk_plugin_row_finalize(GObject *obj)
{
	GPluginGtkPluginRow *row = GPLUGIN_GTK_PLUGIN_ROW(obj);

	if(row->signal_id != 0 && GPLUGIN_IS_PLUGIN(row->plugin)) {
		g_signal_handler_disconnect(G_OBJECT(row->plugin), row->signal_id);
	}

	g_clear_object(&row->plugin);

	G_OBJECT_CLASS(gplugin_gtk_plugin_row_parent_class)->finalize(obj);
}

static void
gplugin_gtk_plugin_row_init(GPluginGtkPluginRow *row)
{
	gtk_widget_init_template(GTK_WIDGET(row));

	row->signal_id = 0;
}

static void
gplugin_gtk_plugin_row_class_init(GPluginGtkPluginRowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = gplugin_gtk_plugin_row_get_property;
	obj_class->set_property = gplugin_gtk_plugin_row_set_property;
	obj_class->finalize = gplugin_gtk_plugin_row_finalize;

	/* properties */
	properties[PROP_PLUGIN] = g_param_spec_object(
		"plugin",
		"plugin",
		"The GPluginPlugin whose info should be displayed",
		G_TYPE_OBJECT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	properties[PROP_EXPANDED] = g_param_spec_boolean(
		"expanded",
		"expanded",
		"Whether the row has been opened to show details",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/* signals */

	/**
	 * GPluginGtkPluginRow::plugin-state-set:
	 * @row: The [class@GPluginGtk.PluginRow] instance.
	 * @enabled: Whether the plugin was requested to be enabled or disabled.
	 *
	 * Emitted when the plugin row enable switch is toggled.
	 *
	 * Since: 0.38.0
	 */
	signals[SIG_PLUGIN_STATE_SET] = g_signal_new_class_handler(
		"plugin-state-set",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		G_TYPE_BOOLEAN);

	/* template stuff */
	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/org/imfreedom/keep/gplugin/gplugin-gtk/plugin-row.ui");

	/* Header */
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		enable);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		title);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		summary);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		version);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		config);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		revealer);
	gtk_widget_class_bind_template_callback(
		widget_class,
		gplugin_gtk_plugin_row_enable_state_set_cb);

	/* Details */
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		description);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		authors_box);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		website);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		dependencies_box);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		error);
	gtk_widget_class_bind_template_child(widget_class, GPluginGtkPluginRow, id);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		filename);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		abi_version);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		loader);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		internal);
	gtk_widget_class_bind_template_child(
		widget_class,
		GPluginGtkPluginRow,
		load_on_query);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * gplugin_gtk_plugin_row_new:
 *
 * Create a new [class@GPluginGtk4.View] which can be used to display info
 * about a [iface@GPlugin.Plugin].
 *
 * Returns: (transfer full): The new widget.
 */
GtkWidget *
gplugin_gtk_plugin_row_new(void)
{
	return g_object_new(GPLUGIN_GTK_TYPE_PLUGIN_ROW, NULL);
}

/**
 * gplugin_gtk_plugin_row_set_plugin:
 * @row: The plugin row instance.
 * @plugin: The plugin instance.
 *
 * Sets the [iface@GPlugin.Plugin] that should be displayed.
 *
 * A @plugin value of %NULL will clear the widget.
 */
void
gplugin_gtk_plugin_row_set_plugin(
	GPluginGtkPluginRow *row,
	GPluginPlugin *plugin)
{
	g_return_if_fail(GPLUGIN_GTK_IS_PLUGIN_ROW(row));

	if(row->signal_id != 0 && GPLUGIN_IS_PLUGIN(row->plugin)) {
		g_signal_handler_disconnect(row->plugin, row->signal_id);
		row->signal_id = 0;
	}

	if(g_set_object(&row->plugin, plugin)) {
		_gplugin_gtk_plugin_row_refresh(row);

		if(GPLUGIN_IS_PLUGIN(plugin)) {
			/* Connect a signal to refresh when the plugin's state changes.  We
			 * can't use g_signal_connect_object because the plugin object never
			 * gets destroyed, as the manager and the loader both keep a
			 * reference to it and the GPluginGtkPluginRow widget is reused for
			 * all plugins so that all means that we just have to manage the
			 * callback ourselves.
			 */
			row->signal_id = g_signal_connect(
				G_OBJECT(plugin),
				"notify::state",
				G_CALLBACK(gplugin_gtk_plugin_row_state_cb),
				row);
		}
	}
}

/**
 * gplugin_gtk_plugin_row_get_plugin:
 * @row: The plugin row instance.
 *
 * Returns the [iface@GPlugin.Plugin] that's being displayed.
 *
 * Returns: (transfer none): The plugin that's being displayed.
 */
GPluginPlugin *
gplugin_gtk_plugin_row_get_plugin(GPluginGtkPluginRow *row)
{
	g_return_val_if_fail(GPLUGIN_GTK_IS_PLUGIN_ROW(row), NULL);

	return row->plugin;
}

/**
 * gplugin_gtk_plugin_row_get_expanded:
 * @row: The plugin row instance.
 *
 * Returns whether the plugin row is showing detailed information.
 *
 * Returns: %TRUE if the plugin row is expanded, %FALSE otherwise.
 */
gboolean
gplugin_gtk_plugin_row_get_expanded(GPluginGtkPluginRow *row)
{
	g_return_val_if_fail(GPLUGIN_GTK_IS_PLUGIN_ROW(row), FALSE);

	return gtk_revealer_get_reveal_child(GTK_REVEALER(row->revealer));
}

/**
 * gplugin_gtk_plugin_row_set_expanded:
 * @row: The plugin row instance.
 * @expanded: Whether the plugin row is expanded.
 *
 * Sets whether the plugin row is showing detailed information.
 */
void
gplugin_gtk_plugin_row_set_expanded(GPluginGtkPluginRow *row, gboolean expanded)
{
	g_return_if_fail(GPLUGIN_GTK_IS_PLUGIN_ROW(row));

	gtk_revealer_set_reveal_child(GTK_REVEALER(row->revealer), expanded);
}

/**
 * gplugin_gtk_plugin_row_get_sort_key:
 * @row: The plugin row instance.
 *
 * Returns a key that can be used to sort this row.
 *
 * Returns: The sort key.
 */
gchar *
gplugin_gtk_plugin_row_get_sort_key(GPluginGtkPluginRow *row)
{
	g_return_val_if_fail(GPLUGIN_GTK_IS_PLUGIN_ROW(row), NULL);

	return g_strdup(gtk_label_get_text(GTK_LABEL(row->title)));
}

/**
 * gplugin_gtk_plugin_row_matches_search:
 * @row: The plugin row instance.
 * @text: The text to search for.
 *
 * Matches this row instance against some text to be searched for.
 *
 * Returns: Whether the row matches the text or not.
 */
gboolean
gplugin_gtk_plugin_row_matches_search(
	GPluginGtkPluginRow *row,
	const gchar *text)
{
	const gchar *value = NULL;

	g_return_val_if_fail(GPLUGIN_GTK_IS_PLUGIN_ROW(row), FALSE);

	value = gtk_label_get_text(GTK_LABEL(row->title));
	if(g_strstr_len(value, -1, text)) {
		return TRUE;
	}

	value = gtk_label_get_text(GTK_LABEL(row->summary));
	if(g_strstr_len(value, -1, text)) {
		return TRUE;
	}

	value = gtk_label_get_text(GTK_LABEL(row->description));
	if(g_strstr_len(value, -1, text)) {
		return TRUE;
	}

	value = gtk_label_get_text(GTK_LABEL(row->filename));
	if(g_strstr_len(value, -1, text)) {
		return TRUE;
	}

	return FALSE;
}
