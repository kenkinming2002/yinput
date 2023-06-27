#include <stdint.h>

struct yinput_event
{
  uint16_t type;
  uint16_t code;
  int32_t  value;
  uint64_t delay; // in microseconds
};

//#define EVENT_TYPE_NONE                    0
//#define EVENT_TYPE_POINTER_MOTION_ABSOLUTE 1
//#define EVENT_TYPE_POINTER_MOTION          2
//#define EVENT_TYPE_BUTTON_UP               3
//#define EVENT_TYPE_BUTTON_DOWN             4
//
//#define EVENT_BUTTON_LEFT   1
//#define EVENT_BUTTON_MIDDLE 2
//#define EVENT_BUTTON_RIGHT  3
//
//struct event
//{
//  uint8_t type;
//  union
//  {
//    struct { double x, y; } motion;
//    uint8_t button;
//  };
//};
