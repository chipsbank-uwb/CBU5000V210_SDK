

#ifndef __FTM_CAL_NVM_H_
#define __FTM_CAL_NVM_H_

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>


//-------------------------------
// DEFINE SECTION
//-------------------------------


//-------------------------------
// ENUM SECTION
//-------------------------------
/**
 * @enum enCALReturnCode
 * @brief Enumeration for calibration return codes.
 *
 * This enumeration defines the possible return codes for calibration functions.
 */
typedef enum
{
  EN_CAL_OK = 0,
  EN_CAL_FAILED = 1,
}enCALReturnCode;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
/**
 * @struct stCaliAoa
 * @brief Structure to hold calibration data for Angle of Arrival (AOA).
 *
 * This structure contains the calibration values for AOA-related parameters.
 */
typedef struct
{
  int16_t           Calaoah;
  int16_t           Calaoav;
  int16_t           Calpdoa1;
  int16_t           Calpdoa2; 
}stCaliAoa;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
enCALReturnCode ftm_cal_nvm_init(void);
enCALReturnCode ftm_cal_nvm_load_data(void);
enCALReturnCode ftm_cal_nvm_read_freqoffset(uint8_t* CalFreOffsetVal);
enCALReturnCode ftm_cal_nvm_read_powercode(uint8_t* CalPowercode);
enCALReturnCode ftm_cal_nvm_read_tofcal(int16_t* CalTof);
enCALReturnCode ftm_cal_nvm_read_nun_of_aoa(uint8_t* NumOfAoa);
enCALReturnCode ftm_cal_nvm_read_aoacal(uint8_t CalAoaIndex, stCaliAoa* CalAoa);
enCALReturnCode ftm_cal_nvm_read_rngaoa_id(uint32_t* DeviceId);
enCALReturnCode ftm_cal_nvm_read_role(uint8_t* Role);
enCALReturnCode ftm_cal_nvm_write_freqoffset(uint8_t CalFreOffsetVal);
enCALReturnCode ftm_cal_nvm_write_powercode(uint8_t CalPowercode);
enCALReturnCode ftm_cal_nvm_write_tofcal(int16_t CalTof);
enCALReturnCode ftm_cal_nvm_write_aoacal(uint8_t aoaindex, stCaliAoa CalAoa);
enCALReturnCode ftm_cal_nvm_write_rngaoa_id(uint32_t DeviceId);
enCALReturnCode ftm_cal_nvm_read_rngaoa_freq(uint8_t* freq);
enCALReturnCode ftm_cal_nvm_write_rngaoa_freq(uint8_t freq);
enCALReturnCode ftm_cal_nvm_write_role(uint8_t Role);
enCALReturnCode ftm_cal_nvm_write_preamblecode(uint8_t PreambleCodeIdx);
enCALReturnCode ftm_cal_nvm_read_preamblecode(uint8_t* PreambleCodeIdx);


#endif /* __FTM_CAL_NVM_H_ */




