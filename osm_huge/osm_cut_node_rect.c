/******************************************************************
 * project:     Trafalgar/OSM-Toolz
 *
 * modul:	osm_cut_node_rect.c
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

// TODO: use a copy in ../osm_nodes?
#include "osm_huge.h"

#include "../common/simple_sax.h"
#include "../common/osm_info.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// TODO: use a copy in ../osm_nodes?

void init_world_ref(World_t * w_ref, int32_t rect[4])
{
	w_ref->node_flags = NULL;
	w_ref->way_flags = NULL;
	//w_ref->node_flags_size = 5;	// 1

	w_ref->node_ref_list = NULL;

	w_ref->type_flags = 0x00;
	w_ref->key_flags = 0x00;
	w_ref->val_flags = 0x00;
	w_ref->seg_flags = 0x00;
	w_ref->id = 0L;
	w_ref->nd_lat = 400.0;
	w_ref->nd_lon = 400.0;
	w_ref->nd_id = 0;

	//if(w_ref->info.node.count == 0)
	//	return -1;

	// <<3 -> 2Bit
	//if((w_ref->node_flags = (uint8_t *)malloc(sizeof(uint8_t) * (w_ref->info.node.count * w_ref->node_flags_size))) == NULL)
	//	return -1;
	//memset(w_ref->node_flags, 0x00, sizeof(uint8_t) * (w_ref->info.node.count * w_ref->node_flags_size /* >> 3*/));

	w_ref->select_rect[0] = ((double)rect[0]) / 10000000.0;
	w_ref->select_rect[1] = ((double)rect[1]) / 10000000.0;
	w_ref->select_rect[2] = ((double)rect[2]) / 10000000.0;
	w_ref->select_rect[3] = ((double)rect[3]) / 10000000.0;
	printf("init rect: %f %f %f %f\n", w_ref->select_rect[0], w_ref->select_rect[1], w_ref->select_rect[2], w_ref->select_rect[3]);
}

int createNodeFlagList(World_t * w_ref)
{
	printf("I %s %i node_flags_size: %i %li\n", __FILE__, __LINE__, w_ref->node_flags_size, w_ref->info.node.count);
	//printf("I %s %i node_flags_size: %i %li\n", __PRETTY_FUNCTION__, __LINE__, w_ref->node_flags_size, w_ref->info.node.count);

	if(w_ref->info.node.count == 0)
		return -1;
	if(w_ref->node_flags != NULL)
	{
		free(w_ref->node_flags);
		w_ref->node_flags = NULL;
	}
	if((w_ref->node_flags = (uint8_t *)malloc(sizeof(uint8_t) * (w_ref->info.node.count * w_ref->node_flags_size))) == NULL)
		return -1;
	memset(w_ref->node_flags, 0x00, sizeof(uint8_t) * (w_ref->info.node.count * w_ref->node_flags_size /* >> 3*/));
	return 0;
}

void setMode(World_t * w_ref, uint8_t mode)
{
	w_ref->node_flags_size = mode;
	printf("mode: %i\n", w_ref->node_flags_size);
}

int checkSize(World_t * w_ref)
{
	int ret = 0;
	uint64_t id_40 = w_ref->nd_id;

	if((w_ref->nd_lat > 399) || (w_ref->nd_lon > 399))
	{
		printf("undefined coor\n");
		ret = -1;
	}

	//printf("### %ii - %f %f\n", w_ref->node_flags_size, w_ref->nd_lon, w_ref->select_rect[0]);

	if((w_ref->nd_lon > w_ref->select_rect[0]) &&
		(w_ref->nd_lon < w_ref->select_rect[1])	 &&
		(w_ref->nd_lat > w_ref->select_rect[2])  &&
		(w_ref->nd_lat < w_ref->select_rect[3]))
	{
		if(w_ref->node_flags_size == 1)
		{
			//printf("+");
			w_ref->node_flags[w_ref->act_idx] |= 0x01;
		}
		else	// > 4
		{
			id_40 |= ID_40_FLAG;
			/*memcpy(w_ref->node_flags + (w_ref->act_idx * w_ref->node_flags_size),
					&id_40, w_ref->node_flags_size);
			printf("checkSize %ld %ld\n", w_ref->nd_id, id_40);*/
		}
		//printf("checkSize %f %f\n",w_ref->nd_lat, w_ref->nd_lon);
		//printf("+");
	}
	//else
	//	printf("-");
	if((w_ref->node_flags == 0) && (w_ref->act_idx == 0))
	{
		printf("value is '0'!\n");
		return (-1);
	}
	if(w_ref->node_flags_size == 5)
	{
		memcpy(w_ref->node_flags + (w_ref->act_idx * w_ref->node_flags_size),
				&id_40, w_ref->node_flags_size);
	}
	w_ref->nd_lat = 400.0;
	w_ref->nd_lon = 400.0;
	w_ref->nd_id = 0;

	return ret;
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

					checkSize(world_);

					world_->act_idx++;
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

						checkSize(world_);

						world_->act_idx++;
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

		//printf("%s,",sax->tag_start);

		size_t tag_act_size = strlen((const char *)sax->tag_start);
		tag_act_size++;

		if(tag_act_size != (size_t)sax->tag_act_size)
			printf("end: sax %i, len %i [%s]\n", sax->tag_act_size,
				(int)tag_act_size,(const char *)sax->tag_start);

		if(world_->type_flags == INSIDE_NODE)
		{
			if(world_->key_flags == ND_ID)
			{
				if(world_->act_idx >= world_->info.node.count)
				{
					printf("node id out of range %lld %lld\n",
						(unsigned long long int)world_->act_idx,
						(unsigned long long int)world_->info.node.count);
				}
				else
				{
					world_->nd_id = atol((const char *)sax->tag_start);
					//printf("node id %ld\n", world_->nd_id);
				}
			}
			if(world_->key_flags == ND_COOR)
			{
				world_->nd_lat = atof((const char*)sax->tag_start);
				//printf("[%f]",world_->nd_lat);
			}
			if(world_->key_flags == (ND_COOR | ND_LON))
			{
				world_->nd_lon = atof((const char*)sax->tag_start);
			}
			world_->key_flags = 0;
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

int readNodes(z_block * z_read, World_t * act_world, StdParam * param) //int32_t rect[4])
{
	int n_read;
	unsigned char * z_buf;
	int z_size;
	simple_sax sax;
	int tag_len=0;

	sax_init(&sax, 0);
	init_world_ref(act_world, param->rect);
	if(createNodeFlagList(act_world))
		return -1;

	node_count_init_06(&sax);

	sax_set_data_ref(&sax, act_world);

	zblock_set_start(z_read, NULL, 0);

	// copy the number of nodes of the old dataset
	param->max_size = act_world->count_node = act_world->info.node.count;

	while((n_read = zblock_read(z_read)) > 0)
	{
		if(1)	// TODO: set flag for debug output like: act_world->flags & DEBUG_1
		{
			printProgress(param, "N-r", act_world->act_idx);	
			//printf("- n-r: %ld -", act_world->act_idx);
		}
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
	if(1)
	{
		printProgress(param, "N-r", act_world->info.node.count);
		printf("\n");
	}
	sax_cleanup(&sax);

	return 0;
}

