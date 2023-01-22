
import math

step_count = 256
max_value = 255

def output_table(func):
  print('  {')
  for i in range(0, step_count):
    print('    {}, // {}'.format(func(i), i))
  print('  },')


def sine_wave(i):
  return int(
    max_value * 0.5 * (
      1.0 + math.sin(2 * math.pi * i / step_count - math.pi / 2)
    )
  )


def triangle_wave(i):
  if i == 128:
    return 255
  if i > 127:
    return 256 - (i - 128) * 2
  else:
    return i * 2


def square_wave(i):
  if i > 127:
    return 255
  else:
    return 0

def sawtooth(i):
  return i


print('#define TRIANGLE_WAVE 0')
print('#define SINE_WAVE 1')
print('#define SQUARE_WAVE 2')
print('#define SAWTOOTH_WAVE 3')
print('const static uint8_t __flash WAVE[4][{}] = {{'.format(step_count))
output_table(triangle_wave)
output_table(sine_wave)
output_table(square_wave)
output_table(sawtooth)
print('};')


