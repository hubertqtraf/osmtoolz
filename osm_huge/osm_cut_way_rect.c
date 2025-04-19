/******************************************************************
 * project:     Trafalgar/OSM-Toolz
 *
 * modul:      	osm_cut_way_rect.c
 *
 * system:      UNIX/LINUX
 * compiler:    gcc
 *
 * beginning:   03.2025
 *
 * (C)          Schmid Hubert 2025-2025
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

//#include "osm_node_cut.h"
#include "osm_huge.h"
#include "../common/simple_sax.h"
#include "../common/osm_info.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "osm_cut_func.c"

// TODO: use a copy in ../osm_nodes?

void init_world_ref_x(World_t * w_ref/*, int32_t rect[4]*/)
{
	w_ref->act_idx = 0;

	w_ref->way_flags = NULL;

	w_ref->type_flags = 0x00;
	w_ref->key_flags = 0x00;
	w_ref->val_flags = 0x00;
	w_ref->seg_flags = 0x00;
	w_ref->id = 0L;
	w_ref->nd_lat = 400.0;
	w_ref->nd_lon = 400.0;

	if(w_ref->info.node.count == 0)
		return;

	if((w_ref->way_flags = (uint8_t *)malloc(sizeof(uint8_t) * (w_ref->info.way.count/* >> 3*/))) == NULL)
		return;
	memset(w_ref->way_flags, 0x00, sizeof(uint8_t) * (w_ref->info.way.count/* >> 3*/));

	if((w_ref->node_ref_list = (uint64_t*)malloc(sizeof(uint64_t*) *
			w_ref->info.way.member)) == NULL)
	{
		fprintf(stderr, "unable to allocate node buffer 'w_ref->node_ref_list'\n");
		exit(-1);
	}
	memset(w_ref->node_ref_list, 0x00, sizeof(uint64_t*) * w_ref->info.way.member);



	//printf("rect %i\n", rect[0]);

	/*w_ref->select_rect[0] = ((double)rect[0]) / 10000000.0; 
	w_ref->select_rect[1] = ((double)rect[1]) / 10000000.0;
	w_ref->select_rect[2] = ((double)rect[2]) / 10000000.0;
	w_ref->select_rect[3] = ((double)rect[3]) / 10000000.0;*/

	printf("rect %f\n", w_ref->select_rect[0]);
}


/*int checkSize(World_t * w_ref)
{
	int ret = 0;

	//printf("=> %f %f | %f %f\n", w_ref->nd_lon, w_ref->select_rect[0], w_ref->nd_lat, w_ref->select_rect[2]);

	if((w_ref->nd_lat > 399) || (w_ref->nd_lon > 399))
	{
		printf("undefined coor\n");
		ret = -1;
	}

	if((w_ref->nd_lon > w_ref->select_rect[0]) &&
		(w_ref->nd_lon < w_ref->select_rect[1])	 &&
		(w_ref->nd_lat > w_ref->select_rect[2])  &&
		(w_ref->nd_lat < w_ref->select_rect[3]))
	{
		w_ref->node_flags[w_ref->act_idx] |= 0x01;
		//printf("checkSize %f %f\n",w_ref->nd_lat, w_ref->nd_lon);
		//printf("+");
	}
	else
		;//printf("-");
	w_ref->nd_lat = 400.0;
	w_ref->nd_lon = 400.0;

	return ret;
}*/

int mark_node(World_t * world, uint64_t node_ref, int next)
{
	int64_t pos = findNode(world->node_flags, world->node_flags_size, //world->size_node,
		world->info.node.count, node_ref);
	//printf("[%ld %ld]\n", node_ref, pos);
	if(pos != -1)
	{
		/*printf("get_node_flags %ld %x\n",
				get_node_id(world->node_flags, world->node_flags_size, pos),
				get_node_flags(world->node_flags, world->node_flags_size, pos));*/
		if(get_node_flags(world->node_flags, world->node_flags_size, pos) & 0x40)
		{
			return 0;
		}
	}
	return -1;
}

int close_way(World_t * world)
{
	// all points are inside the rect
	int valid = 1;
	world->type_flags  &= ~(INSIDE_WAY);

	/*printf("id %li %2x %2x\n", world->act_idx, *//*world->act_way.id, */
	//	world->type_flags, world->key_flags);

	for(int i = 0; i < world->act_member_idx; i++)
	{
		if(mark_node(world, world->node_ref_list[i], i) == (-1))
			valid = (-1);
		//printf("- %i -", valid);
		//printf("--- %li,", world->node_ref_list[i]);
	}
	if(valid == 1)
	{
		world->way_flags[world->act_idx] = 1;
	}
	else
	{
		world->way_flags[world->act_idx] = 0;
	}
	//printf("+++++++++++++++++++++++++++ (%ld %i %i)", world->act_idx, world->way_flags[world->act_idx], valid);
	world->act_member_idx = 0;

	world->act_idx++;
	return 0;
}

int close_ref(World_t * world)
{
	world->type_flags  &= ~(INSIDE_WAY_ND);

	world->key_flags  &= ~(WAY_K | WAY_V);

        return 0;
}

int load_way_tag_open(struct _simple_sax * sax)
{
	return 0;
}

int load_way_tag_close(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;

		if(world_->type_flags & INSIDE_WAY)
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
		case 3:                 // nd
			if(sax->tag_start[0] == 'n')
			{
				if(world_->type_flags & INSIDE_WAY)
					world_->type_flags |= INSIDE_WAY_ND;
				//printf("#%s# %x", sax->tag_start, world_->type_flags);
			}
			break;

		case 4:         // tag, way
			if(sax->tag_start[0] == 'w')
			{
				if(world_->type_flags & INSIDE_WAY)
				{
					printf("Error: load_way_tag_name, already inside way [%s] \n",
						sax->tag_start);
				}
				world_->type_flags |= INSIDE_WAY;
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
		case 2:         // "k", "v"
			if((sax->tag_start[0] == 'k') && (world_->type_flags & INSIDE_WAY))
				world_->key_flags |= WAY_K;
			if((sax->tag_start[0] == 'v') && (world_->type_flags & INSIDE_WAY))
				world_->key_flags |= WAY_V;
			break;

		case 3:         // id, to
			//if((world_->type_flags & 0x1f) == (INSIDE_WAY | INSIDE_WAY_ND))
			if(world_->type_flags == INSIDE_WAY)
			{
				if(sax->tag_start[0] == 'i')
				{
					world_->key_flags |= WAY_ID;
				}
			}
			break;

		case 4:         // ref
			if((world_->type_flags & 0x3f) == (INSIDE_WAY | INSIDE_WAY_ND))
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

		//printf("%s,",sax->tag_start);

		size_t tag_act_size = strlen((const char *)sax->tag_start);
		tag_act_size++;

		if(tag_act_size != (size_t)sax->tag_act_size)
			printf("end: sax %i, len %i [%s]\n", sax->tag_act_size,
				(int)tag_act_size,(const char *)sax->tag_start);

		if(world_->type_flags & INSIDE_WAY)
		{
			if(world_->type_flags & INSIDE_WAY_ND)
			{
				// node ref
				if(world_->key_flags & WAY_REF)
				{
					// TODO: member!
					uint64_t node_ref = atol((const char*)sax->tag_start);
					world_->node_ref_list[world_->act_member_idx] = node_ref;
					//printf("# %id", node_ref);
					world_->act_member_idx++;
				}
			}
			else
			{
				// way id
				if(world_->key_flags & WAY_ID)
				{
					// TODO: way ID
					//world_->way_id = atol((const char *)sax->tag_start);
					//printf("y %s %li\n", sax->tag_start, world_->act_way.id);
					world_->key_flags &= ~(WAY_ID);
				}
			}
		}
	}
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

int cutWays(z_block * z_read, World_t * act_world/*, int32_t rect[4]*/)
{
	int n_read;
	unsigned char * z_buf;
	int z_size;
	simple_sax sax;
	int tag_len=0;

	sax_init(&sax, 0);
	init_world_ref_x(act_world/*, rect*/);
	way_count_init_06(&sax);

	sax_set_data_ref(&sax, act_world);

	zblock_set_start(z_read, NULL, 0);

	while((n_read = zblock_read(z_read)) > 0)
	{
		if(1)	// TODO: set flag for debug output like: act_world->flags & DEBUG_1
			printf("- n-r: %ld -", act_world->act_idx);
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

