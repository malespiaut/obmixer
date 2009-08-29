/* callbacks.c
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

#include <gtk/gtk.h>
#include <linux/soundcard.h>
#include "callbacks.h"
#include "main.h"
#include "support.h"
#include "actions.h"

GtkWidget *about;
GtkWidget *window1;
GtkWidget *checkbutton1;
GtkAdjustment *vol_adjustment;
StereoVolume vol;
int volume;
extern int volume;
extern int mixer_fd;                    
char tmpstring[30];
char tmpstring2[10];


void
on_checkbutton1_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{

gtk_widget_hide (window1);
ioctl(mixer_fd,MIXER_READ(SOUND_MIXER_VOLUME), &vol);

if( vol.left >= 1 ) {
	volume=vol.left;
	vol.left=vol.right=0;	
} else {
	if( volume <= 0 ) {
		vol.left=vol.right = 5;
	} else {
		vol.left=vol.right = volume;
	}

}

ioctl(mixer_fd,MIXER_WRITE(SOUND_MIXER_VOLUME), &vol);
get_mute_state();

}


gboolean
on_button1_button_press_event          (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	const char *cmd = "pavucontrol&";
	gtk_widget_hide (window1);
	system(cmd);
	return FALSE;
}


gboolean
on_hscale1_value_change_event        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	int volumeset;
	volumeset = (int)gtk_adjustment_get_value(vol_adjustment);
	vol.left=vol.right = volumeset;
	ioctl(mixer_fd,MIXER_WRITE(SOUND_MIXER_VOLUME), &vol);

	get_mute_state();

	return FALSE;
}

