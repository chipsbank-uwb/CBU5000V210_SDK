/**
 * @file app_efuse.c
 * @brief Implementation file for example usages of the efuse driver.
 * @details Contains many example usages for the efuse. When called in SysUartCommander,
 *          pass in an argument to run a selected example.aka:
 *          app_efuse_example_handler(args[0]) args meaning the argument that is passed on the terminal
 * @author Chipsbank
 * @date 2024
 */
//-------------------------------
// INCLUDE
//-------------------------------
#include "app_efuse.h"
#include "app_timer.h"
#include "CB_efuse.h"
#include <stdlib.h>

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_CPU_EFUSE_UARTPRINT_ENABLE APP_TRUE

#if (APP_CPU_EFUSE_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_efuse_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_efuse_print(...)
#endif

//-------------------------------
// DEFINE
//-------------------------------
//-------------------------------
// ENUM
//-------------------------------
//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct {
  uint8_t* rx_buffer;             // Receive buffer
  uint8_t  rx_flag;               // Receive complete flag
  uint16_t rx_length;             // Receive length
  uint8_t  rx_flag_done;          // Receive done flag  
}uart_t;

static uart_t uart_efuse;

//-------------------------------
// GLOBAL VARIABLE SECTION
//------------------------------
static void (*s_example_functions[])(void) = {
  app_efuse_check_aes_enabled,
  app_efuse_read_aes_key,
  app_efuse_read_user_config2,
  app_efuse_read_chip_id,
  app_efuse_enable_aes, 
  app_efuse_write_aes_key, 
  app_efuse_set_user_config2_bits,
  app_efuse_lock_aes_setting,
  app_efuse_lock_aes_key,
  app_efuse_lock_user_config2,
};
static uint32_t s_num_examples = sizeof(s_example_functions) / sizeof(s_example_functions[0]);

// Counter used for loop iterations in various functions
static uint8_t efuse_count = 0;
//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Call the example functions according to input from UART
 * @param tcnum The test case number, received through UART.
 */
void app_efuse_example_handler(uint32_t tcnum) 
{
    app_efuse_print("[AppEfuse] Received request to run tc: %d\n", tcnum);

    if (tcnum > s_num_examples-1) 
    {
      app_efuse_print("[AppEfuse] Specified number %d (+1) > supported number of examples %d",tcnum, s_num_examples);
      return;
    }

    app_efuse_print("[AppEfuse] By default, tc are numbered as:\n");
    app_efuse_print("[AppEfuse]    0: (SAFE)          Check if AES mode is enabled\n");
    app_efuse_print("[AppEfuse]    1: (SAFE)          Read AES key\n");
    app_efuse_print("[AppEfuse]    2: (SAFE)          Read User Config 2\n");
    app_efuse_print("[AppEfuse]    3: (SAFE)          Read Chip ID\n");
    app_efuse_print("[AppEfuse]    4: ***DANGEROUS*** Enable AES mode\n");
    app_efuse_print("[AppEfuse]    5: ***DANGEROUS*** Burns AES key into eFuse\n");
    app_efuse_print("[AppEfuse]    6: ***DANGEROUS*** Set User Config 2\n");
    app_efuse_print("[AppEfuse]    7: ***DANGEROUS*** Lock AES Settings\n");
    app_efuse_print("[AppEfuse]    8: ***DANGEROUS*** Lock AES key from R/W\n");
    app_efuse_print("[AppEfuse]    9: ***DANGEROUS*** Lock User Config 2\n\n");

    s_example_functions[tcnum]();
}

/**
 * @brief: Check if aes encryption mode is enabled.
 * @details: Note, once the AesSettings are locked, this result is inaccurate.
 * @see app_efuse_lock_aes_setting()
 */
void app_efuse_check_aes_enabled(void) 
{
    uint8_t is_aes_enable = cb_efuse_qspi_flash_encryption_is_enabled();

    app_efuse_print("[AppEfuse] Aes is enabled: %d - ", is_aes_enable);

    if (is_aes_enable == APP_TRUE) 
    {
      app_efuse_print("TRUE\n");
    } 
    else 
    {
      app_efuse_print("FALSE\n");
    }
}

/**
 * @brief: Enable AES encryption mode.
 * @details: Note, once enabled, AES encryption mode cannot be turned off.
 * Use the AES Check API to reveal if aes is already enabled.
 * @see app_efuse_check_aes_enabled() for example.
 */
void app_efuse_enable_aes(void) 
{
    app_efuse_print("[AppEfuse] Enabling Aes Mode\n");
    cb_efuse_qspi_flash_encryption_enable();
}

/**
 * @brief: Lock the AES encryption setting.
 * @details: Note, once locked, AES encryption mode cannot be further
 * overwritten or read from. This also means that checking API and Enabling API
 * will not work as expected.
 * @see app_efuse_check_aes_enabled()
 * @see app_efuse_enable_aes 
 * for usage of the 2 API mentioned.
 */
void app_efuse_lock_aes_setting(void) 
{
    app_efuse_print("[AppEfuse] Locking Aes Settings (no more Read/Write)\n");
    cb_efuse_qspi_flash_encryption_lock();
}

/**
 * @brief: Write 128-bit AES Key API example usage.
 * @details: Be careful, once ran, this function forever burns the example
 * AES key into the eFuse. You will not be able to clear bits that are already
 * set, so it is recommended to change the aeskey array in the code to the real 
 * desired AES key. This function won't work if they AES key is already locked.
 * @see app_efuse_read_aes_key() for API usage to read back the key.
 * @see app_efuse_lock_aes_key() for API usage to lock the AES key.
 */
void app_efuse_write_aes_key(void) 
{
    uint32_t aeskey[4] = {0x11110211,0x88888888,0x18120501,0x77777777};

    app_efuse_print("[AppEfuse] Burning AES Key: \n\t");

    for (int efuse_count = 0; efuse_count < 4; ++efuse_count) 
    {
      app_efuse_print("0x%x ", aeskey[efuse_count]);
    }
    app_efuse_print("\n");

    cb_efuse_qspi_flash_encryption_key_write(aeskey);
}

/**
 * @brief: Read 128-bit AES Key API example usage.
 * @details: This function reads back the aes key burned into the chip,
 * if nothing has been burned yet, the return values should all be 0. Read
 * API will also returns all 0 if the AES key is locked.
 * @see app_efuse_write_aes_key() for API usage to burn the keys.
 * @see app_efuse_lock_aes_key() for API usage to forbid further read/write of AES keys.
 */
void app_efuse_read_aes_key(void) 
{
    uint32_t aes_out[4];
    cb_efuse_qspi_flash_encryption_key_read(aes_out);

    app_efuse_print("[AppEfuse] Extracted AES Key: \n\t");

    for (int efuse_count = 0; efuse_count < 4; ++efuse_count) 
    {
      app_efuse_print("0x%x ", aes_out[efuse_count]);
    }
    app_efuse_print("\n");
}

/**
 * @brief: Lock AES key read/write API example usage.
 * @details: Once locked, you will not be able to access the AES key
 * for both writing and reading. Be VERY CAREFUL.
 * @see app_efuse_write_aes_key() for burning key API example.
 * @see app_efuse_read_aes_key() for reading key API example.
 */
void app_efuse_lock_aes_key(void) 
{
    app_efuse_print("[AppEfuse] Locking Aes Key (no further Read/Write)\n");
    cb_efuse_qspi_flash_encryption_key_lock();
}

/**
 * @brief Set bits in user config 2 example API usage.
 * @details You can burn your own vendor information or whatever values
 * you desire into a reserved UserConfig1 provided with the eFuse. Once
 * set, you will not be able to clear them, so BE CAREFUL before running
 * this example. If locked, this example will not work.
 * @see app_efuse_read_user_config2() for reading back the user config 2.
 * @see app_efuse_lock_user_config2() for locking user config 2.
 */
void app_efuse_set_user_config2_bits(void) 
{
#define NUM_BITS_TO_SET 16u

    // Define the positions of the bits to set
    uint8_t bitpositions[NUM_BITS_TO_SET] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31};

    // Print the bits being set
    app_efuse_print("[AppEfuse] Setting Bits:\n\t");
    for (uint32_t efuse_count = 0; efuse_count < NUM_BITS_TO_SET; ++efuse_count) 
    {
        app_efuse_print("%d ", bitpositions[efuse_count]);
    }
    app_efuse_print("\n[AppEfuse] in User Config 2\n");

    // Set the specified bits in User Config 2
    cb_efuse_userconfig2_set_bits(NUM_BITS_TO_SET, bitpositions);

#undef NUM_BITS_TO_SET
}

/**
 * @brief Read user config 2 example API usage.
 * @details Read the current user config 2. If user config 2
 * is locked, the read API will return all 0.
 * @see app_efuse_set_user_config2_bits() for writing user config 2.
 * @see app_efuse_lock_user_config2() for locking user config 2.
 */
void app_efuse_read_user_config2(void) 
{
    uint32_t user_comfig2 = cb_efuse_userconfig2_read();
    app_efuse_print("[AppEfuse] User Config 2: 0x%x\n", user_comfig2);
}

/**
 * @brief Lock Read/Write access to user config 2 example API usage.
 * @details Once locked, no further read/write will be valid. BE CAREFUL.
 * @see app_efuse_set_user_config2_bits() for writing user config 2.
 * @see app_efuse_read_user_config2() for reading back the user config 2.
 */
void app_efuse_lock_user_config2(void) 
{
    app_efuse_print("[AppEfuse] Locking User Config 2 (no further Read/Write\n");
    cb_efuse_userconfig2_lock();
}

/**
 * @brief Read the CHIP ID.
 * @details This example also shows which bits in the return value
 * represents what information of the chip id.
 */
void app_efuse_read_chip_id(void) 
{
    uint64_t chipId;
    chipId = cb_efuse_read_chip_id();
    app_efuse_print("[AppEfuse] ChipId:    %llx\n", chipId);
}

/**
 * @brief Main function for demonstrating EFuse and peripheral operations.
 *
 * This function initializes the UART, Timer, and performs an EFuse example operation.
 * It demonstrates how to configure peripherals and interact with EFuse after a power-on reset.
 */
void app_peripheral_efuse_demo_init(void)
{
    app_uart_init();                            // Initialize UART: GPIO00-RX, GPIO01-TX, BPR=115200, Stop bit = 1, BitOrder = LSB, Parity = NONE
    app_timer_main();                           // Configure Timer parameters and start the Timer
    app_efuse_print("\n\nPower-on reset\n");    // Power-on indication

    // Initializes the uart_efuse struct
    uart_efuse.rx_buffer = (uint8_t*)malloc(20 * sizeof(uint8_t)); // Allocate 20 bytes of space

    if (uart_efuse.rx_buffer == NULL) 
    {
        app_efuse_print("[AppEfuse] Failed to allocate memory for rx_buffer\n");
        return;
    }
    uart_efuse.rx_flag = 0;
    uart_efuse.rx_length = 0;
    uart_efuse.rx_flag_done = 0;

    // Perform EFuse example operation with default parameter
    app_efuse_example_handler(0);
    app_efuse_print("\nPlease Send CCCC+command(0x00-0x0c)\n\n");    // Indicate UART data reception
}

/**
 * @brief Main loop function for handling UART received data and EFuse operations.
 *
 * This function checks if the receive flag is set, indicating that data has been received via UART.
 * If valid data is detected (starting with 0xCC 0xCC), it calls the EFuse example handler.
 * Otherwise, it sends the received data back in hexadecimal format.
 * After processing, it clears the receive buffer and resets the length counter.
 */
void app_peripheral_efuse_demo_loop(void)
{
    // Check if the receive operation is complete
    if (uart_efuse.rx_flag_done == 1)  
    {
      uart_efuse.rx_flag = 0;                                 
      uart_efuse.rx_flag_done = 0; 
      app_efuse_print("Uart Receive data\n");    // Indicate UART data reception

      // Validate the received data header
      if (uart_efuse.rx_buffer[0] == 0xCC && uart_efuse.rx_buffer[1] == 0xCC)
      {
          // Call EFuse example handler with the third byte of received data
          app_efuse_example_handler(uart_efuse.rx_buffer[2]);
      }
      else
      {
          // Send received data back in hexadecimal format
          for (efuse_count = 0; efuse_count < uart_efuse.rx_length; efuse_count++)
          {
              app_efuse_print("%d ", uart_efuse.rx_buffer[efuse_count]);
          }
      }

      app_efuse_print("\n\nPlease Send CCCC+command(0x00-0x0c)\n\n");    // Indicate UART data reception

      // Clear the receive buffer and reset length counter
      for (efuse_count = 0; efuse_count < uart_efuse.rx_length; efuse_count++)
      {
          uart_efuse.rx_buffer[efuse_count] = 0;
      }    

      uart_efuse.rx_length = 0;    
    }
}

/**
 * @brief UART 0 RX buffer full callback function.
 */
void app_uart_0_rxb_full_callback(void)
{
    uint8_t temp = 0;
    cb_uart_get_rx_buffer(EN_UART_0, &temp, 1); 
    uart_efuse.rx_buffer[uart_efuse.rx_length++] = temp;
    uart_efuse.rx_flag = 1;
}

/**
 * @brief Timer 0 interrupt callback function.
 */
void app_timer_0_irq_callback(void)
{
    if (uart_efuse.rx_flag == 1)
    {
        if (uart_efuse.rx_flag++ >= 1)
        {
            uart_efuse.rx_flag_done = 1;
        }
    }
}



