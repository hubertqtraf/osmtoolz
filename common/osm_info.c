/******************************************************************
 *
 * @short	reading/writing the info file
 *
 * project:	Trafalgar/OSM-Toolz
 *
 * modul:	osm_info.c	source for XML read/write
 * @version	0.1
 *
 * system:	UNIX/LINUX
 * compiler:	gcc
 *
 * @author	Schmid Hubert (C)2016-2025
 *
 * beginning:	02.2016
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

#include "osm_info.h"
#include "zblock_io.h"
#include "simple_sax.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <stdlib.h>

typedef struct
{
	int section_type;
	int arg_type;
	OsmInfo_t * info;
	Version_t * version;
}OsmXmlInfo;

void initOsmInfo(OsmInfo_t * info)
{
	memset(info, 0x00, sizeof(OsmInfo_t));
}


int info_tag_close(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		OsmXmlInfo * info_ = (OsmXmlInfo *)sax->d_ref;

		if(sax->last_byte == '/')
		{
			if(info_->section_type & 0x200)
				info_->section_type &= 0x01ff;
			info_->section_type &= 0x0f0f;
		}
		else
			;//printf("closing %s\n",sax->tag_start);
	}
	return 0;
}


int info_arg_value(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		OsmXmlInfo * info_ = (OsmXmlInfo *)sax->d_ref;

		uint64_t value_ = 0;

		if(info_->section_type < 0x200)
			value_ = atol((const char*)sax->tag_start);

		//printf("arg value [%s] %i %d %x\n", sax->tag_start, sax->tag_act_size, value_,
		//	info_->section_type + info_->arg_type);

		switch(info_->section_type + info_->arg_type)
		{
		case 0x114:
			info_->info->node.count = value_;
			break;

		case 0x115:
			info_->info->node.min_id = value_;
			break;

		case 0x116:
			info_->info->node.max_id = value_;
			break;

		case 0x117:
			info_->info->node.member = value_;
			break;

		case 0x124:
			info_->info->way.count = value_;
			break;

		case 0x125:
			info_->info->way.min_id = value_;
			break;

		case 0x126:
			info_->info->way.max_id = value_;
			break;

		case 0x127:
			info_->info->way.member = value_;
			break;

		case 0x134:
			info_->info->rel.count = value_;
			break;

		case 0x135:
			info_->info->rel.min_id = value_;
			break;

		case 0x136:
			info_->info->rel.max_id = value_;
			break;

		case 0x137:
			info_->info->rel.member = value_;
			break;

		case 0x144:
			info_->info->seg.n_id = value_;
			break;

		case 0x145:
			info_->info->seg.min_id = value_;
			break;

		case 0x146:
			info_->info->seg.max_id = value_;
			break;

		case 0x147:
			info_->info->seg.member = value_;
			break;

		case 0x184:
			info_->info->point.count = value_;
			break;

		case 0x185:
			info_->info->point.min_id = value_;
			break;

		case 0x186:
			info_->info->point.max_id = value_;
			break;

		case 0x187:
			info_->info->point.member = value_;
			break;

		case 0x201:
			addString(&(info_->version->source), (char*)sax->tag_start,
				sax->tag_act_size-1);
			break;

		case 0x202:
			info_->version->version = (int)(atof((const char*)sax->tag_start) * 10.0);
			break;

		case 0x203:
			addString(&(info_->version->generator), (char*)sax->tag_start,
				sax->tag_act_size-1);
			break;

		default:
			break;
		}
	}
	return 0;
}

int info_tag_name(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		OsmXmlInfo * info_ = (OsmXmlInfo *)sax->d_ref;

		switch(sax->tag_act_size)
		{
		case 6:			// nodes
			info_->section_type += 0x10;
			break;

		case 5:			// ways
			info_->section_type += 0x20;
			break;

		case 10:		// relations
			info_->section_type += 0x30;
			break;

		case 9:			// segments, sections
			if(sax->tag_start[2] == 'g')
				info_->section_type += 0x40;
			if(sax->tag_start[2] == 'c')
				info_->section_type = 0x100;
			//printf("%s %x\n",sax->tag_start,info_->section_type);
			break;

		case 7:			// points
			if(sax->tag_start[1] == 'o')
				info_->section_type += 0x80;
			else		// origin
				info_->section_type = 0x200;
			break;

		default:
			break;
		}
	}

	return 0;
}

int info_tag_arg_name(struct _simple_sax * sax)
{
	if(sax->tag_start)
	{
		//printf("arg_name: %s %i\n",sax->tag_start,sax->tag_act_size);

		OsmXmlInfo * info_ = (OsmXmlInfo *)sax->d_ref;

		if(info_->section_type & 0x100)
		{
			if(sax->tag_act_size == 2)
				info_->arg_type = 4;

			if(sax->tag_act_size == 4)
			{
				if(sax->tag_start[1] == 'i')
					info_->arg_type = 5;
				if(sax->tag_start[1] == 'a')
					info_->arg_type = 6;
			}

			if(sax->tag_act_size == 7)	//member
			{
				if(sax->tag_start[0] == 'm')
					info_->arg_type = 7;
			}
		}

		if(info_->section_type & 0x200)
		{
			if(sax->tag_act_size == 5)
				info_->arg_type = 1;

			if(sax->tag_act_size == 8)
				info_->arg_type = 2;

			if(sax->tag_act_size == 10)
				info_->arg_type = 3;
		}
	}
	return 0;
}

void info_init(struct _simple_sax * xml_ref)
{
	sax_add_cb(xml_ref, info_tag_close, SAX_CB_TAG_CLOSE);
	sax_add_cb(xml_ref, info_arg_value, SAX_CB_ARG_VALUE);
	sax_add_cb(xml_ref, info_tag_name, SAX_CB_TAG_NAME);
	sax_add_cb(xml_ref, info_tag_arg_name, SAX_CB_ARG_NAME);
}

void resetInfoPart(OsmIdInfo_t * info)
{
	info->min_id = 0;
	info->max_id = 0;
	info->n_id = 0;			// number of members
	info->i_count = 0;		// internal counter
	info->member = 0;		// counter for max. number of members
}

void updateInfoPart(OsmIdInfo_t * info, uint64_t act_id)
{
	if(info->min_id == 0)
	{
		info->min_id = act_id;
		info->max_id = act_id;
	}
	else
	{
		if(act_id < info->min_id)
			info->min_id = act_id;
		if(act_id > info->max_id)
			info->max_id = act_id;
	}
	info->i_count++;
}

int readOsmInfo(OsmInfo_t * info, const char * fname, Version_t * version, int accept)
{
	int n_read;
	int tag_len=0;
	simple_sax sax;
	unsigned char * z_buf;
	int z_size;
	//uint32_t version = 0;
	OsmXmlInfo ox_info;
	z_block z_read;

	zblock_new(&z_read, ZB_READ);

	if(zblock_rd_open(&z_read, fname))
	{
		zblock_del(&z_read);
		return -1;
	}

	sax.tag_start=NULL;
	sax_init(&sax, accept);

	info_init(&sax);

	ox_info.section_type = 0;
	ox_info.arg_type = 0;
	initOsmInfo(info);
	ox_info.info = info;
	ox_info.version = version;
	//memset(&ox_info.version, 0x00, sizeof(Version_t));

	sax_set_data_ref(&sax, &ox_info);

	zblock_set_start(&z_read, (uint8_t*)"",0);

	while((n_read = zblock_read(&z_read)) > 0)
	{
		sax.tag_start = zblock_first(&z_read);

		z_buf = zblock_buff(&z_read, &z_size);

		sax_read_block(&sax, z_buf, z_size);

		sax_save_tag_name(&sax);
		if(sax.tag_start != NULL)
			tag_len = strlen((const char *)sax.tag_start);
		else
			tag_len = 0;

		//printf("--> %i %i \n", tag_len, sax.tag_act_size);
		// TODO: crosscheck, final solution?
		sax.tag_act_size = tag_len;

		zblock_set_start(&z_read, sax.tag_start, tag_len);
	}

	sax_cleanup(&sax);

	zblock_close(&z_read);
	zblock_del(&z_read);

	return 0;
}

int writeOsmInfoBlock(OsmIdInfo_t * id_info, const char * name, z_block * zw)
{
	char buffer[256];

	snprintf(buffer,255,"\t\t<%s n=\"%lld\" min=\"%lld\" max=\"%lld\" member=\"%lld\"/>\n",
		name, (unsigned long long int)id_info->count,
		(unsigned long long int)id_info->min_id,
		(unsigned long long int)id_info->max_id,
		(unsigned long long int)id_info->member);
	buffer[255] = 0x00;
	zblock_wr_append(zw, buffer, strlen(buffer));

	return 0;
}

int writeOsmInfo(OsmInfo_t * info, const char * fname, Version_t * version)
{
	z_block zw;
	char version_no[100];

	if(createOsmHead(&zw, fname, ZB_WRITE))
		return (-1);

	zblock_wr_append(&zw,"\t<origin file=\"",15);
	if(version->source)
		zblock_wr_append(&zw, version->source,strlen(version->source));
	sprintf(version_no, "%i.%i",(int)version->version/10, (int)version->version % 10);
	zblock_wr_append(&zw, "\" version=\"",11);
	zblock_wr_append(&zw, version_no, strlen(version_no));
	zblock_wr_append(&zw, "\" ",2);
	if(version->generator)
	{
		zblock_wr_append(&zw,"generator=\"",11);
		zblock_wr_append(&zw, version->generator,strlen(version->generator));
		zblock_wr_append(&zw, "\"",1);
	}
	zblock_wr_append(&zw, " />\n",4);

	if(version->box != NULL)
	{
		char buffer[256];

		zblock_wr_append(&zw,"\t<box ", 6);
		snprintf(buffer,255," x1=\"%ld\" x2=\"%ld\" y1=\"%ld\" y2=\"%ld\"",
			(long)version->box[0],(long)version->box[1],
			(long)version->box[2],(long)version->box[3]);
		buffer[255] = 0x00;
		zblock_wr_append(&zw, buffer, strlen(buffer));
		zblock_wr_append(&zw,"/>\n", 3);
	}

	zblock_wr_append(&zw, "\t<nd_id_64 ", 11);
	zblock_wr_append(&zw, "flag=", 5);
	if(version->n_64_flags == 0x00)
		zblock_wr_append(&zw, "\"no\"/>\n", 7);
	else
		zblock_wr_append(&zw, "\"yes\"/>\n", 8);

	// TODO: flag for relations -> ways/nodes out of border

	zblock_wr_append(&zw, "\t<sections n=\"4\" >\n", 19);
	writeOsmInfoBlock(&info->node, "nodes",    &zw);
	writeOsmInfoBlock(&info->point,"points",   &zw);
	writeOsmInfoBlock(&info->way,  "ways",     &zw);
	writeOsmInfoBlock(&info->rel,  "relations",&zw);
	writeOsmInfoBlock(&info->seg,  "segments", &zw);
	zblock_wr_append(&zw, "\t</sections> \n", 14);

	if(closeOsm(&zw))
		return -1;

	return 0;
}

int createOsmHead(z_block * out, const char * fname, uint8_t flags)
{
	if(zblock_new(out, flags)) //ZB_WRITE))
		return (-1);
	if(zblock_wr_open(out, fname))
		return (-1);
	zblock_wr_append(out, (char*)"<?xml version='1.0' encoding='UTF-8'?>\n",39);
	zblock_wr_append(out, (char*)"<osm version=\"2.0\" generator=\"osmtoolz\">\n",41);

	return 0;
}

int closeOsm(z_block * out)
{
	// TODO: error check?
	zblock_wr_append(out, (char*)"</osm>\n",7);
	zblock_wr_flush(out);
	zblock_close(out);
	zblock_del(out);

	return 0;
}

