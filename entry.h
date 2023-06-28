#include <stddef.h>
#include <linux/input.h>

struct entry
{
  size_t             device_no;
  struct input_event input_event;
};

