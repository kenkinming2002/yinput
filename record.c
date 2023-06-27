#include <libudev.h>
#include <libinput.h>
#include <linux/input-event-codes.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>

#include <libevdev/libevdev.h>

#include "event.h"

int record(const char *device_path, const char *file_path)
{
  int fd = open(device_path, O_RDWR);
  struct libevdev *dev;
  if(libevdev_new_from_fd(fd, &dev) != 0) {
    fprintf(stderr, "Failed to create evdev device\n");
    return -1;
  }

  FILE *file = fopen(file_path, "w+");
  for(;;)
  {
    struct pollfd fds;
    fds.fd      = fd;
    fds.events  = POLLIN;
    fds.revents = 0;
    if(poll(&fds, 1, -1) == -1)
      return -1;

    struct input_event input_event;
    libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &input_event);

    struct yinput_event yinput_event;
    yinput_event.type  = input_event.type;
    yinput_event.code  = input_event.code;
    yinput_event.value = input_event.value;
    fwrite(&yinput_event, sizeof yinput_event, 1, file);
    fflush(file);
  }
}
