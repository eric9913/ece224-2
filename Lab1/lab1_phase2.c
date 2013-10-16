// lab1_phase2.c

void init(int, int);
void background(int);
void finalize(void);
#include "system.h"
#include <io.h>

// eventCounter to count the number of times the event occurs for the Periodic Polling
// eventCounterPulse to count the number of times the event occurs for Interrupt method
volatile int eventCounter = 0;
volatile int eventCounterPulse = 0;

// Periodic Polling using timer_ISR
#ifdef TIMER_1_BASE
static void timer_ISR(void* context, alt_u32 id)
{
   // acknowledge the interrupt by clearing the TO bit in the status register
   IOWR(TIMER_1_BASE, 0, 0x0);
   // only write the current value to the RESPONSE_BASE when the values of PULSE_BASE and RESPONSE_BASE are different
   if(IORD(PIO_PULSE_BASE, 0) == 0 && IORD(PIO_RESPONSE_BASE, 0) == 1){
   		IOWR(PIO_RESPONSE_BASE, 0, 0);
   		eventCounter++;
   }
   if(IORD(PIO_PULSE_BASE, 0) == 1 && IORD(PIO_RESPONSE_BASE, 0) == 0){
      		IOWR(PIO_RESPONSE_BASE, 0, 1);
   }
}
#endif

// Interrupt method using pulse_ISR
#ifdef PIO_PULSE_BASE
static void pulse_ISR(void* context, alt_u32 id)
{
	   if(IORD(PIO_PULSE_BASE, 0) == 0 && IORD(PIO_RESPONSE_BASE, 0) == 1){
	   		IOWR(PIO_RESPONSE_BASE, 0, 0);
	   		eventCounterPulse++;
	   }
	   if(IORD(PIO_PULSE_BASE, 0) == 1 && IORD(PIO_RESPONSE_BASE, 0) == 0){
	      		IOWR(PIO_RESPONSE_BASE, 0, 1);
	   }
	   // acknowledge the interrupt by clearing the TO bit in the status register
	   IOWR(PIO_PULSE_BASE, 3, 0x0);
}
#endif

int main(void){
	int i;	// different values for period
	int j;	// different values for duty cycles
	int k;	// different values for background task

	for(i=1; i<=14; i+=2){
		for(j =1; j<=14; j+=3){
			for(k = 10; k<=10000; k*=10){
				/* ----Periodic Polling with Timer Interrupt---- */
				init(i, j);
				#ifdef TIMER_1_BASE
				alt_u32 timerPeriod;  // 32 bit period used for timer
				// calculate timer period for 20 micro seconds
				timerPeriod = TIMER_1_FREQ/50000;
				// initialize timer interrupt vector
				alt_irq_register(TIMER_1_IRQ, (void*)0, timer_ISR);
				// initialize timer period
				IOWR(TIMER_1_BASE, 2, (alt_u16)timerPeriod);
				IOWR(TIMER_1_BASE, 3, (alt_u16)(timerPeriod >> 16));
				// clear timer interrupt bit in status register
				IOWR(TIMER_1_BASE, 0, 0x0);
				// initialize timer control - start timer, run continuously, enable interrupts
				IOWR(TIMER_1_BASE, 1, 0x7);
				#endif
				printf("Periodic Polling Period:%d DutyCycle:%d BkgTsk:%d \n", i,j,k);
				eventCounter = 0;
				while(eventCounter < 100){
					background(k);
				}
				finalize();
				usleep(100);

				/* ----Pulse Interrupt---- */
				init(i, j);
				// set up the interrupt vector
				alt_irq_register(PIO_PULSE_IRQ, (void*)0, pulse_ISR );
				// reset the edge capture register by writing to it (any value will do)
				IOWR(PIO_PULSE_BASE, 3, 0x0);
				// enable interrupts for all four buttons
				IOWR(PIO_PULSE_BASE, 2, 0x1);
				printf("Interrupt Period:%d DutyCycle:%d BkgTsk:%d \n", i,j,k);
				eventCounterPulse = 0;
				while(eventCounterPulse < 100){
					background(k);
				}
				finalize();
				usleep(100);
			}
		}
	}
}
