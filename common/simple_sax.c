/******************************************************************
 *
 * @short	simple SAX XML interpreter
 *
 * project:	Trafalgar/OSM-Toolz
 *
 * modul:	simple_sax.c	source of SAX XML interpreter
 * @version	0.1
 *
 * system:	UNIX/LINUX
 * compiler:	gcc
 *
 * @author	Schmid Hubert (C)2012-2025
 *
 * beginning:	10.2012
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
   Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simple_sax.h"

int tag_open(struct _simple_sax * xml_ref);
int tag_close(struct _simple_sax * ref);
int arg_start_end(struct _simple_sax * ref);
int tag_start_end(struct _simple_sax * ref);
int newline(struct _simple_sax * ref);
int blank(struct _simple_sax * ref);
int equal(struct _simple_sax * ref);

void sax_init(simple_sax * ref, uint8_t accept)
{
	int i;

	ref->inside_mask    = 0x00;

	ref->tag_start      = NULL;
	ref->tag_name_start = 0x00;
	ref->act_byte       = NULL;
	ref->last_byte      = 0x00;

	ref->tag_act_size   = 0;
	ref->tag_name_len   = 0;

	ref->char_callback=(int (**)(struct _simple_sax*))
		malloc(sizeof(void*)*256);

	for(i=0; i<256; i++)
		ref->char_callback[i]=NULL;
	ref->char_callback['<']  = tag_open;
	ref->char_callback['>']  = tag_close;
	ref->char_callback['"']  = arg_start_end;
	// TODO:  option '-a 1' will fail on:
	// <tag k="from" v="Assisi, Piazza Unità d'Italia"/>
	// Fix?
	if(accept)
		ref->char_callback['\'']  = arg_start_end;	// activate by flag
	ref->char_callback['/']  = tag_start_end;
	ref->char_callback['\n'] = newline;
	ref->char_callback['\t'] = blank;
	ref->char_callback[' ']  = blank;
	ref->char_callback['=']  = equal;

	ref->tag_open_cb     = NULL;
	ref->tag_close_cb    = NULL;
	ref->tag_arg_end_cb  = NULL;
	ref->tag_name_cb     = NULL;
	ref->tag_arg_name_cb = NULL;

	ref->d_ref = NULL;
}

void sax_cleanup(simple_sax * ref)
{
	if(ref->char_callback)
		free(ref->char_callback);
}

void sax_set_data_ref(simple_sax * ref, void * dref)
{
	ref->d_ref = dref;
}

void sax_add_cb(struct _simple_sax * ref, int (* cb)(struct _simple_sax *), int type)
{
	switch (type)
	{
		case SAX_CB_TAG_OPEN:
			ref->tag_open_cb = cb;
			break;

		case SAX_CB_TAG_CLOSE:
			ref->tag_close_cb = cb;
			break;

		case SAX_CB_ARG_VALUE:
			ref->tag_arg_end_cb = cb;
			break;

		case SAX_CB_TAG_NAME:
			ref->tag_name_cb = cb;
			break;

		case SAX_CB_ARG_NAME:
			ref->tag_arg_name_cb = cb;
			break;

		default:
			break;
	}
}

int tag_open(struct _simple_sax * xml_ref)
{
	if(xml_ref->inside_mask & SAX_INSIDE_ATT)
		return(0);

	if(xml_ref->inside_mask & SAX_INSIDE_TAG)
	{
		if(xml_ref->inside_mask & SAX_INSIDE_COMMENT)
		{
			printf("'<' inside comment\n");
			return(0);
		}
		xml_ref->tag_start[100]=0x00;
		printf("'<' inside tag [%s]\n", xml_ref->tag_start);
		//return(-1);
	}
	xml_ref->inside_mask |= SAX_INSIDE_TAG;
	xml_ref->inside_mask |= SAX_INSIDE_TAG_NAME;

	xml_ref->inside_mask &= (0xff^SAX_INSIDE_ATT);

	xml_ref->inside_mask &= (0xff^SAX_TAG_END_FLAG);

	if(xml_ref->tag_open_cb)
		xml_ref->tag_open_cb(xml_ref);

	xml_ref->tag_start = NULL;

	return(0);
}

int tag_close(struct _simple_sax * xml_ref)
{
	if(xml_ref->inside_mask & SAX_INSIDE_ATT)
	{
		return(0);
	}

	if(!(xml_ref->inside_mask & SAX_INSIDE_TAG))
	{
		if(xml_ref->inside_mask & SAX_INSIDE_COMMENT)
		{
			printf("'>' inside comment\n");
			return(0);
		}
		printf("'>' outside tag\n");
			return(-1);
	}

	xml_ref->inside_mask &= (0xff^SAX_INSIDE_TAG);
	xml_ref->inside_mask &= (0xff^SAX_INSIDE_ATT);
	xml_ref->inside_mask &= (0xff^SAX_INSIDE_ATT_NAME);
	*(xml_ref->act_byte)=0x00;

	if(xml_ref->tag_close_cb)
		xml_ref->tag_close_cb(xml_ref);

	xml_ref->tag_start = NULL;

	xml_ref->inside_mask &= (0xff^SAX_TAG_END_FLAG);

	if(xml_ref->inside_mask & SAX_INSIDE_TAG_NAME)
	{
		xml_ref->inside_mask &= (0xff^SAX_INSIDE_TAG_NAME);
			return(SAX_RET_TAG_NAME);
	}

	return(0);
}

int arg_start_end(struct _simple_sax * xml_ref)
{
	if(xml_ref->inside_mask & SAX_INSIDE_COMMENT)
	{
		printf("SAX_INSIDE_COMMENT\n");
		return(0);
	}
	if(xml_ref->inside_mask & SAX_INSIDE_ATT)
	{
		xml_ref->inside_mask &= (0xff^SAX_INSIDE_ATT);
		xml_ref->inside_mask |= SAX_INSIDE_ATT_NAME;
		*(xml_ref->act_byte)=0x00;

		if(xml_ref->tag_arg_end_cb)
			xml_ref->tag_arg_end_cb(xml_ref);
		xml_ref->tag_start = NULL;
		return(SAX_RET_ARG);
	}

	xml_ref->inside_mask &= (0xff^SAX_INSIDE_TAG_NAME);
	xml_ref->inside_mask &= (0xff^SAX_INSIDE_ATT_NAME);
	xml_ref->inside_mask |= SAX_INSIDE_ATT;
	xml_ref->tag_start = NULL;

	return(0);
}

int tag_start_end(struct _simple_sax * xml_ref)
{
	if(xml_ref->inside_mask&SAX_INSIDE_COMMENT)
	{
		return(0);
	}
	if(xml_ref->inside_mask&SAX_INSIDE_ATT)
	{
		return(0);
	}
	if(xml_ref->inside_mask&SAX_INSIDE_TAG)
	{
		if(xml_ref->last_byte == '<')
		{
			xml_ref->inside_mask |= SAX_TAG_END_FLAG;
			xml_ref->tag_start = NULL;
		}
	}
	return(0);
}

int newline(struct _simple_sax * ref)
{
	*(ref->act_byte)=' ';
	return(0);
}

int blank(struct _simple_sax * xml_ref)
{
	if(xml_ref->inside_mask & SAX_INSIDE_ATT)
		return 0;

	if(xml_ref->inside_mask & SAX_INSIDE_TAG_NAME)
	{
		*(xml_ref->act_byte) = 0x00;
		if(xml_ref->tag_name_cb)
		{
			if(xml_ref->inside_mask & SAX_INSIDE_TAG_NAME)
				xml_ref->tag_name_cb(xml_ref);
		}
		xml_ref->inside_mask &= (0xff^SAX_INSIDE_TAG_NAME);
		xml_ref->inside_mask |= SAX_INSIDE_ATT_NAME;

		xml_ref->tag_name_start = xml_ref->tag_start;

		xml_ref->tag_start = NULL;
		return(SAX_RET_TAG_NAME);
	}
	xml_ref->tag_start = NULL;

	return(0);
}

int equal(struct _simple_sax * xml_ref)
{
	if(xml_ref->inside_mask & SAX_INSIDE_ATT)
		return 0;

	xml_ref->inside_mask &= (0xff^SAX_INSIDE_ATT_NAME);
	*(xml_ref->act_byte) = 0x00;

	if(xml_ref->tag_arg_name_cb)
		xml_ref->tag_arg_name_cb(xml_ref);

	xml_ref->tag_start = NULL;

	return(SAX_RET_ARG_NAME);
}


int sax_read_byte(simple_sax * xml_ref, uint8_t * byte)
{
	xml_ref->act_byte = byte;

	if(xml_ref->tag_start == NULL)
	{
		xml_ref->tag_start = byte;

		xml_ref->tag_act_size = 0;
	}

	xml_ref->tag_act_size++;

	if(xml_ref->char_callback[*byte] != NULL)
	{
		if((xml_ref->char_callback[*byte](xml_ref)) != 0)
		{
			if(xml_ref->tag_start != NULL)
				;
		}
	}
	return(0);
}

int sax_read_block(simple_sax * ref, uint8_t * bytes, int size)
{
	int i;
	uint8_t save_byte;

	for(i=0; i<size; i++)
	{
		save_byte = bytes[i];
		sax_read_byte(ref, bytes+i);
		ref->last_byte = save_byte;
	}
	return(0);
}

void sax_set_ignore(simple_sax * ref)
{
	ref->inside_mask |= SAX_IGNORE;
}

void sax_rem_ignore(simple_sax * ref)
{
	ref->inside_mask &= (0xff^SAX_IGNORE);
}

uint8_t * sax_get_tag(simple_sax * ref)
{
	return ref->tag_start;
}

int sax_save_tag_name(simple_sax * xml_ref)
{
	if(xml_ref->tag_name_start == NULL)
		return 0;

	if(strlen((const char *)xml_ref->tag_name_start) >= 100)
		return -1;

	strcpy((char *)xml_ref->tag_name_save,
		(const char *)xml_ref->tag_name_start);

	xml_ref->tag_name_start = &(xml_ref->tag_name_save[0]);

	// tag_act_size???

	return 0;
}

