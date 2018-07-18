#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <vector>

using namespace std;

Adafruit_AMG88xx amg;

uint8_t pixels[AMG88xx_PIXEL_ARRAY_SIZE];

char readGridEye = 'f';

vector <byte*> grid_eye_vector;

int row_count = 0;
int col_count = 0;

byte* grid_eye_read()
{
  amg.readPixels(pixels, (uint8_t)AMG88xx_PIXEL_ARRAY_SIZE);

  return pixels;
}
