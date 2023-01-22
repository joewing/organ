
SAMPLE_HZ     = 12500.0       # Frequency of the sample timer.
A4_OFFSET     = 4 * 12 + 9    # Offset of A4 (we start with C0).
OCTAVE_COUNT  = 8

note_names = [
  'C',
  'C#',
  'D',
  'D#',
  'E',
  'F',
  'F#',
  'G',
  'G#',
  'A',
  'A#',
  'B',
]

print('const static uint16_t __flash FREQUENCIES[] = {')
freq_index = 0
array_index = 0
for octave in range(0, OCTAVE_COUNT):
  for i in range(0, 4):
    print('  {:5}, // {:2}: {}'.format(0, array_index, 'off'))
    array_index += 1
  for i in range(0, 12):
    freq = 440.0 * pow(2.0, (freq_index - A4_OFFSET) / 12.0)
    count = int(round(freq * 65536.0 / SAMPLE_HZ))
    name = "{}{}".format(note_names[freq_index % 12], int((freq_index / 12)))
    print('  {:5}, // {:2}: {} - {}'.format(count, array_index, name, freq))
    array_index += 1
    freq_index += 1

print('};')

