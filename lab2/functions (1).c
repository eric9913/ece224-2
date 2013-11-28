#include <stdio.h>
#include <unistd.h>
#include "system.h"
#include "sys/alt_irq.h"
#include "altera_avalon_pio_regs.h"*/
#include "functions.h"
#include "LCD.h"
#include "SD_Card.h"
#include "fat.h"
#include "wm8731.h"

#define CC_SIZE 3500
data_file df;
int cluster_chain[CC_SIZE]; //what size? //array of the cluster
BYTE buffer[512] = {0}; //array for reading lba or sectors
int currentFile = 0; //set currentFile to zero
data_file songArray[] = {NULL} ; //need to figure out how many songs
volatile alt_u8 playSongOn = 0;

volatile alt_u8 buttons; //pushbutton interrupt
volatile alt_u8 buttonZeroPressed = 0; // set buttonZeroPressed to 0 (0 is not pressed, 1 is pressed)
volatile alt_u8 switches; // initialize switches variable
int totalFiles = 0; // initialize the totalFiles to 0
volatile alt_u8 switchSong = 0;
volatile alt_u8 prev = 255; // set to value not displayed on switches
volatile alt_u8 curr;

//Method to play the song on the SD cards
void playSong()
{
    //Read the value of the switches to display the mode
    switches = IORD(SWITCH_PIO_BASE, 0);
    alt_u8 tempSwitch;
    tempSwitch = switches & 0x7;

    LCD_File_Buffering(songArray[currentFile].Name); //display that it is buffering
    build_cluster_chain(cluster_chain,CC_SIZE, &songArray[currentFile]); //Build the cluster chain
    LCD_Display(songArray[currentFile].Name, tempSwitch); // display song name and play mode

    //If switch is 0 play normal mode, if switch is 1 play half speed (use same function)
    if(tempSwitch == 0 || tempSwitch == 2)
    {
        int numOfSector = 0; // initialize number of sectors to start playing from the 1st one
        int i; // current location in buffer

        // while it can still return a valid sector, it will play the song
        while(get_rel_sector(&songArray[currentFile], buffer, cluster_chain, numOfSector)==0)
        {
            for(i = 0; i < BPB_BytsPerSec; i+=2)
            {
                UINT16 tmp; //Create a 16-bit variable to pass to the FIFO

                if( !IORD(AUD_FULL_BASE, 0) ) // Check if the FIFO is not full
                {
                    //Package 2 8-bit bytes from the sector buffer array into the single 16-bit variable tmp
                    tmp = (buffer[i+1] << 8 | (buffer[i]));

                    //while the stop button is pressed wait, until the song is switched
                    while(buttonZeroPressed == 1){
                        if(switchSong == 1){
                            goto out; //exits the play loop
                        }
                    }
                    //Write the 16-bit variable tmp to the FIFO where it will be processed by the audio CODEC
                    IOWR(AUDIO_0_BASE, 0, tmp);
                    //If half speed mode, will play the audio again
                    if(tempSwitch == 2){
                        IOWR(AUDIO_0_BASE, 0, tmp);
                    }
                    //wait for the buffer to be empty
                    while(IORD(AUD_FULL_BASE, 0)){}
                }
            }
            numOfSector++;
        }
        out:
        //reset the values of switchSong and PlaySongOn
        switchSong = 0;
        playSongOn = 0;
    }
    //If DIP switches set to 1 play song double speed
    if(tempSwitch == 1){
        doubleSpeed();
    }
    //If DIP switches set to 3 play song in delay mode
    if(tempSwitch == 3){
        delayMode();
    }
    //If DIP switches set to 4 play song in reverse
    if(tempSwitch == 4){
        reverseMode();
    }
}

//Play the song at double speed
void doubleSpeed()
{
    int numOfSector = 0;
    int i; // current location in buffer
    // while it can still return a valid sector, it will play the song
    while(get_rel_sector(&songArray[currentFile], buffer, cluster_chain, numOfSector)==0)
    {
        for(i = 0; i < BPB_BytsPerSec; i+=8)
        {
            UINT16 tmp; //Create a 16-bit variable to pass to the FIFO
            if( !IORD(AUD_FULL_BASE, 0) ) //Check if the FIFO is not full
            {
                while(buttonZeroPressed == 1){
                    if(switchSong == 1){
                        goto out;
                    }
                } //to stop playing the current file

                // Play the left audio
                tmp = (buffer[i+1] << 8 | (buffer[i]));
                while(IORD(AUD_FULL_BASE, 0)){}
                IOWR(AUDIO_0_BASE, 0, tmp);

                // Play the right audio
                tmp = (buffer[i+3] << 8 | (buffer[i+2]));
                while(IORD(AUD_FULL_BASE, 0)){}
                IOWR(AUDIO_0_BASE, 0, tmp);

                // Play 0s to left and right side (act if skipping playing the song - only way to play stereo)
                tmp = (buffer[i+5] << 8 | (buffer[i+4]));
                while(IORD(AUD_FULL_BASE, 0)){}
                IOWR(AUDIO_0_BASE, 0, 0);

                tmp = (buffer[i+7] << 8 | (buffer[i+6]));
                while(IORD(AUD_FULL_BASE, 0)){}
                IOWR(AUDIO_0_BASE, 0, 0);

                //while(IORD(AUD_FULL_BASE, 0)){}
            }
        }
        numOfSector++;
    }
    out:
    switchSong = 0;
    playSongOn = 0;
}

//Play the song in reverse
void reverseMode(){
    //Start from the last sector in the file
    int numOfSector = (ceil(songArray[currentFile].FileSize / BPB_BytsPerSec))-2;
    int i; // current location in buffer

    while(get_rel_sector(&songArray[currentFile], buffer, cluster_chain, numOfSector)==0)
    {
        // Start from end of the buffer to play audio in reverse
        for(i = 508; i > 0; i-=4)
        {
            UINT16 tmp; //Create a 16-bit variable to pass to the FIFO

            if( !IORD(AUD_FULL_BASE, 0) ) //Check if the FIFO is not full
            {
                while(buttonZeroPressed == 1){
                    if(switchSong == 1){
                        goto out;
                    }
                } //to stop playing the current file

                //Play the left audio
                tmp = (buffer[i+1] << 8 | (buffer[i]));
                while(IORD(AUD_FULL_BASE, 0)){}
                IOWR(AUDIO_0_BASE, 0, tmp);
                //Play Right audio
                tmp = (buffer[i+3] << 8 | (buffer[i+2]));
                while(IORD(AUD_FULL_BASE, 0)){}
                IOWR(AUDIO_0_BASE, 0, tmp);
            }
        }
        numOfSector--;
    }
    out:
    switchSong = 0;
    playSongOn = 0;

}

//Play the song in delay mode
void delayMode()
{
    BYTE tmpBuffer[88200] = {0}; // initialize temporary buffer to hold the right side of the audio
    int posn = 0; // posistion in buffer
    int delayFirst = 0; // To check if its the 1st second
    int inBuff = 0;

    int numOfSector = 0;
    int totalSectors = (ceil(songArray[currentFile].FileSize / BPB_BytsPerSec))-3;
    //printf("file = %d",totalSectors);
    int i; // current location in buffer
    //printf("BytesPerSector: %d\n", BPB_BytsPerSec);
    while(get_rel_sector(&songArray[currentFile], buffer, cluster_chain, numOfSector)==0)
    {
        for(i = 0; i < BPB_BytsPerSec; i+=4)
        {
            UINT16 tmp; //Create a 16-bit variable to pass to the FIFO

            if( !IORD(AUD_FULL_BASE, 0) ) //Check if the FIFO is not full
            {

                while(buttonZeroPressed == 1){
                    if(switchSong == 1)
                    {
                        goto out;
                    }
                } //to stop playing the current file

                //Play to left side
                tmp = (buffer[i+1] << 8 | (buffer[i]));
                while(IORD(AUD_FULL_BASE, 0)){}
                IOWR(AUDIO_0_BASE, 0, tmp);

                //If at end of right buffer, set delayFirst to 1 and reset position
                if(posn == 88200){
                    delayFirst = 1;
                    posn = 0;
                    inBuff = 0;
                }
                // If its the 1st second, play 0s to the right side
                if(delayFirst == 0){
                    while(IORD(AUD_FULL_BASE, 0)){}
                    IOWR(AUDIO_0_BASE, 0, 0);
                }
                // After the 1 second, play audio from the right buffer
                else{
                    tmp = (tmpBuffer[inBuff+1] << 8 | (tmpBuffer[inBuff]));
                    while(IORD(AUD_FULL_BASE, 0)){}
                    IOWR(AUDIO_0_BASE, 0, tmp);

                }
                //Store the right side audio to the buffer
                tmpBuffer[posn] = buffer[i+2];
                tmpBuffer[posn+1] = buffer[i+3];
                /*if(numOfSector == totalSectors){

                    	while(inBuff < 88200){
									while(buttonZeroPressed == 1){
                    	                    if(switchSong == 1)
                    	                    {
                    	                        goto out;
                    	                    }
                    	                } //to stop playing the current file
										inBuff++;
                    	tmp = (tmpBuffer[inBuff+1] << 8 | (tmpBuffer[inBuff]));
                    	                    while(IORD(AUD_FULL_BASE, 0)){}
                    	                    IOWR(AUDIO_0_BASE, 0, tmp);


                    	}
                    }*/
                while(IORD(AUD_FULL_BASE, 0)){}
            }
            inBuff+=2;
            posn+=2;
        }
        numOfSector++;
    }

    inBuff = 0;
    UINT16 tmp;
    tmp = (buffer[i+1] << 8 | (buffer[i]));
        	while(inBuff < 88200){
        		if( !IORD(AUD_FULL_BASE, 0) ) //Check if the FIFO is not full
        		            {
						while(buttonZeroPressed == 1){
        	                    if(switchSong == 1)
        	                    {
        	                        goto out;
        	                    }
        	                } //to stop playing the current file
							inBuff+=2;
							while(IORD(AUD_FULL_BASE, 0)){}
							                    IOWR(AUDIO_0_BASE, 0, 0);
        	tmp = (tmpBuffer[inBuff+1] << 8 | (tmpBuffer[inBuff]));
        	                    while(IORD(AUD_FULL_BASE, 0)){}
        	                    IOWR(AUDIO_0_BASE, 0, tmp);
        		            }

        	}


    out:
    //printf("numOfSector = %d\n", numOfSector);
    //printf("file = %d", totalSectors);
    switchSong = 0;
    playSongOn = 0;

}
