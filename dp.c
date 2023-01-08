
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>

#include "freq.h"
#include "wave.h"

#define VOICE_BITS        3
#define KEY_BYTES         (2 * 8)

#define INSTRUMENT_COUNT  (sizeof(WAVE) / sizeof(WAVE[0]))
#define VOICE_COUNT       (1 << VOICE_BITS)

#define KEY_LATCH_PIN PB0
#define KEY_CLOCK_PIN PB1
#define FUN_INPUT_PIN PB2
#define KEY_INPUT_PIN PB3
#define AUDIO_OUT_PIN PB4

typedef struct Voice {
  volatile uint16_t timer;
  volatile uint8_t freq_index;
} Voice;

static volatile uint8_t instrument;
static Voice voices[VOICE_COUNT];
static uint8_t key_state[KEY_BYTES];

ISR(TIMER0_COMPA_vect)
{
  uint16_t mix = 0;
  uint8_t vi;
  for(vi = 0; vi < VOICE_COUNT; vi++) {
    const uint8_t freq_index = voices[vi].freq_index;
    if(freq_index) {
      const uint16_t timer = voices[vi].timer;
      const uint8_t wave_index = timer >> 8;
      voices[vi].timer = timer + FREQUENCIES[freq_index];
      mix += WAVE[instrument][wave_index];
    }
  }
  OCR1B = (mix >> VOICE_BITS) & 0xFF;
}

static void press_key(uint8_t freq_index)
{
  uint8_t i;
  for(i = 0; i < VOICE_COUNT; i++) {
    if(voices[i].freq_index == 0) {
      voices[i].timer = 0;
      voices[i].freq_index = freq_index;
      break;
    }
  }
}

static void release_key(uint8_t freq_index)
{
  uint8_t i;
  for(i = 0; i < VOICE_COUNT; i++) {
    if(voices[i].freq_index == freq_index) {
      voices[i].freq_index = 0;
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
  uint8_t i;
  uint8_t freq_index = 1;

  // Pull the load enable high to hold values.
  PORTB |= 1 << KEY_LATCH_PIN;

  for(i = 0; i < KEY_BYTES; i++) {
    const uint8_t control = (~PINB) & (1 << FUN_INPUT_PIN);
    uint8_t state = key_state[i];
    uint8_t mask = 0x80;
    while(mask) {

      // Bring clock low so we shift on the next clock.
      PORTB &= 1 << KEY_CLOCK_PIN;

      const uint8_t input = PINB & (1 << KEY_INPUT_PIN);
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
      mask >>= 1;
    }
    if(!control) {
      key_state[i] = state;
    }
  }

  // Pull load enable low to load new values.
  PORTB &= 1 << KEY_LATCH_PIN;
}

static void init()
{
  uint8_t i;
  for(i = 0; i < VOICE_COUNT; i++) {
    voices[i].freq_index = 0;
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
  // Clock is 8MHz, set prescalr to 8 and divide by 100 to get 10kHz
  TCCR0A = 3 << WGM00;                  // Fast PWM
  TCCR0B = (1 << WGM02) | (2 << CS00);  // Waveform, divide by 8
  OCR0A = 100 - 1;                      // Divide by 100.
  TIMSK = 1 << OCIE0A;                  // Enable compare match interrupt

  init();

  sei();

instrument = 1;
//voices[0].freq_index = 68;
voices[1].freq_index = 75;

  for(;;) {
//    scan();
  }

  return 0;
}

