#include <msp430f2274.h>

volatile int nIndex = 0;
volatile int dutyCycle[4] = { 0xF00, 0xB00, 0x600, 0x0 };

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    P1DIR |= 0x01; 		//Set red LED as an output
    P1REN |= 0x04;		//Enable pull-up button resistor
    P1OUT = 0x04;		//Enable button
    P1IE |= 0x04;		//Enable button interrupt
    P1IES |= 0x04;		//Enable button edge select
    P1IFG &= ~0x04;		//Clear button flag


    // Configure Clocks
	// Use TI factory cal results for DCO = 8 MHz
	DCOCTL = CALDCO_16MHZ;
	BCSCTL1 = CALBC1_16MHZ;
	//MCLK = DCO/1 and SMCLK = DCO / 8
	BCSCTL2 = SELM_0 | DIVM_0 | DIVS_3;


    //TACCTL0 = CCIE;
	TACCTL1 = CCIE;
    TACCR0 = 0xFFF;
    TACCR1 = dutyCycle[nIndex];
    // TimerA using SMCLK Up mode to CCR0 TAIE
    //ID_3 slows down TimerA by dividing by 8
    TACTL = TASSEL_2 | MC_3 | ID_0;

    __bis_SR_register(LPM0_bits + GIE);

	return 0;
}


#pragma vector = TIMERA1_VECTOR
__interrupt void Timer_A1(void) {
	P1OUT ^= 0x01;
	TACCTL1 &= ~CCIFG;
}


#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {

	if(!(P1IN & 0x04)) {
		static volatile unsigned int ni = 0;
		unsigned int nHighCount = 0;
		// Debounce button
		// check if button is still being pressed
		for(ni = 0; ni < 500; ni++) {
			if(!(P1IN & 0x04)) {
				nHighCount++;
			}
		}

		// Registered a debounced button press
		if(nHighCount == 500) {
			// Wrap around
			TACTL |= TACLR;
			P1OUT &= ~0x01;

			nIndex = (nIndex + 1) % 4;
			TACCR1 = dutyCycle[nIndex];
		}


		TACTL |= TASSEL_2 | MC_3 | ID_0;
		P1IFG &= ~0x04;
		//TACCTL1 &= ~CCIFG;
	}
}
