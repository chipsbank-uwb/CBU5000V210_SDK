    
    AREA    LUT_BINARY_DATA, DATA, READONLY, ALIGN=2

    EXPORT  lut_binary_data_start
    EXPORT  lut_binary_data_end

lut_binary_data_start
        INCBIN  lut_default.bin
lut_binary_data_end

    END
