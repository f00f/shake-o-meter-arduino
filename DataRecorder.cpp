// ------------
//  Arduino firmware to detect vibrations of a laundry machine - and when it's finished.
//  License: MIT
//  Github: https://github.com/f00f/shake-o-meter-arduino
// ------------

#include "DataRecorder.h";

bool DataRecorder::setup(int cs_pin) {
  Serial.println("DataRecorder::setup");
  
  Serial.print("Initializing SD card...");
  Serial.print(" [cs_pin = " + String(cs_pin) + "] ");

#if 1
  // see if the card is present and can be initialized:
  if (!SD.begin(cs_pin)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return false;
  }
  Serial.println("card initialized.");
#else
  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, cs_pin)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return false;
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }
#endif
  return true;
}

void DataRecorder::start(int seconds) {
  unsigned long now;
  Serial.println("DataRecorder: starting recording for " + String(seconds) + " s");
  do {
    now = millis();
    String nowStr = String(now);
    int len = nowStr.length();
    if (len > 6) {
      nowStr = nowStr.substring(len - 6);
      len = 6;
    }
    while (len < 6) {
      nowStr = "0" + nowStr;
      ++len;
    }
    filename = "wm" + nowStr + ".txt";
    delay(1);
  } while(SD.exists((char*)filename.c_str()));
  Serial.println("recording to " + filename);

  tsFinished = now + (unsigned long)(seconds * 1000);
}

bool DataRecorder::record(int sensor_value) {
  Serial.println("DataRecorder: recording value " + String(sensor_value));
  
  File dataFile = SD.open(filename.c_str(), FILE_WRITE);
  // if the file is available, write to it:
  if (!dataFile) {
    // if the file isn't open, pop up an error:
    Serial.println("error opening " + filename);
    return false;
  }

  dataFile.println(String(millis()) + '\t' + String(sensor_value));
  dataFile.close();
  return true;
}

