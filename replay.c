#include "entry.h"

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>

// Copied from https://www.gnu.org/software/libc/manual/html_node/Calculating-Elapsed-Time.html
static struct timeval timeval_subtract(struct timeval x, struct timeval y)
{
  struct timeval result;

  if (x.tv_usec < y.tv_usec) {
    int nsec = (y.tv_usec - x.tv_usec) / 1000000 + 1;
    y.tv_usec -= 1000000 * nsec;
    y.tv_sec += nsec;
  }
  if (x.tv_usec - y.tv_usec > 1000000) {
    int nsec = (x.tv_usec - y.tv_usec) / 1000000;
    y.tv_usec += 1000000 * nsec;
    y.tv_sec -= nsec;
  }

  result.tv_sec = x.tv_sec - y.tv_sec;
  result.tv_usec = x.tv_usec - y.tv_usec;
  return result;
}

static struct timespec timeval_to_timespec(struct timeval val)
{
  struct timespec res;
  res.tv_sec = val.tv_sec;
  res.tv_nsec = val.tv_usec * 1000;
  return res;
}

static int entry_cmp(const void *_first, const void *_second)
{
  const struct entry *first = _first, *second = _second;
  if(first->time < second->time)
    return -1;
  else if(first->time > second->time)
    return 1;
  else
    return 0;
}

int replay(const char *file_path)
{
  // 1: Open, read and process input file
  int fd = open(file_path, O_RDONLY);
  if(fd == -1) {
    fprintf(stderr, "ERROR: failed to open input file %s: %s\n", file_path, strerror(errno));
    return -1;
  }

  struct stat stat;
  if(fstat(fd, &stat) == -1) {
    fprintf(stderr, "ERROR: failed to stat input file %s: %s\n", file_path, strerror(errno));
    return -1;
  }

  struct entry *entries = malloc(stat.st_size);
  if(read(fd, entries, stat.st_size) != stat.st_size) {
    fprintf(stderr, "ERROR: failed to read input file %s: %s\n", file_path, strerror(errno));
    return -1;
  }
  size_t entry_count = stat.st_size / sizeof(struct entry);
  qsort(entries, entry_count, sizeof *entries, entry_cmp);

  // 2: Loop through all input devices
  struct libevdev        **devs      = NULL;
  uint8_t                  dev_count = 0;
  for(;;)
  {
    int length = snprintf(NULL, 0, "/dev/input/event%u", dev_count);
    char *path = malloc(length+1);
    snprintf(path, length+1, "/dev/input/event%u", dev_count);

    int fd = open(path, O_RDONLY | O_NONBLOCK);
    if(fd == -1) {
      free(path);
      break;
    }

    struct libevdev *dev;
    if(libevdev_new_from_fd(fd, &dev) != 0) {
      fprintf(stderr, "ERROR: Failed to create evdev device\n");
      return -1;
    }

    ++dev_count;
    devs = realloc(devs, dev_count * sizeof *devs);
    devs[dev_count-1] = dev;
  }

  // 3: Create corresponding virtual devices and wait 1s for it to be picked up
  //    by e.g. X11 server or wayland compositor
  struct libevdev_uinput **uinputs      = malloc(dev_count * sizeof (*uinputs) );
  uint8_t                  uinput_count = dev_count;
  for(uint8_t i=0; i<uinput_count; ++i)
  {
    struct libevdev_uinput *uinput;
    if(libevdev_uinput_create_from_device(devs[i], LIBEVDEV_UINPUT_OPEN_MANAGED, &uinput) != 0) {
      fprintf(stderr, "Failed to create uinput device");
      return -1;
    }
    uinputs[i] = uinput;
  }
  sleep(1);

  // 4: Replay events with uinput
  for(size_t i=0; i<entry_count; ++i)
  {
    if(i != 0)
    {
      assert(entries[i].time >= entries[i-1].time);
      uint64_t delay = entries[i].time - entries[i-1].time;

      struct timespec ts;
      ts.tv_sec  = delay / 1000000000;
      ts.tv_nsec = delay % 1000000000;
      if(nanosleep(&ts, NULL) == -1) {
        fprintf(stderr, "ERROR: nanosleep() failed: %s\n", strerror(errno));
        return -1;
      }

      int status;
      if((status = libevdev_uinput_write_event(uinputs[entries[i].device], entries[i].type, entries[i].code, entries[i].value)) != 0) {
        fprintf(stderr, "ERROR: failed to inject event via uinput: %s\n", strerror(-status));
        fprintf(stderr, "ERROR: event type  = %u\n", entries[i].type);
        fprintf(stderr, "ERROR: event code  = %u\n", entries[i].code);
        fprintf(stderr, "ERROR: event value = %d\n", entries[i].value);
        fprintf(stderr, "ERROR: Maybe your input file is corrupted?\n");
        return -1;
      }
    }
  }
  return 0;
}

