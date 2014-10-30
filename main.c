#include <msp430f2274.h>

#define RED_LED BIT0
#define GREEN_LED BIT1
#define BUTTON 0x04

void swDelay(int milliseconds);
volatile int isBeating = 0;

/*
 * main.c
 */
int main(void) {
	// Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;
	
    // Set Red and Green LED to output direction
    P1DIR |= RED_LED|GREEN_LED;

	// Turn off all LEDs
	P1OUT = 0;
	P1OUT |= BUTTON;

	P1SEL &= ~BUTTON;
	// Set BUTTON as an input
	P1DIR &= ~BUTTON;
	// Enable BUTTON pull-up resistor
	P1REN |= BUTTON;
	// Enable BUTTON interrupts
	P1IE |= BUTTON;
	// Port 1 Edge Select
	P1IES |= BUTTON;
	// Global Interrupt Enable
	__bis_SR_register(GIE);
	// IFG cleared
	P1IFG &= ~BUTTON;

	// Simulate a 60bpm heart beat by flashing the RED and GREEN LEDs
	while (1)							// run forever
	{
		if (!isBeating)
		{
			continue;
		}
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

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
	if (P1IFG & BUTTON) {
		static volatile unsigned int ni;
		static unsigned int nHighCount = 0;

		// Debounce button
		for(ni = 0; ni < 7; ni++) {
			if(!(P1IN & BUTTON)) {
				nHighCount++;
			}
		}

		// Toggle isBeating if the button was pressed after being debounced
		if(nHighCount == 7) {
			isBeating ^= 1;
		}

		// IFG cleared
		P1IFG &= ~BUTTON;
	}

	return;
}
