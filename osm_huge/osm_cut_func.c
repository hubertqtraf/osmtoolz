/******************************************************************
 * project:	Trafalgar/OSM-Toolz/osm_cut
 *
 * modul:	osm_cut_func.c
 *
 * system:	UNIX/LINUX
 * compiler:	gcc
 *
 * beginning:	02.2023
 *
 * (C)		Schmid Hubert 2023-2025
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


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*uint32_t inline intCoor(double coor)
{
	return ((coor + 180) * COOR_FACTOR);
}

double inline floatCoor(int coor)
{
	return ((coor/COOR_FACTOR) - 180.0);
}*/

uint64_t inline get_node_id(uint8_t * nodes, uint8_t n_size, uint64_t id)
{
	uint8_t * pos = nodes + (id * n_size);
	uint64_t * n_id = (uint64_t *)pos;
	return ((*n_id) & 0x000000fffffffff);
	//uint8_t * pos = nodes + (id * n_size);

	//uint64_t * n_id = (uint64_t *)pos;

	//return ((*n_id) & N_ID_64_BASE); // ??? 0x -8- 000000fffffffff);
	//return ((*n_id) & 0x000000fffffffff);
}

uint8_t inline get_node_flags2(uint8_t * nodes, uint8_t n_size, uint64_t id)
{
	uint8_t pos = nodes[(id * n_size) + (n_size-1)];
	return pos >> 6;
}

uint8_t inline get_node_flags(uint8_t * nodes, uint8_t n_size, uint64_t id)
{
	uint8_t * pos = nodes + (id * n_size);
	return (pos[n_size-1] & 0xf0);
}

// TODO: uint64_t???
/*uint64_t inline get_node_lat(uint8_t * nodes, uint8_t n_size, uint64_t id)
{
	uint64_t n_id = 0;

	if(n_size == 8)
	{
		memcpy(&n_id, nodes + (id * n_size), n_size);
		*//*printf(" ++ %ld ++\n", ((n_id & N_COOR_64_LAT) >> N_COOR_64_LAT_S)*//* - 180);*//*
		return(((n_id & N_COOR_64_LAT) >> N_COOR_64_LAT_S)*//* - 180*//*);
	}

	memcpy(&n_id, nodes + (id * n_size), 8);

	//uint8_t * pos = nodes + (id * n_size);

	//uint64_t * n_id = (uint64_t *)pos;

	//printf(" -- %lx %lx \n", n_id, (n_id & 0x0ffffff000000000) >> 28);

	//return ((n_id & 0x0ffffff000000000) >> (32-4));

	// cut 8 Bit and move 36 Bit -> 28
	return ((n_id & N_LAT_24_MASK) >> 28);
}

uint32_t inline get_node_lon(uint8_t * nodes, uint8_t n_size, uint64_t id)
{
	if(n_size == 8)
	{
		uint64_t n_id = 0;
		memcpy(&n_id, nodes + (id * n_size), n_size);
		return(((n_id & N_COOR_64_LAT) >> N_COOR_64_LAT_S)*//* - 180*//*);
	}

	uint32_t lon_32;
	memcpy(&lon_32, nodes + 8 + (id * n_size), 4);

	//printf("(%x)",lon_32 << 8);

	return (lon_32 << 8);
}*/

int64_t inline findNode(uint8_t * nodes, uint8_t n_size, uint64_t n_node, uint64_t id)
{
	int64_t pos1 = 0;
	int64_t pos2 = n_node-1;
	int64_t d;

	//while((nodes[pos2].id & ID_MASK) != id)
	while(get_node_id(nodes, n_size, pos2) != id)
	{
		if(get_node_id(nodes, n_size, pos1) == id)
			return pos1;

		d = ((pos2-pos1) >> 1);

		if(d == 0)
			return -1;            // return 'not found'

		if(id < get_node_id(nodes, n_size, pos1 + d))
		//if(id < (nodes[pos1+d].id  & ID_MASK))
			pos2 -= d;
		else
			pos1 += d;
	}
	return pos2;
}

/*int inline select_node(World_t * world, uint64_t node_ref, uint64_t flag)
{
	int64_t pos = findNode(world->nodes, world->size_node,
	world->info.node.count, node_ref);

	uint8_t * nd_pos = world->nodes + (world->size_node * pos);
	uint64_t * node = (uint64_t *)nd_pos;

	(*node) |= flag; //0x1000000000000000;
	return 0;
}

bool inline isInsidePoint(World_t * world, uint32_t lon, uint32_t lat)
{
	return ((lon > world->select_rect.lon1) &&
		(lon < world->select_rect.lon2) &&
		(lat > world->select_rect.lat1) &&
		(lat < world->select_rect.lat2));
}

bool inline isInsideRect(World_t * world)
{
	return ((world->act_way.lon2 > world->select_rect.lon1) &&
                (world->act_way.lon1 < world->select_rect.lon2) &&
                (world->act_way.lat2 > world->select_rect.lat1) &&
                (world->act_way.lat1 < world->select_rect.lat2));
}*/
