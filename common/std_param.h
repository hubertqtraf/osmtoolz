/******************************************************************
 *
 * @short	common cmd parameters
 *
 * project:	Trafalgar/OSM-Toolz
 *
 * modul:	std_param.h	header for parameters
 * @version	0.1
 *
 * system:	UNIX/LINUX
 * compiler:	gcc
 *
 * @author	Schmid Hubert (C)2025-2025
 *
 * beginning:	04.2025
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

#ifndef STD_PARAM_H
#define STD_PARAM_H

#include <stdint.h>

#define COOR_FAC 10000000.0

#define PARM_FILE 0x00000001
#define PARM_RECT 0x00000002
#define PARM_OUT  0x00000004
#define PARM_ACC  0x00000008
#define PARM_MODE 0x00000010
#define PARM_TR   0x00000100

#define F_INFO    1
#define F_NODE    2
#define F_WAY     3
#define F_REL     4
#define F_POINT   5

#define DIR_IN    0
#define DIR_OUT   1

typedef struct
{
	uint32_t flags;
	int32_t rect[4];
	uint8_t val_thread;
	uint8_t val_accept;
	uint8_t val_mode;
	char * in_fname;
	char * out_fname;
	char * buff;
}StdParam;

int read_param(StdParam * param, int argc, char ** argv);
void set_output(StdParam * param, char * name);
char * get_fname(StdParam * param, int dir, int mode);
void free_param(StdParam * param);

#endif

