/******************************************************************
 *
 * @short	reading the version information
 *
 * project:	Trafalgar/OSM-Toolz
 *
 * modul:	read_version.h	header of osm version
 * @version	0.1
 *
 * system:	UNIX/LINUX
 * compiler:	gcc
 *
 * @author	Schmid Hubert (C)2012-2025
 *
 * beginning:	11.2012
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

#ifndef OSM_READ_VERSION_H
#define OSM_READ_VERSION_H

#include "zblock_io.h"

#include <stdint.h>

typedef struct
{
	uint32_t version;
	char * source;
	char * generator;
	char * timestamp;
	int32_t * box;
	uint8_t n_64_flags;
}Version_t;

int getVersion(char * osm_file, Version_t * version, int accept);
void cleanVersion(Version_t * version);

void addString(char ** dest, char * source, size_t len);
void addBox(int32_t ** dest, int32_t * source);

#endif

