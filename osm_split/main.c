/******************************************************************
 * project:     Trafalgar/OSM-Toolz
 *
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
#include "../common/std_param.h"
#include "osm_split.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char ** argv)
{
	StdParam std_param;

	if(argc < 3)
	{
		printf("usage: osm_split -f <xxx.osm.gz> [-o <output>] [-t 1] [-a 1]\n");
		exit(-1);
	}
	memset(&std_param, 0x00, sizeof(StdParam));
	std_param.flags = PARM_FILE | PARM_OUT | PARM_ACC | PARM_TR;
	read_param(&std_param, argc, argv);

	time_t t1;
	time(&t1);
	uint8_t p_n = 0;	// ?????

	Version_t source_version;
	if(std_param.in_fname == NULL)
	{
		printf("parameter error: input file not defined\n");
                	exit(-1);
	}
	if(std_param.out_fname == NULL)
	{
		setOutput(&std_param, "undefined");
		printf("parameter error: output not defined, set default\n");
	}

	printf("IN  [%s]\n", std_param.in_fname);
	printf("OUT [%s]\n", std_param.out_fname);

	if(getVersion(std_param.in_fname, &source_version, std_param.val_accept) == (-1))
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

	z_block z;
	World_t act_world;
	memset(&act_world, 0x00, sizeof(World_t));

	act_world.accept = std_param.val_accept;
	zblock_new(&z, ZB_READ); //| ZB_USE_R_THREAD);
	zblock_rd_open(&z, std_param.in_fname); //argv[select_file]);

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
		countNodesOut(&z, &act_world, std_param.out_fname, ZB_WRITE | ZB_USE_W_THREAD);
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

	writeOsmInfo(&(act_world.info), std_param.info_fname, &source_version);

	zblock_close(&z);
	zblock_del(&z);

	char t_buffer[100];
	cmd_time(t1, t_buffer);
	//printf("%s\n", t_buffer);

	cleanVersion(&source_version);
	free_param(&std_param);
	printf("\ndone\n");

	return 0;
}

