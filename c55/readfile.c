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

/*
 * C55x readfile. Identical behaviour to ../gdspsim/readfile.c. Kept
 * separate because the c55 tree has its own copy historically.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "readfile.h"
#include "memory.h"
#include "string.h"
#include "symbols.h"

void set_program_start(WordP new_pc);

extern GList *symbolL;
extern GList *symbol_label;

#define FILE_SIZE 100000
GtkWidget *fileWidget = NULL;
struct _file_info *gdsp_file_nfo;

static void file_error(int line, struct _coff_header *header,
                       union _section_header *section_header)
{
  printf("Error reading file in %s:%d\n", __FILE__, line);
  g_free(header);
  g_free(section_header);
}

int readfile(const gchar *file, size_t *amount, char **buffer)
{
  FILE *fp;

  *buffer = g_new(char, FILE_SIZE);

  fp = fopen(file, "rb+");
  if (fp == NULL)
    {
      printf("Can't open file: %s\n", file);
      return -1;
    }

  *amount = fread(*buffer, sizeof(char), FILE_SIZE, fp);
  fclose(fp);
  return 0;
}

void print_mem_list(void);

void open_file(const gchar *filename)
{
  size_t size;
  FILE *fp;
  struct _coff_header *header = NULL;
  union _section_header *section_header = NULL;
  int k;
  size_t read_amount;
  WordP relocate = 0;
  unsigned int optional_header_size;
  struct _optional_header *opt_hdr;
  int binutil;

  fp = fopen(filename, "rb+");
  if (fp == NULL)
    {
      file_error(__LINE__, header, section_header);
      return;
    }

  header = g_new(struct _coff_header, 1);
  read_amount = 22;
  if (read_amount > sizeof(struct _coff_header))
    {
      file_error(__LINE__, header, section_header);
      fclose(fp);
      return;
    }

  size = fread((void *)header, sizeof(char), read_amount, fp);
  if (size != read_amount)
    {
      file_error(__LINE__, header, section_header);
      fclose(fp);
      return;
    }

  printf("version_id = 0x%x\n", CHAR_TO_UINT16(header->version_id));
  printf("flags = 0x%x\n", CHAR_TO_UINT16(header->flags));
  printf("target_id = 0x%x\n", CHAR_TO_UINT16(header->target_id));

  binutil = (CHAR_TO_UINT16(header->version_id) == 0xc1) ? 1 : 0;

  optional_header_size = CHAR_TO_UINT16(header->num_bytes_opt);
  if (optional_header_size != 0)
    {
      if (optional_header_size != sizeof(struct _optional_header))
        {
          file_error(__LINE__, header, section_header);
          fclose(fp);
          return;
        }

      opt_hdr = g_new(struct _optional_header, 1);
      size = fread((void *)opt_hdr, sizeof(char), optional_header_size, fp);
      if (size != optional_header_size)
        {
          file_error(__LINE__, header, section_header);
          fclose(fp);
          return;
        }

      {
        WordP sa = (WordP)CHAR_TO_UINT32(opt_hdr->start_address);
        set_prog_mem_start_end(sa, sa + 0x100);
        set_program_start(sa);
      }
    }
  else
    {
      opt_hdr = NULL;
      relocate = 0x80;
      set_prog_mem_start_end(relocate, relocate + 0x500);
      set_program_start(relocate);
    }

  section_header = g_new(union _section_header,
                         CHAR_TO_UINT16(header->num_sections));
  read_amount = binutil ? sizeof(struct _section_header_binutil)
                        : sizeof(struct _section_header_ti);

  for (k = 0; k < CHAR_TO_UINT16(header->num_sections); k++)
    {
      size = fread((void *)&section_header[k], sizeof(char), read_amount, fp);
      if (size != read_amount)
        {
          file_error(__LINE__, header, section_header);
          fclose(fp);
          return;
        }
    }

  for (k = 0; k < CHAR_TO_UINT16(header->num_sections); k++)
    {
      unsigned long int sec_size;
      size_t read_size;
      WordX *buffer;

      sec_size = section_header[k].ti.s_size;

      if (sec_size > 0)
        {
          if (section_header[k].ti.s_scnptr)
            {
              if (fseek(fp, section_header[k].ti.s_scnptr, SEEK_SET))
                {
                  file_error(__LINE__, header, section_header);
                  fclose(fp);
                  return;
                }

              buffer = g_new(WordX, sec_size);
              read_size = fread((void *)buffer, sizeof(WordX), sec_size, fp);
              if (read_size != sec_size)
                {
                  file_error(__LINE__, header, section_header);
                  fclose(fp);
                  g_free(buffer);
                  return;
                }

              cp_to_mem(buffer,
                        section_header[k].ti.s_paddr + relocate, sec_size,
                        PROGRAM_MEM_TYPE | DATA_MEM_TYPE);
            }
          else
            {
              buffer = g_new0(WordX, sec_size);
              cp_to_mem(buffer,
                        section_header[k].ti.s_paddr + relocate, sec_size,
                        PROGRAM_MEM_TYPE | DATA_MEM_TYPE);
            }
          g_free(buffer);
        }
    }

  if (CHAR_TO_UINT32(header->num_symbols) > 0)
    {
      struct _symbol_element *sym;
      unsigned long int sym_size;
      size_t read_size;
      gchar *str_table = NULL;
      gint32 str_size;
      struct _symL *symL;
      gint32 itemp;
      int num_last = 0;

      sym = g_new(struct _symbol_element,
                  CHAR_TO_UINT32(header->num_symbols));

      if (fseek(fp, CHAR_TO_UINT32(header->symbolP), SEEK_SET))
        {
          file_error(__LINE__, header, section_header);
          fclose(fp);
          return;
        }

      sym_size = sizeof(struct _symbol_element) *
                 CHAR_TO_UINT32(header->num_symbols);
      read_size = fread((void *)sym, sizeof(char), sym_size, fp);
      if (read_size != sym_size)
        {
          file_error(__LINE__, header, section_header);
          fclose(fp);
          return;
        }

      read_size = fread(&str_size, 1, 4, fp);
      if (read_size)
        {
          str_table = g_new(gchar, str_size);
          memset(str_table, 0, 4);
          read_size = fread(str_table + 4, 1, str_size - 4, fp);
          if (read_size != (size_t)(str_size - 4))
            {
              file_error(__LINE__, header, section_header);
              fclose(fp);
              return;
            }
        }

      for (k = 0; k < CHAR_TO_UINT32(header->num_symbols); k++)
        {
          if (num_last > 0)
            {
              num_last--;
              continue;
            }

          symL = g_new(struct _symL, 1);
          symL->type = *((gint16 *)&(sym[k].e_type[0]));

          itemp = *((gint32 *)&(sym[k].norl.name[0]));
          if (itemp == 0 && str_table)
            {
              itemp = *((gint32 *)&(sym[k].norl.name[4]));
              symL->name = g_strdup(str_table + itemp);
            }
          else
            {
              symL->name = g_new(gchar, 9);
              memcpy(symL->name, sym[k].norl.name, 8);
              symL->name[8] = 0;
            }

          symL->value = *((gint32 *)&(sym[k].e_value[0]));
          symL->numaux = sym[k].e_numaux;
          symL->class = sym[k].e_sclass;
          symL->section_num = *((gint16 *)&(sym[k].e_scnum[0]));

          num_last = symL->numaux;

          symbolL = g_list_append(symbolL, symL);

          if ((symL->numaux == 0x0 && symL->class == 0x6 && symL->type == 0x0004 && symL->section_num > 0) ||
              (symL->numaux == 0x0 && symL->class == 0x6 && symL->type == 0x0000 && symL->section_num > 0) ||
              (symL->numaux == 0x0 && symL->class == 0x2 && symL->type == 0x0004 && symL->section_num > 0) ||
              (symL->numaux == 0x0 && symL->class == 0x2 && symL->type == 0x0000 && symL->section_num > 0) ||
              (symL->numaux == 0x1 && symL->class == 0x2 && symL->type == 0x0020 && symL->section_num > 0) ||
              (symL->numaux == 0x1 && symL->class == 0x2 && symL->type == 0x002e && symL->section_num > 0) ||
              (symL->numaux == 0x1 && symL->class == 0x2 && symL->type == 0x0024 && symL->section_num > 0))
            {
              if (strcmp(symL->name, "_c_int00") == 0)
                symbol_label = g_list_prepend(symbol_label, symL);
              else
                symbol_label = g_list_append(symbol_label, symL);
            }
        }
      symbol_label = g_list_first(symbol_label);
    }
  fclose(fp);
  print_mem_list();
}

void load_file_CB(GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog;
  gint res;

  if (fileWidget)
    return;

  dialog = gtk_file_chooser_dialog_new("Open file",
                                       NULL,
                                       GTK_FILE_CHOOSER_ACTION_OPEN,
                                       "_Cancel", GTK_RESPONSE_CANCEL,
                                       "_Open",   GTK_RESPONSE_ACCEPT,
                                       NULL);
  fileWidget = dialog;
  res = gtk_dialog_run(GTK_DIALOG(dialog));
  if (res == GTK_RESPONSE_ACCEPT)
    {
      gchar *filename =
          gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
      if (filename)
        {
          open_file(filename);
          g_free(filename);
        }
    }
  gtk_widget_destroy(dialog);
  fileWidget = NULL;
}
