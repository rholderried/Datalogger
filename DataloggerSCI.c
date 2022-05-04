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
#define DATALOGGER_SCI_ERROR(e) (e + DATALOGGER_ERROR_OFFSET)

/************************************************************************************
 * Globals
 ***********************************************************************************/
extern const uint8_t ui8_byteLength[];
uint32_t ui32ReturnValBuffer[SIZE_OF_RETURN_VAL_BUFFER];

/************************************************************************************
 * Function definitions
 ***********************************************************************************/
COMMAND_CB_STATUS GetDataloggerVersion (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    tDATALOGGER_VERSION sVer = Datalogger_GetVersion();

    pInfo->ui32_datLen = 3;
    ui32ReturnValBuffer[0] = sVer.ui8VersionMajor;
    ui32ReturnValBuffer[1] = sVer.ui8VersionMinor;
    ui32ReturnValBuffer[2] = sVer.ui8Revision;
    pInfo->pui32_dataBuf = &ui32ReturnValBuffer;

    return eCOMMAND_STATUS_SUCCESS_DATA;
}

//=============================================================================
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
        eDlogError = Datalogger_RegisterLog(ui8ChNum, ui8ChNum, ui16FreqDiv, ui32RecLen, (uint8_t*)pVar->val, ui8_byteLength[pVar->datatype]);
    }
    else
        return eCOMMAND_STATUS_ERROR;
    
    if (eDlogError == eDATALOG_ERROR_NONE)
        return eCOMMAND_STATUS_SUCCESS;
    else
    {
        pInfo->ui16_error = DATALOGGER_SCI_ERROR((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}

//=============================================================================
COMMAND_CB_STATUS InitializeDatalogger (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    tDATALOG_ERROR eDlogError = eDATALOG_ERROR_NONE;


    eDlogError = Datalogger_InitLogger(true);
    
    if (eDlogError == eDATALOG_ERROR_NONE)
        return eCOMMAND_STATUS_SUCCESS;
    else
    {
        pInfo->ui16_error = DATALOGGER_SCI_ERROR((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}

//=============================================================================
COMMAND_CB_STATUS StartDatalogger (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    tDATALOG_ERROR eDlogError = eDATALOG_ERROR_NONE;

    eDlogError = Datalogger_Start();
    
    if (eDlogError == eDATALOG_ERROR_NONE)
        return eCOMMAND_STATUS_SUCCESS;
    else
    {
        pInfo->ui16_error = DATALOGGER_SCI_ERROR((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}

//=============================================================================
COMMAND_CB_STATUS StopDatalogger (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    tDATALOG_ERROR eDlogError = eDATALOG_ERROR_NONE;

    eDlogError = Datalogger_Stop();
    
    if (eDlogError == eDATALOG_ERROR_NONE)
        return eCOMMAND_STATUS_SUCCESS;
    else
    {
        pInfo->ui16_error = DATALOGGER_SCI_ERROR((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}

//=============================================================================
COMMAND_CB_STATUS GetLogData (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    tDATALOG_ERROR eDlogError = eDATALOG_ERROR_NONE;
    uint8_t *pui8Data = NULL;
    uint32_t ui32MemLen = 0;

    eDlogError = Datalogger_GetDataPtr(&pui8Data, &ui32MemLen);
    
    if (eDlogError == eDATALOG_ERROR_NONE)
    {
        pInfo->pui8_upStreamBuf = pui8Data;
        pInfo->ui32_datLen = ui32MemLen;
        return eCOMMAND_STATUS_SUCCESS_UPSTREAM;
    }
    else
    {
        pInfo->ui16_error = DATALOGGER_SCI_ERROR((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}

//=============================================================================
COMMAND_CB_STATUS GetChannelInfo (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    tDATALOG_ERROR eDlogError = eDATALOG_ERROR_NONE;
    tDATALOG_CHANNEL sChInfo;
    // Take over the arguments
    uint8_t ui8ChNum = (uint8_t)ui32ValArray[0];
    // uint8_t ui8ReturnSize = 0;

    eDlogError = Datalogger_GetChannelInfo(&sChInfo, ui8ChNum);
    
    if (eDlogError == eDATALOG_ERROR_NONE)
    {
        ui32ReturnValBuffer[0] = sChInfo.ui32ChID;
        ui32ReturnValBuffer[1] = sChInfo.ui16Divider;
        ui32ReturnValBuffer[2] = sChInfo.ui32RecordLength;
        ui32ReturnValBuffer[3] = sChInfo.ui32CurrentCount;
        ui32ReturnValBuffer[4] = sChInfo.ui32MemoryOffset;
        pInfo->pui32_dataBuf = &ui32ReturnValBuffer;
        pInfo->ui32_datLen = 5;

        return eCOMMAND_STATUS_SUCCESS_DATA;
    }
    else
    {
        pInfo->ui16_error = DATALOGGER_SCI_ERROR((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}

//=============================================================================
COMMAND_CB_STATUS ResetDatalogger (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    tDATALOG_ERROR eDlogError = Datalogger_Reset();
    
    if (eDlogError == eDATALOG_ERROR_NONE)
    {
        return eCOMMAND_STATUS_SUCCESS;
    }
    else
    {
        pInfo->ui16_error = DATALOGGER_SCI_ERROR((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}

//=============================================================================
COMMAND_CB_STATUS RemoveLog (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    uint8_t ui8ChNum = (uint8_t)ui32ValArray[0];

    tDATALOG_ERROR eDlogError = Datalogger_RemoveLog(ui8ChNum);
    
    if (eDlogError == eDATALOG_ERROR_NONE)
    {
        return eCOMMAND_STATUS_SUCCESS;
    }
    else
    {
        pInfo->ui16_error = DATALOGGER_SCI_ERROR((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}

//=============================================================================
COMMAND_CB_STATUS SetOpMode (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)
{
    uint32_t ui32OpMode = ui32ValArray[0];

    tDATALOG_ERROR eDlogError = Datalogger_SetOpMode(ui32OpMode);
    
    if (eDlogError == eDATALOG_ERROR_NONE)
    {
        return eCOMMAND_STATUS_SUCCESS;
    }
    else
    {
        pInfo->ui16_error = DATALOGGER_SCI_ERROR((uint16_t)eDlogError);
        return eCOMMAND_STATUS_ERROR;
    }
}
#endif
// EOF
