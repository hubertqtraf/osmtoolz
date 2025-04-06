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
/*#include "osm_huge.h"*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char ** argv)
{
	StdParam std_param;

	if(argc < 3)
	{
		printf("usage: osm_huge -f <xxx.osm.gz> [-o <output>] [-m 1] [-t 1] [-a 1]\n");
		exit(-1);
	}
	memset(&std_param, 0x00, sizeof(StdParam));
	std_param.flags = PARM_FILE | PARM_RECT | PARM_OUT | PARM_ACC | PARM_TR;
	read_param(&std_param, argc, argv);

	time_t t1;
	time(&t1);
	uint8_t p_n = 0;	// ?????

	Version_t source_version;
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

	printf("IN  [%s]\n", fname);
	printf("OUT [%s]\n", std_param.out_fname);

	if(getVersion(fname, &source_version, std_param.val_accept) == (-1))
	{
		printf("error opening gz-file [%s]\n",argv[1]);
		return -1;
	}
	printf("[%s]\n",source_version.source);

	source_version.n_64_flags = p_n;

	if((source_version.version != 6) && (source_version.version != 20))
	{
		printf("version 0.%i not supported\n", source_version.version);
		return -1;
	}

	printf("version: 0.%i; store coordinates in id: %i \n",
		source_version.version, p_n);

	//------------------------------------------------------------------

	fname = get_fname(&std_param, DIR_OUT, F_INFO);

	if(fname != NULL)
	{
		printf("OUT: %s\n", fname);
		//writeOsmInfo(&(act_world.info), fname, &source_version);
	}

	//zblock_close(&z);
	//zblock_del(&z);

	char t_buffer[100];
	cmd_time(t1, t_buffer);
	printf("%s\n", t_buffer);

	cleanVersion(&source_version);
	free_param(&std_param);
	printf("\ndone\n");

	return 0;
}

