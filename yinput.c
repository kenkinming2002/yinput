#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int record(const char *device_path, const char *file_path);
int replay(const char *device_path, const char *file_path);

static int usage()
{
  fprintf(stderr, "Usage: yinput record|replay DEVICE FILE\n");
  return 1;
}

int main(int argc, char *argv[])
{
  if(argc != 4)
    return usage();

  const char *device_path = argv[2];
  const char *file_path   = argv[3];
  if(strcmp(argv[1], "record") == 0)
    return record(device_path, file_path);
  else if(strcmp(argv[1], "replay") == 0)
    return replay(device_path, file_path);
  else
    return usage();
}
