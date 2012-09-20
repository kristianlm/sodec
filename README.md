# Sodec - sound decoder

  [AVR]:http://en.wikipedia.org/wiki/Atmel_AVR
  [Manchester Encoded]: http://en.wikipedia.org/wiki/Manchester_encoding
  
My test-project for making a serial connection from my [AVR] to my PC.
I am using my homebrew simple [Manchester Encoded] signal.

I haven't gotten it to work on anything faster than 60 bps.

You can clone and try this from your terminal

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

Where `gen` is a tool to output my homebrew signal. 

You can generate the same signal from the small avr sodec folder. 
Schematics for the [AVR] are incredibly simple. 
Then, once your [AVR] is connected to your line-in, you can do:

```bash
$ arecord -f u8 -r 44100 | ./sodec | dd bs=1
Recording WAVE 'stdin' : Unsigned 8 bit, Rate 44100 Hz, Mono
Hello World!
Counter: 1
Counter: 2
Counter: 3
Count^C
51+0 records in
51+0 records out
51 bytes (51 B) copied, 6.94628 s, 0.0 kB/s
```

`dd` is too slow for `dd` to bother calculating the bandwidth, which is at around 60 bps.
