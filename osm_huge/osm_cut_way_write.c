/******************************************************************
 * project:     Trafalgar/OSM-Toolz 
 *
 * modul:      	osm_cut_way_write.c 
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
//#include "read_types.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h> 

uint64_t check_way(World_t * world)
{
	/*uint8_t * pos = world->way_flags + (world->act_idx *//** world->size_way*//*);*/

	// TODO: set the number of id-bytes from 8 to 5 -> check
	//uint64_t * n_id = (uint64_t *)pos;

	//return ((*n_id) & COOR_FLAG);
	//printf("%i %i,", world->act_idx, *pos);
	//printf("%i %i,", world->act_idx, world->way_flags[world->act_idx]);
	//printf("%i,", world->way_flags[world->act_idx]);
	if(world->way_flags[world->act_idx] == 1)
		return COOR_FLAG;
	return 0;
}


// tmp test
void test_way(World_t * w_ref)
{
	printf("-- %li --\n", w_ref->info.way.count);

	for(int i =0; i<w_ref->info.way.count; i++)
	{
		// TODO: set the number of id-bytes from 8 to 5 -> check
		uint8_t * pos = w_ref->way_flags + (i /** w_ref->size_way*/);

		uint64_t * n_id = (uint64_t *)pos;

		printf("%i %lx %li %lx\n", i, *n_id, (*n_id) & 0x0000000fffffffff, (*n_id) & COOR_FLAG);
	}
	//printf("== %li %li ==\n", w_ref->info.way.count, w_ref->act_selected);
}


void init_write_world_way(World_t * w_ref)
{
	w_ref->act_idx = 0;

	w_ref->type_flags = 0x00;
	w_ref->key_flags = 0x00;

	w_ref->act_idx = 0;
	w_ref->act_selected = 0;

	w_ref->info.way.min_id = 0;
	w_ref->info.way.max_id = 0;

	w_ref->info.way.member = 0;
	w_ref->act_member_idx = 0;

	//w_ref->info.way.count = 0;
	//w_ref->info.way.n_id = 0;


	/*w_ref->type_flags = 0x00;
	w_ref->key_flags = 0x00;
	w_ref->val_flags = 0x00;
	w_ref->seg_flags = 0x00;

	w_ref->info.way.min_id = 0;

	// max. id of source data needed for boudary check
	w_ref->flat_max_way_id = w_ref->info.way.max_id;
	w_ref->flat_max_node_id = w_ref->info.node.max_id;

	w_ref->info.way.max_id = 0;
	w_ref->info.way.count = 0;
	w_ref->info.way.n_id = 0;

	w_ref->act_idx = 0;*/
}


int write_way_tag_open(struct _simple_sax * sax)
{
	return 0;
}

int write_way_tag_close(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;

		//printf("+ %i %i,", world_->act_idx, world_->way_flags[world_->act_idx]);

		if(world_->type_flags & INSIDE_WAY)
		{
			if(!strncmp((const char*)sax->tag_start,"way",3))
			{
				if(sax->inside_mask & SAX_TAG_END_FLAG)
				{
					// xxx world_->type_flags = 0;
					world_->type_flags  &= ~(INSIDE_WAY);
					//printf("%ld\n", check_way(world_));
					if(check_way(world_) == COOR_FLAG)
					{
						if(world_->act_member_idx > world_->info.way.member)
						{
							world_->info.way.member  = world_->act_member_idx;
							//world_->info.rel.member = world_->rel_member;
						}
						/*printf("%ld %ld %i\n", world_->info.way.count,
								world_->info.way.member,
								world_->act_member_idx);*/

						world_->act_member_idx = 0;

						// xxx if(world_->ways[world_->act_idx].id & INSIDE_FLAG)
						zblock_wr_append(&(world_->zw_out), "\t</way>\n",8);
					}
					world_->act_idx++;
					return 0;
				}
			}
			if(!strncmp((const char*)sax->tag_start,"nd",2))
			{
				if(sax->inside_mask & SAX_TAG_END_FLAG)
				{
					if(check_way(world_) == COOR_FLAG)
						world_->act_member_idx++;
				}
			}
			if(sax->tag_name_start)
			{
				int write_it = 0;

				if(check_way(world_) == COOR_FLAG)
					// xxx if(world_->ways[world_->act_idx].id & INSIDE_FLAG)
					write_it = 1;

				if(!strncmp((const char*)sax->tag_name_start,"nd",2))
				{
					if(write_it)
					{
						world_->act_member_idx++;
						zblock_wr_append(&(world_->zw_out), "/>\n",3);
					}
					return 0;
				}
				if(!strncmp((const char*)sax->tag_name_start,"seg",3))
				{
					if(write_it)
						zblock_wr_append(&(world_->zw_out), "/>\n",3);
					return 0;
				}
				if(!strncmp((const char*)sax->tag_name_start,"tag",3))
				{
					if(write_it)
						zblock_wr_append(&(world_->zw_out), "/>\n",3);
					return 0;
				}
				if(!strncmp((const char*)sax->tag_name_start,"way",3))
				{	
					if(sax->last_byte == '/')
					{
						// xxx world_->type_flags = 0;
						//if(world_->ways[world_->act_idx].id & INSIDE_FLAG)
						if(write_it)
						{
							if(world_->act_member_idx > world_->info.way.member)
							{
								world_->info.way.member  = world_->act_member_idx;
							}
							//printf("%ld %ld %i\n", world_->info.way.count,
							//	world_->info.way.member, world_->act_member_idx);

							world_->act_member_idx = 0;

							zblock_wr_append(&(world_->zw_out), "/>\n",3);
						}
						world_->act_idx++;
					}
				}
				// xxx if(world_->type_flags)
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

int write_way_tag_name(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		//printf(".%s.", sax->tag_start);

		World_t * world_ = (World_t *)sax->d_ref;
		z_block * zw_ = NULL;

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
				if(world_->type_flags & INSIDE_WAY)
				// if(world_->type_flags == INSIDE_WAY)
					//world_->seg_flags = ND_SEG;
					// xxx world_->key_flags = WAY_NODE_REF;
					;
			}
			break;

		case 4:		// tag, way
			if(sax->tag_start[0] == 'w')
			{
				if(world_->type_flags & INSIDE_WAY)
				// if(world_->type_flags == INSIDE_WAY)
				{
					printf("Error: write_way_tag_name, already inside way [%s] %li\n",
						sax->tag_start, world_->info.way.max_id);
				}
				// world_->type_flags = INSIDE_WAY;
				//world_->inside_tag_flags  &= ~(INSIDE_WAY);
				world_->type_flags |= INSIDE_WAY;
				zw_ = &(world_->zw_out);
			}
			break;
			
		default:
			break;	
		}

		int write_it = 0;
		if(check_way(world_) == COOR_FLAG)
			// xxx if(world_->ways[world_->act_idx].id & INSIDE_FLAG)
			write_it = 1;

		if((world_->type_flags & INSIDE_WAY) && (zw_ == NULL))
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



int write_way_tag_arg_name(struct _simple_sax * sax)
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
			if(world_->type_flags & INSIDE_WAY)
			//if(world_->type_flags == INSIDE_WAY)
			{
				// xxx if(world_->key_flags != WAY_NODE_REF)
				{
					if(sax->tag_start[0] == 'i')
						world_->key_flags = WAY_ID;
				}
			}
			break;

		case 4:
			if((sax->tag_start[0] == 'r') && (world_->type_flags & INSIDE_WAY))
			// if((sax->tag_start[0] == 'r') && (world_->type_flags == INSIDE_WAY))
			// xxx	world_->key_flags = WAY_NODE_REF;
				;
			break;

		default:
			break;
		}

		//if(world_->type_flags == INSIDE_WAY)
		if(world_->type_flags & INSIDE_WAY)
			zw_ = &world_->zw_out;

		if(zw_ != NULL)
		{
			if(check_way(world_) == COOR_FLAG)
			// xxx if(world_->ways[world_->act_idx].id & INSIDE_FLAG)
			{
				zblock_wr_append(zw_, " ",1);
				zblock_wr_append(zw_, (const char *)sax->tag_start, tag_act_size-1);
				zblock_wr_append(zw_, "=\"",2);
			}
		}
	}
	return 0;
}

int write_way_arg_end(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;

		size_t tag_act_size = strlen((const char *)sax->tag_start);
		tag_act_size++;

		if(tag_act_size != (size_t)sax->tag_act_size)
			printf("end: sax %i, len %i [%s]\n", sax->tag_act_size, 
				(int)tag_act_size,(const char *)sax->tag_start);
		if(world_->type_flags & INSIDE_WAY)
		{
			int write_it = 0;
			if(check_way(world_) == COOR_FLAG)
				//if(world_->ways[world_->act_idx].id & INSIDE_FLAG)
				write_it = 1;
			if(write_it)
			{
				zblock_wr_append(&(world_->zw_out), (const char *)sax->tag_start,
					tag_act_size-1);
				zblock_wr_append(&(world_->zw_out), "\"",1);
			}
			if(world_->key_flags == WAY_ID)
			{
				{
					//printf("-- %lld %lld\n",world_->id, world_->next_id);
					//world_->next_id++;	
				}

				if(write_it)
				{
					uint64_t act_id = atol((const char *)sax->tag_start);

					//printf("min max %li %li %li\n", act_id, world_->info.way.min_id, world_->info.way.max_id);

					if(world_->info.way.min_id == 0)
					{
						world_->info.way.min_id = act_id;
						world_->info.way.max_id = act_id;
					}
					else
					{
						if(act_id < world_->info.way.min_id)
							world_->info.way.min_id = act_id;
						if(act_id > world_->info.way.max_id)
							world_->info.way.max_id = act_id;
					}
					//world_->info.way.count++;
					world_->act_selected++;
				}
			}
			// xxx if(world_->key_flags == WAY_NODE_REF)
			{
				//if(world_->ways[world_->act_idx].id & INSIDE_FLAG)
				if(write_it)
				{
					// xxx uint64_t n_id = atol((const char*)sax->tag_start);

					//if(world_->store_type & NODE_STORE_TYPE_ID)
					//if(world_->node_store_type == NODE_STORE_TYPE_ID)
					{
						// xxx int64_t n_idx = findNodeX(world_->nodes, 
						// xxx	world_->info.node.count, n_id);
						// xxx if(n_idx != (-1))
						// xxx	world_->nodes[n_idx].id |= COOR_FLAG; 
						// xxx else
						// xxx	printf("ref %lld not found\n",(long long int)n_id);
					}
					/*else
					{
						if(n_id <= world_->flat_max_node_id)
							world_->n_flags[n_id] |= FLAT_INSIDE_FLAG;
						// add coor flag to nodes
					}*/
				}
			}
			world_->key_flags = 0;
		}
	}
	return 0;
}

void way_write_init_06(struct _simple_sax * xml_ref)
{
	sax_add_cb(xml_ref, write_way_tag_open,     SAX_CB_TAG_OPEN);
	sax_add_cb(xml_ref, write_way_tag_close,    SAX_CB_TAG_CLOSE);
	sax_add_cb(xml_ref, write_way_arg_end,      SAX_CB_ARG_VALUE);
	sax_add_cb(xml_ref, write_way_tag_name,     SAX_CB_TAG_NAME);
	sax_add_cb(xml_ref, write_way_tag_arg_name, SAX_CB_ARG_NAME);
}


int writeWays(z_block * z_read, World_t * act_world, char * w_fname)
{
	int n_read;
	unsigned char * z_buf;
	int z_size;
	simple_sax sax;
	int tag_len=0;

	printf("write_ %s\n", w_fname);

	sax_init(&sax, 0);
	init_write_world_way(act_world);
	way_write_init_06(&sax);

	sax_set_data_ref(&sax, act_world);

	if(createOsmHead(&act_world->zw_out, w_fname, ZB_WRITE))
		return -1;

	zblock_set_start(z_read, NULL, 0);

	while((n_read = zblock_read(z_read)) > 0)
	{
		if(1)   // TODO: set flag for debug output like: act_world->flags & DEBUG_1
			printf("- w-w: %ld -", act_world->act_idx);
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

	//test_way(act_world);

	act_world->info.way.count = act_world->act_selected;

	return 0;
}

