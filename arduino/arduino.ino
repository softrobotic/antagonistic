/* Running code for Antagonistic bench with support for different operating modes.

Modes are selected by changing the "next" signalling message received by the serial client.
Each sensor is identified by order:
sensor 1 - Left Load Cell
sensor 2 - Right Load Cell
sensor 3 - Encoder
sensor 4 - Left Pressure Sensor
sensor 5 - Right Pressure Sensor

Then a binary message can be used instead of 'n' to change which sensors will be active.
"11111" → all sensors on  
"10010" → only sensor 1 and 4 on  
"00000" → all off (maybe standby mode)

To ensure backwards compatibility, char 'n' will be interpreted as "all sensors active", equivalent to message "11111".

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
bool continuous_stream = false;  // if true, arduino will just stream all sensors continuously.

// sensor flags
bool sensor1_enabled = false;  // initialize as false, will be changed later.
bool sensor2_enabled = false;
bool sensor3_enabled = false;
bool sensor4_enabled = false;
bool sensor5_enabled = false;


// libraries
#include <Encoder.h>
#include "HX711.h"

// load cell pins
#define DOUTL 4
#define CLKL 5
#define DOUTR 6
#define CLKR 7

// Variable declaration
int encoder_pos;
float force_left, force_right, pressure_left, pressure_right, volt;
float left_calibration = 462;   //459.5
float right_calibration = 462;  //459.5

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
  left_cell.tare();  //Tare to 0

  right_cell.begin(DOUTR, CLKR);
  right_cell.set_scale(right_calibration);
  right_cell.tare();  //Tare to 0
}

void loop() {

  // get data form left load cell (sensor 1)
  if (sensor1_enabled) {
    force_left = left_cell.get_units();
  } else {
    force_left = -1;  // placeholder value when sensor is disabled
  }

  // get data form right load cell (sensor 2)
  if (sensor2_enabled) {
    force_right = right_cell.get_units();
  } else {
    force_right = -1;  // placeholder value when sensor is disabled
  }

  // get data from encoder (sensor 3)
  if (sensor3_enabled) {
    encoder_pos = myEnc.read();  // read encoder value. 360deg = 4000steps
  } else {
    encoder_pos = -1;  // placeholder value when sensor is disabled
  }

  // get data from left pressure sensor (sensor 4)
  if (sensor4_enabled) {
    pressure_left = readPressure_kPa(A0);
  } else {
    pressure_left = -1;  // placeholder value when sensor is disabled
  }

  // get data from right pressure sensor (sensor 5)
  if (sensor5_enabled) {
    pressure_right = readPressure_kPa(A1);
  } else {
    pressure_right = -1;  // placeholder value when sensor is disabled
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

  /*
  //wait for next request from client
  if (continuous_stream) {
    char n = 'a';
    while (n != 'n') {
      //wait response from PC
      n = Serial.read();
    }
  }

*/

  handleSerialInput();
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

float readPressure_kPa(int analogPin) {
  int volt = analogRead(analogPin);
  float pressure_psi = (volt - 0.1 * 1023) * (60.0 / (0.8 * 1023));  // convert to PSI
  return pressure_psi * 6.89475729;                                  // convert to kPa
}

void handleSerialInput() {
  if (continuous_stream) {
    // Enable all sensors for debug streaming
    sensor1_enabled = true;
    sensor2_enabled = true;
    sensor3_enabled = true;
    sensor4_enabled = true;
    sensor5_enabled = true;
    return;  // No need to wait for commands
  }

  // Wait until something is available
  while (Serial.available() == 0) {
    // Block here until a full command is received
  }

  String command = Serial.readStringUntil('\n');  // For example: "10101" or "n"

  if (command == "n") {
    // Legacy mode: all sensors on
    sensor1_enabled = true;
    sensor2_enabled = true;
    sensor3_enabled = true;
    sensor4_enabled = true;
    sensor5_enabled = true;
  } else if (command.length() >= 5) {
    // Expecting a 5-char binary string (e.g., "10010")
    sensor1_enabled = (command.charAt(0) == '1');
    sensor2_enabled = (command.charAt(1) == '1');
    sensor3_enabled = (command.charAt(2) == '1');
    sensor4_enabled = (command.charAt(3) == '1');
    sensor5_enabled = (command.charAt(4) == '1');
  } else {
    Serial.println("Invalid command received.");
  }
}
