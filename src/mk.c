
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "freq.h"
#include "wave.h"

#define VOICE_BITS        3   // 2^n voices at the same time
#define OCTAVE_COUNT      1   // Number of octaves
#define FIRST_OCTAVE      4   // First octave
#define INITIAL_TUNING    0   // Initial tuning offset

#define KEY_LATCH_PIN PB0
#define KEY_CLOCK_PIN PB1
#define FUN_INPUT_PIN PB2
#define KEY_INPUT_PIN PB3
#define AUDIO_OUT_PIN PB4

#define INSTRUMENT_COUNT  (sizeof(WAVE) / sizeof(WAVE[0]))
#define VOICE_COUNT       (1 << VOICE_BITS)
#define KEY_BYTES         (2 * OCTAVE_COUNT)  // 2 bytes per octave
#define FREQ_OFFSET       (FIRST_OCTAVE * 16) // 16 bits per octave

typedef struct Voice {
  volatile uint16_t timer;    // Index into the wave table
  volatile uint16_t freq;     // Increment for the timer (0 if off)
  uint8_t freq_index;         // Frequency index (0 if off)
} Voice;

static volatile uint8_t instrument;   // Selected wave table
static int8_t tuning;                 // Frequency adjustment
static uint8_t control_pressed;
static Voice voices[VOICE_COUNT];
static uint8_t key_state[KEY_BYTES];

ISR(TIMER0_COMPA_vect)
{
  uint16_t mix = 0;
  for(uint8_t vi = 0; vi < VOICE_COUNT; vi++) {
    const uint16_t timer = voices[vi].timer;
    const uint8_t wave_index = timer >> 8;
    mix += WAVE[instrument][wave_index];
    voices[vi].timer = voices[vi].freq + timer;
  }
  OCR1B = (uint8_t)(mix >> VOICE_BITS);
}

static void press_key(uint8_t freq_index)
{
  for(uint8_t i = 0; i < VOICE_COUNT; i++) {
    if(voices[i].freq_index == 0) {
      voices[i].timer = 0;
      voices[i].freq = FREQUENCIES[freq_index + FREQ_OFFSET] + tuning;
      voices[i].freq_index = freq_index;
      key_state[freq_index >> 3] |= 1 << (freq_index & 7);
      break;
    }
  }
}

static void release_key(uint8_t freq_index)
{
  for(uint8_t i = 0; i < VOICE_COUNT; i++) {
    if(voices[i].freq_index == freq_index) {
      voices[i].freq = 0;
      voices[i].freq_index = 0;
      key_state[freq_index >> 3] &= ~(1 << (freq_index & 7));
      break;
    }
  }
}

static void press_tuning_key()
{
  const uint8_t freq_index = 77 - FREQ_OFFSET; // A4

  // Release first in case the key is already pressed.
  // This way we get the new tuning and keep the state valid.
  release_key(freq_index);
  press_key(freq_index);
}

static void set_mode(uint8_t i)
{
  i -= 4;   // First 4 are blank.
  switch(i) {
  case 0: // C
    instrument = TRIANGLE_WAVE;
    break;
  case 2: // D
    instrument = SINE_WAVE;
    break;
  case 4: // E
    instrument = SQUARE_WAVE;
    break;
  case 5: // F
    instrument = SAWTOOTH_WAVE;
    break;
  case 1: // C# (flatten)
    if(tuning > -128) {
      tuning -= 1;
    }
    press_tuning_key();
    break;
  case 3: // D# (sharpen)
    if(tuning < 127) {
      tuning += 1;
    }
    press_tuning_key();
    break;
  case 6: // F# (reset tuning)
    tuning = INITIAL_TUNING;
    press_tuning_key();
    break;
  default:
    break;
  }
}

static void scan()
{
  uint8_t freq_index = 0;
  uint8_t has_press = 0;

  // Pull the load enable high to hold values.
  PORTB |= 1 << KEY_LATCH_PIN;

  const uint8_t control = (~PINB) & (1 << FUN_INPUT_PIN);
  for(uint8_t i = 0; i < KEY_BYTES; i++) {
    uint8_t state = key_state[i];
    for(uint8_t mask = 0x01; mask; mask <<= 1) {

      // Bring clock low so we shift on the next clock.
      PORTB &= ~(1 << KEY_CLOCK_PIN);

      if(FREQUENCIES[freq_index + FREQ_OFFSET]) { // TODO: removed with new hardware
        const uint8_t input = (~PINB) & (1 << KEY_INPUT_PIN);
        if(control) {
          if(input) {
            if(!control_pressed) {
              set_mode(freq_index);
              control_pressed = 1;
            }
            has_press = 1;
          }
        } else {
          const uint8_t current = state & mask;
          if(input && !current) {
            press_key(freq_index);
          } else if(!input && current) {
            release_key(freq_index);
          }
        }
      }

      // Pull clock high to cause a shift.
      PORTB |= 1 << KEY_CLOCK_PIN;

      freq_index += 1;
    }
  }
  if(!has_press) control_pressed = 0;

  // Pull load enable low to load new values.
  PORTB &= ~(1 << KEY_LATCH_PIN);
}

static void init()
{
  uint8_t i;
  for(i = 0; i < VOICE_COUNT; i++) {
    voices[i].freq_index = 0;
    voices[i].freq = 0;
  }
  for(i = 0; i < KEY_BYTES; i++) {
    key_state[i] = 0;
  }
  instrument = 0;
  tuning = INITIAL_TUNING;
  control_pressed = 0;
}

int main()
{
  cli();

  // Clock prescalar
  // Default is 1MHz, switch to 8MHz
  CLKPR = 1 << CLKPCE;
  CLKPR = 0;

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

  for(;;) {
    scan();
  }

  return 0;
}

