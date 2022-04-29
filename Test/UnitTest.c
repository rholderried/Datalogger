#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "Datalogger.h"

int main(void)
{
    // tDATALOGGER inst = tDATALOGGER_DEFAULTS;
    tDATALOG_ERROR err = eDATALOG_ERROR_NONE;
    tDATALOGGER *p_inst = DatalogGetData();
    uint8_t ui8LogVar = 0;
    uint16_t ui16LogVar2 = 1;

    cfg.ui8ActiveLoggers = ((1 << 0) | (1 << 1));
    cfg.ui32TimeBase = 2;
    cfg.eOpMode = eOPMODE_RECMODERAM;

    err = RegisterLog(1, 1, 3, &ui8LogVar, 1);
    err = RegisterLog(2, 1, 2, (uint8_t*)&ui16LogVar2, 2);
    err = DatalogInitialize(cfg);

    err = DatalogStart();
    while (ui8LogVar < 20)
    {
        DatalogService();
        ui8LogVar++;
        ui16LogVar2++;
    }

    free(p_inst->sDatalogControl.pui8Data);
}