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

#define GTK_ENABLE_BROKEN

#include <stdio.h>
#include "decode_window.h"
#include "process_coff.h"
#include "find_opcode.h"
#include "memory.h"
#include "entryCB.h"

static gchar *pipe_tag[6]=
{
  "prefetch_tag", 
  "fetch_tag",
  "read_stg1_tag", 
  "read_stg2_tag",
  "decode_tag",
  "execute_tag"
};

static gchar *pipe_tag_color[6]=
{
  "red", 
  "#808080", 
  "#a0a0a0",
  "#b0b0b0",
  "#d0d0d0",
  "blue"
};


//void entry_hexCB( GtkWidget *widget,  GtkWidget *entryCB_nfo );

// This C file is the interface to a popup window that displays decoded assembly

extern GdkFont *gdsp_Decode_Font;
GtkWidget *textW,*decodeW;

/* needed prototypes */
/* Called when step button pressed */
void decode_step(void);

/* Returns an array of strings that should be freed by the caller */
gchar **get_decoded_text(unsigned long int start_index, unsigned long int end_index);
static void insert_text(WordA start_mem, WordA end_mem, GArray *word2line, GtkTextBuffer *buffer);

struct _decode_window_nfo *dwn=NULL;

struct _decode_window_nfo
{
  GtkWidget *decodeW;
  WordA start;
  WordA end;
  GArray *word2line;
  GtkTextBuffer *buffer;
};


GtkTextIter iter_start,iter_end;
GtkTextBuffer *buffer;


static void change_end_address(GtkWidget *entry, guint64 address, 
				 gpointer data)
{
  struct _decode_window_nfo *dwn;
  WordA mem;
  GtkTextIter iter_start,iter_end;


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

  gtk_text_buffer_get_bounds(dwn->buffer,&iter_start,&iter_end);
  gtk_text_buffer_delete(dwn->buffer,&iter_start,&iter_end);

  insert_text(dwn->start,dwn->end,dwn->word2line,dwn->buffer);
  
}


static void change_start_address(GtkWidget *entry, guint64 address, 
				 gpointer data)
{
  struct _decode_window_nfo *dwn;
  WordA mem;
  GtkTextIter iter_start,iter_end;

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

  gtk_text_buffer_get_bounds(dwn->buffer,&iter_start,&iter_end);
  gtk_text_buffer_delete(dwn->buffer,&iter_start,&iter_end);

  insert_text(dwn->start,dwn->end,dwn->word2line,dwn->buffer);
  
}

static void text_CB( GtkTextView *W, GdkEventButton *event )
{
#if 0
  int line_num=0;
  GList *line;
  
  printf("pos_x=%d pos_y=%d button=%d\n",W->cursor_pos_x,W->cursor_pos_y,W->button);
  printf("current_line=0x%x\n",(int)W->current_line); 
  
  line = W->current_line;
  
  while (line->prev)
    {
      line_num++;
      line=line->prev;
    }
  printf("line number=%d\n",line_num);
  
  printf("gtk_text_get_point=%d\n",gtk_text_get_point(W));  
#endif
  printf("text_CB=%d widget=0x%x \n",(int)event,(int)W);
  
  //	    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, line, -1,
  //					      "red_background", NULL);
  //gtk_text_buffer_apply_tag       (buffer,
  //			   "red_background",
  //			   &iter_start,
  //			   &iter_end);
  printf("text mark insert = 0x%x\n",(int)gtk_text_buffer_get_insert (buffer) );
  // printf("text mark _____  = 0x%x\n",(int)  gtk_text_buffer_get_mark (buffer,"insert"));
  {
    gint x,y;
    gint win_x,win_y;
    GtkTextIter iter;
    GtkTextIter start_iter;
    GtkTextIter end_iter;
    GdkModifierType mask=GDK_BUTTON1_MASK;
    GtkTextWindowType win_type;
    int ln;
    
    gdk_window_get_pointer(event->window,&win_x,&win_y,&mask );
    printf("win x,y %d %d\n",win_x,win_y);
    
    // Gtk-CRITICAL **: file gtktextview.c: line 5373 (gtk_text_view_get_window_type): assertion `GDK_IS_WINDOW (text_view)' failed
    
    win_type =  gtk_text_view_get_window_type(GTK_TEXT_VIEW(textW),event->window);
    
    printf("win_type=%d\n",win_type);
    
    gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW(textW), GTK_TEXT_WINDOW_WIDGET ,win_x,win_y,&x,&y);
    printf("buff x,y %d %d\n",x,y);
    
    gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(textW),&iter,x,y);
    //    printf("itet =0x%x\n",(int)iter);
    
    ln=gtk_text_iter_get_line (&iter);
    printf("line=%d\n",ln);
    //    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, line, -1,
    //				      "red_background", NULL);
    gtk_text_buffer_get_iter_at_line(buffer,&start_iter,ln);
    
    printf("computed iter at start of line\n");
    gtk_text_buffer_apply_tag_by_name(buffer,"red_background",&start_iter,&end_iter);
    
    
  }
  
  return;
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
  int k;
  gchar temp_str[10];
  struct _entryCB_nfo *entry_start_nfo;

  if ( dwn )
    return NULL;

  dwn = g_new(struct _decode_window_nfo,1);

  decodeW = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  dwn->decodeW = decodeW;

  gtk_widget_set_name (decodeW, "Disassembly");
  gtk_widget_set_usize (GTK_WIDGET(decodeW), 300, 200);
  gtk_window_set_title (GTK_WINDOW (decodeW), "Disassebly");
  gtk_container_set_border_width (GTK_CONTAINER (decodeW), 0);

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

  textW = gtk_text_view_new ();

  gtk_container_add(GTK_CONTAINER(scrolledW),textW);

  //gtk_text_set_line_wrap(GTK_TEXT(textW),0);
  gtk_signal_connect( GTK_OBJECT(textW),"button-press-event", 
		      GTK_SIGNAL_FUNC( text_CB), NULL );

  //  gtk_signal_connect( GTK_OBJECT(textW),"mark-set", 
  //	      GTK_SIGNAL_FUNC( text_CB), &nine /*parameter*/ );

  gtk_widget_show (textW);


  // gtk_widget_realize (textW);

  textA = g_ptr_array_new();

  /* textA is an array of strings that represent the decoded opcodes */
  dwn->start = start_mem;
  dwn->end = end_mem;
  dwn->word2line = g_array_sized_new(FALSE, FALSE, sizeof(int),end_mem-start_mem+1);

#if 0
  // This doesn't work.
  if ( !gdsp_Decode_Font )
    {
      printf("Setting font\n");
      gdsp_Decode_Font = gdk_font_load ("-misc-fixed-medium-r-*-*-*-140-*-*-*-*-*-*");
    }
#endif

  {
    GtkTextIter iter;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (textW));
    dwn->buffer = buffer;
 
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);

    for (k=0;k<6;k++)
      {
	gtk_text_buffer_create_tag (buffer, pipe_tag[k],
				    "background", pipe_tag_color[k], NULL);
      }

    insert_text(start_mem,end_mem,dwn->word2line,buffer);

  }



  gtk_widget_show(decodeW);
  return decodeW;

}


/* 
 * If the user is simulating code and stepping through it, then the main
 * program will call this function to highlight the correct line
 */
void highlight_decode_index(unsigned long int index)
{
}

void highlight_pipeline(WordA prefetch)
{
  static int pipe[6]={-1, -1, -1, -1, -1, -1};
  static int cntr=0;
  int lineNo,k;
  GtkTextIter iter_start,iter_end;
  
  if ( pipe[cntr]>0 )
    {
      gtk_text_buffer_get_iter_at_line(dwn->buffer,&iter_start,pipe[cntr]);
      iter_end=iter_start;
      gtk_text_iter_forward_to_line_end ( &iter_end);
      //  gtk_text_buffer_apply_tag(dwn->buffer,NULL,&iter_start,&iter_end);
      gtk_text_buffer_remove_tag_by_name(dwn->buffer,pipe_tag[5],&iter_start,&iter_end);
      gtk_text_buffer_remove_tag_by_name(dwn->buffer,pipe_tag[4],&iter_start,&iter_end);
      gtk_text_buffer_remove_tag_by_name(dwn->buffer,pipe_tag[3],&iter_start,&iter_end);
      gtk_text_buffer_remove_tag_by_name(dwn->buffer,pipe_tag[2],&iter_start,&iter_end);
      gtk_text_buffer_remove_tag_by_name(dwn->buffer,pipe_tag[1],&iter_start,&iter_end);
      gtk_text_buffer_remove_tag_by_name(dwn->buffer,pipe_tag[0],&iter_start,&iter_end);
      
      // Above pipe_tag[5] alone didn't work. below might be better.
      //  gtk_text_buffer_remove_all_tags(dwn->buffer,&iter_start,&iter_end);
      
    }

  // cntr points to the previous execute line, which should
  // be unhighlited
  if ( ( prefetch >= dwn->start ) && ( prefetch <= dwn->end ) )
    {	 
      lineNo = g_array_index(dwn->word2line,int,prefetch-dwn->start);
      pipe[cntr]=lineNo;
    }
  else
    {
      pipe[cntr]=-1;
    }
  
  for (k=0;k<6;k++)
    {
      int last_line=-1;
      if ( pipe[cntr]>0 && (pipe[cntr] != last_line ) )
	{
	  gtk_text_buffer_get_iter_at_line(dwn->buffer,&iter_start,pipe[cntr]);
	  iter_end=iter_start;
	  gtk_text_iter_forward_to_line_end ( &iter_end);
	  gtk_text_buffer_apply_tag_by_name(dwn->buffer,pipe_tag[k],&iter_start,&iter_end);
	}
      // Make sure paint the leading color
      last_line=pipe[cntr];

      cntr++;
      if ( cntr >= 6 )
	cntr=0;
    }
  cntr--;
  if ( cntr < 0 )
    cntr=5;
}
/* 
 * Used to set min and max allowable scrowable range
 */
void set_decode_index_max_range(unsigned long int lower_index, unsigned long int upper_index)
{
}


static void insert_text(WordA start_mem, WordA end_mem, GArray *word2line, GtkTextBuffer *buffer_)
{
  GPtrArray *textA;
  int k;
  gchar *line;
  // GtkTextBuffer *buffer_;
  GtkTextIter iter;
  
  textA = g_ptr_array_new();
  gtk_text_buffer_get_iter_at_offset (buffer_, &iter, 0);
  
  decoded_opcodes(textA, start_mem,end_mem,word2line);
  
  for (k=0;k<textA->len;k++)
    {
      
      line = g_ptr_array_index(textA,k);
      
      gtk_text_buffer_insert (buffer_, &iter, line, -1);
      
      gtk_text_buffer_insert (buffer_, &iter, "\n", -1);
      
    }
  g_ptr_array_free(textA,TRUE);
  
}
