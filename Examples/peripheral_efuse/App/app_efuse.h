/**
 * @file app_efuse.h
 * @brief Header file exposing example functions for Efuse usage.
 * @details Contains app_efuse's function declarations so other modules can call.
 * @date 2024
 * @author Chipsbank
*/
//-------------------------------
// INCLUDE
//-------------------------------
#include "APP_common.h"
//-------------------------------
// DEFINE
//-------------------------------
//-------------------------------
// ENUM
//-------------------------------
//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void app_efuse_example_handler(uint32_t tcNum);
void app_efuse_enable_aes(void);
void app_efuse_check_aes_enabled(void);
void app_efuse_lock_aes_setting(void);
void app_efuse_write_aes_key(void);
void app_efuse_read_aes_key(void);
void app_efuse_lock_aes_key(void);
void app_efuse_read_chip_id(void);
void APP_EfuseSetUserConfig1Bits(void);
void APP_EfuseReadUserConfig1(void);
void APP_EfuseLockUserConfig1(void);
void app_efuse_set_user_config2_bits(void);
void app_efuse_read_user_config2(void);
void app_efuse_lock_user_config2(void);
void app_peripheral_efuse_demo_init(void);
void app_peripheral_efuse_demo_loop(void);

void app_uart_0_rxb_full_callback(void);

void app_timer_0_irq_callback (void);
//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
