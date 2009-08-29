
typedef struct stereovolume
{
  unsigned char left;
  unsigned char right;
} StereoVolume;

int save_to_file(int);
int load_from_file(int);
int args(int, char **);


