/******************************************************************
 * project:     Trafalgar/OSM-Toolz
 *
 * modul:      	osm_huge.h
 *
 * system:      UNIX/LINUX
 * compiler:    gcc
 *
 * beginning:   04.2025
 *
 * (C)          Schmid Hubert 2025-2025
 *
 * history:
 *
 ******************************************************************/

/* The trafalgar package is free software.  You may redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software foundation; either version 2, or (at your
 * option) any later version.
 *
 * The GNU trafalgar package is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the GNU plotutils package; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin St., Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _OSM_NODE_CUT_H
#define _OSM_NODE_CUT_H

#include "../common/zblock_io.h"
#include "../common/osm_info.h"

#include <stdint.h>

#define INSIDE_NODE 0x01
#define INSIDE_WAY  0x02
#define INSIDE_SEG  0x04
#define INSIDE_REL  0x08

#define INSIDE_WAY_ND 0x20      // only for way

// key_flags
#define ND_ID 	0x10
#define ND_COOR	0x20
#define ND_SEG  0x40
#define ND_LON	0x80

#define WAY_ID   	0x04
#define WAY_NODE	0x01
#define WAY_NODE_REF	0x02
#define WAY_K		0x08
#define WAY_V		0x10

#define WAY_REF  0x08	// only for way

#define ID_MASK     0x0fffffffffffffff
#define COOR_FLAG   0x2000000000000000
#define UNREF_FLAG  0x4000000000000000
#define INSIDE_FLAG 0x8000000000000000

#define ID_40_FLAG  0x0000004000000000

// TODO check
#define STORE_COOR_24BIT     0x01
#define STORE_COOR_32BIT     0x02
// used the change coor float value to 32bit integer
#define COOR_FACTOR 10000000
// ID with 36 Bit -> 0xffffffff * 16
#define N_ID_64_BASE    0x0000000fffffffff
// lat with 24 Bit included inside the id
#define N_LAT_24_MASK   0x0ffffff000000000

// bit 42 to 50 lon     9  bit
#define N_COOR_64_LON   0x0007fC0000000000
#define N_COOR_64_LON_S 42

// bit 51 to 59 lat     9  bit
#define N_COOR_64_LAT   0x0ff8000000000000
#define N_COOR_64_LAT_S 51


typedef struct
{
	uint8_t type_flags;
	uint8_t key_flags;
	uint8_t val_flags;
	uint8_t seg_flags;
	uint64_t id;

	OsmInfo_t info;

	char save_tag[10];

	char lat_buf[100];
	char id_buf[100];

	z_block zw_node;
	z_block zw_out;

	uint8_t * node_flags;
	uint8_t * way_flags;
	uint8_t node_flags_size;

	uint64_t * node_ref_list;

	uint32_t act_member_idx;

	uint64_t act_idx;
	// TODO: remove -> info!
        uint64_t act_selected;
	double nd_lat;
	double nd_lon;
	uint64_t nd_id;
	double select_rect[4];

	uint64_t next_id;

	char * in_path;
	char * out_path;
}World_t;

void setMode(World_t * w_ref, uint8_t mode);

int readNodes(z_block * z_read, World_t * act_world, int32_t rect[4]);
int cutWays(z_block * z_read, World_t * act_world/*, int32_t rect[4]*/);
int writeNodes(z_block * z_read, World_t * act_world, uint8_t flags, char * out_path);
int writeWays(z_block * z_read, World_t * act_world, char * w_fname);

#endif // _OSM_NODE_CUT_H


