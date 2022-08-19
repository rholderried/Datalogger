/********************************************************************************//**
 * \file Datalogger.h
 * \author Roman Holderried
 *
 * \brief Typedefs, declarations, etc. for the RobLib datalogger.
 *
 * <b> History </b>
 *      - 2021-04-20: Adapted file from the RoBoard project.
 *                     
 ***********************************************************************************/

#ifndef DATALOGGER_H_
#define DATALOGGER_H_
/************************************************************************************
 * Includes
 ***********************************************************************************/
#include <stdint.h>

/************************************************************************************
 * Defines
 ***********************************************************************************/
#define DATALOGGER_VERSION_MAJOR    0
#define DATALOGGER_VERSION_MINOR    5
#define DATALOGGER_REVISION         0

#define MEMORY_HEADER_SIZE()       (sizeof(tDATALOG_MEMORY_HEADER))
#define MAX_NUM_LOGS                8
#define ARBITRATION_BUFFER_SIZE     20

// Live-Datenlogger
#define MIN_SAMPLE_TIME_MS          2
// Configuration
#define MAX_NUM_CONFIGS             7

#if NUMBER_OF_CONFIGS > MAX_NUM_CONFIGS
#error NUMBER_OF_CONFIGS must be smaller than MAX_NUM_CONFIGS
#endif

/************************************************************************************
 * Enum Type definitions
 ***********************************************************************************/
typedef enum
{
    // Fehler
    eDLOGSTATE_ERROR = -1,

    // Internal Mode Datalog
    eDLOGSTATE_UNINITIALIZED            = 0,   /*!< New logs have been registered. Also POR value. */
    eDLOGSTATE_FORMAT_MEMORY,                  /*!< Writing Header into external memory */
    eDLOGSTATE_INITIALIZED,                    /*!< Datalogger ready to start, not running */
    eDLOGSTATE_DATA_READY,                     /*!< Datalogger has been stopped, data is ready to retrieve*/

    eDLOGSTATE_RUNNING,
    eDLOGSTATE_ABORTING,
}tDATALOG_STATE;

typedef enum
{
    eOPMODE_RECMODERAM  = 0,
    eOPMODE_RECMODEMEM  = 1,
    eOPMODE_LIVE        = 2 
}tDATALOG_OPMODES;

typedef enum
{
    eDATALOG_ERROR_NONE                     = 0,
    eDATALOG_ERROR_WRONG_STATE              = 1,
    eDATALOG_ERROR_WRONG_OPMODE             = 2,
    eDATALOG_ERROR_LOG_NUMBER_INVALID       = 3,
    eDATALOG_ERROR_NUMBER_OF_LOGS_EXCEEDED  = 4,
    eDATALOG_ERROR_CHANNEL_NOT_ACTIVE       = 5,
    eDATALOG_ERROR_NOT_ENOUGH_MEMORY        = 6,
    eDATALOG_ERROR_NO_DATA                  = 7,
    eDATALOG_ERROR_NOT_IMPLEMENTED          = 8
}tDATALOG_ERROR;

/************************************************************************************
 * Structure type definitions
 ***********************************************************************************/
/** @brief Datalogger version data structure */
typedef struct
{
    uint8_t ui8VersionMajor;
    uint8_t ui8VersionMinor;
    uint8_t ui8Revision;
}tDATALOGGER_VERSION;

/************************************************************************************
 * Datalog control data
 ***********************************************************************************/

/** @brief Datalogger callbacks */
typedef struct
{
    void(*StartDataloggerCb)(void);
    void(*StopDataloggerCb)(void);
}tDATALOGGER_CALLBACKS;

#define tDATALOGGER_CALLBACKS_DEFAULTS {NULL, NULL}

/** @brief Datalog channel data structure */
typedef struct
{
    // Channel config variables
    uint32_t    ui32ChID;               /*!< Channel Identification variable.*/
    uint16_t    ui16Divider;            /*!< Frequency divider of this channel.*/
    uint32_t    ui32MemoryOffset;       /*!< Log buffer offset of the channel.*/
    uint32_t    ui32RecordLength;       /*!< Record length of the channel */
    uint8_t    *pui8Variable;           /*!< Memory address of the target variable.*/
    uint8_t     ui8ByteCount;           /*!< Byte count of the variable.*/
    // Channel parameter variables
    uint16_t    ui16RetrieveThreshIdx; /*!< Retrieve threshold index of this channel*/
    // Channel state variables
    uint8_t     ui8BufNum;              /*!< Current buffer number*/
    uint16_t    ui16ValIdx;             /*!< Current buffer value index*/
    uint32_t    ui32CurMemPos;          /*!< Current memory position*/
    uint32_t    ui32CurrentCount;       /*!< Current record count*/
    uint16_t    ui16DivideCount;        /*!< Count for the frequency divider*/
    // RAM buffer for this channel
    uint8_t*    ui8RamBuf[2]; 
}tDATALOG_CHANNEL;

#define tDATALOG_CHANNEL_DEFAULTS {0, 0, 0, 0, NULL, 0, 0, 0, 0, 0, 0, 1, {NULL}}
// #define tDATALOG_CHANNEL_DEFAULTS {0}

/** @brief Datalog control structure */
typedef struct
{
    tDATALOG_OPMODES    eOpMode;
    uint8_t             ui8ActiveLoggers;
    uint8_t             ui8ChannelsRunning;
    uint32_t            ui32MemLen;
    uint8_t             *pui8Data;
    tDATALOG_CHANNEL    sDatalogChannels[MAX_NUM_LOGS];
}tDATALOG_CONTROL;

#define tDATALOG_CONTROL_DEFAULTS {eOPMODE_RECMODERAM, 0, 0, 0, NULL, {tDATALOG_CHANNEL_DEFAULTS}}
// #define tDATALOG_CONTROL_DEFAULTS {0}

/************************************************************************************
 * Record memory mode header module
 ***********************************************************************************/
/** @brief Log description header (Leading information for each log) */
typedef struct
{
    uint32_t ui32ChannelID;     /*!< Channel Identification variable.*/
    uint16_t ui16Divider;       /*!< Timebase frequency divider*/
    uint32_t ui32MemoryOffset;  /*!< Offset address of the channel*/
    uint8_t *pui8Variable;      /*!< Memory address of the target variable.*/
    uint8_t  ui8ByteCount;      /*!< Byte count of the variable.*/
}tDATALOG_CHANNEL_MEMORY;

#define tDATALOG_CHANNEL_MEMORY_DEFAULTS {0, 0, 0, NULL, 0}
// #define tDATALOG_CHANNEL_MEMORY_DEFAULTS {0}

/** @brief Header for the data on an external storage medium */
typedef struct
{
    uint32_t                    ui32TimeBase;
    uint32_t                    ui32LastAddress;
    tDATALOG_CHANNEL_MEMORY     sDatalogChannelsMemory[MAX_NUM_LOGS];
}tDATALOG_MEMORY_HEADER;

#define tDATALOG_MEMORY_HEADER_DEFAULTS {0, 0, {tDATALOG_CHANNEL_MEMORY_DEFAULTS}}
// #define tDATALOG_MEMORY_HEADER_DEFAULTS {0}

/************************************************************************************
 * Serializer control struct 
 ***********************************************************************************/
typedef struct
{
    // Control variables for the internal scheduler of the datalogger
    uint8_t     ui8ArbitrationBuffer[ARBITRATION_BUFFER_SIZE];
    uint8_t     ui8FillIdx;
    uint8_t     ui8RetrieveIdx;
    uint8_t     ui8ArbitrationCount;
    uint8_t     ui8RetrieveFlags;
}tDATALOG_RECMODEMEM_SERIALIZER;

#define tDATALOG_RECMODEMEM_SERIALIIZER_DEFAULTS {{0}, 0, 0, 0, 0}

/* typedef struct */
/* { */
/*     tDATALOG_HEADER                 sHeader; */
/*     tDATALOG_MEMORY_HEADER          sMemoryHeader; */
/*     tDATALOG_RECMODEMEM_SCHEDULER   sMemoryScheduler; */
/* }tDATALOG_INTERNAL; */

/* #define tsDATALOG_INTERNAL_DEFAULTS {\ */
/*     tsDATALOG_HEADER_DEFAULTS, \ */
/*     tsDATALOG_HEADER_EXTERN_DEFAULTS, \ */
/*     tDATALOG_RECMODEMEM_SCHEDULER_DEFAULTS \ */
/* } */

/** @brief Structure for the Live-Mode Datalogger*/
typedef struct
{
    int8_t      i8SampleTimerIdx;
    int8_t      i8TimestampTimerIdx;
}tDATALOG_LIVEMODE_TIMER;

#define tDATALOG_LIVEMODE_TIMER_DEFAULTS {-1, -1}
// #define tDATALOG_LIVEMODE_TIMER_DEFAULTS {0}

/* typedef struct */
/* { */
/*     uint32_t ui32MemoryOffset; */
/*     uint32_t ui32LastAddress; */

/* }tDATALOG_MEMORY_READ; */

/* #define tsDATALOG_MEMORY_READ_DEFAULTS {0, 0} */

// Main data struct of the datalogger
typedef struct
{
    tDATALOGGER_VERSION             sVersion;
    // Datalog-Parameter (Non-Volatile)
    struct
    {
        uint32_t                    ui32EE_TimeBase_Hz;
    }sNVPar;

    tDATALOG_STATE                  eDatalogState;
    tDATALOG_STATE                  eDatalogStatePending;
    tDATALOG_LIVEMODE_TIMER         sDatalogLiveModeTimer;
    tDATALOG_MEMORY_HEADER          sMemoryHeader;
    tDATALOG_RECMODEMEM_SERIALIZER  sDatalogSerializer;
    tDATALOG_CONTROL                sDatalogControl;
    tDATALOGGER_CALLBACKS           sCallbacks;
}tDATALOGGER;

#define tDATALOGGER_DEFAULTS {\
    {DATALOGGER_VERSION_MAJOR, DATALOGGER_VERSION_MINOR, DATALOGGER_REVISION},\
    {0},\
    eDLOGSTATE_UNINITIALIZED, \
    eDLOGSTATE_UNINITIALIZED, \
    tDATALOG_LIVEMODE_TIMER_DEFAULTS, \
    tDATALOG_MEMORY_HEADER_DEFAULTS, \
    tDATALOG_RECMODEMEM_SERIALIIZER_DEFAULTS,\
    tDATALOG_CONTROL_DEFAULTS,\
    tDATALOGGER_CALLBACKS_DEFAULTS}
// #define tDATALOGGER_DEFAULTS {0}

/************************************************************************************
 * Function declarations
 ***********************************************************************************/
/********************************************************************************//**
 * \brief Initializes the datalogger structure
 ***********************************************************************************/
void DataloggerInit(tDATALOGGER *psDatalog, tDATALOGGER_CALLBACKS sCallbacks);

/********************************************************************************//**
 * \brief Resets the datalogger structure
 ***********************************************************************************/
tDATALOG_ERROR DataloggerReset(tDATALOGGER *psDatalog);

/********************************************************************************//**
 * \brief API function to free formerly allocated memory
 ***********************************************************************************/
void DataloggerClearMemory(tDATALOGGER *psDatalog);

/********************************************************************************//**
 * \brief Returns a pointer to the usually hidden data structure
 ***********************************************************************************/
#ifdef UNITTEST
tDATALOGGER* Datalogger_GetData(tDATALOGGER *psDatalog);
#endif

/********************************************************************************//**
 * \brief Return the current state of the 
 ***********************************************************************************/
tDATALOG_STATE DataloggerGetCurrentState(tDATALOGGER *psDatalog);

/********************************************************************************//**
 * \brief Return the current operation mode of the datalogger
 ***********************************************************************************/
tDATALOG_OPMODES DataloggerGetCurrentOpMode(tDATALOGGER *psDatalog);

/********************************************************************************//**
 * \brief Sets the Datalogger to a new operation mode.
 * 
 * @param eNewOpMode    New operation mode to be set.
 ***********************************************************************************/
tDATALOG_ERROR DataloggerSetOpMode(tDATALOGGER *psDatalog, tDATALOG_OPMODES eNewOpMode);

/********************************************************************************//**
 * \brief Returns the current operation mode of the datalogger
 * 
 * @param pui8Data  Data pointer to be set to the top of the data memory.
 * @param ui32Len   Pointer to the variable that shall hold the length of the log 
 *                  data buffer.
 ***********************************************************************************/
tDATALOG_ERROR DataloggerGetDataPtr(tDATALOGGER *psDatalog, uint8_t** pui8Data, uint32_t *ui32Len);

/********************************************************************************//**
 * \brief Returns information of the 
 * 
 * @param pChannel  Pointer to the data target.
 * @param ui8ChNum  Channel number to request.
 ***********************************************************************************/
tDATALOG_ERROR DataloggerGetChannelInfo(tDATALOGGER *psDatalog, tDATALOG_CHANNEL *pChannel, uint8_t ui8ChNum);

/********************************************************************************//**
 * \brief Returns the datalogger version structure
 ***********************************************************************************/
tDATALOGGER_VERSION DataloggerGetVersion(tDATALOGGER *psDatalog);


/********************************************************************************//**
 * \brief This function registers a log into the data structure
 *
 * @param   ui32ChID        Identifier of the channel.
 * @param   ui8LogNum       Log number 1 - LOG_NUM_MAX
 * @param   ui16FreqDiv     Frequency divider, determines the sample time together with
 *                          the time base frequency.
 * @param   ui32RecLen      Length (items, not bytes) of the datalog.
 * @param   pui8Variable    Pointer to the variable to log.
 * @param   ui8ByteCount    Byte count of the variable.
 * 
 * @returns Error indicator
 ***********************************************************************************/
tDATALOG_ERROR DataloggerRegisterLog (tDATALOGGER *psDatalog, uint32_t ui32ChID, uint8_t ui8LogNum, uint16_t ui16FreqDiv, uint32_t ui32RecLen, uint8_t *pui8Variable, uint8_t ui8ByteCount);
tDATALOG_ERROR DataloggerRemoveLog (tDATALOGGER *psDatalog, uint8_t ui8LogNum);
tDATALOG_ERROR DataloggerInitLogger (tDATALOGGER *psDatalog, bool bFreeMemory);
tDATALOG_ERROR DataloggerStart (tDATALOGGER *psDatalog);
tDATALOG_STATE DataloggerStop (tDATALOGGER *psDatalog);
// Datalog service methods
void DataloggerService (tDATALOGGER *psDatalog);
void DataloggerStatemachine (tDATALOGGER *psDatalog);
// Internal functions
void DataloggerSetState (tDATALOGGER *psDatalog);
void DataloggerSetStateImmediate (tDATALOGGER *psDatalog, tDATALOG_STATE eNewState);


/* extern bool LiveModeDatalogStart (uint8_t ui8Var_count, uint16_t* pui16Var_nr, uint16_t pui16Divider); */
/* extern bool LiveModeDatalogStop(void); */
/* void LiveModeSampleData (void); */
/* extern void LiveModeTimerInit (void); */
// void LiveModeCallback (void);
/* extern bool InitializeMemoryRead (void); */

#endif // DATALOGGER_H_
// EOF datalog.h-------------------------------------------------------------------
