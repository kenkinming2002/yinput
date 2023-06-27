#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/uinput.h>

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

#include "event.h"

int replay(const char *device_path, const char *file_path)
{
  int fd   = open(device_path, O_RDWR);
  int uifd = open("/dev/uinput", O_RDWR);

  struct libevdev *dev;
  if(libevdev_new_from_fd(fd, &dev) != 0) {
    fprintf(stderr, "Failed to create evdev device\n");
    return -1;
  }

  struct libevdev_uinput *uidev;
  if(libevdev_uinput_create_from_device(dev, uifd, &uidev) != 0) {
    fprintf(stderr, "Failed to create uinput device");
    return -1;
  }

  sleep(1);

  FILE *file = fopen(file_path, "r");

  struct yinput_event yinput_event;
  while(fread(&yinput_event, sizeof yinput_event, 1, file) == 1)
  {
    libevdev_uinput_write_event(uidev, yinput_event.type, yinput_event.code, yinput_event.value);

    struct timespec ts;
    ts.tv_sec  = 0;
    ts.tv_nsec = 1000000;
    nanosleep(&ts, NULL);
  }

  return 0;
}
