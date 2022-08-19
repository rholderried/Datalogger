/********************************************************************************//**
 * \file DataloggerCfg.h
 * \author Roman Holderried
 *
 * \brief Configuration file template for the Datalogger module.
 *
 * <b> History </b>
 *      - 2022-08-19: File creation.
 *                     
 ***********************************************************************************/

#ifndef _DATALOGGERCFG_H_
#define _DATALOGGERCFG_H_

/******************************************************************************
 * Configuration Macros
 *****************************************************************************/
/** Maximum buffer size the datalogger can write */
#define DATALOGGER_MAX_BUFFER_SIZE 2048
/** Returned error indicators will be offset by this value*/
#define DATALOGGER_SCI_ERROR_OFFSET 10


#endif // _DATALOGGERCFG_H_
