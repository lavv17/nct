/*
 * NCT (Colored Tetris)
 *
 * Copyright (c) 1993, 1998 by Alexander V. Lukyanov (lav@yars.free.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include "nct.h"
#include "score.h"
#include "confpaths.h"

#define SCORE_TABLE_SIZE \
   (SCORE_TABLE_LEN*MAX_COLORS*sizeof(struct score_line_packed))
#define SCORE_TABLE_FILE SCOREDIR "/nct.score"

struct score_line_packed
{
   unsigned char score[4];
   unsigned char full_lines[3];
   unsigned char levels;
   char name[SCORE_NAME_LEN];
};

static struct score_line_packed *score_table;
static int score_table_fd;

int last_added_score=-1;

static
void pack_score_line(struct score_line_packed *p, struct score_line *u)
{
   unsigned long score=u->score;
   unsigned full_lines=u->full_lines;
   unsigned mask=255;
   int i;
   
   for(i=0; i<sizeof(p->score); i++)
   {
      p->score[i]=score&mask;
      score>>=8;
   }
   for(i=0; i<sizeof(p->full_lines); i++)
   {
      p->full_lines[i]=full_lines&mask;
      full_lines>>=8;
   }
   p->levels=((u->start_level&15)<<4)+(u->end_level&15);
   memset(p->name,0,sizeof(p->name));
   strcpy(p->name,u->name);
}

static
void unpack_score_line(struct score_line *u, struct score_line_packed *p)
{
   unsigned long score=0;
   unsigned full_lines=0;
   int i;
   
   for(i=sizeof(p->score)-1; i>=0; i--)
   {
      score<<=8;
      score|=p->score[i];
   }
   u->score=score;

   for(i=sizeof(p->full_lines)-1; i>=0; i--)
   {
      full_lines<<=8;
      full_lines|=p->full_lines[i];
   }
   u->full_lines=full_lines;

   u->start_level=(p->levels>>4)&15;
   u->end_level=p->levels&15;

   strncpy(u->name,p->name,sizeof(p->name));
   u->name[sizeof(p->name)]=0;
}

void lock_score_table(int how,int num_colors,int num,int count)
{
   struct flock lk;
   lk.l_type=how;
   lk.l_whence=0;
   lk.l_start=sizeof(*score_table)*((num_colors-1)*SCORE_TABLE_LEN+num);
   lk.l_len=sizeof(*score_table)*count;
   fcntl(score_table_fd,F_SETLKW,&lk);
}
void unlock_score_table()
{
   /* zero len means to eof */
   lock_score_table(F_UNLCK,1,0,0);
}

struct score_line *get_score_line(int num_colors,int num)
{
   static struct score_line u;
   if(!score_table)
      return 0;
   lock_score_table(F_RDLCK,num_colors,num,1);
   unpack_score_line(&u,score_table+(num_colors-1)*SCORE_TABLE_LEN+num);
   return &u;
}

static int better_score(struct score_line *u,unsigned long score,
			unsigned full_lines)
{
   return u->score<score || (u->score==score && u->full_lines>full_lines);
}

int check_score(unsigned long score,unsigned full_lines,int num_colors)
{
   struct score_line *last;
   int res;
   
   if(!score_table)
      return 0;
   
   if(full_lines<5 || score<1000) /* too bad */
      return 0;
   
   last=get_score_line(num_colors,SCORE_TABLE_LEN-1);
   res=better_score(last,score,full_lines);
   if(!res)
      unlock_score_table();
   return res;
}

void insert_score(const char *name, unsigned long score,
		  unsigned full_lines,int num_colors,int start_level,int end_level)
{
   int pos;
   struct score_line *u;
   
   for(pos=0; pos<SCORE_TABLE_LEN; pos++)
   {
      u=get_score_line(num_colors,pos);
      if(better_score(u,score,full_lines))
      {
	 struct score_line_packed *p=
		     score_table+(num_colors-1)*SCORE_TABLE_LEN+pos;

	 lock_score_table(F_WRLCK,num_colors,pos,SCORE_TABLE_LEN-pos);

	 if(SCORE_TABLE_LEN-pos-1>0)
	    memmove(p+1,p,sizeof(*p)*(SCORE_TABLE_LEN-pos-1));

	 strncpy(u->name,name,sizeof(u->name));
	 u->score=score;
	 u->full_lines=full_lines;
	 u->start_level=start_level;
	 u->end_level=end_level;

	 pack_score_line(p,u);
	 last_added_score=pos;
	 break;
      }
   }
   unlock_score_table();
}

void score_init()
{
   score_table_fd=open(SCORE_TABLE_FILE,O_RDWR|O_CREAT,0660);
   if(score_table_fd==-1)
   {
      perror(SCORE_TABLE_FILE);
      sleep(2);
      return;
   }
   ftruncate(score_table_fd,SCORE_TABLE_SIZE);
   score_table=(struct score_line_packed *)mmap(0,SCORE_TABLE_SIZE,
		  PROT_READ|PROT_WRITE,MAP_SHARED,score_table_fd,0);
}
