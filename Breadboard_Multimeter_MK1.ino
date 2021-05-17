 /*===========================================================================
  Title: Breadboard Multimeter MK1
   - Description: Breadboard Multimeter using Arduino Every or Nano, INA219, and SSD1306 OLED Screen

  Version: 0.8
  Date: 2021 / 05 / 15
  Author: Austin St. Aubin
  Email: AustinSaintAubin@gmail.com
  Released under a MIT license: http://opensource.org/licenses/MIT

  Notes:
   - Based on: https://github.com/adafruit/Adafruit_INA219/blob/master/examples/ina_poweroled/ina_poweroled.ino
essd
  Resources:
   - https://github.com/adafruit/Adafruit_INA219
   - https://github.com/adafruit/Adafruit_SSD1306
   - https://github.com/adafruit/Adafruit-GFX-Library
   - https://github.com/pilotak/MovingAverageFloat

  ============================================================================= */
// [ Included Library's ]
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_INA219.h>
#include "MovingAverageFloat.h"  // https://github.com/pilotak/MovingAverageFloat

// [ Helper Macros ]
#define LINE(name,val) Serial.print(name); Serial.print(":\t"); Serial.println(val);  // LINE("F_CPU", F_CPU);

// [ Preprocessor Statements ]
//#define DEBUG         // Debugging output and logic
#define SERIAL_OUTPUT    // Output statments over serial
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define SCREEN_ROTATION 0  // Configure orientation of the display.: 0 = none, 1 = 90 degrees clockwise, 2 = 180 degrees, 3 = 270 degrees CW
#define SCREEN_RESET  -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define UPDATE_RATE_GRAPH_MS 600//1500   // Update Rate of Graphs in Milliseconds 
#define UPDATE_RATE_SERIAL_MS 3000   // Update Rate of Serial Output in Milliseconds 

// [ Global Pin Constants ]
const int led_pin = LED_BUILTIN;  // LED pin for flashing or error.
// - - - - - - - - - - - - - - - - - - - - - - - - -
const int button_1_pin = 2;
//const int button_2_pin = 3;

// [ Global Constants ]

// [ Global Variables ]
      byte screen  = 0;  // Current/Default Screen Page
const byte screens = 4;  // Quanity of Screens

// [ Global Class Constructors ]

// OLED Screen
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, SCREEN_RESET);

// Averages
#define moving_average_samples 16  // Number of Sample to Average
MovingAverageFloat <moving_average_samples> voltage_load_V_avg;
MovingAverageFloat <moving_average_samples> current_A_avg;
MovingAverageFloat <moving_average_samples> power_W_avg;

// Current Sensor
Adafruit_INA219 ina219;

// Line Graphs
/*
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
};
*/
#include "graph.h"
// LineGraph graphVotage (x, y, width, height, min, max, display);
LineGraph graphVotage (0,                  0, (SCREEN_WIDTH / 2), SCREEN_HEIGHT, 3.3, 5, display);
LineGraph graphCurrent((SCREEN_WIDTH / 2), 0, (SCREEN_WIDTH / 2), SCREEN_HEIGHT, 0.0, 0.25, display);


void setup()  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
{ // put your setup code here, to run once:
  #ifdef SERIAL_OUTPUT
  Serial.begin(115200);
  while (!Serial) {
      // will pause Zero, Leonardo, etc until serial console opens
      delay(1);
  }
  #endif

  // Clear Serial
  Serial.write(27);  // Clears Screen, prints "esc"
  delay(100);        // Wait while screen is cleared
  
  // OLED Screen Setup
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1); // Don't proceed, loop forever
  }
  Wire.setClock(400000);

  // Clear the display.
  display.clearDisplay();
  display.display();

  // Set rotation.
  display.setRotation(SCREEN_ROTATION);

  // Setup Screen Defaults
  display.setTextSize(1);
  display.setTextColor(WHITE);
  //display.invertDisplay(true);
  display.dim(true);
  display.setCursor(0,0);

  // Splash Serial
  Serial.println("Breadboard Multimeter MK1");
  Serial.println("By: Austin St. Aubin");
  
  // Compliation Info
  LINE("Date", __DATE__);
  LINE("Time", __TIME__);
  LINE("Version", __VERSION__);
  LINE("File", __FILE__);

  // Splash Screen
  display.println("Breadboard Multimeter");
  display.println("By: Austin St. Aubin");
  display.println(__DATE__);
  display.println(__TIME__);
  display.display();

  // Initialize Sensor INA219
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    display.println("Failed to find INA219 chip");
    display.display();
    while (true) { delay(10); }
  }
  // By default the INA219 will be calibrated with a range of 32V, 2A.
  // However uncomment one of the below to change the range.  A smaller
  // range can't measure as large of values but will measure with slightly
  // better precision.
  //ina219.setCalibration_32V_1A();
  //ina219.setCalibration_16V_400mA();

  // Output Serial Table Header
  Serial.print("Voltage(VIN+)");
  Serial.print(",");
  Serial.print("Voltage(VIN-)");
  Serial.print(",");
  Serial.print("Shunt(V)");
  Serial.print(",");
  Serial.print("Load(V)");
  Serial.print(",");
  Serial.print("Load(A)[Shunt]");
  Serial.print(",");
  Serial.print("Load(AH)");
  Serial.print(",");
  Serial.print("Power(W)");
  Serial.print(",");
  Serial.print("Power(WH)");
  Serial.print(",");
  Serial.print("");
  Serial.println();
  
  // Set Status LED Pin Mode
  pinMode(led_pin, OUTPUT);

  // Buttons Pin Mode
  pinMode(button_1_pin, INPUT);
//  pinMode(button_2_pin, INPUT);

  // Delay to Show Splash Screen
  delay(1000);

  // Attach Button to Interrupt Service Routine (ISR)
  attachInterrupt(digitalPinToInterrupt(button_1_pin), BUTTON_ISR, RISING);
}

void loop()  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
{ // put your main code here, to run repeatedly:
  // Status LED
  static bool status_led = true;
  digitalWrite(led_pin, status_led);
  status_led = !status_led;
  
  // Track Loop Time
  unsigned long millis_current = millis();
  static unsigned long millis_last_update_graph = millis_current;
  static unsigned long millis_last_update_serial = millis_current;
  static unsigned long millis_watt_hour_loop_start;
  static unsigned long millis_watt_hour_loop_end;
  
  // Read Values from INA219.
  float voltage_shunt_V = ina219.getShuntVoltage_mV() / 1000;
  float voltage_bus_V = ina219.getBusVoltage_V();
  float current_A = ina219.getCurrent_mA() / 1000;
  float power_W = ina219.getPower_mW() / 1000;

  // Calculate Load Voltage, Power, and Amp-Hours.
  float voltage_load_V = voltage_bus_V + voltage_shunt_V;
  
  // Calculate Watt-Hours and and Amp-Hours
  static double watt_hours = 0;
  static double amp_hours = 0;
  millis_watt_hour_loop_end = millis();
  unsigned long millis_watt_hour_loop_time = millis_watt_hour_loop_end - millis_watt_hour_loop_start;
  watt_hours = watt_hours + ((double)power_W * ((double)millis_watt_hour_loop_time/60/60/1000));    //Calculate watt hours used.
  amp_hours = amp_hours + ((double)current_A * ((double)millis_watt_hour_loop_time/60/60/1000));    //Calculate watt hours used.
  millis_watt_hour_loop_start = millis();
  
  // Min, Max, Average Varables
//  static const byte    avg_factor = 16;
//  static unsigned long avg_samples = 0; avg_samples++; // Also ++'s every loop
//  static float voltage_load_V_avg = 0;
  static float voltage_load_V_max = voltage_load_V;
  static float voltage_load_V_min = voltage_load_V;
//  static float current_A_avg = current_A_avg;
  static float  current_A_max = current_A;
  static float  current_A_min = current_A;
  
  
  // Voltage Min, Max, Average
  if (voltage_load_V > voltage_load_V_max) {
    voltage_load_V_max = voltage_load_V;
  } else if (voltage_load_V < voltage_load_V_min) {
    voltage_load_V_min = voltage_load_V;
  }
//  voltage_load_V_avg = voltage_load_V_avg + (voltage_load_V - voltage_load_V_avg) / min(avg_samples, avg_factor);
  voltage_load_V_avg.add(voltage_load_V);
  
  
  // Current Min, Max, Average
  if (current_A > current_A_max) {
    current_A_max = current_A;
  } else if (current_A < current_A_min) {
    current_A_min = current_A;
  }
//  current_A_avg = current_A_avg + (current_A - current_A_avg) / min(avg_samples, avg_factor);
  current_A_avg.add(current_A);


  // Wattage Average
  power_W_avg.add(power_W);

  // - - - - - - - - - - - - - - - - - - - - - - - -
  
  // Serial Logging Output
  if (millis_current - millis_last_update_serial > UPDATE_RATE_SERIAL_MS) {
    // Serial Plotting
    Serial.print(voltage_bus_V + voltage_shunt_V);  // Voltage[VIN+]
    Serial.print(",");
    Serial.print(voltage_bus_V);                    // Voltage[VIN-]
    Serial.print(",");
    Serial.print(voltage_shunt_V);                  // Shunt(V)
    Serial.print(",");
    Serial.print(voltage_load_V);                   // Load(V)
    Serial.print(",");
    Serial.print(current_A);                        // Load(A)[Shunt]
    Serial.print(",");
    Serial.print(amp_hours);                        // Load(AH)
    Serial.print(",");
    Serial.print(power_W);                          // Power(W)
    Serial.print(",");
    Serial.print(watt_hours);                       // Power(WH)
    Serial.println();

//    // Testing Output
//    Serial.write(27);  // Clears Screen, prints "esc"  
//    Serial.println("------------------------");
//    #define SERIAL_PRECISION 8 // number of decial places to show
//    Serial.print("Voltage (VIN+) "); Serial.println(getESUM(voltage_bus_V + voltage_shunt_V, "V", SERIAL_PRECISION));
//    Serial.print("Voltage (VIN-) "); Serial.println(getESUM(voltage_bus_V, "V", SERIAL_PRECISION));
//    Serial.print("Shunt Voltage  "); Serial.println(getESUM(voltage_shunt_V, "V", SERIAL_PRECISION));
//    Serial.print("Load Votage    "); Serial.println(getESUM(voltage_load_V, "V", SERIAL_PRECISION));
//    Serial.print("|Max Votage    "); Serial.println(getESUM(voltage_load_V_max, "V", SERIAL_PRECISION));
//    Serial.print("|Avg Votage    "); Serial.println(getESUM(voltage_load_V_avg, "V", SERIAL_PRECISION));
//    Serial.print("|Min Votage    "); Serial.println(getESUM(voltage_load_V_min, "V", SERIAL_PRECISION));
//    Serial.print("Shunt Current  "); Serial.println(getESUM(current_A, "A", SERIAL_PRECISION));
//    Serial.print("| Max Current  "); Serial.println(getESUM(current_A_max, "A", SERIAL_PRECISION));
//    Serial.print("| Avg Current  "); Serial.println(getESUM(current_A_avg, "A", SERIAL_PRECISION));
//    Serial.print("| Min Current  "); Serial.println(getESUM(current_A_min, "A", SERIAL_PRECISION));
//    Serial.print("Current        "); Serial.println(getESUM(amp_hours, "AH", SERIAL_PRECISION));
//    Serial.print("Power          "); Serial.println(getESUM(watt_hours, "WH", SERIAL_PRECISION));
//    Serial.print("Power Calc.    "); Serial.println(getESUM(voltage_bus_V * current_A, "W", SERIAL_PRECISION));
//    Serial.print("Power Register "); Serial.println(getESUM(power_W, "W", SERIAL_PRECISION));
    
    // Update Last Millis Time
    millis_last_update_serial = millis_current;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - -
  
  // Display Clear
  display.clearDisplay();

//  // Display Screen Location
//  if (screen > 0) {
//    display.setCursor(SCREEN_WIDTH -7, SCREEN_HEIGHT -9);
//    display.print(screen);
//  }

  // Screen Options
  byte Y_ROW = 0;
  if (screen >= screens) { screen = 0; } // Reset Screen Count, really this is just to prevent overflow of the screens varrable.
  switch (screen % screens) {
    default: // Graphing -------------------------------
         
      // Display Volts & Amps
      displayValueUnit(voltage_load_V, "V", 2, (SCREEN_WIDTH / 2), 0, 0);
      displayValueUnit(current_A,      "A", 1, (SCREEN_WIDTH / 2), (SCREEN_WIDTH / 2), 0);
      
      // Update Display Graph & Serial Plotting
      if (millis_current - millis_last_update_graph > UPDATE_RATE_GRAPH_MS) {
        // Graph
        graphVotage.plot(voltage_load_V);
        graphCurrent.plot(current_A);
        
        // Update Last Millis Time
        millis_last_update_graph = millis_current;
      } else { 
        // Show Graph wo/ changed values
        graphVotage.plot();
        graphCurrent.plot();
      }
      
      break;
    case 1:  // Current/Max/Min Screen -----------------
    
      // Display Header
      display.setCursor(0, 0);
      display.print("Current / Max / Min");
    
      // Display Volts & Amps
      #define TEXT_OFFSET 12 // number of decial places to show
//      Y_ROW = 0;  display.setCursor(0, Y_ROW); display.print("#:"); displayValueUnit(voltage_load_V,     "V", 2, 0, TEXT_OFFSET, Y_ROW);  displayValueUnit(current_A,     "A", 1, 0, (SCREEN_WIDTH / 2) + TEXT_OFFSET, Y_ROW);
      Y_ROW = 8; display.setCursor(0, Y_ROW); display.print("+:"); displayValueUnit(voltage_load_V_max, "V", 2, 0, TEXT_OFFSET, Y_ROW);  displayValueUnit(current_A_max, "A", 1, 0, (SCREEN_WIDTH / 2) + TEXT_OFFSET, Y_ROW);
//      Y_ROW = 16;  display.setCursor(0, Y_ROW); display.print("~:"); displayValueUnit(voltage_load_V_avg,     "V", 2, 0, TEXT_OFFSET, Y_ROW);  displayValueUnit(current_A_avg,     "A", 1, 0, (SCREEN_WIDTH / 2) + TEXT_OFFSET, Y_ROW);
      Y_ROW = 16;  display.setCursor(0, Y_ROW); display.print("~:"); displayValueUnit(voltage_load_V_avg.get(),     "V", 2, 0, TEXT_OFFSET, Y_ROW);  displayValueUnit(current_A_avg.get(),     "A", 1, 0, (SCREEN_WIDTH / 2) + TEXT_OFFSET, Y_ROW);
      Y_ROW = 24; display.setCursor(0, Y_ROW); display.print("-:"); displayValueUnit(voltage_load_V_min, "V", 2, 0, TEXT_OFFSET, Y_ROW); displayValueUnit(current_A_min,  "A", 1, 0, (SCREEN_WIDTH / 2) + TEXT_OFFSET, Y_ROW);
      
      break;
    case 2:  // Stats Screen ---------------------------
    
      // Display Header
      display.setCursor(0, 0);
      display.print("Additional Stats");
    
      // Display Volts & Amps
      #define TEXT_OFFSET 0 // number of decial places to show
      Y_ROW = 8;  displayValueUnit(voltage_load_V,  "V", 2, 0, TEXT_OFFSET, Y_ROW);  displayValueUnit(current_A,                 "A", 1, 0, (SCREEN_WIDTH / 2) + (TEXT_OFFSET / 2), Y_ROW);
      Y_ROW = 16; displayValueUnit(voltage_bus_V,   "V", 2, 0, TEXT_OFFSET, Y_ROW);  displayValueUnit(voltage_bus_V * voltage_load_V_avg.get(), "W", 1, 0, (SCREEN_WIDTH / 2) + (TEXT_OFFSET / 2), Y_ROW);
      Y_ROW = 24; displayValueUnit(voltage_shunt_V, "V", 2, 0, TEXT_OFFSET, Y_ROW);  displayValueUnit(power_W,                   "W", 1, 0, (SCREEN_WIDTH / 2) + (TEXT_OFFSET / 2), Y_ROW);
      
      break;
    case 3:  // Battery Estimator Screen ---------------
    
      // Display Header
      display.setCursor(0, 0);
      display.print("Battery Estimator");

      // Display Battery Estimator
      #define TEXT_OFFSET 24 // number of decial places to show
      Y_ROW = 8;  display.setCursor(0, Y_ROW); display.print(" 1hr:"); displayValueUnit(current_A_avg.get(),      "AH", 1, 0, 30, Y_ROW); displayValueUnit(power_W_avg.get(),      "WH", 0, 0, (SCREEN_WIDTH / 2) + TEXT_OFFSET, Y_ROW);
      Y_ROW = 16; display.setCursor(0, Y_ROW); display.print("24hr:"); displayValueUnit(current_A_avg.get() * 24, "AH", 2, 0, 30, Y_ROW); displayValueUnit(power_W_avg.get() * 24, "WH", 0, 0, (SCREEN_WIDTH / 2) + TEXT_OFFSET, Y_ROW);
      
      // Calculate Runtime Timestamp
      unsigned long seconds_current = millis_current / 1000;
      byte run_seconds = seconds_current % 60;
      byte run_minutes = (seconds_current % 3600) / 60;
      byte run_hours   = seconds_current / 3600;
      char timestamp_buffer[14];

      // Display Runtime Timestamp
      sprintf(timestamp_buffer,"%02d:%02d:%02d", run_hours, run_minutes, run_seconds);
      Y_ROW = 24; display.setCursor(0, Y_ROW); display.print(timestamp_buffer); displayValueUnit(amp_hours, "AH", 1, 0, (SCREEN_WIDTH / 2), Y_ROW);
      
      break;
  }
  
  // Display Update
  display.display();

  // Delay
  delay(300);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// -- Function [ Button ISR ]  Button Interrupt Service Routine (ISR)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void BUTTON_ISR() { // ISR to be get called on button press/release
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();

  // If interrupts come faster than 100ms, assume it's a bounce and ignore.
  if (interruptTime - lastInterruptTime > 100) {
    // Increase Screen Index
    screen++;
  }

  // Keep track of when we were here last (no more than every 5ms)
  lastInterruptTime = interruptTime;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// -- Class Fuction [ Get ESUM ]  Get the Electrical Standard Units of Measure, at specified precision of Value
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
String getESUM(float value, char* value_unit, byte precision) {
  
  // Varables
  float value_absolute = fabs(value);
  float value_unit_measure      = value;
  char  value_unit_measure_char = '\0';
  
  // Adjust Standard Electrical Units of Measure
  // https://www.electronics-tutorials.ws/dccircuits/dcp_3.html
  if (value_absolute > pow(10, 9)) {
    value_unit_measure     /= pow(10, 9);
    value_unit_measure_char = 'G';
  } else if (value_absolute >= pow(10, 6)) {
    value_unit_measure     /= pow(10, 6);
    value_unit_measure_char = 'M';
  } else if (value_absolute >= pow(10, 3)) {
    value_unit_measure     /= pow(10, 3);
    value_unit_measure_char = 'k';
  } else if (value_absolute >= 1) {
  //value_unit_measure      = value;
  //value_unit_measure_char = '\0';
  } else {
    value_unit_measure      /= pow(10, -3);
    value_unit_measure_char = 'm';
  } 

  // Formate Value to String
  String output_str = String(value_unit_measure, precision);

  // Add to String the Standard Electrical Units of Measure
  if (value_unit_measure_char != '\0') { output_str += value_unit_measure_char; }
  output_str += value_unit;

//  // Add Space if Not Negative Number
//  if (value >= 0) {
//  output_str = ' ' + output_str; // Leave room for 0 when value is below 0.
//  }

  // Return String
  return output_str;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// -- Class Fuction [ Display Value w/ Unit ]  Display value adjusted to best Standard Electrical Units of Measure, at specified precision
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void displayValueUnit(float value, char* value_unit, byte precision, byte width, byte x, byte y) {
  
  // Formate Value to String
  String output_str = getESUM(value, value_unit, precision);
  
  // Center Justify Output, otherwise output will be Left Justified
  if (width > 0) {
    // Center Text
    // https://forum.arduino.cc/t/adafruit-oled-how-to-center-text/617181/5
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(output_str, x, y, &x1, &y1, &w, &h); //calc width of new string
    display.setCursor((x + (width / 2)) - w / 2, y);
  } else {
    // Left Justify, basically start outputing at cordinates
    display.setCursor(x, y);
  }

  // Display the Value & Standard Electrical Units of Measure String
  display.print(output_str);
}


//// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//// -- Class Constructor [ Display Graph ]  Display line graph in area.
//// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//LineGraph::LineGraph(byte x_0, byte y_0, byte x_1_width, byte y_1_height, float graph_value_min, float graph_value_max, Adafruit_SSD1306 &disp) {
//  config(x_0, y_0, x_1_width, y_1_height, graph_value_min, graph_value_max, disp);
//}
//
//
//// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//// -- Class Constructor [ Display Graph ]  Display line graph in area.
//// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//void LineGraph::config(byte x_0, byte y_0, byte x_1_width, byte y_1_height, float graph_value_min, float graph_value_max, Adafruit_SSD1306 &disp) {
//  // initialize All Variables
//  x = x_0;
//  y = y_0;
//  width  = x_1_width;
//  height = y_1_height;
//  
//  // Pass Display Class 
//  display = &disp;
//  
//  // set min and max values expected for graph
//  min = graph_value_min;
//  max = graph_value_max;
//  
//  // Set Array Size, zeroed
//  plot_data = new int8_t[width] { 0 };
//  
//  // Set Array Initial Value
//  for (int i = width; i > 0; --i) {
//    plot_data[i] = FLT_MAX;
//  }
//}
//
//
//
//// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//// -- Class Fuction [ Display Graph ]  Display line graph in area, just draw last plot.
//// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//void LineGraph::plot() {
//  // Plot Data
//  LineGraph::plot(FLT_MAX);
//}
//
//// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//// -- Class Fuction [ Display Graph ]  Display line graph in area, just draw last plot.
//// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//void LineGraph::plot(float graph_value_min, float graph_value_max) {
//  // Plot Data
//  LineGraph::plot(FLT_MAX, graph_value_min, graph_value_max);
//}
//
//// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//// -- Class Fuction [ Display Graph ]  Display line graph in area.
//// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//void LineGraph::plot(float value, float graph_value_min, float graph_value_max) {
//
//  // Clear if net Min/Max
//  if (min != graph_value_min || max != graph_value_max) {
//    // set min and max values expected for graph
//    min = graph_value_min;
//    max = graph_value_max;
//    
//    // Set Array Initial Value (Clear)
//    for (int i = width; i > 0; --i) {
//      plot_data[i] = FLT_MAX;
//    }
//  }
//
//  // Plot Data
//  LineGraph::plot(value);
//}
//
//// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//// -- Class Fuction [ Display Graph ]  Display line graph in area.
//// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//void LineGraph::plot(float value) {
//  // Offset Varables
//  int8_t offset_frame = 6;
//  int8_t offset = 0;
//  
//  // Check if value out of range, or null value.
//  if (value != FLT_MAX) {
//    // Get latest reading
//    //plot_data[0] = mapfloat(value, min, max, 0, height);
//    plot_data[0] = mapfloat(value, min, max, 0 +offset_frame, height -offset_frame);
//  }
//
//  // Set Offset
//  if ( plot_data[0] > ((byte)height - offset_frame) ) {
//    // Offset Max
//    offset = (plot_data[0] - height) + offset_frame;
//  } else if ( plot_data[0] < 0 + offset_frame) {
//    // Offset Min
//    offset = plot_data[0] - offset_frame;
//  }
//
//  // Debug
//  #ifdef DEBUG
//  Serial.println("0 | x:" + (String)x + " y:" + (String)y +  " | width:" + width + " height:" + height + " | " + (String)plot_data[0]+  + " | offset:" + (String)offset + " | mapped:" + (String)value_mapped + "  x: " + (String)(0 + x) + ", y:" + (String)(  ((height + y) - (map(plot_data[0], min, max, 0, height)))   ));
//
//  // Display Screen Location
//  if (plot_data[0] < 0)
//    display->setCursor(width -30, height -9);
//  else
//    display->setCursor(width -24, height -9);
//  display->setTextSize(1);
//  display->println(plot_data[0]);
//  #endif
//
//  // Line Graph
//  byte plot_data_mapped = 0;
//  for( byte i = 0; i < width; i++ ) {
//    plot_data_mapped = plot_data[i] - offset;
//
//    // Plot & Display Graph
//    if ( plot_data_mapped <= height && plot_data_mapped >= 0 ) {
//      // 1 | x0:0 y0:24 x1:47 y1:63 | width:47 height:39 | 40.49  x: 1, y:54
//      #ifdef DEBUG
//      Serial.println((String)i + " | " + "x:" + (String)x + " y:" + (String)y +  " | width:" + width + " height:" + height + " | " + (String)plot_data[i]+ " | mapped:" + (String)mapfloat(plot_data[i], min, max, 0, height) + "  x: " + (String)(i + x) + ", y:" + (String)(  ((height + y) - (map(plot_data[i], min, max, 0, height)))   ));
//      #endif
//      
//      // Display Pixel
//      display->drawPixel(i + x, (height + y) - constrain(plot_data_mapped, 0, height), WHITE);
//    }
//  }
//  
//  // Side Lines
//  for( byte i = 0; i < height; i++ ) {
//    if ( !((i - offset) % 8) ) {
//      display->drawPixel(x, y + i, WHITE);
//      display->drawPixel(x + width -1, y + i, WHITE);
//    }
//  }
//  
//  // Base Lines
//  for( byte i = 0; i < width; i++ ) {
//    if ( !((i - graph_progress) % 16) ) {
//      display->drawPixel(i + x, height -1 + y, WHITE);
//    }
//  }
//
//  // Check if value out of range, or null value.
//  if (value != FLT_MAX) {
//    // Shift Plotted Graph
//    for( byte i = width; i > 0; i-- ) {
//      plot_data[i] = plot_data[i -1];
//      // Serial.println((String)i + " | " + "plot:" + plot_data[i]);
//    }
//
//    // Shift for Base Lines
//    graph_progress++;
//  }
//}
//
//
//float LineGraph::mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
//{
//return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
//}
