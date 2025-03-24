/******************************************************************
 *
 * @short	reading the version information
 *
 * project:	Trafalgar/OSM-Toolz
 *
 * modul:	read_version.c	source of osm version
 * @version	0.1
 *
 * system:	UNIX/LINUX
 * compiler:	gcc
 *
 * @author	Schmid Hubert (C)2012-2025
 *
 * beginning:	11.2012
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

#include "read_version.h"

#include "simple_sax.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void resetVersion(Version_t * version)
{
	memset(version, 0x00, sizeof(Version_t));
}

void cleanVersion(Version_t * version)
{
	if(version->source)
		free(version->source);
	if(version->generator)
		free(version->generator);
	if(version->timestamp)
		free(version->timestamp);
	if(version->box)
		free(version->box);
}

void addString(char ** dest, char * source, size_t len)
{
	if(*dest != 0)
		return;

	if((*dest = (char *)malloc(len+1)) == NULL)
		return;
	memcpy(*dest, source, len);
	(*dest)[len] = 0x00;
}

void addBox(int32_t ** dest, int32_t *source)
{
	if(*dest != 0)
		return;
	if((*dest = (int32_t *)malloc(sizeof(int32_t)*4)) == NULL)
		return;
	memcpy(*dest, source, sizeof(uint32_t)*4);
}

int version_tag_close(struct _simple_sax * sax)
{
	return 0;
}

int version_arg_value(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		Version_t * version_ = (Version_t *)sax->d_ref;

		if((version_->version & 0xF0000000) == 0xC0000000)
		{
			int result = 0xffff;

			if(strlen((const char *)sax->tag_start) == 3)
			{
				if(sax->tag_start[1] == '.')
				{
					result = (((sax->tag_start[0])-'0') * 10) +
						(sax->tag_start[2] - '0');
				}
			}
			version_->version += result;
		}
		if((version_->version & 0xF0000000) == 0xA0000000)
		{
			// creator
			addString(&version_->generator, (char*)sax->tag_start,
				strlen((const char *)sax->tag_start));
		}
	}
	return 0;
}

int version_tag_name(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		//printf("[%s]", sax->tag_start);

		Version_t * version_ = (Version_t *)sax->d_ref;

		if(!strncmp((const char *)sax->tag_start,"osm",3))
		{
			version_->version |= 0x80000000;
		}
		else
			version_->version &= 0x7fffffff;
	}

	return 0;
}

int version_tag_arg_name(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		Version_t * version_ = (Version_t *)sax->d_ref;

		if(version_->version & 0x80000000)
		{
			if(!strncmp((const char *)sax->tag_start,"version",7))
			{
				version_->version |= 0x40000000;
			}
			else
				version_->version &= 0xbfffffff;
			if(!strncmp((const char *)sax->tag_start,"generator",9))
			{
				version_->version |= 0x20000000;
			}
			else
				version_->version &= 0xdfffffff;
		}
	}
	return 0;
}

void version_init(struct _simple_sax * xml_ref)
{
	sax_add_cb(xml_ref, version_tag_close, SAX_CB_TAG_CLOSE);
	sax_add_cb(xml_ref, version_arg_value, SAX_CB_ARG_VALUE);
	sax_add_cb(xml_ref, version_tag_name, SAX_CB_TAG_NAME);
	sax_add_cb(xml_ref, version_tag_arg_name, SAX_CB_ARG_NAME);
}

int getVersion(char * osm_file, Version_t * version, int accept)
{
	z_block z;
	simple_sax sax;
	unsigned char * z_buf;
	int z_size;

	zblock_new(&z, ZB_READ);

	if(zblock_rd_open(&z, osm_file))
	{
		zblock_del(&z);
		return(-1);
        }

	char * pos = osm_file;
	while(*pos++)
		if(*pos == '/')
			osm_file = pos+1;

	memset(version, 0x00, sizeof(Version_t));
	//printf("[%s] %i\n",osm_file,(int)strlen(osm_file));
	addString(&(version->source), osm_file, strlen(osm_file));

	sax_init(&sax, accept);
	version_init(&sax);

	sax_set_data_ref(&sax, version);

	zblock_set_start(&z, (uint8_t*)"",0);

	if(zblock_read(&z) > 0)
	{
		sax.tag_start = zblock_first(&z);

		z_buf = zblock_buff(&z, &z_size);

		sax_read_block(&sax, z_buf, z_size);

		if(version->version & 0x0fffffff)
		{
			version->version &= 0x0fffffff;
			zblock_close(&z);
			zblock_del(&z);
			return (version->version & 0x0fffffff);
		}
	}

	sax_cleanup(&sax);
	zblock_close(&z);
	zblock_del(&z);

	return 0;
}

