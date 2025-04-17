#ifndef _IMU_42670_H_
#define _IMU_42670_H_

#define PWR_REG              0x1F
#define ACEEL_CONFIG0_REG    0x21
#define GYRO_CONFIG0_REG     0x20
#define GYRO_CONFIG1_REG     0x23
#define ACEEL_CONFIG1_REG    0x24
#define ACCEL_DATA_REG       0x0b
#define chipid_REG           0x75

#define SLAVE_ADDR 0x69  //AD0 CONNECT TO VCC

void iic_imu_42670_reg_init(void);

void iic_imu_get_gyro_accel(void);


#endif

