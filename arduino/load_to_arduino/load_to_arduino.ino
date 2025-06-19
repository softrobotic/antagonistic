/* Running code for Antagonistic bench v2.0 with pressure capturing

Diogo Fonseca, 2025

Notes: frequency stayed at 10Hz in both an Arduino Uno and Due. Baudrate is not the bottleneck.

Pinout
  2 - encoder phase A
  3 - encoder phase B
  4 - DOUT left cell
  5 - CLK left cell
  6 - DOUT right cell
  7 - CLK right cell
  A0 - analogue pressure sensor left
  A1 - analogue pressure sensor right
*/

// useful flags
bool next_flag = true; // if true, need to send 'n' between each data point
bool right_act = false; // true if right actuator is present
bool left_act = true; // true if left actuator is present

// libraries
#include <Encoder.h>
#include "HX711.h"

// load cell pins
#define DOUTL  4
#define CLKL  5
#define DOUTR  6
#define CLKR  7

// Variable declaration
int encoder_pos;
float force_left, force_right, pressure_left, pressure_right, volt;
float left_calibration = 462; //459.5
float right_calibration = 462; //459.5


// objects
Encoder myEnc(2, 3);  // for best performance, use interruptor pins. In the Uno, it's pins 2 and 3
HX711 left_cell;
HX711 right_cell;

void setup() {
  // handshake with serial client
  handshake();
  // configure load cells
    left_cell.begin(DOUTL, CLKL);
    left_cell.set_scale(left_calibration);
    left_cell.tare(); //Tare to 0

    right_cell.begin(DOUTR, CLKR);
    right_cell.set_scale(right_calibration);
    right_cell.tare(); //Tare to 0
}

void loop() {

  // get data from encoder
  encoder_pos = myEnc.read(); // i think 360deg = 4000steps

  // get data form load cells
  if (right_act) {
    force_right = right_cell.get_units();
  }
  if (left_act) {
    force_left = left_cell.get_units();
    delay(500);
  }
   
  // get data from pressure sensors
        if (right_act) {
      volt=analogRead(A1);
      pressure_right=(volt-0.1*1023)*(60/(0.8*1023));   //PSI
      pressure_right=pressure_right*6.89475729; //conversion to kPa
    }
    
    if (left_act) {
      volt=analogRead(A0);
      pressure_left=(volt-0.1*1023)*(60/(0.8*1023));   //PSI
      pressure_left=pressure_left*6.89475729; //conversion to kPa
    }

  // send data over serial
  Serial.print(int(force_left));
  Serial.print(",");
  Serial.print(int(force_right));
  Serial.print(",");
  Serial.print(int(encoder_pos));
  Serial.print(",");
  Serial.print(int(pressure_left));
  Serial.print(",");
  Serial.println(int(pressure_right));
  //wait for next request from client
  if (next_flag) {
    char n = 'a';
    while (n != 'n') {
      //wait response from PC
      n = Serial.read();
    }
  }
}

void handshake() {
  // wait for serial port. Needed for native USB port only
  while (!Serial) {
    ;
  }
  Serial.begin(115200);
  char a = 'b';
  while (a != 'a') {
    //acknowledgement routine
    Serial.println("a");
    //wait response from PC
    Serial.flush();
    a = Serial.read();
  }
}