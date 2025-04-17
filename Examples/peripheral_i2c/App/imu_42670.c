//#include "app_spimaster.h"
#include "app_uart.h"
#include "CB_system.h"
#include "CB_i2c.h"
#include "CB_scr.h"
#include "imu_42670.h"
#include "CB_iomux.h"
#include "CB_gpio.h"


/**
 * @brief I2C configuration structure.
 *
 * This structure defines the configuration for the I2C module, including interrupt settings and speed.
 */
static stI2CConfig i2c_config = {
    .I2CInt = EN_I2C_INT_DISABLE,
    .I2CSpeed = 100000,
};

/**
 * @brief Initializes the GPIO pins for I2C communication.
 *
 * This function configures the GPIO pins for I2C communication, including setting pull-up resistors
 * and multiplexing the pins for I2C functionality.
 */
static void i2c_gpio_init(void)
{
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
    i2c_pins.Pull = EN_GPIO_PULLUP;
    cb_gpio_init(&i2c_pins);

    /* Configure IOMUX for I2C functionality */
    cb_iomux_config(EN_IOMUX_GPIO_6, &(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS, (uint8_t)EN_IOMUX_GPIO_AF_I2C_SCK});
    cb_iomux_config(EN_IOMUX_GPIO_3, &(stIomuxGpioMode){EN_IOMUX_GPIO_MODE_SOC_PERIPHERALS, (uint8_t)EN_IOMUX_GPIO_AF_I2C_SDA});

    /* Initialize I2C module */
    cb_i2c_init(&i2c_config); // Ensure IMU is in I2C mode
}


/**
 * @brief Initialize the IMU 42670 registers.
 * 
 * This function initializes the I2C GPIO, reads the chip ID, and configures the IMU registers:
 * - Sets the gyroscope to 2000 dps, 100 Hz
 * - Sets the accelerometer to ±8g, 100 Hz
 * - Sets the gyroscope filter to 53 Hz
 * - Sets the accelerometer filter to 53 Hz
 * - Enables the gyroscope and accelerometer
 * - Reads and prints the power status register
 */
void iic_imu_42670_reg_init(void)
{
    uint8_t data;
    i2c_gpio_init(); // Initialize I2C GPIO
    cb_i2c_master_read_from_register(SLAVE_ADDR, chipid_REG, &data, 1); // Read chip ID
    app_uart_printf("id reg: %x \r\n", data);
    
    data = 0x06;
    cb_i2c_master_write_to_register(SLAVE_ADDR, GYRO_CONFIG0_REG, &data, 1); // Gyro 2000 dps, 100 Hz
    data = 0x26;
    cb_i2c_master_write_to_register(SLAVE_ADDR, ACEEL_CONFIG0_REG, &data, 1); // Acc ±8g, 100 Hz
    data = 0x04;
    cb_i2c_master_write_to_register(SLAVE_ADDR, GYRO_CONFIG1_REG, &data, 1); // Gyro filter is 53 Hz
    data = 0x44;
    cb_i2c_master_write_to_register(SLAVE_ADDR, ACEEL_CONFIG1_REG, &data, 1); // Acc filter is 53 Hz
    data = 0x0f;
    cb_i2c_master_write_to_register(SLAVE_ADDR, PWR_REG, &data, 1); // Enable gyro and accel
    cb_i2c_master_read_from_register(SLAVE_ADDR, PWR_REG, &data, 1);
    app_uart_printf("power sta: %x\r\n", data);
}

/**
 * @brief Read and print gyroscope and accelerometer data from the IMU 42670.
 * 
 * This function reads 12 bytes of data from the accelerometer and gyroscope registers,
 * processes the data into x, y, z components for both sensors, and prints the results via UART.
 */
void iic_imu_get_gyro_accel(void)
{
    uint8_t read_data[12];
    short gyro_data[3], accel_data[3]; // x, y, z

    cb_i2c_master_read_from_register(SLAVE_ADDR, ACCEL_DATA_REG, read_data, 12);

    accel_data[0] = (short)(read_data[0] << 8) | read_data[1];
    accel_data[1] = (short)(read_data[2] << 8) | read_data[3];
    accel_data[2] = (short)(read_data[4] << 8) | read_data[5];

    gyro_data[0] = (short)(read_data[6] << 8) | read_data[7];
    gyro_data[1] = (short)(read_data[8] << 8) | read_data[9];
    gyro_data[2] = (short)(read_data[10] << 8) | read_data[11];

    app_uart_printf("accel: %d %d %d   gyro: %d %d %d\r\n", accel_data[0], accel_data[1], accel_data[2], gyro_data[0], gyro_data[1], gyro_data[2]);
}























