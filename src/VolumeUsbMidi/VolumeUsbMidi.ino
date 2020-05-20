#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

static const int kBufferSize = 8;

static const int kNoteOffKey = WIO_KEY_B;

static bool is_note_cleared = false;

static uint32_t last_mills = 0;

static int note_buffer_pos = 0;

static int note_buffer[kBufferSize] = {
  0, 0, 0, 0, 0, 0, 0, 0
};


void initMidi() {
  MIDI.begin(MIDI_CHANNEL_OMNI);


  while( !USBDevice.mounted() ) delay(1);

  is_note_cleared = digitalRead(kNoteOffKey) == LOW ? true : false;
}


uint8_t getNoteValue() {
  int value = analogRead(A0);
  value = (int)(value * 64.0 / 1023.0 + 0.5) + 32;
  if (value > 95) value = 95;
  return value;
}

void clearNote() {
  if (is_note_cleared) return;
  int count = note_buffer_pos < kBufferSize ? note_buffer_pos : kBufferSize;

  for (int i = 0; i < count; i++) {
    MIDI.sendNoteOff(note_buffer[i], 0, 1);
    delay(5);
    note_buffer[i] = 0;
  }

  note_buffer_pos = 0;
  is_note_cleared = true;
}

void addNote(uint8_t note) {
  int pos = note_buffer_pos % kBufferSize;
  if (note_buffer[pos] > 0) {
    MIDI.sendNoteOff(note_buffer[pos], 0, 1);
  }

  MIDI.sendNoteOn(note, 64, 1);
  note_buffer[pos] = note;

  note_buffer_pos++;

  if (note_buffer_pos >= kBufferSize * 2) {
    note_buffer_pos = kBufferSize;
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(kNoteOffKey, INPUT_PULLUP);

  initMidi();
}

void loop() {
  if (digitalRead(kNoteOffKey) == LOW) {
    clearNote();
  } else {
    uint32_t c = millis();
    if (last_mills == 0 || c - last_mills > 50) {
      last_mills = c;
      is_note_cleared = false;
  
      uint8_t value = getNoteValue();
      addNote(value);
    }
  }

  MIDI.read();
}
