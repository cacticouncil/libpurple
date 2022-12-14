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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_EVENTLOOP_H
#define PURPLE_EVENTLOOP_H

#include <glib.h>
#include <glib-object.h>

/**
 * PurpleInputCondition:
 * @PURPLE_INPUT_READ:  A read condition.
 * @PURPLE_INPUT_WRITE: A write condition.
 *
 * An input condition.
 */
typedef enum
{
	PURPLE_INPUT_READ  = 1 << 0,
	PURPLE_INPUT_WRITE = 1 << 1

} PurpleInputCondition;

/**
 * PurpleInputFunction:
 * @data: User specified data.
 * @fd: The input file descriptor.
 * @cond: The condition type.
 *
 * The type of callbacks to handle events on file descriptors, as passed to
 * purple_input_add().  The callback will receive the @user_data passed to
 * purple_input_add(), the file descriptor on which the event occurred, and the
 * condition that was satisfied to cause the callback to be invoked.
 */
typedef void (*PurpleInputFunction)(gpointer data, gint fd, PurpleInputCondition cond);

G_BEGIN_DECLS

/**************************************************************************/
/* Event Loop API                                                         */
/**************************************************************************/

/**
 * purple_input_add:
 * @fd:        The input file descriptor.
 * @cond:      The condition type.
 * @func:      (scope call): The callback function for data.
 * @user_data: User-specified data.
 *
 * Adds an input handler.
 *
 * See g_io_add_watch_full().
 *
 * Returns: The resulting handle (will be greater than 0).
 */
guint purple_input_add(int fd, PurpleInputCondition cond,
                       PurpleInputFunction func, gpointer user_data);

/**
 * purple_input_pipe:
 * @pipefd: Array used to return file descriptors for both ends of pipe.
 *
 * Creates a pipe - an unidirectional data channel that can be used for
 * interprocess communication.
 *
 * File descriptors for both ends of pipe will be written into provided array.
 * The first one (pipefd[0]) can be used for reading, the second one (pipefd[1])
 * for writing.
 *
 * On Windows it's simulated by creating a pair of connected sockets, on other
 * systems pipe() is used.
 *
 * Returns: 0 on success, -1 on error.
 */
int
purple_input_pipe(int pipefd[2]);

G_END_DECLS

#endif /* PURPLE_EVENTLOOP_H */
