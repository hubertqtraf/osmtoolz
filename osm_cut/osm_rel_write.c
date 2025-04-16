/******************************************************************
 * project:     Trafalgar/OSM-Toolz 
 *
 * modul:       osm_write_rel.c
 *
 * system:      UNIX/LINUX
 * compiler:    gcc
 *
 * beginning:   04.2017
 *
 * (C)          Schmid Hubert 2017-2025
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
//#include "read_types.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h> 


void init_write_world_rel(World_t * w_ref)
{
	w_ref->inside_tag_flags = 0x00;
	w_ref->key_flags = 0x00;

	w_ref->act_idx = 0;
	w_ref->act_selected = 0;

	w_ref->info.rel.min_id = 0;
	w_ref->info.rel.max_id = 0;
	w_ref->info.rel.member = 0;
	w_ref->test_member = 0;
	//w_ref->info.rel.count = 0;
	//w_ref->info.rel.n_id = 0;

	w_ref->act_member_idx = 0;

}

void close_w_relation(World_t * world, char * end, int size)
{
	uint64_t * rels = (uint64_t *)world->relations;
	if(rels[world->act_idx] & COOR_FLAG)
	{
		zblock_wr_append(&(world->zw_out), end, size);
		//	"\t</relation>\n", 13);

		if(world->act_member_idx > world->info.rel.member)
		{
			world->info.rel.member  = world->act_member_idx;
		}
		/*printf("%ld %ld %i\n", world_->info.rel.count,
			world_->info.rel.member,
		world_->act_member_idx);*/
	}
	world->inside_tag_flags = 0;
	world->act_member_idx = 0;
	world->act_idx++;
}	

int write_rel_tag_open(struct _simple_sax * sax)
{
	return 0;
}

int write_rel_tag_close(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;

		if(world_->inside_tag_flags & INSIDE_REL)
		{
			if(!strncmp((const char*)sax->tag_start,"relation", 8))
			{
				if(sax->inside_mask & SAX_TAG_END_FLAG)
				{
					close_w_relation(world_, "\t</relation>\n", 13);
					
					return 0;
				}
			}
			if(sax->tag_name_start)
			{
				int write_it = 0;

				uint64_t * rels = (uint64_t *)world_->relations;
				
				if(rels[world_->act_idx] & COOR_FLAG)
					write_it = 1;

				if(!strncmp((const char*)sax->tag_name_start,"member",6))
				{
					//if(world_->ways[world_->act_idx].id & INSIDE_FLAG)
					if(write_it)
					{
						zblock_wr_append(&(world_->zw_out), "/>\n",3);
					
						// world_->rel_member++;
						world_->act_member_idx++;
					}
					return 0;
				}
				if(!strncmp((const char*)sax->tag_name_start,"seg",3))
				{
					//if(world_->ways[world_->act_idx].id & INSIDE_FLAG)
					if(write_it)
						zblock_wr_append(&(world_->zw_out), "/>\n",3);
					return 0;
				}
				if(!strncmp((const char*)sax->tag_name_start,"tag",3))
				{
					//if(world_->ways[world_->act_idx].id & INSIDE_FLAG)
					if(write_it)
						zblock_wr_append(&(world_->zw_out), "/>\n",3);
					return 0;
				}
				if(!strncmp((const char*)sax->tag_name_start, "relation", 8))
				{	
					if(sax->last_byte == '/')
					{
						close_w_relation(world_, "/>\n", 3);
					}
				}
// yyy				if(world_->type_flags)
				{
					//if(world_->ways[world_->act_idx].id & INSIDE_FLAG)
					if(write_it)
						zblock_wr_append(&(world_->zw_out), ">\n",2);
				}

			}
		}
	}
	return 0;
}

int write_rel_tag_name(struct _simple_sax * sax)
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
		case 4:		// tag, way
			break;
	
		case 7:
			if(sax->tag_start[0] == 'm')
			{
				if(world_->inside_tag_flags & INSIDE_REL)
					world_->inside_tag_flags |= INSIDE_REL_MB;
				//if(world_->type_flags == INSIDE_REL)
					//world_->key_flags = REL_MEMBER;
			}
			break;

		case 9:
			if(sax->tag_start[0] == 'r')
			{
				if(world_->inside_tag_flags & INSIDE_REL)
				{
					printf("Error: already inside relation [%s] \n",
						sax->tag_start);
				}
				world_->inside_tag_flags |= INSIDE_REL;
				zw_ = &(world_->zw_out);
			}
			break;
	
		default:
			break;	
		}

		int write_it = 0;

		uint64_t * rels = (uint64_t *)world_->relations;
                if(rels[world_->act_idx] & COOR_FLAG)
		// yyy if(world_->relations[world_->act_idx].flags != 0)
			write_it = 1;

		if((world_->inside_tag_flags & INSIDE_REL) && (zw_ == NULL))
		{
			if(write_it)
			{
				zblock_wr_append(&(world_->zw_out), "\t\t<", 3);
				zblock_wr_append(&(world_->zw_out), (const char *)sax->tag_start,
					tag_act_size-1);
			}
		}

		if(zw_ != NULL)
		{
			if(write_it)
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



int write_rel_tag_arg_name(struct _simple_sax * sax)
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
			if(world_->inside_tag_flags & INSIDE_REL)
			{
				if(!(world_->inside_tag_flags & INSIDE_REL_MB))
				{
					if(sax->tag_start[0] == 'i')
						world_->key_flags |= REL_ID;
				}
			}
			break;

		case 4:		// ref
			if((sax->tag_start[0] == 'r') && (world_->inside_tag_flags & INSIDE_REL))
				world_->key_flags |= REL_REF;
			break;

		default:
			break;
		}

		if(world_->inside_tag_flags & INSIDE_REL)
			zw_ = &world_->zw_out;

		if(zw_ != NULL)
		{
			uint64_t * rels = (uint64_t *)world_->relations;
			if(rels[world_->act_idx] & COOR_FLAG)
			{
				zblock_wr_append(zw_, " ",1);
				zblock_wr_append(zw_, (const char *)sax->tag_start, tag_act_size-1);
				zblock_wr_append(zw_, "=\"",2);
			}
		}
	}
	return 0;
}

int write_rel_arg_end(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;

		size_t tag_act_size = strlen((const char *)sax->tag_start);
		tag_act_size++;

		if(tag_act_size != (size_t)sax->tag_act_size)
			printf("end: sax %i, len %i [%s]\n", sax->tag_act_size, 
				(int)tag_act_size,(const char *)sax->tag_start);

		if(world_->inside_tag_flags & INSIDE_REL)
		{
			int write_it = 0;

			uint64_t * rels = (uint64_t *)world_->relations;
			if(rels[world_->act_idx] & COOR_FLAG)
			{
				write_it = 1;
			}
			if(write_it)
			{
				zblock_wr_append(&(world_->zw_out), (const char *)sax->tag_start,
					tag_act_size-1);
				zblock_wr_append(&(world_->zw_out), "\"",1);
			}
			if(world_->key_flags == WAY_ID)
			{
				if(write_it)
				{
					uint64_t act_id = atol((const char *)sax->tag_start);

					if(world_->info.rel.min_id == 0)
					{
						world_->info.rel.min_id = act_id;
						world_->info.rel.max_id = act_id;
					}
					else
					{
						if(act_id < world_->info.rel.min_id)
							world_->info.rel.min_id = act_id;
						if(act_id > world_->info.rel.max_id)
							world_->info.rel.max_id = act_id;
					}
					//printf("MIN %li %li %li\n", act_id, world_->info.rel.min_id, world_->info.rel.max_id);
					world_->act_selected++;
					
					// TODO: add id -> rels[act_idx]
					
					//world_->info.rel.count++;
				}
			}
			if(world_->inside_tag_flags & INSIDE_REL_MB)
			{
				if(write_it)
				{
				}
			}
			world_->key_flags = 0;
		}
	}
	return 0;
}

void rel_write_init_06(struct _simple_sax * xml_ref)
{
	sax_add_cb(xml_ref, write_rel_tag_open,     SAX_CB_TAG_OPEN);
	sax_add_cb(xml_ref, write_rel_tag_close,    SAX_CB_TAG_CLOSE);
	sax_add_cb(xml_ref, write_rel_arg_end,      SAX_CB_ARG_VALUE);
	sax_add_cb(xml_ref, write_rel_tag_name,     SAX_CB_TAG_NAME);
	sax_add_cb(xml_ref, write_rel_tag_arg_name, SAX_CB_ARG_NAME);
}


int writeRelations(z_block * z_read, World_t * act_world, char * w_fname)
{
	int n_read;
	unsigned char * z_buf;
	int z_size;
	simple_sax sax;
	int tag_len=0;

	sax_init(&sax, 0);
	init_write_world_rel(act_world);
	rel_write_init_06(&sax);

	sax_set_data_ref(&sax, act_world);

	if(createOsmHead(&act_world->zw_out, w_fname, ZB_WRITE))
		return -1;

	zblock_set_start(z_read, NULL, 0);

	while((n_read = zblock_read(z_read)) > 0)
	{
		if(1)   // TODO: set flag for debug output like: act_world->flags & DEBUG_1
			printf("- r-w: %ld -", act_world->act_idx);
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

	if(closeOsm(&act_world->zw_out))
		return -1;

	sax_cleanup(&sax);

	act_world->info.rel.count = act_world->act_selected;

	return 0;
}

