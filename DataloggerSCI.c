/********************************************************************************//**
 * \file DataloggerSCI.c
 * 
 * \author Roman Holderried
 *
 * \brief SCI interface functions.
 *
 * <b> History </b>
 *      - 2022-04-27 - File creation.
 *                     
 ***********************************************************************************/
#define SCI
#ifdef SCI
/************************************************************************************
 * Includes
 ***********************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "DataloggerSCI.h"
#include "Datalogger.h"
#include "SCI.h"
/************************************************************************************
 * Defines
 ***********************************************************************************/



/************************************************************************************
 * Function definition
 ***********************************************************************************/
COMMAND_CB_STATUS RegisterLogFromVarStruct (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    // Take over the arguments
    uint16_t ui16VarNum = (uint16_t)ui32ValArray[0];
    uint8_t ui8LogNum = (uint8_t)ui32ValArray[1];
    uint16_t ui16FreqDiv = (uint8_t)ui32ValArray[2];
    uint32_t ui32RecLen = ui32ValArray[3];
    VAR *pVar = NULL;
    tDATALOG_ERROR eDlogError = eDATALOG_ERROR_NONE;

    if (SCI_GetVarFromStruct((int16_t)ui16VarNum, &pVar))
        ;
    else
        return eCOMMAND_STATUS_ERROR;

    
}
#endif
// EOF
