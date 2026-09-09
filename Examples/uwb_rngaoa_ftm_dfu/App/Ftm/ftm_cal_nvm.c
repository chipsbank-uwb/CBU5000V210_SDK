/**
 * @file ftm_cal_nvm.c
 * @brief Header file for NVM calibration functions.
 *
 * @details This file contains function declarations for initializing the NV calibration module
 * and loading calibration data from NVM.
 * @author  Chipsbank
 * @date    2024
 */
 
//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "ftm_cal_nvm.h"
#include "APP_common.h"
#include "CB_flash.h"
#include "CB_system.h"
#include "CB_crc.h"
#include <string.h>

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define FTM_CAL_NVM_ENABLE CB_FALSE
#if (FTM_CAL_NVM_ENABLE == CB_TRUE)
  #include "AppCpuUart.h"
  #define LOG(...) APP_UartPrintf(__VA_ARGS__)
#else
  #define LOG(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define DEF_CAL_NVM_PAGE_NUM                (1)
#define DEF_CAL_NVM_PAGE_SIZE               (256)
#define DEF_MAX_AOA_NUMBER                     16 
#define DEF_MIN_AOA_NUMBER                     1
#define DEF_BACKUP_PAGE_NUM                   (DEF_CAL_NVM_PAGE_NUM+1)

//-------------------------------
// ENUM SECTION
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

enum calDataPos{
  CAL_DEVICEID_POS = 0,  
  CAL_DEVICEROLE_POS,
  CAL_PBCODEIDX_POS,
  CAL_FREQOFFSET_POS,
  CAL_POWERCODE_POS,
  CAL_TOFCAL_POS,
  CAL_RANGAOAFREQ_POS,
};

struct ftm_cal_nv_data_t{
  uint32_t  crc;
    
  uint32_t  cal_mark;
  uint32_t  device_ID;  
  uint8_t   device_role;
  uint8_t   pbcode_idx;
  uint8_t   freq_offset;
  uint8_t   power_code;
  uint8_t   rngaoa_freq;
  uint8_t   reserved[23];//32-4-5
    
  uint32_t  reserved1_mark;  
  uint8_t   reserved1[32];
    
  int16_t   tof_cal;
  uint16_t  aoa_idx_mark;
  stCaliAoa aoaCalAry[DEF_MAX_AOA_NUMBER];// 16*8 +4 = 132
 
  uint8_t   reserved2[48];// 256 -132 - 36 - 36 - 4 = 48
};
#if ((4*5+8*DEF_MAX_AOA_NUMBER) > 0x100)  //page size
#error " sizeof(struct ftm_cal_nv_data_t) over 256bytes(page size)"
#endif

struct ftm_cal_nv_data_t g_cal_nv_data = {0};

// Flash operation functions porting
void CAL_NVM_READ_PORT(uint16_t PageNumber, uint8_t *data, uint16_t length)
{
  cb_flash_enter_elevation();
  cb_flash_read_page(PageNumber, data, length);
  cb_flash_exit_elevation();
}
void CAL_NVM_WRITE_PORT(uint16_t PageNumber, uint8_t *data, uint16_t length)
{
  cb_flash_enter_elevation();
  cb_flash_program_page(PageNumber, data, length);
  cb_flash_exit_elevation();
}
void CAL_NVM_ERASE_PORT(uint16_t PageNumber)
{
  cb_flash_enter_elevation();
  cb_flash_erase_page(PageNumber);
  cb_flash_exit_elevation();
}


/**
 * @brief Updates the calibration data in NVM.
 *
 * This function updates the calibration data in NV and validates it using the CRC module.
 * If the data is valid, it is stored in the global variable `g_cal_nv_data`.
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the calibration data is successfully updated,
 *                         or EN_CAL_FAILED if the data is not valid.
 */
enCALReturnCode ftm_cal_nvm_storage_update(void)
{
  struct ftm_cal_nv_data_t read_cal_nv = {0};
  uint32_t CrcReadBackcheckagain;
  //update crc
  cb_crc_algo_config(EN_CRC32, EN_InitValOne, EN_CRCRefOut_Enable, EN_CRCRefIn_Enable, 0x04C11DB7, 0xFFFFFFFF);
  cb_crc_process_from_input_data(((uint8_t *)&g_cal_nv_data.crc)+4, sizeof(g_cal_nv_data)-4,EN_CRC_ReInit_Enable);//only do the calibration data CRC
  g_cal_nv_data.crc =cb_crc_get_crc_result();
  //check if the write progress necessary
  CAL_NVM_READ_PORT (DEF_CAL_NVM_PAGE_NUM,(uint8_t *)&read_cal_nv,sizeof(read_cal_nv));//read data from qspi to dataram.
  if(memcmp(&read_cal_nv,&g_cal_nv_data,sizeof(struct ftm_cal_nv_data_t)) == 0)
  {
      return EN_CAL_OK;
  }
  
  //backup area
  CAL_NVM_ERASE_PORT(DEF_BACKUP_PAGE_NUM);
  CAL_NVM_WRITE_PORT (DEF_BACKUP_PAGE_NUM,(uint8_t *)&g_cal_nv_data,sizeof(g_cal_nv_data));
  memset(&read_cal_nv,0,sizeof(struct ftm_cal_nv_data_t));
  CAL_NVM_READ_PORT (DEF_BACKUP_PAGE_NUM,(uint8_t *)&read_cal_nv,sizeof(read_cal_nv));
  cb_crc_process_from_input_data(((uint8_t *)&read_cal_nv.crc)+4, sizeof(read_cal_nv)-4,EN_CRC_ReInit_Enable);//only do the calibration data CRC
  CrcReadBackcheckagain=cb_crc_get_crc_result();
  if(CrcReadBackcheckagain != read_cal_nv.crc)
      return EN_CAL_FAILED;
  
  //main area
  CAL_NVM_ERASE_PORT(DEF_CAL_NVM_PAGE_NUM);
  CAL_NVM_WRITE_PORT (DEF_CAL_NVM_PAGE_NUM,(uint8_t *)&g_cal_nv_data,sizeof(g_cal_nv_data));//write 256 byte to eeprom;
  memset(&read_cal_nv,0,sizeof(struct ftm_cal_nv_data_t));
  CAL_NVM_READ_PORT (DEF_CAL_NVM_PAGE_NUM,(uint8_t *)&read_cal_nv,sizeof(read_cal_nv));//read data from qspi to dataram.
  cb_crc_process_from_input_data(((uint8_t *)&read_cal_nv.crc)+4, sizeof(read_cal_nv)-4,EN_CRC_ReInit_Enable);//only do the calibration data CRC
  CrcReadBackcheckagain=cb_crc_get_crc_result();
  return (CrcReadBackcheckagain == read_cal_nv.crc) ? EN_CAL_OK : EN_CAL_FAILED;
}

/**
 * @brief Initializes the NV calibration module.
 *
 * This function initializes the CRC module used for data validation in NVM.
 */
enCALReturnCode ftm_cal_nvm_init(void)
{
  cb_crc_init();// Init CRC Module;
  if (ftm_cal_nvm_load_data() == EN_CAL_OK) {
    return EN_CAL_OK;
  } else {
    return EN_CAL_FAILED;
  }
}

/**
 * @brief Loads calibration data from NVM.
 *
 * This function reads the calibration data from NV and validates it using the CRC module.
 * If the data is valid, it is stored in the global variable `g_cal_nv_data`.
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the calibration data is successfully loaded,
 *                         or EN_CAL_FAILED if the data is not valid.
 */
enCALReturnCode ftm_cal_nvm_load_data (void)
{
  cb_crc_algo_config(EN_CRC32, EN_InitValOne, EN_CRCRefOut_Enable, EN_CRCRefIn_Enable, 0x04C11DB7, 0xFFFFFFFF);//config crc module.
  CAL_NVM_READ_PORT(DEF_CAL_NVM_PAGE_NUM,(uint8_t *)&g_cal_nv_data,sizeof(g_cal_nv_data));//read data from qspi to dataram.
  cb_crc_process_from_input_data(((uint8_t *)&g_cal_nv_data.crc)+4, sizeof(g_cal_nv_data)-4,EN_CRC_ReInit_Enable);//only do the calibration data CRC
  uint32_t crc_of_cal_data=cb_crc_get_crc_result();
  
  if(crc_of_cal_data != g_cal_nv_data.crc) //If the NV data is not calibrated
  {
    //main area check fail, then check the backup area
    memset(&g_cal_nv_data,0,sizeof(struct ftm_cal_nv_data_t));
    CAL_NVM_READ_PORT (DEF_BACKUP_PAGE_NUM,(uint8_t *)&g_cal_nv_data,sizeof(g_cal_nv_data));
    cb_crc_process_from_input_data(((uint8_t *)&g_cal_nv_data.crc)+4, sizeof(g_cal_nv_data)-4,EN_CRC_ReInit_Enable);//only do the calibration data CRC
    crc_of_cal_data=cb_crc_get_crc_result();
    if(crc_of_cal_data != g_cal_nv_data.crc)
    {
        #if 1
        memset(&g_cal_nv_data, 0, sizeof(g_cal_nv_data)); // Clean eeprom 256 byte data to 0
        return ftm_cal_nvm_storage_update();
        #else
        return EN_CAL_FAILED ;
        #endif
    }
  }
  
  //If the NV data is calibrated
  return EN_CAL_OK ;
}



/**
 * @brief Reads the frequency offset calibration value from NVM.
 *
 * This function retrieves the frequency offset calibration value from the non-volatile memory (NVM).
 * It checks if the calibration data is valid by examining the CRC and a specific calibration mark.
 *
 * @param[out] CalFreOffsetVal Pointer to a uint8_t variable where the frequency offset value will be stored.
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the frequency offset value is successfully read and the
 *                           calibration data is valid. Returns EN_CAL_FAILED otherwise.
 */
enCALReturnCode ftm_cal_nvm_read_freqoffset(uint8_t* CalFreOffsetVal)
{
  if (g_cal_nv_data.crc && (g_cal_nv_data.cal_mark&(1<<CAL_FREQOFFSET_POS)))
  {
    *CalFreOffsetVal = g_cal_nv_data.freq_offset;  // Get the calibrated frequency offset value in system byte 3
    return EN_CAL_OK;
  }
  else
  {
    return EN_CAL_FAILED;
  }
}

/**
 * @brief Reads the power code calibration value from NVM.
 *
 * This function retrieves the power code calibration value from the non-volatile memory (NVM).
 * It checks if the calibration data is valid by examining the CRC and a specific calibration mark.
 *
 * @param[out] CalPowercode Pointer to a uint8_t variable where the power code value will be stored.
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the power code value is successfully read and the
 *                           calibration data is valid. Returns EN_CAL_FAILED otherwise.
 */
enCALReturnCode ftm_cal_nvm_read_powercode(uint8_t* CalPowercode)
{
  if (g_cal_nv_data.crc && (g_cal_nv_data.cal_mark&(1<<CAL_POWERCODE_POS)))
  {
    *CalPowercode = g_cal_nv_data.power_code;  // Get the calibrated power code value in system byte 4
    return EN_CAL_OK;
  }
  else
  {
    return EN_CAL_FAILED;
  }
}

/**
 * @brief Reads the time-of-flight (ToF) calibration value from NVM.
 *
 * This function retrieves the time-of-flight (ToF) calibration value from the non-volatile memory (NVM).
 * It checks if the calibration data is valid by examining the CRC and a specific calibration mark.
 *
 * @param[out] CalTof Pointer to an int16_t variable where the ToF calibration value will be stored.
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the ToF calibration value is successfully read and the
 *                           calibration data is valid. Returns EN_CAL_FAILED otherwise.
 */
enCALReturnCode ftm_cal_nvm_read_tofcal(int16_t* CalTof)
{
  if (g_cal_nv_data.crc && (g_cal_nv_data.cal_mark&(1<<CAL_TOFCAL_POS)))
  {
    *CalTof = (int16_t)g_cal_nv_data.tof_cal;  // Combine the two bytes
    return EN_CAL_OK;
  }
  else
  {
    return EN_CAL_FAILED;
  }
}

/**
 * @brief Retrieves the number of Angle of Arrival (AoA) values from NVM.
 *
 * This function checks if the number of AoA values stored in NVM is valid and,
 * if so, retrieves it. The function uses a marker to determine if the data is valid.
 *
 * @param[out] NumOfAoa Pointer to a variable where the number of AoA values will be
 *                      stored if the data is valid. The value is read from a specific
 *                      byte in the NVM.
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the number of AoA values is successfully
 *                         retrieved, or EN_CAL_FAILED if the data is not valid.
 */
enCALReturnCode ftm_cal_nvm_read_nun_of_aoa(uint8_t* NumOfAoa)
{
  if (g_cal_nv_data.crc)
  {
    uint8_t aoa_num=0;
    for(uint8_t i=0;i<DEF_MAX_AOA_NUMBER;i++)
    {
      if(g_cal_nv_data.aoa_idx_mark&(1<<i))
        aoa_num++;
    }
    *NumOfAoa = aoa_num;  // Get the num of aoa value in system byte 7
    return EN_CAL_OK;
  }
  else
  {
    return EN_CAL_FAILED;
  }
}


/**
 * @brief Reads the Angle of Arrival (AoA) calibration values for a specific index from NVM.
 *
 * This function retrieves the AoA calibration values for a specific index from the non-volatile memory (NVM).
 * It checks if the calibration data is valid by examining the CRC and a specific calibration mark.
 *
 * @param[in]  CalAoaIndex The index of the AoA calibration values to be read.
 * @param[out] CalAoa      Pointer to a stCaliAoa structure where the AoA calibration values will be stored.
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the AoA calibration values are successfully read and the
 *                           calibration data is valid. Returns EN_CAL_FAILED otherwise.
 */
enCALReturnCode ftm_cal_nvm_read_aoacal(uint8_t CalAoaIndex ,stCaliAoa* CalAoa) //index range 0-15 total 16 sets of data.
{
  if(CalAoaIndex>=DEF_MAX_AOA_NUMBER)//index value start from 0
  {
    return EN_CAL_FAILED;
  }
  if (g_cal_nv_data.crc)
  {
    if(g_cal_nv_data.aoa_idx_mark&(1<<CalAoaIndex))
    {
      //memcmp(CalAoa,&g_cal_nv_data.aoaCalAry[CalAoaIndex],sizeof(CalAoa));
      *CalAoa = g_cal_nv_data.aoaCalAry[CalAoaIndex];
      return EN_CAL_OK;
    }

  }

  return EN_CAL_FAILED;
}

/**
 * @brief Writes the frequency offset calibration value to NVM.
 *
 * This function stores the calibrated frequency offset value in the non-volatile memory (NVM).
 * It performs the following steps:
 * 1. Stores the calibration value in the global calibration data structure
 * 2. Sets a marker bit to indicate that frequency offset calibration data is present
 * 3. Updates the storage by calculating CRC and writing to NVM
 *
 * @param[in] CalFreOffsetVal The calibrated frequency offset value to be written to NVM
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the calibration data is successfully written
 *                         and verified, or EN_CAL_FAILED if the storage update fails
 */
enCALReturnCode ftm_cal_nvm_write_freqoffset(uint8_t CalFreOffsetVal)
{
    g_cal_nv_data.freq_offset = CalFreOffsetVal;
    g_cal_nv_data.cal_mark |= (1<<CAL_FREQOFFSET_POS);
    return ftm_cal_nvm_storage_update();
}

/**
 * @brief Writes the power code calibration value to NVM.
 *
 * This function stores the calibrated power code value in the non-volatile memory (NVM).
 *
 * @param[in] CalPowercode The calibrated power code value to be written to NVM
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the calibration data is successfully written
 *                         and verified, or EN_CAL_FAILED if the storage update fails
 */
enCALReturnCode ftm_cal_nvm_write_powercode(uint8_t CalPowercode)
{ 
    g_cal_nv_data.power_code = CalPowercode;
    g_cal_nv_data.cal_mark |= (1<<CAL_POWERCODE_POS);
    return ftm_cal_nvm_storage_update();
}

/**
 * @brief Writes the time-of-flight (ToF) calibration value to NVM.
 *
 * This function stores the calibrated ToF value in the non-volatile memory (NVM).
 * It performs the following steps:
 * 1. Stores the ToF calibration value in the global calibration data structure
 * 2. Sets a marker bit to indicate that ToF calibration data is present
 * 3. Updates the storage by calculating CRC and writing to NVM
 *
 * @param[in] CalTof The calibrated ToF value to be written to NVM
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the calibration data is successfully written
 *                         and verified, or EN_CAL_FAILED if the storage update fails
 */
enCALReturnCode ftm_cal_nvm_write_tofcal(int16_t CalTof)
{
    g_cal_nv_data.tof_cal = CalTof;
    g_cal_nv_data.cal_mark |= (1<<CAL_TOFCAL_POS);
    return ftm_cal_nvm_storage_update();
}

/**
 * @brief Writes Angle of Arrival (AoA) calibration values to NVM.
 *
 * This function stores the calibrated AoA values for a specific index in the non-volatile 
 * memory (NVM). 
 *
 * @param[in] aoaindex The index (0-15) where the AoA calibration values should be stored
 * @param[in] CalAoa   The AoA calibration values to be written to NVM
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the calibration data is successfully written
 *                         and verified, or EN_CAL_FAILED if the index is invalid or storage 
 *                         update fails
 */
enCALReturnCode ftm_cal_nvm_write_aoacal (uint8_t aoaindex,stCaliAoa CalAoa)//index from 0 to 15,allow to multipule write in the same index calirabtion aoa value.
{
  if(aoaindex>=DEF_MAX_AOA_NUMBER)
  {
    return EN_CAL_FAILED;
  }
  memcpy(&g_cal_nv_data.aoaCalAry[aoaindex],&CalAoa,sizeof(stCaliAoa));
  g_cal_nv_data.aoa_idx_mark |=  1<<aoaindex;
  return ftm_cal_nvm_storage_update();
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
enCALReturnCode ftm_cal_nvm_write_role(uint8_t role)
{ 
  g_cal_nv_data.device_role = role;
  g_cal_nv_data.cal_mark |= (1<<CAL_DEVICEROLE_POS);
  return ftm_cal_nvm_storage_update();
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
enCALReturnCode ftm_cal_nvm_read_role(uint8_t* role)
{
  if (g_cal_nv_data.crc && (g_cal_nv_data.cal_mark & (1<<CAL_DEVICEROLE_POS)))
  {
    *role = g_cal_nv_data.device_role;  // Get the device role
    return EN_CAL_OK;
  }
  else
  {
    return EN_CAL_FAILED;
  }
}


/**
 * @brief Reads the device ID calibration value from NVM.
 *
 * This function retrieves the device ID calibration value from the non-volatile memory (NVM).
 * It checks if the calibration data is valid by examining the CRC and a specific calibration mark.
 *
 * @param[out] DeviceId Pointer to a uint32_t variable where the device ID value will be stored.
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the device ID value is successfully read and the
 *                           calibration data is valid. Returns EN_CAL_FAILED otherwise.
 */
enCALReturnCode ftm_cal_nvm_read_rngaoa_id(uint32_t* DeviceId)
{
  if (g_cal_nv_data.crc && (g_cal_nv_data.cal_mark & (1<<CAL_DEVICEID_POS)))
  {
    *DeviceId = g_cal_nv_data.device_ID;  // Get the device ID
    return EN_CAL_OK;
  }
  else
  {
    return EN_CAL_FAILED;
  }
}


/**
 * @brief Writes the device ID calibration value to NVM.
 *
 * This function stores the calibrated device ID value in the non-volatile memory (NVM).
 *
 * @param[in] DeviceId The calibrated device ID value to be written to NVM
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the calibration data is successfully written
 *                         and verified, or EN_CAL_FAILED if the storage update fails
 */
enCALReturnCode ftm_cal_nvm_write_rngaoa_id(uint32_t DeviceId)
{ 
  g_cal_nv_data.device_ID = DeviceId;
  g_cal_nv_data.cal_mark |= (1<<CAL_DEVICEID_POS);
  return ftm_cal_nvm_storage_update();
}


/**
 * @brief Reads the ranging AoA frequency calibration value from NVM.
 *
 * This function retrieves the ranging AoA frequency calibration value from the non-volatile memory (NVM).
 * It checks if the calibration data is valid by examining the CRC and a specific calibration mark.
 *
 * @param[out] freq Pointer to a uint8_t variable where the ranging AoA frequency value will be stored.
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the ranging AoA frequency value is successfully read and the
 *                         calibration data is valid. Returns EN_CAL_FAILED otherwise.
 */
enCALReturnCode ftm_cal_nvm_read_rngaoa_freq(uint8_t* freq)
{
  if (g_cal_nv_data.crc && (g_cal_nv_data.cal_mark & (1<<CAL_RANGAOAFREQ_POS)))
  {
    *freq = g_cal_nv_data.rngaoa_freq;  // Get the rngaoa freq
    return EN_CAL_OK;
  }
  else
  {
    return EN_CAL_FAILED;
  }
}

/**
 * @brief Writes the ranging AoA frequency calibration value to NVM.
 *
 * This function stores the calibrated ranging AoA frequency value in the non-volatile memory (NVM).
 *
 * @param[in] freq The calibrated ranging AoA frequency value to be written to NVM
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the calibration data is successfully written
 *                         and verified, or EN_CAL_FAILED if the storage update fails
 */
enCALReturnCode ftm_cal_nvm_write_rngaoa_freq(uint8_t freq)
{
  g_cal_nv_data.rngaoa_freq = freq;
  g_cal_nv_data.cal_mark |= (1<<CAL_RANGAOAFREQ_POS);
  return ftm_cal_nvm_storage_update();
}



/**
 * @brief Writes the preamble code calibration value to NVM.
 *
 * This function stores the calibrated preamble code value in the non-volatile memory (NVM).
 * @param[in] PreambleCodeIdx The calibrated preamble code value to be written to NVM
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the calibration data is successfully written
 *                         and verified, or EN_CAL_FAILED if the storage update fails
 */
enCALReturnCode ftm_cal_nvm_write_preamblecode(uint8_t PreambleCodeIdx)
{ 
  g_cal_nv_data.pbcode_idx = PreambleCodeIdx;
  g_cal_nv_data.cal_mark |= (1<<CAL_PBCODEIDX_POS);
  return ftm_cal_nvm_storage_update();

}


/**
 * @brief Reads the preamble code calibration value from NVM.
 *
 * This function retrieves the preamble code calibration value from the non-volatile memory (NVM).
 * It checks if the calibration data is valid by examining the CRC and a specific calibration mark.
 *
 * @param[out] PreambleCodeIdx Pointer to a uint8_t variable where the preamble code value will be stored.
 *
 * @return enCALReturnCode Returns EN_CAL_OK if the preamble code value is successfully read and the
 *                         calibration data is valid. Returns EN_CAL_FAILED otherwise.
 */
enCALReturnCode ftm_cal_nvm_read_preamblecode(uint8_t* PreambleCodeIdx)
{
  if (g_cal_nv_data.crc && (g_cal_nv_data.cal_mark & (1<<CAL_PBCODEIDX_POS)))
  {
    *PreambleCodeIdx = g_cal_nv_data.pbcode_idx;  // Get the PreambleCodeIdx
    return EN_CAL_OK;
  }
  else
  {
    return EN_CAL_FAILED;
  }
}

