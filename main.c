#include <msp430.h> 

#define GREEN_LED BIT1
#define RED_LED BIT0

void swDelay(int milliseconds);

/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
    P1DIR |= RED_LED|GREEN_LED;                    // set P1.1 Red and Green LED to output direction
	// Turn all pins Off
	P1OUT = 0;

	// Simulate a 60bpm heart beat by flashing the RED and GREEN LEDs
	while (1)							// run forever
	{
		// Red LED On
		P1OUT = RED_LED;

		// pulse red chamber .1 second
		swDelay(100);

		// Red LED Off Green LED On
		P1OUT ^= RED_LED|GREEN_LED;

		// pulse green chamber .1 second
		swDelay(100);

		// All LEDs off
		P1OUT = 0;

		// .8 second delay between beats
		swDelay(800);
	}
}

// Software delay
void swDelay(int nMilliseconds) {
	// volatile is required so the compiler will not optimize
	volatile int ni;
	volatile int nj;
	for(ni=0; ni<nMilliseconds; ++ni) {
		for(nj=0; nj<150; ++nj) {}
	}
}
