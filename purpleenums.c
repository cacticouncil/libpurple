
/* This file is generated by glib-mkenums, do not modify it. This code is licensed under the same license as the containing project. Note that it links to GLib, so must comply with the LGPL linking clauses. */

#include "purpleenums.h"
#include "account.h"
#include "buddyicon.h"
#include "connection.h"
#include "debug.h"
#include "eventloop.h"
#include "notify.h"
#include "plugins.h"
#include "purplechatuser.h"
#include "purpleconnectionerrorinfo.h"
#include "purpleconversation.h"
#include "purpleimconversation.h"
#include "purplemessage.h"
#include "purplenotification.h"
#include "purpleplugininfo.h"
#include "purpleprotocol.h"
#include "purpleproxyinfo.h"
#include "roomlist.h"
#include "status.h"
#include "xfer.h"
#include "xmlnode.h"

#define C_ENUM(v) ((gint) v)
#define C_FLAGS(v) ((guint) v)

/* enumerations from "account.h" */

GType
purple_account_request_response_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_ACCOUNT_RESPONSE_IGNORE), "PURPLE_ACCOUNT_RESPONSE_IGNORE", "ignore" },
    { C_ENUM(PURPLE_ACCOUNT_RESPONSE_DENY), "PURPLE_ACCOUNT_RESPONSE_DENY", "deny" },
    { C_ENUM(PURPLE_ACCOUNT_RESPONSE_PASS), "PURPLE_ACCOUNT_RESPONSE_PASS", "pass" },
    { C_ENUM(PURPLE_ACCOUNT_RESPONSE_ACCEPT), "PURPLE_ACCOUNT_RESPONSE_ACCEPT", "accept" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleAccountRequestResponse"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

GType
purple_account_privacy_type_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_ACCOUNT_PRIVACY_ALLOW_ALL), "PURPLE_ACCOUNT_PRIVACY_ALLOW_ALL", "allow-all" },
    { C_ENUM(PURPLE_ACCOUNT_PRIVACY_DENY_ALL), "PURPLE_ACCOUNT_PRIVACY_DENY_ALL", "deny-all" },
    { C_ENUM(PURPLE_ACCOUNT_PRIVACY_ALLOW_USERS), "PURPLE_ACCOUNT_PRIVACY_ALLOW_USERS", "allow-users" },
    { C_ENUM(PURPLE_ACCOUNT_PRIVACY_DENY_USERS), "PURPLE_ACCOUNT_PRIVACY_DENY_USERS", "deny-users" },
    { C_ENUM(PURPLE_ACCOUNT_PRIVACY_ALLOW_BUDDYLIST), "PURPLE_ACCOUNT_PRIVACY_ALLOW_BUDDYLIST", "allow-buddylist" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleAccountPrivacyType"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "buddyicon.h" */

GType
purple_buddy_icon_scale_flags_get_type (void)
{
static gsize gtype_id = 0;
static const GFlagsValue values[] = {
    { C_FLAGS(PURPLE_ICON_SCALE_DISPLAY), "PURPLE_ICON_SCALE_DISPLAY", "display" },
    { C_FLAGS(PURPLE_ICON_SCALE_SEND), "PURPLE_ICON_SCALE_SEND", "send" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_flags_register_static (g_intern_static_string ("PurpleBuddyIconScaleFlags"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "connection.h" */

GType
purple_connection_flags_get_type (void)
{
static gsize gtype_id = 0;
static const GFlagsValue values[] = {
    { C_FLAGS(PURPLE_CONNECTION_FLAG_HTML), "PURPLE_CONNECTION_FLAG_HTML", "html" },
    { C_FLAGS(PURPLE_CONNECTION_FLAG_NO_BGCOLOR), "PURPLE_CONNECTION_FLAG_NO_BGCOLOR", "no-bgcolor" },
    { C_FLAGS(PURPLE_CONNECTION_FLAG_AUTO_RESP), "PURPLE_CONNECTION_FLAG_AUTO_RESP", "auto-resp" },
    { C_FLAGS(PURPLE_CONNECTION_FLAG_FORMATTING_WBFO), "PURPLE_CONNECTION_FLAG_FORMATTING_WBFO", "formatting-wbfo" },
    { C_FLAGS(PURPLE_CONNECTION_FLAG_NO_NEWLINES), "PURPLE_CONNECTION_FLAG_NO_NEWLINES", "no-newlines" },
    { C_FLAGS(PURPLE_CONNECTION_FLAG_NO_FONTSIZE), "PURPLE_CONNECTION_FLAG_NO_FONTSIZE", "no-fontsize" },
    { C_FLAGS(PURPLE_CONNECTION_FLAG_NO_URLDESC), "PURPLE_CONNECTION_FLAG_NO_URLDESC", "no-urldesc" },
    { C_FLAGS(PURPLE_CONNECTION_FLAG_NO_IMAGES), "PURPLE_CONNECTION_FLAG_NO_IMAGES", "no-images" },
    { C_FLAGS(PURPLE_CONNECTION_FLAG_SUPPORT_MOODS), "PURPLE_CONNECTION_FLAG_SUPPORT_MOODS", "support-moods" },
    { C_FLAGS(PURPLE_CONNECTION_FLAG_SUPPORT_MOOD_MESSAGES), "PURPLE_CONNECTION_FLAG_SUPPORT_MOOD_MESSAGES", "support-mood-messages" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_flags_register_static (g_intern_static_string ("PurpleConnectionFlags"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

GType
purple_connection_state_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_CONNECTION_STATE_DISCONNECTED), "PURPLE_CONNECTION_STATE_DISCONNECTED", "disconnected" },
    { C_ENUM(PURPLE_CONNECTION_STATE_DISCONNECTING), "PURPLE_CONNECTION_STATE_DISCONNECTING", "disconnecting" },
    { C_ENUM(PURPLE_CONNECTION_STATE_CONNECTED), "PURPLE_CONNECTION_STATE_CONNECTED", "connected" },
    { C_ENUM(PURPLE_CONNECTION_STATE_CONNECTING), "PURPLE_CONNECTION_STATE_CONNECTING", "connecting" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleConnectionState"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "debug.h" */

GType
purple_debug_level_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_DEBUG_ALL), "PURPLE_DEBUG_ALL", "all" },
    { C_ENUM(PURPLE_DEBUG_MISC), "PURPLE_DEBUG_MISC", "misc" },
    { C_ENUM(PURPLE_DEBUG_INFO), "PURPLE_DEBUG_INFO", "info" },
    { C_ENUM(PURPLE_DEBUG_WARNING), "PURPLE_DEBUG_WARNING", "warning" },
    { C_ENUM(PURPLE_DEBUG_ERROR), "PURPLE_DEBUG_ERROR", "error" },
    { C_ENUM(PURPLE_DEBUG_FATAL), "PURPLE_DEBUG_FATAL", "fatal" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleDebugLevel"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "eventloop.h" */

GType
purple_input_condition_get_type (void)
{
static gsize gtype_id = 0;
static const GFlagsValue values[] = {
    { C_FLAGS(PURPLE_INPUT_READ), "PURPLE_INPUT_READ", "read" },
    { C_FLAGS(PURPLE_INPUT_WRITE), "PURPLE_INPUT_WRITE", "write" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_flags_register_static (g_intern_static_string ("PurpleInputCondition"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "notify.h" */

GType
purple_notify_type_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_NOTIFY_MESSAGE), "PURPLE_NOTIFY_MESSAGE", "message" },
    { C_ENUM(PURPLE_NOTIFY_FORMATTED), "PURPLE_NOTIFY_FORMATTED", "formatted" },
    { C_ENUM(PURPLE_NOTIFY_SEARCHRESULTS), "PURPLE_NOTIFY_SEARCHRESULTS", "searchresults" },
    { C_ENUM(PURPLE_NOTIFY_USERINFO), "PURPLE_NOTIFY_USERINFO", "userinfo" },
    { C_ENUM(PURPLE_NOTIFY_URI), "PURPLE_NOTIFY_URI", "uri" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleNotifyType"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

GType
purple_notify_message_type_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_NOTIFY_MSG_ERROR), "PURPLE_NOTIFY_MSG_ERROR", "error" },
    { C_ENUM(PURPLE_NOTIFY_MSG_WARNING), "PURPLE_NOTIFY_MSG_WARNING", "warning" },
    { C_ENUM(PURPLE_NOTIFY_MSG_INFO), "PURPLE_NOTIFY_MSG_INFO", "info" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleNotifyMessageType"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

GType
purple_notify_search_button_type_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_NOTIFY_BUTTON_LABELED), "PURPLE_NOTIFY_BUTTON_LABELED", "labeled" },
    { C_ENUM(PURPLE_NOTIFY_BUTTON_CONTINUE), "PURPLE_NOTIFY_BUTTON_CONTINUE", "continue" },
    { C_ENUM(PURPLE_NOTIFY_BUTTON_ADD), "PURPLE_NOTIFY_BUTTON_ADD", "add" },
    { C_ENUM(PURPLE_NOTIFY_BUTTON_INFO), "PURPLE_NOTIFY_BUTTON_INFO", "info" },
    { C_ENUM(PURPLE_NOTIFY_BUTTON_IM), "PURPLE_NOTIFY_BUTTON_IM", "im" },
    { C_ENUM(PURPLE_NOTIFY_BUTTON_JOIN), "PURPLE_NOTIFY_BUTTON_JOIN", "join" },
    { C_ENUM(PURPLE_NOTIFY_BUTTON_INVITE), "PURPLE_NOTIFY_BUTTON_INVITE", "invite" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleNotifySearchButtonType"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

GType
purple_notify_user_info_entry_type_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_NOTIFY_USER_INFO_ENTRY_PAIR), "PURPLE_NOTIFY_USER_INFO_ENTRY_PAIR", "pair" },
    { C_ENUM(PURPLE_NOTIFY_USER_INFO_ENTRY_SECTION_BREAK), "PURPLE_NOTIFY_USER_INFO_ENTRY_SECTION_BREAK", "section-break" },
    { C_ENUM(PURPLE_NOTIFY_USER_INFO_ENTRY_SECTION_HEADER), "PURPLE_NOTIFY_USER_INFO_ENTRY_SECTION_HEADER", "section-header" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleNotifyUserInfoEntryType"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "purplechatuser.h" */

GType
purple_chat_user_flags_get_type (void)
{
static gsize gtype_id = 0;
static const GFlagsValue values[] = {
    { C_FLAGS(PURPLE_CHAT_USER_NONE), "PURPLE_CHAT_USER_NONE", "none" },
    { C_FLAGS(PURPLE_CHAT_USER_VOICE), "PURPLE_CHAT_USER_VOICE", "voice" },
    { C_FLAGS(PURPLE_CHAT_USER_HALFOP), "PURPLE_CHAT_USER_HALFOP", "halfop" },
    { C_FLAGS(PURPLE_CHAT_USER_OP), "PURPLE_CHAT_USER_OP", "op" },
    { C_FLAGS(PURPLE_CHAT_USER_FOUNDER), "PURPLE_CHAT_USER_FOUNDER", "founder" },
    { C_FLAGS(PURPLE_CHAT_USER_TYPING), "PURPLE_CHAT_USER_TYPING", "typing" },
    { C_FLAGS(PURPLE_CHAT_USER_AWAY), "PURPLE_CHAT_USER_AWAY", "away" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_flags_register_static (g_intern_static_string ("PurpleChatUserFlags"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "purpleconnectionerrorinfo.h" */

GType
purple_connection_error_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_CONNECTION_ERROR_NETWORK_ERROR), "PURPLE_CONNECTION_ERROR_NETWORK_ERROR", "network-error" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_INVALID_USERNAME), "PURPLE_CONNECTION_ERROR_INVALID_USERNAME", "invalid-username" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED), "PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED", "authentication-failed" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_AUTHENTICATION_IMPOSSIBLE), "PURPLE_CONNECTION_ERROR_AUTHENTICATION_IMPOSSIBLE", "authentication-impossible" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_NO_SSL_SUPPORT), "PURPLE_CONNECTION_ERROR_NO_SSL_SUPPORT", "no-ssl-support" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_ENCRYPTION_ERROR), "PURPLE_CONNECTION_ERROR_ENCRYPTION_ERROR", "encryption-error" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_NAME_IN_USE), "PURPLE_CONNECTION_ERROR_NAME_IN_USE", "name-in-use" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_INVALID_SETTINGS), "PURPLE_CONNECTION_ERROR_INVALID_SETTINGS", "invalid-settings" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_CERT_NOT_PROVIDED), "PURPLE_CONNECTION_ERROR_CERT_NOT_PROVIDED", "cert-not-provided" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_CERT_UNTRUSTED), "PURPLE_CONNECTION_ERROR_CERT_UNTRUSTED", "cert-untrusted" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_CERT_EXPIRED), "PURPLE_CONNECTION_ERROR_CERT_EXPIRED", "cert-expired" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_CERT_NOT_ACTIVATED), "PURPLE_CONNECTION_ERROR_CERT_NOT_ACTIVATED", "cert-not-activated" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_CERT_HOSTNAME_MISMATCH), "PURPLE_CONNECTION_ERROR_CERT_HOSTNAME_MISMATCH", "cert-hostname-mismatch" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_CERT_FINGERPRINT_MISMATCH), "PURPLE_CONNECTION_ERROR_CERT_FINGERPRINT_MISMATCH", "cert-fingerprint-mismatch" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_CERT_SELF_SIGNED), "PURPLE_CONNECTION_ERROR_CERT_SELF_SIGNED", "cert-self-signed" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_CERT_OTHER_ERROR), "PURPLE_CONNECTION_ERROR_CERT_OTHER_ERROR", "cert-other-error" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_CUSTOM_TEMPORARY), "PURPLE_CONNECTION_ERROR_CUSTOM_TEMPORARY", "custom-temporary" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_CUSTOM_FATAL), "PURPLE_CONNECTION_ERROR_CUSTOM_FATAL", "custom-fatal" },
    { C_ENUM(PURPLE_CONNECTION_ERROR_OTHER_ERROR), "PURPLE_CONNECTION_ERROR_OTHER_ERROR", "other-error" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleConnectionError"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "purpleconversation.h" */

GType
purple_conversation_update_type_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_CONVERSATION_UPDATE_ADD), "PURPLE_CONVERSATION_UPDATE_ADD", "update-add" },
    { C_ENUM(PURPLE_CONVERSATION_UPDATE_REMOVE), "PURPLE_CONVERSATION_UPDATE_REMOVE", "update-remove" },
    { C_ENUM(PURPLE_CONVERSATION_UPDATE_ACCOUNT), "PURPLE_CONVERSATION_UPDATE_ACCOUNT", "update-account" },
    { C_ENUM(PURPLE_CONVERSATION_UPDATE_TYPING), "PURPLE_CONVERSATION_UPDATE_TYPING", "update-typing" },
    { C_ENUM(PURPLE_CONVERSATION_UPDATE_UNSEEN), "PURPLE_CONVERSATION_UPDATE_UNSEEN", "update-unseen" },
    { C_ENUM(PURPLE_CONVERSATION_UPDATE_LOGGING), "PURPLE_CONVERSATION_UPDATE_LOGGING", "update-logging" },
    { C_ENUM(PURPLE_CONVERSATION_UPDATE_TOPIC), "PURPLE_CONVERSATION_UPDATE_TOPIC", "update-topic" },
    { C_ENUM(PURPLE_CONVERSATION_ACCOUNT_ONLINE), "PURPLE_CONVERSATION_ACCOUNT_ONLINE", "account-online" },
    { C_ENUM(PURPLE_CONVERSATION_ACCOUNT_OFFLINE), "PURPLE_CONVERSATION_ACCOUNT_OFFLINE", "account-offline" },
    { C_ENUM(PURPLE_CONVERSATION_UPDATE_AWAY), "PURPLE_CONVERSATION_UPDATE_AWAY", "update-away" },
    { C_ENUM(PURPLE_CONVERSATION_UPDATE_ICON), "PURPLE_CONVERSATION_UPDATE_ICON", "update-icon" },
    { C_ENUM(PURPLE_CONVERSATION_UPDATE_NAME), "PURPLE_CONVERSATION_UPDATE_NAME", "update-name" },
    { C_ENUM(PURPLE_CONVERSATION_UPDATE_TITLE), "PURPLE_CONVERSATION_UPDATE_TITLE", "update-title" },
    { C_ENUM(PURPLE_CONVERSATION_UPDATE_CHATLEFT), "PURPLE_CONVERSATION_UPDATE_CHATLEFT", "update-chatleft" },
    { C_ENUM(PURPLE_CONVERSATION_UPDATE_FEATURES), "PURPLE_CONVERSATION_UPDATE_FEATURES", "update-features" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleConversationUpdateType"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "purpleimconversation.h" */

GType
purple_im_typing_state_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_IM_NOT_TYPING), "PURPLE_IM_NOT_TYPING", "not-typing" },
    { C_ENUM(PURPLE_IM_TYPING), "PURPLE_IM_TYPING", "typing" },
    { C_ENUM(PURPLE_IM_TYPED), "PURPLE_IM_TYPED", "typed" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleIMTypingState"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "purplemessage.h" */

GType
purple_message_flags_get_type (void)
{
static gsize gtype_id = 0;
static const GFlagsValue values[] = {
    { C_FLAGS(PURPLE_MESSAGE_SEND), "PURPLE_MESSAGE_SEND", "send" },
    { C_FLAGS(PURPLE_MESSAGE_RECV), "PURPLE_MESSAGE_RECV", "recv" },
    { C_FLAGS(PURPLE_MESSAGE_SYSTEM), "PURPLE_MESSAGE_SYSTEM", "system" },
    { C_FLAGS(PURPLE_MESSAGE_AUTO_RESP), "PURPLE_MESSAGE_AUTO_RESP", "auto-resp" },
    { C_FLAGS(PURPLE_MESSAGE_ACTIVE_ONLY), "PURPLE_MESSAGE_ACTIVE_ONLY", "active-only" },
    { C_FLAGS(PURPLE_MESSAGE_NICK), "PURPLE_MESSAGE_NICK", "nick" },
    { C_FLAGS(PURPLE_MESSAGE_NO_LOG), "PURPLE_MESSAGE_NO_LOG", "no-log" },
    { C_FLAGS(PURPLE_MESSAGE_ERROR), "PURPLE_MESSAGE_ERROR", "error" },
    { C_FLAGS(PURPLE_MESSAGE_DELAYED), "PURPLE_MESSAGE_DELAYED", "delayed" },
    { C_FLAGS(PURPLE_MESSAGE_RAW), "PURPLE_MESSAGE_RAW", "raw" },
    { C_FLAGS(PURPLE_MESSAGE_IMAGES), "PURPLE_MESSAGE_IMAGES", "images" },
    { C_FLAGS(PURPLE_MESSAGE_NOTIFY), "PURPLE_MESSAGE_NOTIFY", "notify" },
    { C_FLAGS(PURPLE_MESSAGE_NO_LINKIFY), "PURPLE_MESSAGE_NO_LINKIFY", "no-linkify" },
    { C_FLAGS(PURPLE_MESSAGE_INVISIBLE), "PURPLE_MESSAGE_INVISIBLE", "invisible" },
    { C_FLAGS(PURPLE_MESSAGE_REMOTE_SEND), "PURPLE_MESSAGE_REMOTE_SEND", "remote-send" },
    { C_FLAGS(PURPLE_MESSAGE_FORWARDED), "PURPLE_MESSAGE_FORWARDED", "forwarded" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_flags_register_static (g_intern_static_string ("PurpleMessageFlags"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

GType
purple_message_content_type_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_MESSAGE_CONTENT_TYPE_PLAIN), "PURPLE_MESSAGE_CONTENT_TYPE_PLAIN", "plain" },
    { C_ENUM(PURPLE_MESSAGE_CONTENT_TYPE_HTML), "PURPLE_MESSAGE_CONTENT_TYPE_HTML", "html" },
    { C_ENUM(PURPLE_MESSAGE_CONTENT_TYPE_XHTML), "PURPLE_MESSAGE_CONTENT_TYPE_XHTML", "xhtml" },
    { C_ENUM(PURPLE_MESSAGE_CONTENT_TYPE_MARKDOWN), "PURPLE_MESSAGE_CONTENT_TYPE_MARKDOWN", "markdown" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleMessageContentType"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "purplenotification.h" */

GType
purple_notification_type_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_NOTIFICATION_TYPE_UNKNOWN), "PURPLE_NOTIFICATION_TYPE_UNKNOWN", "unknown" },
    { C_ENUM(PURPLE_NOTIFICATION_TYPE_GENERIC), "PURPLE_NOTIFICATION_TYPE_GENERIC", "generic" },
    { C_ENUM(PURPLE_NOTIFICATION_TYPE_CONNECTION_ERROR), "PURPLE_NOTIFICATION_TYPE_CONNECTION_ERROR", "connection-error" },
    { C_ENUM(PURPLE_NOTIFICATION_TYPE_AUTHORIZATION_REQUEST), "PURPLE_NOTIFICATION_TYPE_AUTHORIZATION_REQUEST", "authorization-request" },
    { C_ENUM(PURPLE_NOTIFICATION_TYPE_ADD_CONTACT), "PURPLE_NOTIFICATION_TYPE_ADD_CONTACT", "add-contact" },
    { C_ENUM(PURPLE_NOTIFICATION_TYPE_FILE_TRANSFER), "PURPLE_NOTIFICATION_TYPE_FILE_TRANSFER", "file-transfer" },
    { C_ENUM(PURPLE_NOTIFICATION_TYPE_CHAT_INVITE), "PURPLE_NOTIFICATION_TYPE_CHAT_INVITE", "chat-invite" },
    { C_ENUM(PURPLE_NOTIFICATION_TYPE_MENTION), "PURPLE_NOTIFICATION_TYPE_MENTION", "mention" },
    { C_ENUM(PURPLE_NOTIFICATION_TYPE_REACTION), "PURPLE_NOTIFICATION_TYPE_REACTION", "reaction" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleNotificationType"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "purpleplugininfo.h" */

GType
purple_plugin_info_flags_get_type (void)
{
static gsize gtype_id = 0;
static const GFlagsValue values[] = {
    { C_FLAGS(PURPLE_PLUGIN_INFO_FLAGS_INTERNAL), "PURPLE_PLUGIN_INFO_FLAGS_INTERNAL", "internal" },
    { C_FLAGS(PURPLE_PLUGIN_INFO_FLAGS_AUTO_LOAD), "PURPLE_PLUGIN_INFO_FLAGS_AUTO_LOAD", "auto-load" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_flags_register_static (g_intern_static_string ("PurplePluginInfoFlags"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "purpleprotocol.h" */

GType
purple_protocol_options_get_type (void)
{
static gsize gtype_id = 0;
static const GFlagsValue values[] = {
    { C_FLAGS(OPT_PROTO_UNIQUE_CHATNAME), "OPT_PROTO_UNIQUE_CHATNAME", "unique-chatname" },
    { C_FLAGS(OPT_PROTO_CHAT_TOPIC), "OPT_PROTO_CHAT_TOPIC", "chat-topic" },
    { C_FLAGS(OPT_PROTO_NO_PASSWORD), "OPT_PROTO_NO_PASSWORD", "no-password" },
    { C_FLAGS(OPT_PROTO_MAIL_CHECK), "OPT_PROTO_MAIL_CHECK", "mail-check" },
    { C_FLAGS(OPT_PROTO_PASSWORD_OPTIONAL), "OPT_PROTO_PASSWORD_OPTIONAL", "password-optional" },
    { C_FLAGS(OPT_PROTO_USE_POINTSIZE), "OPT_PROTO_USE_POINTSIZE", "use-pointsize" },
    { C_FLAGS(OPT_PROTO_REGISTER_NOSCREENNAME), "OPT_PROTO_REGISTER_NOSCREENNAME", "register-noscreenname" },
    { C_FLAGS(OPT_PROTO_SLASH_COMMANDS_NATIVE), "OPT_PROTO_SLASH_COMMANDS_NATIVE", "slash-commands-native" },
    { C_FLAGS(OPT_PROTO_INVITE_MESSAGE), "OPT_PROTO_INVITE_MESSAGE", "invite-message" },
    { C_FLAGS(OPT_PROTO_AUTHORIZATION_GRANTED_MESSAGE), "OPT_PROTO_AUTHORIZATION_GRANTED_MESSAGE", "authorization-granted-message" },
    { C_FLAGS(OPT_PROTO_AUTHORIZATION_DENIED_MESSAGE), "OPT_PROTO_AUTHORIZATION_DENIED_MESSAGE", "authorization-denied-message" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_flags_register_static (g_intern_static_string ("PurpleProtocolOptions"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "purpleproxyinfo.h" */

GType
purple_proxy_type_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_PROXY_TYPE_USE_GLOBAL), "PURPLE_PROXY_TYPE_USE_GLOBAL", "use-global" },
    { C_ENUM(PURPLE_PROXY_TYPE_NONE), "PURPLE_PROXY_TYPE_NONE", "none" },
    { C_ENUM(PURPLE_PROXY_TYPE_HTTP), "PURPLE_PROXY_TYPE_HTTP", "http" },
    { C_ENUM(PURPLE_PROXY_TYPE_SOCKS4), "PURPLE_PROXY_TYPE_SOCKS4", "socks4" },
    { C_ENUM(PURPLE_PROXY_TYPE_SOCKS5), "PURPLE_PROXY_TYPE_SOCKS5", "socks5" },
    { C_ENUM(PURPLE_PROXY_TYPE_USE_ENVVAR), "PURPLE_PROXY_TYPE_USE_ENVVAR", "use-envvar" },
    { C_ENUM(PURPLE_PROXY_TYPE_TOR), "PURPLE_PROXY_TYPE_TOR", "tor" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleProxyType"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "roomlist.h" */

GType
purple_roomlist_field_type_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_ROOMLIST_FIELD_BOOL), "PURPLE_ROOMLIST_FIELD_BOOL", "bool" },
    { C_ENUM(PURPLE_ROOMLIST_FIELD_INT), "PURPLE_ROOMLIST_FIELD_INT", "int" },
    { C_ENUM(PURPLE_ROOMLIST_FIELD_STRING), "PURPLE_ROOMLIST_FIELD_STRING", "string" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleRoomlistFieldType"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "status.h" */

GType
purple_status_primitive_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_STATUS_UNSET), "PURPLE_STATUS_UNSET", "unset" },
    { C_ENUM(PURPLE_STATUS_OFFLINE), "PURPLE_STATUS_OFFLINE", "offline" },
    { C_ENUM(PURPLE_STATUS_AVAILABLE), "PURPLE_STATUS_AVAILABLE", "available" },
    { C_ENUM(PURPLE_STATUS_UNAVAILABLE), "PURPLE_STATUS_UNAVAILABLE", "unavailable" },
    { C_ENUM(PURPLE_STATUS_INVISIBLE), "PURPLE_STATUS_INVISIBLE", "invisible" },
    { C_ENUM(PURPLE_STATUS_AWAY), "PURPLE_STATUS_AWAY", "away" },
    { C_ENUM(PURPLE_STATUS_EXTENDED_AWAY), "PURPLE_STATUS_EXTENDED_AWAY", "extended-away" },
    { C_ENUM(PURPLE_STATUS_MOBILE), "PURPLE_STATUS_MOBILE", "mobile" },
    { C_ENUM(PURPLE_STATUS_TUNE), "PURPLE_STATUS_TUNE", "tune" },
    { C_ENUM(PURPLE_STATUS_MOOD), "PURPLE_STATUS_MOOD", "mood" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleStatusPrimitive"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "xfer.h" */

GType
purple_xfer_type_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_XFER_TYPE_UNKNOWN), "PURPLE_XFER_TYPE_UNKNOWN", "unknown" },
    { C_ENUM(PURPLE_XFER_TYPE_SEND), "PURPLE_XFER_TYPE_SEND", "send" },
    { C_ENUM(PURPLE_XFER_TYPE_RECEIVE), "PURPLE_XFER_TYPE_RECEIVE", "receive" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleXferType"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

GType
purple_xfer_status_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_XFER_STATUS_UNKNOWN), "PURPLE_XFER_STATUS_UNKNOWN", "unknown" },
    { C_ENUM(PURPLE_XFER_STATUS_NOT_STARTED), "PURPLE_XFER_STATUS_NOT_STARTED", "not-started" },
    { C_ENUM(PURPLE_XFER_STATUS_ACCEPTED), "PURPLE_XFER_STATUS_ACCEPTED", "accepted" },
    { C_ENUM(PURPLE_XFER_STATUS_STARTED), "PURPLE_XFER_STATUS_STARTED", "started" },
    { C_ENUM(PURPLE_XFER_STATUS_DONE), "PURPLE_XFER_STATUS_DONE", "done" },
    { C_ENUM(PURPLE_XFER_STATUS_CANCEL_LOCAL), "PURPLE_XFER_STATUS_CANCEL_LOCAL", "cancel-local" },
    { C_ENUM(PURPLE_XFER_STATUS_CANCEL_REMOTE), "PURPLE_XFER_STATUS_CANCEL_REMOTE", "cancel-remote" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleXferStatus"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* enumerations from "xmlnode.h" */

GType
purple_xml_node_type_get_type (void)
{
static gsize gtype_id = 0;
static const GEnumValue values[] = {
    { C_ENUM(PURPLE_XMLNODE_TYPE_TAG), "PURPLE_XMLNODE_TYPE_TAG", "tag" },
    { C_ENUM(PURPLE_XMLNODE_TYPE_ATTRIB), "PURPLE_XMLNODE_TYPE_ATTRIB", "attrib" },
    { C_ENUM(PURPLE_XMLNODE_TYPE_DATA), "PURPLE_XMLNODE_TYPE_DATA", "data" },
{ 0, NULL, NULL }
        };
        if (g_once_init_enter (&gtype_id)) {
            GType new_type = g_enum_register_static (g_intern_static_string ("PurpleXmlNodeType"), values);
            g_once_init_leave (&gtype_id, new_type);
        }
        return (GType) gtype_id;
        }

/* Generated data ends here */

