/********************************************************************************//**
 * \file DataloggerSCI.h
 * \author Roman Holderried
 *
 * \brief Typedefs, function declarations for the SCI interface.
 *
 * <b> History </b>
 *      - 2022-04-27 - File creation.
 *                     
 ***********************************************************************************/
#ifndef DATALOGGERSCI_H_
#define DATALOGGERSCI_H_

#define SCI
#ifdef SCI
/************************************************************************************
 * Includes
 ***********************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "SCI.h"

/************************************************************************************
 * Defines
 ***********************************************************************************/
#ifndef VALUE_MODE_HEX
#error "The Datalogger SCI interface only supports VALUE_MODE_HEX at the moment"
#endif

#define SIZE_OF_RETURN_VAL_BUFFER   5

/************************************************************************************
 * Function declarations
 ***********************************************************************************/

/********************************************************************************//**
 * \brief Registers a log channel.
 * 
 * Callback of type COMMAND_CB (Refer to the SCI command structure definition)
 ***********************************************************************************/
COMMAND_CB_STATUS RegisterLogFromVarStruct (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo);

/********************************************************************************//**
 * \brief Initializes the control structure of the data logger.
 * 
 * Callback of type COMMAND_CB (Refer to the SCI command structure definition)
 ***********************************************************************************/
COMMAND_CB_STATUS InitializeDatalogger (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo);

/********************************************************************************//**
 * \brief Starts the data logger.
 * 
 * Callback of type COMMAND_CB (Refer to the SCI command structure definition)
 ***********************************************************************************/
COMMAND_CB_STATUS StartDatalogger (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo);

/********************************************************************************//**
 * \brief Stops the data logger.
 * 
 * Callback of type COMMAND_CB (Refer to the SCI command structure definition)
 ***********************************************************************************/
COMMAND_CB_STATUS StopDatalogger (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo);

/********************************************************************************//**
 * \brief Initializes the upstream for transferring the log data.
 * 
 * Callback of type COMMAND_CB (Refer to the SCI command structure definition)
 ***********************************************************************************/
COMMAND_CB_STATUS GetLogData (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo);

/********************************************************************************//**
 * \brief Returns the channel data.
 * 
 * Callback of type COMMAND_CB (Refer to the SCI command structure definition)
 ***********************************************************************************/
COMMAND_CB_STATUS GetChannelInfo (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo);

/********************************************************************************//**
 * \brief Resets the Datalogger.
 * 
 * Callback of type COMMAND_CB (Refer to the SCI command structure definition)
 ***********************************************************************************/
COMMAND_CB_STATUS ResetDatalogger (uint32_t* ui32ValArray, uint8_t ui8ValArrayLen, PROCESS_INFO *pInfo)

#endif
#endif //DATALOGGERSCI_H_
// EOF
