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

void score_init();
int check_score(unsigned long score,unsigned full_lines,int num_colors);
void insert_score(const char *name, unsigned long score,
		  unsigned full_lines,int num_colors,int start_level,int end_level);

#define SCORE_TABLE_LEN 20
#define SCORE_NAME_LEN	26

struct score_line
{
   unsigned long score;
   unsigned full_lines;
   unsigned start_level;
   unsigned end_level;
   char name[SCORE_NAME_LEN+1];
};

struct score_line *get_score_line(int num_colors,int num);

extern int last_added_score;
