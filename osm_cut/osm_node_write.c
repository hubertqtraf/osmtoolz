/******************************************************************
 * project:     Trafalgar/OSM-Toolz 
 *
 * modul:       osm_cut_way.c
 *
 * system:      UNIX/LINUX
 * compiler:    gcc
 *
 * beginning:   12.2012
 *
 * (C)          Schmid Hubert 2012-2023
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

void init_write_world_node(World_t * w_ref)
{
	w_ref->inside_tag_flags = 0x00;
	w_ref->key_flags = 0x00;

	w_ref->act_idx = 0;

	resetInfoPart(&w_ref->info.node);
	resetInfoPart(&w_ref->info.point);
}

// tmp test
void test_node(World_t * w_ref)
{
	printf("\n* %li *\n", w_ref->info.node.count);

	for(int i =0; i<w_ref->info.node.count; i++)
	{
		uint8_t * pos = w_ref->nodes + (i * w_ref->size_node);
		uint64_t * n_id = (uint64_t *)pos;
		printf("%lx %li %lx\n", *n_id, (*n_id) & 0x0000000fffffffff, (*n_id) & COOR_FLAG);
	}
}

uint64_t check_node(World_t * world)
{
	uint8_t * pos = world->nodes + (world->act_idx * world->size_node);
	uint64_t * n_id = (uint64_t *)pos;
	return ((*n_id) & (COOR_FLAG | ND_COOR_FLAG));
}

void inline append_node_part(World_t * world, uint64_t flags, const char * text, int size)
{
	if(flags & COOR_FLAG)
	{
		zblock_wr_append(&world->zw_out, text, size);
	}
	else
	{
		if(flags & ND_COOR_FLAG)
		{
			// TODO: point mode is removed; rework to activate this mode
			//zblock_wr_append(&world->zw_point_out, text, size);
			zblock_wr_append(&world->zw_out, text, size);
		}
	}
}

void inline close_write_node(World_t * world, const char * text, int size)
{
	append_node_part(world, check_node(world), text, size);

	world->inside_tag_flags  &= ~(INSIDE_NODE);
	world->key_flags = 0;
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

		if(world_->inside_tag_flags & INSIDE_NODE)
		{
			if(!strncmp((const char*)sax->tag_start, "node", 4))
			{
				if(sax->inside_mask & SAX_TAG_END_FLAG)
				{
					close_write_node(world_, "\t</node>\n", 9);
					return 0;
				}
			}
			if(!strncmp((const char*)sax->tag_name_start, "node", 4))
			{
				if(sax->last_byte == '/')
				{
					close_write_node(world_, "/>\n", 3);
				}
				if(world_->inside_tag_flags)
				{
					append_node_part(world_, check_node(world_), ">\n", 2);
					world_->key_flags = 0;
				}
			}
			if(!strncmp((const char*)sax->tag_name_start, "tag", 3))
			{
				append_node_part(world_, check_node(world_), "/>\n", 3);
			}

		}
	}
	return 0;
}

int write_node_tag_name(struct _simple_sax * sax)
{

	if(sax->tag_start)
	{
		//printf("%s=",sax->tag_start);

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
				if(world_->inside_tag_flags & INSIDE_NODE)
				{
				}
				world_->inside_tag_flags |= INSIDE_NODE;
				// world_->type_flags = INSIDE_NODE;
				//printf("nd;");
			}
			break;
			
		default:
			break;	
		}

		if(world_->inside_tag_flags & INSIDE_NODE)
		{
			append_node_part(world_, check_node(world_), "\t<", 2);
			append_node_part(world_, check_node(world_), (const char *)sax->tag_start, tag_act_size-1);
		}
	}
	return 0;
}



int write_node_tag_arg_name(struct _simple_sax * sax)
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
			break;

		case 3:		// id, to
			if(world_->inside_tag_flags & INSIDE_NODE)
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

		if(world_->inside_tag_flags & INSIDE_NODE)
		{
			append_node_part(world_, check_node(world_), " ", 1);
			append_node_part(world_, check_node(world_), (const char *)sax->tag_start,
				tag_act_size-1);
			append_node_part(world_, check_node(world_), "=\"", 2);
		}
	}
	return 0;
}

int write_node_arg_end(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;

		size_t tag_act_size = strlen((const char *)sax->tag_start);
		tag_act_size++;

		if(tag_act_size != (size_t)sax->tag_act_size)
			printf("end: sax %i, len %i [%s]\n", sax->tag_act_size,
				(int)tag_act_size,(const char *)sax->tag_start);

		if(world_->inside_tag_flags & INSIDE_NODE)
		{
			append_node_part(world_, check_node(world_), (const char *)sax->tag_start,
				tag_act_size-1);
			append_node_part(world_, check_node(world_), "\"", 1);

			if(world_->key_flags == ND_ID)
			{
				if(check_node(world_) & COOR_FLAG)
				{
					uint64_t act_id = atol((const char *)sax->tag_start);
					updateInfoPart(&world_->info.node, act_id);
				}
				else
				{
					if(check_node(world_) & ND_COOR_FLAG)
					{
						uint64_t act_id = atol((const char *)sax->tag_start);
						updateInfoPart(&world_->info.node, act_id);
						// TODO: point mode is removed; rework to activate this mode
						//updateInfoPart(&world_->info.point, act_id);
					}
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

int writeNodes(z_block * z_read, World_t * act_world, char * p_fname, StdParam * param)
{
	int n_read;
	unsigned char * z_buf;
	int z_size;
	simple_sax sax;
	int tag_len=0;

	if(p_fname == NULL)
	{
		return -1;
	}

	sax_init(&sax, 0);
	init_write_world_node(act_world);
	node_write_init_06(&sax);

	sax_set_data_ref(&sax, act_world);

	if(createOsmHead(&act_world->zw_out, act_world->out_path, ZB_WRITE))
		return -1;
	// TODO: point mode is removed; rework to activate this mode
	//f(createOsmHead(&act_world->zw_point_out, p_fname, ZB_WRITE))
	//	return -1;
	//printf("out files:  %s %s\n", act_world->out_path, p_fname);

	param->max_size = act_world->count_node;

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

	if(closeOsm(&act_world->zw_out))
		return -1;
	// TODO: point mode is removed; rework to activate this mode
	//if(closeOsm(&act_world->zw_point_out))
	//	return -1;

	sax_cleanup(&sax);

	//test_node(act_world);

	act_world->info.node.count =  act_world->info.node.i_count;
	act_world->info.point.count = act_world->info.point.i_count;

	return 0;
}

