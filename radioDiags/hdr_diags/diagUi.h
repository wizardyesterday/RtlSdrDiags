//**************************************************************************
// file name: diagUi.h
//**************************************************************************

#ifndef __DIAGUI__
#define __DIAGUI__

/************************************************************************/
/* general defines                                                      */
/************************************************************************/

/************************************************************************/
/* structure definitions                                                */
/************************************************************************/
/* structure of entry in the user interface command table */

/**************************************************************************/
/* function prototypes                                                    */
/**************************************************************************/
void diagUi_start(int networkPort);
void diagUi_stop(void);
void nprintf(FILE *s,const char *formatPtr, ...);

#endif // __DIAGUI__

