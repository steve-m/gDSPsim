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

#include <stdio.h>
#include "readfile.h"
#include "process_coff.h"
#include <time.h>
#include "memory.h"
#include "string.h"

void set_PC(WordA new_pc);


struct _symL
{
  gchar *name;
  gint32 value;
  gint16 type;
  unsigned char numaux;
  unsigned char class;
  gint16 section_num;
};

// List of the whole symbol table
GList *symbolL=NULL;
// List of the symbol table that are labels, sorted on value
GList *symbol_label=NULL;

WordA start_address=0x0;

#define FILE_SIZE 100000
GtkWidget *fileWidget=NULL;
struct _file_info *gdsp_file_nfo;

static void destroy( GtkWidget *widget, gpointer   data )
{
  fileWidget=NULL;
  gtk_widget_destroy(widget);
}


int readfile(const gchar *file,size_t *amount, char **buffer)
{
  FILE *fp;
  
  *buffer = g_new(char,FILE_SIZE);

  fp=fopen(file,"rb+");
  if (fp==NULL)
    {
      printf("Can't open file: %s\n",file);
      return -1;
    }

  *amount = fread(*buffer,sizeof(char),FILE_SIZE,fp);

  if ( *amount == FILE_SIZE )
    {
      printf("Warning only read in %d bytes. Need to change FILE_SIZE to a larger value inside file %s\n",FILE_SIZE,__FILE__);
    }
  return 0;
}

void file_ok_sel( GtkWidget        *w,
		  GtkFileSelection *fs )
{
  const gchar *filename;
  int error;
  char *buffer;
  size_t size;


  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
  g_print ("reading %s\n", filename);

  error = readfile(filename,&size,&buffer);

  if ( !error )
    {
      gdsp_file_nfo = process_coff(buffer,size);
      gdsp_file_nfo->filename = g_strdup(filename);

      gtk_widget_destroy(fileWidget);
      fileWidget=NULL;
    }
}

static void file_error(int line, struct _coff_header *header, 
		       struct _section_header *section_header)
{
  
  printf("Error reading file in  %s:%d\n",__FILE__,line);
  if (header)
    g_free(header);
  if (section_header)
    g_free(section_header);

  gtk_widget_destroy(fileWidget);
  fileWidget=NULL;
  return;
}

void print_mem_list(void);

void file_ok_sel2( GtkWidget        *w,
		  GtkFileSelection *fs )
{
  const gchar *filename;
  size_t size;
  FILE *fp;
  struct _coff_header *header=NULL;
  struct _section_header *section_header=NULL;
  int k;
  size_t read_amount;
  WordA relocate=0; // Used to load object files
  unsigned int optional_header_size;
  struct _optional_header *opt_hdr;

  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));

  // Open file
  fp=fopen(filename,"rb+");
  if (fp==NULL)
    file_error(__LINE__,header,section_header);

  // Read header
  header = g_new(struct _coff_header,1);
  read_amount=22;
  if ( read_amount > sizeof(struct _coff_header) )
    file_error(__LINE__,header,section_header);

  size = fread((void *)header,sizeof(char),read_amount,fp);
  if ( size != read_amount )
    file_error(__LINE__,header,section_header);

  //printf("Time: %s\n",ctime((time_t *)&CHAR_TO_UINT32(header->time_stamp)));


  // Read in Optional header if it exists
  optional_header_size = CHAR_TO_UINT16(header->num_bytes_opt);
  if (optional_header_size != 0 )
    {
      if (optional_header_size != sizeof(struct _optional_header)) 
	file_error(__LINE__,header,section_header);
      
      opt_hdr = g_new(struct _optional_header,1);

      size = fread((void *)opt_hdr,sizeof(char),optional_header_size,fp);
      if ( size != optional_header_size)
	file_error(__LINE__,header,section_header);

      // Set the default view ranges
      {
	WordA sa;

	sa = (WordA)CHAR_TO_UINT32(opt_hdr->start_address);
	set_prog_mem_start_end(sa,sa+0x30);
	set_PC(sa);
      }
    }
  else
    {
      opt_hdr=NULL;
      relocate=0x80; // To put .obj files in a reasonable location
      set_prog_mem_start_end(relocate,relocate+0x20 );
      set_PC(relocate);
    }

  section_header = g_new(struct _section_header, CHAR_TO_UINT16(header->num_sections));
  
  read_amount=48;
  if ( read_amount > sizeof(struct _section_header) )
    file_error(__LINE__,header,section_header);

  for (k=0;k< CHAR_TO_UINT16(header->num_sections) ; k++)
    {
      size = fread((void *)&section_header[k],sizeof(char),
	       read_amount,fp);
      if ( size != read_amount )
	file_error(__LINE__,header,section_header);
    }

  for (k=0;k<CHAR_TO_UINT16(header->num_sections);k++)
    {
      unsigned long int size;
      size_t read_size;
      Word *buffer;


      size = section_header[k].s_size;

      if ( size > 0 )
	{
		
	  // There is data to move
	  if ( section_header[k].s_scnptr )
	    {
	      // Set position in file to start reading
	      if ( fseek(fp,section_header[k].s_scnptr,SEEK_SET) )
		file_error(__LINE__,header,section_header);

	      // Read data from file
	      buffer=g_new(Word,size);
	      read_size = fread((void *)buffer,sizeof(Word), size,fp);
	      if ( read_size != size )
		file_error(__LINE__,header,section_header);

	      // Put data into internal representation
	      cp_to_mem( buffer, section_header[k].s_paddr+relocate, size, 
		     PROGRAM_MEM_TYPE | DATA_MEM_TYPE);


	      //  print_mem_list();
	    }
	  else
	    {
	      // Undefined data
	      buffer=g_new(Word,size);
	      cp_to_mem( buffer, section_header[k].s_paddr+relocate, size, 
		     PROGRAM_MEM_TYPE | DATA_MEM_TYPE);

	      //  print_mem_list();
	    }
	  g_free(buffer);
	}
    }

  if ( CHAR_TO_UINT32(header->num_symbols) > 0 )
    {
      struct _symbol_element *sym;
      unsigned long int size;
      size_t read_size;
      gchar *str_table;
      gint32 str_size;
      struct _symL *symL;
      gint32 itemp;
      int num_last=0;
      
      // There is a symbol table
      printf("symbol=0x%x\n",CHAR_TO_UINT32(header->symbolP));
      
      sym = g_new(struct _symbol_element, CHAR_TO_UINT32(header->num_symbols) );
      
      if ( fseek(fp,CHAR_TO_UINT32(header->symbolP),SEEK_SET) )
	file_error(__LINE__,header,section_header);
      
      size = sizeof(struct _symbol_element) * CHAR_TO_UINT32(header->num_symbols);
      read_size = fread((void *)sym,sizeof(char), size,fp);
      if ( read_size != size )
	{
	  file_error(__LINE__,header,section_header);
	}
      
      // Read string table size
      fread(&str_size, 1, 4, fp);
      
      str_table = g_new(gchar,str_size);
      memset(str_table, 0, 4);
      
      read_size = fread(str_table+4, 1, str_size-4, fp);
      if ( read_size != str_size-4 )
	file_error(__LINE__,header,section_header);
      
      // Move symbol table into linked list
      for (k=0;k<CHAR_TO_UINT32(header->num_symbols);k++ )
	{

	  if ( num_last > 0 )
	    {
	      num_last--;
	    }
	  else
	    {
	      symL = g_new(struct _symL,1);
	      
	      symL->type = *((gint16 *)&(sym[k].e_type[0]));
	      
	      itemp = *((gint32 *)&(sym[k].norl.name[0]));
	      if ( itemp == 0 ) 
		{
		  // label name is in string table
		  // get offset in string table
		  itemp = *((gint32 *)&(sym[k].norl.name[4]));
		  symL->name = g_strdup(str_table+itemp);
		}
	      else
		{
		  symL->name = g_new(gchar,9);
		  
		  memcpy(symL->name,sym[k].norl.name,8);
		  symL->name[8]=0;
		}

	      symL->value = *((gint32 *)&(sym[k].e_value[0]));
	      symL->numaux = sym[k].e_numaux;
	      symL->class = sym[k].e_sclass;
	      symL->section_num = *((gint16 *)&(sym[k].e_scnum[0]));
	      
	  
	      // Look for starting address
	      if ( strcmp(symL->name,"_c_int00") == 0 )
		{
		  start_address = symL->value;
		  printf("found start 0x%x\n",start_address);
		}

	      num_last = symL->numaux;
	      
	      symbolL = g_list_append(symbolL,symL);
	      
	      // Extract labels
	      if ( (symL->numaux==0x0 && symL->class==0x6 && symL->type==0x0004 && symL->section_num > 0 ) ||
		   (symL->numaux==0x0 && symL->class==0x2 && symL->type==0x0004 && symL->section_num > 0 ) )
		{
		  symbol_label = g_list_append(symbol_label,symL);
		}
	    }
	}

    }
  // Debug
  //  print_mem_list();

  gtk_widget_destroy(fileWidget);
  fileWidget=NULL;
}


void load_file_CB( GtkWidget *widget,  gpointer   data )
{

  if ( fileWidget )
    return;

  fileWidget = gtk_file_selection_new("File Menu");

  gtk_signal_connect (GTK_OBJECT (fileWidget), "destroy",
		      (GtkSignalFunc) destroy, &fileWidget);
  /* Connect the ok_button to file_ok_sel function */
  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fileWidget)->ok_button),
		      "clicked", (GtkSignalFunc) file_ok_sel2, fileWidget );
  
  /* Connect the cancel_button to destroy the widget */
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
					 (fileWidget)->cancel_button),
			     "clicked", (GtkSignalFunc) destroy,
			     GTK_OBJECT (fileWidget));
  
  /* Lets set the filename, as if this were a save dialog, and we are giving
     a default filename */
  // gtk_file_selection_set_filename (GTK_FILE_SELECTION(fileWidget), 
  //				   "penguin.png");
  
  gtk_widget_show(fileWidget);
}

