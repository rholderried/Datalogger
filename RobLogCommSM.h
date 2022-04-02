/********************************************************************************//**
 * \file RobLogCommSM.h
 * \author Roman Holderried
 *
 * \brief Typedefs, function declarations for the RobLog Communication Submodule
 *
 * <b> History </b>
 *      - 2021-05-01 - File creation.
 *                     
 ***********************************************************************************/

#ifndef ROBLOGCOMMSM_H__
#define ROBLOGCOMMSM_H__

/************************************************************************************
 * Includes
 ***********************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "../common/CommonTypes.h"

/************************************************************************************
 * Defines
 ***********************************************************************************/
#define ROBLOG_COMM_SM_NUMBER 1
/************************************************************************************
 * Type definitions
 ***********************************************************************************/
/*
typedef enum
{
    eROBLOG_ERROR_NONE = 0,
    // RobLog internal Errors
    eROBLOG_ERROR_DATALOG =     1,
    eROBLOG_ERROR_ROBLOGIF =    2
}tROBLOG_ERROR;*/

/************************************************************************************
 * Function declarations
 ***********************************************************************************/
bool RobLogSetCommands(tMESSAGE sMessage);
bool RobLogGetCommands(tMESSAGE sMessage);

#endif // ROBLOGCOMMSM_H__
// EOF
