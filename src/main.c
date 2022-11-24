/* main.c
 * OBmixer was programmed by Lee Ferrett, derived 
 * from the program "AbsVolume" by Paul Sherman
 * This program is free software; you can redistribute 
 * it and/or modify it under the terms of the GNU General 
 * Public License v2. source code is available at 
 * <http://www.jpegserv.com>
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <linux/soundcard.h>
#include "callbacks.h"
#include "main.h"
#include "support.h"
#include "actions.h"

int mixer_fd;
extern int mixer_fd;
StereoVolume tmpvol;
struct originalset orig;

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
g_object_set_data_full (G_OBJECT (component), name, gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)
#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
g_object_set_data (G_OBJECT (component), name, widget)
GtkStatusIcon *tray_icon;
GtkWidget *window1;
GtkWidget *vbox1;
GtkWidget *hbox2;
GtkWidget *image1;
GtkWidget *hscale1;
GtkWidget *image2;
GtkWidget *hbox1;
GtkWidget *checkbutton1;
GtkWidget *button1;
static GtkWidget *menuitem_mute = NULL;
static GtkWidget *menuitem_about = NULL;
static GtkWidget *menuitem_vol = NULL;
GtkAdjustment *vol_adjustment;
GdkPixbuf *icon0;

void on_mixer(void)
{	

	int no_pavucontrol = system("which pavucontrol | grep /pavucontrol");
	int no_alsamixer = system("which gnome-volume-control | grep /gnome-volume-control");

	if (no_pavucontrol) {
 		if (no_alsamixer) {
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(window1),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
				"\nNo mixer application was not found on your system.\n\nYou will need to install either pavucontrol or gnome-volume-control if you wish to use a mixer from the volume control.");
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
			} else {
			const char *cmd1 = "gnome-volume-control&";
			system(cmd1);
			}
    	} else {
		const char *cmd = "pavucontrol&";
		system(cmd);
	}

gtk_widget_hide (window1);

}

void tray_icon_on_click(GtkStatusIcon *status_icon, gpointer user_data)
{
	get_current_levels();
	if (!GTK_WIDGET_VISIBLE(window1)) {
		gtk_widget_grab_focus(hscale1);
  		gtk_widget_show(window1);
  	} else {
    		gtk_widget_hide (window1);
	}
}


GtkStatusIcon *create_tray_icon() {

	tray_icon = gtk_status_icon_new();

	get_mute_state();

	gtk_status_icon_set_visible(tray_icon, TRUE);
	return tray_icon;

}


GtkWidget*
create_window1 (void)
{

	window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request (window1, 285, 82);
	GTK_WIDGET_SET_FLAGS (window1, GTK_CAN_FOCUS);
	gtk_window_set_position (GTK_WINDOW (window1), GTK_WIN_POS_MOUSE);
	gtk_window_set_resizable (GTK_WINDOW (window1), FALSE);
	gtk_window_set_decorated (GTK_WINDOW (window1), FALSE);
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window1), TRUE);
	gtk_window_set_skip_pager_hint (GTK_WINDOW (window1), TRUE);
	gtk_window_set_type_hint (GTK_WINDOW (window1), GDK_WINDOW_TYPE_HINT_DIALOG);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
 	gtk_container_add (GTK_CONTAINER (window1), vbox1);

	hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox2);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox2, TRUE, TRUE, 0);

	image1 = gtk_image_new_from_stock ("gtk-remove", GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image1);
	gtk_box_pack_start (GTK_BOX (hbox2), image1, FALSE, FALSE, 0);
	gtk_widget_set_size_request (image1, 40, -1);

	vol_adjustment=GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 100, 1, 10, 0));
	/* get original adjustments */
	get_current_levels();

	hscale1 = gtk_hscale_new (GTK_ADJUSTMENT (vol_adjustment));
	gtk_widget_show (hscale1);
	gtk_box_pack_start (GTK_BOX (hbox2), hscale1, FALSE, FALSE, 0);
	gtk_widget_set_size_request (hscale1, 205, -1);
	gtk_scale_set_draw_value (GTK_SCALE (hscale1), FALSE);
	gtk_scale_set_value_pos (GTK_SCALE (hscale1), GTK_POS_BOTTOM);
	gtk_scale_set_digits (GTK_SCALE (hscale1), 0);

	image2 = gtk_image_new_from_stock ("gtk-add", GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image2);
	gtk_box_pack_start (GTK_BOX (hbox2), image2, FALSE, FALSE, 0);
	gtk_widget_set_size_request (image2, 40, -1);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

	checkbutton1 = gtk_check_button_new_with_mnemonic (_("Mute"));
	gtk_widget_show (checkbutton1);
	gtk_box_pack_start (GTK_BOX (hbox1), checkbutton1, FALSE, FALSE, 0);
	gtk_widget_set_size_request (checkbutton1, 97, 18);
	gtk_container_set_border_width (GTK_CONTAINER (checkbutton1), 8);

	button1 = gtk_button_new_with_mnemonic (_("Volume Control..."));
	gtk_widget_show (button1);
	gtk_box_pack_start (GTK_BOX (hbox1), button1, FALSE, FALSE, 0);
	gtk_widget_set_size_request (button1, 185, 19);
	gtk_container_set_border_width (GTK_CONTAINER (button1), 6);

	g_signal_connect ((gpointer) window1, "focus-out-event",G_CALLBACK (hide_me),NULL);
	g_signal_connect ((gpointer) window1, "button_release_event",G_CALLBACK (hide_me),NULL);
	g_signal_connect ((gpointer) hscale1, "key_press_event",G_CALLBACK (hide_me),NULL);
	g_signal_connect ((gpointer) hscale1, "value-changed",G_CALLBACK (on_hscale1_value_change_event),NULL);
	g_signal_connect ((gpointer) checkbutton1, "pressed",G_CALLBACK (on_checkbutton1_clicked),NULL);
	g_signal_connect ((gpointer) button1, "button_press_event",G_CALLBACK (on_mixer),NULL);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (window1, window1, "window1");
	GLADE_HOOKUP_OBJECT (window1, vbox1, "vbox1");
	GLADE_HOOKUP_OBJECT (window1, hbox2, "hbox2");
	GLADE_HOOKUP_OBJECT (window1, image1, "image1");
	GLADE_HOOKUP_OBJECT (window1, hscale1, "hscale1");
	GLADE_HOOKUP_OBJECT (window1, image2, "image2");
	GLADE_HOOKUP_OBJECT (window1, hbox1, "hbox1");
	GLADE_HOOKUP_OBJECT (window1, checkbutton1, "checkbutton1");
	GLADE_HOOKUP_OBJECT (window1, button1, "button1");

	gtk_widget_grab_focus (hscale1);
	return window1;

}


static void popup_callback(GObject *widget, guint button,
				guint activate_time, gpointer user_data)
{

	GtkWidget *menu = user_data;

	gtk_widget_set_sensitive(menuitem_about,TRUE);
	gtk_widget_set_sensitive(menuitem_vol,TRUE);
	gtk_widget_set_sensitive(menuitem_mute,TRUE);

	gtk_widget_hide (window1);

	gtk_menu_popup(GTK_MENU(menu), NULL, NULL,
			gtk_status_icon_position_menu,
			GTK_STATUS_ICON(widget), button, activate_time);
}


static GtkWidget *create_popupmenu(void)
{

	GtkWidget *menu;
	GtkWidget *item;
	GtkWidget *image;

	menu = gtk_menu_new();

	item = gtk_menu_item_new_with_label(_("Mute/Unmute"));
	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	menuitem_mute = item;
	g_signal_connect(item, "activate",G_CALLBACK(on_checkbutton1_clicked), NULL);

	item = gtk_menu_item_new_with_label(_("Volume Control"));
	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	menuitem_vol = item;
	g_signal_connect(item, "activate",G_CALLBACK(on_mixer), NULL);


	image = gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
	item = gtk_image_menu_item_new_with_label(_("About"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	menuitem_about = item;
	g_signal_connect(item, "activate",G_CALLBACK(create_about), NULL);

	return menu;

}


GtkWidget*
create_about (void)
{

	GtkWidget *about;
	GtkWidget *vbox1;
	GtkWidget *about_image;
	GtkWidget *label1;

	about = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (about), _("About OBMixer"));

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (about), vbox1);

	about_image = create_pixmap (about, "obmixer-sp.png");
	gtk_widget_show (about_image);
	gtk_box_pack_start (GTK_BOX (vbox1), about_image, TRUE, TRUE, 16);

	label1 = gtk_label_new (_("A GTK mixer Applet for window mangers\n\nhttp://www.jpegserv.com/obmixer"));;

	gtk_widget_show (label1);
	gtk_box_pack_start (GTK_BOX (vbox1), label1, FALSE, FALSE, 0);
	gtk_widget_set_size_request (label1, 250, 70);
	gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_CENTER);

	g_signal_connect ((gpointer) about, "delete_event",G_CALLBACK (gtk_widget_destroy),NULL);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (about, about, "about");
	GLADE_HOOKUP_OBJECT (about, vbox1, "vbox1");
	GLADE_HOOKUP_OBJECT (about, about_image, "about_image");
	GLADE_HOOKUP_OBJECT (about, label1, "label1");

	gtk_widget_show(about);

}


void get_current_levels() {

	ioctl(mixer_fd,MIXER_READ(SOUND_MIXER_VOLUME), &tmpvol);
	orig.mainvol=tmpvol.left;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(vol_adjustment), (double) tmpvol.left);

}


void get_mute_state() {

ioctl(mixer_fd,MIXER_READ(SOUND_MIXER_VOLUME), &tmpvol);

	if( tmpvol.left > 0 ) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton1), FALSE);
		icon0 = create_pixbuf ("obmixer-a.png");
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton1), TRUE);
  		icon0 = create_pixbuf ("obmixer-i.png");
	}

gtk_status_icon_set_from_pixbuf(tray_icon, icon0);

}


void hide_me() {
	gtk_widget_hide(window1);
}


main (int argc, char *argv[])
{

	mixer_fd = open ("/dev/mixer", O_RDWR, 0);
	if (mixer_fd < 0)
		printf ("Error opening mixer device\n"), exit (1);

	GtkWidget *window1;
	GtkWidget *menu;

	#ifdef ENABLE_NLS
		bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
		bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
		textdomain (GETTEXT_PACKAGE);
	#endif

	gtk_set_locale ();
	gtk_init (&argc, &argv);

	add_pixmap_directory (PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps");

	window1 = create_window1 ();
	gtk_widget_realize(window1);
	gtk_widget_hide(window1);

	tray_icon = create_tray_icon();        
	menu = create_popupmenu();
	g_signal_connect(G_OBJECT(tray_icon), "popup-menu",G_CALLBACK(popup_callback), menu);
	g_signal_connect(G_OBJECT(tray_icon), "activate", G_CALLBACK(tray_icon_on_click), NULL);

	gtk_main ();
	
	close(mixer_fd);
	return 0;

}
