/******************************************************************
 * project:	Trafalgar/OSM-Toolz/osm_cut
 *
 * modul:	osm_cut_way.c
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

int select_rect(World_t * world, uint32_t lon, uint32_t lat, int next)
{
	if(next)
	{	
		if(world->act_way.lon1 > lon)
		{
			world->act_way.lon1 = lon;
		}
		if(world->act_way.lon2 < lon)
		{
			world->act_way.lon2 = lon;
		}
		if(world->act_way.lat1 > lat)
		{
			world->act_way.lat1 = lat;
		}
		if(world->act_way.lat2 < lat)
		{
			world->act_way.lat2 = lat;
		}
	}
	else
	{                       // first node
		world->act_way.lon1 = world->act_way.lon2 = lon;
		world->act_way.lat1 = world->act_way.lat2 = lat;
	}

	return 0;
}


int mark_node(World_t * world, uint64_t node_ref, int next)
{
	int64_t pos = findNode(world->nodes, world->size_node,
		world->info.node.count, node_ref);

	//printf("[%li]", pos);

	if(pos != -1)
	{
		//double lat = ((get_node_lat(world->nodes, world->size_node, pos) / COOR_FACTOR) - 180.0);
		//double lon = ((get_node_lon(world->nodes, world->size_node, pos) / COOR_FACTOR) - 180.0);

		select_rect(world, get_node_lon(world->nodes, world->size_node, pos),
			get_node_lat(world->nodes, world->size_node, pos), next);

		//printf("--> %li %f %f\n", pos, lat, lon);
	}
	else
	{
		// TODO: add/check the single points?
		// TODO: set the number of id-bytes from 8 to 5 (way id size is 32bit+)
		// rework here! func?
		uint64_t * id_list = (uint64_t*)world->ways;

		if(!(id_list[world->act_idx] & WAY_NO_ID_FLAG))
		{
			id_list[world->act_idx] |= WAY_NO_ID_FLAG;
			//printf("TODO: mark the way -> some nodes are not valid (%li, %li, %li)\n",
			//	       	node_ref, world->act_idx, world->act_way.id);
			//printf("!");
			return (-1);
		}
	}
	return 0;
}

int init_world_way(World_t * w_ref)
{
	//w_ref->store_type = 0; keep?

	w_ref->inside_tag_flags = 0x00;
	w_ref->key_flags = 0x00;

	w_ref->act_idx = 0;

	if((w_ref->node_ref_list = (uint64_t*)malloc(sizeof(uint64_t*) *
					w_ref->info.way.member)) == NULL)
	{
		fprintf(stderr, "unable to allocate node buffer 'w_ref->node_ref_list'\n");
		exit(-1);
	}
	memset(w_ref->node_ref_list, 0x00, sizeof(uint64_t*) * w_ref->info.way.member);

	//w_ref->current_ref_count = 0;
	w_ref->max_ref_count = 0;
	w_ref->act_member_idx = 0;

	// TODO: set the number of id-bytes from 8 to 5
	// check here...
	printf(">>>> %i %ld %ld\n", w_ref->size_way, w_ref->size_way * w_ref->info.way.count, w_ref->info.way.count);
	//exit(0);
	if((w_ref->ways = (uint8_t*)malloc(w_ref->size_way * w_ref->info.way.count)) == NULL)
	{
		fprintf(stderr,
			"unable to allocate way id buffer 'w_ref->ways' %i %li\n",
			w_ref->size_way, w_ref->info.way.count);
		exit(-1);
	}
	memset(w_ref->ways, 0x00, (w_ref->size_way * w_ref->info.way.count));
	return 0;
}


int load_way_tag_open(struct _simple_sax * sax)
{
	return 0;
}

int close_way(World_t * world)
{
	int valid = 1;
	world->inside_tag_flags  &= ~(INSIDE_WAY);

	//printf("id %li %li %2x %2x\n", world->act_idx, world->act_way.id, 
	//	world->inside_tag_flags, world->key_flags);

	for(int i = 0; i < world->act_member_idx; i++)
	{

		if(mark_node(world, world->node_ref_list[i], i) == (-1))
			valid = (-1);
		//printf("%li,", world->node_ref_list[i]);
	}

	// TODO: set the number of id-bytes from 8 to 5
	// rework here! -> func?
	uint64_t * id_list = (uint64_t*)world->ways;

	id_list[world->act_idx] = world->act_way.id;


	/*if((world->act_way.lon2 > world->select_rect.lon1) &&
		(world->act_way.lon1 < world->select_rect.lon2) &&
		(world->act_way.lat2 > world->select_rect.lat1) &&
		(world->act_way.lat1 < world->select_rect.lat2))*/
	if(isInsideRect(world))
	{

		//printf("**************** select ****************\n");

		for(int i = 0; i < world->act_member_idx; i++)
		{
			select_node(world, world->node_ref_list[i], COOR_FLAG);

			//mark_node(world, world->node_ref_list[i], i);
			//printf("%li,", world->node_ref_list[i]);
        	}
		// add way with id and flag
		if(valid == 1)
			id_list[world->act_idx] |= COOR_FLAG;  //0x1000000000000000; 
	}
	else
	{
	}

        //world->inside_tag_flags = 0x00;
        world->key_flags = 0x00;

	if(world->max_ref_count < world->act_member_idx)
		world->max_ref_count = world->act_member_idx;

	world->act_member_idx = 0;

	world->act_idx++;
	return 0;
}

int close_ref(World_t * world)
{
	world->inside_tag_flags  &= ~(INSIDE_WAY_ND);

	world->key_flags  &= ~(WAY_K | WAY_V);

	return 0;
}

int load_way_tag_close(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;

		if(world_->inside_tag_flags & INSIDE_WAY)
		{
			//printf("= %s\n", sax->tag_start);
			if(!strcmp((const char*)sax->tag_start, "way"))
			{
				if(sax->inside_mask & SAX_TAG_END_FLAG)
				{
					return(close_way(world_));
				}
			}
			if(!strcmp((const char*)sax->tag_start, "nd"))
			{
				if(sax->inside_mask & SAX_TAG_END_FLAG)
				{
					return(close_ref(world_));
				}
			}
			if(sax->tag_name_start)
			{
				//if(!strncmp((const char*)sax->tag_name_start,"nd",2))
				if(!strcmp((const char*)sax->tag_name_start, "way"))
				{
					if(sax->last_byte == '/')
					{
						return(close_way(world_));
					}
				}
				if(!strcmp((const char*)sax->tag_name_start, "nd"))
				{
					if(sax->last_byte == '/')
					{
						return(close_ref(world_));
					}
				}
			}
		}
	}
	return 0;
}


int load_way_tag_name(struct _simple_sax * sax)
{

	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;

		size_t tag_act_size = strlen((const char *)sax->tag_start);
		tag_act_size++;	

		if(tag_act_size != (size_t)sax->tag_act_size)
			printf("tag: sax %i, len %i [%s]\n", sax->tag_act_size, 
				(int)tag_act_size,(const char *)sax->tag_start);

		switch (sax->tag_act_size)
		{
		case 3:			// nd
			if(sax->tag_start[0] == 'n')
			{
				if(world_->inside_tag_flags & INSIDE_WAY)
					world_->inside_tag_flags |= INSIDE_WAY_ND;
			}
			break;

		case 4:		// tag, way
			if(sax->tag_start[0] == 'w')
			{
				if(world_->inside_tag_flags & INSIDE_WAY)
				{
					printf("Error: load_way_tag_name, already inside way [%s] \n",
						sax->tag_start);
				}
				world_->inside_tag_flags |= INSIDE_WAY;
			}
			break;
			
		default:
			break;	
		}
	}
	return 0;
}



int load_way_tag_arg_name(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;
		
		size_t tag_act_size = strlen((const char*) sax->tag_start);
		tag_act_size++;

		if(tag_act_size != (size_t)sax->tag_act_size)
			printf("arg: sax %i, len %i [%s]\n", sax->tag_act_size, 
				(int)tag_act_size,(const char *)sax->tag_start);

		switch (sax->tag_act_size)
		{
		case 2:		// "k", "v"
			if((sax->tag_start[0] == 'k') && (world_->inside_tag_flags & INSIDE_WAY))
				world_->key_flags |= WAY_K;
			if((sax->tag_start[0] == 'v') && (world_->inside_tag_flags & INSIDE_WAY))
				world_->key_flags |= WAY_V;
			break;

		case 3:		// id, to
			//if((world_->inside_tag_flags & 0x1f) == (INSIDE_WAY | INSIDE_WAY_ND))
			if(world_->inside_tag_flags == INSIDE_WAY)
			{
				if(sax->tag_start[0] == 'i')
				{
					world_->key_flags |= WAY_ID;
				}
			}
			break;

		case 4:		// ref
			if((world_->inside_tag_flags & 0x3f) == (INSIDE_WAY | INSIDE_WAY_ND))
			{
				if(sax->tag_start[0] == 'r')
				{
					world_->key_flags |= WAY_REF;
				}
			}
			break;

		default:
			break;
		}
	}
	return 0;
}

int load_way_arg_end(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;

		size_t tag_act_size = strlen((const char *)sax->tag_start);
		tag_act_size++;

		//if(tag_act_size != (size_t)sax->tag_act_size)
		//	printf("end: sax %i, len %i [%s]\n", sax->tag_act_size,
		//		(int)tag_act_size,(const char *)sax->tag_start);

		if(world_->inside_tag_flags & INSIDE_WAY)
		{
			if(world_->inside_tag_flags & INSIDE_WAY_ND)
			{
				// node ref
				if(world_->key_flags & WAY_REF)
				{
					uint64_t node_ref = atol((const char*)sax->tag_start);
					world_->node_ref_list[world_->act_member_idx] = node_ref;
					world_->act_member_idx++;
				}
			}
			else
			{
				// way id
				if(world_->key_flags & WAY_ID)
				{
					world_->act_way.id = atol((const char *)sax->tag_start);
					//printf("y %s %li\n", sax->tag_start, world_->act_way.id);
					world_->key_flags &= ~(WAY_ID);
				}
			}
		}
	}

	// WAY_V and WAY_K are unused!

	return 0;
}

void way_count_init_06(struct _simple_sax * xml_ref)
{
	sax_add_cb(xml_ref, load_way_tag_open,     SAX_CB_TAG_OPEN);
	sax_add_cb(xml_ref, load_way_tag_close,    SAX_CB_TAG_CLOSE);
	sax_add_cb(xml_ref, load_way_arg_end,      SAX_CB_ARG_VALUE);
	sax_add_cb(xml_ref, load_way_tag_name,     SAX_CB_TAG_NAME);
	sax_add_cb(xml_ref, load_way_tag_arg_name, SAX_CB_ARG_NAME);
}

int readWays(z_block * z_read, World_t * act_world)
{
	int n_read;
	unsigned char * z_buf;
	int z_size;
	simple_sax sax;
	int tag_len=0;

	sax_init(&sax, 0);

	if(init_world_way(act_world) != 0)
	{
		return 1;
	}

	way_count_init_06(&sax);

	sax_set_data_ref(&sax, act_world);

	zblock_set_start(z_read, NULL, 0);

	while((n_read = zblock_read(z_read)) > 0)
	{
		if(1)   // TODO: set flag for debug output like: act_world->flags & DEBUG_1
			printf("- w-r: %ld -", act_world->act_idx);

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

	sax_cleanup(&sax);

	return 0;
}

