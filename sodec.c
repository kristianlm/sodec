

/**
Kristian

*/

#include <stdio.h>
#include <signal.h>

#define ONE '#'
#define ZERO '.'

#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)
#define ABS(a) (a>=0?a:-(a))

#define ERROR 50
#define WARNING 100
#define INFO 200
#define DEBUG 500



//#define SIGNAL_DIGITAL

unsigned long pos = 0;

char verbose = 0;

int max_pulse_len = 0;
int current_pulse_len = 0;
#define SPECTRUM_SIZE 16
int spectrum [ SPECTRUM_SIZE ];

void spectrum_init() {
  int i;
  for(i = 0 ; i < SPECTRUM_SIZE ; i++)
    spectrum[i] = 0;
}
void spectrum_add(int pos) {
  static char _inited = 0;
  if(!_inited) {
    _inited = 1;
    spectrum_init();
  }
  
  if(pos >= SPECTRUM_SIZE) {
    fprintf(stderr, "out of bounds %d", pos);
    return;
  }

  spectrum[pos] ++;

}

void print_spectrum() {
  int i;
  int max_spectrum = 0;
  for(i = 0 ; i < SPECTRUM_SIZE ; i++) {
    if(spectrum[i] > max_spectrum) 
      max_spectrum = spectrum[i];
  }
  for(i = 0 ; i < SPECTRUM_SIZE ; i++) {
    fprintf(stderr, "%3d. ", i*0x10);
    int j;
    for(j = 0 ; j < (double)spectrum[i] * (80.0 / (double)max_spectrum) ; j++) 
      fprintf(stderr, "=");
    fprintf(stderr, "\n");
  }
 
}

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

  if(c == EOF) return EOF;
  return c == ONE ? 0xFE : 0;
#else

  int i;
  int lowest = 0xFF;
  int highest = 0;
  for(i = 0 ; i < 5 ; i++) {
    c = getchar(); pos++;
    //    printf(" %d(%d) ", c, last);
    if(c == EOF) return EOF;
    if(c < lowest) lowest = c;
    if(c > highest) highest = c;
  }
  int maxdiff;
  maxdiff = highest - lowest;
  //printf(" maxdiff = %d\n", maxdiff);
    return maxdiff;

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

  if(sme_is_high(_curr)) current_pulse_len++;
  else current_pulse_len = 0;

  if(current_pulse_len > max_pulse_len) max_pulse_len = current_pulse_len;

  spectrum_add(_curr / 0x10);

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

  if(verbose >10)
    fprintf(stderr, "Reading 8-BIT sequence (pulse_len = %d) @ %d\n", pulse_len, pos);

  // read 8 bits
  for(bit = 0 ; bit < 8 ; bit++) {
    p1 = readavg(clock_len);
    p2 = readavg(clock_len);
    
    if(verbose>10)
      fprintf(stderr, "  BITPAIR %d\t %d  @ %d\n", p1, p2, pos);

    if(!sme_is_high(p1))
      pulse_len = 0;
    if(sme_is_high(p1) && sme_is_high(p2)) {
      pulse_len += clock_len * 2;
      is_byte_valid = 0;
    }
    if(!sme_is_high(p1) && !sme_is_high(p2)) {
      is_byte_valid = 0;
    }
    if(!is_byte_valid) 
      break;

    if(sme_is_high(p1)) byte = (byte << 1) | 0x01;
    else          byte = (byte << 1) | 0x00;

    // clear pulse_len (reset if low)
    if(sme_is_high(p2)) pulse_len = clock_len;
    else pulse_len = 0;
  }
  if(is_byte_valid)
    return byte;
  else {
    if(verbose > 5)
      fprintf(stderr, "invalid byteseq @ %d, resuming with pulse_len %d\n", pos, pulse_len);
    return -(pulse_len + 1);
  }

}

static char keepRunning = 1;
void intHandler(int dummy) {
    keepRunning = 0;
}

int main(int args, char** argv) {

  // allow us to exit this loop
  // gracefully with C-c
  signal(SIGINT, intHandler);
  signal(SIGKILL, intHandler);

  int avg;
  int clock_len = 0;

  int errors = 0;
  int pulse_len = 0;


  while(keepRunning) {

    int c;
    c = read_signal();//(pulse_len += readhighpulse()) != EOF) {
    if(c == EOF) break;
    if(verbose > 30)
      fprintf(stderr, "waiting for carrier ....\n");

    if(sme_is_high(peek_signal())) {
        
      // we've found the clock pulse (it was $len long)
      // continue to measure length of pulse if we're still up
      // pulse_len is not always 0 here if reading last byte failed
      
      if(verbose>10)
        fprintf(stderr, "PULSE UP (pulse_len = %d) @ %d\n", pulse_len, pos);

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
      pulse_len = 0;

      if(verbose>30)
        fprintf(stderr, "   (l %d) gap  %d \n", clock_len, avg);

      int readstat = readbits(clock_len);
      if(readstat >= 0) {
        // byte sequence read sucessfully
        clock_len = 0;
        pulse_len = 0;
        if(verbose>1)
          fprintf(stderr, "BYTE OK: 0x%X (%d)\n", readstat, readstat);

        putchar(readstat);
        fflush(stdout);
      }
      else {
        pulse_len = -readstat - 1;
        // byte-sequence corrupt
        if(verbose > 5)
          fprintf(stderr,"BYTE FAIL: pulse-length kept as %d @ %d\n", pulse_len, pos);

      }

    }
    else {
      // no signal
      if(pulse_len > 0)
        if(verbose > 10)
          fprintf(stderr, "Pulselen killed @ %d\n", pos);
      if(verbose > 30)
        fprintf(stderr, "still at low @ %d \n", pos);
      pulse_len = 0;
    }
    
  }

  printf("\n");
  print_spectrum();
  printf("%d bytes read\n", pos);
  
  

  return errors;
}



