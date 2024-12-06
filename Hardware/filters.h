#ifndef __FILTERS_H__
#define __FILTERS_H__

#include <stdint.h>

void Kalman_Filter_Init(void);


float DC_Filter(uint16_t *pValue);
float AC_Cal_Filter(void);
float R_Filter(uint16_t *pValue);

#endif
