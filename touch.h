#include <FT6336.h>

 #define TOUCH_FT6336
 #define TOUCH_FT6336_SCL 22
 #define TOUCH_FT6336_SDA 21
 #define TOUCH_FT6336_INT 15
 #define TOUCH_FT6336_RST 13
 #define TOUCH_MAP_X1 0
 #define TOUCH_MAP_X2 240
 #define TOUCH_MAP_Y1 0
 #define TOUCH_MAP_Y2 320

int touch_last_x = 0, touch_last_y = 0;
unsigned short int width=0, height=0, rotation,min_x=0,max_x=0,min_y=0,max_y=0;

FT6336 ts = FT6336(TOUCH_FT6336_SDA, TOUCH_FT6336_SCL, TOUCH_FT6336_INT, TOUCH_FT6336_RST, max(TOUCH_MAP_X1, TOUCH_MAP_X2), max(TOUCH_MAP_Y1, TOUCH_MAP_Y2));

void touch_init(unsigned short int w, unsigned short int h,unsigned char r)
{
  width = w; 
  height = h;
  switch (r){
    case ROTATION_NORMAL:
    case ROTATION_INVERTED:
      min_x = TOUCH_MAP_X1;
      max_x = TOUCH_MAP_X2;
      min_y = TOUCH_MAP_Y1;
      max_y = TOUCH_MAP_Y2;
      break;
    case ROTATION_LEFT:
    case ROTATION_RIGHT:
      min_x = TOUCH_MAP_Y1;
      max_x = TOUCH_MAP_Y2;
      min_y = TOUCH_MAP_X1;
      max_y = TOUCH_MAP_X2;
      break;
    default:
      break;
  }
  ts.begin();
  ts.setRotation(r);
}

bool touch_touched(void)
{
  ts.read();
  if (ts.isTouched)
  {
    touch_last_x = map(ts.points[0].x, min_x, max_x, 0, width - 1);
    touch_last_y = map(ts.points[0].y, min_y, max_y, 0, height - 1);
    Serial.printf("Touched at x: %.1d. y: %.1d\n", touch_last_x, touch_last_y);
    return true;
  }
  else
  {
    return false;
  }
}

bool touch_getXY(uint16_t* x, uint16_t* y) {
    if (touch_touched()) {
        *x = touch_last_x;
        *y = touch_last_y;
        return true;
    }
    return false;
}

bool touch_has_signal(void)
{
  return true;
}

bool touch_released(void)
{
  return true;
}
