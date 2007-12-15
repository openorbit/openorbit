/* Open Orbit - An interactive spacecraft simulator.
 * Copyright (C) 2006  Mattias Holm (mattias.holm@contra.nu)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Linking Open Orbit statically or dynamically with other modules is making a
 * combined work based on Open Orbit.  Thus, the terms and conditions of the
 * GNU General Public License cover the whole combination.
 *
 * As a special exception, the copyright holders of Open Orbit give you
 * permission to link Open Orbit with independent modules that communicate with
 * Open Orbit solely through the public interfaces (defined as such in the
 * documentation), regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under terms
 * of your choice, provided that every copy of the combined work is accompanied
 * by a complete copy of the source code of Open Orbit (the version of Open
 * Orbit used to produce the combined work), being distributed under the terms
 * of the GNU General Public License plus this exception. An independent module
 * is a module which is not derived from or based on Open Orbit.
 *
 * Note that people who make modified versions of Open Orbit are not obligated
 * to grant this special exception for their modified versions; it is
 * their choice whether to do so.  The GNU General Public License gives
 * permission to release a modified version without this exception; this
 * exception also makes it possible to release a modified version which
 * carries forward this exception.
 */

 
#ifndef __ERROR_H__
#define __ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	ERROR_NONE = 0,
	ERROR_FILE_NOT_FOUND,
	ERROR_INVALID_PARAMS,
	ERROR_MEM,
	ERROR_NET,
    ERROR_SCRIPT
} oo_error_t;

char *get_error_string(oo_error_t err);
void print_error_message(oo_error_t err);


#ifdef __cplusplus
}
#endif

#endif /* ! __ERROR_H__ */
