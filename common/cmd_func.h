/******************************************************************
 *
 * @short	helper functions
 *
 * project:	Trafalgar/OSM-Toolz
 *
 * modul:	cmd_func.h	header of helper functions
 * @version	0.1
 *
 * system:	UNIX/LINUX
 * compiler:	gcc
 *
 * @author	Schmid Hubert (C)2020-2025
 *
 * beginning:	02.2020
 *
 * history:
 */
/******************************************************************/

/* The trafalgar package is free software.  You may redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software foundation; either version 2, or (at your
   option) any later version.

   The GNU trafalgar package is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with the trafalgar package; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin St., Fifth Floor,
   Boston, MA 02110-1301, USA. */

#ifndef _CMD_FUNC_H
#define _CMD_FUNC_H

#include <time.h>

int cmd_args(int argc, char ** argv, char * opt);

char * cmd_get_path(char * where);

int cmd_get_fname(char ** result, char * path, char * name, char * type);

int cmd_get_rect(char * param, double rect[4]);

int cmd_time(time_t t1, char * buffer);

#endif // _CMD_FUNC_H

