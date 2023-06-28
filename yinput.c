#include <stdio.h>
#include <string.h>

int record(const char *file_path);
int replay(const char *file_path);

static int usage()
{
  fprintf(stderr, "Usage: yinput record|replay FILE\n");
  return 1;
}

int main(int argc, char *argv[])
{
  if(argc != 3)
    return usage();

  const char *file_path = argv[2];
  if(strcmp(argv[1], "record") == 0)
    return record(file_path);
  else if(strcmp(argv[1], "replay") == 0)
    return replay(file_path);
  else
    return usage();
}
