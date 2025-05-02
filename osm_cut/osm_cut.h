/******************************************************************
 * project:	Trafalgar/OSM-Toolz/osm_cut
 *
 * modul:	osm_cut.h
 *
 * system:	UNIX/LINUX
 * compiler:	gcc
 *
 * beginning:	11.2012
 *
 * (C)		Schmid Hubert 2012-2025
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

#ifndef _OSM_CUT_H
#define _OSM_CUT_H

#include "../common/zblock_io.h"
#include "../common/osm_info.h"
#include "../common/std_param.h"

#include <stdint.h> 

#define STORE_COOR_24BIT     0x01
#define STORE_COOR_32BIT     0x02

#define INSIDE_NODE   0x01
#define INSIDE_WAY    0x02
#define INSIDE_SEG    0x04
#define INSIDE_REL    0x08

#define INSIDE_TAG_   0x10
#define INSIDE_WAY_ND 0x20	// only for way
#define INSIDE_REL_MB 0x40	// only relation

// move to own header?
#define ND_ID    0x01
#define ND_COOR  0x02
#define ND_LAT   0x04
#define ND_LON   0x08
#define ND_SEG   0x10

#define WAY_ID   0x01
#define WAY_K    0x02
#define WAY_V    0x04
#define WAY_REF  0x08

#define REL_ID   0x01
#define REL_K    0x02
#define REL_V    0x04
#define REL_REF  0x08
#define REL_ROLE 0x10
#define REL_TYPE 0x20

#define REL_MEMBER_NODE 0x01
#define REL_MEMBER_WAY  0x02
#define REL_MEMBER_REL  0x04

// used the change coor float value to 32bit integer
#define COOR_FACTOR 10000000

// is inside world rect flags
#define COOR_FLAG         0x1000000000000000
#define ND_COOR_FLAG      0x2000000000000000
#define WAY_NO_ID_FLAG    0x2000000000000000

#define WAY_ID_MASK       0x8fffffffffffffff

typedef struct
{
	uint64_t id;
	double lat;
	double lon;
}Node_t;

typedef struct
{
	uint64_t id;
	uint32_t lon1;
	uint32_t lon2;
	uint32_t lat1;
	uint32_t lat2;
}Way_t;

/*typedef struct
{
        uint8_t flags;		// TODO: bit?
}Rel_t;*/

typedef struct
{
	//uint8_t store_type;

	uint8_t inside_tag_flags;
	uint8_t key_flags;
	uint8_t rel_type_flags;	// only rel!

	uint64_t act_idx;
	// TODO: remove -> info!
	uint64_t act_selected;

	Node_t act_node;

	Way_t act_way;
	Way_t select_rect;

	uint8_t * nodes;
	uint8_t size_node;

	uint8_t * ways;
	uint8_t size_way;

	uint8_t * relations;
	uint8_t size_rel;

	uint64_t * node_ref_list;
	//int current_ref_count;
	int max_ref_count;
	int act_member_idx;

	uint64_t * member_ref_list;

	// save size
	uint64_t count_node;
	uint64_t count_way;
	uint64_t count_rel;

	//Rel_t * rel;

	char * out_path;
	z_block zw_out;
	z_block zw_point_out;

	OsmInfo_t info;

	uint64_t count_nd;
	uint64_t test_member;
	uint64_t count_member;
	uint64_t member_id;

	uint8_t nine_bit_mode;
}World_t;



/*#define INSIDE_NODE 0x01
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
#define WAY_K		0x08
#define WAY_V		0x10

#define REL_TAG         0x01
#define REL_ID          0x04
#define REL_K           0x08
#define REL_V           0x10
#define REL_MEMBER	0x02
#define REL_REF		0x40
#define REL_TYPE        0x80
#define REL_ROLE	0x20

#define REL_MEMBER_NODE 0x01
#define REL_MEMBER_WAY	0x02
#define REL_MEMBER_REL	0x04

#define ID_MASK     0x0fffffffffffffff

#define UNREF_FLAG        0x4000000000000000
#define INSIDE_FLAG       0x8000000000000000

#define NODE_STORE_TYPE_PART 0x02

// test 
//#define IDW_64_MARK  0x0000004000000000
//#define IDW_64_DLON  0x0000038000000000
//#define IDW_64_DLAT  0x00001c0000000000
//#define IDW_64_LON   0x003FE00000000000
//#define IDW_64_LAT   0x7FC0000000000000
*/
// TODO: mapping into coor to the 64 bit ID: 9Bit -> 512 -> 360

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

// bit 60 to 62 flags   3  bit
#define N_64_FLAGS      0x6700000000000000
// bit 63       reserved

// 12 Bit -> 4096 -> xxx.x (*10)
// bit 36 to 48 lon     12  bit
#define N_COOR_12_LON   0x0000fff000000000
#define N_COOR_12_LON_S 36

// bit 48 to 60 lat     12  bit
#define N_COOR_12_LAT   0x0fff000000000000
#define N_COOR_12_LAT_S 48

// 42 + 9 + 9 + 3 + 1 = 64	// -> node

// 8 + (4 * 4) -> 8 - 2 = 6 + 2 + 2 + 4 = 14
// use 2 Byte from ID -> function for x and y

// TODO test: do not link, just include the source
/*uint64_t get_node_id(uint8_t * nodes, uint8_t n_size, uint64_t id);
uint64_t get_node_lat(uint8_t * nodes, uint8_t n_size, uint64_t id);
uint32_t get_node_lon(uint8_t * nodes, uint8_t n_size, uint64_t id);*/

int readNodes(z_block * z_read, World_t * act_world, int mode, StdParam * param);
int readWays(z_block * z_read, World_t * act_world, StdParam * param);
int readRelations(z_block * z_read, World_t * act_world, StdParam * param);
int writeWays(z_block * z_read, World_t * act_world, char * w_fname, StdParam * param);
int writeNodes(z_block * z_read, World_t * act_world, char * p_fname, StdParam * param);
int writeRelations(z_block * z_read, World_t * act_world, char * w_fname, StdParam * param);

#endif // _OSM_CUT_H


