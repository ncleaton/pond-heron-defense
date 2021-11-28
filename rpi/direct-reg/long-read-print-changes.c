
//
//  How to access GPIO registers from C-code on the Raspberry-Pi
//  Example program
//  15-January-2012
//  Dom and Gert
//  Revised: 15-Feb-2013


// Access from ARM Running Linux



// gettimeofday() takes about 1us




#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int  mem_fd;
void *gpio_map;

// I/O access
volatile unsigned *gpio;


// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH

#define GET_ALL_GPIO  (*(gpio+13))

#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock

void setup_io();

void printButton(int g)
{
  if (GET_GPIO(g)) // !=0 <-> bit is 1 <- port is HIGH=3.3V
    printf("Button pressed!\n");
  else // port is LOW=0V
    printf("Button released!\n");
}

void print_gpio(unsigned n);

void print_gpio(unsigned n)
{
    printf("gpio: ");
	while (n) {
	    if (n & 1)
		printf("1");
	    else
		printf("0");
	    n >>= 1;
	}
    printf("\n");
}


#define pulse_trigger_out_pin 13
#define pulse_old_in_pin 6
#define pulse_in_pin 5
#define pulse_active_in_pin 2
#define ir_in_pin 3

#define pulse_readmask ( (1<<pulse_in_pin) | (1<<pulse_old_in_pin) | (1<<pulse_active_in_pin) | (1<<ir_in_pin) )
#define PULSE_SAMPLE ( GET_ALL_GPIO & pulse_readmask ) 

#define SAMPLE_COUNT 1000000

int main(int argc, char **argv)
{
  struct timeval started_tv, finished_tv;
  long us_diff;
  int x, old_x;

  // Set up gpi pointer for direct register access
  setup_io();

  INP_GPIO(pulse_trigger_out_pin);
  OUT_GPIO(pulse_trigger_out_pin);
  GPIO_CLR = (1<<pulse_trigger_out_pin);

  INP_GPIO(pulse_in_pin);
  INP_GPIO(pulse_old_in_pin);
  INP_GPIO(pulse_active_in_pin);
  INP_GPIO(ir_in_pin);

  sleep(1);

  gettimeofday(&started_tv, NULL);
  GPIO_SET = (1<<pulse_trigger_out_pin);
  
  old_x = -1;
  for ( int i=0 ; i<SAMPLE_COUNT ; i++ ) {
    x = PULSE_SAMPLE ;
    if (x != old_x) {
      printf("%d %d %d %d %d\n", i,
	 x&(1<<pulse_in_pin) ? 1 : 0,
	 x&(1<<pulse_old_in_pin) ? 1 : 0,
	 x&(1<<pulse_active_in_pin) ? 1 : 0,
	 x&(1<<ir_in_pin) ? 1 : 0);
      old_x = x;
    }
  }
      printf("%d %d %d %d %d\n", SAMPLE_COUNT,
	 x&(1<<pulse_in_pin) ? 1 : 0,
	 x&(1<<pulse_old_in_pin) ? 1 : 0,
	 x&(1<<pulse_active_in_pin) ? 1 : 0,
	 x&(1<<ir_in_pin) ? 1 : 0);

  gettimeofday(&finished_tv, NULL);

  GPIO_CLR = (1<<pulse_trigger_out_pin);

  us_diff = (finished_tv.tv_sec - started_tv.tv_sec) * 1000000;
  us_diff += finished_tv.tv_usec - started_tv.tv_usec;
  printf("elapsed us %ld\n", us_diff);

  return 0;
} // main


//
// Set up a memory regions to access GPIO
//
void setup_io()
{
   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }

   /* mmap GPIO */
   gpio_map = mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      GPIO_BASE         //Offset to GPIO peripheral
   );

   close(mem_fd); //No need to keep mem_fd open after mmap

   if (gpio_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)gpio_map);//errno also set!
      exit(-1);
   }

   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;


} // setup_io
