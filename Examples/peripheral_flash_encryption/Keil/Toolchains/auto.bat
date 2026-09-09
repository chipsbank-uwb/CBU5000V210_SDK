Toolchains\srec_cat.exe .\Objects\CBU5000V210.hex -intel -offset -0x00000000 -o .\Objects\CBU5000V210.bin -binary
copy /b .\Toolchains\UserConfig_PUYA_P25Q40SL.bin+.\Objects\CBU5000V210.bin .\Objects\CBU5000V210_unencrypted.bin



