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

#include "osm_cut.h"
#include "../common/simple_sax.h"
#include "../common/osm_info.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h> 


int64_t findWay(uint64_t * ways, uint64_t n_way, uint64_t id)
{
	int64_t pos1=0;
	int64_t pos2=n_way-1;
	int64_t d;

	if((id > (ways[n_way-1] & WAY_ID_MASK)) || 
		(id < (ways[0] & WAY_ID_MASK)))
		return -1;

	while((ways[pos2] & WAY_ID_MASK) != id)
	{
		//printf("POS %ld %ld | %ld %ld %ld\n",pos1,pos2,ways[pos1].id, ways[pos2].id, id);
		if((ways[pos1]  & WAY_ID_MASK) == id)
			return pos1;

		d = ((pos2-pos1) >> 1);

		if(d == 0)
			return -1;

		if(id < (ways[pos1+d]  & WAY_ID_MASK))
			pos2 -= d;
		else
			pos1 += d;
	}
	return pos2;
}

int init_world_rel(World_t * w_ref)
{
	w_ref->inside_tag_flags = 0x00;
	w_ref->key_flags = 0x00;

	w_ref->act_idx = 0;	

	// setting 0 to the id -> should be OK, id '0' is not in planet file ...
	w_ref->member_id = 0;
        w_ref->rel_type_flags = 0;

	// TODO: not needed here? info.rel.member is the 'old/bigger' data set 
	if((w_ref->member_ref_list = (uint64_t*)malloc(sizeof(uint64_t*) * w_ref->info.rel.member)) == NULL)
	{
		printf("unable to allocate node buffer 'w_ref->node_ref_list'\n");
		return 1;
	}
	memset(w_ref->member_ref_list, 0x00, sizeof(uint64_t*) * w_ref->info.rel.member);

	//if(w_ref->store_type & WAY_STORE_TYPE_ID)
	{
		if((w_ref->relations = (uint8_t*)malloc(w_ref->size_rel * w_ref->info.rel.count)) == NULL)
		{
			printf("unable to allocate relations id buffer 'w_ref->relations'\n");
			return 1;
		}
		memset(w_ref->relations, 0x00, (w_ref->size_rel * w_ref->info.rel.count));
	}

	w_ref->test_member = 0;
	// printf("-> info relation: %li\n", w_ref->info.rel.member);

	return(0);
}

void way_mark_inside(World_t * world, uint64_t id)
{
	//if(world->store_type & WAY_STORE_TYPE_ID)
	{
		int64_t w_idx = findWay((uint64_t *)world->ways, world->info.way.count, id);

		//printf("{way: %li %li}", id, w_idx);

		if(w_idx != -1)
		{
			uint64_t * ways = (uint64_t *)world->ways;
			if(ways[w_idx] & COOR_FLAG)
			{
				//printf("\n===> way_mark_inside: %li %li\n", id, w_idx);
				uint64_t * rels = (uint64_t *)world->relations;
				rels[world->act_idx] |= COOR_FLAG; 
			}
		}
	}
}

void close_member(World_t * world)
{
	if(world->rel_type_flags == REL_MEMBER_WAY)
	{	
		way_mark_inside(world, world->member_id);
	}
	world->member_id = 0;
	world->rel_type_flags = 0;
	world->count_member++;
}


void close_relation(World_t * world)
{
	world->inside_tag_flags = 0;
	world->key_flags = 0;
	world->rel_type_flags = 0;

	world->act_idx++;

	if(world->test_member < world->count_member)
	{
		world->test_member = world->count_member;
		//printf("(%li %li)\n", world->test_member, world->count_member);
		world->count_member = 0;
	}
}


int load_rel_tag_open(struct _simple_sax * sax)
{
	return 0;
}


int load_rel_tag_close(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		//printf("e[%s]",sax->tag_start);

		World_t * world_ = (World_t *)sax->d_ref;

		if(world_->inside_tag_flags & INSIDE_REL)
		{
			if(!strncmp((const char*)sax->tag_start, "relation", 8))
			{
				if(sax->inside_mask & SAX_TAG_END_FLAG)
				{
					close_relation(world_);
					return 0;
				}
			}
			if(!strncmp((const char*)sax->tag_start, "member", 6))
			{
				if(sax->inside_mask & SAX_TAG_END_FLAG)
				{
					world_->inside_tag_flags  &= ~(INSIDE_REL_MB); //member

					close_member(world_);
				}	
			}
			if(!strncmp((const char*)sax->tag_start, "tag", 3))
			{
				if(sax->inside_mask & SAX_TAG_END_FLAG)
				{
					world_->inside_tag_flags  &= ~(INSIDE_TAG_);

					//world_->rel_type_flags = 0;
				}
			}
			if(sax->tag_name_start)
			{
				if(!strncmp((const char*)sax->tag_name_start, "relation", 8))
				{	
					if(sax->last_byte == '/')
					{
						close_relation(world_);
					}
				}
				if(!strncmp((const char*)sax->tag_name_start, "member", 6))
				{
					if(sax->last_byte == '/')
					{
						world_->inside_tag_flags  &= ~(INSIDE_REL_MB);

						close_member(world_);
					}
				}
				if(!strncmp((const char*)sax->tag_name_start, "tag", 3))
				{
					if(sax->last_byte == '/')
					{
						world_->inside_tag_flags  &= ~(INSIDE_TAG_);
						world_->key_flags = 0;
						//world_->rel_type_flags = 0;
					}
				}
			}
		}
	}
	return 0;
}

int load_rel_tag_name(struct _simple_sax * sax)
{

	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;
		//z_block * zw_ = NULL;

		size_t tag_act_size = strlen((const char *)sax->tag_start);
		tag_act_size++;	

		if(tag_act_size != (size_t)sax->tag_act_size)
			printf("tag: sax %i, len %i [%s]\n", sax->tag_act_size, 
				(int)tag_act_size,(const char *)sax->tag_start);

		switch (sax->tag_act_size)
		{
		case 4:			// tag 
			if(sax->tag_start[0] == 't')
			{
				if(world_->inside_tag_flags & INSIDE_REL)
				{
					//world_->key_flags = REL_TAG;
					world_->inside_tag_flags |= INSIDE_TAG_;
				}	
			}
			break;

		case 7:			// member
			if(sax->tag_start[0] == 'm')
			{
				if(world_->inside_tag_flags & INSIDE_REL)
				{
					//world_->key_flags = REL_MEMBER;  //WAY_NODE_REF;	// TODO...
					world_->inside_tag_flags |= INSIDE_REL_MB;
				}
			}
			break;

		case 9:			// relation
			if(sax->tag_start[0] == 'r')
			{
				if(world_->inside_tag_flags & INSIDE_REL)
				{
					printf("Error: already inside relation [%s] \n",
						sax->tag_start);
				}
				world_->inside_tag_flags |= INSIDE_REL;
			}
			break;
			
		default:
			break;	
		}
	}
	return 0;
}



int load_rel_tag_arg_name(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		World_t * world_ = (World_t *)sax->d_ref;
		
		size_t tag_act_size = strlen((const char*) sax->tag_start);
		tag_act_size++;

		if(tag_act_size != (size_t)sax->tag_act_size)
			printf("arg: sax %i, len %i [%s]\n", sax->tag_act_size, 
				(int)tag_act_size,(const char *)sax->tag_start);

		// type="way" ref="82900775" role="outer"

		switch (sax->tag_act_size)
		{
		case 2:		// "k", "v" 
			if((world_->inside_tag_flags & INSIDE_REL) && 
				(world_->inside_tag_flags & INSIDE_TAG_))
			{
				if(sax->tag_start[0] == 'k')
					world_->key_flags |= REL_K;
				if(sax->tag_start[0] == 'v')
					world_->key_flags |= REL_V;
			}
			break;

		case 3:		// id, to
			if(world_->inside_tag_flags & INSIDE_REL)
			{
				if(!(world_->inside_tag_flags & INSIDE_REL_MB))
				//if(world_->key_flags != REL_MEMBER) //WAY_NODE_REF)	// TODO ..-
				{
					if(sax->tag_start[0] == 'i')
					{	
						world_->key_flags |= REL_ID;
					}	
				}
			}
			break;

		case 4:		// ref
			if((world_->inside_tag_flags & INSIDE_REL) && 
				(world_->inside_tag_flags & INSIDE_REL_MB))
			{
				if((sax->tag_start[0] == 'r') && (sax->tag_start[1] == 'e'))
				{
					world_->key_flags |= REL_REF;
				}
			}
			break;

		case 5:		// role, type
			if((world_->inside_tag_flags & INSIDE_REL) && 
				(world_->inside_tag_flags & INSIDE_REL_MB))
			{
				if(sax->tag_start[0] == 'r')
				{
					world_->key_flags |= REL_ROLE;
				}	
				if(sax->tag_start[0] == 't')
				{
					world_->key_flags |= REL_TYPE;
				}
			}
			break;

		default:
			break;
		}
	}
	return 0;
}

int load_rel_arg_end(struct _simple_sax * sax)
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
			if(world_->key_flags & REL_ID)
			{
				// TODO: relation ID -> store

				if(world_->act_idx >= world_->info.rel.max_id) //count)
				{
					printf("%s %i: relation id out of range %ld  %ld\n",
						__func__, __LINE__, (int64_t)world_->act_idx,
						world_->info.rel.count);
					exit(0);
				}
				else
				{
					//world_->relations[world_->act_idx].id =
					//	atol((const char *)sax->tag_start);

					//printf("id: %ld %s\n", world_->act_idx, sax->tag_start);

					world_->key_flags = 0;
					//world_->rel_type_flags = 0;
				}
			}
			if(world_->inside_tag_flags & INSIDE_REL_MB)
			{
				if(world_->key_flags & REL_REF)
				{
					// move to end of 'member'
					uint64_t id = atol((const char *)sax->tag_start);
					world_->key_flags  &= ~(REL_REF);
					world_->member_id = id;
					//way_mark_inside(world_ , id);
				}	

				if(world_->key_flags & REL_TYPE)
				{
					switch (sax->tag_act_size)
					{
					case 4:
						if(sax->tag_start[0] == 'w')
							world_->rel_type_flags = REL_MEMBER_WAY;
						break;

					case 5:
						if(sax->tag_start[0] == 'n')
							world_->rel_type_flags = REL_MEMBER_NODE;
						break;

					case 9:
						if(sax->tag_start[0] == 'r')
							world_->rel_type_flags = REL_MEMBER_REL;
						break;

					default:
						world_->rel_type_flags = 0x00;
						break;
					}
					//w_ref->seg_flags
					//#define REL_MEMBER_NODE 0x01 "node"
					//#define REL_MEMBER_WAY  0x02 "way"
					//#define REL_MEMBER_REL  0x04 "relation"
					
					world_->key_flags  &= ~(REL_TYPE);
				}

				/*if(world_->key_flags & REL_REF)	
				{
					if(world_->rel_type_flags == REL_MEMBER_WAY)
					{
						uint64_t id = atol((const char *)sax->tag_start);

						way_mark_inside(world_ , id);

				}*/
				

				// type="way" ref="82900775" role="outer"
				//uint64_t node_ref = atol((const char*)sax->tag_start);

				// TODO: check id -> all ways -> mark

			}
			if(world_->inside_tag_flags |= INSIDE_TAG_)
			{
				// +++ if(world_->val_flags == REL_K)
				{
				}
				// +++ if(world_->val_flags == REL_V)
				{
				}
			}
			// +++ world_->val_flags = 0;
		}
	}
	return 0;
}

void rel_count_init_06(struct _simple_sax * xml_ref)
{
	sax_add_cb(xml_ref, load_rel_tag_open,     SAX_CB_TAG_OPEN);
	sax_add_cb(xml_ref, load_rel_tag_close,    SAX_CB_TAG_CLOSE);
	sax_add_cb(xml_ref, load_rel_arg_end,      SAX_CB_ARG_VALUE);
	sax_add_cb(xml_ref, load_rel_tag_name,     SAX_CB_TAG_NAME);
	sax_add_cb(xml_ref, load_rel_tag_arg_name, SAX_CB_ARG_NAME);
}

int readRelations(z_block * z_read, World_t * act_world)
{
	int n_read;
	unsigned char * z_buf;
	int z_size;
	simple_sax sax;
	int tag_len=0;

	sax_init(&sax, 0);

	if(init_world_rel(act_world) != 0)
	{
		return 1;
	}

	rel_count_init_06(&sax);

	sax_set_data_ref(&sax, act_world);

	zblock_set_start(z_read, NULL, 0);

	while((n_read = zblock_read(z_read)) > 0)
	{
		if(1)   // TODO: set flag for debug output like: act_world->flags & DEBUG_1
			printf("- r-r: %ld -", act_world->act_idx);

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

	if(act_world->test_member > act_world->info.rel.member)
	{
		printf("Warning: test %li is greater as %li\n", 
			act_world->test_member, act_world->info.rel.member);
		free(act_world->member_ref_list);
		if((act_world->member_ref_list = (uint64_t*)malloc(sizeof(uint64_t*) * 
						act_world->test_member)) == NULL)
		{
			return -1;
		}
	}

	//tmp
	//printSels(act_world);

	sax_cleanup(&sax);

	return 0;
}

