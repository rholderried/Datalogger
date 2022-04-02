/********************************************************************************//**
 * \file RobLogCommSM.c
 * \author Roman Holderried
 *
 * \brief Routines for the RobLib communications interface (RobCom).
 *
 * <b> History </b>
 *      - 2021-05-01 - File creation. 
 *                     
 ***********************************************************************************/

/************************************************************************************
 * Includes
 ***********************************************************************************/
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include "../RobCom/RobCom.h"
#include "RobLogCommSM.h"
#include "../common/varAccess.h"

extern tRAMVAR ramVars [];
extern const uint8_t ui8Byte_count [];
#ifdef APP_USES_EEPROM
extern tEEVAR eepromVarseepromVars;
#endif

/************************************************************************************
 * State variables
 ***********************************************************************************/
/* static tROBCOM sRobCom = tROBCOM_DEFAULTS; */

/************************************************************************************
 * Function definitions
 ***********************************************************************************/
