#include <gtk/gtk.h>

extern GtkAdjustment *vol_adjustment;
extern GtkWidget *checkbutton1;
extern GtkWidget *window1;

void
on_checkbutton1_clicked                (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_hscale1_value_change_event        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);
