#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

static const int kBufferSize = 8;

static const int kNoteOffKey = 8;
static const int kPitch = A7;
static const int kNoteNum = A6;

static int beforeNote = -1;

static uint32_t last_millis = 0;
static bool isNoteOff = true;

void initPeripheral() {
  pinMode(kNoteOffKey, INPUT_PULLUP);
  analogReadResolution(14);
}

void initMidi() {
  MIDI.begin(MIDI_CHANNEL_OMNI);


  while( !USBDevice.mounted() ) delay(1);
  isNoteOff = true;
}


int getNoteValue() {
  int value = analogRead(kNoteNum);
  return (value >> 8) + 32;
}

int getPitchValue() {
  return analogRead(kPitch) - 8192;
}

void clearNote() {
  if (beforeNote < 0) return;
//  MIDI.sendNoteOn(beforeNote, 0, 1);
  MIDI.sendNoteOff(beforeNote, 64, 1);
  //beforeNote = -1;
  isNoteOff = true;
}

void sendValue(int note, int pitch) {
  if (!isNoteOff) {
    MIDI.sendNoteOff(beforeNote, 64, 1);
  }
  MIDI.sendPitchBend(pitch, 1);
  //if (isNoteOff) {
    MIDI.sendNoteOn(note, 64, 1);
    isNoteOff = false;
  //}
  beforeNote = note;
  
}

void setup() {
  initPeripheral();
  initMidi();
}

void loop() {
  uint32_t c = millis();
  if (last_millis > 0 && c - last_millis < 100) return;
  last_millis = c;
  if (digitalRead(kNoteOffKey) == LOW) {
    clearNote();
  } else  {
    sendValue(getNoteValue(), getPitchValue());
  }

  MIDI.read();
}
