//******************************************************************************
//  Blink LED at P1.0 on eZ430-RF2500
//
//  Description; Toggle P1.0 by inside a nested software delay loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//  Paul Flikkema
//  Northern Arizona University
//	For MSP430F2274 on-board the eZ430-RF2500
//  Built with CCS for MSP430 V5.2
//	28 Aug 12
//******************************************************************************

//Modified by Jason Travis, Trent Cooper

#include <msp430.h> // CCS finds the device-specific header file msp430f2274.h

// Microseconds
#define OUTERMAX 100
#define INNERMAX 5000


int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;         // stop watchdog timer
  P1DIR |= BIT1;                    // set P1.1 Green LED to output direction
  // Turn all pins Off
  P1OUT = 0;

  while (1)							// run forever
  {
    int ni, nj;
    // On
    P1OUT ^= BIT1;                  // toggle P1.1 Green LED

    // Half second delay
    for (ni = 0; ni < 10; ni++)
	// outer software delay loop
	{
		for (nj = 0; nj < INNERMAX; nj++){}	// inner software delay loop
	}

    // Off
    P1OUT ^= BIT1;                  // toggle P1.1 Green LED

    // 5 second delay
	for (ni = 0; ni < OUTERMAX; ni++)
	// outer software delay loop
	{
		for (nj = 0; nj < INNERMAX; nj++){}	// inner software delay loop
	}
  }
}
