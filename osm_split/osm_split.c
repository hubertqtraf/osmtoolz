/******************************************************************
 * project:     Trafalgar/OSM-Toolz
 *
 * class:       ---
 * superclass:  ---
 * modul:       osm_split.c
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

void init_world_ref(World_t * w_ref)
{
	initOsmInfo(&(w_ref->info));

	w_ref->n_64_mode = 0x00;

	w_ref->type_flags = 0x00;
	w_ref->key_flags = 0x00;
	w_ref->val_flags = 0x00;
	w_ref->seg_flags = 0x00;
	w_ref->id = 0L;
	w_ref->lat = 0;
	w_ref->lon = 0;

	w_ref->world_lon = NULL;
	w_ref->way_min_lon = 0xff;
	w_ref->way_max_lon = 0xff;

	w_ref->id64_nd_lat = NULL;
	w_ref->id64_nd_lon = NULL;
	w_ref->id64_nd_lat_lon_bit9 = NULL;
	w_ref->store_x = 0;
	w_ref->store_y = 0;
	w_ref->store_id = 0;

	w_ref->count_nd = 0;
	w_ref->count_member = 0;
}


int split_tag_open(struct _simple_sax * sax)
{
	return 0;
}

int split_tag_close(struct _simple_sax * sax)
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
					zblock_wr_append(&world_->zw_node,"\t</node>\n",9);
					world_->type_flags = 0;
					return 0;
				}
			}
			if(!strncmp((const char*)sax->tag_name_start,"node",4))
			{
				if(sax->last_byte == '/')
				{
					zblock_wr_append(&world_->zw_node,"/>\n",3);
					world_->type_flags = 0;
				}
				if(world_->type_flags)
					zblock_wr_append(&world_->zw_node, ">\n",2);
			}
			if(!strncmp((const char*)sax->tag_name_start,"tag",3))
			{
				zblock_wr_append(&world_->zw_node, "/>\n",3);
			}
		}
		if(world_->type_flags == INSIDE_WAY)
		{
			if(!strncmp((const char*)sax->tag_start,"way",3))
			{
				if(sax->inside_mask & SAX_TAG_END_FLAG)
				{
					zblock_wr_append(&(world_->zw_way), "\t</way>\n",8);
					world_->type_flags = 0;

					if(world_->info.way.member < world_->count_nd)
					{
						world_->info.way.member = world_->count_nd;
						printf("\nway max node count: %ld\n",world_->info.way.member);
					}
					world_->count_nd = 0;

					return 0;
				}
			}

			if(sax->tag_name_start)
			{
				if(!strncmp((const char*)sax->tag_name_start,"nd",2))
				{
					zblock_wr_append(&(world_->zw_way), "/>\n",3);
					world_->count_nd++;
					return 0;
				}
				if(!strncmp((const char*)sax->tag_name_start,"seg",3))
				{
					zblock_wr_append(&(world_->zw_way), "/>\n",3);
					return 0;
				}
				if(!strncmp((const char*)sax->tag_name_start,"tag",3))
				{
					zblock_wr_append(&(world_->zw_way), "/>\n",3);
					return 0;
				}
				if(!strncmp((const char*)sax->tag_name_start,"way",3))
				{
					if(sax->last_byte == '/')
					{
						zblock_wr_append(&(world_->zw_way), "/>\n",3);
						world_->type_flags = 0;
					}
				}
				if(world_->type_flags)
					zblock_wr_append(&(world_->zw_way), ">\n",2);
			}
			else
				printf("sax->tag_name_start = NULL\n");
		}
		if(world_->type_flags == INSIDE_REL)
		{
			if(!strncmp((const char*)sax->tag_start,"relation",8))
			{
				if(sax->inside_mask & SAX_TAG_END_FLAG)
				{
					zblock_wr_append(&(world_->zw_rel), "\t</relation>\n",13);
					if(world_->info.rel.member < world_->count_member)
					{
						world_->info.rel.member = world_->count_member;
						printf("\nrelation member max count: %li\n", world_->info.rel.member);
					}
					world_->count_member = 0;
					world_->type_flags = 0;
					return 0;
				}
			}
			if(sax->tag_name_start)
			{
				if(!strncmp((const char*)sax->tag_name_start,"member",6))
				{
					zblock_wr_append(&(world_->zw_rel), "/>\n",3);
					world_->count_member++;
					return 0;
				}
				if(!strncmp((const char*)sax->tag_name_start,"tag",3))
				{
					zblock_wr_append(&(world_->zw_rel), "/>\n",3);
					return 0;
				}
				if(!strncmp((const char*)sax->tag_name_start,"relation",8))
				{
					if(sax->last_byte == '/')
					{
						zblock_wr_append(&(world_->zw_rel), "/>\n",3);
						if(world_->info.rel.member < world_->count_member)
						{
							world_->info.rel.member = world_->count_member;
							world_->count_member = 0;
						}
						world_->type_flags = 0;
					}
				}
				if(world_->type_flags)
				{
					zblock_wr_append(&world_->zw_rel, ">\n",2);
				}
			}
			else
				printf("sax->tag_name_start = NULL\n");
		}
	}
	return 0;
}

int split_tag_name(struct _simple_sax * sax)
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

		switch (tag_act_size)
		{
		case 3:
			if(sax->tag_start[0] == 'n')
			{
				if(world_->type_flags == INSIDE_WAY)
					world_->key_flags = WAY_NODE_REF;
			}
			break;

		case 4:		// tag, way
			if(sax->tag_start[0] == 'w')
			{
				if(world_->type_flags == INSIDE_WAY)
				{
					printf("Error: already inside way [%s] \n",
						sax->tag_start);
				}
				world_->type_flags = INSIDE_WAY;
				zw_ = &(world_->zw_way);
				world_->count_nd = 0;
			}
			if(sax->tag_start[0] == 's')
			{
				if(world_->type_flags == INSIDE_WAY)
					world_->seg_flags = ND_SEG;
			}
			else
				world_->seg_flags = 0x00;
			break;

		case 5:		// node
			if((sax->tag_start[0] == 'n') && (sax->tag_start[1] == 'o'))
			{
				if(world_->type_flags == INSIDE_NODE)
				{
					printf("Error: already inside node [%s] [%s] %ld\n",sax->tag_start,
						sax->tag_name_start,world_->id);
				}
				world_->type_flags = INSIDE_NODE;
				zw_ = &(world_->zw_node);
			}
			break;

		case 9:			// relation
			if(sax->tag_start[0] == 'r')
			{
				if(world_->type_flags == INSIDE_REL)
				{
					printf("Error: already inside relation [%s] \n",
						sax->tag_start);
				}
				world_->type_flags = INSIDE_REL;
				zw_ = &(world_->zw_rel);
			}
			break;

		default:
			break;
		}

		if((world_->type_flags == INSIDE_WAY) && (zw_ == NULL))
		{
			zblock_wr_append(&(world_->zw_way), "\t\t<", 3);
			zblock_wr_append(&(world_->zw_way), (const char *)sax->tag_start,
				tag_act_size-1);
		}
		if((world_->type_flags == INSIDE_REL) && (zw_ == NULL))
		{
			zblock_wr_append(&(world_->zw_rel), "\t\t<", 3);
			zblock_wr_append(&(world_->zw_rel), (const char *)sax->tag_start,
				tag_act_size-1);
		}
		if((world_->type_flags == INSIDE_NODE) && (zw_ == NULL))
		{
			zblock_wr_append(&(world_->zw_node), "\t\t<", 3);
			zblock_wr_append(&(world_->zw_node), (const char *)sax->tag_start,
				tag_act_size-1);
		}

		if(zw_ != NULL)
		{
			zblock_wr_append(zw_, "\t<", 2);
			zblock_wr_append(zw_, (const char *)sax->tag_start,
				tag_act_size-1);
			zblock_wr_append(zw_, " ", 1);
		}
	}
	return 0;
}



int split_tag_arg_name(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		//printf("3 %3i |%s| \n", sax->tag_act_size, sax->tag_start);

		World_t * world_ = (World_t *)sax->d_ref;
		uint8_t key_flags_ = 0x00;
		//uint8_t type_flags_ = 0x00;
		z_block * zw_ = NULL;

		size_t tag_act_size = strlen((const char*) sax->tag_start);
		tag_act_size++;

		if(tag_act_size != (size_t)sax->tag_act_size)
			printf("arg: sax %i, len %i [%s]\n", sax->tag_act_size,
				(int)tag_act_size,(const char *)sax->tag_start);

		switch (tag_act_size)
		{

		case 3:		// id, to
			if((world_->type_flags == INSIDE_NODE) ||
				(world_->type_flags == INSIDE_REL))
			{
				if(sax->tag_start[0] == 'i')
					key_flags_ = ND_ID;
			}
			if(world_->type_flags == INSIDE_WAY)
			{
				if(world_->seg_flags != ND_SEG)
				{
					if(sax->tag_start[0] == 'i')
						key_flags_ = ND_ID;
				}
			}
			break;

		case 4:		// lat, lon
			if((sax->tag_start[0] == 'l') && (world_->type_flags == INSIDE_NODE))
			{
				key_flags_ = ND_COOR;
				if(sax->tag_start[1] == 'o')
					key_flags_ |= ND_LON;
			}
			break;

		default:
			break;
		}

		if(key_flags_)
			world_->key_flags = key_flags_;

		if(world_->type_flags == INSIDE_NODE)
			zw_ = &world_->zw_node;

		if(world_->type_flags == INSIDE_WAY)
			zw_ = &world_->zw_way;

		if(world_->type_flags == INSIDE_REL)
			zw_ = &world_->zw_rel;

		if(zw_ != NULL)
		{
			zblock_wr_append(zw_, " ",1);
			zblock_wr_append(zw_, (const char *)sax->tag_start, tag_act_size-1);
			zblock_wr_append(zw_, "=\"",2);
		}
	}
	return 0;
}

int split_arg_end(struct _simple_sax * sax)
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
			zblock_wr_append(&world_->zw_node, (const char *)sax->tag_start,
				tag_act_size-1);
			zblock_wr_append(&world_->zw_node, "\"",1);

			if(world_->key_flags == ND_ID)
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
				// TODO: is count used? --> nodes of way?
				world_->info.node.count++;
			}
			world_->key_flags = 0;
		}
		if(world_->type_flags == INSIDE_WAY)
		{
			zblock_wr_append(&(world_->zw_way), (const char *)sax->tag_start,
				tag_act_size-1);
			zblock_wr_append(&(world_->zw_way), "\"",1);

			if(world_->key_flags == ND_ID)
			{
				world_->id = atol((const char *)sax->tag_start);

				if(world_->info.way.min_id == 0)
				{
					world_->info.way.min_id = world_->id;
					world_->info.way.max_id = world_->id;
				}
				else
				{
					if(world_->id < world_->info.way.min_id)
						world_->info.way.min_id = world_->id;
					if(world_->id > world_->info.way.max_id)
						world_->info.way.max_id = world_->id;
				}
				// TODO: is count used? --> nodes of way?
				world_->info.way.count++;
			}
			/*if(world_->key_flags == WAY_NODE_REF)
			{
			}*/
			world_->key_flags = 0;
		}
		if(world_->type_flags == INSIDE_REL)
		{
			size_t len = strlen((const char *)sax->tag_start);
			if(len)
				zblock_wr_append(&(world_->zw_rel), (const char *)sax->tag_start, len);
			zblock_wr_append(&(world_->zw_rel), "\"",1);

			if(world_->key_flags == ND_ID)
			{
				world_->id = atol((const char *)sax->tag_start);
				if(world_->info.rel.min_id == 0)
				{
					world_->info.rel.min_id = world_->id;
					world_->info.rel.max_id = world_->id;
				}
				else
				{
					if(world_->id < world_->info.rel.min_id)
						world_->info.rel.min_id = world_->id;
					if(world_->id > world_->info.rel.max_id)
						world_->info.rel.max_id = world_->id;
				}
				// TODO: is count used? --> nodes of way?
				world_->info.rel.count++;
			}
			world_->key_flags = 0;
		}
	}
	return 0;
}

void node_count_init_06(struct _simple_sax * xml_ref)
{
	sax_add_cb(xml_ref, split_tag_open,     SAX_CB_TAG_OPEN);
	sax_add_cb(xml_ref, split_tag_close,    SAX_CB_TAG_CLOSE);
	sax_add_cb(xml_ref, split_arg_end,      SAX_CB_ARG_VALUE);
	sax_add_cb(xml_ref, split_tag_name,     SAX_CB_TAG_NAME);
	sax_add_cb(xml_ref, split_tag_arg_name, SAX_CB_ARG_NAME);
}

int countNodes(z_block * z_read, World_t * act_world, uint8_t n_64_mode, uint8_t flags)
{
	int n_read;
	unsigned char * z_buf;
	int z_size;
	simple_sax sax;
	int tag_len=0;

	sax_init(&sax, act_world->accept);
	init_world_ref(act_world);
	act_world->n_64_mode = n_64_mode;
	node_count_init_06(&sax);

	sax_set_data_ref(&sax, act_world);

	printf("act_world->n_64_mode %i\n", act_world->n_64_mode);

	if(createOsmHead(&act_world->zw_node, "./result/node_20.osm.gz", flags)) //ZB_WRITE | ZB_USE_W_THREAD))
		return -1;
	if(act_world->n_64_mode)
		createOsmHead(&act_world->zw_way, "./result/way_32_20.osm.gz", flags); //ZB_WRITE | ZB_USE_W_THREAD);
	else
		createOsmHead(&act_world->zw_way, "./result/way_20.osm.gz", flags); //ZB_WRITE | ZB_USE_W_THREAD);
	if(createOsmHead(&act_world->zw_rel, "./result/rel_20.osm.gz", flags)) //ZB_WRITE | ZB_USE_W_THREAD))
		return -1;

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

	closeOsm(&act_world->zw_node);
	closeOsm(&act_world->zw_way);
	closeOsm(&act_world->zw_rel);

	sax_cleanup(&sax);

	return 0;
}

int countNodesOut(z_block * z_read, World_t * act_world, char * out, uint8_t flags)
{
	int n_read;
	unsigned char * z_buf;
	int z_size;
	simple_sax sax;
	int tag_len=0;

	sax_init(&sax, act_world->accept);
	init_world_ref(act_world);
	//act_world->n_64_mode = n_64_mode;
	node_count_init_06(&sax);

	sax_set_data_ref(&sax, act_world);

	char * fname = (char *)malloc(strlen(out) + 100);

	sprintf(fname, "%s_node_20.osm.gz", out);
	if(createOsmHead(&act_world->zw_node, fname, flags)) //ZB_WRITE | ZB_USE_W_THREAD))
		return -1;

	sprintf(fname, "%s_way_20.osm.gz", out);
	if(createOsmHead(&act_world->zw_way, fname, flags)) //ZB_WRITE | ZB_USE_W_THREAD))
		return -1;

	sprintf(fname, "%s_rel_20.osm.gz", out);
	if(createOsmHead(&act_world->zw_rel, fname, flags)) //ZB_WRITE | ZB_USE_W_THREAD))
		return -1;

	//printf("act_world->n_64_mode %i\n", act_world->n_64_mode);
	//system("rm ./result/*.gz");

	//open_split_file(&act_world->zw_node, "./result/node_20.osm.gz");
	//if(act_world->n_64_mode)
	//	open_split_file(&act_world->zw_way, "./result/way_32_20.osm.gz");
	//else
	//	open_split_file(&act_world->zw_way, "./result/way_20.osm.gz");
	//open_split_file(&act_world->zw_rel, "./result/rel_20.osm.gz");

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

	closeOsm(&act_world->zw_node);
	closeOsm(&act_world->zw_way);
	closeOsm(&act_world->zw_rel);

	sax_cleanup(&sax);

	free(fname);

	return 0;
}

