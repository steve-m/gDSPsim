/*
 * gDSPsim - GNU Digital Signal Processor Simulator
 *
 * Copyright (C) 2001, Kerry Keal, kerry@industrialmusic.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

//#define GTK_ENABLE_BROKEN

#include <stdio.h>
#include "decode_window.h"
#include "find_opcode.h"
#include "memory.h"
#include "entryCB.h"
#include "pipeline.h"

#if 0
static gchar *pipe_tag_color[6]=
{
  "red", 
  "#808080", 
  "#a0a0a0",
  "#b0b0b0",
  "#d0d0d0",
  "blue"
};
#endif

static GdkColor pipe_color[6]=
{
  {0,0x8000,0x0,0x0},
  {0,0x4000,0x4000,0x4000},
  {0,0x6000,0x6000,0x6000},
  {0,0x8000,0x8000,0x8000},
  {0,0xa000,0xa000,0xa000},
  {0,0xc000,0xc000,0xc000},
};
static GdkColormap *colormap;

//#define USE_PIXMAP



#define COLUMN_OPCODE 3
#define COLUMN_ADDRESS 1
#define COLUMN_MACHCODE 2
#define COLUMN_LABEL 1
#define COLUMN_BREAK 0

//void entry_hexCB( GtkWidget *widget,  GtkWidget *entryCB_nfo );

// This C file is the interface to a popup window that displays decoded assembly

extern GdkFont *gdsp_Decode_Font;
GtkWidget *decodeW;
GdkPixmap *pixmap;
GtkWidget *clist;
GdkBitmap *mask;

/* needed prototypes */
/* Called when step button pressed */
void decode_step(void);

/* Returns an array of strings that should be freed by the caller */
gchar **get_decoded_text(unsigned long int start_index, unsigned long int end_index);
static void insert_text(WordA start_mem, WordA end_mem, GArray *word2line);

struct _decode_window_nfo *dwn=NULL;

struct _decode_window_nfo
{
  GtkWidget *decodeW;
  WordA start;
  WordA end;
  GArray *word2line;
  //  GtkListStore *model;
};


//GtkTextIter iter_start,iter_end;
//GtkTextBuffer *buffer;


static void change_end_address(GtkWidget *entry, guint64 address, 
				 gpointer data)
{
  struct _decode_window_nfo *dwn;
  WordA mem;

  dwn = data;
  mem = address;

  // Force reasonable values
  if ( mem < dwn->start )
    {
      dwn->start=mem;
    }
  if ( mem > (dwn->start + 0x400) )
    {
      dwn->start=mem-0x400;
    }
  dwn->end = address;

#if 0
  g_snprintf(temp_str,10,"0x%x",dwn->start);
  gtk_entry_set_text (GTK_ENTRY(dwn->entryTop),temp_str);
  g_snprintf(temp_str,10,"0x%x",dwn->end);
  gtk_entry_set_text (GTK_ENTRY(dwn->entryBottom),temp_str);
#endif

  while ( dwn->word2line->len > 0 )
    g_array_remove_index_fast(dwn->word2line,0);

  insert_text(dwn->start,dwn->end,dwn->word2line);

  highlight_pipeline(0,0);
}


static void change_start_address(GtkWidget *entry, guint64 address, 
				 gpointer data)
{
  struct _decode_window_nfo *dwn;
  WordA mem;

  dwn = data;
  mem = address;

  // Force reasonable values
  if ( mem > dwn->end )
    {
      dwn->end=mem;
    }
  if ( mem < (dwn->end - 0x400) )
    {
      dwn->end=mem+0x400;
    }
  dwn->start = address;

#if 0
  g_snprintf(temp_str,10,"0x%x",dwn->start);
  gtk_entry_set_text (GTK_ENTRY(dwn->entryTop),temp_str);
  g_snprintf(temp_str,10,"0x%x",dwn->end);
  gtk_entry_set_text (GTK_ENTRY(dwn->entryBottom),temp_str);
#endif

  while ( dwn->word2line->len > 0 )
    g_array_remove_index_fast(dwn->word2line,0);

  insert_text(dwn->start,dwn->end,dwn->word2line);
  highlight_pipeline(0,0);
}


static void click_CB( GtkWidget *W, gint row, gint column, GdkEventButton *event, gpointer data )
{

  if ( column == 0 )
    {
      WordA addr;
      addr = (WordA)gtk_clist_get_row_data(GTK_CLIST(clist),row);
      // Only allow breakpoints on instructions and not labels
      if ( addr )
	{
	  printf("Word 0x%x\n",addr);
	  if ( toggle_breakpoint(addr) )
	    gtk_clist_set_pixmap(GTK_CLIST(clist), row, column, pixmap, mask);
	  else
	    gtk_clist_set_text(GTK_CLIST(clist), row, column, NULL);
	}
    }
}

/*
 * This will be called by the main routine to create a popup window.
 * At the top there should be 2 spin buttons that controls the range
 * of memory that the scroll bars can access (and labeled as such). One for
 * the top index and one for the bottom index. For 
 * memory efficieny reasons, the user may only want part of the memory visiable.
 * There should also be a step button at the top that will give a call to 
 * decode_step when pressed.
 */

GtkWidget *create_decode_window()
{
  GtkWidget *vbox,*hbox,*scrolledW;
  GPtrArray *textA;
  GtkWidget *entryTop,*entryBottom;
  WordA start_mem,end_mem;
  gchar temp_str[10];
  struct _entryCB_nfo *entry_start_nfo;
  extern GtkAccelGroup *gDSP_keyboard_accel;

  if ( dwn )
    return NULL;

  dwn = g_new(struct _decode_window_nfo,1);

  decodeW = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  dwn->decodeW = decodeW;

  gtk_widget_set_name (decodeW, "Disassembly");
  gtk_widget_set_usize (GTK_WIDGET(decodeW), 300, 200);
  gtk_window_set_title (GTK_WINDOW (decodeW), "Disassebly");
  gtk_container_set_border_width (GTK_CONTAINER (decodeW), 0);

  // Attach keyboard accelerations from main window
  gtk_window_add_accel_group (GTK_WINDOW (decodeW), gDSP_keyboard_accel);
  

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (decodeW), vbox);
  gtk_widget_show (vbox);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 6);
  gtk_widget_show (hbox);

  entryTop = gtk_entry_new();
  gtk_box_pack_start (GTK_BOX (hbox), entryTop, TRUE, TRUE, 0);
  gtk_entry_set_max_length(GTK_ENTRY(entryTop),10);
  // start memory

  get_prog_mem_start_end(&start_mem,&end_mem);

  g_snprintf(temp_str,10,"0x%x",start_mem);

  gtk_entry_set_text (GTK_ENTRY(entryTop),temp_str);


  entry_start_nfo = g_new(struct _entryCB_nfo,1);
  entry_start_nfo->entry = entryTop;
  entry_start_nfo->bits = BITS_PER_ADDRESS;
  entry_start_nfo->CB_func = change_start_address;
  entry_start_nfo->data = dwn;


  gtk_signal_connect(GTK_OBJECT(entryTop), "activate",
		     GTK_SIGNAL_FUNC(entry_hexCB),
		     entry_start_nfo);

  gtk_widget_show (entryTop);


  entryBottom = gtk_entry_new();
  gtk_box_pack_start (GTK_BOX (hbox), entryBottom, TRUE, TRUE, 0);
  gtk_entry_set_max_length(GTK_ENTRY(entryTop),10);

  g_snprintf(temp_str,10,"0x%x",end_mem);
  gtk_entry_set_text (GTK_ENTRY(entryBottom),temp_str);

  entry_start_nfo = g_new(struct _entryCB_nfo,1);
  entry_start_nfo->entry = entryBottom;
  entry_start_nfo->bits = BITS_PER_ADDRESS;
  entry_start_nfo->CB_func = change_end_address;
  entry_start_nfo->data = dwn;


  gtk_signal_connect(GTK_OBJECT(entryBottom), "activate",
		     GTK_SIGNAL_FUNC(entry_hexCB),
		     entry_start_nfo);


  gtk_widget_show (entryBottom);


  scrolledW = gtk_scrolled_window_new(NULL, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), scrolledW, TRUE, TRUE, 0);
  gtk_widget_show(scrolledW);

  textA = g_ptr_array_new();

  /* textA is an array of strings that represent the decoded opcodes */
  dwn->start = start_mem;
  dwn->end = end_mem;
#ifdef GTK2
  dwn->word2line = g_array_sized_new(FALSE, FALSE, sizeof(int),end_mem-start_mem+1);
#else
  dwn->word2line = g_array_new(FALSE, FALSE, sizeof(int));
  //g_array_set_size(dwn->word2line,end_mem-start_mem+1);
#endif


  {
    gchar *titles[]={"break","address","mach code","opcode"};

    clist = gtk_clist_new_with_titles(4,titles);
  }

  gtk_clist_set_selection_mode(GTK_CLIST(clist), GTK_SELECTION_SINGLE);
  
  insert_text(start_mem,end_mem,dwn->word2line);

  gtk_signal_connect( GTK_OBJECT(clist),"select_row", 
		      GTK_SIGNAL_FUNC( click_CB), NULL );
  
  gtk_widget_realize(decodeW);
  //pixmap = gdk_pixmap_create_from_xpm(decodeW->window,&mask,
  //				     &style->bg[GTK_STATE_NORMAL],"check.xpm");
  pixmap = gdk_pixmap_create_from_xpm(decodeW->window,&mask,
				      NULL,"stop.xpm");

  {
    int k;
    // set colors
    colormap = gdk_colormap_get_system ();
    for (k=0;k<6;k++)
      {
	gdk_color_alloc(colormap,&pipe_color[k]);
      }
  }

  gtk_container_add(GTK_CONTAINER(scrolledW),clist);


  gtk_widget_show(clist);


  gtk_widget_show(decodeW);
  return decodeW;

}


void highlight_pipeline(WordA prefetch, int advance)
{
  static int pipe[6]={-2, -2, -2, -2, -2, -2};
  static WordA pipeW[6];

  static int cntr=0;
  int lineNo,k;
  int last_line=-1;

  // TODO need to find out if program memory has logged a change
  // if so, redo window.

  if ( dwn==NULL)
    {
      // no decode window yet
      
      if ( advance )
	{
	  cntr++;
	  if ( cntr > 5 )
	    cntr=0;
	  pipeW[cntr]=prefetch;
	}
      return;
    }

  if ( advance )
    {
      cntr++;
      if ( cntr > 5 )
	cntr=0;
    }
  else
    {
      // Redo the lines
      for (k=0;k<6;k++)
	{
	  if (pipe[k]>-2)
	    {
	      if ( ( pipeW[k] >= dwn->start ) && ( pipeW[k] <= dwn->end ) )
		{
		  lineNo = g_array_index(dwn->word2line,int,pipeW[k]-dwn->start);
		  pipe[k]=lineNo;
		}
	      else
		{
		  pipe[k]=-1;
		}
	    }
	}
    }

  gtk_clist_freeze(GTK_CLIST(clist));

  if ( advance )
    pipeW[cntr]=prefetch;

  if ( advance && pipe[cntr]>0 )
    {
      // cntr points to the previous execute line, which should
      // be unhighlited
      gtk_clist_set_background(GTK_CLIST(clist),pipe[cntr],NULL);
    }

  if ( advance )
    {
      if ( ( prefetch >= dwn->start ) && ( prefetch <= dwn->end ) )
	{	 
	  lineNo = g_array_index(dwn->word2line,int,prefetch-dwn->start);
	  pipe[cntr]=lineNo;
	  printf("line=%d prefetch=0x%x 0x%x\n",lineNo,prefetch,dwn->start);
	}
      else
	{
	  pipe[cntr]=-1;
	}
    }

  for (k=0;k<6;k++)
    {
      if ( pipe[cntr]>0 && (pipe[cntr] != last_line ) )
	{
	  gtk_clist_set_background(GTK_CLIST(clist),pipe[cntr],&pipe_color[k]);
	}
      // Make sure paint the leading color
      last_line=pipe[cntr];

      cntr--;
      if ( cntr < 0 )
	cntr=5;
    }
#if 0
  cntr++;
  if ( cntr > 5 )
    cntr=0;
#endif

  gtk_clist_thaw(GTK_CLIST(clist));

}
/* 
 * Used to set min and max allowable scrowable range
 */
void set_decode_index_max_range(unsigned long int lower_index, unsigned long int upper_index)
{
}


static void insert_text(WordA start_mem, WordA end_mem, GArray *word2line)
{
  GPtrArray *textA;
  int k;
  int row;
  struct _decode_opcode *op;
  gchar *data[4];
  int offset=0;
  int width;

  gtk_clist_freeze(GTK_CLIST(clist));

  // remove old items
  gtk_clist_clear(GTK_CLIST(clist));

  // Poplate model with data
  textA = g_ptr_array_new();
  
  decoded_opcodes(textA, start_mem,end_mem,word2line);
  
  for (k=0;k<textA->len;k++)
    {
      while ( g_array_index(dwn->word2line,int,offset) < k)
	offset++;

      op = g_ptr_array_index(textA,k);

      if ( op->machine_code )
	{
	  // Not a label
	  data[0]=NULL;
	  data[1]=op->address;
	  data[2]=op->machine_code;
	  data[3]=op->opcode_text;

	  row = gtk_clist_append(GTK_CLIST(clist),data);

	  // g_warning(row!=k,"warning");
	  gtk_clist_set_row_data(GTK_CLIST(clist),row,(gpointer)(offset+dwn->start));

	  g_free(op->machine_code);
	}
      else
	{
	  // label
	  // Not a label
	  data[0]=NULL;
	  data[1]=op->opcode_text;
	  data[2]=NULL;
	  data[3]=NULL;

	  row = gtk_clist_append(GTK_CLIST(clist),data);
	  gtk_clist_set_row_data(GTK_CLIST(clist),row,NULL);

	  //	  g_warning(row!=k,"warning");

	}
      g_free(op->opcode_text);
      g_free(op->address);
      g_free(op);
    }

  width = gtk_clist_optimal_column_width(GTK_CLIST(clist),1);
  gtk_clist_set_column_width(GTK_CLIST(clist),1,width);

  width = gtk_clist_optimal_column_width(GTK_CLIST(clist),2);
  gtk_clist_set_column_width(GTK_CLIST(clist),2,width);

  width = gtk_clist_optimal_column_width(GTK_CLIST(clist),3);
  gtk_clist_set_column_width(GTK_CLIST(clist),3,width);

  // set size for pixmaps
  // gtk_clist_set_column_width(GTK_CLIST(clist),0,16);
  gtk_clist_set_row_height(GTK_CLIST(clist),16);

  gtk_clist_thaw(GTK_CLIST(clist));

  g_ptr_array_free(textA,FALSE);  
}

WordA line2word(GArray *word2line, int line, WordA start)
{
  WordA guess,guess_hi,guess_low;
  int line_guess;

  guess_hi = word2line->len-1;
  guess_low = 0;
  guess = guess_hi>>1;

  while ( 1 )
    {
      line_guess = g_array_index(word2line,int,guess);

      if ( line_guess == line || guess == guess_hi)
	{
	  // Make sure it's the first word of the line
	  while ( guess > 0 )
	    {
	      guess--;
	      line_guess = g_array_index(word2line,int,guess);
	      if ( line_guess < line )
		{
		  guess++;
		  return guess+start;
		}
	    }
	  return start;
	      
	}
      else if ( line_guess > line )
	{
	  guess_hi = guess;
	  guess = guess_low + (( guess_hi - guess_low + 1 )>>1);
	}
      else // ( line_guess < line )
	{
	  guess_low = guess;
	  guess = guess_low + (( guess_hi - guess_low + 1 )>>1);
	}
    }
}
