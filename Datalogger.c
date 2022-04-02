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
#include "RobLog.h"
#include "RobLogIF.h"
#include "Datalogger.h"
/************************************************************************************
 * Defines
 ***********************************************************************************/

/************************************************************************************
 * State variable
 ***********************************************************************************/
static tDATALOGGER sDatalogger = tDATALOGGER_DEFAULTS;

/************************************************************************************
 * Globals
 ***********************************************************************************/
extern uint8_t ui8Byte_count[];

/************************************************************************************
 * Function definitions
 ***********************************************************************************/

//===================================================================================
// Funktion: RegisterLog
//===================================================================================
/********************************************************************************//**
 * \brief This function registers a log into the data structure
 *
 * @param   initializer
 * @returns Error indicator
 ***********************************************************************************/
bool RegisterLog (tDATALOG_INITS sDatalogInits)
{
    tDATALOG_CHANNEL    *pChannel = &sDatalogger.sDatalogControl.sDatalogChannels[sDatalogInits.ui8LogNum - 1];
    tDATALOG_CHANNEL_MEMORY   *pMemChannel = &sDatalogger.sMemoryHeader.sDatalogChannelsMemory[sDatalogInits.ui8LogNum - 1];
    uint16_t    ui16VarIdx;

    if (sDatalogger.eDatalogState != eDLOGSTATE_INITIALIZED ||
        sDatalogger.eDatalogState != eDLOGSTATE_UNINITIALIZED)
        return false;

    if (sDatalogInits.ui8LogNum > MAX_NUM_LOGS - 1)
        return false;

    // Get the index of the variable
    ui16VarIdx = ramVarGetIdx(sDatalogInits.ui16VarNr);

    if (ui16VarIdx < 0)
        return false;

    // Initialize parameter variables
    pChannel->ui16Index = pMemChannel->ui16Index = ui16VarIdx;
    pChannel->ui16Divider = pMemChannel->ui16Divider = sDatalogInits.ui16Divider;
    pChannel->ui32RecordLength = sDatalogInits.ui32RecordLength;


    // New value is set -> Need to initialize the datalogger prior to next log run.
    sDatalogger.eDatalogState = eDLOGSTATE_UNINITIALIZED;

    return true;
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
bool DatalogInitialize (tDATALOG_CONFIG sDatalogConfig)
{
    uint8_t     i = 0;
    uint8_t     ui8LogCount = 0;
    uint8_t     ui8LogIdx[MAX_NUM_LOGS] = {0};
    uint16_t    ui16TempSize;
    tDATALOG_CHANNEL *pChannel;

    // Get out of this function if the state is not correct
    if (sDatalogger.eDatalogState != eDLOGSTATE_UNINITIALIZED)
        return false;

    // If memory has been already allocated, free these memory portions
    while (sDatalogger.sDatalogControl.sDataLogConfig.ui8ActiveLoggers > 0)
    {
        if(sDatalogger.sDatalogControl.sDataLogConfig.eOpMode != eOPMODE_LIVEMODE)
        {
            if (sDatalogger.sDatalogControl.sDataLogConfig.ui8ActiveLoggers & (1 << i))
            {
                free(sDatalogger.sDatalogControl.sDatalogChannels[i].pvRamBuf[0]);

                if (sDatalogger.sDatalogControl.sDataLogConfig.eOpMode == eOPMODE_RECMODEMEM)
                    free(sDatalogger.sDatalogControl.sDatalogChannels[i].pvRamBuf[1]);

                sDatalogger.sDatalogControl.sDataLogConfig.ui8ActiveLoggers &= ~(1 << i);
            }
        }
        i++;
    }

    // Preinitialize Datalog structure
    for(i = 0; i < MAX_NUM_LOGS; i++)
    {
        if (!(sDatalogConfig.ui8ActiveLoggers & (1 << i)))
            continue;

        ui8LogIdx[ui8LogCount] = i;

        // Determine the offset of the current channel in external memory
        if (sDatalogConfig.eOpMode == eOPMODE_RECMODEMEM)
        {
            if (ui8LogCount > 0)
            {
                sDatalogger.sMemoryHeader.sDatalogChannelsMemory[i].ui32MemoryOffset = 
                sDatalogger.sMemoryHeader.sDatalogChannelsMemory[ui8LogIdx[ui8LogCount - 1]].ui32MemoryOffset +
                sDatalogger.sDatalogControl.sDatalogChannels[ui8LogIdx[ui8LogCount - 1]].ui32RecordLength *
                ui8Byte_count[ramVars[sDatalogger.sDatalogControl.sDatalogChannels[ui8LogIdx[ui8LogCount - 1]].ui16Index].ui8DataType];
            }
            else
            {
                sDatalogger.sMemoryHeader.sDatalogChannelsMemory[i].ui32MemoryOffset = 
                sDatalogger.sDatalogControl.sDatalogChannels[ui8LogIdx[ui8LogCount]].ui32CurMemPos = MEMORY_HEADER_SIZE();
            }
        }
        ui8LogCount++;
    }


    /********************************************************************************
     * RAM initialization routines
     *******************************************************************************/
    if (sDatalogConfig.eOpMode != eOPMODE_LIVEMODE)
    {
        ui16TempSize = LOG_MEMORY_SIZE / (ui8LogCount << 1);

        for(i = 0; i < ui8LogCount; i++)
        {
            pChannel = &sDatalogger.sDatalogControl.sDatalogChannels[ui8LogIdx[i]];

            if (sDatalogConfig.eOpMode == eOPMODE_RECMODEMEM)
            {
                pChannel->pvRamBuf[0] = malloc((size_t)(ui16TempSize));
                pChannel->pvRamBuf[1] = malloc((size_t)(ui16TempSize));

                pChannel->ui16RetrieveThreshIdx = ui16TempSize / 
                        ui8Byte_count[ramVars[pChannel->ui16Index].ui8DataType] - 1;
            }
            // Operation mode == eOPMODE_RECMODERAM
            else
            {
                pChannel->pvRamBuf[0] = 
                    malloc((size_t)(pChannel->ui32RecordLength * 
                                ui8Byte_count[ramVars[pChannel->ui16Index].ui8DataType]));
            }

        }
    }
    
    //TODO: LIVEMODE Ringbuffer initialization

    /********************************************************************************
     * Memory Record mode related variables
     *******************************************************************************/
    if (sDatalogConfig.eOpMode == eOPMODE_RECMODEMEM)
    {
        // Note: pChannel still pointing on the last active memory channel
        sDatalogger.sMemoryHeader.ui32LastAddress = 
            sDatalogger.sMemoryHeader.sDatalogChannelsMemory[ui8LogIdx[ui8LogCount - 1]].ui32MemoryOffset + 
            pChannel->ui32RecordLength - 1;

        sDatalogger.sMemoryHeader.ui32TimeBase = sDatalogConfig.ui32TimeBase;

    }

    sDatalogger.sDatalogControl.sDataLogConfig = sDatalogConfig;

    if (sDatalogConfig.eOpMode != eOPMODE_RECMODEMEM)
        sDatalogger.eDatalogStatePending = eDLOGSTATE_INITIALIZED;
    else 
        sDatalogger.eDatalogStatePending = eDLOGSTATE_INITIALIZE_EXTERNAL_MEMORY;

    return true;
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
bool DatalogStart (void)
{
    uint8_t i = 0;
    tDATALOG_CHANNEL* pChannel = sDatalogger.sDatalogControl.sDatalogChannels;

    if (sDatalogger.eDatalogState != eDLOGSTATE_INITIALIZED)
        return false;

    // Alle Laufvariablen zur�cksetzen
    while (i < MAX_NUM_LOGS)
    {
        if (!(sDatalogger.sDatalogControl.sDataLogConfig.ui8ActiveLoggers & (1 << i)))
            continue;

        // Reset of the state variables
        pChannel[i].ui8BufNum = 0;
        pChannel[i].ui16DivideCount = 1;
        pChannel[i].ui16ValIdx = 0;
        pChannel[i].ui32CurrentCount = 0;

        if(sDatalogger.sDatalogControl.sDataLogConfig.eOpMode == eOPMODE_RECMODEMEM)
        {
            pChannel[i].ui32CurMemPos = sDatalogger.sMemoryHeader.sDatalogChannelsMemory[i].ui32MemoryOffset;

            // Reset the control variables of the serializer
            sDatalogger.sDatalogSerializer.ui8ArbitrationCount = 0;
            sDatalogger.sDatalogSerializer.ui8FillIdx = 0;
            sDatalogger.sDatalogSerializer.ui8RetrieveFlags = 0;
            sDatalogger.sDatalogSerializer.ui8RetrieveIdx = 0;
        }

        i++;
    }

    sDatalogger.sDatalogControl.ui8ChannelsRunning = 
        sDatalogger.sDatalogControl.sDataLogConfig.ui8ActiveLoggers;

    // Switch the datalog on (directly, because this is time critical)
    DataloggerSetStateImmediate((tDATALOG_STATE)(eDLOGSTATE_RECMODERAM_RUNNING + 
                                sDatalogger.sDatalogControl.sDataLogConfig.eOpMode));

    return true;
}

//===================================================================================
// Function: DatalogStop
//===================================================================================
/********************************************************************************//**
 * \brief Stops the datalogger
 *
 * @returns Error indicator
 ***********************************************************************************/
bool DatalogStop (void)
{
    if (sDatalogger.eDatalogState < eDLOGSTATE_RECMODERAM_RUNNING ||
        sDatalogger.eDatalogState > eDLOGSTATE_LIVEMODE_RUNNING)
        return false;

    DataloggerSetStateImmediate((tDATALOG_STATE)(eDLOGSTATE_RECMODERAM_FINISHING+ 
                                sDatalogger.sDatalogControl.sDataLogConfig.eOpMode));

    // Arbitration-count definiert auf 0 setzen
    sDatalogger.sDatalogSerializer.ui8ArbitrationCount = 0;

    return true;
}

//===================================================================================
// Funktion: RobLogService
//===================================================================================
/********************************************************************************//**
 * \brief Samples the previously selected values
 *
 * This routine must get called regularly with a defined time base.
 ***********************************************************************************/
void DataloggerService (void)
{
    uint8_t i = 0;
    bool bAbort_flag = false;
    tDATALOG_CHANNEL *pChannel = sDatalogger.sDatalogControl.sDatalogChannels;

    if (sDatalogger.eDatalogState != eDLOGSTATE_RECMODERAM_RUNNING && 
        sDatalogger.eDatalogState != eDLOGSTATE_RECMODEMEM_RUNNING)
        return;

    // Alle Daten einspeichern
    while (i < MAX_NUM_LOGS)
    {
        // Check if Channel is activated and running
        if (!(sDatalogger.sDatalogControl.sDataLogConfig.ui8ActiveLoggers & (1 << i)) ||
            !(sDatalogger.sDatalogControl.ui8ChannelsRunning & (1 << i)))
            continue;

        if (!(--pChannel[i].ui16DivideCount))
        {
            // Typabh�ngiges Buffern der Daten ins RAM
            switch (ramVars[pChannel[i].ui16Index].ui8DataType)
            {
            case  TYPE_UI8:
                ((uint8_t*)pChannel[i].pvRamBuf[pChannel->ui8BufNum])[pChannel[i].ui16ValIdx] = 
                    *(uint8_t*)ramVars[pChannel[i].ui16Index].pVar;
                break;
            case  TYPE_I8:
                ((int8_t*)pChannel[i].pvRamBuf[pChannel->ui8BufNum])[pChannel[i].ui16ValIdx] = 
                    *(int8_t*)ramVars[pChannel[i].ui16Index].pVar;
                break;
            case  TYPE_UI16:
                ((uint16_t*)pChannel[i].pvRamBuf[pChannel->ui8BufNum])[pChannel[i].ui16ValIdx] = 
                    *(uint16_t*)ramVars[pChannel[i].ui16Index].pVar;
                break;
            case  TYPE_I16:
                ((int16_t*)pChannel[i].pvRamBuf[pChannel->ui8BufNum])[pChannel[i].ui16ValIdx] = 
                    *(int16_t*)ramVars[pChannel[i].ui16Index].pVar;
                break;
            case  TYPE_UI32:
                ((uint32_t*)pChannel[i].pvRamBuf[pChannel->ui8BufNum])[pChannel[i].ui16ValIdx] = 
                    *(uint32_t*)ramVars[pChannel[i].ui16Index].pVar;
                break;
            case  TYPE_I32:
                ((int32_t*)pChannel[i].pvRamBuf[pChannel->ui8BufNum])[pChannel[i].ui16ValIdx] = 
                    *(int32_t*)ramVars[pChannel[i].ui16Index].pVar;
                break;
            case  TYPE_F32:
                ((float*)pChannel[i].pvRamBuf[pChannel->ui8BufNum])[pChannel[i].ui16ValIdx] = 
                    *(float*)ramVars[pChannel[i].ui16Index].pVar;
                break;
            }
            
            if (sDatalogger.sDatalogControl.sDataLogConfig.eOpMode == eOPMODE_RECMODEMEM)
            {
                // Set flag to empty the currently used buffer
                if (pChannel[i].ui16ValIdx++ == pChannel[i].ui16RetrieveThreshIdx)
                    sDatalogger.sDatalogSerializer.ui8RetrieveFlags |= (1 << i);

                // Debug-Funktion: Mitloggen des maximalen Value-Indexes
                /* if (pInstance[i].ui16Val_idx > pInstance[i].ui16Debug_max_val_idx) */
                /*     pInstance[i].ui16Debug_max_val_idx = pInstance[i].ui16Val_idx; */
                // Make sure that no buffer overflow can happen
                if (pChannel[i].ui16ValIdx == pChannel[i].ui16RetrieveThreshIdx << 1)
                    bAbort_flag = true;
            }

            // Switch off channel if it has reached its maximum count
            if (++pChannel[i].ui32CurrentCount == pChannel[i].ui32RecordLength)
                sDatalogger.sDatalogControl.ui8ChannelsRunning &= ~(1 << i);

            pChannel[i].ui16DivideCount = pChannel[i].ui16Divider;
        }
    }

    // If all channels reached their record length, switch off datalogger
    bAbort_flag |= (bool)~(sDatalogger.sDatalogControl.sDataLogConfig.ui8ActiveLoggers & 
                           sDatalogger.sDatalogControl.ui8ChannelsRunning);

    if (bAbort_flag)
        DatalogStop();
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
void DataloggerStatemachine (void)
{
    //tROBLOG_ERROR eErrorIndicator = eROBLOG_ERROR_NONE;
    tDATALOG_STATE eNewState = sDatalogger.eDatalogState;
    /* uint16_t ui16Data_size; */
    /* uint16_t ui16End_index; */
    /* uint16_t ui16RAM_buffer_offset; */
    /* uint32_t ui32Memory_address_temp; */
    /* uint16_t ui16Max_bytes_temp; */


    switch (sDatalogger.eDatalogState)
    {
    case eDLOGSTATE_UNINITIALIZED:
        break;


    case eDLOGSTATE_INITIALIZE_EXTERNAL_MEMORY:

        // Wait for the memory write before changing the state angain
        // TODO: Functions not implemented yet
        /*if (checkMemoryInterfaceState() == eROBLOGIF_IDLE)
            sDatalogger.eDatalogStatePending = eDLOGSTATE_RECMODEMEM_RUNNING;*/

        break;

    case eDLOGSTATE_INITIALIZED:
        // Wait for the datalogger to be started (Done by API)
        break;

    case eDLOGSTATE_RECMODEMEM_RUNNING:

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

    case eDLOGSTATE_RECMODERAM_RUNNING:
        break;

    case eDLOGSTATE_RECMODEMEM_FINISHING:
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
        break;

    case eDLOGSTATE_RECMODERAM_FINISHING:
        break;

    // Live-Datenlogger
    case eDLOGSTATE_LIVEMODE_RUNNING:

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
    DataloggerSetState();
    
}

//===================================================================================
// Funktion: DataloggerSetState
//===================================================================================
/********************************************************************************//**
 * \brief Set a new datalogger state. 
 *
 * Handles the state transitions from one state to another. 
 ***********************************************************************************/
static void DataloggerSetState()
{
    bool successFlag = false;

    // State transition only if there has been flagged no error
    if (sDatalogger.eDatalogStatePending != sDatalogger.eDatalogState)
    {
        switch (sDatalogger.eDatalogState)
        {
            case eDLOGSTATE_UNINITIALIZED:

                // State transition resolver
                switch(sDatalogger.eDatalogStatePending)
                {
                    case eDLOGSTATE_INITIALIZE_EXTERNAL_MEMORY:
                        // Write the header into the memory
                        // TODO: Implement these functions
                        /*
                        if (checkMemoryInterfaceState() == eROBLOGIF_IDLE)
                            successFlag = memoryWrite((uint8_t*)&sDatalogger.sMemoryHeader, 0, MEMORY_HEADER_SIZE());
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
            sDatalogger.eDatalogState = sDatalogger.eDatalogStatePending;
    }
}

//===================================================================================
// Function: DataloggerSetStateImmediate
//===================================================================================
/********************************************************************************//**
 * \brief Set a new datalogger state immediately without handling any state
 * transitions. 
 ***********************************************************************************/
void DataloggerSetStateImmediate (tDATALOG_STATE eNewState)
{
    sDatalogger.eDatalogState = sDatalogger.eDatalogStatePending = eNewState;
}
