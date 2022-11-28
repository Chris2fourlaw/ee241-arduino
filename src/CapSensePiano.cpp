/*
  Capacitive-Touch Arduino Keyboard Piano

  Plays piano tones through a buzzer when the user taps touch-sensitive piano "keys"

  Created  18 May 2013
  Modified 23 May 2013
  by Tyler Crumpton and Nicholas Jones

  This code is released to the public domain. For information about the circuit,
  visit the Instructable tutorial at http://www.instructables.com/id/Capacitive-Touch-Arduino-Keyboard-Piano/
*/

#include <CapacitiveSensor.h>
#include "pitches.h"
#include <Arduino.h>

// Metal detector constants
#define MAG_FREQ_PIN 10    // Frequency from metal detector
#define FREQ_TRIGGER_PIN 7 // Output pin for the metal detector frequency change trigger

// CapacitiveSensor constants
#define COMMON_PIN 2      // The common 'send' pin for all keys
#define BUZZER_PIN A4     // The output pin for the piezo buzzer
#define NUM_OF_SAMPLES 10 // Higher number whens more delay but more consistent readings
#define CAP_THRESHOLD 150 // Capacitive reading that triggers a note (adjust to fit your needs)
#define NUM_OF_KEYS 4     // Number of keys that are on the keyboard
#define LED_PIN 8         // The LED is connected to pin 8
#define SEQUENCE_SIZE 5   // The number of notes in the sequence

// This macro creates a capacitance "key" sensor object for each key on the piano keyboard:
#define CS(Y) CapacitiveSensor(2, Y)

// Each key corresponds to a note, which are defined here
int notes[] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4};

// Defines the pins that the keys are connected to:
CapacitiveSensor keys[] = {CS(3), CS(4), CS(5), CS(6)};
int initialArray[SEQUENCE_SIZE] = {0, 1, 2, 3, 0};
size_t arrayIndex = 0;

unsigned long startTriggerMillis; // start time of the trigger
bool trigger = false;             // trigger for the frequency change
int avg_count = 0;                // counter for the average frequency
unsigned long sum_freq = 0;       // sum of the frequencies

void setup()
{
  // Turn off autocalibrate on all channels:
  for (int i = 0; i < 4; ++i)
  {
    keys[i].set_CS_AutocaL_Millis(0xFFFFFFFF);
  }

  // Set the buzzer as an output:
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(MAG_FREQ_PIN, INPUT);
  pinMode(FREQ_TRIGGER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(FREQ_TRIGGER_PIN, LOW);
  Serial.begin(9600);
}

void loop()
{
  /******************/
  /* Metal Detector */
  /******************/

  // Read the frequency from the metal detector 20 times and average it
  if (avg_count < 20)
  {
    // Get the frequency from the metal detector
    int pulseHigh = pulseIn(MAG_FREQ_PIN, HIGH);
    int pulseLow = pulseIn(MAG_FREQ_PIN, LOW);
    int pulseTotal = pulseHigh + pulseLow; // Time period of the pulse in microseconds
    double freq = 1000000 / pulseTotal;    // Frequency in Hz

    // Throw out readings that are too high or too low
    if (freq > 8000 && freq < 9500)
    {
      sum_freq += freq;
      avg_count++;
    }
  }
  else
  {
    // Calculate the average frequency
    double frequency = sum_freq / avg_count;

    // If the frequency is within the range of the trigger frequency, set the trigger to true
    if (frequency < 8900)
    {
      if (trigger == false)
      {
        startTriggerMillis = millis();
        trigger = true;
      }
    }
    // If the frequency is outside the range of the trigger frequency, set the trigger to false
    else if (frequency > 9000)
    {
      trigger = false;
      digitalWrite(FREQ_TRIGGER_PIN, LOW);
    }

    // Reset the frequency variables
    sum_freq = 0;
    avg_count = 0;
  }

  // If the trigger is true, we have detected a metal object.
  // The trigger must be true for 0.1 seconds before the pin is set to high
  if (trigger && millis() - startTriggerMillis > 100)
  {
    digitalWrite(FREQ_TRIGGER_PIN, HIGH);
  }

  /******************/
  /* Capacitive Key */
  /******************/

  // Loop through each key:
  for (int i = 0; i < 4; ++i)
  {
    // If the capacitance reading is greater than the threshold, play the note:
    if (keys[i].capacitiveSensor(NUM_OF_SAMPLES) > CAP_THRESHOLD)
    {
      tone(BUZZER_PIN, notes[i]); // Plays the note corresponding to the key pressed
      delay(500);
      noTone(BUZZER_PIN);

      // If the key matches the next key in the sequence, increment the index:
      if (i == initialArray[arrayIndex])
      {
        arrayIndex++;

        // If the index is equal to the length of the sequence, the sequence was completed:
        if (arrayIndex == SEQUENCE_SIZE)
        {
          digitalWrite(LED_PIN, HIGH);
          delay(10000);
          digitalWrite(LED_PIN, LOW);
          arrayIndex = 0;
          break;
        }
      }
      else
      {
        arrayIndex = 0;
      }
    }
  }
}
