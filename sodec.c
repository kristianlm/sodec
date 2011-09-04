/**
Kristian

*/

#include <stdio.h>

#define ONE '#'
#define ZERO '.'

#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)
#define ABS(a) (a>=0?a:-(a))

#define ERROR 50
#define WARNING 100
#define INFO 200
#define DEBUG 500



#define SIGNAL_DIGITAL 2

unsigned long pos = 0;

char verbose = 20;

int max_pulse_len = 0;
int current_pulse_len = 0;

unsigned char sme_is_high(unsigned char signal) {
  if(signal > 0x20) return 1;
  else return 0;
}

// TODO support sound (tone = one, silence = zero)
// read a single bit from stdin, return if it was high or not
// returns 0 (low), 0xFE (high), or EOF (0xFF)
// todo rename to read_sample
int read_signal_nobuffer() {
  int c;

#ifdef SIGNAL_DIGITAL

  c = getchar(); pos++;
  if(sme_is_high(c)) current_pulse_len++;
  else current_pulse_len = 0;

  if(current_pulse_len > max_pulse_len) max_pulse_len = current_pulse_len;

  if(c == EOF) return EOF;
  return c == ONE ? 0xFE : 0;
#else

  int maxdiff = 0, i;
  static int last = 0; 
  for(i = 0 ; i < 64 ; i++) {
    c = getchar(); pos++;
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

int _peek = -2; // can't use -1 cause it's EOF
int _curr = -2;
/**
 * read_signal is buffered and offers peek_signal to see the next sample
 * using getchar anywhere will break this.
 */
int read_signal() {
  if(_curr == -2) {
    _curr = read_signal_nobuffer();
    _peek = read_signal_nobuffer();
  } else {
    _curr = _peek;
    _peek = read_signal_nobuffer();
  }
  //if(sme_is_high(_curr)) printf("#");
  //else printf(".");
  return _curr;
}

int peek_signal() {
  return _peek;
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

/**
 * Try to read 8 bits using clock_len as pulse-width
 * returns the byte if successful (unsigned, 0 ~ 0xFF)
 * if it fails, return value is negative and specifies the 
 * length of the last high-pulse. this can be used to recover
 * byte-marks.
 */
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

    if(sme_is_high(p1) && sme_is_high(p2)) {
      pulse_len += clock_len * 2;
      is_byte_valid = 0;
    }
    if(!sme_is_high(p1) && !sme_is_high(p2)) {
      is_byte_valid = 0;
    }
    if(!is_byte_valid) {
      fprintf(stderr, "data corrupt, skipping byte\n", p1, p2);
      break;
    }

    if(sme_is_high(p1)) byte = (byte << 1) | 0x01;
    else          byte = (byte << 1) | 0x00;

    // clear pulse_len (reset if low)
    if(sme_is_high(p2)) pulse_len = clock_len;
    else pulse_len = 0;
  }
  if(is_byte_valid)
    return byte;
  else {
    printf("invalid byteseq @ %d, resuming with pulse_len %d\n", pos, pulse_len);
    return -pulse_len;
  }

}

int main(int args, char** argv) {


  int avg;
  int clock_len = 0;

  int errors = 0;
  int pulse_len = 0;


  while(1) {

    int c;
    c = read_signal();//(pulse_len += readhighpulse()) != EOF) {
    if(c == EOF) break;
    if(verbose > 30)
      printf("waiting for carrier ....\n");

    if(sme_is_high(peek_signal())) {
        
      // we've found the clock pulse (it was $len long)
      // continue to measure length of pulse if we're still up
      // pulse_len is not always 0 here if reading last byte failed
      
      if(verbose>10)
        fprintf(stderr, "PULSE UP (pulse = %d)\n", pulse_len);

      while(sme_is_high(peek_signal())) {
        //printf("  peeking high pulse is %d @ %d\n", peek_signal(), pos);
        pulse_len ++;
        if(read_signal() == EOF) break;
      }
      if(verbose>10)
        fprintf(stderr, "PULSE DOWN (len = %d) @ %d\n", pulse_len, pos);

      // spec: the high-signal length is three clock cycles high
      clock_len = pulse_len / 3;

      if(clock_len <= 0) {
        if(verbose > 10)
          fprintf(stderr, "clock-len of %d is invalid, skipping byte-mark @ %d\n", clock_len, pos);
        continue;
      }
      if(verbose>10)
        fprintf(stderr, "FOUND BYTE-MARK clock_len = %d @ %d\n", clock_len, pos);

      avg = readavg(clock_len);
      if(sme_is_high(avg)) {
        if(verbose > 10) printf("low-gap after byte-mark missing or too short @ %d\n", pos);
        continue; // should be signal-low! (always gap after byte-mark)
      }

      if(verbose>30)
        fprintf(stderr, "   (l %d) gap  %d \n", clock_len, avg);

      int readstat = readbits(clock_len);
      if(readstat >= 0) {
        // byte sequence read sucessfully
        clock_len = 0;
        if(verbose>5)
          fprintf(stderr, "BYTE OK: 0x%X (%d) char %c\n", readstat, readstat, (char)readstat);
        printf("%c", readstat);
      }
      else {
        // byte-sequence corrupt
        printf("BYTE FAIL: pulse-length kept as %d @ %d\n", pulse_len, pos);
        pulse_len = -readstat;
      }

    }

    else {
      if(verbose > 30)
        printf("still at low @ %d \n", pos);
      pulse_len = 0;
    }
    
  }

  //  fprintf(stderr, "clock_len was %d\n", clock_len);

  return errors;
}
