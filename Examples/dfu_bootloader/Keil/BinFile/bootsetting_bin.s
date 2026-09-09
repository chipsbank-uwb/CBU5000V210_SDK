        AREA    BOOTSETTING_ROM, DATA, READONLY

        EXPORT  bootsettingBin

bootsettingBin
        INCBIN  bootsetting.bin

bootsettingBin_End

        END