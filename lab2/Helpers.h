/*
 * Helpers.h
 *
 *  Created on: Nov 13, 2013
 *      Author: swmaung
 */

#ifndef HELPERS_H_
#define HELPERS_H_

//-------------------------------------------------------------------------
#define BYTE    unsigned char
#define UINT16  unsigned int
#define UINT32  unsigned long
//-------------------------------------------------------------------------
// data_file Data Structure
typedef struct {
  BYTE  Name[11];   //FileName
  BYTE  Attr;       //File attribute tag
  UINT32  Clus;     //First cluster of file data
  UINT32  FileSize;  //Size of the file in bytes
  UINT32  Sector;   //First sector of file data
  UINT32  Posn;      //FilePtr byte absolute to the file
} data_file;





#endif /* HELPERS_H_ */
