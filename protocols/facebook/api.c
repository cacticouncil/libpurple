/* purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
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
 */

#include <glib/gi18n-lib.h>

#include <json-glib/json-glib.h>
#include <libsoup/soup.h>
#include <stdarg.h>
#include <string.h>

#include "libpurple/glibcompat.h"
#include "libpurple/soupcompat.h"

#include "api.h"
#include "http.h"
#include "json.h"
#include "thrift.h"
#include "util.h"

enum
{
	PROP_0,

	PROP_CID,
	PROP_DID,
	PROP_MID,
	PROP_STOKEN,
	PROP_TOKEN,
	PROP_UID,

	PROP_N
};

/**
 * FbApi:
 *
 * Represents a Facebook Messenger connection.
 */
struct _FbApi {
	GObject parent;

	FbMqtt *mqtt;
	SoupSession *cons;
	PurpleConnection *gc;
	gboolean retrying;

	FbId uid;
	gint64 sid;
	guint64 mid;
	gchar *cid;
	gchar *did;
	gchar *stoken;
	gchar *token;

	GQueue *msgs;
	gboolean invisible;
	guint unread;
	FbId lastmid;
	gchar *contacts_delta;
};

static void fb_api_error_literal(FbApi *api, FbApiError error,
                                 const gchar *msg);

static void
fb_api_attach(FbApi *api, FbId aid, const gchar *msgid, FbApiMessage *msg);

static void
fb_api_contacts_after(FbApi *api, const gchar *cursor);

static void
fb_api_message_send(FbApi *api, FbApiMessage *msg);

static void
fb_api_sticker(FbApi *api, FbId sid, FbApiMessage *msg);

void
fb_api_contacts_delta(FbApi *api, const gchar *delta_cursor);

G_DEFINE_TYPE(FbApi, fb_api, G_TYPE_OBJECT);

static void
fb_api_set_property(GObject *obj, guint prop, const GValue *val,
                    GParamSpec *pspec)
{
	FbApi *api = FB_API(obj);

	switch (prop) {
	case PROP_CID:
		g_free(api->cid);
		api->cid = g_value_dup_string(val);
		break;
	case PROP_DID:
		g_free(api->did);
		api->did = g_value_dup_string(val);
		break;
	case PROP_MID:
		api->mid = g_value_get_uint64(val);
		break;
	case PROP_STOKEN:
		g_free(api->stoken);
		api->stoken = g_value_dup_string(val);
		break;
	case PROP_TOKEN:
		g_free(api->token);
		api->token = g_value_dup_string(val);
		break;
	case PROP_UID:
		api->uid = g_value_get_int64(val);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop, pspec);
		break;
	}
}

static void
fb_api_get_property(GObject *obj, guint prop, GValue *val, GParamSpec *pspec)
{
	FbApi *api = FB_API(obj);

	switch (prop) {
	case PROP_CID:
		g_value_set_string(val, api->cid);
		break;
	case PROP_DID:
		g_value_set_string(val, api->did);
		break;
	case PROP_MID:
		g_value_set_uint64(val, api->mid);
		break;
	case PROP_STOKEN:
		g_value_set_string(val, api->stoken);
		break;
	case PROP_TOKEN:
		g_value_set_string(val, api->token);
		break;
	case PROP_UID:
		g_value_set_int64(val, api->uid);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop, pspec);
		break;
	}
}


static void
fb_api_dispose(GObject *obj)
{
	FbApi *api = FB_API(obj);

	if(api->cons != NULL) {
		soup_session_abort(api->cons);
	}

	g_clear_object(&api->mqtt);

	g_clear_object(&api->cons);
	if(api->msgs != NULL) {
		g_queue_free_full(api->msgs, (GDestroyNotify)fb_api_message_free);
		api->msgs = NULL;
	}

	g_clear_pointer(&api->cid, g_free);
	g_clear_pointer(&api->did, g_free);
	g_clear_pointer(&api->stoken, g_free);
	g_clear_pointer(&api->token, g_free);
	g_clear_pointer(&api->contacts_delta, g_free);
}

static void
fb_api_class_init(FbApiClass *klass)
{
	GObjectClass *gklass = G_OBJECT_CLASS(klass);
	GParamSpec *props[PROP_N] = {NULL};

	gklass->set_property = fb_api_set_property;
	gklass->get_property = fb_api_get_property;
	gklass->dispose = fb_api_dispose;

	/**
	 * FbApi:cid:
	 *
	 * The client identifier for MQTT. This value should be saved
	 * and loaded for persistence.
	 */
	props[PROP_CID] = g_param_spec_string(
		"cid",
		"Client ID",
		"Client identifier for MQTT",
		NULL,
		G_PARAM_READWRITE);

	/**
	 * FbApi:did:
	 *
	 * The device identifier for the MQTT message queue. This value
	 * should be saved and loaded for persistence.
	 */
	props[PROP_DID] = g_param_spec_string(
		"did",
		"Device ID",
		"Device identifier for the MQTT message queue",
		NULL,
		G_PARAM_READWRITE);

	/**
	 * FbApi:mid:
	 *
	 * The MQTT identifier. This value should be saved and loaded
	 * for persistence.
	 */
	props[PROP_MID] = g_param_spec_uint64(
		"mid",
		"MQTT ID",
		"MQTT identifier",
		0, G_MAXUINT64, 0,
		G_PARAM_READWRITE);

	/**
	 * FbApi:stoken:
	 *
	 * The synchronization token for the MQTT message queue. This
	 * value should be saved and loaded for persistence.
	 */
	props[PROP_STOKEN] = g_param_spec_string(
		"stoken",
		"Sync Token",
		"Synchronization token for the MQTT message queue",
		NULL,
		G_PARAM_READWRITE);

	/**
	 * FbApi:token:
	 *
	 * The access token for authentication. This value should be
	 * saved and loaded for persistence.
	 */
	props[PROP_TOKEN] = g_param_spec_string(
		"token",
		"Access Token",
		"Access token for authentication",
		NULL,
		G_PARAM_READWRITE);

	/**
	 * FbApi:uid:
	 *
	 * The #FbId of the user of the #FbApi.
	 */
	props[PROP_UID] = g_param_spec_int64(
		"uid",
		"User ID",
		"User identifier",
		0, G_MAXINT64, 0,
		G_PARAM_READWRITE);
	g_object_class_install_properties(gklass, PROP_N, props);

	/**
	 * FbApi::auth:
	 * @api: The #FbApi.
	 *
	 * Emitted upon the successful completion of the authentication
	 * process. This is emitted as a result of #fb_api_auth().
	 */
	g_signal_new("auth",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             0);

	/**
	 * FbApi::connect:
	 * @api: The #FbApi.
	 *
	 * Emitted upon the successful completion of the connection
	 * process. This is emitted as a result of #fb_api_connect().
	 */
	g_signal_new("connect",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             0);

	/**
	 * FbApi::contact:
	 * @api: The #FbApi.
	 * @user: The #FbApiUser.
	 *
	 * Emitted upon the successful reply of a contact request. This
	 * is emitted as a result of #fb_api_contact().
	 */
	g_signal_new("contact",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             1, G_TYPE_POINTER);

	/**
	 * FbApi::contacts:
	 * @api: The #FbApi.
	 * @users: The #GSList of #FbApiUser's.
	 * @complete: #TRUE if the list is fetched, otherwise #FALSE.
	 *
	 * Emitted upon the successful reply of a contacts request.
	 * This is emitted as a result of #fb_api_contacts(). This can
	 * be emitted multiple times before the entire contacts list
	 * has been fetched. Use @complete for detecting the completion
	 * status of the list fetch.
	 */
	g_signal_new("contacts",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             2, G_TYPE_POINTER, G_TYPE_BOOLEAN);

	/**
	 * FbApi::contacts-delta:
	 * @api: The #FbApi.
	 * @added: The #GSList of added #FbApiUser's.
	 * @removed: The #GSList of strings with removed user ids.
	 *
	 * Like 'contacts', but only the deltas.
	 */
	g_signal_new("contacts-delta",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             2, G_TYPE_POINTER, G_TYPE_POINTER);

	/**
	 * FbApi::error:
	 * @api: The #FbApi.
	 * @error: The #GError.
	 *
	 * Emitted whenever an error is hit within the #FbApi. This
	 * should disconnect the #FbApi with #fb_api_disconnect().
	 */
	g_signal_new("error",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             1, G_TYPE_POINTER);

	/**
	 * FbApi::events:
	 * @api: The #FbApi.
	 * @events: The #GSList of #FbApiEvent's.
	 *
	 * Emitted upon incoming events from the stream.
	 */
	g_signal_new("events",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             1, G_TYPE_POINTER);

	/**
	 * FbApi::messages:
	 * @api: The #FbApi.
	 * @msgs: The #GSList of #FbApiMessage's.
	 *
	 * Emitted upon incoming messages from the stream.
	 */
	g_signal_new("messages",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             1, G_TYPE_POINTER);

	/**
	 * FbApi::presences:
	 * @api: The #FbApi.
	 * @press: The #GSList of #FbApiPresence's.
	 *
	 * Emitted upon incoming presences from the stream.
	 */
	g_signal_new("presences",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             1, G_TYPE_POINTER);

	/**
	 * FbApi::thread:
	 * @api: The #FbApi.
	 * @thrd: The #FbApiThread.
	 *
	 * Emitted upon the successful reply of a thread request. This
	 * is emitted as a result of #fb_api_thread().
	 */
	g_signal_new("thread",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             1, G_TYPE_POINTER);

	/**
	 * FbApi::thread-create:
	 * @api: The #FbApi.
	 * @tid: The thread #FbId.
	 *
	 * Emitted upon the successful reply of a thread creation
	 * request. This is emitted as a result of
	 * #fb_api_thread_create().
	 */
	g_signal_new("thread-create",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             1, FB_TYPE_ID);

	/**
	 * FbApi::thread-kicked:
	 * @api: The #FbApi.
	 * @thrd: The #FbApiThread.
	 *
	 * Emitted upon the reply of a thread request when the user is no longer
	 * part of that thread. This is emitted as a result of #fb_api_thread().
	 */
	g_signal_new("thread-kicked",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             1, G_TYPE_POINTER);

	/**
	 * FbApi::threads:
	 * @api: The #FbApi.
	 * @thrds: The #GSList of #FbApiThread's.
	 *
	 * Emitted upon the successful reply of a threads request. This
	 * is emitted as a result of #fb_api_threads().
	 */
	g_signal_new("threads",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             1, G_TYPE_POINTER);

	/**
	 * FbApi::typing:
	 * @api: The #FbApi.
	 * @typg: The #FbApiTyping.
	 *
	 * Emitted upon an incoming typing state from the stream.
	 */
	g_signal_new("typing",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             1, G_TYPE_POINTER);
}

static void
fb_api_init(FbApi *api)
{
	api->msgs = g_queue_new();
}

GQuark
fb_api_error_quark(void)
{
	static GQuark q = 0;

	if (G_UNLIKELY(q == 0)) {
		q = g_quark_from_static_string("fb-api-error-quark");
	}

	return q;
}

static gboolean
fb_api_json_chk(FbApi *api, gconstpointer data, gssize size, JsonNode **node)
{
	const gchar *str;
	FbApiError errc = FB_API_ERROR_GENERAL;
	FbJsonValues *values;
	gboolean success = TRUE;
	gchar *msg;
	GError *err = NULL;
	gint64 code;
	guint i;
	JsonNode *root;

	static const gchar *exprs[] = {
		"$.error.message",
		"$.error.summary",
		"$.error_msg",
		"$.errorCode",
		"$.failedSend.errorMessage",
	};

	g_return_val_if_fail(FB_IS_API(api), FALSE);

	if (G_UNLIKELY(size == 0)) {
		fb_api_error_literal(api, FB_API_ERROR_GENERAL, _("Empty JSON data"));
		return FALSE;
	}

	fb_util_debug(FB_UTIL_DEBUG_INFO, "Parsing JSON: %.*s\n",
	              (gint) size, (const gchar *) data);

	root = fb_json_node_new(data, size, &err);
	FB_API_ERROR_EMIT(api, err, return FALSE);

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_INT, FALSE, "$.error_code");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE, "$.error.type");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE, "$.errorCode");
	fb_json_values_update(values, &err);

	FB_API_ERROR_EMIT(api, err,
		g_object_unref(values);
		json_node_free(root);
		return FALSE
	);

	code = fb_json_values_next_int(values, 0);
	str = fb_json_values_next_str(values, NULL);

	if (purple_strequal(str, "OAuthException") || (code == 401)) {
		errc = FB_API_ERROR_AUTH;
		success = FALSE;

		g_clear_pointer(&api->stoken, g_free);
		g_clear_pointer(&api->token, g_free);
	}

	/* 509 is used for "invalid attachment id" */
	if (code == 509) {
		errc = FB_API_ERROR_NONFATAL;
		success = FALSE;
	}

	str = fb_json_values_next_str(values, NULL);

	if (purple_strequal(str, "ERROR_QUEUE_NOT_FOUND") ||
	    purple_strequal(str, "ERROR_QUEUE_LOST"))
	{
		errc = FB_API_ERROR_QUEUE;
		success = FALSE;

		g_clear_pointer(&api->stoken, g_free);
	}

	g_object_unref(values);

	for (msg = NULL, i = 0; i < G_N_ELEMENTS(exprs); i++) {
		msg = fb_json_node_get_str(root, exprs[i], NULL);

		if (msg != NULL) {
			success = FALSE;
			break;
		}
	}

	if (!success && (msg == NULL)) {
		msg = g_strdup(_("Unknown error"));
	}

	if (msg != NULL) {
		fb_api_error_literal(api, errc, msg);
		json_node_free(root);
		g_free(msg);
		return FALSE;
	}

	if (node != NULL) {
		*node = root;
	} else {
		json_node_free(root);
	}

	return TRUE;
}

static gboolean
fb_api_http_chk(FbApi *api, SoupSession *session, GAsyncResult *result,
                SoupMessage *msg, JsonNode **root)
{
	GBytes *response_body = NULL;
	const gchar *reason = NULL;
	const gchar *data = NULL;
	GError *err = NULL;
	gint code;
	gsize size = 0;

	reason = soup_message_get_reason_phrase(msg);
	code = soup_message_get_status(msg);

	fb_util_debug(FB_UTIL_DEBUG_INFO, "HTTP Response (%p):", msg);
	if (reason != NULL) {
		fb_util_debug(FB_UTIL_DEBUG_INFO, "  Response Error: %s (%d)", reason,
		              code);
	} else {
		fb_util_debug(FB_UTIL_DEBUG_INFO, "  Response Error: %d", code);
	}

	if (fb_http_error_chk(msg, &err) && (root == NULL)) {
		return TRUE;
	}

	response_body = soup_session_send_and_read_finish(session, result, &err);
	if(response_body != NULL) {
		data = g_bytes_get_data(response_body, &size);
	}

	if (G_LIKELY(size > 0)) {
		fb_util_debug(FB_UTIL_DEBUG_INFO, "  Response Data: %.*s",
		              (gint) size, data);
	}

	/* Rudimentary check to prevent wrongful error parsing */
	if ((size < 2) || (data[0] != '{') || (data[size - 1] != '}')) {
		FB_API_ERROR_EMIT(api, err, return FALSE);
	}

	if (!fb_api_json_chk(api, data, size, root)) {
		if (G_UNLIKELY(err != NULL)) {
			g_error_free(err);
		}

		return FALSE;
	}

	FB_API_ERROR_EMIT(api, err, return FALSE);
	return TRUE;
}

static SoupMessage *
fb_api_http_req(FbApi *api, const gchar *url, const gchar *name,
                const gchar *method, FbHttpParams *params,
                GAsyncReadyCallback callback)
{
	gchar *data;
	gchar *key;
	gchar *val;
	GList *keys;
	GList *l;
	GString *gstr;
	SoupMessage *msg;

	fb_http_params_set_str(params, "api_key", FB_API_KEY);
	fb_http_params_set_str(params, "device_id", api->did);
	fb_http_params_set_str(params, "fb_api_req_friendly_name", name);
	fb_http_params_set_str(params, "format", "json");
	fb_http_params_set_str(params, "method", method);

	val = fb_util_get_locale();
	fb_http_params_set_str(params, "locale", val);
	g_free(val);

	/* Ensure an old signature is not computed */
	g_hash_table_remove(params, "sig");

	gstr = g_string_new(NULL);
	keys = g_hash_table_get_keys(params);
	keys = g_list_sort(keys, (GCompareFunc) g_ascii_strcasecmp);

	for (l = keys; l != NULL; l = l->next) {
		key = l->data;
		val = g_hash_table_lookup(params, key);
		g_string_append_printf(gstr, "%s=%s", key, val);
	}

	g_string_append(gstr, FB_API_SECRET);
	data = g_compute_checksum_for_string(G_CHECKSUM_MD5, gstr->str,
	                                     gstr->len);
	fb_http_params_set_str(params, "sig", data);
	g_string_free(gstr, TRUE);
	g_list_free(keys);
	g_free(data);

	msg = soup_message_new_from_encoded_form("POST", url, soup_form_encode_hash(params));
	fb_http_params_free(params);

	if (api->token != NULL) {
		data = g_strdup_printf("OAuth %s", api->token);
		soup_message_headers_replace(soup_message_get_request_headers(msg),
		                             "Authorization", data);
		g_free(data);
	}

	g_object_set_data(G_OBJECT(msg), "facebook-api", api);
	soup_session_send_and_read_async(api->cons, msg, G_PRIORITY_DEFAULT, NULL,
	                                 callback, msg);

	fb_util_debug(FB_UTIL_DEBUG_INFO, "HTTP Request (%p):", msg);
	fb_util_debug(FB_UTIL_DEBUG_INFO, "  Request URL: %s", url);

	return msg;
}

static SoupMessage *
fb_api_http_query(FbApi *api, gint64 query, JsonBuilder *builder,
                  GAsyncReadyCallback hcb)
{
	const gchar *name;
	FbHttpParams *prms;
	gchar *json;

	switch (query) {
	case FB_API_QUERY_CONTACT:
		name = "UsersQuery";
		break;
	case FB_API_QUERY_CONTACTS:
		name = "FetchContactsFullQuery";
		break;
	case FB_API_QUERY_CONTACTS_AFTER:
		name = "FetchContactsFullWithAfterQuery";
		break;
	case FB_API_QUERY_CONTACTS_DELTA:
		name = "FetchContactsDeltaQuery";
		break;
	case FB_API_QUERY_STICKER:
		name = "FetchStickersWithPreviewsQuery";
		break;
	case FB_API_QUERY_THREAD:
		name = "ThreadQuery";
		break;
	case FB_API_QUERY_SEQ_ID:
	case FB_API_QUERY_THREADS:
		name = "ThreadListQuery";
		break;
	case FB_API_QUERY_XMA:
		name = "XMAQuery";
		break;
	default:
		g_return_val_if_reached(NULL);
		return NULL;
	}

	prms = fb_http_params_new();
	json = fb_json_bldr_close(builder, JSON_NODE_OBJECT, NULL);

	fb_http_params_set_strf(prms, "query_id", "%" G_GINT64_FORMAT, query);
	fb_http_params_set_str(prms, "query_params", json);
	g_free(json);

	return fb_api_http_req(api, FB_API_URL_GQL, name, "get", prms, hcb);
}

static void
fb_api_cb_http_bool(GObject *source, GAsyncResult *result, gpointer data) {
	SoupSession *session = SOUP_SESSION(source);
	SoupMessage *soupmsg = data;
	FbApi *api = g_object_get_data(G_OBJECT(soupmsg), "facebook-api");
	JsonNode *root;

	if (!fb_api_http_chk(api, session, result, soupmsg, &root)) {
		return;
	}

	if (!json_node_get_boolean(root)) {
		fb_api_error_literal(api, FB_API_ERROR,
		                     _("Failed generic API operation"));
	}

	json_node_free(root);
}

static void
fb_api_cb_mqtt_error(FbMqtt *mqtt, GError *error, gpointer data)
{
	FbApi *api = data;

	if (!api->retrying) {
		api->retrying = TRUE;
		fb_util_debug_info("Attempting to reconnect the MQTT stream...");
		fb_api_connect(api, api->invisible);
	} else {
		g_signal_emit_by_name(api, "error", error);
	}
}

static void
fb_api_cb_mqtt_open(FbMqtt *mqtt, gpointer data)
{
	const GByteArray *bytes;
	FbApi *api = data;
	FbThrift *thft;
	GByteArray *cytes;
	GError *err = NULL;

	static guint8 flags = FB_MQTT_CONNECT_FLAG_USER |
	                      FB_MQTT_CONNECT_FLAG_PASS |
	                      FB_MQTT_CONNECT_FLAG_CLR;

	thft = fb_thrift_new(NULL, 0);

	/* Write the client identifier */
	fb_thrift_write_field(thft, FB_THRIFT_TYPE_STRING, 1, 0);
	fb_thrift_write_str(thft, api->cid);

	fb_thrift_write_field(thft, FB_THRIFT_TYPE_STRUCT, 4, 1);

	/* Write the user identifier */
	fb_thrift_write_field(thft, FB_THRIFT_TYPE_I64, 1, 0);
	fb_thrift_write_i64(thft, api->uid);

	/* Write the information string */
	fb_thrift_write_field(thft, FB_THRIFT_TYPE_STRING, 2, 1);
	fb_thrift_write_str(thft, FB_API_MQTT_AGENT);

	/* Write the UNKNOWN ("cp"?) */
	fb_thrift_write_field(thft, FB_THRIFT_TYPE_I64, 3, 2);
	fb_thrift_write_i64(thft, 23);

	/* Write the UNKNOWN ("ecp"?) */
	fb_thrift_write_field(thft, FB_THRIFT_TYPE_I64, 4, 3);
	fb_thrift_write_i64(thft, 26);

	/* Write the UNKNOWN */
	fb_thrift_write_field(thft, FB_THRIFT_TYPE_I32, 5, 4);
	fb_thrift_write_i32(thft, 1);

	/* Write the UNKNOWN ("no_auto_fg"?) */
	fb_thrift_write_field(thft, FB_THRIFT_TYPE_BOOL, 6, 5);
	fb_thrift_write_bool(thft, TRUE);

	/* Write the visibility state */
	fb_thrift_write_field(thft, FB_THRIFT_TYPE_BOOL, 7, 6);
	fb_thrift_write_bool(thft, !api->invisible);

	/* Write the device identifier */
	fb_thrift_write_field(thft, FB_THRIFT_TYPE_STRING, 8, 7);
	fb_thrift_write_str(thft, api->did);

	/* Write the UNKNOWN ("fg"?) */
	fb_thrift_write_field(thft, FB_THRIFT_TYPE_BOOL, 9, 8);
	fb_thrift_write_bool(thft, TRUE);

	/* Write the UNKNOWN ("nwt"?) */
	fb_thrift_write_field(thft, FB_THRIFT_TYPE_I32, 10, 9);
	fb_thrift_write_i32(thft, 1);

	/* Write the UNKNOWN ("nwst"?) */
	fb_thrift_write_field(thft, FB_THRIFT_TYPE_I32, 11, 10);
	fb_thrift_write_i32(thft, 0);

	/* Write the MQTT identifier */
	fb_thrift_write_field(thft, FB_THRIFT_TYPE_I64, 12, 11);
	fb_thrift_write_i64(thft, api->mid);

	/* Write the UNKNOWN */
	fb_thrift_write_field(thft, FB_THRIFT_TYPE_LIST, 14, 12);
	fb_thrift_write_list(thft, FB_THRIFT_TYPE_I32, 0);
	fb_thrift_write_stop(thft);

	/* Write the token */
	fb_thrift_write_field(thft, FB_THRIFT_TYPE_STRING, 15, 14);
	fb_thrift_write_str(thft, api->token);

	/* Write the STOP for the struct */
	fb_thrift_write_stop(thft);

	bytes = fb_thrift_get_bytes(thft);
	cytes = fb_util_zlib_deflate(bytes, &err);

	FB_API_ERROR_EMIT(api, err,
		g_object_unref(thft);
		return;
	);

	fb_util_debug_hexdump(FB_UTIL_DEBUG_INFO, bytes, "Writing connect");
	fb_mqtt_connect(mqtt, flags, cytes);

	g_byte_array_free(cytes, TRUE);
	g_object_unref(thft);
}

static void
fb_api_connect_queue(FbApi *api)
{
	FbApiMessage *msg;
	gchar *json;
	JsonBuilder *bldr;

	bldr = fb_json_bldr_new(JSON_NODE_OBJECT);
	fb_json_bldr_add_int(bldr, "delta_batch_size", 125);
	fb_json_bldr_add_int(bldr, "max_deltas_able_to_process", 1250);
	fb_json_bldr_add_int(bldr, "sync_api_version", 3);
	fb_json_bldr_add_str(bldr, "encoding", "JSON");

	if (api->stoken == NULL) {
		fb_json_bldr_add_int(bldr, "initial_titan_sequence_id", api->sid);
		fb_json_bldr_add_str(bldr, "device_id", api->did);
		fb_json_bldr_add_int(bldr, "entity_fbid", api->uid);

		fb_json_bldr_obj_begin(bldr, "queue_params");
		fb_json_bldr_add_str(bldr, "buzz_on_deltas_enabled", "false");

		fb_json_bldr_obj_begin(bldr, "graphql_query_hashes");
		fb_json_bldr_add_str(bldr, "xma_query_id",
		                     G_STRINGIFY(FB_API_QUERY_XMA));
		fb_json_bldr_obj_end(bldr);

		fb_json_bldr_obj_begin(bldr, "graphql_query_params");
		fb_json_bldr_obj_begin(bldr, G_STRINGIFY(FB_API_QUERY_XMA));
		fb_json_bldr_add_str(bldr, "xma_id", "<ID>");
		fb_json_bldr_obj_end(bldr);
		fb_json_bldr_obj_end(bldr);
		fb_json_bldr_obj_end(bldr);

		json = fb_json_bldr_close(bldr, JSON_NODE_OBJECT, NULL);
		fb_api_publish(api, "/messenger_sync_create_queue", "%s",
		               json);
		g_free(json);
		return;
	}

	fb_json_bldr_add_int(bldr, "last_seq_id", api->sid);
	fb_json_bldr_add_str(bldr, "sync_token", api->stoken);

	json = fb_json_bldr_close(bldr, JSON_NODE_OBJECT, NULL);
	fb_api_publish(api, "/messenger_sync_get_diffs", "%s", json);
	g_signal_emit_by_name(api, "connect");
	g_free(json);

	if (!g_queue_is_empty(api->msgs)) {
		msg = g_queue_peek_head(api->msgs);
		fb_api_message_send(api, msg);
	}

	if (api->retrying) {
		api->retrying = FALSE;
		fb_util_debug_info("Reconnected the MQTT stream");
	}
}

static void
fb_api_cb_seqid(GObject *source, GAsyncResult *result, gpointer data) {
	SoupSession *session = SOUP_SESSION(source);
	SoupMessage *soupmsg = data;
	FbApi *api = g_object_get_data(G_OBJECT(soupmsg), "facebook-api");
	const gchar *str;
	FbJsonValues *values;
	GError *err = NULL;
	JsonNode *root;

	if (!fb_api_http_chk(api, session, result, soupmsg, &root)) {
		return;
	}

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.viewer.message_threads.sync_sequence_id");
	fb_json_values_add(values, FB_JSON_TYPE_INT, TRUE,
	                   "$.viewer.message_threads.unread_count");
	fb_json_values_update(values, &err);

	FB_API_ERROR_EMIT(api, err,
		g_object_unref(values);
		json_node_free(root);
		return;
	);

	str = fb_json_values_next_str(values, "0");
	api->sid = g_ascii_strtoll(str, NULL, 10);
	api->unread = fb_json_values_next_int(values, 0);

	if (api->sid == 0) {
		fb_api_error_literal(api, FB_API_ERROR_GENERAL,
		                     _("Failed to get sync_sequence_id"));
	} else {
		fb_api_connect_queue(api);
	}

	g_object_unref(values);
	json_node_free(root);
}

static void
fb_api_cb_mqtt_connect(FbMqtt *mqtt, gpointer data)
{
	FbApi *api = data;
	gchar *json;
	JsonBuilder *bldr;

	bldr = fb_json_bldr_new(JSON_NODE_OBJECT);
	fb_json_bldr_add_bool(bldr, "foreground", TRUE);
	fb_json_bldr_add_int(bldr, "keepalive_timeout", FB_MQTT_KA);

	json = fb_json_bldr_close(bldr, JSON_NODE_OBJECT, NULL);
	fb_api_publish(api, "/foreground_state", "%s", json);
	g_free(json);

	fb_mqtt_subscribe(mqtt,
		"/inbox", 0,
		"/mercury", 0,
		"/messaging_events", 0,
		"/orca_presence", 0,
		"/orca_typing_notifications", 0,
		"/pp", 0,
		"/t_ms", 0,
		"/t_p", 0,
		"/t_rtc", 0,
		"/webrtc", 0,
		"/webrtc_response", 0,
		NULL
	);

	/* Notifications seem to lead to some sort of sending rate limit */
	fb_mqtt_unsubscribe(mqtt, "/orca_message_notifications", NULL);

	if (api->sid == 0) {
		bldr = fb_json_bldr_new(JSON_NODE_OBJECT);
		fb_json_bldr_add_str(bldr, "1", "0");
		fb_api_http_query(api, FB_API_QUERY_SEQ_ID, bldr,
		                  fb_api_cb_seqid);
	} else {
		fb_api_connect_queue(api);
	}
}

static void
fb_api_cb_publish_mark(FbApi *api, GByteArray *pload)
{
	FbJsonValues *values;
	GError *err = NULL;
	JsonNode *root;

	if (!fb_api_json_chk(api, pload->data, pload->len, &root)) {
		return;
	}

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_BOOL, FALSE, "$.succeeded");
	fb_json_values_update(values, &err);

	FB_API_ERROR_EMIT(api, err,
		g_object_unref(values);
		json_node_free(root);
		return;
	);

	if (!fb_json_values_next_bool(values, TRUE)) {
		fb_api_error_literal(api, FB_API_ERROR_GENERAL,
		                     _("Failed to mark thread as read"));
	}

	g_object_unref(values);
	json_node_free(root);
}

static GSList *
fb_api_event_parse(FbApi *api, FbApiEvent *event, GSList *events,
                   JsonNode *root, GError **error)
{
	const gchar *str;
	FbApiEvent *devent;
	FbJsonValues *values;
	GError *err = NULL;
	guint i;

	static const struct {
		FbApiEventType type;
		const gchar *expr;
	} evtypes[] = {
		{
			FB_API_EVENT_TYPE_THREAD_USER_ADDED,
			"$.log_message_data.added_participants"
		}, {
			FB_API_EVENT_TYPE_THREAD_USER_REMOVED,
			"$.log_message_data.removed_participants"
		}
	};

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.log_message_type");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE, "$.author");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.log_message_data.name");
	fb_json_values_update(values, &err);

	if (G_UNLIKELY(err != NULL)) {
		g_propagate_error(error, err);
		g_object_unref(values);
		return events;
	}

	str = fb_json_values_next_str(values, NULL);

	if (g_strcmp0(str, "log:thread-name") == 0) {
		str = fb_json_values_next_str(values, "");
		str = strrchr(str, ':');

		if (str != NULL) {
			devent = g_new(FbApiEvent, 1);
			devent->type = FB_API_EVENT_TYPE_THREAD_TOPIC;
			devent->uid = FB_ID_FROM_STR(str + 1);
			devent->tid = event->tid;
			devent->text = fb_json_values_next_str_dup(values, NULL);
			events = g_slist_prepend(events, devent);
		}
	}

	g_object_unref(values);

	for (i = 0; i < G_N_ELEMENTS(evtypes); i++) {
		values = fb_json_values_new(root);
		fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE, "$");
		fb_json_values_set_array(values, FALSE, evtypes[i].expr);

		while (fb_json_values_update(values, &err)) {
			str = fb_json_values_next_str(values, "");
			str = strrchr(str, ':');

			if (str != NULL) {
				devent = g_new0(FbApiEvent, 1);
				devent->type = evtypes[i].type;
				devent->uid = FB_ID_FROM_STR(str + 1);
				devent->tid = event->tid;
				events = g_slist_prepend(events, devent);
			}
		}

		g_object_unref(values);

		if (G_UNLIKELY(err != NULL)) {
			g_propagate_error(error, err);
			break;
		}
	}

	return events;
}

static void
fb_api_cb_publish_mercury(FbApi *api, GByteArray *pload)
{
	const gchar *str;
	FbApiEvent event;
	FbJsonValues *values;
	GError *err = NULL;
	GSList *events = NULL;
	JsonNode *root;
	JsonNode *node;

	if (!fb_api_json_chk(api, pload->data, pload->len, &root)) {
		return;
	}

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE, "$.thread_fbid");
	fb_json_values_set_array(values, FALSE, "$.actions");

	while (fb_json_values_update(values, &err)) {
		fb_api_event_reset(&event, FALSE);
		str = fb_json_values_next_str(values, "0");
		event.tid = FB_ID_FROM_STR(str);

		node = fb_json_values_get_root(values);
		events = fb_api_event_parse(api, &event, events, node, &err);
	}

	if (G_LIKELY(err == NULL)) {
		events = g_slist_reverse(events);
		g_signal_emit_by_name(api, "events", events);
	} else {
		fb_api_error_emit(api, err);
	}

	g_slist_free_full(events, (GDestroyNotify) fb_api_event_free);
	g_object_unref(values);
	json_node_free(root);

}

static void
fb_api_cb_publish_typing(FbApi *api, GByteArray *pload)
{
	const gchar *str;
	FbApiTyping typg;
	FbJsonValues *values;
	GError *err = NULL;
	JsonNode *root;

	if (!fb_api_json_chk(api, pload->data, pload->len, &root)) {
		return;
	}

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE, "$.type");
	fb_json_values_add(values, FB_JSON_TYPE_INT, TRUE, "$.sender_fbid");
	fb_json_values_add(values, FB_JSON_TYPE_INT, TRUE, "$.state");
	fb_json_values_update(values, &err);

	FB_API_ERROR_EMIT(api, err,
		g_object_unref(values);
		json_node_free(root);
		return;
	);

	str = fb_json_values_next_str(values, NULL);

	if (g_ascii_strcasecmp(str, "typ") == 0) {
		typg.uid = fb_json_values_next_int(values, 0);

		if (typg.uid != api->uid) {
			typg.state = fb_json_values_next_int(values, 0);
			g_signal_emit_by_name(api, "typing", &typg);
		}
	}

	g_object_unref(values);
	json_node_free(root);
}

static void
fb_api_cb_publish_ms_r(FbApi *api, GByteArray *pload)
{
	FbApiMessage *msg;
	FbJsonValues *values;
	GError *err = NULL;
	JsonNode *root;

	if (!fb_api_json_chk(api, pload->data, pload->len, &root)) {
		return;
	}

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_BOOL, TRUE, "$.succeeded");
	fb_json_values_update(values, &err);

	FB_API_ERROR_EMIT(api, err,
		g_object_unref(values);
		json_node_free(root);
		return;
	);

	if (fb_json_values_next_bool(values, TRUE)) {
		/* Pop and free the successful message */
		msg = g_queue_pop_head(api->msgs);
		fb_api_message_free(msg);

		if (!g_queue_is_empty(api->msgs)) {
			msg = g_queue_peek_head(api->msgs);
			fb_api_message_send(api, msg);
		}
	} else {
		fb_api_error_literal(api, FB_API_ERROR_GENERAL,
		                     "Failed to send message");
	}

	g_object_unref(values);
	json_node_free(root);
}

static gchar *
fb_api_xma_parse(FbApi *api, const gchar *body, JsonNode *root, GError **error)
{
	const gchar *str;
	const gchar *url;
	FbHttpParams *params;
	FbJsonValues *values;
	gchar *text;
	GError *err = NULL;

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.story_attachment.target.__type__.name");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.story_attachment.url");
	fb_json_values_update(values, &err);

	if (G_UNLIKELY(err != NULL)) {
		g_propagate_error(error, err);
		g_object_unref(values);
		return NULL;
	}

	str = fb_json_values_next_str(values, NULL);
	url = fb_json_values_next_str(values, NULL);

	if ((str == NULL) || (url == NULL)) {
		text = g_strdup(_("<Unsupported Attachment>"));
		g_object_unref(values);
		return text;
	}

	if (purple_strequal(str, "ExternalUrl")) {
		params = fb_http_params_new_parse(url, TRUE);
		if (g_str_has_prefix(url, FB_API_FBRPC_PREFIX)) {
			text = fb_http_params_dup_str(params, "target_url", NULL);
		} else {
			text = fb_http_params_dup_str(params, "u", NULL);
		}
		fb_http_params_free(params);
	} else {
		text = g_strdup(url);
	}

	if (fb_http_urlcmp(body, text, FALSE)) {
		g_free(text);
		g_object_unref(values);
		return NULL;
	}

	g_object_unref(values);
	return text;
}

static GSList *
fb_api_message_parse_attach(FbApi *api, const gchar *mid, FbApiMessage *msg,
                            GSList *msgs, const gchar *body, JsonNode *root,
                            GError **error)
{
	const gchar *str;
	FbApiMessage *dmsg;
	FbId id;
	FbJsonValues *values;
	gchar *xma;
	GError *err = NULL;
	JsonNode *node;
	JsonNode *xode;

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE, "$.xmaGraphQL");
	fb_json_values_add(values, FB_JSON_TYPE_INT, FALSE, "$.fbid");
	fb_json_values_set_array(values, FALSE, "$.attachments");

	while (fb_json_values_update(values, &err)) {
		str = fb_json_values_next_str(values, NULL);

		if (str == NULL) {
			id = fb_json_values_next_int(values, 0);
			dmsg = g_memdup2(msg, sizeof(*msg));
			fb_api_attach(api, id, mid, dmsg);
			continue;
		}

		node = fb_json_node_new(str, -1, &err);

		if (G_UNLIKELY(err != NULL)) {
			break;
		}

		xode = fb_json_node_get_nth(node, 0);
		xma = fb_api_xma_parse(api, body, xode, &err);

		if (xma != NULL) {
			dmsg = g_memdup2(msg, sizeof(*msg));
			dmsg->text = xma;
			msgs = g_slist_prepend(msgs, dmsg);
		}

		json_node_free(node);

		if (G_UNLIKELY(err != NULL)) {
			break;
		}
	}

	if (G_UNLIKELY(err != NULL)) {
		g_propagate_error(error, err);
	}

	g_object_unref(values);
	return msgs;
}


static GSList *
fb_api_cb_publish_ms_new_message(FbApi *api, JsonNode *root, GSList *msgs, GError **error);

static GSList *
fb_api_cb_publish_ms_event(FbApi *api, JsonNode *root, GSList *events, FbApiEventType type, GError **error);

static void
fb_api_cb_publish_mst(FbThrift *thft, GError **error)
{
	if (fb_thrift_read_isstop(thft)) {
		FB_API_TCHK(fb_thrift_read_stop(thft));
	} else {
		FbThriftType type;
		gint16 id;

		FB_API_TCHK(fb_thrift_read_field(thft, &type, &id, 0));
		FB_API_TCHK(type == FB_THRIFT_TYPE_STRING);
		// FB_API_TCHK(id == 2);
		FB_API_TCHK(fb_thrift_read_str(thft, NULL));
		FB_API_TCHK(fb_thrift_read_stop(thft));
	}
}

static void
fb_api_cb_publish_ms(FbApi *api, GByteArray *pload)
{
	const gchar *data;
	FbJsonValues *values;
	FbThrift *thft;
	gchar *stoken;
	GError *err = NULL;
	GList *elms, *l;
	GSList *msgs = NULL;
	GSList *events = NULL;
	guint size;
	JsonNode *root;
	JsonNode *node;
	JsonArray *arr;

	static const struct {
		const gchar *member;
		FbApiEventType type;
		gboolean is_message;
	} event_types[] = {
		{"deltaNewMessage", 0, 1},
		{"deltaThreadName", FB_API_EVENT_TYPE_THREAD_TOPIC, 0},
		{"deltaParticipantsAddedToGroupThread", FB_API_EVENT_TYPE_THREAD_USER_ADDED, 0},
		{"deltaParticipantLeftGroupThread", FB_API_EVENT_TYPE_THREAD_USER_REMOVED, 0},
	};

	/* Read identifier string (for Facebook employees) */
	thft = fb_thrift_new(pload, 0);
	fb_api_cb_publish_mst(thft, &err);
	size = fb_thrift_get_pos(thft);
	g_object_unref(thft);

	FB_API_ERROR_EMIT(api, err,
		return;
	);

	g_return_if_fail(size < pload->len);
	data = (gchar *) pload->data + size;
	size = pload->len - size;

	if (!fb_api_json_chk(api, data, size, &root)) {
		return;
	}

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_INT, FALSE,
	                   "$.lastIssuedSeqId");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE, "$.syncToken");
	fb_json_values_update(values, &err);

	FB_API_ERROR_EMIT(api, err,
		g_object_unref(values);
		json_node_free(root);
		return;
	);

	api->sid = fb_json_values_next_int(values, 0);
	stoken = fb_json_values_next_str_dup(values, NULL);
	g_object_unref(values);

	if (G_UNLIKELY(stoken != NULL)) {
		g_free(api->stoken);
		api->stoken = stoken;
		g_signal_emit_by_name(api, "connect");
		json_node_free(root);
		return;
	}

	arr = fb_json_node_get_arr(root, "$.deltas", NULL);
	elms = json_array_get_elements(arr);

	for (l = elms; l != NULL; l = l->next) {
		guint i = 0;
		JsonObject *o = json_node_get_object(l->data);

		for (i = 0; i < G_N_ELEMENTS(event_types); i++) {
			if ((node = json_object_get_member(o, event_types[i].member))) {
				if (event_types[i].is_message) {
					msgs = fb_api_cb_publish_ms_new_message(
						api, node, msgs, &err
					);
				} else {
					events = fb_api_cb_publish_ms_event(
						api, node, events, event_types[i].type, &err
					);
				}
			}
		}

		if (G_UNLIKELY(err != NULL)) {
			break;
		}
	}

	g_list_free(elms);
	json_array_unref(arr);

	if (G_LIKELY(err == NULL)) {
		if (msgs) {
			msgs = g_slist_reverse(msgs);
			g_signal_emit_by_name(api, "messages", msgs);
		}

		if (events) {
			events = g_slist_reverse(events);
			g_signal_emit_by_name(api, "events", events);
		}
	} else {
		fb_api_error_emit(api, err);
	}

	g_slist_free_full(msgs, (GDestroyNotify) fb_api_message_free);
	g_slist_free_full(events, (GDestroyNotify) fb_api_event_free);
	json_node_free(root);
}

static GSList *
fb_api_cb_publish_ms_new_message(FbApi *api, JsonNode *root, GSList *msgs, GError **error)
{
	const gchar *body;
	const gchar *str;
	GError *err = NULL;
	FbApiMessage *dmsg;
	FbApiMessage msg;
	FbId id;
	FbId oid;
	FbJsonValues *values;
	JsonNode *node;

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_INT, FALSE,
	                   "$.messageMetadata.offlineThreadingId");
	fb_json_values_add(values, FB_JSON_TYPE_INT, FALSE,
	                   "$.messageMetadata.actorFbId");
	fb_json_values_add(values, FB_JSON_TYPE_INT, FALSE,
	                   "$.messageMetadata"
	                    ".threadKey.otherUserFbId");
	fb_json_values_add(values, FB_JSON_TYPE_INT, FALSE,
	                   "$.messageMetadata"
	                    ".threadKey.threadFbId");
	fb_json_values_add(values, FB_JSON_TYPE_INT, FALSE,
	                   "$.messageMetadata.timestamp");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.body");
	fb_json_values_add(values, FB_JSON_TYPE_INT, FALSE,
	                   "$.stickerId");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.messageMetadata.messageId");

	if (fb_json_values_update(values, &err)) {
		id = fb_json_values_next_int(values, 0);

		/* Ignore everything but new messages */
		if (id == 0) {
			goto beach;
		}

		/* Ignore sequential duplicates */
		if (id == api->lastmid) {
			fb_util_debug_info("Ignoring duplicate %" FB_ID_FORMAT, id);
			goto beach;
		}

		api->lastmid = id;
		fb_api_message_reset(&msg, FALSE);
		msg.uid = fb_json_values_next_int(values, 0);
		oid = fb_json_values_next_int(values, 0);
		msg.tid = fb_json_values_next_int(values, 0);
		msg.tstamp = fb_json_values_next_int(values, 0);

		if (msg.uid == api->uid) {
			msg.flags |= FB_API_MESSAGE_FLAG_SELF;

			if (msg.tid == 0) {
				msg.uid = oid;
			}
		}

		body = fb_json_values_next_str(values, NULL);

		if (body != NULL) {
			dmsg = g_memdup2(&msg, sizeof(msg));
			dmsg->text = g_strdup(body);
			msgs = g_slist_prepend(msgs, dmsg);
		}

		id = fb_json_values_next_int(values, 0);

		if (id != 0) {
			dmsg = g_memdup2(&msg, sizeof(msg));
			fb_api_sticker(api, id, dmsg);
		}

		str = fb_json_values_next_str(values, NULL);

		if (str == NULL) {
			goto beach;
		}

		node = fb_json_values_get_root(values);
		msgs = fb_api_message_parse_attach(api, str, &msg, msgs, body,
		                                   node, &err);

		if (G_UNLIKELY(err != NULL)) {
			g_propagate_error(error, err);
			goto beach;
		}
	}

beach:
	g_object_unref(values);
	return msgs;
}

static GSList *
fb_api_cb_publish_ms_event(FbApi *api, JsonNode *root, GSList *events, FbApiEventType type, GError **error)
{
	FbApiEvent *event;
	FbJsonValues *values = NULL;
	FbJsonValues *values_inner = NULL;
	GError *err = NULL;

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_INT, FALSE,
	                   "$.messageMetadata.threadKey.threadFbId");
	fb_json_values_add(values, FB_JSON_TYPE_INT, FALSE,
	                   "$.messageMetadata.actorFbId");

	switch (type) {
	case FB_API_EVENT_TYPE_THREAD_TOPIC:
		fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
		                   "$.name");
		break;

	case FB_API_EVENT_TYPE_THREAD_USER_ADDED:
		values_inner = fb_json_values_new(root);

		fb_json_values_add(values_inner, FB_JSON_TYPE_INT, FALSE,
		                   "$.userFbId");

		/* use the text field for the full name */
		fb_json_values_add(values_inner, FB_JSON_TYPE_STR, FALSE,
		                   "$.fullName");

		fb_json_values_set_array(values_inner, FALSE,
		                         "$.addedParticipants");
		break;

	case FB_API_EVENT_TYPE_THREAD_USER_REMOVED:
		fb_json_values_add(values, FB_JSON_TYPE_INT, FALSE,
		                   "$.leftParticipantFbId");

		/* use the text field for the kick message */
		fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
		                   "$.messageMetadata.adminText");
		break;
	}

	fb_json_values_update(values, &err);

	event = g_new0(FbApiEvent, 1);
	event->type = type;
	event->tid = fb_json_values_next_int(values, 0);
	event->uid = fb_json_values_next_int(values, 0);

	if (type == FB_API_EVENT_TYPE_THREAD_TOPIC) {
		event->text = fb_json_values_next_str_dup(values, NULL);
	} else if (type == FB_API_EVENT_TYPE_THREAD_USER_REMOVED) {
		/* overwrite actor with subject */
		event->uid = fb_json_values_next_int(values, 0);
		event->text = fb_json_values_next_str_dup(values, NULL);
	} else if (type == FB_API_EVENT_TYPE_THREAD_USER_ADDED) {

		while (fb_json_values_update(values_inner, &err)) {
			FbApiEvent *devent = g_new0(FbApiEvent, 1);

			devent->type = event->type;
			devent->uid = fb_json_values_next_int(values_inner, 0);
			devent->tid = event->tid;
			devent->text = fb_json_values_next_str_dup(values_inner, NULL);

			events = g_slist_prepend(events, devent);
		}
		g_clear_pointer(&event, fb_api_event_free);
		g_object_unref(values_inner);
	}

	g_object_unref(values);

	if (G_UNLIKELY(err != NULL)) {
		g_propagate_error(error, err);
	} else if (event) {
		events = g_slist_prepend(events, event);
	}

	return events;
}

static void
fb_api_cb_publish_pt(FbThrift *thft, GSList **presences, GError **error)
{
	FbApiPresence *api_presence;
	FbThriftType type;
	gint16 id;
	gint32 i32;
	gint64 i64;
	guint i;
	guint size = 0;

	/* Read identifier string (for Facebook employees) */
	FB_API_TCHK(fb_thrift_read_str(thft, NULL));

	/* Read the full list boolean field */
	FB_API_TCHK(fb_thrift_read_field(thft, &type, &id, 0));
	FB_API_TCHK(type == FB_THRIFT_TYPE_BOOL);
	FB_API_TCHK(id == 1);
	FB_API_TCHK(fb_thrift_read_bool(thft, NULL));

	/* Read the list field */
	FB_API_TCHK(fb_thrift_read_field(thft, &type, &id, id));
	FB_API_TCHK(type == FB_THRIFT_TYPE_LIST);
	FB_API_TCHK(id == 2);

	/* Read the list */
	FB_API_TCHK(fb_thrift_read_list(thft, &type, &size));
	FB_API_TCHK(type == FB_THRIFT_TYPE_STRUCT);

	for (i = 0; i < size; i++) {
		/* Read the user identifier field */
		FB_API_TCHK(fb_thrift_read_field(thft, &type, &id, 0));
		FB_API_TCHK(type == FB_THRIFT_TYPE_I64);
		FB_API_TCHK(id == 1);
		FB_API_TCHK(fb_thrift_read_i64(thft, &i64));

		/* Read the active field */
		FB_API_TCHK(fb_thrift_read_field(thft, &type, &id, id));
		FB_API_TCHK(type == FB_THRIFT_TYPE_I32);
		FB_API_TCHK(id == 2);
		FB_API_TCHK(fb_thrift_read_i32(thft, &i32));

		api_presence = g_new0(FbApiPresence, 1);
		api_presence->uid = i64;
		api_presence->active = i32 != 0;
		*presences = g_slist_prepend(*presences, api_presence);

		fb_util_debug_info("Presence: %" FB_ID_FORMAT " (%d) id: %d",
		                   i64, i32 != 0, id);

		while (id <= 6) {
			if (fb_thrift_read_isstop(thft)) {
				break;
			}

			FB_API_TCHK(fb_thrift_read_field(thft, &type, &id, id));

			switch (id) {
			case 3:
				/* Read the last active timestamp field */
				FB_API_TCHK(type == FB_THRIFT_TYPE_I64);
				FB_API_TCHK(fb_thrift_read_i64(thft, NULL));
				break;

			case 4:
				/* Read the active client bits field */
				FB_API_TCHK(type == FB_THRIFT_TYPE_I16);
				FB_API_TCHK(fb_thrift_read_i16(thft, NULL));
				break;

			case 5:
				/* Read the VoIP compatibility bits field */
				FB_API_TCHK(type == FB_THRIFT_TYPE_I64);
				FB_API_TCHK(fb_thrift_read_i64(thft, NULL));
				break;

			case 6:
				/* Unknown new field */
				FB_API_TCHK(type == FB_THRIFT_TYPE_I64);
				FB_API_TCHK(fb_thrift_read_i64(thft, NULL));
				break;

			default:
				/* Try to read unknown fields as varint */
				FB_API_TCHK(type == FB_THRIFT_TYPE_I16 ||
				            type == FB_THRIFT_TYPE_I32 ||
				            type == FB_THRIFT_TYPE_I64);
				FB_API_TCHK(fb_thrift_read_i64(thft, NULL));
				break;
			}
		}

		/* Read the field stop */
		FB_API_TCHK(fb_thrift_read_stop(thft));
	}

	/* Read the field stop */
	if (fb_thrift_read_isstop(thft)) {
		FB_API_TCHK(fb_thrift_read_stop(thft));
	}
}

static void
fb_api_cb_publish_p(FbApi *api, GByteArray *pload)
{
	FbThrift *thft;
	GError *err = NULL;
	GSList *presences = NULL;

	thft = fb_thrift_new(pload, 0);
	fb_api_cb_publish_pt(thft, &presences, &err);
	g_object_unref(thft);

	if (G_LIKELY(err == NULL)) {
		g_signal_emit_by_name(api, "presences", presences);
	} else {
		fb_api_error_emit(api, err);
	}

	g_slist_free_full(presences, (GDestroyNotify)fb_api_presence_free);
}

static void
fb_api_cb_mqtt_publish(FbMqtt *mqtt, const gchar *topic, GByteArray *pload,
                       gpointer data)
{
	FbApi *api = data;
	gboolean comp;
	GByteArray *bytes;
	GError *err = NULL;
	guint i;

	static const struct {
		const gchar *topic;
		void (*func) (FbApi *api, GByteArray *pload);
	} parsers[] = {
		{"/mark_thread_response", fb_api_cb_publish_mark},
		{"/mercury", fb_api_cb_publish_mercury},
		{"/orca_typing_notifications", fb_api_cb_publish_typing},
		{"/send_message_response", fb_api_cb_publish_ms_r},
		{"/t_ms", fb_api_cb_publish_ms},
		{"/t_p", fb_api_cb_publish_p}
	};

	comp = fb_util_zlib_test(pload);

	if (G_LIKELY(comp)) {
		bytes = fb_util_zlib_inflate(pload, &err);
		FB_API_ERROR_EMIT(api, err, return);
	} else {
		bytes = (GByteArray *) pload;
	}

	fb_util_debug_hexdump(FB_UTIL_DEBUG_INFO, bytes,
	                      "Reading message (topic: %s)",
			      topic);

	for (i = 0; i < G_N_ELEMENTS(parsers); i++) {
		if (g_ascii_strcasecmp(topic, parsers[i].topic) == 0) {
			parsers[i].func(api, bytes);
			break;
		}
	}

	if (G_LIKELY(comp)) {
		g_byte_array_free(bytes, TRUE);
	}
}

FbApi *
fb_api_new(PurpleConnection *gc, GProxyResolver *resolver)
{
	FbApi *api;

	api = g_object_new(FB_TYPE_API, NULL);

	api->gc = gc;
	api->cons = soup_session_new_with_options(
	        "proxy-resolver", resolver,
	        "user-agent", FB_API_AGENT,
	        NULL);
	api->mqtt = fb_mqtt_new(gc);

	g_signal_connect(api->mqtt,
	                 "connect",
	                 G_CALLBACK(fb_api_cb_mqtt_connect),
	                 api);
	g_signal_connect(api->mqtt,
	                 "error",
	                 G_CALLBACK(fb_api_cb_mqtt_error),
	                 api);
	g_signal_connect(api->mqtt,
	                 "open",
	                 G_CALLBACK(fb_api_cb_mqtt_open),
	                 api);
	g_signal_connect(api->mqtt,
	                 "publish",
	                 G_CALLBACK(fb_api_cb_mqtt_publish),
	                 api);

	return api;
}

void
fb_api_rehash(FbApi *api)
{
	g_return_if_fail(FB_IS_API(api));

	if (api->cid == NULL) {
		api->cid = fb_util_rand_alnum(32);
	}

	if (api->did == NULL) {
		api->did = g_uuid_string_random();
	}

	if (api->mid == 0) {
		api->mid = g_random_int();
	}

	if (strlen(api->cid) > 20) {
		api->cid = g_realloc_n(api->cid , 21, sizeof *api->cid);
		api->cid[20] = 0;
	}
}

gboolean
fb_api_is_invisible(FbApi *api)
{
	g_return_val_if_fail(FB_IS_API(api), FALSE);

	return api->invisible;
}

static void
fb_api_error_literal(FbApi *api, FbApiError error, const gchar *msg)
{
	GError *err;

	g_return_if_fail(FB_IS_API(api));

	err = g_error_new_literal(FB_API_ERROR, error, msg);

	fb_api_error_emit(api, err);
}

void
fb_api_error(FbApi *api, FbApiError error, const gchar *format, ...)
{
	GError *err;
	va_list ap;

	g_return_if_fail(FB_IS_API(api));

	va_start(ap, format);
	err = g_error_new_valist(FB_API_ERROR, error, format, ap);
	va_end(ap);

	fb_api_error_emit(api, err);
}

void
fb_api_error_emit(FbApi *api, GError *error)
{
	g_return_if_fail(FB_IS_API(api));
	g_return_if_fail(error != NULL);

	g_signal_emit_by_name(api, "error", error);
	g_error_free(error);
}

static void
fb_api_cb_attach(GObject *source, GAsyncResult *result, gpointer data) {
	SoupSession *session = SOUP_SESSION(source);
	SoupMessage *soupmsg = data;
	FbApi *api = g_object_get_data(G_OBJECT(soupmsg), "facebook-api");
	const gchar *str;
	FbApiMessage *msg;
	FbJsonValues *values;
	gchar *name;
	GError *err = NULL;
	GSList *msgs = NULL;
	guint i;
	JsonNode *root;

	static const gchar *imgexts[] = {".jpg", ".png", ".gif"};

	if (!fb_api_http_chk(api, session, result, soupmsg, &root)) {
		return;
	}

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE, "$.filename");
	fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE, "$.redirect_uri");
	fb_json_values_update(values, &err);

	FB_API_ERROR_EMIT(api, err,
		g_object_unref(values);
		json_node_free(root);
		return;
	);

	msg = g_object_steal_data(G_OBJECT(soupmsg), "fb-api-msg");
	str = fb_json_values_next_str(values, NULL);
	name = g_ascii_strdown(str, -1);

	for (i = 0; i < G_N_ELEMENTS(imgexts); i++) {
		if (g_str_has_suffix(name, imgexts[i])) {
			msg->flags |= FB_API_MESSAGE_FLAG_IMAGE;
			break;
		}
	}

	g_free(name);
	msg->text = fb_json_values_next_str_dup(values, NULL);
	msgs = g_slist_prepend(msgs, msg);

	g_signal_emit_by_name(api, "messages", msgs);
	g_slist_free_full(msgs, (GDestroyNotify) fb_api_message_free);
	g_object_unref(values);
	json_node_free(root);

}

static void
fb_api_attach(FbApi *api, FbId aid, const gchar *msgid, FbApiMessage *msg)
{
	FbHttpParams *prms;
	SoupMessage *http;

	prms = fb_http_params_new();
	fb_http_params_set_str(prms, "mid", msgid);
	fb_http_params_set_strf(prms, "aid", "%" FB_ID_FORMAT, aid);

	http = fb_api_http_req(api, FB_API_URL_ATTACH, "getAttachment",
	                       "messaging.getAttachment", prms,
			       fb_api_cb_attach);
	g_object_set_data_full(G_OBJECT(http), "fb-api-msg", msg,
	                       (GDestroyNotify)fb_api_message_free);
}

static void
fb_api_cb_auth(GObject *source, GAsyncResult *result, gpointer data) {
	SoupSession *session = SOUP_SESSION(source);
	SoupMessage *soupmsg = data;
	FbApi *api = g_object_get_data(G_OBJECT(soupmsg), "facebook-api");
	FbJsonValues *values;
	GError *err = NULL;
	JsonNode *root;

	if (!fb_api_http_chk(api, session, result, soupmsg, &root)) {
		return;
	}

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE, "$.access_token");
	fb_json_values_add(values, FB_JSON_TYPE_INT, TRUE, "$.uid");
	fb_json_values_update(values, &err);

	FB_API_ERROR_EMIT(api, err,
		g_object_unref(values);
		json_node_free(root);
		return;
	);

	g_free(api->token);
	api->token = fb_json_values_next_str_dup(values, NULL);
	api->uid = fb_json_values_next_int(values, 0);

	g_signal_emit_by_name(api, "auth");
	g_object_unref(values);
	json_node_free(root);
}

void
fb_api_auth(FbApi *api, const gchar *user, const gchar *pass)
{
	FbHttpParams *prms;

	prms = fb_http_params_new();
	fb_http_params_set_str(prms, "email", user);
	fb_http_params_set_str(prms, "password", pass);
	fb_api_http_req(api, FB_API_URL_AUTH, "authenticate", "auth.login",
	                prms, fb_api_cb_auth);
}

static gchar *
fb_api_user_icon_checksum(gchar *icon)
{
	gchar *csum;
	FbHttpParams *prms;

	if (G_UNLIKELY(icon == NULL)) {
		return NULL;
	}

	prms = fb_http_params_new_parse(icon, TRUE);
	csum = fb_http_params_dup_str(prms, "oh", NULL);
	fb_http_params_free(prms);

	if (G_UNLIKELY(csum == NULL)) {
		/* Revert to the icon URL as the unique checksum */
		csum = g_strdup(icon);
	}

	return csum;
}

static void
fb_api_cb_contact(GObject *source, GAsyncResult *result, gpointer data) {
	SoupSession *session = SOUP_SESSION(source);
	SoupMessage *soupmsg = data;
	FbApi *api = g_object_get_data(G_OBJECT(soupmsg), "facebook-api");
	const gchar *str;
	FbApiUser user;
	FbJsonValues *values;
	GError *err = NULL;
	JsonNode *node;
	JsonNode *root;

	if (!fb_api_http_chk(api, session, result, soupmsg, &root)) {
		return;
	}

	node = fb_json_node_get_nth(root, 0);

	if (node == NULL) {
		fb_api_error_literal(api, FB_API_ERROR_GENERAL,
		                     _("Failed to obtain contact information"));
		json_node_free(root);
		return;
	}

	values = fb_json_values_new(node);
	fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE, "$.id");
	fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE, "$.name");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.profile_pic_large.uri");
	fb_json_values_update(values, &err);

	FB_API_ERROR_EMIT(api, err,
		g_object_unref(values);
		json_node_free(root);
		return;
	);

	fb_api_user_reset(&user, FALSE);
	str = fb_json_values_next_str(values, "0");
	user.uid = FB_ID_FROM_STR(str);
	user.name = fb_json_values_next_str_dup(values, NULL);
	user.icon = fb_json_values_next_str_dup(values, NULL);

	user.csum = fb_api_user_icon_checksum(user.icon);

	g_signal_emit_by_name(api, "contact", &user);
	fb_api_user_reset(&user, TRUE);
	g_object_unref(values);
	json_node_free(root);
}

void
fb_api_contact(FbApi *api, FbId uid)
{
	JsonBuilder *bldr;

	bldr = fb_json_bldr_new(JSON_NODE_OBJECT);
	fb_json_bldr_arr_begin(bldr, "0");
	fb_json_bldr_add_strf(bldr, NULL, "%" FB_ID_FORMAT, uid);
	fb_json_bldr_arr_end(bldr);

	fb_json_bldr_add_str(bldr, "1", "true");
	fb_api_http_query(api, FB_API_QUERY_CONTACT, bldr, fb_api_cb_contact);
}

static GSList *
fb_api_cb_contacts_nodes(FbApi *api, JsonNode *root, GSList *users)
{
	const gchar *str;
	FbApiUser *user;
	FbId uid;
	FbJsonValues *values;
	gboolean is_array;
	GError *err = NULL;

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.represented_profile.id");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.represented_profile.friendship_status");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.structured_name.text");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.hugePictureUrl.uri");

	is_array = (JSON_NODE_TYPE(root) == JSON_NODE_ARRAY);

	if (is_array) {
		fb_json_values_set_array(values, FALSE, "$");
	}

	while (fb_json_values_update(values, &err)) {
		str = fb_json_values_next_str(values, "0");
		uid = FB_ID_FROM_STR(str);
		str = fb_json_values_next_str(values, NULL);

		if ((!purple_strequal(str, "ARE_FRIENDS") &&
		    (uid != api->uid)) || (uid == 0))
		{
			if (!is_array) {
				break;
			}
			continue;
		}

		user = g_new0(FbApiUser, 1);
		user->uid = uid;
		user->name = fb_json_values_next_str_dup(values, NULL);
		user->icon = fb_json_values_next_str_dup(values, NULL);

		user->csum = fb_api_user_icon_checksum(user->icon);

		users = g_slist_prepend(users, user);

		if (!is_array) {
			break;
		}
	}

	g_object_unref(values);

	return users;
}

/* base64(contact:<our id>:<their id>:<whatever>) */
static GSList *
fb_api_cb_contacts_parse_removed(FbApi *api, JsonNode *node, GSList *users)
{
	gsize len;
	char **split;
	char *decoded = (char *) g_base64_decode(json_node_get_string(node), &len);

	g_return_val_if_fail(decoded[len] == '\0', users);
	g_return_val_if_fail(len == strlen(decoded), users);
	g_return_val_if_fail(g_str_has_prefix(decoded, "contact:"), users);

	split = g_strsplit_set(decoded, ":", 4);

	if (g_strv_length(split) != 4) {
		g_strfreev(split);
		g_return_val_if_reached(users);
	}

	users = g_slist_prepend(users, g_strdup(split[2]));

	g_strfreev(split);
	g_free(decoded);

	return users;
}

static void
fb_api_cb_contacts(GObject *source, GAsyncResult *result, gpointer data) {
	SoupSession *session = SOUP_SESSION(source);
	SoupMessage *soupmsg = data;
	FbApi *api = g_object_get_data(G_OBJECT(soupmsg), "facebook-api");
	const gchar *cursor;
	const gchar *delta_cursor;
	FbJsonValues *values;
	gboolean complete;
	gboolean is_delta;
	GError *err = NULL;
	GList *l;
	GSList *users = NULL;
	JsonNode *root;
	JsonNode *croot;
	JsonNode *node;

	if (!fb_api_http_chk(api, session, result, soupmsg, &root)) {
		return;
	}

	croot = fb_json_node_get(root, "$.viewer.messenger_contacts.deltas", NULL);
	is_delta = (croot != NULL);

	if (!is_delta) {
		croot = fb_json_node_get(root, "$.viewer.messenger_contacts", NULL);
		node = fb_json_node_get(croot, "$.nodes", NULL);
		users = fb_api_cb_contacts_nodes(api, node, users);
		json_node_free(node);

	} else {
		GSList *added = NULL;
		GSList *removed = NULL;
		JsonArray *arr = fb_json_node_get_arr(croot, "$.nodes", NULL);
		GList *elms = json_array_get_elements(arr);

		for (l = elms; l != NULL; l = l->next) {
			if ((node = fb_json_node_get(l->data, "$.added", NULL))) {
				added = fb_api_cb_contacts_nodes(api, node, added);
				json_node_free(node);
			}

			if ((node = fb_json_node_get(l->data, "$.removed", NULL))) {
				removed = fb_api_cb_contacts_parse_removed(api, node, removed);
				json_node_free(node);
			}
		}

		g_signal_emit_by_name(api, "contacts-delta", added, removed);

		g_slist_free_full(added, (GDestroyNotify) fb_api_user_free);
		g_slist_free_full(removed, g_free);

		g_list_free(elms);
		json_array_unref(arr);
	}

	values = fb_json_values_new(croot);
	fb_json_values_add(values, FB_JSON_TYPE_BOOL, FALSE,
	                   "$.page_info.has_next_page");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.page_info.delta_cursor");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.page_info.end_cursor");
	fb_json_values_update(values, NULL);

	complete = !fb_json_values_next_bool(values, FALSE);

	delta_cursor = fb_json_values_next_str(values, NULL);

	cursor = fb_json_values_next_str(values, NULL);

	if (G_UNLIKELY(err == NULL)) {
		if (is_delta || complete) {
			g_free(api->contacts_delta);
			api->contacts_delta = g_strdup(is_delta ? cursor : delta_cursor);
		}

		if (users) {
			g_signal_emit_by_name(api, "contacts", users, complete);
		}

		if (!complete) {
			fb_api_contacts_after(api, cursor);
		}
	} else {
		fb_api_error_emit(api, err);
	}

	g_slist_free_full(users, (GDestroyNotify) fb_api_user_free);
	g_object_unref(values);

	json_node_free(croot);
	json_node_free(root);
}

void
fb_api_contacts(FbApi *api)
{
	JsonBuilder *bldr;

	g_return_if_fail(FB_IS_API(api));

	if (api->contacts_delta) {
		fb_api_contacts_delta(api, api->contacts_delta);
		return;
	}

	bldr = fb_json_bldr_new(JSON_NODE_OBJECT);
	fb_json_bldr_arr_begin(bldr, "0");
	fb_json_bldr_add_str(bldr, NULL, "user");
	fb_json_bldr_arr_end(bldr);

	fb_json_bldr_add_str(bldr, "1", G_STRINGIFY(FB_API_CONTACTS_COUNT));
	fb_api_http_query(api, FB_API_QUERY_CONTACTS, bldr,
	                  fb_api_cb_contacts);
}

static void
fb_api_contacts_after(FbApi *api, const gchar *cursor)
{
	JsonBuilder *bldr;

	bldr = fb_json_bldr_new(JSON_NODE_OBJECT);
	fb_json_bldr_arr_begin(bldr, "0");
	fb_json_bldr_add_str(bldr, NULL, "user");
	fb_json_bldr_arr_end(bldr);

	fb_json_bldr_add_str(bldr, "1", cursor);
	fb_json_bldr_add_str(bldr, "2", G_STRINGIFY(FB_API_CONTACTS_COUNT));
	fb_api_http_query(api, FB_API_QUERY_CONTACTS_AFTER, bldr,
	                  fb_api_cb_contacts);
}

void
fb_api_contacts_delta(FbApi *api, const gchar *delta_cursor)
{
	JsonBuilder *bldr;

	bldr = fb_json_bldr_new(JSON_NODE_OBJECT);

	fb_json_bldr_add_str(bldr, "0", delta_cursor);

	fb_json_bldr_arr_begin(bldr, "1");
	fb_json_bldr_add_str(bldr, NULL, "user");
	fb_json_bldr_arr_end(bldr);

	fb_json_bldr_add_str(bldr, "2", G_STRINGIFY(FB_API_CONTACTS_COUNT));
	fb_api_http_query(api, FB_API_QUERY_CONTACTS_DELTA, bldr,
	                  fb_api_cb_contacts);
}

void
fb_api_connect(FbApi *api, gboolean invisible)
{
	g_return_if_fail(FB_IS_API(api));

	api->invisible = invisible;
	fb_mqtt_open(api->mqtt, FB_MQTT_HOST, FB_MQTT_PORT);
}

void
fb_api_disconnect(FbApi *api)
{
	g_return_if_fail(FB_IS_API(api));

	fb_mqtt_disconnect(api->mqtt);
}

static void
fb_api_message_send(FbApi *api, FbApiMessage *msg)
{
	const gchar *tpfx;
	FbId id;
	FbId mid;
	gchar *json;
	JsonBuilder *bldr;

	mid = FB_API_MSGID(g_get_real_time() / 1000, g_random_int());
	api->lastmid = mid;

	if (msg->tid != 0) {
		tpfx = "tfbid_";
		id = msg->tid;
	} else {
		tpfx = "";
		id = msg->uid;
	}

	bldr = fb_json_bldr_new(JSON_NODE_OBJECT);
	fb_json_bldr_add_str(bldr, "body", msg->text);
	fb_json_bldr_add_strf(bldr, "msgid", "%" FB_ID_FORMAT, mid);
	fb_json_bldr_add_strf(bldr, "sender_fbid", "%" FB_ID_FORMAT, api->uid);
	fb_json_bldr_add_strf(bldr, "to", "%s%" FB_ID_FORMAT, tpfx, id);

	json = fb_json_bldr_close(bldr, JSON_NODE_OBJECT, NULL);
	fb_api_publish(api, "/send_message2", "%s", json);
	g_free(json);
}

void
fb_api_message(FbApi *api, FbId id, gboolean thread, const gchar *text)
{
	FbApiMessage *msg;
	gboolean empty;

	g_return_if_fail(FB_IS_API(api));
	g_return_if_fail(text != NULL);

	msg = g_new0(FbApiMessage, 1);
	msg->text = g_strdup(text);

	if (thread) {
		msg->tid = id;
	} else {
		msg->uid = id;
	}

	empty = g_queue_is_empty(api->msgs);
	g_queue_push_tail(api->msgs, msg);

	if (empty && fb_mqtt_connected(api->mqtt, FALSE)) {
		fb_api_message_send(api, msg);
	}
}

void
fb_api_publish(FbApi *api, const gchar *topic, const gchar *format, ...)
{
	GByteArray *bytes;
	GByteArray *cytes;
	gchar *msg;
	GError *err = NULL;
	va_list ap;

	g_return_if_fail(FB_IS_API(api));
	g_return_if_fail(topic != NULL);
	g_return_if_fail(format != NULL);

	va_start(ap, format);
	msg = g_strdup_vprintf(format, ap);
	va_end(ap);

	bytes = g_byte_array_new_take((guint8 *) msg, strlen(msg));
	cytes = fb_util_zlib_deflate(bytes, &err);

	FB_API_ERROR_EMIT(api, err,
		g_byte_array_free(bytes, TRUE);
		return;
	);

	fb_util_debug_hexdump(FB_UTIL_DEBUG_INFO, bytes,
	                      "Writing message (topic: %s)",
			      topic);

	fb_mqtt_publish(api->mqtt, topic, cytes);
	g_byte_array_free(cytes, TRUE);
	g_byte_array_free(bytes, TRUE);
}

void
fb_api_read(FbApi *api, FbId id, gboolean thread)
{
	const gchar *key;
	gchar *json;
	JsonBuilder *bldr;

	g_return_if_fail(FB_IS_API(api));

	bldr = fb_json_bldr_new(JSON_NODE_OBJECT);
	fb_json_bldr_add_bool(bldr, "state", TRUE);
	fb_json_bldr_add_int(bldr, "syncSeqId", api->sid);
	fb_json_bldr_add_str(bldr, "mark", "read");

	key = thread ? "threadFbId" : "otherUserFbId";
	fb_json_bldr_add_strf(bldr, key, "%" FB_ID_FORMAT, id);

	json = fb_json_bldr_close(bldr, JSON_NODE_OBJECT, NULL);
	fb_api_publish(api, "/mark_thread", "%s", json);
	g_free(json);
}

static GSList *
fb_api_cb_unread_parse_attach(FbApi *api, const gchar *mid, FbApiMessage *msg,
                              GSList *msgs, JsonNode *root, GError **error)
{
	const gchar *str;
	FbApiMessage *dmsg;
	FbId id;
	FbJsonValues *values;
	GError *err = NULL;

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE,
	                   "$.attachment_fbid");
	fb_json_values_set_array(values, FALSE, "$.blob_attachments");

	while (fb_json_values_update(values, &err)) {
		str = fb_json_values_next_str(values, NULL);
		id = FB_ID_FROM_STR(str);
		dmsg = g_memdup2(msg, sizeof(*msg));
		fb_api_attach(api, id, mid, dmsg);
	}

	if (G_UNLIKELY(err != NULL)) {
		g_propagate_error(error, err);
	}

	g_object_unref(values);
	return msgs;
}

static void
fb_api_cb_unread_msgs(GObject *source, GAsyncResult *result, gpointer data) {
	SoupSession *session = SOUP_SESSION(source);
	SoupMessage *soupmsg = data;
	FbApi *api = g_object_get_data(G_OBJECT(soupmsg), "facebook-api");
	const gchar *body;
	const gchar *str;
	FbApiMessage *dmsg;
	FbApiMessage msg;
	FbId id;
	FbId tid;
	FbJsonValues *values;
	gchar *xma;
	GError *err = NULL;
	GSList *msgs = NULL;
	JsonNode *node;
	JsonNode *root;
	JsonNode *xode;

	if (!fb_api_http_chk(api, session, result, soupmsg, &root)) {
		return;
	}

	node = fb_json_node_get_nth(root, 0);

	if (node == NULL) {
		fb_api_error_literal(api, FB_API_ERROR_GENERAL,
		                     _("Failed to obtain unread messages"));
		json_node_free(root);
		return;
	}

	values = fb_json_values_new(node);
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.thread_key.thread_fbid");
	fb_json_values_update(values, &err);

	FB_API_ERROR_EMIT(api, err,
		g_object_unref(values);
		return;
	);

	fb_api_message_reset(&msg, FALSE);
	str = fb_json_values_next_str(values, "0");
	tid = FB_ID_FROM_STR(str);
	g_object_unref(values);

	values = fb_json_values_new(node);
	fb_json_values_add(values, FB_JSON_TYPE_BOOL, TRUE, "$.unread");
	fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE,
	                   "$.message_sender.messaging_actor.id");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE, "$.message.text");
	fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE,
	                   "$.timestamp_precise");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE, "$.sticker.id");
	fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE, "$.message_id");
	fb_json_values_set_array(values, FALSE, "$.messages.nodes");

	while (fb_json_values_update(values, &err)) {
		if (!fb_json_values_next_bool(values, FALSE)) {
			continue;
		}

		str = fb_json_values_next_str(values, "0");
		body = fb_json_values_next_str(values, NULL);

		fb_api_message_reset(&msg, FALSE);
		msg.uid = FB_ID_FROM_STR(str);
		msg.tid = tid;

		str = fb_json_values_next_str(values, "0");
		msg.tstamp = g_ascii_strtoll(str, NULL, 10);

		if (body != NULL) {
			dmsg = g_memdup2(&msg, sizeof(msg));
			dmsg->text = g_strdup(body);
			msgs = g_slist_prepend(msgs, dmsg);
		}

		str = fb_json_values_next_str(values, NULL);

		if (str != NULL) {
			dmsg = g_memdup2(&msg, sizeof(msg));
			id = FB_ID_FROM_STR(str);
			fb_api_sticker(api, id, dmsg);
		}

		node = fb_json_values_get_root(values);
		xode = fb_json_node_get(node, "$.extensible_attachment", NULL);

		if (xode != NULL) {
			xma = fb_api_xma_parse(api, body, xode, &err);

			if (xma != NULL) {
				dmsg = g_memdup2(&msg, sizeof(msg));
				dmsg->text = xma;
				msgs = g_slist_prepend(msgs, dmsg);
			}

			json_node_free(xode);

			if (G_UNLIKELY(err != NULL)) {
				break;
			}
		}

		str = fb_json_values_next_str(values, NULL);

		if (str == NULL) {
			continue;
		}

		msgs = fb_api_cb_unread_parse_attach(api, str, &msg, msgs,
		                                     node, &err);

		if (G_UNLIKELY(err != NULL)) {
			break;
		}
	}

	if (G_UNLIKELY(err == NULL)) {
		msgs = g_slist_reverse(msgs);
		g_signal_emit_by_name(api, "messages", msgs);
	} else {
		fb_api_error_emit(api, err);
	}

	g_slist_free_full(msgs, (GDestroyNotify) fb_api_message_free);
	g_object_unref(values);
	json_node_free(root);
}

static void
fb_api_cb_unread(GObject *source, GAsyncResult *result, gpointer data) {
	SoupSession *session = SOUP_SESSION(source);
	SoupMessage *soupmsg = data;
	FbApi *api = g_object_get_data(G_OBJECT(soupmsg), "facebook-api");
	const gchar *id;
	FbJsonValues *values;
	GError *err = NULL;
	gint64 count;
	JsonBuilder *bldr;
	JsonNode *root;

	if (!fb_api_http_chk(api, session, result, soupmsg, &root)) {
		return;
	}

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_INT, TRUE, "$.unread_count");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.thread_key.other_user_id");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.thread_key.thread_fbid");
	fb_json_values_set_array(values, FALSE, "$.viewer.message_threads"
	                                         ".nodes");

	while (fb_json_values_update(values, &err)) {
		count = fb_json_values_next_int(values, -5);

		if (count < 1) {
			continue;
		}

		id = fb_json_values_next_str(values, NULL);

		if (id == NULL) {
			id = fb_json_values_next_str(values, "0");
		}

		bldr = fb_json_bldr_new(JSON_NODE_OBJECT);
		fb_json_bldr_arr_begin(bldr, "0");
		fb_json_bldr_add_str(bldr, NULL, id);
		fb_json_bldr_arr_end(bldr);

		fb_json_bldr_add_str(bldr, "10", "true");
		fb_json_bldr_add_str(bldr, "11", "true");
		fb_json_bldr_add_int(bldr, "12", count);
		fb_json_bldr_add_str(bldr, "13", "false");
		fb_api_http_query(api, FB_API_QUERY_THREAD, bldr,
		                  fb_api_cb_unread_msgs);
	}

	if (G_UNLIKELY(err != NULL)) {
		fb_api_error_emit(api, err);
	}

	g_object_unref(values);
	json_node_free(root);
}

void
fb_api_unread(FbApi *api)
{
	JsonBuilder *bldr;

	g_return_if_fail(FB_IS_API(api));

	if (api->unread < 1) {
		return;
	}

	bldr = fb_json_bldr_new(JSON_NODE_OBJECT);
	fb_json_bldr_add_str(bldr, "2", "true");
	fb_json_bldr_add_int(bldr, "1", api->unread);
	fb_json_bldr_add_str(bldr, "12", "true");
	fb_json_bldr_add_str(bldr, "13", "false");
	fb_api_http_query(api, FB_API_QUERY_THREADS, bldr,
	                  fb_api_cb_unread);
}

static void
fb_api_cb_sticker(GObject *source, GAsyncResult *result, gpointer data) {
	SoupSession *session = SOUP_SESSION(source);
	SoupMessage *soupmsg = data;
	FbApi *api = g_object_get_data(G_OBJECT(soupmsg), "facebook-api");
	FbApiMessage *msg;
	FbJsonValues *values;
	GError *err = NULL;
	GSList *msgs = NULL;
	JsonNode *node;
	JsonNode *root;

	if (!fb_api_http_chk(api, session, result, soupmsg, &root)) {
		return;
	}

	node = fb_json_node_get_nth(root, 0);
	values = fb_json_values_new(node);
	fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE,
	                   "$.thread_image.uri");
	fb_json_values_update(values, &err);

	FB_API_ERROR_EMIT(api, err,
		g_object_unref(values);
		json_node_free(root);
		return;
	);

	msg = g_object_steal_data(G_OBJECT(soupmsg), "fb-api-msg");
	msg->flags |= FB_API_MESSAGE_FLAG_IMAGE;
	msg->text = fb_json_values_next_str_dup(values, NULL);
	msgs = g_slist_prepend(msgs, msg);

	g_signal_emit_by_name(api, "messages", msgs);
	g_slist_free_full(msgs, (GDestroyNotify) fb_api_message_free);
	g_object_unref(values);
	json_node_free(root);
}

static void
fb_api_sticker(FbApi *api, FbId sid, FbApiMessage *msg)
{
	JsonBuilder *bldr;
	SoupMessage *http;

	bldr = fb_json_bldr_new(JSON_NODE_OBJECT);
	fb_json_bldr_arr_begin(bldr, "0");
	fb_json_bldr_add_strf(bldr, NULL, "%" FB_ID_FORMAT, sid);
	fb_json_bldr_arr_end(bldr);

	http = fb_api_http_query(api, FB_API_QUERY_STICKER, bldr,
	                         fb_api_cb_sticker);
	g_object_set_data_full(G_OBJECT(http), "fb-api-msg", msg,
	                       (GDestroyNotify)fb_api_message_free);
}

static gboolean
fb_api_thread_parse(FbApi *api, FbApiThread *thrd, JsonNode *root,
                    GError **error)
{
	const gchar *str;
	FbApiUser *user;
	FbId uid;
	FbJsonValues *values;
	gboolean haself = FALSE;
	guint num_users = 0;
	GError *err = NULL;

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE,
	                   "$.thread_key.thread_fbid");
	fb_json_values_add(values, FB_JSON_TYPE_STR, FALSE, "$.name");
	fb_json_values_update(values, &err);

	if (G_UNLIKELY(err != NULL)) {
		g_propagate_error(error, err);
		g_object_unref(values);
		return FALSE;
	}

	str = fb_json_values_next_str(values, NULL);

	if (str == NULL) {
		g_object_unref(values);
		return FALSE;
	}

	thrd->tid = FB_ID_FROM_STR(str);
	thrd->topic = fb_json_values_next_str_dup(values, NULL);
	g_object_unref(values);

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE,
	                   "$.messaging_actor.id");
	fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE,
	                   "$.messaging_actor.name");
	fb_json_values_set_array(values, TRUE, "$.all_participants.nodes");

	while (fb_json_values_update(values, &err)) {
		str = fb_json_values_next_str(values, "0");
		uid = FB_ID_FROM_STR(str);
		num_users++;

		if (uid != api->uid) {
			user = g_new0(FbApiUser, 1);
			user->uid = uid;
			user->name = fb_json_values_next_str_dup(values, NULL);
			thrd->users = g_slist_prepend(thrd->users, user);
		} else {
			haself = TRUE;
		}
	}

	if (G_UNLIKELY(err != NULL)) {
		g_propagate_error(error, err);
		fb_api_thread_reset(thrd, TRUE);
		g_object_unref(values);
		return FALSE;
	}

	if (num_users < 2 || !haself) {
		g_object_unref(values);
		return FALSE;
	}

	g_object_unref(values);
	return TRUE;
}

static void
fb_api_cb_thread(GObject *source, GAsyncResult *result, gpointer data) {
	SoupSession *session = SOUP_SESSION(source);
	SoupMessage *soupmsg = data;
	FbApi *api = g_object_get_data(G_OBJECT(soupmsg), "facebook-api");
	FbApiThread thrd;
	GError *err = NULL;
	JsonNode *node;
	JsonNode *root;

	if (!fb_api_http_chk(api, session, result, soupmsg, &root)) {
		return;
	}

	node = fb_json_node_get_nth(root, 0);

	if (node == NULL) {
		fb_api_error_literal(api, FB_API_ERROR_GENERAL,
		                     _("Failed to obtain thread information"));
		json_node_free(root);
		return;
	}

	fb_api_thread_reset(&thrd, FALSE);

	if (!fb_api_thread_parse(api, &thrd, node, &err)) {
		if (G_LIKELY(err == NULL)) {
			if (thrd.tid) {
				g_signal_emit_by_name(api, "thread-kicked", &thrd);
			} else {
				fb_api_error_literal(api, FB_API_ERROR_GENERAL,
				                     _("Failed to parse thread information"));
			}
		} else {
			fb_api_error_emit(api, err);
		}
	} else {
		g_signal_emit_by_name(api, "thread", &thrd);
	}

	fb_api_thread_reset(&thrd, TRUE);
	json_node_free(root);
}

void
fb_api_thread(FbApi *api, FbId tid)
{
	JsonBuilder *bldr;

	bldr = fb_json_bldr_new(JSON_NODE_OBJECT);
	fb_json_bldr_arr_begin(bldr, "0");
	fb_json_bldr_add_strf(bldr, NULL, "%" FB_ID_FORMAT, tid);
	fb_json_bldr_arr_end(bldr);

	fb_json_bldr_add_str(bldr, "10", "false");
	fb_json_bldr_add_str(bldr, "11", "false");
	fb_json_bldr_add_str(bldr, "13", "false");
	fb_api_http_query(api, FB_API_QUERY_THREAD, bldr, fb_api_cb_thread);
}

static void
fb_api_cb_thread_create(GObject *source, GAsyncResult *result, gpointer data) {
	SoupSession *session = SOUP_SESSION(source);
	SoupMessage *soupmsg = data;
	FbApi *api = g_object_get_data(G_OBJECT(soupmsg), "facebook-api");
	const gchar *str;
	FbId tid;
	FbJsonValues *values;
	GError *err = NULL;
	JsonNode *root;

	if (!fb_api_http_chk(api, session, result, soupmsg, &root)) {
		return;
	}

	values = fb_json_values_new(root);
	fb_json_values_add(values, FB_JSON_TYPE_STR, TRUE, "$.id");
	fb_json_values_update(values, &err);

	FB_API_ERROR_EMIT(api, err,
		g_object_unref(values);
		json_node_free(root);
		return;
	);

	str = fb_json_values_next_str(values, "0");
	tid = FB_ID_FROM_STR(str);
	g_signal_emit_by_name(api, "thread-create", tid);

	g_object_unref(values);
	json_node_free(root);
}

void
fb_api_thread_create(FbApi *api, GSList *uids)
{
	FbHttpParams *prms;
	FbId *uid;
	gchar *json;
	GSList *l;
	JsonBuilder *bldr;

	g_return_if_fail(FB_IS_API(api));
	g_warn_if_fail(g_slist_length(uids) > 1);

	bldr = fb_json_bldr_new(JSON_NODE_ARRAY);
	fb_json_bldr_obj_begin(bldr, NULL);
	fb_json_bldr_add_str(bldr, "type", "id");
	fb_json_bldr_add_strf(bldr, "id", "%" FB_ID_FORMAT, api->uid);
	fb_json_bldr_obj_end(bldr);

	for (l = uids; l != NULL; l = l->next) {
		uid = l->data;
		fb_json_bldr_obj_begin(bldr, NULL);
		fb_json_bldr_add_str(bldr, "type", "id");
		fb_json_bldr_add_strf(bldr, "id", "%" FB_ID_FORMAT, *uid);
		fb_json_bldr_obj_end(bldr);
	}

	json = fb_json_bldr_close(bldr, JSON_NODE_ARRAY, NULL);
	prms = fb_http_params_new();
	fb_http_params_set_str(prms, "recipients", json);
	fb_api_http_req(api, FB_API_URL_THREADS, "createGroup", "POST",
	                prms, fb_api_cb_thread_create);
	g_free(json);
}

void
fb_api_thread_invite(FbApi *api, FbId tid, FbId uid)
{
	FbHttpParams *prms;
	gchar *json;
	JsonBuilder *bldr;

	bldr = fb_json_bldr_new(JSON_NODE_ARRAY);
	fb_json_bldr_obj_begin(bldr, NULL);
	fb_json_bldr_add_str(bldr, "type", "id");
	fb_json_bldr_add_strf(bldr, "id", "%" FB_ID_FORMAT, uid);
	fb_json_bldr_obj_end(bldr);
	json = fb_json_bldr_close(bldr, JSON_NODE_ARRAY, NULL);

	prms = fb_http_params_new();
	fb_http_params_set_str(prms, "to", json);
	fb_http_params_set_strf(prms, "id", "t_%" FB_ID_FORMAT, tid);
	fb_api_http_req(api, FB_API_URL_PARTS, "addMembers", "POST",
	                prms, fb_api_cb_http_bool);
	g_free(json);
}

void
fb_api_thread_remove(FbApi *api, FbId tid, FbId uid)
{
	FbHttpParams *prms;
	gchar *json;
	JsonBuilder *bldr;

	g_return_if_fail(FB_IS_API(api));

	prms = fb_http_params_new();
	fb_http_params_set_strf(prms, "id", "t_%" FB_ID_FORMAT, tid);

	if (uid == 0) {
		uid = api->uid;
	}

	if (uid != api->uid) {
		bldr = fb_json_bldr_new(JSON_NODE_ARRAY);
		fb_json_bldr_add_strf(bldr, NULL, "%" FB_ID_FORMAT, uid);
		json = fb_json_bldr_close(bldr, JSON_NODE_ARRAY, NULL);
		fb_http_params_set_str(prms, "to", json);
		g_free(json);
	}

	fb_api_http_req(api, FB_API_URL_PARTS, "removeMembers", "DELETE",
	                prms, fb_api_cb_http_bool);
}

void
fb_api_thread_topic(FbApi *api, FbId tid, const gchar *topic)
{
	FbHttpParams *prms;

	prms = fb_http_params_new();
	fb_http_params_set_str(prms, "name", topic);
	fb_http_params_set_int(prms, "tid", tid);
	fb_api_http_req(api, FB_API_URL_TOPIC, "setThreadName",
	                "messaging.setthreadname", prms,
			fb_api_cb_http_bool);
}

static void
fb_api_cb_threads(GObject *source, GAsyncResult *result, gpointer data) {
	SoupSession *session = SOUP_SESSION(source);
	SoupMessage *soupmsg = data;
	FbApi *api = g_object_get_data(G_OBJECT(soupmsg), "facebook-api");
	FbApiThread *dthrd;
	FbApiThread thrd;
	GError *err = NULL;
	GList *elms;
	GList *l;
	GSList *thrds = NULL;
	JsonArray *arr;
	JsonNode *root;

	if (!fb_api_http_chk(api, session, result, soupmsg, &root)) {
		return;
	}

	arr = fb_json_node_get_arr(root, "$.viewer.message_threads.nodes",
	                           &err);
	FB_API_ERROR_EMIT(api, err,
		json_node_free(root);
		return;
	);

	elms = json_array_get_elements(arr);

	for (l = elms; l != NULL; l = l->next) {
		fb_api_thread_reset(&thrd, FALSE);

		if (fb_api_thread_parse(api, &thrd, l->data, &err)) {
			dthrd = g_memdup2(&thrd, sizeof(thrd));
			thrds = g_slist_prepend(thrds, dthrd);
		} else {
			fb_api_thread_reset(&thrd, TRUE);
		}

		if (G_UNLIKELY(err != NULL)) {
			break;
		}
	}

	if (G_LIKELY(err == NULL)) {
		thrds = g_slist_reverse(thrds);
		g_signal_emit_by_name(api, "threads", thrds);
	} else {
		fb_api_error_emit(api, err);
	}

	g_slist_free_full(thrds, (GDestroyNotify) fb_api_thread_free);
	g_list_free(elms);
	json_array_unref(arr);
	json_node_free(root);
}

void
fb_api_threads(FbApi *api)
{
	JsonBuilder *bldr;

	bldr = fb_json_bldr_new(JSON_NODE_OBJECT);
	fb_json_bldr_add_str(bldr, "2", "true");
	fb_json_bldr_add_str(bldr, "12", "false");
	fb_json_bldr_add_str(bldr, "13", "false");
	fb_api_http_query(api, FB_API_QUERY_THREADS, bldr, fb_api_cb_threads);
}

void
fb_api_typing(FbApi *api, FbId uid, gboolean state)
{
	gchar *json;
	JsonBuilder *bldr;

	bldr = fb_json_bldr_new(JSON_NODE_OBJECT);
	fb_json_bldr_add_int(bldr, "state", state != 0);
	fb_json_bldr_add_strf(bldr, "to", "%" FB_ID_FORMAT, uid);

	json = fb_json_bldr_close(bldr, JSON_NODE_OBJECT, NULL);
	fb_api_publish(api, "/typing", "%s", json);
	g_free(json);
}

FbApiEvent *
fb_api_event_dup(const FbApiEvent *event)
{
	FbApiEvent *ret;

	if (event == NULL) {
		return NULL;
	}

	ret = g_memdup2(event, sizeof *event);
	ret->text = g_strdup(event->text);

	return ret;
}

void
fb_api_event_reset(FbApiEvent *event, gboolean deep)
{
	g_return_if_fail(event != NULL);

	if (deep) {
		g_free(event->text);
	}

	memset(event, 0, sizeof *event);
}

void
fb_api_event_free(FbApiEvent *event)
{
	if (G_LIKELY(event != NULL)) {
		g_free(event->text);
		g_free(event);
	}
}

G_DEFINE_BOXED_TYPE(FbApiEvent, fb_api_event, fb_api_event_dup,
                    fb_api_event_free)

FbApiMessage *
fb_api_message_dup(const FbApiMessage *msg)
{
	FbApiMessage *ret;

	if (msg == NULL) {
		return NULL;
	}

	ret = g_memdup2(msg, sizeof *msg);
	ret->text = g_strdup(msg->text);

	return ret;
}

void
fb_api_message_reset(FbApiMessage *msg, gboolean deep)
{
	g_return_if_fail(msg != NULL);

	if (deep) {
		g_free(msg->text);
	}

	memset(msg, 0, sizeof *msg);
}

void
fb_api_message_free(FbApiMessage *msg)
{
	if (G_LIKELY(msg != NULL)) {
		g_free(msg->text);
		g_free(msg);
	}
}

G_DEFINE_BOXED_TYPE(FbApiMessage, fb_api_message, fb_api_message_dup,
                    fb_api_message_free)

FbApiPresence *
fb_api_presence_dup(const FbApiPresence *presence)
{
	return g_memdup2(presence, sizeof *presence);
}

void
fb_api_presence_reset(FbApiPresence *presence)
{
	g_return_if_fail(presence != NULL);
	memset(presence, 0, sizeof *presence);
}

void
fb_api_presence_free(FbApiPresence *presence)
{
	if (G_LIKELY(presence != NULL)) {
		g_free(presence);
	}
}

G_DEFINE_BOXED_TYPE(FbApiPresence, fb_api_presence, fb_api_presence_dup,
                    fb_api_presence_free)

FbApiThread *
fb_api_thread_dup(const FbApiThread *thrd)
{
	FbApiThread *ret;

	if (thrd == NULL) {
		return NULL;
	}

	ret = g_memdup2(thrd, sizeof *thrd);
	ret->topic = g_strdup(thrd->topic);
	ret->users = g_slist_copy_deep(thrd->users, (GCopyFunc)fb_api_user_dup, NULL);

	return ret;
}

void
fb_api_thread_reset(FbApiThread *thrd, gboolean deep)
{
	g_return_if_fail(thrd != NULL);

	if (deep) {
		g_slist_free_full(thrd->users, (GDestroyNotify) fb_api_user_free);
		g_free(thrd->topic);
	}

	memset(thrd, 0, sizeof *thrd);
}

void
fb_api_thread_free(FbApiThread *thrd)
{
	if (G_LIKELY(thrd != NULL)) {
		g_slist_free_full(thrd->users, (GDestroyNotify) fb_api_user_free);
		g_free(thrd->topic);
		g_free(thrd);
	}
}

G_DEFINE_BOXED_TYPE(FbApiThread, fb_api_thread, fb_api_thread_dup,
                    fb_api_thread_free)

FbApiTyping *
fb_api_typing_dup(const FbApiTyping *typg)
{
	return g_memdup2(typg, sizeof *typg);
}

void
fb_api_typing_reset(FbApiTyping *typg)
{
	g_return_if_fail(typg != NULL);
	memset(typg, 0, sizeof *typg);
}

void
fb_api_typing_free(FbApiTyping *typg)
{
	if (G_LIKELY(typg != NULL)) {
		g_free(typg);
	}
}

G_DEFINE_BOXED_TYPE(FbApiTyping, fb_api_typing, fb_api_typing_dup,
                    fb_api_typing_free)

FbApiUser *
fb_api_user_dup(const FbApiUser *user)
{
	FbApiUser *ret;

	if (user == NULL) {
		return NULL;
	}

	ret = g_memdup2(user, sizeof *user);
	ret->name = g_strdup(user->name);
	ret->icon = g_strdup(user->icon);
	ret->csum = g_strdup(user->csum);

	return ret;
}

void
fb_api_user_reset(FbApiUser *user, gboolean deep)
{
	g_return_if_fail(user != NULL);

	if (deep) {
		g_free(user->name);
		g_free(user->icon);
		g_free(user->csum);
	}

	memset(user, 0, sizeof *user);
}

void
fb_api_user_free(FbApiUser *user)
{
	if (G_LIKELY(user != NULL)) {
		g_free(user->name);
		g_free(user->icon);
		g_free(user->csum);
		g_free(user);
	}
}

G_DEFINE_BOXED_TYPE(FbApiUser, fb_api_user, fb_api_user_dup, fb_api_user_free)