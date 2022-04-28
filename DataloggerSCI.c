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
#include <stddef.h>
#include "DataloggerSCI.h"
#include "DataloggerConfig.h"
#include "Datalogger.h"
#include "SCI.h"
/************************************************************************************
 * Defines
 ***********************************************************************************/
#define GET_ERROR_NUMBER(e) (e + DATALOGGER_ERROR_OFFSET)

/************************************************************************************
 * Globals
 ***********************************************************************************/
extern const uint8_t ui8_byteLength[];
uint32_t ui32ReturnValBuffer[SIZE_OF_RETURN_VAL_BUFFER];

/************************************************************************************
 * Function definitions
 ***********************************************************************************/
COMMAND_CB_STATUS RegisterLogFromVarStruct (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    // Take over the arguments
    uint16_t ui16VarNum = (uint16_t)ui32ValArray[0];
    uint8_t ui8ChNum = (uint8_t)ui32ValArray[1];
    uint16_t ui16FreqDiv = (uint8_t)ui32ValArray[2];
    uint32_t ui32RecLen = ui32ValArray[3];
    VAR *pVar = NULL;
    tDATALOG_ERROR eDlogError = eDATALOG_ERROR_NONE;


    if (SCI_GetVarFromStruct((int16_t)ui16VarNum, &pVar))
    {
        // Register the log by using the channel number as identifier
        eDlogError = RegisterLog(ui8ChNum, ui8ChNum, ui16FreqDiv, ui32RecLen, (uint8_t*)pVar->val, ui8_byteLength[pVar->datatype]);
    }
    else
        return eCOMMAND_STATUS_ERROR;
    
    if (eDlogError == eDATALOG_ERROR_NONE)
        return eCOMMAND_STATUS_SUCCESS;
    else
    {
        pInfo->ui16_error = GET_ERROR_NUMBER((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}

//=============================================================================
COMMAND_CB_STATUS InitializeDatalogger (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    tDATALOG_ERROR eDlogError = eDATALOG_ERROR_NONE;


    eDlogError = DatalogInitialize();
    
    if (eDlogError == eDATALOG_ERROR_NONE)
        return eCOMMAND_STATUS_SUCCESS;
    else
    {
        pInfo->ui16_error = GET_ERROR_NUMBER((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}

//=============================================================================
COMMAND_CB_STATUS StartDatalogger (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    tDATALOG_ERROR eDlogError = eDATALOG_ERROR_NONE;

    eDlogError = DatalogStart();
    
    if (eDlogError == eDATALOG_ERROR_NONE)
        return eCOMMAND_STATUS_SUCCESS;
    else
    {
        pInfo->ui16_error = GET_ERROR_NUMBER((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}

//=============================================================================
COMMAND_CB_STATUS StopDatalogger (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    tDATALOG_ERROR eDlogError = eDATALOG_ERROR_NONE;

    eDlogError = DatalogStop();
    
    if (eDlogError == eDATALOG_ERROR_NONE)
        return eCOMMAND_STATUS_SUCCESS;
    else
    {
        pInfo->ui16_error = GET_ERROR_NUMBER((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}

//=============================================================================
COMMAND_CB_STATUS GetLogData (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    tDATALOG_ERROR eDlogError = eDATALOG_ERROR_NONE;
    uint8_t *pui8Data = NULL;
    uint32_t ui32MemLen = 0;

    eDlogError = DatalogGetDataPtr(&pui8Data, &ui32MemLen);
    
    if (eDlogError == eDATALOG_ERROR_NONE)
    {
        pInfo->pui8_upStreamBuf = pui8Data;
        pInfo->ui32_datLen = ui32MemLen;
        return eCOMMAND_STATUS_SUCCESS_UPSTREAM;
    }
    else
    {
        pInfo->ui16_error = GET_ERROR_NUMBER((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}

//=============================================================================
COMMAND_CB_STATUS GetChannelInfo (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    tDATALOG_ERROR eDlogError = eDATALOG_ERROR_NONE;
    tDATALOG_CHANNEL ChInfo;
    // Take over the arguments
    uint8_t ui8ChNum = (uint8_t)ui32ValArray[0];
    // uint8_t ui8ReturnSize = 0;

    eDlogError = DatalogGetChannelInfo(&ChInfo, ui8ChNum);
    
    if (eDlogError == eDATALOG_ERROR_NONE)
    {
        ui32ReturnValBuffer[0] = ChInfo.ui32ChID;
        ui32ReturnValBuffer[1] = ChInfo.ui16Divider;
        ui32ReturnValBuffer[2] = ChInfo.ui32RecordLength;
        ui32ReturnValBuffer[3] = ChInfo.ui32CurrentCount;
        ui32ReturnValBuffer[4] = ChInfo.ui32MemoryOffset;
        pInfo->pui32_dataBuf = &ui32ReturnValBuffer;
        pInfo->ui32_datLen = 5;

        return eCOMMAND_STATUS_SUCCESS_DATA;
    }
    else
    {
        pInfo->ui16_error = GET_ERROR_NUMBER((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}

//=============================================================================
COMMAND_CB_STATUS ResetDatalogger (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    tDATALOG_ERROR eDlogError = DataloggerReset();
    
    if (eDlogError == eDATALOG_ERROR_NONE)
    {
        return eCOMMAND_STATUS_SUCCESS;
    }
    else
    {
        pInfo->ui16_error = GET_ERROR_NUMBER((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}
#endif
// EOF
