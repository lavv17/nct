/*
 * NCT (Colored Tetris)
 *
 * Copyright (c) 1993, 1998, 1999 by Alexander V. Lukyanov (lav@yars.free.net)
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <pwd.h>
#include <ctype.h>
#ifdef HAVE_NCURSES_H
# include <ncurses.h>
#else
# include <curses.h>
#endif

#include "nct.h"
#include "score.h"

#define  FIG_NUM     7
#define  SQUARE_NUM  4
#define  ORIENT_NUM  4

#define  WELL_DEPTH  22
#define  WELL_WIDTH  10
#define	 WELL_X ((COLS-WELL_WIDTH*2)/2)

struct coord
{
   int   x,y;
};

void  DrawFigureAt(WINDOW *,int x,int y,struct coord *fig,chtype c);
void  SaveUnderFigure(int x,int y,struct coord *fig,chtype *c);
void  RestoreUnderFigure(int x,int y,struct coord *fig,chtype *c);
int   CheckFigurePlace(int x,int y,struct coord *fig,int color);
void  RandomNext(void);
chtype ColorNum2Cell(int cn);
void  RandomFigure(void);
void  DisplayStatistics(void);
void  RedrawWell(void);
void  TerminateGame(void);
void  CheckKey(void);
void  AskStartParams(void);
void  InitializeGame(void);
unsigned Speed2Delay(int speed);
struct coord   *CurrentFigurePtr(void);
void  GlueFigure(void);
int   TryNewPos(int x,int y,int orient);
void  CheckFullLines(void);
void  DrawNext(void);
void  ClearNext(void);
void  Play(void);

/* Current game parameters */
int            ColorsNum;
int            StartSpeed;
int            CurrentSpeed;
unsigned       FullLines;
unsigned long  Score;
int            ShowNext;
int            HaveSeenNext;

/* Current figure state */
int            CurrentFigure;
int            CurrentOrient;
int            CurrentColor;
struct coord   CurrentPos;
unsigned       FigureScore;
chtype	       CurrentCell;

int            NextFigure;
int            NextColor;

int	       LastKey;

int	 CurrentWell[WELL_DEPTH][WELL_WIDTH];
chtype	 Save[SQUARE_NUM];

WINDOW   *WellWindow;
WINDOW   *NextWindow;
WINDOW   *AskWindow;
WINDOW   *ColorWindow;
WINDOW   *WellDecor;
WINDOW   *StatusWindow;

chtype WellBG;

chtype ColorTable[MAX_COLORS];

char PlayerName[SCORE_NAME_LEN+1];

struct coord   Figures[FIG_NUM][ORIENT_NUM][SQUARE_NUM]=
{
   {
      {{1,1},{2,1},{1,2},{2,2}},    /*        */
      {{1,1},{2,1},{1,2},{2,2}},    /*  %%%%  */
      {{1,1},{2,1},{1,2},{2,2}},    /*  %%%%  */
      {{1,1},{2,1},{1,2},{2,2}}     /*        */
   },
   {
      {{2,1},{3,1},{1,2},{2,2}},    /*        */
      {{1,0},{1,1},{2,1},{2,2}},    /*    %%%%*/
      {{2,1},{3,1},{1,2},{2,2}},    /*  %%%%  */
      {{1,0},{1,1},{2,1},{2,2}}     /*        */
   },
   {
      {{0,1},{1,1},{1,2},{2,2}},    /*        */
      {{2,0},{1,1},{2,1},{1,2}},    /*%%%%    */
      {{0,1},{1,1},{1,2},{2,2}},    /*  %%%%  */
      {{2,0},{1,1},{2,1},{1,2}}     /*        */
   },
   {
      {{0,1},{1,1},{2,1},{3,1}},    /*        */
      {{2,0},{2,1},{2,2},{2,3}},    /*%%%%%%%%*/
      {{0,1},{1,1},{2,1},{3,1}},    /*        */
      {{2,0},{2,1},{2,2},{2,3}}     /*        */
   },
   {
      {{0,1},{1,1},{2,1},{1,2}},    /*        */
      {{1,0},{1,1},{2,1},{1,2}},    /*%%%%%%  */
      {{1,0},{0,1},{1,1},{2,1}},    /*  %%    */
      {{1,0},{0,1},{1,1},{1,2}}     /*        */
   },
   {
      {{0,1},{1,1},{2,1},{2,2}},    /*        */
      {{1,0},{2,0},{1,1},{1,2}},    /*%%%%%%  */
      {{0,1},{0,2},{1,2},{2,2}},    /*    %%  */
      {{2,0},{2,1},{1,2},{2,2}}     /*        */
   },
   {
      {{0,1},{1,1},{2,1},{0,2}},    /*        */
      {{1,0},{1,1},{1,2},{2,2}},    /*%%%%%%  */
      {{2,1},{0,2},{1,2},{2,2}},    /*%%      */
      {{1,0},{2,0},{2,1},{2,2}}     /*        */
   }
};

void  DrawFigureAt(WINDOW *w,int x,int y,struct coord *fig,chtype c)
{
   int   i;
   chtype oldbkgd=getbkgd(w);
   wbkgdset(w,A_NORMAL|' ');
   for(i=SQUARE_NUM; i>0; i--,fig++)
   {
      if(wmove(w,y+fig->y,x+fig->x*2)==ERR)
	 continue;
      waddch(w,c);
      waddch(w,c);
   }
   wbkgdset(w,oldbkgd);
}

void  SaveUnderFigure(int x,int y,struct coord *fig,chtype *c)
{
   int   i;
   for(i=SQUARE_NUM; i>0; i--,fig++,c++)
      *c=mvwinch(WellWindow,y+fig->y,(x+fig->x)*2);
}
void  RestoreUnderFigure(int x,int y,struct coord *fig,chtype *c)
{
   int   i;
   wbkgdset(WellWindow,A_NORMAL|' ');
   for(i=SQUARE_NUM; i>0; i--,fig++,c++)
   {
      if(wmove(WellWindow,y+fig->y,(x+fig->x)*2)==ERR)
	 continue;
      waddch(WellWindow,*c);
      waddch(WellWindow,*c);
   }
   wbkgdset(WellWindow,WellBG);
}

int   CheckFigurePlace(int x,int y,struct coord *fig,int color)
{
   int   i;
   for(i=SQUARE_NUM; i>0; i--,fig++)
   {
      if(y+fig->y<0) /* top limit does not exist */
         continue;
      if(y+fig->y>=WELL_DEPTH || x+fig->x<0 || x+fig->x>=WELL_WIDTH
      || CurrentWell[y+fig->y][x+fig->x]>=color)
         return(0);
   }
   return(1);
}

void  RandomNext(void)
{
   NextFigure=rand()%FIG_NUM;
   NextColor=rand()%ColorsNum;
}

chtype	 ColorNum2Cell(int cn)
{
   if(has_colors())
      return(ColorTable[(cn+1)*MAX_COLORS/ColorsNum-1]);
   else
      return(ColorTable[0]);
}

void  RandomFigure()
{
   CurrentFigure=NextFigure;
   CurrentColor=NextColor;
   RandomNext();
   CurrentOrient=0;
   CurrentPos.x=WELL_WIDTH/2-2;
   CurrentPos.y=-1;
   CurrentCell=ColorNum2Cell(CurrentColor);
   /*
    * Potential score for the current figure. Actual one depends
    * on the drop height and whether user has seen NextFigure.
    * Less for light colors, because they are less significant.
    */
   FigureScore = WELL_DEPTH+4 + 3*CurrentSpeed*(CurrentColor+1)/ColorsNum;
}

void  DisplayStatistics(void)
{
   wbkgdset(StatusWindow,A_NORMAL|' ');
   wmove(StatusWindow,1,0);
   wprintw(StatusWindow,"  Speed:      %9u\n",CurrentSpeed);
   wprintw(StatusWindow,"  Full lines: %9u\n\n",FullLines);
   wprintw(StatusWindow,"  SCORE:      %9lu\n",Score);
   wbkgdset(StatusWindow,A_DIM|' ');
   box(StatusWindow,0,0);

   wnoutrefresh(StatusWindow);
}

void  RedrawWell(void)
{
   int   x,y;
   chtype c;

   werase(WellWindow);
   wbkgdset(WellWindow,A_NORMAL|' ');
   for(y=0; y<WELL_DEPTH; y++)
   {
      for(x=0; x<WELL_WIDTH; x++)
      {
	 wmove(WellWindow,y,x*2);
	 if(CurrentWell[y][x]==-1)
	    c=(" ."[(x+y)&1])|WellBG;
         else
	    c=ColorNum2Cell(CurrentWell[y][x]);
	 waddch(WellWindow,c);
	 waddch(WellWindow,c);
      }
   }
   wbkgdset(WellWindow,WellBG);
}

void  TerminateGame(void)
{
   clear();
   curs_set(1);
   refresh();
   endwin();
   exit(0);
}

unsigned long Timer()
{
   struct timeval t;
   gettimeofday(&t,0);
   return ((unsigned long)t.tv_sec)*1000+t.tv_usec/1000;
}

void  ReadKey()
{
   LastKey=getch();
}
int   WaitKey(int delay)
{
   if(delay<0)
      delay=0;
   timeout(delay);
   LastKey=getch();
   timeout(-1);
#ifndef NCURSES_VERSION /* workaround for not working timeout(-1) */
   nocbreak();
   cbreak();
#endif
   return LastKey;
}

void  CheckKey(void)
{
   switch(LastKey)
   {
   case(27):
   case(KEY_CANCEL):
   case(KEY_EXIT):
   case('q'):
   case('Q'):
      TerminateGame();
      break;
   case(KEY_REFRESH):
   case('L'-'@'):
      wrefresh(curscr);
      break;
   }
}

void  cwaddstr(WINDOW *w,int line,const char *m)
{
   int my,mx;
   getmaxyx(w,my,mx);
   mvwaddstr(w,line,(mx-strlen(m))/2,m);
}

void  AskStartParams()
{
   werase(AskWindow);
   box(AskWindow,0,0);

   mvwaddstr(AskWindow,2,4,"Start speed [0-9]: ");
   curs_set(1);
   wrefresh(AskWindow);

   for(;;)
   {
      ReadKey();
      if(LastKey>='0' && LastKey<='9')
         break;
      CheckKey();
   }
   StartSpeed=LastKey-'0';

   if(has_colors())
   {
      werase(AskWindow);
      box(AskWindow,0,0);

      mvwprintw(AskWindow,2,4,"Colors number [1-%d]:",MAX_COLORS);
      wrefresh(AskWindow);

      for(;;)
      {
         ReadKey();
         if(LastKey>='1' && LastKey<=MAX_COLORS+'0')
            break;
         CheckKey();
      }
      ColorsNum=LastKey-'0';
   }
   else
      ColorsNum=1;

   curs_set(0);
}

void  ShowColors(void)
{
   int   c;
   WINDOW *sub;
   int h,w,i;

   wbkgdset(ColorWindow,A_NORMAL|' ');
   werase(ColorWindow);

   if(ColorsNum==1)
      return;

   getmaxyx(ColorWindow,h,w);

   for(c=0; c<ColorsNum; c++)
   {
      chtype b=ColorNum2Cell(c);
      wmove(ColorWindow,c+1,2);
      for(i=w-4; i>0; i--)
	 waddch(ColorWindow,b);
   }
   
   sub=derwin(ColorWindow,ColorsNum+2,w,0,0);
   wbkgdset(sub,A_DIM|' ');
   box(sub,0,0);
   delwin(sub);
   
   wnoutrefresh(ColorWindow);
}

void  ShowWellDecor()
{
   int x,y,my,mx;
   chtype decor_char=ACS_VLINE;
   getmaxyx(WellDecor,my,mx);
   werase(WellDecor);
   for(y=0; y<my; y++)
   {
      wmove(WellDecor,y,0);
      waddch(WellDecor,decor_char);
      waddch(WellDecor,decor_char);
      wmove(WellDecor,y,mx-2);
      waddch(WellDecor,decor_char);
      waddch(WellDecor,decor_char);
   }
   wmove(WellDecor,my-1,0);
   waddch(WellDecor,ACS_LLCORNER);
   waddch(WellDecor,ACS_BTEE);
   for(x=2; x<mx-2; x++)
      waddch(WellDecor,ACS_HLINE);
   waddch(WellDecor,ACS_BTEE);
   waddch(WellDecor,ACS_LRCORNER);
   wnoutrefresh(WellDecor);
}

void  InitializeGame(void)
{
   int   x,y;

   srand(Timer());

   AskStartParams();

   clear();
   refresh();

   for(y=0; y<WELL_DEPTH; y++)
      for(x=0; x<WELL_WIDTH; x++)
         CurrentWell[y][x]=-1;

   CurrentSpeed=StartSpeed;
   FullLines=Score=0;
   ShowNext=0;
   RandomNext();

   ShowWellDecor();
   if(ColorsNum>1)
      ShowColors();
   ClearNext();
   RedrawWell();
   DisplayStatistics();
}

unsigned Speed2Delay(int speed)
{
   return(55*(10-speed));
}

struct coord   *CurrentFigurePtr(void)
{
   return(Figures[CurrentFigure][CurrentOrient]);
}

void  GlueFigure(void)
{
   struct coord   *fig;
   int            i;

   for(fig=CurrentFigurePtr(),i=SQUARE_NUM; i>0; i--,fig++)
      CurrentWell[CurrentPos.y+fig->y][CurrentPos.x+fig->x]=CurrentColor;
}

int   TryNewPos(int x,int y,int orient)
{
   if(!CheckFigurePlace(x,y,Figures[CurrentFigure][orient],CurrentColor))
      return(0);

   RestoreUnderFigure(CurrentPos.x,CurrentPos.y,CurrentFigurePtr(),Save);

   CurrentPos.x=x;
   CurrentPos.y=y;
   CurrentOrient=orient;

   SaveUnderFigure(x,y,Figures[CurrentFigure][orient],Save);
   DrawFigureAt(WellWindow,x*2,y,Figures[CurrentFigure][orient],CurrentCell);

   return(1);
}

void  CheckFullLines(void)
{
   int   in;
   int   out;
   int   x;
   int   color;

   for(in=WELL_DEPTH-1,out=in; in>=0; in--)
   {
      color=CurrentWell[in][0];
      for(x=WELL_WIDTH-1; x>0 && CurrentWell[in][x]==color && color!=-1; x--);
      if(x!=0)
      {
         if(in!=out)
            memcpy(CurrentWell[out],CurrentWell[in],sizeof(int)*WELL_WIDTH);
         out--;
      }
   }
   if(out>=0)
   {
      FullLines+=out+1;
      /* score for full line of light color */
      Score+=(out+1)*(ColorsNum-CurrentColor-1)*10*CurrentSpeed;

      for( ; out>=0; out--)
         for(x=WELL_WIDTH-1; x>=0; x--)
            CurrentWell[out][x]=-1;

      RedrawWell();
   }
}

void  DrawNext(void)
{
   struct coord *nfig=Figures[NextFigure][0];
   struct coord *fig;
   int i,mx=0,mn=4;

   for(fig=nfig,i=SQUARE_NUM; i>0; i--,fig++)
   {
      if(mx<fig->x)
	 mx=fig->x;
      if(mn>fig->x)
	 mn=fig->x;
   }
   DrawFigureAt(NextWindow,2+3-(mx+mn),1,nfig,ColorNum2Cell(NextColor));
}
void  ClearNext(void)
{
   werase(NextWindow);
   box(NextWindow,0,0);
}

void Sync()
{
   wnoutrefresh(WellWindow);
   wnoutrefresh(NextWindow);
   doupdate();
}

void  Play(void)
{
   unsigned long end_time,start_time;
   int   y;

   for(;;)
   {
      if(ShowNext)
         ClearNext();
      RandomFigure();
      if(ShowNext)
      {
         DrawNext();
         HaveSeenNext=1;
      }

      if(!CheckFigurePlace(CurrentPos.x,CurrentPos.y,
                           Figures[CurrentFigure][CurrentOrient],CurrentColor))
         return;

      SaveUnderFigure(CurrentPos.x,CurrentPos.y,CurrentFigurePtr(),Save);
      DrawFigureAt(WellWindow,
	 CurrentPos.x*2,CurrentPos.y,CurrentFigurePtr(),CurrentCell);

      start_time=Timer();
      for(;;)
      {
         end_time=start_time+Speed2Delay(CurrentSpeed);

         for(;;)
         {
            if(Timer()-start_time>=end_time-start_time)
            {
               FigureScore--;
               break;
            }
	    Sync();
            switch(WaitKey(end_time-Timer()))
            {
	    /* LEFT */
            case('7'):
            case('j'):
            case('J'):
            case(KEY_HOME):
	    case(KEY_A1):
	    case(KEY_LEFT):
               TryNewPos(CurrentPos.x-1,CurrentPos.y,CurrentOrient);
               break;
	    /* RIGHT */
            case('9'):
            case('l'):
            case('L'):
            case(KEY_PPAGE):
	    case(KEY_A3):
	    case(KEY_RIGHT):
               TryNewPos(CurrentPos.x+1,CurrentPos.y,CurrentOrient);
               break;
	    /* ROTATE */
            case('8'):
            case('k'):
            case('K'):
            case(KEY_UP):
               TryNewPos(CurrentPos.x,CurrentPos.y,(CurrentOrient+1)%ORIENT_NUM);
               break;
	    /* HELP */
            case('1'):
	    case(KEY_HELP):
	    case(KEY_END):
	    case(KEY_C1):
               if(!ShowNext)
               {
                  ShowNext=1;
                  HaveSeenNext=1;
                  DrawNext();
               }
               else
               {
                  ClearNext();
                  ShowNext=0;
               }
               break;
	    /* DROP */
            case(' '):
	    case('5'):
	    case(KEY_DOWN):
               for(y=CurrentPos.y;
                   CheckFigurePlace(CurrentPos.x,y+1,CurrentFigurePtr(),CurrentColor);
                   y++);
               TryNewPos(CurrentPos.x,y,CurrentOrient);
	       Sync();
               goto glue;
	    case(KEY_SUSPEND):
	    case('p'):
	    case('P'):
	       flushinp();
	       ReadKey();
	       CheckKey();
	       end_time=Timer();
	       break;
	    default:
	       CheckKey();
	       break;
            }
         }

         if(!TryNewPos(CurrentPos.x,CurrentPos.y+1,CurrentOrient))
         {
glue:       GlueFigure();
            CheckFullLines();
            Score+=FigureScore-HaveSeenNext*5;
            if(FullLines/10>CurrentSpeed)
               CurrentSpeed=FullLines/10;
            if(CurrentSpeed>9)
               CurrentSpeed=9;
            DisplayStatistics();
            break;
         }
	 start_time=end_time;
      }
   }
}

void GameOver()
{
#define W (WELL_WIDTH*2+4)
   WINDOW *w=newwin(1,W,WELL_DEPTH+1,WELL_X-2);
   if(!w)
      return;
   
   wbkgd(w,(has_colors()?COLOR_PAIR(11):0)|A_BOLD|' ');
   cwaddstr(w,0,"G A M E    O V E R");
   beep();
   flushinp();
   wrefresh(w);

   sleep(1);
   flushinp();
   
   delwin(w);
#undef W
}

void AskName()
{
   char *s;
   int i;
   chtype a1,a2;
   
#define W (SCORE_NAME_LEN+6)
#define H (4+4)
   WINDOW *w=newwin(H,W,LINES-H-2,(COLS-W)/2);
   if(!w)
      return;

   a1=(has_colors() ? COLOR_PAIR(9)|A_BOLD : A_REVERSE)|' ';
   a2=(has_colors() ? COLOR_PAIR(10)|A_BOLD : A_BOLD)|'_';

   wbkgd(w,a1);

   box(w,0,0);

   cwaddstr(w,2,"Congratulations!");
   mvwaddstr(w,4,3,"Enter your name:");

   echo();
   curs_set(1);
   keypad(w,TRUE);
   meta(w,TRUE);

   /* push old name */
   for(s=PlayerName+strlen(PlayerName); s>PlayerName; s--)
      ungetch((unsigned char)s[-1]);

   /* clear input field */
   wbkgdset(w,a2);
   wmove(w,5,3);
   for(i=0; i<sizeof(PlayerName)-1; i++)
      waddch(w,' ');
   
   /* now read it */
   wmove(w,5,3);
   wgetnstr(w,PlayerName,sizeof(PlayerName)-1);
   
   for(s=PlayerName+strlen(PlayerName); s>PlayerName; s--)
      if(!isprint((unsigned char)s[-1]))
	 s[-1]='?';

   noecho();
   delwin(w);
#undef W
#undef H
}

void ShowHallOfFame()
{
#define W (6+2+2+SCORE_NAME_LEN+1+3+1+5+1+9+1)
#define H (SCORE_TABLE_LEN+4)

   int i;
   chtype a1,a2;
   char msg[80];
   WINDOW *w=newwin(H,W,(LINES-H)/2,(COLS-W)/2);
   if(!w)
      return;

   a1=(has_colors() ? COLOR_PAIR(9)|A_BOLD : A_REVERSE)|' ';
   a2=(has_colors() ? COLOR_PAIR(10)|A_BOLD : A_BOLD)|' ';
   
   wbkgd(w,a1);

   box(w,0,0);

   if(ColorsNum>1)
      sprintf(msg,"Hall of Fame for %d colors",ColorsNum);
   else
      sprintf(msg,"Hall of Fame for classic game");
   cwaddstr(w,1,msg);
   
   for(i=0; i<SCORE_TABLE_LEN; i++)
   {
      struct score_line *l=get_score_line(ColorsNum,i);
      if(!l)
	 goto out;
      if(l->score)
      {
	 wbkgdset(w,i==last_added_score?a2:a1);
	 mvwprintw(w,i+3,3,"%2d. %-*s %1u-%1u %5u %9lu",i+1,SCORE_NAME_LEN,
	    l->name,l->start_level,l->end_level,l->full_lines,l->score);
      }
      else
      {
	 if(i==0)
	    goto out;
      }
   }
   unlock_score_table();
   
   curs_set(0);
   wrefresh(w);
   ReadKey();
   CheckKey();

   if(LastKey>='0' && LastKey<='9')
      ungetch(LastKey); /* for speed dialog */

out:
   unlock_score_table();
   delwin(w);
#undef W
#undef H
}

void winerror()
{
   endwin();
   fprintf(stderr,"\nCould not create curses window!\n");
   exit(1);
}

void drop_privs()
{
   setuid(getuid());
   setgid(getgid());
}

void make_default_player_name()
{
/*   char *comma;*/
   struct passwd *p=getpwuid(getuid());
   if(!p)
      return;
   strncpy(PlayerName,p->pw_name,sizeof(PlayerName));
   PlayerName[sizeof(PlayerName)-1]=0;
/*   comma=strchr(PlayerName,',');*/
/*   if(comma)*/
/*      *comma=0;*/
}

int   main()
{
   int i;

   score_init();
   drop_privs();

   initscr();
   start_color();
#ifdef HAVE_USE_DEFAULT_COLORS
   use_default_colors();
#endif
   cbreak();
   noecho();
   keypad(stdscr,TRUE);
   meta(stdscr,TRUE);
   
   addstr("NCT "VERSION"  Copyright (c) 1998-1999 Alexander V. Lukyanov <lav@yars.free.net>\n\n"
	  "There is absolutely no warranty. This is free software that gives you\n"
	  "freedom to use, modify and redistribute it under certain conditions.\n"
	  "See COPYING (GNU GPL) for details.\n");
   
   refresh();

   if(has_colors())
   {
#if 0
      if(can_change_color())
      {
	 init_color(COLOR_YELLOW,250*1000/256,240*1000/256,0);
      }
#endif
      init_pair(1,COLOR_BLACK,COLOR_BLUE);
      init_pair(2,COLOR_BLACK,COLOR_GREEN);
      init_pair(3,COLOR_BLACK,COLOR_RED);
      init_pair(4,COLOR_BLACK,COLOR_MAGENTA);
      init_pair(5,COLOR_BLACK,COLOR_YELLOW);
      init_pair(6,COLOR_BLACK,COLOR_WHITE);
      for(i=0; i<MAX_COLORS; i++)
	 ColorTable[i]=COLOR_PAIR(i+1)|' ';

      init_pair(7,COLOR_WHITE,COLOR_BLACK);
      init_pair(8,COLOR_BLACK,COLOR_CYAN);
      init_pair(9,COLOR_WHITE,COLOR_GREEN);
      init_pair(10,COLOR_YELLOW,COLOR_GREEN);
      init_pair(11,COLOR_RED,COLOR_BLACK);
      WellBG=' '|COLOR_PAIR(7)|A_DIM;
   }
   else
   {
      ColorTable[0]=A_REVERSE|' ';
      WellBG=A_DIM|' ';
   }
   
   WellWindow=newwin(WELL_DEPTH,WELL_WIDTH*2,0,WELL_X);
   if(!WellWindow)
      winerror();
   wbkgd(WellWindow,WellBG);
   leaveok(WellWindow,TRUE);
   NextWindow=newwin(SQUARE_NUM+2,(SQUARE_NUM+2)*2,LINES-3-(SQUARE_NUM+2),4);
   if(!NextWindow)
      winerror();
   wbkgd(NextWindow,WellBG);
   leaveok(NextWindow,TRUE);
   box(NextWindow,0,0);

   WellDecor=newwin(WELL_DEPTH+1,WELL_WIDTH*2+4,0,WELL_X-2);
   if(!WellDecor)
      winerror();
   wbkgd(WellDecor,WellBG);

#define ASK_H 5
#define ASK_W 30
   AskWindow=newwin(ASK_H,ASK_W,(LINES-ASK_H)/2,(COLS-ASK_W)/2);
   if(!AskWindow)
      winerror();
   wbkgd(AskWindow,COLOR_PAIR(8)|' ');

#define COL_H (MAX_COLORS+2)
#define COL_W (25)
   ColorWindow=newwin(COL_H,COL_W,(WELL_DEPTH-COL_H)/2,COLS-COL_W-1);
   if(!ColorWindow)
      winerror();

   StatusWindow=newwin(6,25,0,1);
   if(!StatusWindow)
      winerror();
   leaveok(StatusWindow,TRUE);

   make_default_player_name();

   for(;;)
   {
      InitializeGame();
      Play();
      GameOver();

      last_added_score=-1;
      if(check_score(Score,FullLines,ColorsNum))
      {
	 AskName();
	 insert_score(PlayerName,Score,FullLines,ColorsNum,
			StartSpeed,CurrentSpeed);
      }
      ShowHallOfFame();
   }

   TerminateGame();
   return(0);
}
