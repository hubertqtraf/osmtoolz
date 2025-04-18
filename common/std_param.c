/******************************************************************
 *
 * @short	common cmd parameters
 *
 * project:	Trafalgar/OSM-Toolz
 *
 * modul:	std_param.c	header for parameters
 * @version	0.1
 *
 * system:	UNIX/LINUX
 * compiler:	gcc
 *
 * @author	Schmid Hubert (C)2025-2025
 *
 * beginning:	04.2025
 *
 * history:
 */
/******************************************************************/

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

#include "std_param.h"
#include "cmd_func.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int read_param(StdParam * param, int argc, char ** argv)
{
	int select_file   = cmd_args(argc, argv, "-f");
	int select_rect   = cmd_args(argc, argv, "-r");
	int select_out    = cmd_args(argc, argv, "-o");
	int select_accept = cmd_args(argc, argv, "-a");
	int select_thread = cmd_args(argc, argv, "-t");
	int select_mode   = cmd_args(argc, argv, "-m");
	int select_point  = cmd_args(argc, argv, "-p");
	int select_verbose= cmd_args(argc, argv, "-v");

	if(select_rect && (param->flags & PARM_RECT) && (argc > select_rect))
	{
		double rect[4];
		if(cmd_get_rect(argv[select_rect], rect) == 4)
		{
			param->rect[0] = (uint32_t)(rect[0] * COOR_FAC);
			param->rect[1] = (uint32_t)(rect[1] * COOR_FAC);
			param->rect[2] = (uint32_t)(rect[2] * COOR_FAC);
			param->rect[3] = (uint32_t)(rect[3] * COOR_FAC);
		}
	}

	if(select_file && (param->flags & PARM_FILE) && (argc > select_file))
	{
		param->in_fname = (char *)malloc(strlen(argv[select_file]) + 1);
		sprintf(param->in_fname, "%s", argv[select_file]);
	}

	if(select_out && (param->flags & PARM_OUT) && (argc > select_out))
	{
		set_output(param, argv[select_out]);
		//param->out_fname = (char *)malloc(strlen(argv[select_out]) + 100);
		//sprintf(param->out_fname, "%s.osminfo.gz", argv[select_out]);
	}

	if(select_accept && (param->flags & PARM_ACC) && (argc > select_accept))
	{
		param->val_accept = strtol(argv[select_accept], NULL, 10);
	}

	if(select_mode && (param->flags & PARM_MODE) && (argc > select_mode))
	{
		param->val_mode = strtol(argv[select_mode], NULL, 10);
	}

	if(select_point && (param->flags & PARM_PT) && (argc > select_point))
	{
		param->val_point = strtol(argv[select_point], NULL, 10);
	}

	if(select_thread && (param->flags & PARM_TR) && (argc > select_thread))
	{
		param->val_thread = strtol(argv[select_thread], NULL, 10);
	}

	if(select_verbose && (param->flags & PARM_VERB) && (argc > select_verbose))
	{
		param->val_verbose = strtol(argv[select_verbose], NULL, 10);
	}
	return 0;
}

void set_output(StdParam * param, char * name)
{
	if(name == NULL)
	{
		return;
	}
	param->out_fname = (char *)malloc(strlen(name) + 100);
	sprintf(param->out_fname, "%s", name);
}

char * get_fname(StdParam * param, int dir, int mode)
{
	char * base = NULL;

	if(param->buff != NULL)
	{
		free(param->buff);
		param->buff = NULL;
	}
	if(dir)
		base = param->out_fname;
	else
		base = param->in_fname;
	if(base == NULL)
		return NULL;

	param->buff = (char *)malloc(strlen(base) + 100);
	param->buff[0] = 0;
	switch(mode)
	{
	case F_INFO:
		sprintf(param->buff, "%s.osminfo.gz", base);
		break;
	case F_NODE:
		sprintf(param->buff, "%s_node_20.osm.gz", base);
		break;
	case F_WAY:
		sprintf(param->buff, "%s_way_20.osm.gz", base);
		break;
	case F_REL:
		sprintf(param->buff, "%s_rel_20.osm.gz", base);
		break;
	case F_POINT:
		sprintf(param->buff, "%s_point_20.osm.gz", base);
		break;

	default:
		return NULL;
	}
	return param->buff;
}

void free_param(StdParam * param)
{
	if(param->out_fname != NULL)
	{
		free(param->out_fname);
		param->out_fname = NULL;
	}
	if(param->in_fname != NULL)
	{
		free(param->in_fname);
		param->in_fname = NULL;
	}
	if(param->buff != NULL)
	{
		free(param->buff);
		param->buff = NULL;
	}
}

int openOsmInFile(StdParam * param, z_block * zref, uint8_t type)
{
	char * fname = get_fname(param, DIR_IN, type);
	//printf("open #1: %s\n", fname);
	if(fname == NULL)
	{
		printf("error name == NULL\n");
		return -1;
	}
	if(zblock_rd_open(zref, fname))
	{
		printf("error opening gz-file\n");
		zblock_del(zref);
		return -1;
	}
	return 0;
}

