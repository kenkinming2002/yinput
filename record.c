#include <libudev.h>
#include <libinput.h>
#include <linux/input-event-codes.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include <libevdev/libevdev.h>

#include "event.h"

static uint64_t time_subtract(struct timeval x, struct timeval y)
{
  {
    time_t tmp = x.tv_sec < y.tv_sec ? x.tv_sec : y.tv_sec;
    x.tv_sec -= tmp;
    y.tv_sec -= tmp;
  }

  {
    suseconds_t tmp = x.tv_usec < y.tv_usec ? x.tv_usec : y.tv_usec;
    x.tv_usec -= tmp;
    y.tv_usec -= tmp;
  }

  return ((uint64_t)y.tv_sec * (uint64_t)1000000 + (uint64_t)y.tv_usec) - ((uint64_t)x.tv_sec * (uint64_t)1000000 + (uint64_t)x.tv_usec);
}

int record(const char *device_path, const char *file_path)
{
  int fd = open(device_path, O_RDWR);
  struct libevdev *dev;
  if(libevdev_new_from_fd(fd, &dev) != 0) {
    fprintf(stderr, "Failed to create evdev device\n");
    return -1;
  }

  FILE *file = fopen(file_path, "w+");

  bool           initial   = true;
  struct timeval prev_time;
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

    if(initial)
    {
      initial = false;
      prev_time = input_event.time;
    }
    yinput_event.delay = time_subtract(prev_time, input_event.time);
    prev_time = input_event.time;

    fwrite(&yinput_event, sizeof yinput_event, 1, file);
    fflush(file);
  }
}
