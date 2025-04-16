/******************************************************************
 * project:	Trafalgar/OSM-Toolz/osm_cut
 *
 * modul:	main.c
 *
 * system:	UNIX/LINUX
 * compiler:	gcc
 *
 * beginning:	11.2012
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

#include "../common/cmd_func.h"
#include "../common/zblock_io.h"
#include "../common/read_version.h"
#include "../common/std_param.h"
#include "osm_cut.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>


int main(int argc, char ** argv)
{
	StdParam std_param;
	z_block z_;
	World_t act_world_;
	time_t t1;

	time(&t1);
	if(argc < 5)
	{
		printf("usage: osm_cut -f <*.osminfo.gz> -r <w1,w2,h1,h2> [-o <output>] [-n <1>]\n");
		return(-1);
	}

	memset(&std_param, 0x00, sizeof(StdParam));
	std_param.flags = PARM_FILE | PARM_RECT | PARM_OUT | PARM_ACC | PARM_TR;
	read_param(&std_param, argc, argv);	

	Version_t source_version;
	memset(&source_version, 0x00, sizeof(Version_t));

	char * fname = get_fname(&std_param, DIR_IN, F_INFO);
	if(fname == NULL)
	{
		printf("parameter error: input file not defined\n");
			exit(-1);
	}
	if(std_param.out_fname == NULL)
	{
		set_output(&std_param, "undefined");
		printf("parameter error: output not defined, set default\n");
	}

	memset(&act_world_, 0x00, sizeof(World_t));
	//act_world_.nine_bit_mode = 1;

	printf("rect: %i %i %i %i\n", std_param.rect[0], std_param.rect[1], std_param.rect[2], std_param.rect[3]);
	addBox(&source_version.box, std_param.rect);
	initOsmInfo(&(act_world_.info));
	printf("read info file:  [%s]\n", fname);
	if(readOsmInfo(&(act_world_.info), fname, &source_version, 0))
	{
		printf("error '%s'\n", fname);
		return -1;
	}
	printf("IN  [%s]\n", fname);
	printf("OUT [%s]\n", std_param.out_fname);

	printf("nodes: %lld %lld %lld\n",(unsigned long long int)act_world_.info.node.count,
		(unsigned long long int)act_world_.info.node.min_id,
		(unsigned long long int)act_world_.info.node.max_id);
	printf("point: %lld %lld %lld\n",(unsigned long long int)act_world_.info.point.count,
		(unsigned long long int)act_world_.info.point.min_id,
		(unsigned long long int)act_world_.info.point.max_id);

	uint64_t count_points = act_world_.info.point.count;

	//addBox(&source_version.box, rect);	// TODO: addBox with uint32_t


	uint64_t node_mem_a = act_world_.info.node.count * 11; //16;
	uint64_t node_mem_b = act_world_.info.node.max_id * 7; //9;

	printf("%10lld %10llx\n",(long long unsigned int)node_mem_a,(long long int)node_mem_a);
	printf("%10lld %10llx\n",(long long unsigned int)node_mem_b,(long long int)node_mem_b);

	printf("using node buffer type id: %lld bytes needed\n",
		(long long unsigned int)node_mem_a);

	// TODO: change to 'select_12'
	int select_nine = 0;
	act_world_.nine_bit_mode = 0;

	if(select_nine)
	{
		act_world_.size_node = 8;
		// TODO
		act_world_.size_way = 8;
		act_world_.size_rel = 8;
	}
	else
	{
		act_world_.size_node = 11;

		// TODO: check 8 for size
		act_world_.size_way = 8;

		act_world_.size_rel = 8;
	}
	printf("select_nine %i, %i, node %i, way %i\n", select_nine, act_world_.nine_bit_mode, act_world_.size_node, act_world_.size_way);
	printf("%i %i %i %i\n", std_param.rect[0], std_param.rect[1], std_param.rect[2], std_param.rect[3]);

	act_world_.select_rect.id = 0;
	//act_world_.select_rect.lon1 = (uint32_t)((std_param.rect[0] + 180) * COOR_FACTOR);
	act_world_.select_rect.lon1 = (uint32_t)(std_param.rect[0] + (180 * COOR_FACTOR));
	act_world_.select_rect.lon2 = (uint32_t)(std_param.rect[1] + (180 * COOR_FACTOR));
	act_world_.select_rect.lat1 = (uint32_t)(std_param.rect[2] + (180 * COOR_FACTOR));
	act_world_.select_rect.lat2 = (uint32_t)(std_param.rect[3] + (180 * COOR_FACTOR));

	printf("%x %x %x %x | %i %i %i %i\n",
		act_world_.select_rect.lon1, act_world_.select_rect.lon2, act_world_.select_rect.lat1, act_world_.select_rect.lat2,
		act_world_.select_rect.lon1, act_world_.select_rect.lon2, act_world_.select_rect.lat1, act_world_.select_rect.lat2);

	fname = get_fname(&std_param, DIR_IN, F_NODE);
	printf("open #1: %s\n", fname);
	if(fname == NULL)
        {
		printf("error name == NULL\n");
		return -1;
	}

	zblock_new(&z_, ZB_READ);

	if(zblock_rd_open(&z_, fname))
	{
		printf("error opening gz-file\n");
		zblock_del(&z_);
		// TODO: cleanup
		return -1;
	}

	readNodes(&z_, &act_world_, 0);

	printf(" done\n");

	zblock_close(&z_);

	fname = get_fname(&std_param, DIR_IN, F_WAY);
	printf("open for reading #1: [%s]\n", fname);

	if(zblock_rd_open(&z_, fname))
	{
		printf("error opening gz-file\n");
		zblock_del(&z_);
		return -1;
	}

	if(readWays(&z_, &act_world_) != 0)
	{
		printf("main: mem fault\n");
		return -1;
	}

	printf(" done\n");

	zblock_close(&z_);

	fname = get_fname(&std_param, DIR_IN, F_REL);

	printf("open for reading #1: [%s]\n", fname);

        if(zblock_rd_open(&z_, fname))
        {
                printf("error opening gz-file\n");
                zblock_del(&z_);
                return -1;
        }

	if(readRelations(&z_, &act_world_) != 0)
        {
                printf("main: mem fault\n");
                return -1;
        }
	// TODO -> select rel
	printf(" done\n");
	zblock_close(&z_);

	fname = get_fname(&std_param, DIR_IN, F_NODE);
	printf("open for reading #2: [%s]\n", fname);
	if(zblock_rd_open(&z_, fname))
	{
		printf("error opening gz-file\n");
		zblock_del(&z_);
		//free(act_world_.in_path);
		return -1;
	}

	char buff[strlen(get_fname(&std_param, DIR_OUT, F_POINT))+1];
	strcpy(buff, get_fname(&std_param, DIR_OUT, F_POINT));
	act_world_.out_path = get_fname(&std_param, DIR_OUT, F_NODE);

	if(act_world_.out_path == NULL)
	{
		return -1;
	}

	printf("open #2 writeNodes N (%s) P (%s) ", act_world_.out_path, buff);
	writeNodes(&z_, &act_world_, buff);

	zblock_close(&z_);	

	printf(" done\n");

	fname = get_fname(&std_param, DIR_IN, F_WAY);
	if(fname == NULL)
	{
		printf("error 'cmd_get_fname'\n");
		return -1;
	}

	if(zblock_rd_open(&z_, fname))
	{
		printf("error opening gz-file\n");
		zblock_del(&z_);
		//free(xfname_);
		return -1;
	}

	printf("open for reading #2: %s\n", fname);


	//get_out_fname(&act_world_.out_path, select_out, out_file, "_way_20.osm.gz");
	char * fname_way = get_fname(&std_param, DIR_OUT, F_WAY);
	printf("open for writing: %s ", fname_way);
	writeWays(&z_, &act_world_, fname_way);

	zblock_close(&z_);

	printf(" done\n");

	fname = get_fname(&std_param, DIR_IN, F_REL);
	//if(cmd_get_fname(&act_world_.in_path, path, in_file, "_rel_20.osm.gz") != 0)
	if(fname == NULL)
	{
		printf("error 'cmd_get_fname'\n");
		return -1;
	}
	if(zblock_rd_open(&z_, fname))
	{
		printf("error opening gz-file\n");
		zblock_del(&z_);
		return -1;
	}

	printf("open for reading #2: %s\n", fname);

	
	//get_out_fname(&act_world_.out_path, select_out, out_file, "_rel_20.osm.gz");
	char * fname_rel = get_fname(&std_param, DIR_OUT, F_REL);
	printf("open for writing: %s ", fname_rel);
	writeRelations(&z_, &act_world_, fname_rel);

	zblock_close(&z_);

	printf(" done\n");

	// TODO: fix the coping the infos values
	printf("count_points: %li\n", count_points);
	if(count_points != 0)
	{
		act_world_.info.point.count = count_points;
		uint64_t node_count = act_world_.info.node.count;
		uint64_t node_min = act_world_.info.node.min_id;
		uint64_t node_max = act_world_.info.node.max_id;

		fname = get_fname(&std_param, DIR_IN, F_POINT);
		//if(cmd_get_fname(&act_world_.in_path, path, in_file, "_point_20.osm.gz") != 0)
		if(fname == NULL)
		{
			return -1;
		}

		printf("open for reading #1: [%s]", fname);

		zblock_new(&z_, ZB_READ);

		if(zblock_rd_open(&z_, fname))
		{
			printf("error opening gz-file\n");
			zblock_del(&z_);
			// TODO: cleanup
			return -1;
		}

		readNodes(&z_, &act_world_, 1);

		printf(" done\n");

		zblock_close(&z_);

		fname = get_fname(&std_param, DIR_IN, F_POINT);
		//if(cmd_get_fname(&act_world_.in_path, path, in_file, "_point_20.osm.gz") != 0)
		if(fname == NULL)
		{
			printf("error 'cmd_get_fname'\n");
			return -1;
		}

		if(zblock_rd_open(&z_, fname))
		{
			printf("error opening gz-file\n");
			zblock_del(&z_);
			//free(act_world_.in_path);
			return -1;
		}

		act_world_.out_path = get_fname(&std_param, DIR_OUT, F_NODE);
		//act_world_.out_point_path = get_fname(&std_param, DIR_OUT, F_POINT);
		if(act_world_.out_path == NULL)
		{
			return -1;
		}

		printf("open #2 writeNodes N (%s) P (%s) ", act_world_.out_path, get_fname(&std_param, DIR_OUT, F_POINT)); //act_world_.out_point_path);

		act_world_.out_path = get_fname(&std_param, DIR_OUT, F_NODE);

		writeNodes(&z_, &act_world_, buff);

		zblock_close(&z_);

		printf(" done\n");

		act_world_.info.node.count = node_count;
		act_world_.info.node.min_id = node_min;
		act_world_.info.node.max_id = node_max;
	}

        fname = get_fname(&std_param, DIR_OUT, F_INFO);
        if(fname != NULL)
        {
                printf("INFO OUT: %s\n", fname);
                writeOsmInfo(&(act_world_.info), fname, &source_version);
        }

	char t_buffer[100];
	cmd_time(t1, t_buffer);
	printf("%s\n", t_buffer);

	/*free(xfname_);
	if(act_world_.ways)
		free(act_world_.ways);
	if(act_world_.nodes)
		free(act_world_.nodes);
	if(act_world_.relations)
		free(act_world_.relations);*/

	cleanVersion(&source_version);

	return 0;
}

