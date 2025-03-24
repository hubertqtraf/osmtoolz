/******************************************************************
 *
 * @short	zip (gzip) stream reader/writer
 *
 * project:	Trafalgar/OSM-Toolz
 *
 * modul:	zblock_io.h	header of zip stream reader/writer
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
   Boston, MA 02110-1301, USA. */

#ifndef _READ_ZBLOCK_H
#define _READ_ZBLOCK_H

#include <stdint.h>
#include <stdbool.h>
#include <zlib.h>
#include <pthread.h>

//#define ZB_RD_BLOCK_SIZE 0x0000000000001000
#define ZB_RD_BLOCK_SIZE 0x0000000001000000
//#define ZB_WR_BLOCK_SIZE 0x0000000000100000
#define ZB_WR_BLOCK_SIZE 0x0000000004000000

#define ZB_READ         0x01
#define ZB_WRITE        0x02

#define ZB_BLOCK_COPY   0x10
#define ZB_USE_R_THREAD 0x20
#define ZB_USE_W_THREAD 0x40

#define NUM_THREADS  2
#define READ_THREAD  0
#define WRITE_THREAD 1

typedef struct
{
	uint8_t flags;
	uint8_t * rd_buff;
	uint8_t * rd_cp_buff;

	uint8_t * wr_buff;
	uint8_t * wr_cp_buff;

	uint64_t read_buff_size;
	uint64_t read_size;
	uint64_t write_size;
	int wr_filled;
	int wr_cp_filled;

	int act_size;
	gzFile z_file;
	int pos;

	pthread_t block_thread[NUM_THREADS];
        uint8_t tflags[NUM_THREADS];
}z_block;

int zblock_new(z_block * z_read, uint8_t flags);
void zblock_set_block_size(z_block * z_block, uint64_t size, bool read);
void zblock_set_thread_opt(z_block * z_block, uint8_t flags);
void zblock_reset(z_block * z_info);
int zblock_set_copy_mode(z_block * r_read);
void zblock_del(z_block * z_read);
int zblock_rd_open(z_block * z_read, const char * fname);
int zblock_wr_open(z_block * z_read, const char * fname);
int zblock_read(z_block * z_read);
int zblock_wr_append(z_block * z_read, const char * text, size_t length);
int zblock_wr_flush(z_block * z_read);

void zblock_close(z_block * z_read);
int zblock_size(z_block * z_read);
uint8_t * zblock_buff(z_block * z_read, int * pos);
uint8_t zblock_last(z_block * z_read);
uint8_t * zblock_first(z_block * z_read);
void zblock_set_start(z_block * z_read, uint8_t * start, int n);

#endif // _READ_ZBLOCK_H

