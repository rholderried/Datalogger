/********************************************************************************//**
 * \file RobLogIF.h
 * \author Roman Holderried
 *
 * \brief Typedefs, function declarations for the RobLog interface
 *
 * <b> History </b>
 *      - 2021-05-01 - File creation.
 *                     
 ***********************************************************************************/
/************************************************************************************
 * Includes
 ***********************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "RobLog.h"
/************************************************************************************
 * Defines
 ***********************************************************************************/
/************************************************************************************
 * Type definitions
 ***********************************************************************************/

typedef enum 
{
    eROBLOGIF_IDLE,
    eROBLOGIF_BUSY,
    eROBLOGIF_ERROR
}tROBLOGIF_NOTIFICATION;

typedef enum 
{
    eROBLOGIF_ERROR_NONE
}tROBLOGIF_ERROR;
/************************************************************************************
 * Function declarations
 ***********************************************************************************/
bool initiateDatalogReadout (uint8_t *pBuf, uint16_t ui16NoOfBytes);
bool memoryWrite(uint8_t *pBuf, uint32_t ui32Address, uint16_t ui16NoOfBytes); 
tROBLOGIF_NOTIFICATION checkMemoryInterfaceState(void);


// EOF
