
#ifndef __GTKBOX_ADD_H__
#define __GTKBOX_ADD_H__
// Adapted from GTK+, gtkbox.c:gtk_box_pack_start()
// Under LGPL, see GTK+ www.gtk.org for copyright info.
void
gtk_box_pack_start_prepend (GtkBox    *box,
			    GtkWidget *child,
			    gboolean   expand,
			    gboolean   fill,
			    guint      padding);
#endif // __GTKBOX_ADD_H__
