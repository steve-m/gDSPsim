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
  size_t amount_read = 0;
  int k;
  size_t read_amount;
  WordA relocate;

  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
  g_print ("reading %s\n", filename);

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

  printf("read in %d header entries   f_opthdr=0x%x\n",size,header->f_opthdr);
  amount_read =+ size;

  printf("Time: %s\n",ctime(&header->f_timdat));

  // Read in section headers
  printf("going to read %d sections of size %d\n",header->f_nscns,sizeof(struct _section_header));

  if ( fseek(fp,read_amount+header->f_opthdr,SEEK_SET) )
    file_error(__LINE__,header,section_header);

  section_header = g_new(struct _section_header, header->f_nscns);
  
  read_amount=48;
  if ( read_amount > sizeof(struct _section_header) )
    file_error(__LINE__,header,section_header);

  for (k=0;k< header->f_nscns ; k++)
    {
      size = fread((void *)&section_header[k],sizeof(char),
	       read_amount,fp);
      if ( size != read_amount )
	file_error(__LINE__,header,section_header);

      amount_read =+ size;
    }

  // Determine if object file or program file by if the
  // first section is called .vers
  if ( strncmp(section_header[0].s_name,".vers",6) == 0 )
    {
      relocate=0;
    }
  else
    {
      relocate=0x80;
      printf("Object File going to relocate to 0x80\n");
    }

 

  for (k=0;k<header->f_nscns;k++)
    {
      unsigned long int size;
      size_t read_size;
      Word *buffer;


      size = section_header[k].s_size;

      printf("Section name %s padd=0x%lx size=0x%lx flag=0x%lx raw=0x%lx\n",section_header[k].s_name,section_header[k].s_paddr,section_header[k].s_size,section_header[k].s_flags,section_header[k].s_scnptr);

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

	      if ( strncmp(section_header[k].s_name,".text",6) == 0 )
		{
		  // Set the default view ranges
		  set_prog_mem_start_end(section_header[k].s_paddr+relocate,
					 section_header[k].s_paddr+relocate+size-1);
		}

	      printf("-----k= %d\n",k);
  print_mem_list();
	    }
	  else
	    {
	      // Undefined data
	      buffer=g_new(Word,size);
	      cp_to_mem( buffer, section_header[k].s_paddr+relocate, size, 
		     PROGRAM_MEM_TYPE | DATA_MEM_TYPE);

	      printf("-----k= %d\n",k);
  print_mem_list();
	    }
	  g_free(buffer);
	}
    }

  // Debug
	      printf("-----\n");
  print_mem_list();

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

