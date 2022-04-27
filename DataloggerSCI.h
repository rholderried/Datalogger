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

/************************************************************************************
 * Type definitions
 ***********************************************************************************/

#endif
#endif //DATALOGGERSCI_H_
// EOF
