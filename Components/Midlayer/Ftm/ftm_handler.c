/**
 * @file    mp_handler.c
 * @brief   [SYSTEM] UART Commander Application Source File
 * @details This file contains the implementation of functions related to UART command handling.
 *          It defines functions for processing UART receive buffer, command handling, and associated callback functions.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "CB_efuse.h"
#include "CB_system.h"
#include "CB_SleepDeepSleep.h"
#include "ftm_uwb_cal.h"
#include "ftm_handler.h"
#include "CB_efuse.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------
//#define FTM_LOG_ENABLE  APP_TRUE
#if (FTM_LOG_ENABLE == APP_TRUE)
#define LOG(...) APP_UartPrintf(__VA_ARGS__)
#else
#define LOG(...)   
#endif

#define FACTORY_CMD_START_BYTE 0x9B
#define FACTORY_CMD_STOP_BYTE  0x5B


#define BigLittleSwap16(A) ((((uint16_t)(A) & 0xff00) >> 8 ) |\
                            (((uint16_t)(A) & 0x00ff) << 8 ))
#define BigLittleSwap32(A) ((((uint32_t)(A) & 0xff000000) >> 24) |\
                            (((uint32_t)(A) & 0x00ff0000) >> 8 ) |\
                            (((uint32_t)(A) & 0x0000ff00) << 8 ) |\
                            (((uint32_t)(A) & 0x000000ff) << 24))
//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef void (*ftFunction)(uint16_t, uint8_t*, uint8_t);

typedef struct {
    uint16_t command;
    ftFunction handler;
} ftCmd;


//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void ftm_on_0ff(uint16_t command, uint8_t *buf, uint8_t len);

void ftm_preamble_scan(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_preamble_set(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_preamble_get(uint16_t command, uint8_t *buf, uint8_t len);

void ftm_device_role_set(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_device_role_get(uint16_t command, uint8_t *buf, uint8_t len);

void ftm_read_chip_id(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_read_freq_offset_cal(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_write_freq_offset_cal(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_read_power_code(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_write_power_code(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_read_tof_cal(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_write_tof_cal(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_read_num_of_aoa(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_read_aoa_cal(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_write_aoa_cal(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_set_tx_num_of_packet(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_set_tx_interval(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_set_tx_on_off(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_set_rx_channel(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_set_rx_on_off(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_read_rx_num_of_packs(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_read_rang_aoa_id(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_write_rang_aoa_id(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_read_rang_aoa_freq(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_write_rang_aoa_freq(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_set_rang_aoa_tx_on_off(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_set_rang_aoa_rx_channel(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_set_rang_aoa_rx_on_off(uint16_t command, uint8_t *buf, uint8_t len);
void ftm_get_rang_aoa_rx_result(uint16_t command, uint8_t *buf, uint8_t len);

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static cmdhandler_t g_respond_func = NULL;
static uint8_t ft_active =APP_FALSE;
static ftCmd user_command_table[] = 
{
    //Greneral commmand
    {0x7525, ftm_on_0ff},

};


static const ftCmd ftm_command_table[] = 
{    
    // factory command
    {0x00F0, ftm_preamble_scan},
    {0x00F1, ftm_preamble_set},
    {0x00F2, ftm_preamble_get},
    {0x00F3, ftm_device_role_set},
    {0x00F4, ftm_device_role_get},
    
    {0x0001, ftm_read_chip_id},  
    {0x0010, ftm_read_freq_offset_cal},
    {0x0011, ftm_write_freq_offset_cal},
    {0x0012, ftm_read_power_code},  
    {0x0013, ftm_write_power_code},
    {0x0014, ftm_read_tof_cal}, 
    {0x0015, ftm_write_tof_cal},
    {0x0016, ftm_read_num_of_aoa}, 
    {0x0017, ftm_read_aoa_cal}, 
    {0x0018, ftm_write_aoa_cal}, 
    {0x0030, ftm_set_tx_num_of_packet}, 
    {0x0031, ftm_set_tx_interval}, 
    {0x0032, ftm_set_tx_on_off}, 
    {0x0040, ftm_set_rx_channel}, 
    {0x0041, ftm_set_rx_on_off}, 
    {0x0042, ftm_read_rx_num_of_packs}, 
    {0x0051, ftm_read_rang_aoa_id}, 
    {0x0050, ftm_write_rang_aoa_id}, 
    {0x0053, ftm_read_rang_aoa_freq}, 
    {0x0052, ftm_write_rang_aoa_freq}, 
    {0x0060, ftm_set_rang_aoa_tx_on_off}, 
    {0x0070, ftm_set_rang_aoa_rx_channel}, 
    {0x0071, ftm_set_rang_aoa_rx_on_off}, 
    {0x0072, ftm_get_rang_aoa_rx_result}, 

    // Add more commands and handlers as needed
};
//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------


/**
 * @brief init Factory test command feature.
 * 
 */
void ftm_handler_init(void)
{
  ftm_cal_nvm_init();
}

/**
 * @brief Get the current state of the factory test mode.
 * 
 * @return uint8_t The current state of the factory test mode (active or inactive).
 */
uint8_t ftm_halder_get_state(void)
{
  return ft_active;
}
/**
 * @brief Factory test command responder.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_command_respond(uint16_t command, uint8_t* buf, uint8_t len)
{
    if(g_respond_func != NULL)
    {
        g_respond_func(command,buf,len);
    }
}

/**
 * @brief Process command.
 * 
 * This function processes the UART receive buffer by separating the command
 * and arguments, converting argument strings to integers, and then executing
 * the corresponding command handler function.
 * 
 * @param command   the command id.
 * @param prtData   Pointer to the data buffer.
 * @param len       The data len of buffer.
 */

uint8_t ftm_halder_polling(uint16_t command, uint8_t *prtData, uint16_t len, cmdhandler_t responder)
{
  uint8_t cb_done = APP_FALSE;
  uint32_t cmd_table_size;
  ftCmd* table;
  g_respond_func = responder;
  cmd_table_size = sizeof(user_command_table) / sizeof(user_command_table[0]);
  table = user_command_table;
  // Find command in the lookup table
  for (uint8_t i = 0; i < cmd_table_size; i++) 
  {
    if (table[i].command == command) 
    {
      table[i].handler(command,prtData,len);
      cb_done = APP_TRUE;
      break;
    }
  }
  //Factory mode command
  if((cb_done == APP_FALSE) && (ft_active == APP_TRUE))
  {
    cmd_table_size = sizeof(ftm_command_table) / sizeof(ftm_command_table[0]);
    table = ftm_command_table;
    // Find command in the lookup table
    for (uint8_t i = 0; i < cmd_table_size; i++) 
    {
      if (table[i].command == command) 
      {
        table[i].handler(command,prtData,len);
        cb_done = APP_TRUE;
        break;
      }
    }
  }
  return cb_done;
}




void ftm_preamble_scan(uint16_t command, uint8_t *buf, uint8_t len)
{
  uint8_t preambleCodeIdx = 0xff;
//  // off/on    timeout
//  // 00         0000
//  uint8_t mode = buf[0]; 
//  if(len == 3 && mode > 0)
//  {
//    uint16_t duration = (uint16_t)buf[1]<<8|buf[2];
//    
//    stPreambleScanningParameters preambleScanningParams;
//    memset(&preambleScanningParams, 0, sizeof(preambleScanningParams));

//    preambleScanningParams.uwbPacketConfig.packetMode      = EN_UWB_PACKET_BPRF_MODE_BPRF_6P81;
//    preambleScanningParams.uwbPacketConfig.stsMode         = EN_UWB_STS_MODE_SP0;
//    
//    preambleScanningParams.trxMode = EN_PSR_RX;
//    preambleScanningParams.psrMode = EN_PSR_SINGLE_MODE;
//    preambleScanningParams.scanDuration = duration;  
//    
//    APP_UWB_PSR_Init(preambleScanningParams);
//    //APP_UWB_PSR_Main();
//    CB_SYSTEM_UwbInit();
//    APP_UWB_PSR_RegisterIrqCallbacks();	
//    preambleCodeIdx = APP_UWB_PSR_Sequence();
//    APP_UWB_PSR_DeregisterIrqCallbacks();
//    
//  }
//  else{
//    APP_UWB_PSR_Suspend();
//    preambleCodeIdx = 0;
//  }
  
  uint8_t respondLen = sizeof(preambleCodeIdx);
  ftm_command_respond(command,&preambleCodeIdx,respondLen);
}


/**
 * @brief Writes the preamble code calibration value to NVM.
 *
 * This function stores the calibrated preamble code value in the non-volatile memory (NVM).
 * @param[in] command The command ID.
 * @param[in] buf Pointer to the input data buffer.
 * @param[in] len Length of the input data buffer.
 */
void ftm_preamble_set(uint16_t command, uint8_t *buf, uint8_t len)
{
  uint8_t statuscode  = 0;
  uint8_t preambleCodeIdx = buf[0];
  
  //save to NV
  statuscode = ftm_cal_nvm_write_preamblecode(preambleCodeIdx);
  
  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
  
}

/**
 * @brief Retrieves the preamble code calibration value.
 *
 * This function reads the preamble code calibration value from NV.
 * If the value is not available or the read operation fails, it returns a default value of 0xFF.
 *
 * @param[in] command The command ID.
 * @param[in] buf Pointer to the input data buffer.
 * @param[in] len Length of the input data buffer.
 */
void ftm_preamble_get(uint16_t command, uint8_t *buf, uint8_t len)
{
  uint8_t statuscode  = 0;
  uint8_t preambleCodeIdx ;
  
  //read from NV
  statuscode = ftm_cal_nvm_read_preamblecode(&preambleCodeIdx);
  if(statuscode != EN_CAL_OK)
  {
    preambleCodeIdx = 0xff;
  }
  uint8_t respondLen = sizeof(preambleCodeIdx);
  ftm_command_respond(command,&preambleCodeIdx,respondLen);
  
}


/**
 * @brief Writes the device role calibration value to NVM.
 *
 * This function stores the calibrated device role value in the non-volatile memory (NVM).
 * @param[in] role The calibrated device role value to be written to NVM
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the calibration data is successfully written
 *                         and verified, or EN_CAL_FAILED if the storage update fails
 */
void ftm_device_role_set(uint16_t command, uint8_t *buf, uint8_t len)
{
  uint8_t statuscode  = 0;
  uint8_t role = buf[0];
  
  //save to NV
  statuscode = ftm_cal_nvm_write_role(role);
  
  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
  
}

/**
 * @brief Reads the device role calibration value from NVM.
 *
 * This function retrieves the device role calibration value from the non-volatile memory (NVM).
 * It checks if the calibration data is valid by examining the CRC and a specific calibration mark.
 *
 * @param[out] role Pointer to a uint8_t variable where the device role value will be stored.
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the device role value is successfully read and the
 *                           calibration data is valid. Returns EN_CAL_FAILED otherwise.
 */
void ftm_device_role_get(uint16_t command, uint8_t *buf, uint8_t len)
{
  uint8_t statuscode  = 0;
  uint8_t role;
  
  //read from NV
  statuscode = ftm_cal_nvm_read_role(&role);
  if(statuscode != EN_CAL_OK)
  {
    role = 0xff;
  }
  uint8_t respondLen = sizeof(role);
  ftm_command_respond(command,&role,respondLen);
  
}


/**
 * @brief Factory test mode on/off Command parser for customer.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_on_0ff(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s:%s\r\n",__func__,(*buf==0)? "Off":"On" );
  uint8_t statuscode= 0;
  uint8_t cal_en =  buf[0];
  if(cal_en)
  {
    LOG("entry Factory mode \n");
    ft_active = APP_TRUE;
  }
  else{
    ft_active = APP_FALSE;
    LOG("Exit Factory mode \n");
  }

  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
}


/**
 * @brief Read chip ID Command parser for customer.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_read_chip_id(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint32_t chipid = cb_efuse_read_chip_id();
  LOG("chipid: %x \n", chipid);
  chipid = BigLittleSwap32(chipid);
  uint8_t respondLen = sizeof(chipid);
  ftm_command_respond(command,(uint8_t*)&chipid,respondLen);
}

/**
 * @brief Read frequency Offset calibration value Command parser for customer.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_read_freq_offset_cal(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint8_t freq = 0;
  uint8_t statuscode = ftm_cal_nvm_read_freqoffset(&freq);
  LOG("ftm_cal_nvm_read_freqoffset return(0:ok, 1:fail): %d \n", statuscode);
  LOG("freqOffsetCalValue: %d \n", freq);

  if(statuscode != EN_CAL_OK)
  {
    freq = 0xff;
  }
  uint8_t respondLen = sizeof(freq);
  ftm_command_respond(command,&freq,respondLen);
}

/**
 * @brief Write frequency Offset calibration value Command parser for customer.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_write_freq_offset_cal(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint8_t freq = *buf;
  uint8_t statuscode = ftm_cal_nvm_write_freqoffset(freq);
  LOG("ftm_cal_nvm_write_freqoffset return(0:ok, 1:fail): %d \n", statuscode);
  LOG("freqOffsetCalValue: %d \n", freq);

  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
}

/**
 * @brief Read power code Command parser for customer.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_read_power_code(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint8_t powerCode = 0;
  uint8_t statuscode = ftm_cal_nvm_read_powercode(&powerCode);
  LOG("APP_CAL_ReadPowerCode return(0:ok, 1:fail): %d \n", statuscode);
  LOG("powerCode: %d \n", powerCode);

  if(statuscode != EN_CAL_OK)
  {
    powerCode = 0xff;
  }
  uint8_t respondLen = sizeof(powerCode);
  ftm_command_respond(command,&powerCode,respondLen);
}

/**
 * @brief Write power code Command parser for customer.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_write_power_code(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint8_t powerCode = *buf;
  uint8_t statuscode = ftm_cal_nvm_write_powercode(powerCode);
  LOG("ftm_cal_nvm_write_powercode return(0:ok, 1:fail): %d \n", statuscode);
  LOG("powerCode: %d \n", powerCode);

  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
}



/**
 * @brief Read the time-of-flight (Tof) calibration value Command parser for customer.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_read_tof_cal(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  int16_t cal_tof = 0;
  uint8_t statuscode = ftm_cal_nvm_read_tofcal(&cal_tof);
  LOG("ftm_cal_nvm_read_tofcal return(0:ok, 1:fail): %d \n", statuscode);
  LOG("cal_tof: %d \n", cal_tof);

  if(statuscode != EN_CAL_OK)
  {
    cal_tof = 0xffff;
  }
  cal_tof = BigLittleSwap16(cal_tof);
  uint8_t respondLen = sizeof(cal_tof);
  ftm_command_respond(command,(uint8_t*)&cal_tof,respondLen);
}


/**
 * @brief Write the time-of-flight (Tof) calibration value Command parser for customer.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_write_tof_cal(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint16_t cal_tof = buf[0]<<8 | buf[1];
  uint8_t statuscode = ftm_cal_nvm_write_tofcal(cal_tof);
  LOG("ftm_cal_nvm_write_tofcal return(0:ok, 1:fail): %d \n", statuscode);
  LOG("cal_tof: %d \n", cal_tof);

  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
}


/**
 * @brief Read the number of Angle of Arrival (AoA) calibration entries Command parser for customer.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_read_num_of_aoa(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint8_t NumOfAoa = 0;
  uint8_t statuscode = ftm_cal_nvm_read_nun_of_aoa(&NumOfAoa);
  LOG("ftm_cal_nvm_read_nun_of_aoa return(0:ok, 1:fail): %d \n", statuscode);
  LOG("NumOfAoa: %d \n", NumOfAoa);

  if(statuscode != EN_CAL_OK)
  {
    NumOfAoa = 0;
  }
  uint8_t respondLen = sizeof(NumOfAoa);
  ftm_command_respond(command,&NumOfAoa,respondLen);
}


/**
 * @brief Read the Angle of Arrival (AoA) calibration values for a specific index Command parser for customer.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_read_aoa_cal(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);
  stCaliAoa aoacal1;
  uint8_t index = buf[0];
  uint8_t statuscode = ftm_cal_nvm_read_aoacal(index, &aoacal1);
  if(statuscode != EN_CAL_OK)
  {
    aoacal1.Calaoah = 0xffff;
    aoacal1.Calaoav = 0xffff;
    aoacal1.Calpdoa1 = 0xffff;
    aoacal1.Calpdoa2 = 0xffff;
  }  
  LOG("ftm_cal_nvm_read_aoacal return: %d \n", statuscode);
  LOG("AoaCalValue, index: %x, aoah: %x, aoav: %x, pdoa1: %x, pdoa2: %x \n", \
      index, aoacal1.Calaoah, aoacal1.Calaoav, aoacal1.Calpdoa1, aoacal1.Calpdoa2);

  stCaliAoa calAoa;
  calAoa.Calaoah = BigLittleSwap16(aoacal1.Calaoah);
  calAoa.Calaoav = BigLittleSwap16(aoacal1.Calaoav);
  calAoa.Calpdoa1 = BigLittleSwap16(aoacal1.Calpdoa1);
  calAoa.Calpdoa2 = BigLittleSwap16(aoacal1.Calpdoa2);
  
  uint8_t respondLen = sizeof(calAoa);
  ftm_command_respond(command,(uint8_t*)&calAoa,respondLen);
}

/**
 * @brief Write the Angle of Arrival (AoA) calibration values for a specific index Command parser for customer.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_write_aoa_cal(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  if(len != 9)
  {
    return;
  }
  stCaliAoa aoacal1;
  uint8_t index = buf[0];
  aoacal1.Calaoah = (int16_t)buf[1]<<8|buf[2];
  aoacal1.Calaoav = (int16_t)buf[3]<<8|buf[4];
  aoacal1.Calpdoa1 = (int16_t)buf[5]<<8|buf[6];
  aoacal1.Calpdoa2 = (int16_t)buf[7]<<8|buf[8];
  uint8_t statuscode = ftm_cal_nvm_write_aoacal(index, aoacal1);
  LOG("ftm_cal_nvm_write_aoacal return: %d \n", statuscode);
  LOG("AoaCalValue, index: %x, aoah: %x, aoav: %x, pdoa1: %x, pdoa2: %x \n", \
      aoacal1.index, aoacal1.aoah, aoacal1.aoav, aoacal1.pdoa1, aoacal1.pdoa2);

  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
}


/**
 * @brief Sets the number of packets for transmission during calibration Command parser for customer.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_set_tx_num_of_packet(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint32_t NumOfPacket =0;
  if(len != sizeof(NumOfPacket))
  {
    return;
  }
  NumOfPacket = buf[0]<<24|buf[1]<<16|buf[2]<<8|buf[3];
  uint8_t statuscode = ftm_uwb_cal_set_tx_packets(NumOfPacket);
  LOG("ftm_uwb_cal_set_tx_packets return(0:ok, 1:fail): %d \n", statuscode);
  LOG("NumOfPacket: %d \n", NumOfPacket);

  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
}

/**
 * @brief Sets the interval between transmission during calibration Command parser for customer.
 * 
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_set_tx_interval(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint16_t interval =0;
  if(len != sizeof(interval))
  {
    return;
  }
  interval = buf[0]<<8|buf[1];
  uint8_t statuscode = ftm_uwb_cal_set_tx_interval(interval);
  LOG("ftm_uwb_cal_set_tx_interval return(0:ok, 1:fail): %d \n", statuscode);
  LOG("interval: %d \n", interval);

  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
}

/**
 * @brief Controls the start or stop of periodic UWB signal transmission.
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_set_tx_on_off(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint8_t txmode = *buf;
  LOG("txmode: %d \n", txmode);
  uint8_t statuscode = 0;  
  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);

  ftm_uwb_cal_set_tx_onoff(txmode);
  LOG("ftm_uwb_cal_set_tx_onoff return(0:ok, 1:fail): %d \n", statuscode);


}


/**
 * @brief Sets the UWB RX channel for calibration.
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_set_rx_channel(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint8_t rxChannel = *buf;
  uint8_t statuscode = ftm_uwb_cal_set_rx_channel(rxChannel);
  LOG("ftm_uwb_cal_set_rx_channel return(0:ok, 1:fail): %d \n", statuscode);
  LOG("rxChannel: %d \n", rxChannel);

  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
}

/**
 * @brief Controls the start or stop of UWB signal reception for calibration.
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_set_rx_on_off(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint8_t rxOnOff = *buf;
  uint8_t statuscode = 0;
  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
  ftm_uwb_cal_set_rx_onoff(rxOnOff);
  LOG("ftm_uwb_cal_set_rx_onoff return(0:ok, 1:fail): %d \n", statuscode);
  LOG("rxOnOff: %d \n", rxOnOff);


}

/**
 * @brief Retrieves the number of received UWB packets during calibration.
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_read_rx_num_of_packs(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint32_t RxNumofPacks ;
  uint8_t statuscode = ftm_uwb_cal_get_rx_packets(&RxNumofPacks);
  LOG("ftm_uwb_cal_get_rx_packets return(0:ok, 1:fail): %d \n", statuscode);
  LOG("RxNumofPacks: %d \n", RxNumofPacks);

  RxNumofPacks = BigLittleSwap32(RxNumofPacks);
  uint8_t respondLen = sizeof(RxNumofPacks);
  ftm_command_respond(command,(uint8_t*)&RxNumofPacks,respondLen);
}

/**
 * @brief Read the Rang AoA ID
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_read_rang_aoa_id(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint32_t rangAoaId ;
  uint8_t statuscode = ftm_cal_nvm_read_rngaoa_id(&rangAoaId);
  LOG("ftm_cal_nvm_read_rngaoa_id return(0:ok, 1:fail): %d \n", statuscode);
  LOG("rangAoaId: %d \n", rangAoaId);

  rangAoaId = BigLittleSwap32(rangAoaId);
  uint8_t respondLen = sizeof(rangAoaId);
  ftm_command_respond(command,(uint8_t*)&rangAoaId,respondLen);
}


/**
 * @brief Write the Rang AoA ID
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_write_rang_aoa_id(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint32_t rangAoaId ;

  if(len == 4)
    rangAoaId = buf[0]<<24|buf[1]<<16|buf[2]<<8|buf[3];
  else
    rangAoaId = buf[0]<<8|buf[1];
  uint8_t statuscode = ftm_cal_nvm_write_rngaoa_id(rangAoaId);
  LOG("ftm_cal_nvm_write_rngaoa_id return(0:ok, 1:fail): %d \n", statuscode);
  LOG("rangAoaId: %d \n", rangAoaId);

  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
}


/**
 * @brief Read the Rang AoA frequency
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_read_rang_aoa_freq(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint8_t freq ;
  uint8_t statuscode = ftm_cal_nvm_read_rngaoa_freq(&freq);
  LOG("ftm_cal_nvm_read_rngaoa_freq return(0:ok, 1:fail): %d \n", statuscode);
  LOG("Rang AoA freq: %d \n", freq);

  uint8_t respondLen = sizeof(freq);
  ftm_command_respond(command,&freq,respondLen);
}

/**
 * @brief Write the Rang AoA frequency
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_write_rang_aoa_freq(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint8_t freq = *buf;
  uint8_t statuscode = ftm_cal_nvm_write_rngaoa_freq(freq);
  LOG("ftm_cal_nvm_read_rngaoa_freq return(0:ok, 1:fail): %d \n", statuscode);
  LOG("Rang AoA freq: %d \n", freq);

  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
}

/**
 * @brief Control the Rang AoA tx On or Off
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_set_rang_aoa_tx_on_off(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint8_t OnOff = *buf;
  uint8_t statuscode = 0;
  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
  ftm_uwb_cal_set_rngaoa_tx_onoff(OnOff);
  LOG("ftm_uwb_cal_set_rngaoa_tx_onoff return(0:ok, 1:fail): %d \n", statuscode);
  LOG("Rang AoA OnOff: %d \n", OnOff);

 
}

/**
 * @brief Set the Rang AoA rx channel
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_set_rang_aoa_rx_channel(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint8_t RxChannel = *buf;
  uint8_t statuscode = ftm_uwb_cal_set_rngaoa_rx_channel(RxChannel); 
  LOG("ftm_uwb_cal_set_rngaoa_rx_channel return(0:ok, 1:fail): %d \n", statuscode);
  LOG("Rang AoA RxChannel: %d \n", RxChannel);

  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
}

/**
 * @brief Control the Rang AoA rx On or Off
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_set_rang_aoa_rx_on_off(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  uint8_t OnOff = *buf;
  uint8_t statuscode = 0;
  uint8_t respondLen = sizeof(statuscode);
  ftm_command_respond(command,&statuscode,respondLen);
  ftm_uwb_cal_set_rngaoa_rx_onoff(OnOff);
  LOG("ftm_uwb_cal_set_rngaoa_rx_onoff return(0:ok, 1:fail): %d \n", statuscode);
  LOG("Rang AoA OnOff: %d \n", OnOff);


}

/**
 * @brief Read the Rang AoA rx result data
 * @param command command ID.
 * @param buf Pointer to the input data.
 * @param len The len of input data.
 */
void ftm_get_rang_aoa_rx_result(uint16_t command, uint8_t *buf, uint8_t len)
{
  LOG("%s\r\n",__func__);

  stRNGAOARx rxResultData;
  uint8_t statuscode = ftm_uwb_cal_get_rngaoa_rx_resulf(&rxResultData); 
  LOG("ftm_uwb_cal_get_rngaoa_rx_resulf return(0:ok, 1:fail): %d \n", statuscode);
  LOG("DIS:%d\n", rxResultData.dis);
  LOG("AOAH:%d\n", rxResultData.aoah);
  LOG("AOAV:%d\n", rxResultData.aoav);
  LOG("PDOAH:%d\n", rxResultData.pdoah);
  LOG("PDOAV:%d\n", rxResultData.pdoav);
  LOG("AVG_RSL1:%d\n", rxResultData.avg_rsl1);
  LOG("AVG_RSL2:%d\n", rxResultData.avg_rsl2);
  LOG("AVG_RSL3:%d\n", rxResultData.avg_rsl3);

  rxResultData.dis = BigLittleSwap16(rxResultData.dis);
  rxResultData.aoah = BigLittleSwap16(rxResultData.aoah);
  rxResultData.aoav = BigLittleSwap16(rxResultData.aoav);
  rxResultData.pdoah = BigLittleSwap16(rxResultData.pdoah);
  rxResultData.pdoav = BigLittleSwap16(rxResultData.pdoav);
  rxResultData.avg_rsl1 = BigLittleSwap16(rxResultData.avg_rsl1);
  rxResultData.avg_rsl2 = BigLittleSwap16(rxResultData.avg_rsl2);
  rxResultData.avg_rsl3 = BigLittleSwap16(rxResultData.avg_rsl3);
  uint8_t respondLen = sizeof(rxResultData);
  ftm_command_respond(command,(uint8_t*)&rxResultData,respondLen);
}

