/******************************************************************
 *
 * @short	reading/write the info file
 *
 * project:	Trafalgar/OSM-Toolz
 *
 * modul:	osm_info.h	header for XML read/write
 * @version	0.1
 *
 * system:	UNIX/LINUX
 * compiler:	gcc
 *
 * @author	Schmid Hubert (C)2016-2025
 *
 * beginning:	02.2016
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

#ifndef OSM_INFO_H
#define OSM_INFO_H

#include "zblock_io.h"
#include "read_version.h"

#include <stdint.h>

//TODO: rework -> remove 'count'

typedef struct
{
	uint64_t min_id;
	uint64_t max_id;
	uint64_t n_id;			// number of members
	uint64_t i_count;		// internal counter
	uint64_t count;			// counter
	uint64_t member;		// counter for max. number of members
	char * file;
}OsmIdInfo_t;

typedef struct
{
	OsmIdInfo_t node;
	OsmIdInfo_t point;
	OsmIdInfo_t way;
	OsmIdInfo_t rel;
	OsmIdInfo_t seg;		// only v3 API
}OsmInfo_t;

void initOsmInfo(OsmInfo_t * info);

int readOsmInfo(OsmInfo_t * info, const char * fname, Version_t * version, int accept);
int writeOsmInfo(OsmInfo_t * info, const char * fname, Version_t * version);

void resetInfoPart(OsmIdInfo_t * info);
void updateInfoPart(OsmIdInfo_t * info, uint64_t act_id);

int createOsmHead(z_block * out, const char * fname, uint8_t flags);
int closeOsm(z_block * out);

#endif

