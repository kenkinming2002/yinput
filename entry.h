#include <stddef.h>
#include <stdint.h>
#include <linux/input.h>

struct entry
{
  uint8_t  device;
  uint16_t type;
  uint16_t code;
  int32_t  value;
  uint64_t time; // In nanoseconds
};

