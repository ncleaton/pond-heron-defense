#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

int main (int argc, char **argv)
{
  unsigned pin;

  wiringPiSetup () ;

  pinMode(29, OUTPUT);

  digitalWrite(29, LOW);

  return 0;
} 
