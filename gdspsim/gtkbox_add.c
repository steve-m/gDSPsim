#include <gtk/gtkbox.h>

// Adapted from GTK+, gtkbox.c:gtk_box_pack_start()
// Under LGPL, see GTK+ www.gtk.org for copyright info.
void
gtk_box_pack_start_prepend (GtkBox    *box,
			    GtkWidget *child,
			    gboolean   expand,
			    gboolean   fill,
			    guint      padding)
{
  GtkBoxChild *child_info;

  g_return_if_fail (box != NULL);
  g_return_if_fail (GTK_IS_BOX (box));
  g_return_if_fail (child != NULL);
  g_return_if_fail (child->parent == NULL);

  child_info = g_new (GtkBoxChild, 1);
  child_info->widget = child;
  child_info->padding = padding;
  child_info->expand = expand ? TRUE : FALSE;
  child_info->fill = fill ? TRUE : FALSE;
  child_info->pack = GTK_PACK_START;

  box->children = g_list_prepend (box->children, child_info);

  gtk_widget_set_parent (child, GTK_WIDGET (box));
  
  if (GTK_WIDGET_REALIZED (box))
    gtk_widget_realize (child);

  if (GTK_WIDGET_VISIBLE (box) && GTK_WIDGET_VISIBLE (child))
    {
      if (GTK_WIDGET_MAPPED (box))
	gtk_widget_map (child);

      gtk_widget_queue_resize (child);
    }
}

