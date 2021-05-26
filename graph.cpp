#include "graph.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// -- Class Constructor [ Display Graph ]  Display line graph in area.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LineGraph::LineGraph(byte x_0, byte y_0, byte x_1_width, byte y_1_height, float graph_value_min, float graph_value_max, Adafruit_SSD1306 &disp) {
  config(x_0, y_0, x_1_width, y_1_height, graph_value_min, graph_value_max, disp);
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// -- Class Constructor [ Display Graph ]  Display line graph in area.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void LineGraph::config(byte x_0, byte y_0, byte x_1_width, byte y_1_height, float graph_value_min, float graph_value_max, Adafruit_SSD1306 &disp) {
  // initialize All Variables
  x = x_0;
  y = y_0;
  width  = x_1_width;
  height = y_1_height;
  
  // Pass Display Class 
  display = &disp;
  
  // set min and max values expected for graph
  min = graph_value_min;
  max = graph_value_max;
  
  // Set Array Size, zeroed
  plot_data = new int8_t[width] { 0 };
  
  // Set Array Initial Value
  for (int i = width; i > 0; --i) {
    plot_data[i] = FLT_MAX;
  }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// -- Class Fuction [ Display Graph ]  Display line graph in area, just draw last plot.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void LineGraph::plot() {
  // Plot Data
  LineGraph::plot(FLT_MAX);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// -- Class Fuction [ Display Graph ]  Display line graph in area, just draw last plot.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void LineGraph::plot(float graph_value_min, float graph_value_max) {
  // Plot Data
  LineGraph::plot(FLT_MAX, graph_value_min, graph_value_max);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// -- Class Fuction [ Display Graph ]  Display line graph in area.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void LineGraph::plot(float value, float graph_value_min, float graph_value_max) {

  // Clear if net Min/Max
  if (min != graph_value_min || max != graph_value_max) {
    // set min and max values expected for graph
    min = graph_value_min;
    max = graph_value_max;
    
    // Set Array Initial Value (Clear)
    for (int i = width; i > 0; --i) {
      plot_data[i] = FLT_MAX;
    }
  }

  // Plot Data
  LineGraph::plot(value);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// -- Class Fuction [ Update & Store Values for Graph ]  Update the values, but do not show anything
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void LineGraph::store(float value) {

  // Check if value out of range, or null value.
  if (value != FLT_MAX) {
    // Shift Plotted Graph
    for( byte i = width; i > 0; i-- ) {
      plot_data[i] = plot_data[i -1];
      // Serial.println((String)i + " | " + "plot:" + plot_data[i]);
    }
    
    // Shift for Base Lines
    graph_progress++;
  }
  
  // Check if value out of range, or null value.
  if (value != FLT_MAX) {
    // Get latest reading
    //plot_data[0] = mapfloat(value, min, max, 0, height);
    plot_data[0] = mapfloat(value, min, max, 0 +offset_frame, height -offset_frame);
  }
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// -- Class Fuction [ Display Graph ]  Display line graph in area.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void LineGraph::plot(float value) {

  // Store Lastest Value and Shift Array
  LineGraph::store(value);
  
  // Offset Varables
  //int8_t offset_frame = 6;
  int8_t offset = 0;
  
  // Set Offset
  if ( plot_data[0] > ((byte)height - offset_frame) ) {
    // Offset Max
    offset = (plot_data[0] - height) + offset_frame;
  } else if ( plot_data[0] < 0 + offset_frame) {
    // Offset Min
    offset = plot_data[0] - offset_frame;
  }

  // Debug
  #ifdef DEBUG
  Serial.println("0 | x:" + (String)x + " y:" + (String)y +  " | width:" + width + " height:" + height + " | " + (String)plot_data[0]+  + " | offset:" + (String)offset + " | mapped:" + (String)value_mapped + "  x: " + (String)(0 + x) + ", y:" + (String)(  ((height + y) - (map(plot_data[0], min, max, 0, height)))   ));

  // Display Screen Location
  if (plot_data[0] < 0)
    display->setCursor(width -30, height -9);
  else
    display->setCursor(width -24, height -9);
  display->setTextSize(1);
  display->println(plot_data[0]);
  #endif

  // Line Graph
  byte plot_data_mapped = 0;
  for( byte i = 0; i < width; i++ ) {
    plot_data_mapped = plot_data[i] - offset;

    // Plot & Display Graph
    if ( plot_data_mapped <= height && plot_data_mapped >= 0 ) {
      // 1 | x0:0 y0:24 x1:47 y1:63 | width:47 height:39 | 40.49  x: 1, y:54
      #ifdef DEBUG
      Serial.println((String)i + " | " + "x:" + (String)x + " y:" + (String)y +  " | width:" + width + " height:" + height + " | " + (String)plot_data[i]+ " | mapped:" + (String)mapfloat(plot_data[i], min, max, 0, height) + "  x: " + (String)(i + x) + ", y:" + (String)(  ((height + y) - (map(plot_data[i], min, max, 0, height)))   ));
      #endif
      
      // Display Pixel
      display->drawPixel(i + x, (height + y) - constrain(plot_data_mapped, 0, height), WHITE);
    }
  }
  
  // Side Lines
  for( byte i = 0; i < height; i++ ) {
    if ( !((i - offset) % 8) ) {
      display->drawPixel(x, y + i, WHITE);
      display->drawPixel(x + width -1, y + i, WHITE);
    }
  }
  
  // Base Lines
  for( byte i = 0; i < width; i++ ) {
    if ( !((i - graph_progress) % 16) ) {
      display->drawPixel(i + x, height -1 + y, WHITE);
    }
  }
}


float LineGraph::mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
