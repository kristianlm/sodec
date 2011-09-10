
#include <inttypes.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>


#define PLEN 6

int g = 0;


void wait(int delay) {
  long x = 0x40;
  long y = delay;
  for( ; x  >= 0 ; x--) {
    for(; y >= 0 ; y--) {
      // wait a little 
      g = g + 1;
    }
    
  }

}

#define on() PORTC=0xFF
#define off() PORTC=0
void mysilent(int delay) {
  long x = delay * 10;
  for( ; x  >= 0 ; x--) {
    wait(1);
    //off();
    wait(1);
    //off();
  }
}


void myvibrate(int delay) {
  long x = delay * 10;
  for( ; x  >= 0 ; x--) {
    wait(1);
    on();
    wait(1);
    off();
  }
}



void push_one() {
  myvibrate(PLEN);
  mysilent(PLEN);
}
void push_zero() {
  mysilent(PLEN);
  myvibrate(PLEN);
}
void byte_pause() {
  mysilent(PLEN);
  myvibrate(PLEN);
  myvibrate(PLEN);
  myvibrate(PLEN);
  mysilent(PLEN);
}

static void bang_bit(char bit) {
  if(bit) push_one();
  else push_zero();
}

static int uart_putchar(char c, FILE *stream) {
	if (c == '\n')
		uart_putchar('\r', stream);
	byte_pause();
	int bit = 7;
	for( ; bit >= 0 ; bit--) {
	  bang_bit(c & 0x80);
	  c = c << 1;
	}

	return 0;
}

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,
                                         _FDEV_SETUP_WRITE);



void main(void)
{
  stdout = &mystdout;
  DDRC = 0xFF;

  printf("Hello World!\n");
  int val = 0;
  while(1) {
    
    printf("Counter: %d\n", val);
    val++;
    //uart_putchar((char)0x41, 0);
  }
  /*    DDRB = 0xff;                //enable port b for output 
        for ( cnt=0xff; cnt > 0; cnt-- )
        PORTB = cnt;
  */
  sleep_cpu();
}
