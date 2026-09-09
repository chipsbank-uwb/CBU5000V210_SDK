    
#include "../Configuration/CB_CompileOption.h"

    AREA    LUT_BINARY_DATA, DATA, READONLY, ALIGN=2

    EXPORT  lut_binary_data_start
    EXPORT  lut_binary_data_end

lut_binary_data_start

        #if (GC_DEMO_BOARD_TYPE == GC_USE_CBD5213)
                INCBIN ../../../Components/Lut/lut_CBD5213.bin
        #else
                INCBIN ../../../Components/Lut/lut_CBD5214.bin
        #endif

lut_binary_data_end

    END
