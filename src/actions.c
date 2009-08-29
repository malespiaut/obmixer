/* actions.c
 * OBmixer was programmed by Lee Ferrett, derived 
 * from the program "AbsVolume" by Paul Sherman
 * This program is free software; you can redistribute 
 * it and/or modify it under the terms of the GNU General 
 * Public License v2. source code is available at 
 * <http://www.jpegserv.com>
 */
#include <linux/soundcard.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "actions.h"

int args(int argc, char **argv) 
{
  StereoVolume vol;
  extern int mixer_fd;
  if(argc>2) {
    vol.left=vol.right=atoi(argv[2]);
  } 

    if (argv[1][0]=='-') 
    {
      ioctl(mixer_fd,MIXER_READ(SOUND_MIXER_VOLUME), &vol);
      vol.left=vol.right=vol.left-atoi(argv[1]+1);
      ioctl(mixer_fd,MIXER_WRITE(SOUND_MIXER_VOLUME), &vol);
    } 
    else if (argv[1][0]=='+') 
    {
      ioctl(mixer_fd,MIXER_READ(SOUND_MIXER_VOLUME), &vol);
      vol.left=vol.right=vol.left+atoi(argv[1]+1);
      ioctl(mixer_fd,MIXER_WRITE(SOUND_MIXER_VOLUME), &vol);
    }
    else if (!strcmp(argv[1],"list")) 
    {
      ioctl(mixer_fd,MIXER_READ(SOUND_MIXER_VOLUME), &vol);
      printf("vol: \t\t%i\n",vol.left);
    }
    else if (!strcmp(argv[1],"mute")) 
    {
      vol.left=vol.right=0;
      ioctl(mixer_fd,MIXER_WRITE(SOUND_MIXER_VOLUME), &vol);
    }
    else if (argv[1][0]<48 || argv[1][0]>57) 
    {
      printf("option not recognized ... exiting\n");
      exit(1);
    } 
    else 
    {
      vol.left=vol.right=atoi(argv[1])%100;
      ioctl(mixer_fd,MIXER_WRITE(SOUND_MIXER_VOLUME), &vol);
    }
    
  return 0;
}



