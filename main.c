#include <msp430x22x4.h>
#include <math.h>

// Prototypes
void vUSCI_A0_UART_SendBytes(const unsigned char * pucData,
                             unsigned int unCount);
void initClocks();
void vUSCI_A0_UART_Init();
void vUSCI_A0_UART_ClearRXBuffer(unsigned char ucWipeData);

// Globals
// These are used by the USCI_A0 RX ISR for storing incoming characters.
// See the ISR for more details (usci_uart.c)
volatile unsigned char g_ucaUSCI_A0_RXBuffer[0x100];
volatile unsigned char g_ucUSCI_A0_RXBufferIndex = 0;
char prompt[] = "\r\nEnter LFSR Seed (in binary): ";
char invalidInputErr[] = "\r\nOnly 0 and 1 are valid characters\r\n";
volatile char isValid;
volatile char isDone = 0;

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
    initClocks();
    vUSCI_A0_UART_Init();

	// Begin main
	// Loop forever
	while(1) {
		// Assume true until proven otherwise
		isValid = 1;
		isDone = 0;
		g_ucUSCI_A0_RXBufferIndex = 0;
		// Send prompt (“\r\nEnter LFSR Seed”) to PC
		vUSCI_A0_UART_SendBytes(prompt, sizeof(prompt));

		// wait until desired number of characters from PC
		// The user has not pressed 'Enter' and the buffer is not full
		while(!isDone) {
			__bis_SR_register(GIE);       // Enter LPM0, interrupts enabled
		}


		int i;
		// convert seed to unsigned char (why?)
		unsigned int seed = 0;
		unsigned char bit = 0;
		for(i=0; i<g_ucUSCI_A0_RXBufferIndex; ++i) {
			isValid = 1;
			// check for invalid values (!= 0 or 1)
			if(g_ucaUSCI_A0_RXBuffer[i] != 0 && g_ucaUSCI_A0_RXBuffer[i] != 1) {
				isValid = 0;
				break;
			}
			seed <<= 1;
			seed |= g_ucaUSCI_A0_RXBuffer[i];
		}

		if(!isValid) {
			vUSCI_A0_UART_SendBytes(invalidInputErr, sizeof(invalidInputErr));
			vUSCI_A0_UART_ClearRXBuffer(0);
			continue;
		}

	// initialize LFSR
	// ********** Big Loop #2 **********//
	// roll the LFSR (sequence period) times
		int a, b, c, d;
		switch(g_ucUSCI_A0_RXBufferIndex) {
			case 3:
				a = 3;
				b = 2;
				c = -1;
				d = -1;
				break;
			case 4:
				a = 4;
				b = 3;
				c = -1;
				d = -1;
				break;
			case 5:
				a = 5;
				b = 3;
				c = -1;
				d = -1;
				break;
			case 6:
				a = 6;
				b = 5;
				c = -1;
				d = -1;
				break;
			case 7:
				a = 7;
				b = 6;
				c = -1;
				d = -1;
				break;
			case 8:
				a = 8;
				b = 6;
				c = 5;
				d = 4;
				break;
			case 9:
				a = 9;
				b = 5;
				c = -1;
				d = -1;
				break;
			case 10:
				a = 10;
				b = 7;
				c = -1;
				d = -1;
				break;
			case 11:
				a = 11;
				b = 9;
				c = -1;
				d = -1;
				break;
			case 12:
				a = 12;
				b = 6;
				c = 4;
				d = 1;
				break;
			case 13:
				a = 13;
				b = 4;
				c = 3;
				d = 1;
				break;
			case 14:
				a = 14;
				b = 5;
				c = 3;
				d = 1;
				break;
			case 15:
				a = 15;
				b = 14;
				c = -1;
				d = -1;
				break;
			case 16:
				a = 16;
				b = 15;
				c = 13;
				d = 4;
				break;
			}
		a = g_ucUSCI_A0_RXBufferIndex - a;
		b = g_ucUSCI_A0_RXBufferIndex - b;
		c = g_ucUSCI_A0_RXBufferIndex - c;
		d = g_ucUSCI_A0_RXBufferIndex - d;

		int j;
		for(j=0; j<pow(2, g_ucUSCI_A0_RXBufferIndex)-1; j++) {
			// Convert seed bits to an array of bytes
			char foo[0x100];
			for(i=0; i<g_ucUSCI_A0_RXBufferIndex; ++i) {
				foo[g_ucUSCI_A0_RXBufferIndex-i-1] = ((seed >> i) & 1) + '0';
			}
			vUSCI_A0_UART_SendBytes("\r\n", sizeof("\r\n"));
			// Send seed to PC
			vUSCI_A0_UART_SendBytes(foo, g_ucUSCI_A0_RXBufferIndex);

			// Roll the LFSR
			bit  = (seed >> a) ^ (seed >> b);

			// If the c or d value is greater than the index value, it means when they
			// were marked as unused (-1) for a LFSR with the current size n
			if(c > g_ucUSCI_A0_RXBufferIndex) {
				bit ^= (seed >> c);
			}
			if(d > g_ucUSCI_A0_RXBufferIndex) {
				bit ^= (seed >> d);
			}
			bit &= 1;
			seed =  (seed >> 1) | (bit << (g_ucUSCI_A0_RXBufferIndex-1));
		}
	// Send terminator sequence “\r\n” (twice)
	vUSCI_A0_UART_SendBytes("\r\n", sizeof("\r\n"));
	vUSCI_A0_UART_SendBytes("\r\n", sizeof("\r\n"));
	// ********** End Big Loop #2 **********//
		__bis_SR_register(GIE);       // Enter LPM0, interrupts enabled
	} // END loop forever
	// END main
}

void initClocks() {
	// Good clock configuration
	// Need to define in order to define set up of USCI_A0
	// 16 MHz DCO
	// 16 MHz MCLK
	// 4 MHz SMCLK
	DCOCTL = CALDCO_16MHZ;
	BCSCTL1 = CALBC1_16MHZ;
	BCSCTL2 = SELM_0 | DIVM_0 | DIVS_2;
}

// Initialize USCI in UART mode
void vUSCI_A0_UART_Init() {
	// Hold USCI_A0 in reset
	UCA0CTL1 |= UCSWRST; // ??
	// Set up UCACTL0: Async UART mode, No parity, LSB first, 8-bit char length, 1 stop bit
	UCA0CTL0 = 0x00;

	// Set up UCA0CTL1:
	// - Clock source (SMCLK)
	// - No interrupts on Rx error break chars
	// - Not dormant
	// - Next frame is data (not address), not break
	UCA0CTL1 |= UCSSEL_2;
	UCA0CTL1 &= ~(UCRXEIE | UCBRKIE | UCDORM | UCTXADDR | UCTXBRK);

	// Program baud rate: low-frequency mode (~UCOS16)
	// Baud rate = SMCLK / 0x01A0 = ~9600
	// See Table 15-4 MSP430x2xx Family User's Guide (0x01A0=416)
	UCA0BR1 = 0x01;
	UCA0BR0 = 0xA0;
	UCA0MCTL |= UCBRS_6;
	UCA0MCTL &= ~UCOS16; // ??

	// Turn off loopback mode and clear status register
	UCA0STAT = 0; // ??
	// Disable IrDA encoding
	UCA0IRTCTL &= ~UCIREN;
	UCA0IRRCTL &= ~UCIRRXFE;
	// Turn off auto-baud rate detection
	UCA0ABCTL &= ~UCABDEN; // ??


	// Configure pins
	P3SEL |= (BIT4 | BIT5);
	// Release from reset
	UCA0CTL1 &= ~UCSWRST; // ??
	// Enable RX interrupt
	IE2 |= UCA0RXIE; // ??
}

void vUSCI_A0_UART_ClearRXBuffer(unsigned char ucWipeData) {

}

/////////////////////////////////////////////////////////////////////////
// USCI_A0_UART_SendBytes( pucData: * uint8, unCount : uint16)
//
// Send unCount bytes pointed to by pucData on USCI_A0
/////////////////////////////////////////////////////////////////////////
void vUSCI_A0_UART_SendBytes(const unsigned char * pucData,
                             unsigned int unCount)
{
	for (; unCount > 0; --unCount)
	{
		while (!(IFG2 & UCA0TXIFG));
		UCA0TXBUF = *pucData;
		++pucData;
	}
}

//  Echo back RXed character, confirm TX buffer is ready first
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{
	while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
	if(UCA0RXBUF == '\r') {
		// The user pressed "Enter"
		vUSCI_A0_UART_SendBytes("\r\n", sizeof("\r\n"));
		isDone = 1;
	} else if(g_ucUSCI_A0_RXBufferIndex > 0 && UCA0RXBUF == 8 || UCA0RXBUF == 127 ) {
		// The user pressed backspace or delete
		g_ucUSCI_A0_RXBufferIndex -= 1;
		UCA0TXBUF = UCA0RXBUF;                    // TX -> RXed character
	} else {
		// Assuming the character is a digit, convert from ascii value to numerical value before storing in the buffer
		g_ucaUSCI_A0_RXBuffer[g_ucUSCI_A0_RXBufferIndex++] = UCA0RXBUF - '0';
		UCA0TXBUF = UCA0RXBUF;                    // TX -> RXed character
	}
}
