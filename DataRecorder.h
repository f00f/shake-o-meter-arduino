// ------------
//  Arduino firmware to detect vibrations of a laundry machine - and when it's finished.
//  License: MIT
//  Github: https://github.com/f00f/shake-o-meter-arduino
// ------------

#if !defined(DataRecorder_h)
#define DataRecorder_h

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <SD.h> // must also be included in main .ino file

class DataRecorder
{
  public:
    bool setup(int cs_pin);
    void start(int seconds);
    bool record(int sensor_value);
    inline bool isFinished() { return millis() > tsFinished; }
    
  private:
    unsigned long tsFinished;
    Sd2Card card;
    String filename;
};

#endif //DataRecorder_h
