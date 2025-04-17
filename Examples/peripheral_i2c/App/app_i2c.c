#include "app_i2c.h"
#include "CB_i2c.h"
#include "CB_dma.h"
#include "CB_iomux.h"
#include "CB_gpio.h"
#include "CB_scr.h"
#include "CB_system.h"
#include "AppSysIrqCallback.h"
#include "imu_42670.h"
#include <string.h>
#include "NonLIB_sharedUtils.h"


//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_CPU_I2C_UARTPRINT_ENABLE APP_TRUE

#if (APP_CPU_I2C_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_i2c_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_i2c_print(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
//#define USE_DMA_I2C       APP_TRUE
#define USE_DMA_I2C        APP_FALSE

#define I2C_SPEED_HZ				                   100000
#define DEF_I2C_APP_TIMEOUT_MS                 500
#define DEF_I2C_APP_TIMEOUT_CPU_CYCLES         (((SystemCoreClock) / 1000U) * DEF_I2C_APP_TIMEOUT_MS)

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static stI2CConfig imu_config = {0};
static volatile uint8_t i2c_txbuf[32] __attribute__((aligned(4))) = {0};  // Transmit buffer for I2C data
static volatile uint8_t i2c_rxbuf[32] __attribute__((aligned(4))) = {0};  // Receive buffer for I2C data
 
/* I2C Ready interrupt flag */
static volatile uint8_t i2c_done_flag = CB_FALSE;  // Flag to indicate I2C operation completion
static volatile uint8_t dma_done_flag = CB_FALSE;  // Flag to indicate DMA operation completion

static volatile uint8_t i2c_cnt = 0; // Counter for I2C operations
static volatile uint8_t dmacnt = 0;  // Counter for DMA operations
static volatile uint8_t done = 0;    // General completion flag
static volatile uint8_t test = 0;    // Test flag (unused in current context)

/* I2C DMA channel selection */
static enDMAChannel i2c_dma_channel = EN_DMA_CHANNEL_0;

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Application I2C ready interrupt callback function
 */
static void app_i2c_ready_irq_callback(void)
{
    i2c_done_flag = CB_TRUE;
}

/**
 * @brief DMA Channel 0 Interrupt Callback Function
 */
static void app_dma_channel_0_irq_callback(void)
{
    dma_done_flag = CB_TRUE;
}

/**
 * @brief Initialize I2C for application
 */
void app_i2c_init(void)
{
    stIomuxGpioMode i2c_iomux_config = {0};
    stGPIO_InitTypeDef i2c_pins;

    /* Manually configure I2C Pull-up */
    cb_scr_gpio_module_on();

    /* Pin 6 - SCK/SCL */
    i2c_pins.Pin = EN_GPIO_PIN_6;
    i2c_pins.Mode = EN_GPIO_MODE_OUTPUT;
    i2c_pins.Pull = EN_GPIO_PULLUP;
    cb_gpio_init(&i2c_pins);

    /* Pin 3 - SDA */
    i2c_pins.Pin = EN_GPIO_PIN_3;
    // i2c_pins.Mode = EN_GPIO_MODE_OUTPUT;
    i2c_pins.Pull = EN_GPIO_PULLUP;
    cb_gpio_init(&i2c_pins);

    /* Pull-up CS Pin to choose I2C mode */
    i2c_pins.Pin = EN_GPIO_PIN_2;
    i2c_pins.Mode = EN_GPIO_MODE_OUTPUT;
    i2c_pins.Pull = EN_GPIO_NOPULL;
    cb_gpio_init(&i2c_pins);
    cb_gpio_write_pin(EN_GPIO_PIN_2, EN_GPIO_PIN_SET);

    /* Pull-down AP_AD0 Pin to choose i2c address 0x68 */
    i2c_pins.Pin = EN_GPIO_PIN_7;
    i2c_pins.Mode = EN_GPIO_MODE_OUTPUT;
    i2c_pins.Pull = EN_GPIO_NOPULL;
    cb_gpio_init(&i2c_pins);
    cb_gpio_write_pin(EN_GPIO_PIN_2, EN_GPIO_PIN_RESET);

    /* Configure SCL - Pin 6 & SDA - Pin 7 Pins */
    i2c_iomux_config.IomuxGpioMode = EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS;
    i2c_iomux_config.SourceSel		 = (uint8_t) EN_IOMUX_GPIO_AF_I2C_SCK;
    cb_iomux_config(EN_IOMUX_GPIO_6, &i2c_iomux_config);
    i2c_iomux_config.SourceSel     = (uint8_t) EN_IOMUX_GPIO_AF_I2C_SDA;
    cb_iomux_config(EN_IOMUX_GPIO_3, &i2c_iomux_config);

    imu_config.I2CSpeed = I2C_SPEED_HZ;
  
  /* Init I2C module with DMA */
#if (USE_DMA_I2C == APP_TRUE)
    imu_config.I2CInt = EN_I2C_INT_READY;    /* I2C Done interrupt */

    /* Register APP-level I2C interrupt */
    app_irq_deregister_irqcallback(EN_IRQENTRY_I2C_APP_IRQ, app_i2c_ready_irq_callback);
    app_irq_deregister_irqcallback(EN_IRQENTRY_DMA_CHANNEL_0_APP_IRQ, app_dma_channel_0_irq_callback);
#endif

    /* Init i2c and DMA module */
    cb_i2c_init(&imu_config);
    cb_dma_init();
}

/**
 * @brief Reads data from a specified I2C device register.
 *
 * This function reads a specified number of bytes from a given I2C device's register. 
 * It supports both DMA and non-DMA modes based on the `USE_DMA_I2C` configuration.
 *
 * @param address The I2C address of the target device.
 * @param reg The register address within the target device.
 * @param buf Pointer to the buffer where the read data will be stored.
 * @param size Number of bytes to read.
 * @return CB_STATUS Returns `CB_SUCCESS` if the operation is successful, otherwise returns `CB_FAIL`.
 */
CB_STATUS app_i2c_read(uint8_t address, uint8_t reg, uint8_t *buf, uint8_t size)
{
    CB_STATUS status = CB_FAIL;

#if (USE_DMA_I2C == CB_TRUE)
    i2c_done_flag = CB_FALSE;
    dma_done_flag = CB_FALSE;

    while (cb_i2c_is_ready() != CB_TRUE);

    status = cb_i2c_master_read_from_register_dma(i2c_dma_channel, address, reg, (uint32_t) buf, (uint8_t)size);

    /* Timeout waiting for interrupt flag set */
    uint32_t start_cpu_cycle = DWT->CYCCNT;
    uint32_t ellapsed_cpu_cycles = 0;

    do {
        if (dma_done_flag == CB_TRUE && cb_i2c_is_ready() == CB_TRUE)
        {
          break;
        }
      
         ellapsed_cpu_cycles = (DWT->CYCCNT < start_cpu_cycle) ? 
         (0xFFFFFFFF - start_cpu_cycle + DWT->CYCCNT + 1) : DWT->CYCCNT - start_cpu_cycle;
    } while ((dma_done_flag != CB_TRUE) && cb_i2c_is_ready() != CB_TRUE); // Wait I2C ready.

    if (ellapsed_cpu_cycles >= DEF_I2C_APP_TIMEOUT_CPU_CYCLES)
    {
      app_i2c_print("\nI2C WRITE Timeout!\n");
      return CB_FAIL;
    }

    /* Check if NACK received */
    if (cb_i2c_is_nack() == CB_TRUE)
    {
      app_i2c_print("\nI2C NACK received!\n");
      return CB_FAIL;
    }
#else
    cb_i2c_master_read_from_register(address, reg, buf, size);
#endif
  
    return status;
}

/**
 * @brief Write data to a specified register of an I2C device.
 * 
 * This function writes a block of data to a specified register of an I2C device. It supports both DMA and non-DMA modes.
 * In DMA mode, the function waits for the DMA transfer to complete within a specified timeout period and checks for any NACK.
 * In non-DMA mode, it directly calls the write function.
 * 
 * @param address The I2C device address.
 * @param reg The register address to write data to.
 * @param buf Pointer to the data buffer.
 * @param size Number of bytes to write.
 * @return CB_STATUS The operation status, indicating success or failure.
 */
CB_STATUS app_i2c_write(uint8_t address, uint8_t reg, uint8_t *buf, uint8_t size)
{
	  CB_STATUS status = CB_FAIL;

#if (USE_DMA_I2C == CB_TRUE)
    i2c_done_flag = CB_FALSE;
    dma_done_flag = CB_FALSE;

    while (cb_i2c_is_ready() != CB_TRUE);

    status = cb_i2c_master_write_to_register_dma(i2c_dma_channel, address, reg, (uint32_t) buf, (uint8_t)size);

    /* Timeout waiting for interrupt flag set */
    uint32_t start_cpu_cycle = DWT->CYCCNT;
    uint32_t ellapsed_cpu_cycles = 0;

    do {    
        if (dma_done_flag == CB_TRUE && cb_i2c_is_ready() == CB_TRUE)
        {
          break;
        }
        ellapsed_cpu_cycles = (DWT->CYCCNT < start_cpu_cycle) ? 
        (0xFFFFFFFF - start_cpu_cycle + DWT->CYCCNT + 1) : DWT->CYCCNT - start_cpu_cycle;
    } while (ellapsed_cpu_cycles < DEF_I2C_APP_TIMEOUT_CPU_CYCLES); // Wait I2C ready.


    if (ellapsed_cpu_cycles >= DEF_I2C_APP_TIMEOUT_CPU_CYCLES)
    {
      app_i2c_print("\nI2C WRITE Timeout!\n");
      return CB_FAIL;
    }

    /* Check if NACK received */
    if (cb_i2c_is_nack() == CB_TRUE)
    {
      app_i2c_print("\nI2C NACK received!\n");
      return CB_FAIL;
    }
#else
	    status = cb_i2c_master_write_to_register(address, reg, buf, size);
#endif

    return status;
}

/**
 * @brief Main function to initialize and start the I2C peripheral.
 * 
 * This function initializes the UART and timer modules, starts the timer, 
 * prints "Power on" via UART, and initializes the IMU 42670 registers.
 */
void app_peripheral_i2c_init(void)
{
    app_uart_init();                            //Initializing serial port
    app_i2c_print("Power on \n");
    cb_hal_delay_in_ms(1000);                // Delay for 1 second
    iic_imu_42670_reg_init();                  // Initialize the IMU registers

}

/**
 * @brief Loop function for I2C peripheral operations.
 * 
 * This function checks the I2C flag and retrieves gyroscope and accelerometer 
 * data from the IMU if the flag is set.
 */
void app_peripheral_i2c_loop(void)
{
    iic_imu_get_gyro_accel();              // Retrieve gyroscope and accelerometer data
    cb_hal_delay_in_ms(1000);                // Delay for 1 second
}


