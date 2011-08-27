
#include <stdio.h>

#define ONE '#'
#define ZERO '.'

#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)
#define ABS(a) (a>=0?a:-(a))

#define SIGNAL_DIGITAL

char verbose = 1;

// TODO support sound (tone = one, silence = zero)
// read a single bit from stdin, return if it was high or not
// returns 0 (low), 0xFE (high), or EOF (0xFF)
unsigned char read_signal() {
  int c;
#ifdef SIGNAL_DIGITAL
  c = getchar();
  if(c == EOF) return EOF;
  return c == ONE ? 0xFE : 0;
#else
  int maxdiff = 0, i;
  static int last = 0; 
  for(i = 0 ; i < 64 ; i++) {
    c = getchar();
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
  if(signal > 0x20) return 1;
  else return 0;
}

int readhighpulse() {
  char c ;
  int len = 1;
  // skip all zeros
  while((c = read_signal()) != EOF) {
    if(c) break;
    //printf(".");
  }
  while((c = read_signal()) != EOF) {
    if(c == 0)
      return len;
    len ++;
  }
  return EOF;
}

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

int main5() { 
  int c;
  while((c = read_signal()) != EOF) {
    if(is_high(c)) c = ONE;
    else c = ZERO;
    printf("%c", c);
  }
}

int main(int args, char** argv) {
  char bit;
  unsigned char p1, p2, byte;
  int avg;
  int clock_len = 0;

  int errors = 0;
  int continuation_len = 0;
  char is_corrupt;

  while((clock_len = readhighpulse()) != EOF) {
    // we've found the clock pulse (it was $len long)
    // next tick should be low
    if(verbose>20)
      fprintf(stderr, "*** (pulse = %d)", clock_len);

    clock_len = clock_len / 3;
    if(verbose>20)
      fprintf(stderr, ", clock_len = %d\n", clock_len);

    avg = readavg(MAX(clock_len - 1, 0));
    if(is_high(avg))
      continue; // should be signal-low! (always gap after byte-mark)

    if(verbose>30)
      fprintf(stderr, "   (l %d) lowarea  %d \n", clock_len, avg);

    is_corrupt = 0;
    byte = 0;
    // read 8 bits
    for(bit = 0 ; bit < 8 ; bit++) {
      p1 = readavg(clock_len);
      p2 = readavg(clock_len);

      if(verbose>10)
        fprintf(stderr, "   p1 %d   p2 %d\n", p1, p2);

      if(is_high(p1) && is_high(p2)) {
        continuation_len = clock_len * 2;
        is_corrupt = 1;
      }
      if(!is_high(p1) && !is_high(p2)) {
        is_corrupt = 1;
      }
      if(is_corrupt) {
        fprintf(stderr, "data corrupt, skipping byte\n", p1, p2);
        break;
      }

      if(is_high(p1)) byte = (byte << 1) | 0x01;
      else          byte = (byte << 1) | 0x00;
    }

    if(verbose>5)
      fprintf(stderr, "read byte 0x%X (%d) =====  %c\n", byte, byte, (char)byte);
    printf("\033[32m%c\033[0m", byte);
  }

  //  fprintf(stderr, "clock_len was %d\n", clock_len);

  return errors;
}
