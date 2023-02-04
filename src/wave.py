
import math

step_count = 256
max_value = 255


def sine_wave(i):
    x = 2 * math.pi * i / step_count
    return int(max_value * 0.5 * (1.0 + math.sin(x)))


def output_stop(func):
  for i in range(0, step_count):
    print('    {}, // {}'.format(func(i), i))


print('const static uint8_t __flash SINE_WAVE[{}] = {{'.format(step_count))
output_stop(sine_wave);
print('};')


