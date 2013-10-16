#include "alt_types.h"  		// define types used by Altera code, e.g. alt_u8
#include <stdio.h>
#include <unistd.h>
#include "system.h"  		   // constants such as the base addresses of any PIOs, defined in your hardware
#include "sys/alt_irq.h"  	   // required when using interrupts
#include <io.h>

// declare global variable of LED state, initially all LEDs should be off
alt_u8 led_state = (alt_u8)0x00;
volatile alt_u8 buttons;
volatile alt_u8 switches;
volatile alt_u8 switches2;
//volatile alt_u8 count_flag = (alt_u8)0x00;
volatile int countLED;
volatile int countSS;
volatile int flagLED;
volatile int flagSS;

// button_ISR
#ifdef BUTTON_PIO_BASE
#ifdef LED_PIO_BASE
static void button_ISR(void* context, alt_u32 id){
   /* get value from edge capture register and mask off all bits except
      the 4 least significant */
   buttons = IORD(BUTTON_PIO_BASE, 3) & 0xf;
   // check if button 1 is pressed
   if(buttons == 1){
	   countLED = 0;
	   switches = IORD(SWITCH_PIO_BASE, 0);	// Read the value from SW[0] to SW[7]
	   flagLED = 1;	// Set the flag to 1 so that the timer_ISR knows the interrupt has been raised
   }
   // check if button 2 is pressed
   if(buttons == 2){
	   countSS = 0;
	   switches2 = IORD(SWITCH_PIO_BASE, 0);
   	   flagSS = 1;
   }
   /* reset edge capture register to clear interrupt */
   IOWR(BUTTON_PIO_BASE, 3, 0x0);

}
#endif
#endif

// timer interrupt, raised every 1 second
#ifdef TIMER_0_BASE
static void timer_ISR(void* context, alt_u32 id)
{
   // acknowledge the interrupt by clearing the TO bit in the status register
   IOWR(TIMER_0_BASE, 0, 0x0);
   // check if the flagLED is 1 and the countLED is less than 9
   if(flagLED == 1 && countLED < 9){
	   FlashLed();	// Flash the values of the switches on the LED0
	   countLED++;
   }
   if(flagSS == 1 && countSS < 9)
   {
	   SevenSegment();	// Display the values of the switches on the HEX6 SS Display
	   countSS++;
   }
   // if count is 9, reset count and flag to 0, and turn off the LED
   if(countLED == 9){
   		flagLED = 0;
   		countLED = 0;
   		IOWR(LED_PIO_BASE, 0, 0);
   }
   if(countSS == 9){
	   flagSS = 0;
	   countSS = 0;
	   IOWR(SEVEN_SEG_PIO_BASE, 0, 0xffff);
   }
}
#endif

int main(void){
	/* set up the interrupt vector */
	alt_irq_register( BUTTON_PIO_IRQ, (void*)0, button_ISR );
	/* reset the edge capture register by writing to it (any value will do) */
	IOWR(BUTTON_PIO_BASE, 3, 0x0);
	/* enable interrupts for all four buttons*/
	IOWR(BUTTON_PIO_BASE, 2, 0xf);

	alt_u32 timerPeriod;  // 32 bit period used for timer
	#ifdef TIMER_0_BASE
	// calculate timer period for 1 second
	timerPeriod = TIMER_0_FREQ;
	// initialize timer interrupt vector
	alt_irq_register(TIMER_0_IRQ, (void*)0, timer_ISR);
	// initialize timer period
	IOWR(TIMER_0_BASE, 2, (alt_u16)timerPeriod);
	IOWR(TIMER_0_BASE, 3, (alt_u16)(timerPeriod >> 16));
	// clear timer interrupt bit in status register
	IOWR(TIMER_0_BASE, 0, 0x0);
	// initialize timer control - start timer, run continuously, enable interrupts
	IOWR(TIMER_0_BASE, 1, 0x7);

	//set the led to 0
	IOWR(LED_PIO_BASE, 0, 0);
	//set the seven seg to off
	IOWR(SEVEN_SEG_PIO_BASE, 0, 0xffff);
	while(1){

	}
	#endif
	return(0);

}

// FlashLed() flashes the values of the switches to LED0
void FlashLed(){
	if(countLED < 9){
		int tempSwitch;
		tempSwitch = switches & 0x1;	// Get the least significant bit

		#ifdef LED_PIO_BASE
		IOWR(LED_PIO_BASE, 0, tempSwitch);	// Write the value of the least significant bit to LED0
		#endif
		switches = switches >> 1;	// Right shift by 1 bit to update the least significant bit
	}
}

// SevenSegment() display the values of the switches on HEX6 SS Display
void SevenSegment(){
	if(countSS < 9){
		int tempSwitch;
		tempSwitch = switches2 & 0x1;
		#ifdef SEVEN_SEG_PIO_BASE
		if(tempSwitch == 0){ // if the value of the least significant bit is 0, then write 0xFF81 to display 0
			IOWR(SEVEN_SEG_PIO_BASE, 0, 0xFF81);
		}
		if(tempSwitch == 1){
			IOWR(SEVEN_SEG_PIO_BASE, 0, 0xFFCF);
		}
		switches2 = switches2 >> 1;	// Right shift to update the least significant bit
		#endif
	}
}
