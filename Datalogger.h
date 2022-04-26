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

#ifndef INCLUDES_DATALOG_H_
#define INCLUDES_DATALOG_H_
/************************************************************************************
 * Includes
 ***********************************************************************************/
#include <stdint.h>

/************************************************************************************
 * Defines
 ***********************************************************************************/
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
    eDATALOG_ERROR_LOG_NUMBER_INVALID       = 2,
    eDATALOG_ERROR_NUMBER_OF_LOGS_EXCEEDED  = 3
}tDATALOG_ERROR;

/************************************************************************************
 * Structure type definitions
 ***********************************************************************************/
// Log configuration structure
typedef struct
{
    uint8_t             ui8ActiveLoggers;
    uint32_t            ui32TimeBase;
    tDATALOG_OPMODES    eOpMode;
}tDATALOG_CONFIG;

#define tDATALOG_CONFIG_DEFAULTS {0, 0, eOPMODE_RECMODERAM}
// #define tDATALOG_CONFIG_DEFAULTS {0}

// typedef struct
// {
//     uint8_t     ui8LogNum;          /*!< Number of the log to fill.*/
//     uint16_t    ui16Divider;        /*!< Frequency divider for this log.*/
//     uint32_t    ui32RecordLength;   /*!< Maximum record length of this log.*/
//     uint8_t    *pui8Variable;       /*!< Memory address of the target variable.*/
//     uint8_t     ui8ByteCount;       /*!< Byte count of the variable.*/
// }tDATALOG_INITS;

// #define tDATALOG_INITS_DEFAULTS {0}

/************************************************************************************
 * Datalog control data
 ***********************************************************************************/

/** @brief Datalog channel data structure */
typedef struct
{
    // Channel config variables
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
    uint32_t    ui32CurMemPos;          /*!<Current memory position*/
    uint32_t    ui32CurrentCount;       /*!< Current record count*/
    uint16_t    ui16DivideCount;        /*!< Count for the frequency divider*/
    // RAM buffer for this channel
    uint8_t*    ui8RamBuf[2]; 
}tDATALOG_CHANNEL;

#define tDATALOG_CHANNEL_DEFAULTS {0, 0, 0, NULL, 0, 0, 0, 0, 0, 0, 1, {NULL}}
// #define tDATALOG_CHANNEL_DEFAULTS {0}

/** @brief Datalog control structure */
typedef struct
{
    tDATALOG_CONFIG     sDataLogConfig;
    uint8_t             ui8ChannelsRunning;
    tDATALOG_CHANNEL    sDatalogChannels[MAX_NUM_LOGS];
    uint8_t            *pui8Data;
    uint32_t            ui32CurrentByteIdx;
}tDATALOG_CONTROL;

#define tDATALOG_CONTROL_DEFAULTS {tDATALOG_CONFIG_DEFAULTS, 0, {tDATALOG_CHANNEL_DEFAULTS}, NULL, 0}
// #define tDATALOG_CONTROL_DEFAULTS {0}

/************************************************************************************
 * Record memory mode header module
 ***********************************************************************************/
/** @brief Log description header (Leading information for each log) */
typedef struct
{
    uint16_t ui16Divider;           // Timebase frequency divider
    uint32_t ui32MemoryOffset;      // Offset address of the channel
    uint8_t *pui8Variable;           /*!< Memory address of the target variable.*/
    uint8_t  ui8ByteCount;           /*!< Byte count of the variable.*/
}tDATALOG_CHANNEL_MEMORY;

#define tDATALOG_CHANNEL_MEMORY_DEFAULTS {0, 0, NULL, 0}
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

#define tDATALOG_RECMODEMEM_SERIALIIZER_DEFAULTS {0}

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

// RobLog Main data struct
typedef struct
{
    // Datalog-Parameter
    tDATALOG_STATE                  eDatalogState;
    tDATALOG_STATE                  eDatalogStatePending;
    tDATALOG_LIVEMODE_TIMER         sDatalogLiveModeTimer;
    tDATALOG_MEMORY_HEADER          sMemoryHeader;
    tDATALOG_RECMODEMEM_SERIALIZER  sDatalogSerializer;
    tDATALOG_CONTROL                sDatalogControl;
}tDATALOGGER;

#define tDATALOGGER_DEFAULTS {\
    eDLOGSTATE_UNINITIALIZED, \
    eDLOGSTATE_UNINITIALIZED, \
    tDATALOG_LIVEMODE_TIMER_DEFAULTS, \
    tDATALOG_MEMORY_HEADER_DEFAULTS, \
    tDATALOG_RECMODEMEM_SERIALIIZER_DEFAULTS,\
    tDATALOG_CONTROL_DEFAULTS}
// #define tDATALOGGER_DEFAULTS {0}

/************************************************************************************
 * Funktionsdeklarationen
 ***********************************************************************************/
// API functions
tDATALOG_ERROR RegisterLog (uint8_t ui8LogNum, uint16_t ui16FreqDiv, uint32_t ui32RecLen, uint8_t *pui8Variable, uint8_t ui8ByteCount);
tDATALOG_ERROR DatalogInitialize (tDATALOG_CONFIG sDatalogConfig);
tDATALOG_ERROR DatalogStart (void);
bool DatalogStop (void);
// Datalog service methods
void DatalogService (void);
void DatalogStatemachine (void);
// Internal functions
static void DatalogSetState (void);
static void DatalogSetStateImmediate (tDATALOG_STATE eNewState);

/* extern bool LiveModeDatalogStart (uint8_t ui8Var_count, uint16_t* pui16Var_nr, uint16_t pui16Divider); */
/* extern bool LiveModeDatalogStop(void); */
/* void LiveModeSampleData (void); */
/* extern void LiveModeTimerInit (void); */
// void LiveModeCallback (void);
/* extern bool InitializeMemoryRead (void); */

#endif // INCLUDES_DATALOG_H_
// EOF datalog.h-------------------------------------------------------------------
