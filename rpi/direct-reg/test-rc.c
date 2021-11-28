
//
//  How to access GPIO registers from C-code on the Raspberry-Pi
//  Example program
//  15-January-2012
//  Dom and Gert
//  Revised: 15-Feb-2013


// Access from ARM Running Linux



// gettimeofday() takes about 1us


//  pulse out = bcm5, in = bcm 13


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

int gpio_time_to_change(void)
{
   struct timeval started_tv, finished_tv;
   long us_diff;
   unsigned started_at, got;

   started_at = GET_ALL_GPIO;
   gettimeofday(&started_tv, NULL);
   for ( int i=0 ; i<10000000 ; i++ ) {
     if (GET_ALL_GPIO != started_at) {
       gettimeofday(&finished_tv, NULL);
       got = GET_ALL_GPIO;
       us_diff = (finished_tv.tv_sec - started_tv.tv_sec) * 1000000;
       us_diff += finished_tv.tv_usec - started_tv.tv_usec;
       printf("elapsed us %ld\n", us_diff);
       print_gpio(started_at);
       print_gpio(got);

       return i;
     }
   }

   return -1;
}
 
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

int measurement(int meas_pin, int pulse_pin, int pulse_start)
{
    int count;

    // measure pin to output mode, low
    INP_GPIO(meas_pin);
    OUT_GPIO(meas_pin);
    GPIO_CLR = (1<<meas_pin);

    // wait for capacitor to empty
    usleep(5);

    // start measurement
    INP_GPIO(meas_pin);
    
    // start or stop the pulse
    if (pulse_start)
        GPIO_SET = (1<<pulse_pin);
    else
        GPIO_CLR = (1<<pulse_pin);
	
    // time how many loops it takes for the input to go high
    for ( count=0 ; count < 100000 && ! GET_GPIO(meas_pin) ; count++ )
        ;
	
    return count;
}

int main(int argc, char **argv)
{
  int src_pin;
  src_pin = 21;

  // Set up gpi pointer for direct register access
  setup_io();

  INP_GPIO(src_pin);
  OUT_GPIO(src_pin);
  GPIO_CLR = (1<<src_pin);

  sleep(1);

  GPIO_SET = (1<<src_pin);
  printf("changetime: %d\n", gpio_time_to_change());

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
