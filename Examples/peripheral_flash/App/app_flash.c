#include "app_flash.h"
#include "app_uart.h"
#include "CB_flash.h"

/**
 * @brief  Flash test configuration
 */
#define FLASH_TEST_ADDR             0x00040000UL
#define FLASH_TEST_SECTOR           0x40
#define FLASH_TEST_LENGTH           0x100

/**
 * @brief  Test data array (0 ~ 255)
 */
uint8_t write_buf[FLASH_TEST_LENGTH] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

uint8_t read_buf[FLASH_TEST_LENGTH] = {0};

/**
 * @brief   Flash basic operation example for application layer
 * @details This example shows standard flash operations:
 *          Init -> GetInfo -> Erase -> Write -> Read -> Verify
 * @note    Only for demo, keep simple and clear.
 *
 */
void app_flash_test_process(void)
{
    enFlashStatus status = EN_FLASH_SUCCESS;
    uint16_t i = 0;

    // UART initialization
    app_uart_init();
    app_uart_printf("\n=============================================\n");
    app_uart_printf("        Flash App Layer Example Start        \n");
    app_uart_printf("=============================================\n");

    // ------------------------------
    // Step1: Flash initialization
    // ------------------------------
    status = cb_flash_init();
    if(status != EN_FLASH_SUCCESS)
    {
        app_uart_printf("[ERROR] Flash init failed\r\n");
        return;
    }
    app_uart_printf("[INFO] Flash init success\r\n");

    // ------------------------------
    // Step2: Read Flash information
    // ------------------------------
    enFlashVendorID vendor = cb_flash_get_vendor();
    enFlashCapacity cap = cb_flash_get_capacity();
    app_uart_printf("[INFO] Flash Vendor ID: 0x%02X\r\n", vendor);
    app_uart_printf("[INFO] Flash Capacity : 0x%02X\r\n", cap);

    // ------------------------------
    // Step3: Erase sector
    // ------------------------------
    status = cb_flash_erase_sector(FLASH_TEST_SECTOR);
    if(status != EN_FLASH_SUCCESS)
    {
        app_uart_printf("[ERROR] Erase sector failed\r\n");
        return;
    }
    app_uart_printf("[INFO] Erase sector success\r\n");

    // ------------------------------
    // Step4: Write data to flash
    // ------------------------------
    status = cb_flash_program_by_addr(FLASH_TEST_ADDR, write_buf, FLASH_TEST_LENGTH);
    if(status != EN_FLASH_SUCCESS)
    {
        app_uart_printf("[ERROR] Write flash failed\r\n");
        return;
    }
    app_uart_printf("[INFO] Write data success\r\n");

    // ------------------------------
    // Step5: Read data from flash
    // ------------------------------
    status = cb_flash_read_by_addr(FLASH_TEST_ADDR, read_buf, FLASH_TEST_LENGTH);
    if(status != EN_FLASH_SUCCESS)
    {
        app_uart_printf("[ERROR] Read flash failed\r\n");
        return;
    }
    app_uart_printf("[INFO] Read data success\r\n");

    // ------------------------------
    // Step6: Verify data
    // ------------------------------
    for(i = 0; i < FLASH_TEST_LENGTH; i++)
    {
        if(read_buf[i] != write_buf[i])
        {
            app_uart_printf("[ERROR] Verify fail at index %d\r\n", i);
            return;
        }
    }

    app_uart_printf("\r\n[INFO] All flash operations passed\r\n");
    app_uart_printf("=============================================\r\n\r\n");
}




