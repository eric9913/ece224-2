
#include "functions.h"
//#include "functions.c"

//volatile alt_u8 buttons; //pushbutton interrupt
//volatile alt_u8 buttonZeroPressed = 0;

#ifdef BUTTON_PIO_BASE
static void button_ISR(void* context, alt_u32 id){
   /* get value from edge capture register and mask off all bits except
      the 4 least significant */
   buttons = IORD(BUTTON_PIO_BASE, 3) & 0xf;
   // check if button 1 is pressed
   if(buttons == 1){
	   //stop playing
	   buttonZeroPressed = 1;
	   IOWR(LED_PIO_BASE, 0, 1);

   }
   // check if button 2 is pressed
   if(buttons == 2){
	   //play file
	   buttonZeroPressed = 0;
	   //playSong();
	   IOWR(LED_PIO_BASE, 0, 0);
   }
   if(buttons == 3){
	   //go to next file
	   IOWR(LED_PIO_BASE, 0, 1);
	   //nextSong();
   }
   if(buttons == 4){
	   //go to prev file
	   IOWR(LED_PIO_BASE, 0, 0);
   }
   /* reset edge capture register to clear interrupt */
   IOWR(BUTTON_PIO_BASE, 3, 0x0);

}
#endif
int main(){

	/* set up the interrupt vector */
		alt_irq_register( BUTTON_PIO_IRQ, (void*)0, button_ISR );
		/* reset the edge capture register by writing to it (any value will do) */
		IOWR(BUTTON_PIO_BASE, 3, 0x0);
		/* enable interrupts for all four buttons*/
		IOWR(BUTTON_PIO_BASE, 2, 0xf);

		LCD_Init(); //initialize LCD

    //initialize SD card, file system, and audio codec
    SD_card_init();
    // Master Boot Record initialization
    init_mbr();
    //"Boot Sector initialization
    init_bs();
    init_audio_codec();

    IOWR(LED_PIO_BASE, 0, 0);
    //Search for the file
    //printf("The search for file returned: %d\t Name: %d\t Attr: %d\t FileSize: %d\t Sector: %d\t Position: %d\t\n",
    //		search_for_filetype("WAV", &df, 0, 1);//, df.Name, df.Attr, df.FileSize, df.Sector, df.Posn;

    //Build the cluster chain
    //build_cluster_chain(cluster_chain,CC_SIZE, &df);

    //Read a sector from a file and returns 0 if valid sector
    //printf("The sector return code: %d\n", get_rel_sector(&df, buffer, cluster_chain, 0));
/*
    int numOfSector = 0;
    int i;
    while(get_rel_sector(&df, buffer, cluster_chain, numOfSector)==0)
    {
		for(i = 0; i < BPB_BytsPerSec; i+=2)
		{
			UINT16 tmp; //Create a 16-bit variable to pass to the FIFO

			if( !IORD(AUD_FULL_BASE, 0) ) //Check if the FIFO is not full
			{
				//Package 2 8-bit bytes from the sector buffer array into the single 16-bit variable tmp
				tmp = (buffer[i+1] << 8 | (buffer[i]));
				while(buttonZeroPressed == 1){}

				//Write the 16-bit variable tmp to the FIFO where it will be processed by the audio CODEC
					IOWR(AUDIO_0_BASE, 0, tmp);

				while(IORD(AUD_FULL_BASE, 0)){}
			}
		}
		numOfSector++;
    }*/
    //printf("number of sectors: %d\n", numOfSector);
    //put into play function after
    /*search_for_filetype("WAV", &df, 0, 1);
    playSong();
    search_for_filetype("WAV", &df, 0, 1);
    playSong();*/

}
