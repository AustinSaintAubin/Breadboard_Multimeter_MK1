#ifndef graph_h
#define graph_h
#include <Arduino.h>
#include <Adafruit_SSD1306.h>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// -- Class [ Display Graph ]  Display line graph in area.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LineGraph { // x0, y0, x1, y1, min, max
  // private:
    // Class Member Variables
    // Variables initialized at startup
    byte x, y;
    byte width, height;
    float min, max;

    const int8_t offset_frame = 6;

    byte graph_progress = 0;

    // Variables to maintain the current state
    //byte plot_data [128 +2] = { 0 }; // all elements 0  // JUST GOING TO START WITH 65 (width + 1)
    //byte plot_data [];  // Setting size in constructor
    int8_t *plot_data;  // Setting size in constructor
    #define FLT_MAX 3.4028235E+38

    // Map Float Fuction
    float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);

  public:
    Adafruit_SSD1306 *display;
    //TwoWire *wire (TwoWire *twi = &Wire)
    
    //LineGraph(unsigned int x_0, unsigned int y_0, unsigned int x_1, unsigned int y_1, float graph_value_min, float graph_value_max, Adafruit_SSD1306 *disp = &display);
    LineGraph(byte x_0, byte y_0, byte x_1_width, byte y_1_height, float graph_value_min, float graph_value_max, Adafruit_SSD1306 &display);
    void config(byte x_0, byte y_0, byte x_1_width, byte y_1_height, float graph_value_min, float graph_value_max, Adafruit_SSD1306 &display);
    void plot(float value, float graph_value_min, float graph_value_max);
    void plot(float graph_value_min, float graph_value_max);
    void plot(float value);
    void plot();
    void store(float value);
};
#endif
