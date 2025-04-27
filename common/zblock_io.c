/******************************************************************
 *
 * @short	zip (gzip) stream reader/writer
 *
 * project:	Trafalgar/OSM-Toolz
 *
 * modul:	zblock_io.c	source of zip stream reader/writer
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


/*
   The trafalgar package is free software.  You may redistribute it
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "zblock_io.h"

//uint8_t test_block[16777216];

void *block_func(void * data)
{
	int ret = 0;

	z_block * block_ptr = (z_block *)data;

	if((block_ptr->flags & ZB_READ) && (block_ptr->flags & ZB_USE_R_THREAD))
	{
		if(block_ptr->z_file)
		{
			int err;
			printf("# a %li\n", block_ptr->read_buff_size);
			ret = gzread(block_ptr->z_file, block_ptr->rd_cp_buff, block_ptr->read_buff_size);
			printf("# %s\n", gzerror(block_ptr->z_file, &err));
			printf("# %i \n", err);
			printf("# b %i\n", ret);
		}
	}

	if((block_ptr->flags & ZB_WRITE) && (block_ptr->flags & ZB_USE_W_THREAD))
	{
		ret = gzwrite(block_ptr->z_file, block_ptr->wr_cp_buff, block_ptr->wr_cp_filled);
		if(ret != block_ptr->wr_cp_filled)
		{
			printf("warning: %i != %i\n", ret, block_ptr->wr_cp_filled);
		}
		block_ptr->wr_cp_filled = 0;
	}

	return(NULL);
}

int zblock_new(z_block * z_block, uint8_t flags)
{
	z_block->flags = flags;

	// TMP!
	/*if(z_block->flags == ZB_WRITE)
		z_block->flags |= ZB_USE_W_THREAD;
	if(z_block->flags == ZB_READ)
		z_block->flags |= ZB_USE_R_THREAD;*/

	if(z_block->flags & ZB_USE_R_THREAD)
	{
		z_block->read_buff_size = ZB_RD_BLOCK_SIZE;
		z_block->read_size = z_block->read_buff_size * 2;
	}
	else
	{
		z_block->read_buff_size = ZB_RD_BLOCK_SIZE;
		z_block->read_size = z_block->read_buff_size;
	}
	z_block->z_file = NULL;

	z_block->write_size = ZB_WR_BLOCK_SIZE;

	z_block->act_size = 0;
	z_block->pos = 0;

	z_block->rd_buff = NULL;
	z_block->rd_cp_buff = NULL;
	z_block->wr_buff = NULL;
	z_block->wr_cp_buff = NULL;

	z_block->tflags[0] = 0;
	z_block->tflags[1] = 0;

	if(flags & ZB_WRITE)
	{
		//printf("\nmalloc z_block wr_buff: %lu", z_block->write_size);
		if((z_block->wr_buff = (uint8_t*)malloc(z_block->write_size + 1)) == NULL)
		{
			return(-1);
		}
		memset(z_block->wr_buff, 0, z_block->write_size + 1);

		if(z_block->flags & ZB_USE_W_THREAD)
		{
			if((z_block->wr_cp_buff = (uint8_t*)malloc(z_block->write_size + 1)) == NULL)
			{
				return(-1);
			}
			memset(z_block->wr_cp_buff, 0, z_block->write_size + 1);
		}
		z_block->rd_buff = NULL;
	}
	if(flags & ZB_READ)
	{
		//printf("\nmalloc rd_buff: %lu\n", z_block->read_size);
		if((z_block->rd_buff = (uint8_t*)malloc(z_block->read_size + 1)) == NULL)
		{
			return(-1);
		}
		memset(z_block->rd_buff, 0, z_block->read_size + 1);

		if(z_block->flags & ZB_USE_R_THREAD)
		{
			printf("\nmalloc *** rd_cp_buff: %lu\n", z_block->read_size);

			if((z_block->rd_cp_buff = (uint8_t*)malloc(z_block->read_size + 1)) == NULL)
			{
				return(-1);
			}
			memset(z_block->rd_cp_buff, 0, z_block->read_size + 1);
		}
		z_block->wr_buff = NULL;
	}

	//z_block->rd_cp_buff = (uint8_t*)malloc(z_block->read_size + 1);

	z_block->wr_filled = 0;

	return(0);
}

void zblock_set_block_size(z_block * z_block, uint64_t size, bool read)
{
	if(read)
	{
		z_block->read_buff_size = size;
	}
	else
	{
		z_block->write_size = size;
	}
}

void zblock_set_thread_opt(z_block * z_block, uint8_t flags)
{
	z_block->flags |= flags;
}

void zblock_reset(z_block * z_info)
{
	// TODO reset parameter here
}

int zblock_set_copy_mode(z_block * r_read)
{
	/* if(r_read->read_buff_size>0)
	{
		r_read->flags |= ZB_BLOCK_COPY;
		if((r_read->rd_buff = (uint8_t*)
			malloc(r_read->read_buff_size)) == NULL)
		{
			r_read->flags&=(0xff^ZB_BLOCK_COPY);
			return(-1);
		}
		if(pthread_create(&r_read->block_thread[READ_THREAD], NULL,
			block_func, (void*)r_read) != 0)
		{
			fprintf (stderr, "error creating thread\n");
			exit (EXIT_FAILURE);
		}

		// pthread_join(sax_thread, &ret);

		return(0);
	}*/
	return(-1);
}

void zblock_del(z_block * zblock)
{
	zblock->flags     = 0x00;
	zblock->read_buff_size = 0;
	zblock->z_file    = NULL;
	zblock->act_size  = 0;
	zblock->pos       = 0;

	if(zblock->flags & ZB_BLOCK_COPY)
	{
		//free(z_read->buff_2);
	}
	if(zblock->flags & ZB_WRITE)
	{
		if(zblock->wr_buff != NULL)
			free(zblock->wr_buff);
	}
	if(zblock->flags & ZB_READ)
	{
		if(zblock->rd_buff != NULL)
			free(zblock->rd_buff);
	}
}

int zblock_zip_size(z_block * z_read, const char * z_name)
{
	z_read->zip_size = 0;
	z_read->zip_read = 0;
	FILE * ffd = fopen(z_name, "r");
	if(ffd == NULL)
	{
		return -1;
	}
	fseek(ffd, 0, SEEK_END); // seek to end of file
	z_read->zip_size = ftell(ffd); // get current file pointer
	fseek(ffd, 0, SEEK_SET);
	fclose(ffd);
	return 0;
}

int zblock_rd_open(z_block * z_read, const char * z_name)
{
	int fd;

	//printf("zblock_rd_open: %s\n", z_name);

	if((fd = open(z_name,O_RDONLY/*|O_LARGEFILE*/))==(-1))
	{
		return(-1);
	}
	if((z_read->z_file = gzdopen(fd, "rb")) == NULL)
	{
		return(-1);
	}
	z_read->act_size = 0;
	z_read->pos = 0;
	return(0);
}

int zblock_wr_open(z_block * z_read, const char * z_name)
{
	int fd;

	if((fd=open(z_name,O_WRONLY | O_CREAT /*|O_LARGEFILE*/,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))==(-1))
	{
		return(-1);
	}

	if((z_read->z_file = gzdopen(fd,"wb")) == NULL)
	{
		return(-1);
	}

	z_read->act_size = 0;
	z_read->pos = 0;
	z_read->wr_filled = 0;

	return 0;
}

int zblock_read(z_block * z_read)
{
	if(z_read->z_file == NULL)
		return(-1);

	if(z_read->flags & ZB_USE_R_THREAD)
	{
		printf("z_file");

		if(z_read->z_file)
		{
			z_read->act_size = gzread(z_read->z_file,
				z_read->rd_cp_buff, z_read->read_buff_size);
			z_read->zip_read += z_read->read_buff_size;
			printf("## %li %li\n", z_read->zip_size, z_read->zip_read);
		}

		/*if(z_read->tflags[READ_THREAD] == 1)
		{
			pthread_join(z_read->block_thread[READ_THREAD], NULL);
		}
		//memcpy(z_read->rd_cp_buff, z_read->rd_buff, z_read->wr_filled);
		//if(pthread_create(&z_read->block_thread[READ_THREAD], NULL,
			//block_func, (void*)z_read) != 0)
		{
			//fprintf (stderr, "error creating thread\n");
			//exit (EXIT_FAILURE);
		}
		z_read->tflags[READ_THREAD] = 1;*/
	}
	else
	{
		//printf("rd %i %li \n", z_read->pos, z_read->read_size);
		z_read->act_size = gzread(z_read->z_file,
			z_read->rd_buff+z_read->pos, z_read->read_size);
		z_read->zip_read += z_read->read_buff_size;
	}
	return(z_read->act_size);
}

/*
 * append a token to the buffer
 * if the limit is reached call the zblock_wr_flush function
 */

int zblock_wr_append(z_block * z_write, const char * text, size_t length)
{
	int ret = 0;

	if(length > z_write->write_size)
		return -1;

	if((z_write->wr_filled + length) >= z_write->write_size)
	{
		ret = zblock_wr_flush(z_write);
		//return ret;
	}
	memcpy(z_write->wr_buff + z_write->wr_filled, text, length);
	z_write->wr_filled += length;
	return ret;
}

/* TODO:
 * copy the buffer compress buffer
 * start a new thread to compress (gzwrite)
 * wait to sync
 * return
 */
int zblock_wr_flush(z_block * z_write)
{
	int ret = 0;

	if(z_write->flags & ZB_USE_W_THREAD)
	{
		// check 'old' thread -> pthread_join
		//printf("zblock_wr_flush 1 write join %i\n", z_write->tflags[WRITE_THREAD]);
		if(z_write->tflags[WRITE_THREAD] == 1)
		{
			pthread_join(z_write->block_thread[WRITE_THREAD], NULL);
			//printf("zblock_wr_flush3 write join\n");
		}

		// critical section start
		memcpy(z_write->wr_cp_buff, z_write->wr_buff, z_write->wr_filled);
		z_write->wr_cp_filled = z_write->wr_filled;
		z_write->wr_filled = 0;
		// critical section end
		if(pthread_create(&z_write->block_thread[WRITE_THREAD], NULL,
			block_func, (void*)z_write) != 0)
		{
			fprintf (stderr, "error creating thread\n");
			exit (EXIT_FAILURE);
		}

		z_write->tflags[WRITE_THREAD] = 1;
		//printf("zblock_wr_flush set tflags %i\n", z_write->tflags[WRITE_THREAD]);
	}
	else
	{
		ret = gzwrite (z_write->z_file, z_write->wr_buff, z_write->wr_filled);
		z_write->wr_filled = 0;
	}
	return ret;
}

void zblock_close(z_block * z_block)
{
	if((z_block->flags & ZB_USE_W_THREAD) && (z_block->tflags[WRITE_THREAD] == 1))
	{
		pthread_join(z_block->block_thread[WRITE_THREAD], NULL);
	}

	gzclose(z_block->z_file);
	z_block->z_file = NULL;
	z_block->pos = 0;
	z_block->act_size = 0;
}

int zblock_size(z_block * z_read)
{
	return(z_read->read_buff_size);
}

uint8_t * zblock_buff(z_block * z_read, int * pos)
{
	*pos = z_read->act_size;

	return z_read->rd_buff + z_read->pos;
}

uint8_t zblock_last(z_block * z_read)
{
	return(z_read->rd_buff[z_read->act_size-1]);
}

uint8_t * zblock_first(z_block * z_read)
{
	return(z_read->rd_buff);
}

void zblock_set_start(z_block * z_read, uint8_t * start, int n)
{
	if(!z_read->z_file)
	{
		printf("!z_read->z_file\n");
		return;
	}

	if(z_read->flags & ZB_USE_R_THREAD)
	{
		if(z_read->tflags[READ_THREAD] == 0)
		{
			printf("tflags 0\n");
			z_read->act_size = gzread(z_read->z_file,
				z_read->rd_cp_buff, z_read->read_buff_size);
			z_read->tflags[READ_THREAD] = 1;
		}
		else
		{
			pthread_join(z_read->block_thread[READ_THREAD], NULL);
			exit(0);
		}

		if(n > 0)
		{
			memcpy(z_read->rd_buff, start, n);
		}
		memcpy(z_read->rd_buff+n, z_read->rd_cp_buff, z_read->read_buff_size);

		z_read->read_size = z_read->read_buff_size;
		z_read->pos       = n;

		if(pthread_create(&z_read->block_thread[READ_THREAD], NULL,
			block_func, (void*)z_read) != 0)
		{
			fprintf (stderr, "error creating thread\n");
			exit (EXIT_FAILURE);
		}

		//z_read->read_size = z_read->read_buff_size; // - n;
		//z_read->pos       = n;
	}
	else
	{
		if(n > 0)
		{
			memcpy(z_read->rd_buff, start, n);
		}
		z_read->read_size = z_read->read_buff_size - n;
		z_read->pos       = n;
	}
}

