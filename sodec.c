
#include <stdio.h>

#define ONE '#'
#define ZERO '.'

#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)
#define ABS(a) (a>=0?a:-(a))

#define SIGNAL_DIGITAL 2

unsigned long pos = 0;

char verbose = 20;

int _next = -2;
int _read = -2;
int readbyte() {
  if(_next == -2) {
    printf(" READ TWO ");
    _read = getchar();
    _next = getchar();
  } else {
    _read = _next;
    _next = getchar();
  }
  pos++;
  return _read;
}
int peekbyte() {
  return _next;
}
#define getchar #error "don't use getchar(), use readbyte for a single-byte buffer"


// TODO support sound (tone = one, silence = zero)
// read a single bit from stdin, return if it was high or not
// returns 0 (low), 0xFE (high), or EOF (0xFF)
unsigned char read_signal() {
  int c;
#ifdef SIGNAL_DIGITAL
  c = readbyte(); pos++;
  if(c == EOF) return EOF;
  return c == ONE ? 0xFE : 0;
#else
  int maxdiff = 0, i;
  static int last = 0; 
  for(i = 0 ; i < 64 ; i++) {
    c = readbyte();
    //    printf(" %d(%d) ", c, last);
    if(c == EOF) return EOF;
    maxdiff = MAX(c - last, maxdiff);
    last = c;
  }
  //  printf(" maxdiff = %d\n", maxdiff);
  if(maxdiff > 0x10) return 0xFE;
  else return 0;
#endif
}

unsigned char is_high(unsigned char signal) {
  printf(" signal %d ", signal);
  if(signal > 0x20) return 1;
  else return 0;
}

/*
int readhighpulse() {
  char c ;
  int len = 1;
  int apple;

  
  //  printf("skipping low-signal @ %d\n", pos);
  // skip all zeros
  while((c = read_signal()) != EOF) {
    if(is_high(c)) break;
  }
  printf("found high signal @ %d\n", pos);
  // we have already read 1 high signal
  while((c = read_signal()) != EOF) {
    if(!is_high(c))
      return len;
    len ++;
  }
  return EOF;
}
*/

int readavg(int len) {
  if(len <= 0) return 0;
  int i;
  int sum = 0;
  int c;

  for(i = 0 ; i < len ; i++) {
    c = read_signal();
    if(c == EOF) return EOF;

    sum += (int)c;
  }
  if(verbose>30) 
    fprintf(stderr, "        avg for %d period is %d \n", len, sum / len);
  return sum / len;
}

int readbits(int clock_len) {
  char bit;
  int is_byte_valid = 1;
  int pulse_len = clock_len * 3;
  unsigned char p1, p2, byte;

  // read 8 bits
  for(bit = 0 ; bit < 8 ; bit++) {
    p1 = readavg(clock_len);
    p2 = readavg(clock_len);

    if(verbose>10)
      fprintf(stderr, "   p1 %d   p2 %d  @ %d\n", p1, p2, pos);

    if(is_high(p1) && is_high(p2)) {
      pulse_len += clock_len * 2;
      is_byte_valid = 0;
    }
    if(!is_high(p1) && !is_high(p2)) {
      is_byte_valid = 0;
    }
    if(!is_byte_valid) {
      fprintf(stderr, "data corrupt, skipping byte\n", p1, p2);
      break;
    }

    if(is_high(p1)) byte = (byte << 1) | 0x01;
    else          byte = (byte << 1) | 0x00;

    // clear pulse_len (reset if low)
    if(is_high(p2)) pulse_len = clock_len;
    else pulse_len = 0;
  }
  if(is_byte_valid)
    return byte;
  else {
    printf("invalid byteseq, resuming with pulse_len %d", pulse_len);
    return -pulse_len;
  }

}

/* int main5() { */
/*   int c; */
/*   while((c = read_signal()) != EOF) { */
/*     if(is_high(c)) c = ONE; */
/*     else c = ZERO; */
/*     printf("%c", c); */
/*   } */
/* } */

int main(int args, char** argv) {


  int avg;
  int clock_len = 0;

  int errors = 0;
  int pulse_len = 0;


  while(1) {

    int c;
    c = read_signal();//(pulse_len += readhighpulse()) != EOF) {
    if(c == EOF) break;
    printf("read %c, peek %c (is high %d) \n", c, peekbyte(), is_high(peekbyte()));


    if(!is_high(peekbyte())) {

      // we've found the clock pulse (it was $len long)
      // next tick should be low
      if(verbose>10)
        fprintf(stderr, "*** (pulse = %d)", pulse_len);

      // spec: the high-signal length is three clock cycles high
      clock_len = pulse_len / 3;
      if(verbose>10)
        fprintf(stderr, ", clock_len = %d @ %d\n", clock_len, pos);

      //      if(clock_len <= 0) continue;

      avg = readavg(MAX(clock_len - 1, 0));
      if(is_high(avg))
        continue; // should be signal-low! (always gap after byte-mark)

      if(verbose>30)
        fprintf(stderr, "   (l %d) lowarea  %d \n", clock_len, avg);

      int readstat = readbits(clock_len);
      if(readstat >= 0) {
        clock_len = 0;
        if(verbose>5)
          fprintf(stderr, "\t\t\tread byte 0x%X (%d) =====  %c\n", readstat, readstat, (char)readstat);
        printf("\033[32m%c\033[0m", readstat);
      } 
      else {
        printf("pulse-length kept as %d @ %d\n", pulse_len, pos);
        pulse_len = -readstat;
      }

      printf(" ........ clock_len %d\n", clock_len);
    } else {
      printf("still at low @ %d \n", pos);
      pulse_len++;
    }
  }

  //  fprintf(stderr, "clock_len was %d\n", clock_len);

  return errors;
}
