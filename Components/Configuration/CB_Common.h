#ifndef __SDK_COMMON_H
#define __SDK_COMMON_H
#include <stdint.h>
#include "ARMCM33_DSP_FP.h"

#define CB_TRUE   1
#define CB_FALSE  0

#define DRIVER_SET   1
#define DRIVER_CLR   0

#define   __I     volatile const       /*!< Defines 'read only' permissions */
#define   __O     volatile             /*!< Defines 'write only' permissions */
#define   __IO    volatile             /*!< Defines 'read / write' permissions */

#ifndef NULL
#define NULL 0
#endif

typedef enum
{
  CB_PASS   = 1,
  CB_FAIL   = 0,
} CB_STATUS;

#endif /*__SDK_COMMON_H*/
