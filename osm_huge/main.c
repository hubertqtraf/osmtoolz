/******************************************************************
 * project:     Trafalgar/OSM-Toolz
 *
 * modul:       main.c
 *
 * system:      UNIX/LINUX
 * compiler:    gcc
 *
 * beginning:   04.2025
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

#include "../common/cmd_func.h"
#include "../common/zblock_io.h"
#include "../common/read_version.h"
#include "../common/std_param.h"
#include "osm_huge.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char ** argv)
{
	StdParam std_param;
	World_t act_world_;

	if(argc < 3)
	{
		printf("usage: osm_huge -f <xxx.osm.gz> [-o <output>] [-m 1] [-t 1] [-a 1]\n");
		exit(-1);
	}
	memset(&act_world_, 0x00, sizeof(World_t));
	memset(&std_param, 0x00, sizeof(StdParam));
	std_param.flags = PARM_FILE | PARM_RECT | PARM_OUT | PARM_MODE | PARM_ACC | PARM_TR | PARM_PT | PARM_VERB;
	std_param.val_verbose = 1;
	read_param(&std_param, argc, argv);

	std_param.bar_width = 60;

	time_t t1;
	time(&t1);
	uint8_t p_n = 0;	// ?????

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
	if(std_param.val_verbose)
	{
		printf("rect: %i %i %i %i\n", std_param.rect[0],
			std_param.rect[1], std_param.rect[2], std_param.rect[3]);
	}
	addBox(&source_version.box, std_param.rect);
	initOsmInfo(&(act_world_.info));
	if(readOsmInfo(&(act_world_.info), fname, &source_version, 0))
	{
		printf("error '%s'\n", fname);
		return -1;
	}

	if(getVersion(fname, &source_version, std_param.val_accept) == (-1))
	{
		printf("error opening gz-file [%s]\n",argv[1]);
		return -1;
	}
	if(std_param.val_verbose)
	{
		printf("\nIN  [%s]\n", get_fname(&std_param, DIR_IN, F_NODE));
		printf("OUT [%s]\n", get_fname(&std_param, DIR_OUT, F_NODE));
		printf("[%s]\n",source_version.source);
	}

	source_version.n_64_flags = p_n;

	if((source_version.version != 6) && (source_version.version != 20))
	{
		printf("version 0.%i not supported\n", source_version.version);
		return -1;
	}

	//------------------------------------------------------------------

	z_block z_;
	zblock_new(&z_, ZB_READ);

	if(!std_param.val_mode)
		std_param.val_mode = 1;
	setMode(&act_world_, std_param.val_mode);

	if(std_param.val_verbose)
	{
		printf("1: %li \n", act_world_.info.node.count);
		printf("5: %li (%li)\n", act_world_.info.node.count * 5, act_world_.info.node.count);
		printf("X: %li \n", act_world_.info.node.max_id);
	}
	if((act_world_.info.node.count * 6) > act_world_.info.node.max_id)
	{
		printf("A: %ld\n", act_world_.info.node.max_id);
		act_world_.mem_mode = MODE_MAX_ID;	// max id mode
	}
	else
	{
		printf("B: %ld\n", act_world_.info.node.count * 5);
		act_world_.mem_mode = MODE_BLOCK;	// count mode, without gaps
	}
	act_world_.mem_mode = MODE_MAX_ID;
	if(openOsmInFile(&std_param, &z_, F_NODE))
	{
		return -1;
	}
	int ret = readNodes(&z_, &act_world_, &std_param); //std_param.rect);

	if(std_param.val_verbose)
		printf("... done (ret = %i)\n", ret);

	zblock_close(&z_);

	if(std_param.val_mode == 5)
	{
		if(openOsmInFile(&std_param, &z_, F_WAY))
			return -1;
		if(cutWays(&z_, &act_world_, &std_param) != 0)
		{
			printf("main: mem fault\n");
			return -1;
		}
		zblock_close(&z_);
		if(std_param.val_verbose)
			printf("... done\n");

		if(openOsmInFile(&std_param, &z_, F_WAY))
			return -1;
		fname = get_fname(&std_param, DIR_OUT, F_WAY);
		if(fname == NULL)
			return -1;
		writeWays(&z_, &act_world_, fname, &std_param);
		zblock_close(&z_);
		if(std_param.val_verbose)
			printf("... done, created file: %s\n", fname);
	}
	if(openOsmInFile(&std_param, &z_, F_NODE))
		return -1;

	char * ofname = get_fname(&std_param, DIR_OUT, F_NODE);

	if(std_param.val_verbose)
		printf("open write nodes %s ", ofname);
	ret = writeNodes(&z_, &act_world_, ZB_WRITE | ZB_USE_W_THREAD, ofname, &std_param);
	zblock_close(&z_);

	if(std_param.val_verbose)
		printf("... done, created file: %s\n", ofname);

	fname = get_fname(&std_param, DIR_OUT, F_INFO);
	if(fname != NULL)
	{
		if(std_param.val_verbose)
			printf("INFO OUT: %s\n", fname);
		writeOsmInfo(&(act_world_.info), fname, &source_version);
	}

	zblock_close(&z_);
	zblock_del(&z_);

	if(std_param.val_verbose)
	{
		char t_buffer[100];
		cmd_time(t1, t_buffer);
		printf("\n%s", t_buffer);
	}

	cleanVersion(&source_version);
	free_param(&std_param);
	if(std_param.val_verbose)
		printf("... done\n");

	return 0;
}

