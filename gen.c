/**
Kristian Lein-Mathisen

*/

#include <stdio.h>

#define PULSE_LENGTH 1
// set to true to generate pulses (vibrate)
// set to false to generate simple digital signal (static)
#define AUDIO_OUTPUT 0 // boolean

#define ONE '#'
#define ZERO '.'

void inflate(char symbol, int length) {
  int i;
  for(i = 0 ; i < length ; i++) {
    putc(symbol, stdout);
  }
}


#if AUDIO_OUTPUT
void biplex(char hi, char lo, int length) {
  int i;
  for(i = 0 ; i < length ; i++) {
    inflate(hi, PULSE_LENGTH);
    inflate(lo, PULSE_LENGTH);
  } 
}
void dohigh() {
  biplex(ONE, ZERO, 1); // vibrate
}
void dolow() {
  biplex(ZERO, ZERO, 1); // silence
}
#else
void dohigh() {
  putc(ONE, stdout);
}
void dolow() {
  putc(ZERO, stdout);
}
#endif


void doone() {
  dohigh();
  dolow();
}
void dozero() {
  dolow();
  dohigh();
}


void dochar(char c) {
  // byte-mark: three consequetive high's in a row.
  // we know this is new byte because it is an 'illegal' bit
  // max high is two pulses with manchester encoding
  dozero();
  dohigh();  
  doone();
  int bit;
  for(bit = 0 ; bit < 8 ; bit++) {
    if((c & 0x80) > 0) doone();
    else dozero();
    c = c << 1;
  }
}

int main(int args, char** argv) {
  int c;

  while((c = getchar()) != EOF) {
    dochar(c);
    printf("\n");
  } 
  
}

