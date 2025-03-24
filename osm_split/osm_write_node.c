/******************************************************************
 * project:     Trafalgar/OSM-Toolz
 *
 * class:       ---
 * superclass:  ---
 * modul:       osm_write_node.c
 *
 * system:      UNIX/LINUX
 * compiler:    gcc
 *
 * beginning:   12.2012
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

#include "osm_split.h"
#include "../common/simple_sax.h"
#include "../common/osm_info.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int update_world_ref(World_t * act_world)
{
	uint64_t bit_9 = (act_world->info.node.max_id << 6) + 1;

	act_world->id64_nd_lat = malloc(act_world->info.node.max_id);
	act_world->id64_nd_lon = malloc(act_world->info.node.max_id);
	act_world->id64_nd_lat_lon_bit9 = malloc(bit_9);

	return 0;
}

int load_node_tag_open(struct _simple_sax * sax)
{
	return 0;
}

int load_node_tag_close(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;

		if(world_->type_flags == INSIDE_NODE)
		{
			if(!strncmp((const char*)sax->tag_start,"node",4))
			{
				if(sax->inside_mask & SAX_TAG_END_FLAG)
				{
					world_->type_flags = 0;

					//checkSize(world_);

					return 0;
				}
			}
			if(sax->tag_name_start)
			{
				if(!strncmp((const char*)sax->tag_name_start,"node",4))
				{
					if(sax->last_byte == '/')
					{
						world_->type_flags = 0;

						//checkSize(world_);

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

		size_t tag_act_size = strlen((const char *)sax->tag_start);
		tag_act_size++;

		if(tag_act_size != (size_t)sax->tag_act_size)
			printf("tag: sax %i, len %i [%s]\n", sax->tag_act_size,
				(int)tag_act_size,(const char *)sax->tag_start);

		switch (sax->tag_act_size)
		{
		case 5:		// node
			if((sax->tag_start[0] == 'n') && (sax->tag_start[1] == 'o'))
			{
				if(world_->type_flags == INSIDE_NODE)
				{
					printf("Error: already inside node [%s] [%s] %lld\n",sax->tag_start,
						sax->tag_name_start,
						(unsigned long long int)world_->id);
				}
				world_->type_flags = INSIDE_NODE;
			}
			break;

		default:
			break;
		}

	}
	return 0;
}



int load_node_tag_arg_name(struct _simple_sax * sax)
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

		case 3:		// id, to
			if(world_->type_flags == INSIDE_NODE) /*||
				(world_->type_flags == INSIDE_REL))*/
			{
				if(sax->tag_start[0] == 'i')
					world_->key_flags = ND_ID;
			}
			break;

		case 4:		// lat, lon
			if((sax->tag_start[0] == 'l') && (world_->type_flags == INSIDE_NODE))
			{
				world_->key_flags = ND_COOR;
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

		size_t tag_act_size = strlen((const char *)sax->tag_start);
		tag_act_size++;

		if(tag_act_size != (size_t)sax->tag_act_size)
			printf("end: sax %i, len %i [%s]\n", sax->tag_act_size,
				(int)tag_act_size,(const char *)sax->tag_start);

		if(world_->type_flags == INSIDE_NODE)
		{
			if(world_->key_flags == ND_ID)
			{
				world_->store_id = atol((const char *)sax->tag_start);
			}
			if(world_->key_flags == ND_COOR)
			{
			}
			if(world_->key_flags == (ND_COOR | ND_LON))
			{
			}
			world_->key_flags = 0;
		}
	}
	return 0;
}

void node_init_list_coor_06(struct _simple_sax * xml_ref)
{
	sax_add_cb(xml_ref, load_node_tag_open,     SAX_CB_TAG_OPEN);
	sax_add_cb(xml_ref, load_node_tag_close,    SAX_CB_TAG_CLOSE);
	sax_add_cb(xml_ref, load_node_arg_end,      SAX_CB_ARG_VALUE);
	sax_add_cb(xml_ref, load_node_tag_name,     SAX_CB_TAG_NAME);
	sax_add_cb(xml_ref, load_node_tag_arg_name, SAX_CB_ARG_NAME);
}

int readNodes(z_block * z_read, World_t * act_world)
{
	int n_read;
	unsigned char * z_buf;
	int z_size;
	simple_sax sax;
	int tag_len=0;

	sax_init(&sax, 0);
	update_world_ref(act_world);
	node_init_list_coor_06(&sax);

	sax_set_data_ref(&sax, act_world);

	zblock_set_start(z_read, NULL, 0);

	while((n_read = zblock_read(z_read)) > 0)
	{
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

