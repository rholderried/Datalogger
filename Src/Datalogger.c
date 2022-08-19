/********************************************************************************//**
 * \file Datalogger.c
 * \author Roman Holderried
 *
 * \brief Datalogger routines.
 *
 * <b> History </b>
 *      - 2021-05-01 - File adapted from project Roboard.
 *                     
 ***********************************************************************************/

/************************************************************************************
 * Includes
 ***********************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "DataloggerCfg.h"
#include "Datalogger.h"

/************************************************************************************
 * Defines
 ***********************************************************************************/

/************************************************************************************
 * Globals
 ***********************************************************************************/

/************************************************************************************
 * Function definitions
 ***********************************************************************************/
void DataloggerInit(tDATALOGGER *psDatalog, tDATALOGGER_CALLBACKS sCallbacks)
{
    psDatalog->sCallbacks = sCallbacks;
}

//===================================================================================
tDATALOG_ERROR DataloggerReset(tDATALOGGER *psDatalog)
{
    tDATALOGGER sTemp = tDATALOGGER_DEFAULTS;

    // Check if datalogger tasks are going on
    switch(psDatalog->eDatalogState)
    {
        case eDLOGSTATE_RUNNING:
        case eDLOGSTATE_FORMAT_MEMORY:
        case eDLOGSTATE_ABORTING:
            return eDATALOG_ERROR_WRONG_STATE;
        
        default:
            break;
    }

    // Preserve the parameters and the callbacks
    sTemp.sNVPar = psDatalog->sNVPar;
    sTemp.sCallbacks = psDatalog->sCallbacks;

    memcpy(psDatalog, &sTemp, sizeof(tDATALOGGER));

    return eDATALOG_ERROR_NONE;
}

//===================================================================================
#ifdef UNITTEST
tDATALOGGER* DataloggerGetData(tDATALOGGER *psDatalog,)
{
    return psDatalog;
}
#endif

//===================================================================================
void _DataloggerClearMemory (tDATALOGGER *psDatalog)
{
    // If memory has been already allocated, free these memory portions
    for (uint8_t i = 0; i < MAX_NUM_LOGS; i++)
    {
        if (!psDatalog->sDatalogControl.ui8ActiveLoggers)
            break;

        if (psDatalog->sDatalogControl.ui8ActiveLoggers & (1 << i))
        {
            if (psDatalog->sDatalogControl.eOpMode == eOPMODE_RECMODERAM)
                free(psDatalog->sDatalogControl.pui8Data);
            else if(psDatalog->sDatalogControl.eOpMode == eOPMODE_RECMODEMEM)
            {
                free(psDatalog->sDatalogControl.sDatalogChannels[i].ui8RamBuf[0]);
                free(psDatalog->sDatalogControl.sDatalogChannels[i].ui8RamBuf[1]);
            }
            psDatalog->sDatalogControl.ui8ActiveLoggers &= ~(1 << i);
        }
    i++;
    }   
}

//===================================================================================
void DataloggerClearMemory(tDATALOGGER *psDatalog)
{
    _DataloggerClearMemory(psDatalog);
}

//===================================================================================
tDATALOG_STATE DataloggerGetCurrentState(tDATALOGGER *psDatalog)
{
    return psDatalog->eDatalogState;
}

//===================================================================================
tDATALOG_OPMODES DataloggerGetCurrentOpMode(tDATALOGGER *psDatalog)
{
    return psDatalog->sDatalogControl.eOpMode;
}

//===================================================================================
tDATALOG_ERROR DataloggerSetOpMode(tDATALOGGER *psDatalog, tDATALOG_OPMODES eNewOpMode)
{
    // Check if datalogger tasks are going on
    switch(psDatalog->eDatalogState)
    {
        case eDLOGSTATE_RUNNING:
        case eDLOGSTATE_FORMAT_MEMORY:
        case eDLOGSTATE_ABORTING:
            return eDATALOG_ERROR_WRONG_STATE;
        
        default:
            break;
    }

    // Check new operation mode
    switch(eNewOpMode)
    {
        case eOPMODE_LIVE:
            return eDATALOG_ERROR_NOT_IMPLEMENTED;

        case eOPMODE_RECMODEMEM:
            return eDATALOG_ERROR_NOT_IMPLEMENTED;

        case eOPMODE_RECMODERAM:
            break;

        default:
            return eDATALOG_ERROR_NOT_IMPLEMENTED;
    }

    psDatalog->sDatalogControl.eOpMode = eNewOpMode;
    DataloggerSetStateImmediate(psDatalog, eDLOGSTATE_UNINITIALIZED);

    return eDATALOG_ERROR_NONE;
}

//===================================================================================
tDATALOG_ERROR DataloggerGetDataPtr(tDATALOGGER *psDatalog, uint8_t** pui8Data, uint32_t *ui32Len)
{
    // Data must be available
    if (psDatalog->eDatalogState != eDLOGSTATE_DATA_READY)
        return eDATALOG_ERROR_WRONG_STATE;

    // Memory mode datalogger cannot be read out directly
    if (psDatalog->sDatalogControl.eOpMode == eOPMODE_RECMODEMEM)
        return eDATALOG_ERROR_WRONG_OPMODE;

    *pui8Data = psDatalog->sDatalogControl.pui8Data;
    *ui32Len = psDatalog->sDatalogControl.ui32MemLen;

    return eDATALOG_ERROR_NONE;
}

//===================================================================================
tDATALOG_ERROR DataloggerGetChannelInfo(tDATALOGGER *psDatalog, tDATALOG_CHANNEL *pChannel, uint8_t ui8ChNum)
{
    if (ui8ChNum > MAX_NUM_LOGS)
        return eDATALOG_ERROR_NUMBER_OF_LOGS_EXCEEDED;
    
    if (!(psDatalog->sDatalogControl.ui8ActiveLoggers & (1 << (ui8ChNum - 1))))
        return eDATALOG_ERROR_CHANNEL_NOT_ACTIVE;

    *pChannel = psDatalog->sDatalogControl.sDatalogChannels[ui8ChNum - 1];

    return eDATALOG_ERROR_NONE;
}

//===================================================================================
tDATALOGGER_VERSION DataloggerGetVersion(tDATALOGGER *psDatalog)
{
    return psDatalog->sVersion;
}

//===================================================================================
tDATALOG_ERROR DataloggerRegisterLog (tDATALOGGER *psDatalog, uint32_t ui32ChID, uint8_t ui8LogNum, uint16_t ui16FreqDiv, uint32_t ui32RecLen, uint8_t *pui8Variable, uint8_t ui8ByteCount)
{
    tDATALOG_CHANNEL        *pChannel; 
    tDATALOG_CHANNEL_MEMORY *pMemChannel;

    if (ui8LogNum > MAX_NUM_LOGS)
        return eDATALOG_ERROR_NUMBER_OF_LOGS_EXCEEDED;
    
    pChannel = &psDatalog->sDatalogControl.sDatalogChannels[ui8LogNum - 1];
    pMemChannel = &psDatalog->sMemoryHeader.sDatalogChannelsMemory[ui8LogNum - 1];

    if (!(psDatalog->eDatalogState != eDLOGSTATE_INITIALIZED ||
        psDatalog->eDatalogState != eDLOGSTATE_UNINITIALIZED))
        return eDATALOG_ERROR_WRONG_STATE;

    // Initialize parameter variables
    pChannel->ui32ChID = pMemChannel->ui32ChannelID     = ui32ChID;
    pChannel->pui8Variable = pMemChannel->pui8Variable  = pui8Variable;
    pChannel->ui8ByteCount = pMemChannel->ui8ByteCount  = ui8ByteCount;
    pChannel->ui16Divider = pMemChannel->ui16Divider    = ui16FreqDiv;
    pChannel->ui32RecordLength                          = ui32RecLen;

    // Activate logger immediately
    psDatalog->sDatalogControl.ui8ActiveLoggers |= (1 << (ui8LogNum - 1));

    // New value is set -> Need to initialize the datalogger prior to next log run.
    DataloggerSetStateImmediate(psDatalog, eDLOGSTATE_UNINITIALIZED);

    return eDATALOG_ERROR_NONE;
}

//===================================================================================
tDATALOG_ERROR DataloggerRemoveLog (tDATALOGGER *psDatalog, uint8_t ui8LogNum)
{
    if (ui8LogNum > MAX_NUM_LOGS)
        return eDATALOG_ERROR_LOG_NUMBER_INVALID;

    if (!(psDatalog->sDatalogControl.ui8ActiveLoggers & (1 << (ui8LogNum - 1)) ))
        return eDATALOG_ERROR_CHANNEL_NOT_ACTIVE;

    psDatalog->sDatalogControl.ui8ActiveLoggers &= ~(1 << (ui8LogNum - 1));

    DataloggerSetStateImmediate(psDatalog, eDLOGSTATE_UNINITIALIZED);

    // Reinitialize the logger
    return eDATALOG_ERROR_NONE;
}

//===================================================================================
// Function: DatalogInitialize
//===================================================================================
/********************************************************************************//**
 * \brief Initialzes the Datalogger for a datalog run.
 *
 * This funciton must be called prior to start the datalogger. The memory is
 * initialized depending on the selected log configuration. 
 *
 * @param   sDatalogConfig Desired configuration of the datalogger.
 * @returns Error indicator
 ***********************************************************************************/
tDATALOG_ERROR DataloggerInitLogger (tDATALOGGER *psDatalog, bool bFreeMemory)
{
    uint16_t    ui16TempSize;
    uint8_t     i = 0;
    uint8_t     ui8LogCount = 0;
    uint8_t     ui8LogIdx[MAX_NUM_LOGS] = {0};
    uint32_t   ui32CurrentByteSize = 0;
    tDATALOG_CHANNEL *pChannel = &psDatalog->sDatalogControl.sDatalogChannels[0];

    // Get out of this function if the state is not correct
    if (psDatalog->eDatalogState != eDLOGSTATE_UNINITIALIZED)
        return eDATALOG_ERROR_WRONG_STATE;


    /********************************************************************************
     * Free former memory allocation
     *******************************************************************************/

    if (bFreeMemory)
        _DataloggerClearMemory(psDatalog);

    // Preinitialize Datalog structure
    for(i = 0; i < MAX_NUM_LOGS; i++)
    {
        if (!(psDatalog->sDatalogControl.ui8ActiveLoggers & (1 << i)))
            continue;

        ui8LogIdx[ui8LogCount] = i;

        // Determine the offset of the current channel in external memory
        if (psDatalog->sDatalogControl.eOpMode == eOPMODE_RECMODEMEM || 
            psDatalog->sDatalogControl.eOpMode == eOPMODE_RECMODERAM)
        {
            if (ui8LogCount > 0)
            {
                psDatalog->sMemoryHeader.sDatalogChannelsMemory[i].ui32MemoryOffset = 
                psDatalog->sDatalogControl.sDatalogChannels[i].ui32MemoryOffset = 
                    psDatalog->sDatalogControl.sDatalogChannels[ui8LogIdx[ui8LogCount - 1]].ui32MemoryOffset +
                    psDatalog->sDatalogControl.sDatalogChannels[ui8LogIdx[ui8LogCount - 1]].ui32RecordLength *
                    psDatalog->sDatalogControl.sDatalogChannels[ui8LogIdx[ui8LogCount - 1]].ui8ByteCount;
            }
            else
            {
                if (psDatalog->sDatalogControl.eOpMode == eOPMODE_RECMODEMEM )
                {
                    psDatalog->sMemoryHeader.sDatalogChannelsMemory[i].ui32MemoryOffset = 
                    psDatalog->sDatalogControl.sDatalogChannels[i].ui32CurMemPos = MEMORY_HEADER_SIZE();
                }
                else
                {
                    psDatalog->sDatalogControl.sDatalogChannels[i].ui32MemoryOffset = 
                    psDatalog->sDatalogControl.sDatalogChannels[i].ui32CurMemPos = 0;
                }
            }

            ui32CurrentByteSize +=  psDatalog->sDatalogControl.sDatalogChannels[i].ui32RecordLength *
                                    psDatalog->sDatalogControl.sDatalogChannels[i].ui8ByteCount;
        }
        ui8LogCount++;
    }


    /********************************************************************************
     * RAM initialization routines
     *******************************************************************************/
    if (psDatalog->sDatalogControl.eOpMode == eOPMODE_RECMODEMEM)
    {
        // temporary buffer is always set to max size
        ui16TempSize = DATALOGGER_MAX_BUFFER_SIZE / (ui8LogCount << 1);

        for(i = 0; i < ui8LogCount; i++)
        {
            pChannel = &psDatalog->sDatalogControl.sDatalogChannels[ui8LogIdx[i]];

            pChannel->ui8RamBuf[0] = (uint8_t*)malloc((size_t)(ui16TempSize));
            pChannel->ui8RamBuf[1] = (uint8_t*)malloc((size_t)(ui16TempSize));

            // Retrieve data whenn the index exceeds half of the memory size
            pChannel->ui16RetrieveThreshIdx = ui16TempSize >> 2;
            
        }
        // Note: pChannel still pointing on the last active memory channel
        psDatalog->sMemoryHeader.ui32LastAddress = 
            psDatalog->sMemoryHeader.sDatalogChannelsMemory[ui8LogIdx[ui8LogCount - 1]].ui32MemoryOffset + 
            pChannel->ui32RecordLength - 1;

        psDatalog->sMemoryHeader.ui32TimeBase = psDatalog->sNVPar.ui32EE_TimeBase_Hz;
    }
    else if (psDatalog->sDatalogControl.eOpMode == eOPMODE_RECMODERAM)
    {
        if (ui32CurrentByteSize > DATALOGGER_MAX_BUFFER_SIZE)
            return eDATALOG_ERROR_NOT_ENOUGH_MEMORY;
        
        psDatalog->sDatalogControl.pui8Data = (uint8_t*)calloc((size_t)ui32CurrentByteSize, 1);
    }

    psDatalog->sDatalogControl.ui32MemLen = ui32CurrentByteSize;
    /********************************************************************************
     * State control
     *******************************************************************************/
    
    //TODO: LIVEMODE Ringbuffer initialization

    if (psDatalog->sDatalogControl.eOpMode != eOPMODE_RECMODEMEM)
    {
        DataloggerSetStateImmediate(psDatalog, eDLOGSTATE_INITIALIZED);
        // psDatalog->eDatalogStatePending = eDLOGSTATE_INITIALIZED;
    }
    else 
    {
        DataloggerSetStateImmediate(psDatalog, eDLOGSTATE_FORMAT_MEMORY);
        // psDatalog->eDatalogStatePending = eDLOGSTATE_FORMAT_MEMORY;
    }

    return eDATALOG_ERROR_NONE;
}

//===================================================================================
// Funktion: DatalogInitializeRead
//===================================================================================
// Parameter: keine
//-----------------------------------------------------------------------------------
// R�ckgabewerte:   true: Initialisierung erfolgreich.
//                  false: Fehler.
//-----------------------------------------------------------------------------------
// Beschreibung: Initialisiert und startet das Auslesen des Datalog-Memorys
//===================================================================================
/* bool InitializeMemoryRead (void) */
/* { */
/*     // Raus bei falschem Zustand */
/*     if (sDatalog.eDatalog_state != eINT_MODE_UNINITIALIZED       && */
/*         sDatalog.eDatalog_state != eINT_MODE_INITIALIZED         && */
/*         sDatalog.eDatalog_state != eINT_MODE_READY_TO_START) */
/*     { */
/*         return false; */
/*     } */

/*     // G�ltigkeitscheck der zu lesenden Adressen */
/*     if (sDatalog.sMemory_read.ui32Memory_offset >= FRAM_MAX_ADDRESS || */
/*         sDatalog.sMemory_read.ui32Last_address > FRAM_MAX_ADDRESS) */
/*     { */
/*         return false; */
/*     } */

/*     // Data IN Endpoint aktivieren */
/*     sRoboard_interface.sData_in_endpoint.bEndpoint_active = true; */
/*     // Abspeichern des alten Datenloggerzustandes f�r den Restore */
/*     sDatalog.eDatalog_old_state = sDatalog.eDatalog_state; */
/*     // Zustand umschalten */
/*     sDatalog.eDatalog_state = eMEMORY_READ; */

/*     return true; */
/* } */


//===================================================================================
// Function: DatalogStart
//===================================================================================
/********************************************************************************//**
 * \brief Starts the datalogger
 *
 * @returns Error indicator
 ***********************************************************************************/
tDATALOG_ERROR DataloggerStart (tDATALOGGER *psDatalog)
{
    uint8_t i = 0;
    tDATALOG_CHANNEL* pChannel = psDatalog->sDatalogControl.sDatalogChannels;

    if (psDatalog->eDatalogState != eDLOGSTATE_INITIALIZED)
        return eDATALOG_ERROR_WRONG_STATE;

    // Resets all relevant variables
    while (i < MAX_NUM_LOGS)
    {
        if (!(psDatalog->sDatalogControl.ui8ActiveLoggers & (1 << i)))
        {
            i++;
            continue;
        }

        // Reset of the state variables
        pChannel[i].ui8BufNum = 0;
        pChannel[i].ui16DivideCount = 1;
        pChannel[i].ui16ValIdx = 0;
        pChannel[i].ui32CurrentCount = 0;
        
        if(psDatalog->sDatalogControl.eOpMode == eOPMODE_RECMODERAM)
        {
            pChannel[i].ui32CurMemPos = psDatalog->sDatalogControl.sDatalogChannels[i].ui32MemoryOffset;
            // psDatalog->sDatalogControl.ui32CurIdx = 0;
        }
        else if(psDatalog->sDatalogControl.eOpMode == eOPMODE_RECMODEMEM)
        {
            pChannel[i].ui32CurMemPos = psDatalog->sMemoryHeader.sDatalogChannelsMemory[i].ui32MemoryOffset;

            // Reset the control variables of the serializer
            psDatalog->sDatalogSerializer.ui8ArbitrationCount = 0;
            psDatalog->sDatalogSerializer.ui8FillIdx = 0;
            psDatalog->sDatalogSerializer.ui8RetrieveFlags = 0;
            psDatalog->sDatalogSerializer.ui8RetrieveIdx = 0;
        }

        i++;
    }

    psDatalog->sDatalogControl.ui8ChannelsRunning = 
        psDatalog->sDatalogControl.ui8ActiveLoggers;

    // Switch the datalog on (directly, because this is time critical)
    DataloggerSetStateImmediate(psDatalog, eDLOGSTATE_RUNNING);

    if(psDatalog->sCallbacks.StartDataloggerCb != NULL)
        psDatalog->sCallbacks.StartDataloggerCb();

    return eDATALOG_ERROR_NONE;
}

//===================================================================================
// Function: DataloggerStop
//===================================================================================
/********************************************************************************//**
 * \brief Stops the datalogger
 *
 * @returns Error indicator
 ***********************************************************************************/
tDATALOG_STATE DataloggerStop (tDATALOGGER *psDatalog)
{
    if (psDatalog->eDatalogState != eDLOGSTATE_RUNNING)
        return eDATALOG_ERROR_WRONG_STATE;

    DataloggerSetStateImmediate(psDatalog, eDLOGSTATE_ABORTING);

    // Arbitration-count definiert auf 0 setzen
    psDatalog->sDatalogSerializer.ui8ArbitrationCount = 0;

    if(psDatalog->sCallbacks.StopDataloggerCb != NULL)
        psDatalog->sCallbacks.StopDataloggerCb();

    return eDATALOG_ERROR_NONE;
}

/********************************************************************************//**
 * \brief Samples the previously selected values
 *
 * This routine must get called regularly with a defined time base.
 ***********************************************************************************/
void DataloggerService (tDATALOGGER *psDatalog)
{
    uint8_t i;
    bool bAbort_flag = false;
    // bool tmp;
    tDATALOG_CHANNEL *pChannel = psDatalog->sDatalogControl.sDatalogChannels;
    uint8_t ui8ChannelsRunningTemp = psDatalog->sDatalogControl.ui8ChannelsRunning;
    // uint32_t ui32CurrentOffset = 0;

    if (psDatalog->eDatalogState != eDLOGSTATE_RUNNING)
        return;

    // Get all data
    for (i = 0; i < MAX_NUM_LOGS; i++)
    {
        if (!ui8ChannelsRunningTemp)
            break;

        // Check if Channel is activated and running
        if (!(psDatalog->sDatalogControl.ui8ChannelsRunning & (1 << i)))
            continue;

        ui8ChannelsRunningTemp &= ~ (1 << i);

        // Sample data
        if (!(--pChannel[i].ui16DivideCount))
        {
            if (psDatalog->sDatalogControl.eOpMode == eOPMODE_RECMODERAM) 
            {
                // Fill buffer in big endian format
                for(uint8_t j = 0; j < pChannel[i].ui8ByteCount; j++)
                {
                    psDatalog->sDatalogControl.pui8Data[pChannel[i].ui32MemoryOffset + (pChannel[i].ui32CurrentCount << (pChannel[i].ui8ByteCount >> 1)) + j] = 
                        pChannel[i].pui8Variable[pChannel[i].ui8ByteCount - 1 - j];
                }
            }
            else if (psDatalog->sDatalogControl.eOpMode == eOPMODE_RECMODEMEM)
            {
                // Fill the appropirate arbitration buffer with data in big endian format
                for(uint8_t j = 0; j < pChannel[i].ui8ByteCount; j++)
                {
                    pChannel[i].ui8RamBuf[pChannel[i].ui8BufNum][(pChannel[i].ui32CurrentCount << (pChannel[i].ui8ByteCount >> 1)) + j] = 
                        pChannel[i].pui8Variable[pChannel[i].ui8ByteCount - 1 - j];
                }

                // Set flag to empty the currently used buffer
                if (++pChannel[i].ui16ValIdx == pChannel[i].ui16RetrieveThreshIdx)
                    psDatalog->sDatalogSerializer.ui8RetrieveFlags |= (1 << i);

                // Make sure that no buffer overflow can happen
                if (pChannel[i].ui16ValIdx == pChannel[i].ui16RetrieveThreshIdx << 1)
                    bAbort_flag = true;
            }

            // Switch off channel if it has reached its maximum count
            if (++pChannel[i].ui32CurrentCount == pChannel[i].ui32RecordLength)
                psDatalog->sDatalogControl.ui8ChannelsRunning &= ~(1 << i);

            pChannel[i].ui16DivideCount = pChannel[i].ui16Divider;
        }
    }

    bAbort_flag = (psDatalog->sDatalogControl.ui8ActiveLoggers & psDatalog->sDatalogControl.ui8ChannelsRunning) == 0;
    // If all channels reached their record length, switch off datalogger

    if (bAbort_flag)
        DataloggerStop(psDatalog);
}

//===================================================================================
// Funktion: LiveModeTimerInit
//===================================================================================
// Parameter:   keiner
//-----------------------------------------------------------------------------------
// R�ckgabewerte: keiner
//-----------------------------------------------------------------------------------
// Beschreibung: Konfiguriert Timestamp-Timer und Sync Timer f�r den Live Mode des
//               Datenloggers. Muss beim Initialisieren der Applikation aufgerufen
//               werden.
//===================================================================================
/* void LiveModeTimerInit (void) */
/* { */
/*     tsTIMERVAR32 sTemp = tsTIMERVAR32_DEFAULTS; */

/*     // Zun�chst der Sync Timer (Down Counter) */
/*     sTemp.pfnTimer_cb = LiveModeSampleData; */
/*     sDatalog.sDatalog_live.i8Sample_Timer_idx = ms_timer_32Bit_append(&sTemp); */

/*     // Timestamp Timer (Up-Counter ohne Grenzwert) */
/*     sTemp.pfnTimer_cb = NULL; */
/*     sTemp.bUp_counter = true; */
/*     sTemp.ui32Max_count = 0xFFFFFFFF; */
/*     sDatalog.sDatalog_live.i8Timestamp_timer_idx = ms_timer_32Bit_append(&sTemp); */
/* } */

//===================================================================================
// Funktion: LiveModeDatalogStart
//===================================================================================
// Parameter:   ui8Var_count -> Anzahl der Variablen
//              pui16Var_nr  -> Startadresse des Arrays mit den Variablennummern
//              pui16Divider -> Teilungsfaktor der Zeitbasis
//-----------------------------------------------------------------------------------
// R�ckgabewerte: keiner
//-----------------------------------------------------------------------------------
// Beschreibung: Initialisiert den Live-Datalog
//===================================================================================
/*bool LiveModeDatalogStart (uint8_t ui8Var_count, uint16_t* pui16Var_nr, uint16_t ui16Divider)*/
/*{*/
/*    uint16_t ui16Current_var_idx = 0;*/
/*    uint16_t i = 0;*/
/*    // Pointer auf Variablenstruktur*/
/*    tsDATALOG_LIVE* pInstance = &sDatalog.sDatalog_live;*/

/*    // Raus bei falschem Zustand*/
/*    if (sDatalog.eDatalog_state != eINT_MODE_UNINITIALIZED       &&*/
/*        sDatalog.eDatalog_state != eINT_MODE_INITIALIZED         &&*/
/*        sDatalog.eDatalog_state != eINT_MODE_READY_TO_START)*/
/*        return false;*/

/*    // Pr�fung auf die Grenzwerte des Live-Datenloggers*/
/*    if (ui8Var_count > MAX_NUM_LIVE_LOGS || ui16Divider < MIN_SAMPLE_TIME_MS)*/
/*        return false;*/

/*    while (i < ui8Var_count)*/
/*    {*/

       /***************************************************************************
       * Initialisierung der Variablenstruktur
       ***************************************************************************/
/*        // Welchen Index hat die Variable mit der Nummer in der Variablenstruktur?*/
/*        while (ui16Current_var_idx < MAX_ANZAHL_RAM_VARS)*/
/*        {*/
/*            if (ramVars[ui16Current_var_idx].uiRAMVarNr == pui16Var_nr[i])*/
/*                break;*/

/*            ui16Current_var_idx++;*/
/*        }*/

/*        // Variablennummer gefunden? -> Pointer auf Variablenstruktur speichern.*/
/*        if (ui16Current_var_idx < MAX_ANZAHL_RAM_VARS)*/
/*            // Pointer auf RAM-Variablenstruktur �bernehmen*/
/*            pInstance->pRamvar_ptr[i] = &ramVars[ui16Current_var_idx];*/

/*        // Eintrag nicht vorhanden -> raus*/
/*        else*/
/*            return false;*/

/*        // Z�hlindex f�r das �bergebene Array erh�hen*/
/*        i++;*/
/*        ui16Current_var_idx = 0;*/
/*    }*/
/*    // Anzahl der Logs �bernehmen*/
/*    pInstance->ui8Number_of_logs = ui8Var_count;*/
/*    // Timebase Divider �bernehmen*/
/*    pInstance->ui16Timebase_divider = ui16Divider;*/

/*    // Abspeichern des alten Datenloggerzustandes f�r den Restore*/
/*    sDatalog.eDatalog_old_state = sDatalog.eDatalog_state;*/
/*    sDatalog.eDatalog_state = eLIVE_MODE_LOG_RUNNING;*/
/*    // Zeitstempel-Timer resetten und aktivieren*/
/*    ms_timer_32Bit_set_val(sDatalog.sDatalog_live.i8Timestamp_timer_idx, true, 0xFFFFFFFF);*/
/*    // Ersten Datensatz aufnehmen*/
/*    LiveModeSampleData();*/
/*    // Timer f�r den n�chsten Zeitpunkt zum Sammeln der Daten setzen*/
/*    //ms_timer_32Bit_set_val (sDatalog.sDatalog_live.i8Sample_Timer_idx, true, ui16Divider - 1);*/

/*    return true;*/
/*}*/

//===================================================================================
// Funktion: LiveModeDatalogStop
//===================================================================================
// Parameter:   keine
//-----------------------------------------------------------------------------------
// R�ckgabewerte: Fehler-Indikator
//-----------------------------------------------------------------------------------
// Beschreibung: Stoppt den Live-Datalog
//===================================================================================
/* bool LiveModeDatalogStop (void) */
/* { */
/*     // Raus, wenn der Live-Datenlogger nicht l�uft */
/*     if (sDatalog.eDatalog_state != eLIVE_MODE_LOG_RUNNING) */
/*         return false; */

/*     // Sample Timer stoppen */
/*     ms_timer_32Bit_set_active(sDatalog.sDatalog_live.i8Sample_Timer_idx, false); */
/*     // Endpoint abschalten */
/*     sRoboard_interface.sData_in_endpoint.bEndpoint_active = false; */
/*     // Reset des Datenendpoints */
/*     DataInEPReset(); */
/*     // Timestamp Timer stoppen */
/*     ms_timer_32Bit_set_active(sDatalog.sDatalog_live.i8Timestamp_timer_idx, false); */
/*     // Zustand wechseln */
/*     sDatalog.eDatalog_state = sDatalog.eDatalog_old_state; */

/*     return true; */
/* } */

//===================================================================================
// Funktion: LiveModeSampleData
//===================================================================================
// Parameter:   keine
//-----------------------------------------------------------------------------------
// R�ckgabewerte: keiner
//-----------------------------------------------------------------------------------
// Beschreibung: Sammelt Daten und veranlasst die �bertragung �ber den
//               Ausgangsdaten-EP der USB-Schnittstelle
//===================================================================================
/*void LiveModeSampleData (void)*/
/*{*/
/*    // Z�hlvariable*/
/*    uint16_t i = 0;*/
/*    uint8_t* pui8Send_buffer = &sRoboard_interface.sData_in_endpoint.ui8Endpoint_data[0];*/
/*    uint16_t ui16Byte_count = 4;*/
/*    // Pointer auf Variablenstruktur*/
/*    tsDATALOG_LIVE* pInstance = &sDatalog.sDatalog_live;*/

/*    // Raus, wenn der Zustand nicht passt.*/
/*    if (sDatalog.eDatalog_state != eLIVE_MODE_LOG_RUNNING)*/
/*        return;*/

   /*******************************************************************************
   * Vorbereitung der Schnittstelle
   *******************************************************************************/
/*    sRoboard_interface.sData_in_endpoint.bEndpoint_active = false;*/
/*    DataInEPReset();*/
/*    sRoboard_interface.sData_in_endpoint.bEndpoint_active = true;*/

/*    // Daten EP Konfiguration so, dass Buffer sofort gesendet wird*/
/*    sRoboard_interface.sData_in_endpoint.bSend_buffer = true;*/
   /*******************************************************************************
   * Werte Laden
   *******************************************************************************/
/*    // Zeitstempel ganz vorn einspeichern*/
/*    *((uint32_t*)pui8Send_buffer) = sTimer_ms_32.sTimer_struct[pInstance->i8Timestamp_timer_idx].ui32Timer_val;*/
/*    // Buffer Pointer erh�hen*/
/*    pui8Send_buffer += ui16Byte_count;*/

/*    while (i < pInstance->ui8Number_of_logs)*/
/*    {*/
/*        // Typabh�ngiges Einspeichern in den Sendebuffer*/
/*        switch (pInstance->pRamvar_ptr[i]->uiRAMVarTyp)*/
/*        {*/
/*        case  TYP_UI8:*/
/*            *(uint8_t*)pui8Send_buffer = *(uint8_t*)pInstance->pRamvar_ptr[i]->pRAMVarPtr;*/
/*            break;*/
/*        case  TYP_I8:*/
/*            *(int8_t*)pui8Send_buffer = *(int8_t*)pInstance->pRamvar_ptr[i]->pRAMVarPtr;*/
/*            break;*/
/*        case  TYP_UI16:*/
/*            *(uint16_t*)pui8Send_buffer = *(uint16_t*)pInstance->pRamvar_ptr[i]->pRAMVarPtr;*/
/*            break;*/
/*        case  TYP_I16:*/
/*            *(I16*)pui8Send_buffer = *(I16*)pInstance->pRamvar_ptr[i]->pRAMVarPtr;*/
/*            break;*/
/*        case  TYP_UI32:*/
/*            *(uint32_t*)pui8Send_buffer = *(uint32_t*)pInstance->pRamvar_ptr[i]->pRAMVarPtr;*/
/*            break;*/
/*        case  TYP_I32:*/
/*            *(I32*)pui8Send_buffer = *(I32*)pInstance->pRamvar_ptr[i]->pRAMVarPtr;*/
/*            break;*/
/*        case  TYP_F32:*/
/*            *(F32*)pui8Send_buffer = *(F32*)pInstance->pRamvar_ptr[i]->pRAMVarPtr;*/
/*            break;*/
/*        }*/
/*        // Byte Count und Pointer erh�hen*/
/*        ui16Byte_count += ui8Byte_count[pInstance->pRamvar_ptr[i]->uiRAMVarTyp];*/
/*        pui8Send_buffer += ui8Byte_count[pInstance->pRamvar_ptr[i]->uiRAMVarTyp];*/

/*        i++;*/
/*    }*/

/*    // Bufferindex zum Senden setzen*/
/*    sRoboard_interface.sData_in_endpoint.ui16Buffer_write_index = ui16Byte_count;*/
/*}*/

//===================================================================================
// Funktion: RobLogSM
//===================================================================================
/********************************************************************************//**
 * \brief State machine of the RobLog datalogger
 *
 * @returns Error indicator
 ***********************************************************************************/
void DataloggerStatemachine (tDATALOGGER *psDatalog)
{
    //tROBLOG_ERROR eErrorIndicator = eROBLOG_ERROR_NONE;
    // tDATALOG_STATE eNewState = psDatalog->eDatalogState;
    /* uint16_t ui16Data_size; */
    /* uint16_t ui16End_index; */
    /* uint16_t ui16RAM_buffer_offset; */
    /* uint32_t ui32Memory_address_temp; */
    /* uint16_t ui16Max_bytes_temp; */

    // Reset the pending datalogger state to not transition to something else at the end.
    psDatalog->eDatalogStatePending = psDatalog->eDatalogState;

    switch (psDatalog->eDatalogState)
    {
    case eDLOGSTATE_UNINITIALIZED:
        break;


    case eDLOGSTATE_FORMAT_MEMORY:

        // Wait for the memory write before changing the state angain
        // TODO: Functions not implemented yet
        /*if (checkMemoryInterfaceState() == eROBLOGIF_IDLE)
            psDatalog->eDatalogStatePending = eDLOGSTATE_RECMODEMEM_RUNNING;*/

        break;

    case eDLOGSTATE_INITIALIZED:
        // Wait for the datalogger to be started (Done by API)
        break;

    case eDLOGSTATE_RUNNING:

/*         // Arbitration Buffer schreiben. Dabei Ringbuffer handeln */
/*         if(pMem_sched->ui8Retrieve_flags & (1 << pMem_sched->ui8Arbitration_count)) */
/*         { */
/*             pMem_sched->ui8Arbitration_buffer[pMem_sched->ui8Fill_idx] = pMem_sched->ui8Arbitration_count; */

/*             //Flag zur�cksetzen */
/*             pMem_sched->ui8Retrieve_flags &= ~(1 << pMem_sched->ui8Arbitration_count); */

/*             if (++pMem_sched->ui8Fill_idx >= ARBITRATION_BUFFER_SIZE) */
/*                 pMem_sched->ui8Fill_idx = 0; */
/*         } */

/*         // Wenn Sendeauftr�ge im Arbitration Buffer stehen */
/*         if(pMem_sched->ui8Fill_idx != pMem_sched->ui8Retrieve_idx) */
/*         { */
/*             // Pointer auf Datenloggerstruktur */
/*             pInstance = &sDatalog.sDatalog_internal.sHeader.sDatalog_array[pMem_sched->ui8Arbitration_buffer[pMem_sched->ui8Retrieve_idx]]; */

/*             // Wenn der F-RAM I�C frei ist, n�chsten Sendeauftrag initiieren */
/*             if(sFram_interface.eState == eIDLE) */
/*             { */
/*                 // Hier darf kein Datalog-Interrupt kommen (Datenverlust)! */
/*                 ROM_TimerIntDisable(TIMER2_BASE, TIMER_TIMA_TIMEOUT); */

/*                 // Datengr��e und Buffer-Offset aus dem Byte-Count in der Variablenstruktur berechnen und */
/*                 // Laufvariablen direkt zur�cksetzen. */
/*                 ui16Data_size = pInstance->ui16Val_idx * ui8Byte_count[ramVars[pInstance->ui16Index].uiRAMVarTyp]; */
/*                 pInstance->ui16Val_idx = 0; */
/*                 ui16RAM_buffer_offset = pInstance->ui16Offset_idx * ui8Byte_count[ramVars[pInstance->ui16Index].uiRAMVarTyp]; */
/*                 // Offset des genutzten Bufferabschnittes switchen */
/*                 pInstance->ui16Offset_idx = pInstance->ui16Offset_idx ? 0 : RAM_BUFFER_HIGH_IDX; */

/*                 ROM_TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT); */

/*                 // Transaktion ins Memory anleiern */
/*                 fram_initiate_transaction (ui16Data_size, pInstance->ui32Memory_offset, &(((uint8_t*)pInstance->pvRam_buf)[ui16RAM_buffer_offset]), NULL, false, false); */

/*                 // Memory Offset anpassen */
/*                 pInstance->ui32Memory_offset += ui16Data_size; */

/*                 // Buffer�berlauf handeln */
/*                if(++pMem_sched->ui8Retrieve_idx >= ARBITRATION_BUFFER_SIZE) */
/*                    pMem_sched->ui8Retrieve_idx = 0; */
/*             } */
/*         } */

/*         // �berlauf Arbitration Counter handeln */
/*         if (++pMem_sched->ui8Arbitration_count >= sDatalog.sDatalog_internal.sHeader.ui8Active_loggers) */
/*             pMem_sched->ui8Arbitration_count = 0; */
        break;

    // case eDLOGSTATE_RECMODERAM_RUNNING:
    //     break;

    case eDLOGSTATE_ABORTING:
        /* // Pointer auf Loggerstruktur (Arbitration Count wurde beim Zustandswechsel auf 0 gesetzt!) */
        /* pInstance = &sDatalog.sDatalog_internal.sHeader.sDatalog_array[pMem_sched->ui8Arbitration_count]; */

        /* // M�ssen noch Daten ins Memory geschrieben werden? */
        /* if (pInstance->ui16Val_idx) */
        /* { */
        /*     if(sFram_interface.eState == eIDLE) */
        /*     { */
        /*         ui16Data_size = pInstance->ui16Val_idx * ui8Byte_count[ramVars[pInstance->ui16Index].uiRAMVarTyp]; */
        /*         ui16RAM_buffer_offset = pInstance->ui16Offset_idx * ui8Byte_count[ramVars[pInstance->ui16Index].uiRAMVarTyp]; */
        /*         // Transaktion ins Memory anleiern */
        /*         fram_initiate_transaction (ui16Data_size, pInstance->ui32Memory_offset, &(((uint8_t*)pInstance->pvRam_buf)[ui16RAM_buffer_offset]), NULL, false, false); */

        /*         //DebugOutput(DEBUG_MEDIUM,"max val: %d", pInstance->ui16Debug_max_val_idx); */

        /*         // Zustandswechsel, wenn alle verbliebenen Daten geschrieben wurden */
        /*         if (++pMem_sched->ui8Arbitration_count >= sDatalog.sDatalog_internal.sHeader.ui8Active_loggers) */
        /*             sDatalog.eDatalog_state = eINT_MODE_READY_TO_START; */
        /*     } */
        /* } */
        /* // Sonst direkt Laufvariablen zur�cksetzen */
        /* else */
        /* { */
        /*     // Zustandswechsel, wenn alle verbliebenen Daten geschrieben wurden */
        /*     if (++pMem_sched->ui8Arbitration_count >= sDatalog.sDatalog_internal.sHeader.ui8Active_loggers) */
        /*         sDatalog.eDatalog_state = eINT_MODE_READY_TO_START; */
        /* } */
        if (psDatalog->sDatalogControl.eOpMode == eOPMODE_RECMODERAM)
        {
            psDatalog->eDatalogStatePending = eDLOGSTATE_DATA_READY;
        }
        break;

    // case eDLOGSTATE_RECMODERAM_FINISHING:
    //     break;

    // // Live-Datenlogger
    // case eDLOGSTATE_LIVEMODE_RUNNING:

/*        // Wurde der Bulk-Transfer abgeschlossen, dann sind wir in Sync mit dem Host-Timer*/
/*        if (sRoboard_interface.sData_in_endpoint.bTransfer_concluded)*/
/*        {*/
/*            // Sample-Timer neu laden*/
/*            ms_timer_32Bit_set_val(sDatalog.sDatalog_live.i8Sample_Timer_idx, true,*/
/*                                   sDatalog.sDatalog_live.ui16Timebase_divider);*/

/*            // Endpoint ausschalten*/
/*            sRoboard_interface.sData_in_endpoint.bEndpoint_active = false;*/
/*            // Reset des Data IN Endpoints*/
/*            DataInEPReset();*/
/*        }*/

/*        break;*/

/*    case eMEMORY_READ:*/
/*        // Check, ob der Sendebuffer voll ist*/
/*        if (sRoboard_interface.sData_in_endpoint.eBuffer_state == eLOW_FULL_HIGH_FULL)*/
/*            break;*/

/*        // Auch wenn gerade ein Transfer l�uft, darf die Funktion nicht betreten werden*/
       /*if (sRoboard_interface.sData_in_endpoint.eTransfer_state == eTRANSFER_LOW ||
               sRoboard_interface.sData_in_endpoint.eTransfer_state == eTRANSFER_HIGH)
           break;*/

/*        // Wenn das F-RAM bereit ist, Datenbulk lesen*/
/*        if (sFram_interface.eState == eIDLE)*/
/*        {*/
/*            // K�nnen wir die maximale Anzahl an Datenbytes aus dem Speicher lesen?*/
/*            ui32Memory_address_temp = sDatalog.sMemory_read.ui32Memory_offset + sRoboard_interface.sData_in_endpoint.ui16Max_data_to_write;*/



/*            if(ui32Memory_address_temp <= sDatalog.sMemory_read.ui32Last_address)*/
/*                ui16Max_bytes_temp = sRoboard_interface.sData_in_endpoint.ui16Max_data_to_write;*/
/*            else*/
/*            {*/

/*                ui16Max_bytes_temp = (uint16_t)(sDatalog.sMemory_read.ui32Last_address - sDatalog.sMemory_read.ui32Memory_offset + 1);*/
/*                // Daten�bertragung Ende: Zustandswechsel*/
/*                sDatalog.eDatalog_state = eFINISH_MEMORY_READ;*/

/*                // Sofort raus, wenn keine Daten mehr zu schreiben sind*/
/*                if (!ui16Max_bytes_temp)*/
/*                    break;*/
/*            }*/

/*            ui16End_index = SetDataInBufferWriteIndex(ui16Max_bytes_temp);*/
/*            // Memory-Read ausl�sen. Da wir beim Memory Read direkt den Z�hlindex vom USB EP1 Datenbuffer verwenden,*/
/*            // muss dieser vorher noch auf die richtige Stelle gesetzt werden (SetDataInBufferWriteIndex).*/
/*            fram_initiate_transaction (ui16End_index, sDatalog.sMemory_read.ui32Memory_offset,*/
/*                                       &sRoboard_interface.sData_in_endpoint.ui8Endpoint_data[0],*/
/*                                       &sRoboard_interface.sData_in_endpoint.ui16Buffer_write_index, true, true);*/

/*            // Memory Offset anpassen*/
/*            sDatalog.sMemory_read.ui32Memory_offset += ui16Max_bytes_temp;*/
/*        }*/

        break;
    // Hier signalisieren, dass die letzten Bytes durch den Kommunikationskanal geschoben werden k�nnen
    /* case eFINISH_MEMORY_READ: */


    /*     // Wenn alle Daten aus dem Memory �bertragen wurden, alle Transfers abschlie�en. Achtung! Dabei werden */
    /*     // auch eventuelle noch verbliebene Restbytes aus dem Buffer �bertragen! (Daten-Packet auf dem USB-Bus */
    /*     // sind immer 64 Bytes). F�r eine sinnvolle Datenauswertung auf dem Host muss ihm die richtige Datenl�nge */
    /*     // also bekannt sein! */
    /*     if (sFram_interface.eState == eIDLE) */
    /*     { */
    /*         sRoboard_interface.sData_in_endpoint.bConclude_transfer = true; */

    /*         // �bertragungskanal wieder schlie�en. Dabei warten, bis die FSM f�r den Data Endpoint die Transaktion */
    /*         // abgeschlossen hat (Signalisierung durch R�cknahme des bConclude_transfer Flags) */
    /*         if (sRoboard_interface.sData_in_endpoint.eBuffer_state == eLOW_EMPTY_HIGH_EMPTY && */
    /*             sRoboard_interface.sData_in_endpoint.bTransfer_concluded == true) */
    /*         { */
    /*             //USBEndpointDataSend(USB0_BASE, USB_EP_1, USB_TRANS_IN); */
    /*             sRoboard_interface.sData_in_endpoint.bEndpoint_active = false; */
    /*             // Alten Datenloggerzustand wieder herstellen */
    /*             sDatalog.eDatalog_state = sDatalog.eDatalog_old_state; */
    /*         } */
    /*     } */

    /*     break; */

    default:
        break;
    }
    DataloggerSetState(psDatalog);
    
}

//===================================================================================
// Funktion: DataloggerSetState
//===================================================================================
/********************************************************************************//**
 * \brief Set a new datalogger state. 
 *
 * Handles the state transitions from one state to another. 
 ***********************************************************************************/
void DataloggerSetState(tDATALOGGER *psDatalog)
{
    bool successFlag = false;

    // State transition only if there has been flagged no error
    if (psDatalog->eDatalogStatePending != psDatalog->eDatalogState)
    {
        switch (psDatalog->eDatalogState)
        {
            case eDLOGSTATE_UNINITIALIZED:

                // State transition resolver
                switch(psDatalog->eDatalogStatePending)
                {
                    case eDLOGSTATE_FORMAT_MEMORY:
                        // Write the header into the memory
                        // TODO: Implement these functions
                        /*
                        if (checkMemoryInterfaceState() == eROBLOGIF_IDLE)
                            successFlag = memoryWrite((uint8_t*)&psDatalog->sMemoryHeader, 0, MEMORY_HEADER_SIZE());
                        */
                        break;

                    case eDLOGSTATE_INITIALIZED:
                        // No action required, just transition
                        successFlag = true;
                        break;

                    // Desired state transition is illegal
                    default:
                        break;
                }
                break;

            case eDLOGSTATE_INITIALIZED:
                break;

            default:
                break;
        }

        // Change state if the operation was successfull
        if (successFlag)
            psDatalog->eDatalogState = psDatalog->eDatalogStatePending;
    }
}

//===================================================================================
// Function: DataloggerSetStateImmediate
//===================================================================================
/********************************************************************************//**
 * \brief Set a new datalogger state immediately without handling any state
 * transitions. 
 ***********************************************************************************/
void DataloggerSetStateImmediate (tDATALOGGER *psDatalog, tDATALOG_STATE eNewState)
{
    psDatalog->eDatalogState = eNewState;
}
