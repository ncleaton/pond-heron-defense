#include <stdio.h>
#include <wiringPi.h>

#define LED     7

int main (void)
{
  printf ("Raspberry Pi  blink\n") ;

  wiringPiSetup () ;
  printf("tick\n");
  pinMode (LED, OUTPUT) ;
  printf("tick\n");

  for (;;)
  {
      printf("LED ON\n");
      digitalWrite (LED, HIGH) ;  // On
    delay (500) ;               // mS
       printf("LED OFF\n");
    digitalWrite (LED, LOW) ;   // Off
    delay (500) ;
  }
  return 0;
} 
