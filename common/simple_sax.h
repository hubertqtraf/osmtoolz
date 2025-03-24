/******************************************************************
 *
 * @short	simple SAX XML interpreter
 *
 * project:	Trafalgar/OSM-Toolz
 *
 * modul:	simple_sax.h	header of SAX XML interpreter
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

#ifndef _SIMPLE_SAX_H
#define _SIMPLE_SAX_H

#define SAX_INSIDE_TAG_NAME	0x01
#define SAX_INSIDE_ATT          0x02
#define SAX_INSIDE_ATT_NAME	0x04
#define SAX_TAG_END_FLAG	0x08

#define SAX_INSIDE_COMMENT      0x20
#define SAX_INSIDE_TAG          0x40
#define SAX_IGNORE		0x80

#define SAX_RESTORE_NAME	0x10

#define SAX_RET_TAG_NAME	4
#define SAX_RET_ARG_NAME	5
#define SAX_RET_ARG		6
#define SAX_RET_ERR		-1

#define SAX_CB_TAG_OPEN		100
#define SAX_CB_TAG_CLOSE	101
#define SAX_CB_ARG_VALUE	102
#define SAX_CB_TAG_NAME		103
#define SAX_CB_ARG_NAME		104

#include <stdint.h>

typedef struct _simple_sax
{
	uint8_t inside_mask;

	uint8_t * tag_start;
	uint8_t * tag_name_start;
	uint8_t tag_name_save[100];

	uint8_t * act_byte;
	uint8_t last_byte;

	int tag_act_size;
	//int tag_len;
	int tag_name_len;

	int tag_buf_size;

	int (**char_callback)(struct _simple_sax *);

	int (* tag_open_cb)(struct _simple_sax *);
	int (* tag_close_cb)(struct _simple_sax *);
	int (* tag_arg_end_cb)(struct _simple_sax *);
	int (* tag_name_cb)(struct _simple_sax *);
	int (* tag_arg_name_cb)(struct _simple_sax *);

	void * d_ref;

}simple_sax;

void sax_init(simple_sax * ref, uint8_t accept);

void sax_cleanup(simple_sax * ref);

void sax_set_data_ref(simple_sax * ref, void *);

void sax_add_cb(struct _simple_sax * ref, int (* cb)(struct _simple_sax *), int type);

int sax_read_byte(simple_sax * xml_ref, uint8_t * byte);

int sax_read_block(simple_sax * ref, uint8_t * byte, int size);

void sax_set_ignore(simple_sax * ref);

void sax_rem_ignore(simple_sax * ref);

uint8_t * sax_get_tag(simple_sax * ref);

int sax_save_tag_name(simple_sax * ref);

#endif // _SIMPLE_SAX_H

