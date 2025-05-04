/******************************************************************
 * project:	Trafalgar/OSM-Toolz/osm_cut
 *
 * modul:	osm_cut_node.c
 *
 * system:	UNIX/LINUX
 * compiler:	gcc
 *
 * beginning:	12.2012
 *
 * (C)		Schmid Hubert 2012-2025
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

#include "osm_cut.h"
#include "../common/simple_sax.h"
#include "../common/osm_info.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h> 

#include "osm_cut_func.c"

void init_world_ref(World_t * w_ref, int mode)
{
	uint64_t node_count = 0;
	//initOsmInfo(&(w_ref->info));

	if(mode)	// read points
	{
		w_ref->inside_tag_flags = 0x00;
		w_ref->key_flags = 0x00;

		w_ref->act_idx = 0;

		//node_count = w_ref->info.point.count;
		if(w_ref->nodes != NULL)
			free(w_ref->nodes);
		w_ref->nodes = NULL;
		// fixed, check needed: TODO: memmory fault, info.point.count != info.node.count
		node_count = w_ref->info.point.count;
		if(w_ref->info.node.count == 0)
			return;
		resetInfoPart(&w_ref->info.point);
	}
	else		// read nodes
	{
		node_count = w_ref->info.node.count;
		if(w_ref->info.node.count == 0)
			return;
	}

	if(node_count == 0)
	{
		fprintf(stderr, "node/point count is '0'\n");
		exit (-1);
	}
	if((w_ref->nodes = (uint8_t *)malloc(w_ref->size_node *
			(node_count +1))) == NULL)
	{
		fprintf(stderr, "unable to allocate node buffer %i %li %li\n",
			w_ref->size_node, node_count + 1,
			w_ref->size_node * (node_count + 1));
		exit (-1);
	}
	memset(w_ref->nodes, 0x00, (w_ref->size_node * node_count) + 1);
	// TODO: try int mode: id 34Bit + (2*9) Bit = 52Bit
	// 7 Byte *8 Bit = 56Bit - 52Bit
}


int load_node_tag_open(struct _simple_sax * sax)
{
	return 0;
}


int inline close_node(World_t * world)
{
	world->inside_tag_flags  &= ~(INSIDE_NODE);

	world->key_flags = 0;

	if(!world->nine_bit_mode)	// 24 bit mode
	{
		uint64_t lat_64 = (uint64_t)(intCoor(world->act_node.lat));
		lat_64 >>= 8;
		lat_64 <<= (32+4);
		lat_64 |= world->act_node.id;
		//printf("I %s %i | %lx %lx %lx\n", __FILE__, __LINE__,world->act_node.id, lat_64, world->act_node.id | lat_64);

		if(isInsidePoint(world, intCoor(world->act_node.lon), intCoor(world->act_node.lat)))
		{
			lat_64 |= ND_COOR_FLAG;
		}
		memcpy(world->nodes + (world->act_idx * world->size_node), &lat_64, 8);

		uint32_t lon_32 = intCoor(world->act_node.lon);
		lon_32 >>= 8;

		memcpy(world->nodes + 8 + (world->act_idx * world->size_node), &lon_32, 3);
		//printf("I %s %i | %lx %lx %x\n", __FILE__, __LINE__,*((uint64_t *)world->nodes), *((uint64_t *)(world->nodes+8)), lon_32);
	}
	else		// 9 Bit -> 512 > *360* > 256
	{
		// bit 42 to 50 lon     9  bit
		// bit 51 to 59 lat     9  bit

		printf("%ld %f %f\n", world->act_node.id, world->act_node.lon, world->act_node.lat);
		uint64_t nd64 = (uint64_t)(world->act_node.lon + 180);
		nd64 = (nd64 << N_COOR_64_LON_S) & N_COOR_64_LON;
		uint64_t lat_64 = (world->act_node.lat + 180);
		lat_64 = (lat_64 << N_COOR_64_LAT_S) & N_COOR_64_LAT;
		nd64 |= lat_64;
		nd64 |= (world->act_node.id & N_ID_64_BASE);

		/*printf("%lx %ld %ld %ld\n", nd64, nd64 & N_ID_64_BASE,
				((nd64 & N_COOR_64_LON) >> N_COOR_64_LON_S) - 180,
				((nd64 & N_COOR_64_LAT) >> N_COOR_64_LAT_S) - 180);*/
		if(isInsidePoint(world, intCoor(world->act_node.lon), intCoor(world->act_node.lat)))
			nd64 |= ND_COOR_FLAG;
		memcpy(world->nodes + (world->act_idx * world->size_node), &nd64, world->size_node);
	}
	world->act_idx++;

	return 0;
}

int load_node_tag_close(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;

		if(world_->inside_tag_flags & INSIDE_NODE)
		{
			if(!strcmp((const char*)sax->tag_start, "node"))
			{
				if(sax->inside_mask & SAX_TAG_END_FLAG)
				{
					return(close_node(world_));
				}
			}
			if(sax->tag_name_start)
			{
				if(!strcmp((const char*)sax->tag_name_start, "node"))
				{
					if(sax->last_byte == '/')
					{
						return(close_node(world_));
					}
				}
			}
		}
	}

	return 0;
}

int load_node_tag_name(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;

		if(!strcmp((const char*)sax->tag_start, "node"))
		{
			if(world_->inside_tag_flags & INSIDE_NODE)
			{
				//printf("Error: already inside node [%s] [%s] %lld\n",sax->tag_start,
				//	sax->tag_name_start, 
				//	(unsigned long long int)world_->id);
			}
			else
				world_->inside_tag_flags |= INSIDE_NODE;
		}
	}

	return 0;
}



int load_node_tag_arg_name(struct _simple_sax * sax)
{
	World_t * world_ = (World_t *)sax->d_ref;

	if(sax->tag_start && (world_->inside_tag_flags & INSIDE_NODE))
	{
		size_t tag_act_size = strlen((const char*) sax->tag_start);
		tag_act_size++;

		switch (sax->tag_act_size)
		{
		case 3:		// id, to
			if(sax->tag_start[0] == 'i')
				world_->key_flags |= ND_ID;
			break;

		case 4:		// lat, lon
			if(sax->tag_start[0] == 'l')
			{
				world_->key_flags |= ND_COOR;
				if(sax->tag_start[1] == 'a')
					world_->key_flags |= ND_LAT;
				if(sax->tag_start[1] == 'o')
					world_->key_flags |= ND_LON;
			}
			break;

		default:
			break;
		}
	}

	return 0;
}

int load_node_arg_end(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;


		if(world_->inside_tag_flags & INSIDE_NODE)
		{
			if(world_->key_flags == ND_ID)
			{
				world_->act_node.id = atol((const char *)sax->tag_start);
				world_->key_flags = 0x00;
			}

			if(world_->key_flags == (ND_LON | ND_COOR))
			{
				world_->act_node.lon = atof((const char*)sax->tag_start);
				//world_->act_node.lon = (uint32_t)(atof((const char*)sax->tag_start) * COOR_FACTOR);
				world_->key_flags = 0x00;
			}

			if(world_->key_flags == (ND_LAT | ND_COOR))
			{
				world_->act_node.lat = atof((const char*)sax->tag_start);
				//world_->act_node.lat = (uint32_t)(atof((const char*)sax->tag_start) * COOR_FACTOR);
				world_->key_flags = 0x00;
			}
		}
	}

	return 0;
}

void node_count_init_06(struct _simple_sax * xml_ref)
{
	sax_add_cb(xml_ref, load_node_tag_open,     SAX_CB_TAG_OPEN);
	sax_add_cb(xml_ref, load_node_tag_close,    SAX_CB_TAG_CLOSE);
	sax_add_cb(xml_ref, load_node_arg_end,      SAX_CB_ARG_VALUE);
	sax_add_cb(xml_ref, load_node_tag_name,     SAX_CB_TAG_NAME);
	sax_add_cb(xml_ref, load_node_tag_arg_name, SAX_CB_ARG_NAME);
}


int readNodes(z_block * z_read, World_t * act_world, int mode, StdParam * param)
{
	int n_read;
	unsigned char * z_buf;
	int z_size;
	simple_sax sax;
	int tag_len=0;

	sax_init(&sax, 0);
	init_world_ref(act_world, mode);
	//printf("I %s %i, after init_world_ref\n", __FILE__, __LINE__);
	node_count_init_06(&sax);

	sax_set_data_ref(&sax, act_world);

	zblock_set_start(z_read, NULL, 0);

	param->max_size = act_world->count_node = act_world->info.node.count;
	//printf("after zblock_set_start\n");

	while((n_read = zblock_read(z_read)) > 0)
	{
		printProgress(param, "N-r", act_world->act_idx);

		sax.tag_start = zblock_first(z_read);

		z_buf = zblock_buff(z_read, &z_size);

		sax_read_block(&sax, z_buf, z_size);

		sax_save_tag_name(&sax);
		if(sax.tag_start != NULL)
			tag_len = strlen((const char *)sax.tag_start);
		else
			tag_len = 0;

		sax.tag_act_size = tag_len;

		zblock_set_start(z_read, sax.tag_start, tag_len);
	}
	fullProgress(param, "N-r");
	sax_cleanup(&sax);

	//printf("after sax_cleanup\n");

	return 0;
}

