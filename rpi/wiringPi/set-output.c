#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>


int main (int argc, char **argv)
{
  unsigned pin;

  pin = atoi(argv[1]);

  wiringPiSetup () ;
  pinMode (pin, OUTPUT) ;

  return 0;
} 
