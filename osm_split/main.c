/******************************************************************
 * project:     Trafalgar/OSM-Toolz
 *
 * class:       ---
 * superclass:  ---
 * modul:       main.c
 *
 * system:      UNIX/LINUX
 * compiler:    gcc
 *
 * beginning:   11.2012
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

#include "../common/cmd_func.h"
#include "../common/zblock_io.h"
#include "../common/read_version.h"
#include "osm_split.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char ** argv)
{
	z_block z;
	time_t t1;

	if(argc < 3)
	{
		printf("usage: osm_split -f <xxx.osm.gz> [-o <output>] [-t 1] [-a 1]\n");
		exit(-1);
	}

	int select_file   = cmd_args(argc, argv, "-f");
	int select_thread = cmd_args(argc, argv, "-t");
	int select_out    = cmd_args(argc, argv, "-o");
	int select_accept = cmd_args(argc, argv, "-a");

	if(!select_file)
	{
		printf("parameter error: input file not defined\n");
		exit(-1);
	}

	if(select_out)
	{
		printf("using input [%s] and output [%s]\n",
			argv[select_file], argv[select_out]);
	}
	else
	{
		printf("using input [%s] \n", argv[select_file]);
	}

	// TODO
	printf("use thread option: %i\n", select_thread);
	printf("use accept option: %i\n", select_accept);

	uint8_t p_n = 0;

	/*if(argc == 3)
	{
		if(strlen(argv[2]) == 2)
			if((argv[2][0] == '-') && (argv[2][1] == 'n'))
				p_n = 0x01;
	}*/

	time(&t1);

	Version_t source_version;

	if(getVersion(argv[select_file], &source_version, select_accept) == (-1))
	{
		printf("error opening gz-file [%s]\n",argv[1]);
		return -1;
	}

	printf("[%s]\n",source_version.source);

	source_version.n_64_flags = p_n;

	if(source_version.version > 6)
	{
		printf("version 0.%i not supported\n", source_version.version);
		return -1;
	}

	printf("version: 0.%i; store coordinates in id: %i \n",
		source_version.version, p_n);

	//------------------------------------------------------------------

	World_t act_world;
	memset(&act_world, 0x00, sizeof(World_t));

	if(select_accept)
		act_world.accept = 1;

	//zblock_new(&z,4,0);
	zblock_new(&z, ZB_READ); //| ZB_USE_R_THREAD);
	zblock_rd_open(&z,argv[select_file]);

	//exit(0);

	switch(source_version.version)
	{
	case 3:
	case 4:
		zblock_close(&z);
		zblock_del(&z);
		printf("old API\n");
		return -1;

	case 5:
	case 6:
		if(select_out)
		{
			printf("using '-o' option\n");
			countNodesOut(&z, &act_world, argv[select_out], ZB_WRITE | ZB_USE_W_THREAD);
		}
		else
		{
			system("rm ./result/*.gz");
			countNodes(&z, &act_world, p_n, ZB_WRITE | ZB_USE_W_THREAD);
		}
		break;

	default:
		zblock_close(&z);
		zblock_del(&z);
		// unknown
		printf("unknown format\n");
		return -1;
	}
	zblock_close(&z);

	printf("split done, nodes: min %ld, max %ld\n",
		act_world.info.node.min_id, act_world.info.node.max_id);

	if(select_out)
	{
		char * fname = (char *)malloc(strlen(argv[select_out]) + 100);
		sprintf(fname, "%s.osminfo.gz", argv[select_out]);
		writeOsmInfo(&(act_world.info), fname, &source_version);
		free(fname);
	}
	else
		writeOsmInfo(&(act_world.info), "./result/info_20.osm.gz", &source_version);

	zblock_close(&z);
	zblock_del(&z);

	// TODO: remove? -> readNodes -> osm_store_node_coor.c
	if((source_version.version == 6) && (source_version.n_64_flags == 1))
	{
		printf("additional step: add coordinates to node ids\n");

		zblock_new(&z, ZB_READ); //| ZB_USE_R_THREAD);

		if(zblock_rd_open(&z, "./result/node_20.osm.gz"))
		{
			printf("error opening gz-file\n");
			zblock_del(&z);
			return -1;
		}

		readNodes(&z, &act_world);

		zblock_close(&z);
		zblock_del(&z);
	}

	char t_buffer[100];
	cmd_time(t1, t_buffer);
	//printf("%s\n", t_buffer);

	cleanVersion(&source_version);

	return 0;
}

