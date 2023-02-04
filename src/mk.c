
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#define ATTINY861A

#include <stdint.h>
#include <string.h>

#include "freq.h"
#include "wave.h"

#define VOICE_BITS        3       // 2^n voices at the same time
#define OCTAVE_COUNT      2       // Number of physcial octaves
#define INITIAL_OCTAVE    3       // Initial lowest physical octave
#define INITIAL_TUNING    0       // Initial tuning offset
#define A4_INDEX          77      // Frequency index of A4

#ifdef ATTINY861A
#define KEY_LATCH_PIN PB0
#define KEY_CLOCK_PIN PB1
#define FUN_INPUT_PIN PB2
#define AUDIO_OUT_PIN PB3
#define KEY_INPUT_PIN PB6
#else
#define KEY_LATCH_PIN PB0
#define KEY_CLOCK_PIN PB1
#define FUN_INPUT_PIN PB2
#define KEY_INPUT_PIN PB3
#define AUDIO_OUT_PIN PB4
#endif

#define FREQ_COUNT          (sizeof(FREQUENCIES) / sizeof(FREQUENCIES[0]))
#define VOICE_COUNT         (1 << VOICE_BITS)
#define KEY_BYTES           (OCTAVE_COUNT * 2)  // 2 bytes per octave
#define TUNING_KEY          (A4_INDEX - INITIAL_OCTAVE * 16) // Key index of A4
#define TUNING_EEPROM_ADDR  (uint8_t*)0x00
#define TUNING_COOKIE       0xA5

#define MIN_FREQ_OFFSET     0
#define MAX_FREQ_OFFSET     (FREQ_COUNT - 16 * OCTAVE_COUNT)

// Number of steps to move through the waveform each beat.
// In the order of Hammond drawbars:
const static uint8_t __flash STEPS[] = {
  1,    // 16'    - 16'               (16' - 1st)
  3,    // 5 1/3' - Quint             (16' - 3rd)
  2,    // 8'     - 8'          - 1st (16' - 2nd)
  4,    // 4'     - 4'          - 2nd (16' - 4th, 4' - 1st)
  6,    // 2 2/3' - Nazard      - 3rd (16' - 6th)
  8,    // 2'     - 2'          - 4th (16' - 8th, 4' - 2nd, 2' - 1st)
  10,   // 1 3/5' - Tierce      - 5th (16' - 10th)
  12,   // 1 1/3' - Larigot     - 6th (16' - 12th, 4' - 3rd)
  16,   // 1      - 1'          - 8th (16' - 16th, 4' - 4rd)
};
#define HARMONIC_COUNT (sizeof(STEPS) / sizeof(STEPS[0]))

typedef struct Voice {
  volatile uint16_t timer;    // Index into the wave table
  volatile uint16_t freq;     // Increment for the timer (0 if off)
  uint8_t key_index;          // Key index (0 if off)
} Voice;

static int8_t tuning_updated;         // Set if tuning needs to be saved
static uint8_t control_pressed;
static uint8_t freq_offset = INITIAL_OCTAVE * 16;
static Voice voices[VOICE_COUNT];
static uint8_t key_state[KEY_BYTES];
static uint8_t wave[256];
static uint8_t drawbars[HARMONIC_COUNT];

ISR(TIMER0_COMPA_vect)
{
  uint16_t mix = 0;
  for(Voice *vp = voices; vp != &voices[VOICE_COUNT]; vp++) {
    const uint8_t wave_index = (uint8_t)(vp->timer >> 8);
    mix += wave[wave_index];
    vp->timer += vp->freq;
  }
  OCR1B = (uint8_t)(mix >> VOICE_BITS);
}

static void press_key(uint8_t key_index)
{
  for(Voice *vp = voices; vp != &voices[VOICE_COUNT]; vp++) {
    if(vp->key_index == 0) {
      const uint8_t freq_index = key_index + freq_offset;
      vp->freq = FREQUENCIES[freq_index];
      vp->key_index = key_index;
      key_state[key_index >> 3] |= 1 << (key_index & 7);
      break;
    }
  }
}

static void release_key(uint8_t key_index)
{
  for(Voice *vp = voices; vp != &voices[VOICE_COUNT]; vp++) {
    if(vp->key_index == key_index) {
      vp->freq = 0;
      vp->timer = 0;
      vp->key_index = 0;
      key_state[key_index >> 3] &= ~(1 << (key_index & 7));
      break;
    }
  }
}

static void release_all_keys()
{
  memset(voices, 0, sizeof(voices));
  memset(key_state, 0, sizeof(key_state));
}

static void press_tuning_key()
{
  release_all_keys();
  press_key(TUNING_KEY);
}

static void read_tuning()
{
  if(eeprom_read_byte(TUNING_EEPROM_ADDR + 1) == TUNING_COOKIE) {
    const uint8_t value = eeprom_read_byte(TUNING_EEPROM_ADDR);
    while(OSCCAL > value) OSCCAL -= 1;
    while(OSCCAL < value) OSCCAL += 1;
  }
  tuning_updated = 0;
}

static void write_tuning()
{
  if(tuning_updated) {
    eeprom_write_byte(TUNING_EEPROM_ADDR, OSCCAL);
    eeprom_write_byte(TUNING_EEPROM_ADDR + 1, TUNING_COOKIE);
    tuning_updated = 0;
  }
}

static void update_wave()
{

  // Release all keys before doing this.
  release_all_keys();

  // Compute the total mixture.
  uint8_t total_mixture = 0;
  for(uint8_t h = 0; h < HARMONIC_COUNT; h++) {
    total_mixture += drawbars[h];
  }

  // Create the new waveform.
  uint8_t current_index[HARMONIC_COUNT] = { 0 };
  for(uint8_t *wp = wave; wp != &wave[256]; wp++) {

    // Add in each stop.
    uint16_t mixture = 0;
    for(uint8_t h = 0; h < HARMONIC_COUNT; h++) {
      const uint8_t wave_index = current_index[h];
      mixture += SINE_WAVE[wave_index] * drawbars[h];
      current_index[h] += STEPS[h];
    }

    if(total_mixture) {
      *wp = mixture / total_mixture;
    } else {
      *wp = 0;
    }
  }
}

#ifndef ATTINY861A
static void update_stop(uint8_t i)
{
  drawbars[i] ^= 1;
  update_wave();
}
#endif

#ifdef ATTINY861A
static void update_drawbars()
{
  if(ADCSRA & (1 << ADIF)) {
    const uint8_t index = ADMUX & 31;
    const uint8_t value = ADCH >> 5;
    if(value != drawbars[index]) {
      drawbars[index] = value;
      update_wave();
    }

    // Next input.
    ADMUX = (ADMUX & ~31) | (index >= HARMONIC_COUNT) ? 0 : (index + 1);
    ADCSRA |= (1 << ADSC) | (1 << ADIF);
  }
}
#endif

static void set_mode(uint8_t i)
{
  i -= 4;   // First 4 are blank.
  switch(i) {
  case 0:   // C
    update_stop(2);
    break;
  case 2:   // D
    update_stop(3);
    break;
  case 4:   // E
    update_stop(4);
    break;
  case 5:   // F
    update_stop(5);
    break;
  case 7:   // G
    update_stop(6);
    break;
  case 9:   // A
    update_stop(7);
    break;
  case 11:  // B
    update_stop(8);
    break;
  case 1: // C#
    update_stop(0);
    break;
  case 3: // D#
    update_stop(1);
    break;
  case 6: // F# (flatten)
    if(OSCCAL > 0) {
      OSCCAL -= 1;
      tuning_updated = 1;
    }
    break;
  case 8: // G# (reset tuning)
    update_wave();
    read_tuning();
    break;
  case 10: // A# (sharpen)
    if(OSCCAL < 127) {
      OSCCAL += 1;
      tuning_updated = 1;
    }
    break;
  default:
    break;
  }
  press_tuning_key();
}

static void scan()
{
  uint8_t key_index = 0;
  uint8_t has_press = 0;

  // Pull the load enable high to hold values.
  PORTB |= 1 << KEY_LATCH_PIN;

  const uint8_t control = (~PINB) & (1 << FUN_INPUT_PIN);
  for(uint8_t i = 0; i < KEY_BYTES; i++) {
    const uint8_t state = key_state[i];
    for(uint8_t mask = 0x01; mask; mask <<= 1) {

      // Bring clock low so we shift on the next clock.
      PORTB &= ~(1 << KEY_CLOCK_PIN);

      const uint8_t input = PINB & (1 << KEY_INPUT_PIN);
      if(control) {
        if(input) {
          if(!control_pressed) {
            set_mode(key_index);
            control_pressed = 1;
          }
          has_press = 1;
        }
      } else {
        const uint8_t current = state & mask;
        if(input && !current) {
          press_key(key_index);
        } else if(!input && current) {
          release_key(key_index);
        }
      }

      // Pull clock high to cause a shift.
      PORTB |= 1 << KEY_CLOCK_PIN;

      key_index += 1;
    }
  }
  if(!has_press) control_pressed = 0;
  if(!control && tuning_updated) write_tuning();

  // Pull load enable low to load new values.
  PORTB &= ~(1 << KEY_LATCH_PIN);
}

static void init()
{
  memset(drawbars, 0, sizeof(drawbars));
  drawbars[2] = 1;
  control_pressed = 0;
  read_tuning();
  update_wave();
}

int main()
{
  cli();

#ifdef ATTINY861A

  // Clock prescalar
  // Default is 1MHz, switch to 8MHz
  CLKPR = 1 << CLKPCE;
  CLKPR = 0;

  // Set input pullups and output state.
  PORTA = (1 << FUN_INPUT_PIN);
  PORTB = (1 << KEY_INPUT_PIN);

  // Set outputs.
  DDRA = 0;
  DDRB = (1 << KEY_LATCH_PIN)
       | (1 << KEY_CLOCK_PIN)
       | (1 << AUDIO_OUT_PIN);

  // PWM timer setup (timer1)
  PLLCSR = (1 << PCKE) | (1 << PLLE);     // 64MHz PLL source for timer1
  TIMSK = 0;                              // Timer interrupts off
  TCCR1A = (1 << CS10);                   // 1:1 prescalar
  TCCR1B = (1 << PWM1B) | (2 << COM1B0);  // PWM B, clear on match, OC1B
  TCCR1D = (0 << WGM10);                  // Fast PWM
  OCR1C = 255;                            // Divide by 256 -> 250 kHz
  OCR1B = 0;                              // Duty cycle
  
  // Tone generator timer setup (timer0)
  // Clock is 8MHz, set prescaler to 8 and divide by 50 to get 20kHz
  TCCR0A = 1;                             // 8-bit, waveform mode.
  TCCR0B = (2 << CS00);                   // Divide by 8
  OCR0A = 50 - 1;                         // Divide by 50.
  TIMSK = 1 << OCIE0A;                    // Enable compare match interrupt

  // Set up ADC for drawbars.
  ADMUX = (0 << REFS0)    // Vcc reference
        | (1 << ADLAR)    // Left adjust (we only need 8 bits of resolution)
        | (0 << MUX0);    // ADC0 - PA0
  ADCSRA  = (1 << ADEN)   // Enable ADC
          | (0 << ADSC)   // Don't start yet.
          | (0 << ADATE)  // Disable auto-trigger
          | (0 << ADIF)   // Clear interrupt flag
          | (0 << ADIE)   // Disable interupt
          | (7 << ADPS0); // Divide by 128
  ADCSRA |= (1 << ADSC);  // Start

#else

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
  // Clock is 8MHz, set prescaler to 8 and divide by 50 to get 20kHz
  TCCR0A = 3 << WGM00;                  // Fast PWM
  TCCR0B = (1 << WGM02) | (2 << CS00);  // Waveform, divide by 8
  OCR0A = 50 - 1;                       // Divide by 50.
  TIMSK = 1 << OCIE0A;                  // Enable compare match interrupt

#endif

  init();

  sei();

  for(;;) {
    scan();
#ifdef ATTINY861A
    update_drawbars();
#endif
  }

  return 0;
}

