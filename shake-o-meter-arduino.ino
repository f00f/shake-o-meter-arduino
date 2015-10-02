// ------------
//  Arduino firmware to detect vibrations of a laundry machine - and when it's finished.
//  License: MIT
//  Github: https://github.com/f00f/shake-o-meter-arduino
// ------------

#include <Button.h>
#include <SPI.h> // must be included manually to use SD library
#include <SD.h> // include in main file, otherwise it will not work

#include "DataRecorder.h";


//////////////////////////////////////
// Software Configuration
//////////////////////////////////////

//! Sampling delay in [ms]
const int SamplingDelay = 100;
//! Recording duration in [s]
const int RecordingDuration = 10/* * 60 * 60*/;

//! sensor reading when in rest
const int SensorIdleValue = 511;


//////////////////////////////////////
// Hardware Configuration (i.e. Pins)
//////////////////////////////////////

const int activityLED = 8;
const int sensorPower = 9;// pin outs have a more stable voltage than the regular power supply
const int sensor = A0;

// SPI pins
const int MY_CS_PIN   = 10;

//create a Button object at pin 4
/*
|| Wiring:
|| GND -----/ ------ pin 4
*/
Button buttonStartRecording = Button(4, INPUT_PULLUP);
Button buttonStartSensing = Button(5, INPUT_PULLUP);
Button buttonReInit = Button(6, INPUT_PULLUP);

//////////////////////////////////////
// End of Configuration
//////////////////////////////////////


// Application states
const int AS_INVALID = 0;   // invalid state. should never be entered.
const int AS_ERROR = 1;     // some error occurred
const int AS_INIT = 2;      // after reset button was pressed. SD card init is repeated (amongst other things)
const int AS_IDLE = 3;      // idle. after start, and after leaving any of the other modes.
const int AS_RECORDING = 4; // acquiring sensor data and recording to disc. DataRecorder determines when this is finished.
const int AS_SENSING = 5;   // sensing for vibrations. active detection.
int app_state;

// Error codes
const int E_OK = 0;
const int E_INIT_SD = 1;
int err_state;

void BlinkActivityLED(int _delay);
inline void BlinkActivityLED() { BlinkActivityLED(120); }
inline int ReadSensor() { return analogRead(sensor); }
inline int GetSensorAmplitude(int sensor_value) { return abs(SensorIdleValue - sensor_value); }
#if 0
inline void PublishSample(int amp);
inline void StoreSample(int amp);
inline bool AnalyzeWindow();
inline void PublishStateChange(const char* newState);
#endif

DataRecorder dataRecorder;


void setup() {
  app_state = AS_IDLE;
  err_state = E_OK;

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}

  // enable power for the sensor (pin outs are more stable than the regular power supply)
  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, HIGH);

  pinMode(sensor, INPUT);

  pinMode(activityLED, OUTPUT);

  Serial.println("app: warming up");

  // read some values until sensor has stabilized
  delay(200);
  for (int i = 0; i < 5; i++) {
      ReadSensor();
      delay(200);
  }

  if (!dataRecorder.setup(MY_CS_PIN)) {
    app_state = AS_ERROR;
    err_state = E_INIT_SD;
  }

  BlinkActivityLED();
  delay(200);
  for(int i = 0; i < 2; i++){
    BlinkActivityLED();
  }
}


void loop() {
  int sensor_value = ReadSensor();

  if (buttonReInit.isPressed()) {
    app_state = AS_INIT;
    Serial.println("app: button pressed, resetting app");
  }
  
  switch (app_state) {
    case AS_ERROR:
      delay(500);
      for(int i = 0; i < err_state; i++){
        BlinkActivityLED();
      }
      break;

    case AS_INIT:
      digitalWrite(activityLED, LOW);
      setup();
      break;

    case AS_IDLE:
      digitalWrite(activityLED, LOW);
      //Serial.println("app: idle [sensor value = " + String(sensor_value) + "]");

      if (buttonStartRecording.isPressed()) {
        Serial.println("app: button pressed, start recording");
        app_state = AS_RECORDING;
        dataRecorder.start(RecordingDuration);
      } else if (buttonStartSensing.isPressed()) {
        Serial.println("app: button pressed, start sensing");
        app_state = AS_SENSING;
        //
      }
      break;

    case AS_RECORDING:
      digitalWrite(activityLED, HIGH);
      dataRecorder.record(sensor_value);

      if (dataRecorder.isFinished()) {
        Serial.println("app: recording finished, entering idle mode");
        app_state = AS_IDLE;
      }
      
      delay(SamplingDelay);
      break;

    case AS_SENSING:
      digitalWrite(activityLED, HIGH);
      Serial.println("sensing... [sensor_value = " + String(sensor_value) + "]");

      if (!buttonStartSensing.isPressed()) {
        Serial.println("app: sensing finished, entering idle mode");
        app_state = AS_IDLE;
      }

      delay(SamplingDelay);
      break;
  }

  delay(10);
}


void BlinkActivityLED(int _delay) {
  digitalWrite(activityLED, HIGH);
  delay(_delay);
  digitalWrite(activityLED, LOW);
  delay(_delay);
}

