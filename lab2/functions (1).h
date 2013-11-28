/*
 * functions.h
 *
 *  Created on: Nov 13, 2013
 *      Author: swmaung
 */
/*
#include "alt_types.h"
#include <stdio.h>
#include <unistd.h>
#include "system.h"
#include "sys/alt_irq.h"
#include "altera_avalon_pio_regs.h"

#include "basic_io.h"
#include "LCD.h"
#include "SD_Card.h"
#include "fat.h"
#include "wm8731.h"
*/

#ifndef   __functions_H__
#define   __functions_H__

#include "Helpers.h"
#include "basic_io.h"


#define CC_SIZE 3500
extern data_file df; //struct store the data file properties
extern int cluster_chain[CC_SIZE]; //array of the clusters
extern BYTE buffer[512]; //array for reading sectors

extern data_file songArray[20]; // check to make sure that we can initialize an array without a size
extern int currentFile; //keep track of the current song displayed
extern volatile alt_u8 playSongOn; // to check that the song is playing
extern int totalFiles; // count hold the total number of files on the SD card
extern volatile alt_u8 switchSong; // after song has stopped, if next/prev pressed, will exit stop loop

extern volatile alt_u8 buttons; //used to check pushbutton is pressed in interrupt
extern volatile alt_u8 buttonZeroPressed; //check if pushbutton0 (stop) is pressed
extern volatile alt_u8 switches; //switches for the DIP values

//Values used to check the dip switches- only change display if dip switch changes
extern volatile alt_u8 prev;
extern volatile alt_u8 curr;

/* LCD Related Prototype */
//Methods used to traverse the SD card songs and to change the play modes
extern void playSong(void);
extern void halfSpeed(void);
extern void doubleSpeed(void);
extern void reverseMode(void);
extern void delayMode(void);
#endif
