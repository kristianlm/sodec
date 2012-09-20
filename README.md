# Sodec - sound decoder

  [AVR]:http://en.wikipedia.org/wiki/Atmel_AVR
  [Manchester Encoded]: http://en.wikipedia.org/wiki/Manchester_encoding
  [aplay]:http://en.wikipedia.org/wiki/Aplay
  
A test-project for making a serial connection from my [AVR] to my PC
using a simple [Manchester Encoded] signal variant.

My setup involves signal from a digital-out [AVR]-pin into a
sound-card's line-in. I haven't gotten it to work on anything faster than 60 bps.

Since it's 1-pin, it is also possible to transmit the signal wirelessly using IR diodes or by
using a speaker and a mic.

## Building

```bash
$ cmake .
$ make
```

## Trying it out

`gen` will encode each byte from stdin
into a sequence of bits where `#` is on and `.` is off, one byte per line.
The clock is encoded in the signal (as always with [Manchester encoding][Manchester encoded]).

```bash
$ echo -n hello world | ./gen
.###..##.#..##..#.#.#
.###..##.#..#.##..##.
.###..##.#..##.#..#.#
.###..##.#..##.#..#.#
.###..##.#..##.#.#.#.
.###..#.##..#.#.#.#.#
.###..##.#.#..##.#.#.
.###..##.#..##.#.#.#.
.###..##.#.#..#.##..#
.###..##.#..##.#..#.#
.###..##.#..#.##..#.#
```

`gen` can be configured to output sinus waves (AUDIO_OUTPUT) which can be piped to [aplay], for example.

You can generate the same signal from the project in the `avr`
subfolder. Then, once your [AVR] output pin is connected to your
sound-card's line-in, you can do:

```bash
$ arecord -f u8 -r 44100 | ./sodec | dd bs=1
Recording WAVE 'stdin' : Unsigned 8 bit, Rate 44100 Hz, Mono
Hello World from AVR!
Counter: 1
Counter: 2
Counter: 3
Count^C
51+0 records in
51+0 records out
51 bytes (51 B) copied, 6.94628 s, 0.0 kB/s
```

The bandwidth seems to be too slow for `dd` to pick up,
which ends up at a staggering 60 bps.
