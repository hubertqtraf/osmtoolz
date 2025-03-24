/******************************************************************
 *
 * @short	helper functions
 *
 * project:	Trafalgar/OSM-Toolz
 *
 * modul:	cmd_func.c	source of helper functions
 * @version	0.1
 *
 * system:	UNIX/LINUX
 * compiler:	gcc
 *
 * @author	Schmid Hubert (C)2020-2025
 *
 * beginning:	02.2020
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "cmd_func.h"

int cmd_args(int argc, char ** argv, char * opt)
{
	bool select = false;

	for(int i=1; i<argc; i++)
	{
		if(!select)
		{
			if(strlen(argv[i]) == 2)
			{
				if(strncmp(argv[i], opt, 2) == 0)
					select = true;
			}
		}
		else
		{
			if(select)
				return i;
			select = false;
		}
	}
	return 0;
}

char * cmd_get_path(char * where)
{
	char * f_pos = where;
	char * last = where;

	while(*f_pos)
	{
		if(*f_pos == '/')
			last = f_pos;
		f_pos++;
	}
	if(last)
	{
		if(*last == '/')
		{
			*last = 0x00;
			return(++last);
		}
	}

	return(NULL);
}


int cmd_get_fname(char ** result, char * path, char * name, char * type)
{
	int name_len = strlen(name);
	char * pos = *result;

	// printf("==> [%s][%s][%s]\n", path, name, type);

	if(name_len <= 11)
		return -1;

	if(strncmp(name + (name_len-10), "osminfo.gz", 11) != 0)
		return -2;

	*pos = 0x00;

	if(path != NULL)
	{
                strcat(pos, path);
		strcat(pos, "/");
	}
	strcat(pos, name);
	pos[strlen(pos) - 11] = 0;
	strcat(pos, type);

	printf("result: [%s]\n", *result);

	return 0;
}

int cmd_get_rect(char * param, double rect[4])
{
	char * pch;
	int count = 0;
	char* pEnd;

	pch = strtok(param, ",");

	while(pch != NULL)
	{
		if(count >= 4)
			return -1;
		rect[count] = strtod(pch, &pEnd);
		pch = strtok(NULL, ",");
		// printf("%f\n", rect[count]);
		count++;
	}

	return count;
}

int cmd_time(time_t t1, char * buffer)
{
	time_t t2;

	time(&t2);

	int dt = t2-t1;
	int nsec_abs = dt;

	int hours = dt / 3600;
	dt = dt % 3600;
	int minutes = dt / 60;
	int secs = dt % 60;

	sprintf(buffer, "duration: %02i:%02i:%02i (%i seconds)\n",
			hours, minutes, secs, nsec_abs);

	return nsec_abs;
}
