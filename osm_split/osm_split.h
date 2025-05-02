/******************************************************************
 * project:     Trafalgar/OSM-Toolz
 *
 * class:       ---
 * superclass:  ---
 * modul:       osm_split.h
 *
 * system:      UNIX/LINUX
 * compiler:    gcc
 *
 * beginning:   11.2012
 *
 * (C)          Schmid Hubert 2012-2025
 *
 * history:
 *
 ******************************************************************/

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

#ifndef _OSM_SPLIT_H
#define _OSM_SPLIT_H

#include "../common/zblock_io.h"
#include "../common/osm_info.h"
#include "../common/std_param.h"

#include <stdint.h>

#define INSIDE_NODE 0x01
#define INSIDE_WAY  0x02
#define INSIDE_SEG  0x04
#define INSIDE_REL  0x08

#define ND_ID 	0x10
#define ND_COOR	0x20
#define ND_SEG  0x40
#define ND_LON	0x80

#define WAY_ID   	0x04
#define WAY_NODE	0x01
#define WAY_NODE_REF	0x02

// bit 0  to 41 id	42 bit
#define ID_64_BASE  0x000003ffffffffff
// bit 42 to 50 lon	9  bit
#define ID_64_LON   0x0007fC0000000000
// bit 51 to 59 lat	9  bit
#define ID_64_LAT   0x0ff8000000000000
// bit 60 to 62 flags	3  bit
#define ID_64_FLAGS 0x7000000000000000
// bit 63       reserved

// 42 + 9 + 9 + 3 + 1 = 64

//#define SECTIONS 36

typedef struct
{
	uint8_t n_64_mode;

	uint8_t type_flags;
	uint8_t key_flags;
	uint8_t val_flags;
	uint8_t seg_flags;
	uint64_t id;
	uint32_t lat;
	uint32_t lon;

	OsmInfo_t info;

	char save_tag[10];

	char lat_buf[100];
	char id_buf[100];

	z_block zw_node;
	z_block zw_way;
	z_block zw_rel;
	//uint64_t zw_node_n[SECTIONS];

	uint8_t * world_lon;
	uint8_t way_min_lon;
	uint8_t way_max_lon;
	z_block zw_box;

	uint8_t * id64_nd_lat;
	uint8_t * id64_nd_lon;
	uint8_t * id64_nd_lat_lon_bit9;
	uint64_t store_id;
	uint64_t store_x;
	uint64_t store_y;

	uint64_t count_nd;
	uint64_t count_member;

	int accept;
}World_t;

int countNodes(z_block * z_read, World_t * act_node, uint8_t n_64_mode, uint8_t flags);

int countNodesOut(z_block * z_read, World_t * act_node, char * out, StdParam * param, uint8_t flags);

int readNodes(z_block * z_read, World_t * act_world);

#endif // _OSM_SPLIT_H


