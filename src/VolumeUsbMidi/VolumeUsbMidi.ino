#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

static const int kNoteChannel = 1;
static const int kFootSwitch = 8;
static const int kPitch = A5;
static const int kNoteNum = A6;
static const int kNoteVel = A7;
static const int kNoteType = 0;
static const int kNoteOutNumBase = 60;
static const int kNoteOutWidth = 40;
static const uint32_t kUpdateSpanMillisec = 50;
static const int kVelBase = 64;
static const int kVelWidth = 40;

static int beforeNote = -1;
static int beforeVel = -1;
static uint32_t last_millis = 0;
static bool isNoteOff = true;
static bool forceUpdate = false;

static inline bool isNoteUpdate() {
  return digitalRead(kNoteType) == LOW;
}

static inline bool isFootSwitchOn() {
  return digitalRead(kFootSwitch) == LOW;
}

void initPeripheral() {
  pinMode(kNoteType, INPUT_PULLUP);
  pinMode(kFootSwitch, INPUT_PULLUP);
  analogReadResolution(14);
}

void initMidi() {
  MIDI.begin(MIDI_CHANNEL_OMNI);


  while( !USBDevice.mounted() ) delay(1);
  isNoteOff = true;
}

int getNoteValue() {
  double value = (double)(analogRead(kNoteNum) >> 2) - 2048;
  return kNoteOutNumBase + (int) (value * kNoteOutWidth / 2048 + 0.5);
}

int getPitchValue() {
  return analogRead(kPitch) - 8192;
}

int getNoteVel() {
  double value = (double)(analogRead(kNoteVel) >> 2) - 2048;
  return kVelBase + (int) (value * kVelWidth / 2048 + 0.5);
}

void clearNote() {
  if (beforeNote < 0) return;
  MIDI.sendNoteOff(beforeNote, beforeVel, kNoteChannel);
  isNoteOff = true;
}

void sendValue(int note, int pitch, int vel, bool isForce) {
  if (!isNoteOff && beforeNote >= 0 && (isForce || forceUpdate || note != beforeNote || vel != beforeVel)) {
    MIDI.sendNoteOff(beforeNote, beforeVel, kNoteChannel);
  }
  MIDI.sendPitchBend(pitch, kNoteChannel);
  if (isForce || forceUpdate || note != beforeNote || vel != beforeVel) {
    MIDI.sendNoteOn(note, vel, kNoteChannel);
  }
  isNoteOff = false;
  beforeNote = note;
  beforeVel = vel;
  forceUpdate = isForce;
}

void setup() {
  initPeripheral();
  initMidi();
}

void loop() {
  if (isFootSwitchOn()) {
    clearNote();
  }

  int m = millis();
  if (!isNoteOff && last_millis > 0 && (m - last_millis) < kUpdateSpanMillisec) {
    return;
  }

  last_millis = m;
  sendValue(getNoteValue(), getPitchValue(), getNoteVel(), isNoteUpdate());

  MIDI.read();
}
