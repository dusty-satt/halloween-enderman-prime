#include <Time.h>
#include <TimeLib.h>

#include <SPI.h>
#include <SD.h>
#include <arduino.h>
#include "TMRpcm.h" 
TMRpcm audioPlayer;

// pins
int sdCardReaderCSPin = 3;
int amplifierPin = 10;
int motionSensorPin = 2;

bool motionSensorIsReady = false;
bool setupSuccess = true;
int motionSensorDelay = 75; // 75 second delay before the PIR can be used
int motionSensorStartTime;
int timestamp;
int aggroCooldownExpiration;
int idleCooldownExpiration;
bool isJawOpen = false; // Define the starting position of the jaw.  False = closed.
int jawMotorTurnDirectionMultiplier = 1; // + or -, to determine the direction the motor will turn
const char IDLE_SOUND[] = "idle.wav";
const char AGGRO_SOUND[] = "aggro.wav";

// How long of a delay should there be before the motion sensor can trigger aggro again?
//int aggroDelay = 3; // 36 seconds (16 seconds for the audio + 20 second delay)

// How long of a delay should there be before the idle event can happen after aggro has started?
// 16 seconds is the length of the aggro sound, after that, we can close the jaw, then resume the idle sounds
int idleDelay = 16;

void setup() {
  // Initialize Serial Port for Debugging
  Serial.begin(9600);
  
  // Setup SD Card Reader for audio reading
//  if (!SD.begin(sdCardReaderCSPin)) {
//    Serial.println("Fatal Error: The SD Card Reader Failed to begin.");
//    setupSuccess = false;
//    return;
//  }
  
  // Setup Amplifier and Audio
  audioPlayer.speakerPin = amplifierPin;
  audioPlayer.setVolume(2); // temporarily set at 2 for testing - buttons will control this later
  audioPlayer.quality(1); // 0 = normal, 1 = 2x Oversampling

  // Setup PIR MotionSensor
  pinMode(motionSensorPin, INPUT);

  // Set the time of day
  timestamp = now();

  // Set the default expiration time for aggro event delays
  aggroCooldownExpiration = timestamp + motionSensorDelay;
  motionSensorStartTime = aggroCooldownExpiration;
  
  // Set the default expiration time for idle event delays
  idleCooldownExpiration = timestamp;

  // Define the starting position of the jaw.  False = closed.
  bool isJawOpen = false;
  int jawMotorTurnDirectionMultiplier = 1;
}

void loop() {
  // Don't do anything if there was a critical failure during setup
  if (!setupSuccess) {
    return;
  }
  
  // Update our timestamp value
  timestamp = now();

  // Debugging - motion sensor is first ready
  if (timestamp == motionSensorStartTime && motionSensorStartTime != false) {
    // Disengage this by changing the int to false so the message doesn't spam
    motionSensorStartTime = false;
    Serial.println("Motion Sensor Initialization Complete.");
  }
  
  // When sensor detects motion, getAngry - but only if the cooldown has expired
  int motionSensorState = digitalRead(motionSensorPin);
  if (
    motionSensorState == HIGH &&
    timestamp > aggroCooldownExpiration
  ) {
    getAngry();
    Serial.println("Aggro behavior initiated. Cooldown until next aggro: (in seconds)");
    Serial.println(motionSensorDelay);
    aggroCooldownExpiration = timestamp + motionSensorDelay;
    idleCooldownExpiration = timestamp + idleDelay;
  }

  // Start idle if it's cooldown has expired
  if (timestamp > idleCooldownExpiration) {
    idle();
  }
}

/**
 * Close the jaw, play the idle sound
 */
void idle() {
  // retract the jaw lever
  openJaw(false);
  // play idle sound
  if(!audioPlayer.play(IDLE_SOUND)) {
    Serial.println("idle.wav failed to play.");
    return;
  }
}

/**
 * Stop idle sounds, dramatically open the jaw, and play aggro sounds (moaning)
 */
void getAngry() {
  // pause idle sounds
  if(!audioPlayer.pause()) {
    Serial.println("audioPlayer failed to pause.");
    return;
  }
  
  // extend the jaw lever
  openJaw(true);
  
  // Play the aggro sounds 
  // TODO: (as the motor is turning, hopefully, otherwise code this to start playing before motor operation)
  if(!audioPlayer.play(AGGRO_SOUND)) {
    Serial.println("aggro.wav failed to play.");
    return;
  }
}

/**
 * Open/Close the Enderman jaw by turning the motor in the correct direction
 *
 * @param bool open true)  Open the jaw if it's not already open
 *                  false) Close the jaw if it's not already closed
 */
void openJaw(bool open) {
  // don't move the jaw if it's already in the requested position (open/closed)
  if (open == isJawOpen) {
    return;
  }

  // define the direction to turn the motor
  if (open) {
    // When the jaw is open, turn motor counter clockwise to close
    int jawMotorTurnDirectionMultiplier = -1; 
  } else {
    // When the jaw is closed, turn motor clockwise to open
    int jawMotorTurnDirectionMultiplier = 1;
  }

  // TODO ^ actually turn the motor...
}
