#include "entry.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

int record(const char *file_path)
{
  // 1: Open all input devices and setup necessary data structures for polling
  struct pollfd *fds  = NULL;
  uint8_t        nfds = 0;
  for(;;)
  {
    int length = snprintf(NULL, 0, "/dev/input/event%u", nfds);
    char *path = malloc(length+1);
    snprintf(path, length+1, "/dev/input/event%u", nfds);

    int fd = open(path, O_RDONLY | O_NONBLOCK);
    if(fd == -1) {
      free(path);
      break;
    }

    ++nfds;
    fds = realloc(fds, nfds * sizeof *fds);
    fds[nfds-1].fd      = fd;
    fds[nfds-1].events  = POLLIN;
    fds[nfds-1].revents = 0;
  }

  if(nfds == 0)
  {
    fprintf(stderr, "ERROR: No input device detected.\n");
    fprintf(stderr, "ERROR: Please check that /dev/input/event* is accessible.\n");
    fprintf(stderr, "ERROR: You may have to add yourself to the input group.\n");
    return -1;
  }
  fprintf(stderr, "INFO: input device count = %u\n", nfds);

  // 2: Open output file
  int fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd == -1) {
    fprintf(stderr, "ERROR: failed to open output file: %s\n", strerror(errno));
    return -1;
  }

  // 3: Loop the loop
  for(;;)
  {
    if(poll(fds, nfds, -1) == -1) {
      fprintf(stderr, "ERROR: poll() failed: %s\n", strerror(errno));
      return -1;
    }

    for(uint8_t i=0; i<nfds; ++i)
    {
      if(fds[i].revents == 0)
        continue;

      for(;;)
      {
        struct input_event input_event;
        if(read(fds[i].fd, &input_event, sizeof input_event) == -1)
        {
          if(errno == EWOULDBLOCK || errno == EAGAIN)
            break;

          fprintf(stderr, "ERROR: read() failed: %s\n", strerror(errno));
          return -1;
        }

        struct entry entry;
        memset(&entry, 0, sizeof entry);
        entry.device   = i;
        entry.type  = input_event.type;
        entry.code  = input_event.code;
        entry.value = input_event.value;
        entry.time  = (uint64_t)input_event.time.tv_sec  * (uint64_t)1000000000
                    + (uint64_t)input_event.time.tv_usec * (uint64_t)1000;

        if(write(fd, &entry, sizeof entry) == -1) {
          fprintf(stderr, "ERROR: write() failed: %s\n", strerror(errno));
          return -1;
        }
      }
    }
  }
}

