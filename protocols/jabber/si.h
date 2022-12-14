/**
 * @file si.h SI transfer functions
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

#ifndef PURPLE_JABBER_SI_H
#define PURPLE_JABBER_SI_H

#include <purple.h>

#include "jabber.h"

G_BEGIN_DECLS

#define JABBER_TYPE_SI_XFER (jabber_si_xfer_get_type())
G_DECLARE_FINAL_TYPE(JabberSIXfer, jabber_si_xfer, JABBER, SI_XFER, PurpleXfer);

void jabber_bytestreams_parse(JabberStream *js, const char *from,
                              JabberIqType type, const char *id, PurpleXmlNode *query);
PurpleXfer *jabber_si_new_xfer(PurpleProtocolXfer *prplxfer, PurpleConnection *gc, const char *who);
void jabber_si_xfer_send(PurpleProtocolXfer *prplxfer, PurpleConnection *gc, const char *who, const char *file);

void jabber_si_xfer_register(GTypeModule *module);

void jabber_si_init(void);
void jabber_si_uninit(void);

G_END_DECLS

#endif /* PURPLE_JABBER_SI_H */
