
base_freq_hz = 16.3515976563    # C0
int_freq_hz = 10000.0

note_names = [
  'C',
  'CS',
  'D',
  'DS',
  'E',
  'F',
  'FS',
  'G',
  'GS',
  'A',
  'AS',
  'B',
]

print('const static uint16_t __flash FREQUENCIES[] = {')
freq_index = 0
array_index = 0
for octave in range(0, 7):
  for i in range(0, 4):
    print('  {:5}, // {:2}: {}'.format(0, array_index, 'off'))
    array_index += 1
  for i in range(0, 12):
    freq = base_freq_hz * pow(2.0, freq_index / 12.0)
    count = int(round((freq * 65535.0) / int_freq_hz))
    name = "{}{}".format(note_names[freq_index % 12], int((freq_index / 12)))
    print('  {:5}, // {:2}: {}'.format(count, array_index, name))
    array_index += 1
    freq_index += 1

print('};')

