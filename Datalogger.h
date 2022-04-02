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
#include "RobLog.h"

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
    eDLOGSTATE_UNINITIALIZED = 0,           /*!< New logs have been registered. Also POR value. */
    eDLOGSTATE_INITIALIZE_EXTERNAL_MEMORY,  /*!< Writing Header into external memory */
    eDLOGSTATE_INITIALIZED,                 /*!< Datalogger ready to start, not running */

    eDLOGSTATE_RECMODERAM_RUNNING,
    eDLOGSTATE_RECMODEMEM_RUNNING,
    eDLOGSTATE_LIVEMODE_RUNNING,

    eDLOGSTATE_RECMODERAM_FINISHING,
    eDLOGSTATE_RECMODEMEM_FINISHING,
    eDLOGSTATE_LIVEMODE_FINISHING,

    eDLOGSTATE_RECMODERAM_ABORTING,
    eDLOGSTATE_RECMODEMEM_ABORTING,
    eDLOGSTATE_LIVEMODE_ABORTION,

    // Memory Read-Out
    eDLOGSTATE_MEMORY_READOUT_RAM,
    eDLOGSTATE_MEMORY_READOUT_FINISHING
}tDATALOG_STATE;

typedef enum
{
    eOPMODE_NONE        = -1,
    eOPMODE_RECMODERAM  = 0,
    eOPMODE_RECMODEMEM  = 1,
    eOPMODE_LIVEMODE    = 2 
}tROBLOG_OPMODES;

typedef enum
{
    eDATALOG_ERROR_WRONG_STATE,
    eDATALOG_ERROR_LOG_NUMBER_INVALID,
    // Errors from other modules
    eDATALOG_ERROR_VARIABLE_ACCESS,  // Varible Access module
}tDATALOG_ERROR;

/************************************************************************************
 * Structure type definitions
 ***********************************************************************************/
// Log configuration structure
typedef struct
{
    uint8_t             ui8ActiveLoggers;
    uint32_t            ui32TimeBase;
    tROBLOG_OPMODES     eOpMode;
}tDATALOG_CONFIG;

#define tDATALOG_CONFIG_DEFAULTS {0, 0, eOPMODE_NONE}

typedef struct
{
    uint8_t     ui8LogNum;
    uint16_t    ui16VarNr;
    uint16_t    ui16Divider;
    uint32_t    ui32RecordLength;
}tDATALOG_INITS;

#define tDATALOG_INITS_DEFAULTS {0}

/************************************************************************************
 * Datalog control data
 ***********************************************************************************/
typedef struct
{
    // Channel config variables
    uint16_t    ui16Index;              // Var index in the variable structure
    uint16_t    ui16Divider;            // Frequency divider
    uint32_t    ui32RecordLength;       // Record length of the channel
    // Channel parameter variables
    uint16_t    ui16RetrieveThreshIdx;  // Retrieve threshold index of this channel
    // Channel state variables
    uint8_t     ui8BufNum;              // Current buffer number
    uint16_t    ui16ValIdx;             // Current buffer value index
    uint32_t    ui32CurMemPos;          // Current memory position
    uint32_t    ui32CurrentCount;       // Current record count
    uint16_t    ui16DivideCount;        // Count for the frequency divider
    // RAM buffer for this channel
    void*       pvRamBuf[2]; 
    /* uint16_t    ui16DebugMaxValIdx;     // Debug: Maximaler Buffer Index */
}tDATALOG_CHANNEL;

#define tDATALOG_CHANNEL_DEFAULTS {0, 0, 0, 0, 0, 0, 0, 0, 1, {NULL}}

// Datalog control structure
typedef struct
{
    tDATALOG_CONFIG         sDataLogConfig;
    uint8_t                 ui8ChannelsRunning;
    tDATALOG_CHANNEL        sDatalogChannels[MAX_NUM_LOGS];
}tDATALOG_CONTROL;

#define tDATALOG_CONTROL_DEFAULTS {tDATALOG_CONFIG_DEFAULTS, 0, {tDATALOG_CHANNEL_DEFAULTS}}

/************************************************************************************
 * Record memory mode header module
 ***********************************************************************************/
// Log description header (Leading information for each log)
typedef struct
{
    uint16_t ui16Index;             // Variable struct index
    uint16_t ui16Divider;           // Timebase frequency divider
    uint32_t ui32MemoryOffset;      // Offset address of the channel
}tDATALOG_CHANNEL_MEMORY;

#define tDATALOG_CHANNEL_MEMORY_DEFAULTS {0, 0, 0}

// Header f�r die Logvariablen-Instanzen auf dem externen Speichermedium
typedef struct
{
    /* uint8_t                     ui8ActiveLoggers; */
    uint32_t                    ui32TimeBase;
    uint32_t                    ui32LastAddress;
    tDATALOG_CHANNEL_MEMORY     sDatalogChannelsMemory[MAX_NUM_LOGS];
}tDATALOG_MEMORY_HEADER;

#define tDATALOG_MEMORY_HEADER_DEFAULTS {/*0,*/ 0, 0, {tDATALOG_CHANNEL_MEMORY_DEFAULTS}}

/************************************************************************************
 * Serializer control struct 
 ***********************************************************************************/
typedef struct
{
    // Steuervariablen f�r den Scheduler des internen Datenloggers
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

// Struktur f�r den Live-Datenlogger
typedef struct
{
    int8_t      i8SampleTimerIdx;
    int8_t      i8TimestampTimerIdx;
}tDATALOG_LIVEMODE_TIMER;

#define tDATALOG_LIVEMODE_TIMER_DEFAULTS {-1, -1}

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
    tDATALOG_CONTROL_DEFAULTS \
}

/************************************************************************************
 * Funktionsdeklarationen
 ***********************************************************************************/
// API functions
bool RegisterLog (tDATALOG_INITS sDatalogInits);
bool DatalogInitialize (tDATALOG_CONFIG sDatalogConfig);
bool DatalogStart (void);
bool DatalogStop (void);
// Datalog service methods
void DataloggerService (void);
void DataloggerStatemachine (void);
// Internal functions
static void DataloggerSetState (void);
static void DataloggerSetStateImmediate (tDATALOG_STATE eNewState);

/* extern bool LiveModeDatalogStart (uint8_t ui8Var_count, uint16_t* pui16Var_nr, uint16_t pui16Divider); */
/* extern bool LiveModeDatalogStop(void); */
/* void LiveModeSampleData (void); */
/* extern void LiveModeTimerInit (void); */
// void LiveModeCallback (void);
/* extern bool InitializeMemoryRead (void); */

#endif // INCLUDES_DATALOG_H_
// EOF datalog.h-------------------------------------------------------------------
