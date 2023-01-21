

#define F_CPU 8000000

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include <stdint.h>

#include "freq.h"
#include "wave.h"

#define VOICE_BITS        3
#define KEY_BYTES         (2 * 1)

#define INSTRUMENT_COUNT  (sizeof(WAVE) / sizeof(WAVE[0]))
#define VOICE_COUNT       (1 << VOICE_BITS)

#define KEY_LATCH_PIN PB0
#define KEY_CLOCK_PIN PB1
#define FUN_INPUT_PIN PB2
#define KEY_INPUT_PIN PB3
#define AUDIO_OUT_PIN PB4

typedef struct Voice {
  volatile uint16_t timer;
  volatile uint16_t freq;
} Voice;

static volatile uint8_t instrument;
static Voice voices[VOICE_COUNT];
static uint8_t key_state[KEY_BYTES];

ISR(TIMER0_COMPA_vect)
{
  uint16_t mix = 0;
  for(uint8_t vi = 0; vi < VOICE_COUNT; vi++) {
    const uint16_t freq = voices[vi].freq;
    const uint16_t timer = voices[vi].timer;
    const uint8_t wave_index = timer >> 8;
    mix += WAVE[instrument][wave_index];
    voices[vi].timer = timer + freq;
  }
  OCR1B = (uint8_t)(mix >> VOICE_BITS);
}

static void press_key(uint8_t freq_index)
{
  for(uint8_t i = 0; i < VOICE_COUNT; i++) {
    if(voices[i].freq == 0) {
      voices[i].timer = 0;
      voices[i].freq = FREQUENCIES[freq_index];
      break;
    }
  }
}

static void release_key(uint8_t freq_index)
{
  const uint16_t freq = FREQUENCIES[freq_index];
  for(uint8_t i = 0; i < VOICE_COUNT; i++) {
    if(voices[i].freq == freq) {
      voices[i].freq = 0;
      break;
    }
  }
}

static void set_mode(uint8_t i)
{
  i -= 4; // First 4 notes are blank
  if(i < INSTRUMENT_COUNT) {
    instrument = i;
  }
}

static void scan()
{
  uint8_t freq_index = 68 - 4;

  // Pull the load enable high to hold values.
  PORTB |= 1 << KEY_LATCH_PIN;

  for(uint8_t i = 0; i < KEY_BYTES; i++) {
    const uint8_t control = 0; //(~PINB) & (1 << FUN_INPUT_PIN);
    uint8_t state = key_state[i];
    for(uint8_t mask = 0x80; mask; mask >>= 1) {

      // Bring clock low so we shift on the next clock.
      PORTB &= ~(1 << KEY_CLOCK_PIN);

      const uint8_t input = (~PINB) & (1 << KEY_INPUT_PIN);
      if(control) {
        if(input) set_mode(freq_index);
      } else {
        const uint8_t current = state & mask;
        if(input && !current) {
          press_key(freq_index);
          state |= mask;
        } else if(!input && current) {
          release_key(freq_index);
          state &= ~mask;
        }
      }

      // Pull clock high to cause a shift.
      PORTB |= 1 << KEY_CLOCK_PIN;

      freq_index += 1;
    }
    if(!control) {
      key_state[i] = state;
    }
  }

  // Pull load enable low to load new values.
  PORTB &= ~(1 << KEY_LATCH_PIN);
}

static void init()
{
  uint8_t i;
  for(i = 0; i < VOICE_COUNT; i++) {
    voices[i].freq = 0;
  }
  for(i = 0; i < KEY_BYTES; i++) {
    key_state[i] = 0;
  }
  instrument = 0;
}

int main()
{
  cli();

  // Clock prescalar
  // Default is 1MHz, switch to 8MHz
  CLKPR = 1 << CLKPCE;
  CLKPR = 0;

  // Ports
  //  Port 0 = key latch out
  //  Port 1 = key clock out
  //  Port 2 = control (input)
  //  Port 3 = key in
  //  Port 4 = audio out

  // Set input pullups and output state.
  PORTB = (1 << FUN_INPUT_PIN);

  // Set outputs.
  DDRB = (1 << KEY_LATCH_PIN)
       | (1 << KEY_CLOCK_PIN)
       | (1 << AUDIO_OUT_PIN);

  // PWM timer setup (timer1)
  PLLCSR = (1 << PCKE) | (1 << PLLE);   // 64MHz PLL source for timer1
  TIMSK = 0;                            // Timer interrupts off
  TCCR1 = 1 << CS10;                    // 1:1 prescalar
  GTCCR = (1 << PWM1B) | (2 << COM1B0); // PWM B, Clear on match
  OCR1C = 255;                          // Divide by 256 -> 250 kHz
  OCR1B = 0;                            // Duty cycle
  
  // Tone generator timer setup (timer0)
  // Clock is 8MHz, set prescaler to 8 and divide by 80 to get 12.5kHz
  TCCR0A = 3 << WGM00;                  // Fast PWM
  TCCR0B = (1 << WGM02) | (2 << CS00);  // Waveform, divide by 8
  OCR0A = 80 - 1;                       // Divide by 80.
  TIMSK = 1 << OCIE0A;                  // Enable compare match interrupt

  init();

  sei();

#define TOP 100
/*
  for(;;) {
    init();
    uint8_t i;
    for(instrument = 0; instrument <= 3; instrument++) {
      uint8_t note;
      for(note = 4; note < 112; note++) {
        voices[0].freq = FREQUENCIES[note];
        if(FREQUENCIES[note]) {
          for(i = 0; i < TOP; i++) {
            scan();
          }
          voices[0].freq = 0;
          for(i = 0; i < TOP / 4; i++) {
            scan();
          }
        }
      }
voices[0].freq = FREQUENCIES[84];
      for(i = 0; i < TOP; i++) {
        scan();
        scan();
        scan();
      }
voices[1].freq = FREQUENCIES[88];
      for(i = 0; i < TOP; i++) {
        scan();
        scan();
        scan();
      }
voices[2].freq = FREQUENCIES[91];
      for(i = 0; i < TOP; i++) {
        scan();
        scan();
        scan();
        scan();
        scan();
      }
      for(i = 0; i < VOICE_COUNT; i++) voices[i].freq = 0;
    }
  }
*/
  for(;;) scan();

  return 0;
}

