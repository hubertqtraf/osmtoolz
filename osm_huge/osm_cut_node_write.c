/******************************************************************
 * project:     Trafalgar/OSM-Toolz 
 *
 * modul:      	osm_cut_node_write.c
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

#include "osm_huge.h"
#include "../common/simple_sax.h"
#include "../common/osm_info.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h> 

#include "osm_cut_func.c"

void init_write_world_node(World_t * w_ref)
{
	w_ref->max_id = w_ref->info.node.max_id;

	w_ref->type_flags = 0x00;
	w_ref->key_flags = 0x00;
	w_ref->val_flags = 0x00;
	w_ref->seg_flags = 0x00;

	w_ref->info.node.min_id = 0;

	w_ref->info.node.max_id = 0;
	w_ref->info.node.count = 0;
	w_ref->info.node.n_id = 0;

	w_ref->act_idx = 0;

	w_ref->next_id = 0;
}

uint64_t getNextID(World_t * w_ref)
{
	do
	{
		w_ref->next_id++;
		//if(w_ref->node_flags[w_ref->next_id >> 1])
		//	printf("vvv %ld %x\n", w_ref->next_id, w_ref->node_flags[w_ref->next_id >> 1]);
		if(w_ref->next_id & 1)
		{
			if((w_ref->node_flags[w_ref->next_id >> 1] & 0x10) == 0x10)
				return 1;
		}
		else
		{
			if((w_ref->node_flags[w_ref->next_id >> 1] & 0x01) == 0x01)
				return 1;
		}
	}
	while(w_ref->next_id <= w_ref->max_id);
	return 0;
}

int inline checkNodeFlag(World_t * world)
{
	// TODO: add mode '5'
	switch(world->node_flags_size)
	{
	case 1:
		if((world->node_flags[world->act_idx] & 0x01) == 0x01)
		{
			//printf(".");
			return 1;
		}
		break;
	case 5:
		if(world->mem_mode == MODE_MAX_ID)
		{
			if(world->next_id <= world->max_id)
			{
				//printf("## %x\n", world->node_flags[world->next_id >> 1]);
				if(world->next_id & 1)
				{
					if((world->node_flags[world->next_id >> 1] & 0x70) == 0x70)
						return 1;
				}
				else
				{
					if((world->node_flags[world->next_id >> 1] & 0x07) == 0x07)
						return 1;
				}
			}
		}
		else
		{
			if(get_node_flags2(world->node_flags, world->node_flags_size, world->act_idx))
			{
				return 1;
			}
		}
		//printf("TODO! (%x) ",get_node_flags2(world->node_flags, world->node_flags_size, world->act_idx));
		break;
	default:
		printf("unknown value: %i\n", world->node_flags_size);
		break;
	}
	return 0;
}

void inline closeNodeTag(World_t * world, char * end, int size)
{
	if(checkNodeFlag(world))
	{
		zblock_wr_append(&world->zw_node, end, size);
	}
	world->type_flags = 0;
	world->act_idx++;
}

int write_node_tag_open(struct _simple_sax * sax)
{
	return 0;
}

int write_node_tag_close(struct _simple_sax * sax)
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
					closeNodeTag(world_, "\t</node>\n", 9);
					return 0;
				}
			}
			if(!strncmp((const char*)sax->tag_name_start,"node",4))
			{
				if(sax->last_byte == '/')
				{
					closeNodeTag(world_, "/>\n",3);
				}
				if(world_->type_flags)
				{
					if(checkNodeFlag(world_))	
						zblock_wr_append(&world_->zw_node, ">\n",2);
				}
			}
			if(!strncmp((const char*)sax->tag_name_start,"tag",3))
			{
				if(checkNodeFlag(world_))
					zblock_wr_append(&world_->zw_node, "/>\n",3);
			}

		}
	}
	return 0;
}

int write_node_tag_name(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;
		z_block * zw_ = NULL;

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
					printf("Error: already inside node [%s] [%s] %ld\n",sax->tag_start,
						sax->tag_name_start,world_->id);
				}
				world_->type_flags = INSIDE_NODE;
				if((world_->mem_mode == MODE_MAX_ID) && (world_->node_flags_size == 5))
					getNextID(world_);
				//printf("nd;");
			}
			break;
			
		default:
			break;	
		}

		if((world_->type_flags == INSIDE_NODE) && (zw_ == NULL))
		{
			if(checkNodeFlag(world_))
			{
				zblock_wr_append(&(world_->zw_node), "\t<", 2);
				zblock_wr_append(&(world_->zw_node), (const char *)sax->tag_start,
					tag_act_size-1);
			}
		}

		if(zw_ != NULL)
		{
			if(checkNodeFlag(world_))
			{
				zblock_wr_append(zw_, "\t<", 2);
				zblock_wr_append(zw_, (const char *)sax->tag_start,
					tag_act_size-1);
				zblock_wr_append(zw_, " ", 1);
			}
		}
	}
	return 0;
}



int write_node_tag_arg_name(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;
		z_block * zw_ = NULL;
		
		
		size_t tag_act_size = strlen((const char*) sax->tag_start);
		tag_act_size++;

		if(tag_act_size != (size_t)sax->tag_act_size)
			printf("arg: sax %i, len %i [%s]\n", sax->tag_act_size, 
				(int)tag_act_size,(const char *)sax->tag_start);

		switch (sax->tag_act_size)
		{
		case 2:		// "k", "v"
			break;

		case 3:		// id, to
			if(world_->type_flags == INSIDE_NODE)
			{
				if(sax->tag_start[0] == 'i')
					world_->key_flags = ND_ID;
			}
			break;

		case 4:
			break;

		default:
			break;
		}

		if(world_->type_flags == INSIDE_NODE)
			zw_ = &world_->zw_node;

		if(zw_ != NULL)
		{
			if(checkNodeFlag(world_))
			{
				zblock_wr_append(zw_, " ",1);
				zblock_wr_append(zw_, (const char *)sax->tag_start, tag_act_size-1);
				zblock_wr_append(zw_, "=\"",2);
			}
		}
	}
	return 0;
}

int write_node_arg_end(struct _simple_sax * sax)
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
			int write_it = 0;

			if(checkNodeFlag(world_))
				write_it = 1;

			if(write_it)
			{
				zblock_wr_append(&world_->zw_node, (const char *)sax->tag_start,
					tag_act_size-1);
				zblock_wr_append(&world_->zw_node, "\"",1);
			}

			if(world_->key_flags == ND_ID)
			{
				if(write_it)
				{
					world_->id = atol((const char *)sax->tag_start);
					if(world_->info.node.min_id == 0)
					{
						world_->info.node.min_id = world_->id;
						world_->info.node.max_id = world_->id;
					}
					else
					{
						if(world_->id < world_->info.node.min_id)
							world_->info.node.min_id = world_->id;
						if(world_->id > world_->info.node.max_id)
							world_->info.node.max_id = world_->id;
					}
					world_->info.node.count++;
				}
			}
			world_->key_flags = 0;
		}
	}
	return 0;
}

void node_write_init_06(struct _simple_sax * xml_ref)
{
	sax_add_cb(xml_ref, write_node_tag_open,     SAX_CB_TAG_OPEN);
	sax_add_cb(xml_ref, write_node_tag_close,    SAX_CB_TAG_CLOSE);
	sax_add_cb(xml_ref, write_node_arg_end,      SAX_CB_ARG_VALUE);
	sax_add_cb(xml_ref, write_node_tag_name,     SAX_CB_TAG_NAME);
	sax_add_cb(xml_ref, write_node_tag_arg_name, SAX_CB_ARG_NAME);
}


int writeNodes(z_block * z_read, World_t * act_world, uint8_t flags, char * out_path, StdParam * param)
{
	int n_read;
	unsigned char * z_buf;
	int z_size;
	simple_sax sax;
	int tag_len=0;

	sax_init(&sax, 0);
	init_write_world_node(act_world);
	node_write_init_06(&sax);

	sax_set_data_ref(&sax, act_world);

	if(createOsmHead(&act_world->zw_node, out_path, flags))
		return -1;

	param->max_size = act_world->count_node;
	//printf("[--%s--]", out_path);

	zblock_set_start(z_read, NULL, 0);

	while((n_read = zblock_read(z_read)) > 0)
	{
		printProgress(param, "N-w", act_world->act_idx);

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
	fullProgress(param, "N-w");

	if(closeOsm(&act_world->zw_node))
		return -1;

	sax_cleanup(&sax);

	return 0;
}

