/**
 * @file roster.h Roster manipulation
 *
 * purple
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

#ifndef PURPLE_JABBER_ROSTER_H
#define PURPLE_JABBER_ROSTER_H

/* it must *not* be localized */
#define JABBER_ROSTER_DEFAULT_GROUP "Buddies"

#include "jabber.h"

void jabber_roster_request(JabberStream *js);

void jabber_roster_parse(JabberStream *js, const char *from,
                         JabberIqType type, const char *id, PurpleXmlNode *query);

void jabber_roster_add_buddy(PurpleProtocolServer *protocol_server, PurpleConnection *gc, PurpleBuddy *buddy,
		PurpleGroup *group, const gchar *message);
void jabber_roster_alias_change(PurpleProtocolServer *protocol_server, PurpleConnection *gc, const gchar *name,
		const gchar *alias);
void jabber_roster_group_change(PurpleProtocolServer *protocol_server, PurpleConnection *gc, const gchar *name,
		const gchar *old_group, const gchar *new_group);
void jabber_roster_group_rename(PurpleProtocolServer *protocol_server, PurpleConnection *gc, const gchar *old_name,
		PurpleGroup *group, GList *moved_buddies);
void jabber_roster_remove_buddy(PurpleProtocolServer *protocol_server, PurpleConnection *gc, PurpleBuddy *buddy,
		PurpleGroup *group);

#endif /* PURPLE_JABBER_ROSTER_H */
