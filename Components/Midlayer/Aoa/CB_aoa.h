/**
 * @file    CB_aoa.h
 * @brief   Angle of Arrival (AOA) Feature Module Header
 * @details This header file contains function prototypes and data structures related to the Angle of Arrival (AOA) feature module.
 * @author  Chipsbank
 * @date    2024
 */
 
#ifndef __CB_AOA_H
#define __CB_AOA_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_system_types.h"
//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------
typedef enum
{
  EN_AOA_OK     = 0x00,   /**< Result Ok */
  EN_AOA_ERROR  = 0x01,   /**< Result Error */
} CB_AOA_STATUS;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

/**
 * @brief Structure for calculated elevation and azimuth phase differences.
 */
typedef struct
{  
  float phaseDiffRx0Rx1;
  float phaseDiffRx0Rx2;
  float phaseDiffRx1Rx2;
} __attribute__((aligned(4))) stAOA_CompensatedData;

/**
 * @brief Structure for 3D antenna attributes
 */
typedef struct {
    float ant_height;
    float ant_width;
    uint8_t ant_type;
    uint8_t ant_pos[3];
} st_antenna_attribute_3d;

/**
 * @brief Structure for 2D antenna attributes
 */
typedef struct {
    float ant_width;
    uint8_t ant_pos[2];
} st_antenna_attribute_2d;

/**
 * @brief Structure for LUT attributes
 */
typedef struct {
    uint8_t size_azi;
    uint8_t size_ele;
    uint8_t step_azi;
    uint8_t step_ele;
    uint8_t size_col;
    int8_t azi_est_lower_limit;
    int8_t azi_est_upper_limit;
    int8_t ele_est_lower_limit;
    int8_t ele_est_upper_limit;
    const int16_t *lut_data;
} cb_uwbaoa_lut_attribute_st;

/**
 * @brief Structure for fov attributes
 */
typedef struct {
    uint8_t step_ele;
    int8_t ele_est_lower_limit;
    int8_t ele_est_upper_limit;
} cb_uwbaoa_fov_attribute_st;
/**
 * @brief Structure for LUT file header
 */
typedef struct
{
  uint32_t magic_number;  // 0xA5A5A5A5
  uint32_t crc32;         // CRC32 checksum of the file
  uint32_t version;       // 0x00000001
  uint32_t lut_storage_size;      // size of total LUT in bytes
} stPdLutFileHeader_st; 

/**
 * @brief Structure for single LUT content
 */
typedef struct
{
  cb_uwbaoa_lut_attribute_st lut_attribute;
  int16_t *data;          //corresponding to lut data
} stSingle_lut;

/**
 * @brief Structure for LUT file
 */
typedef struct
{
  stPdLutFileHeader_st LutFileHeader;
  stSingle_lut lut_storage[]; 
} stPdLutFile_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/**
 * @brief   Calculate 3D Angle of Arrival (AOA) using the M2 algorithm
 * @details This function calculates the 3D AOA by using phase differences between antenna pairs
 *          and lookup tables (LUT) to estimate azimuth and elevation angles. The M2 algorithm 
 *          provides improved accuracy by utilizing multiple antenna pairs.
 *
 * @param   AOA_PD      Pointer to structure containing compensated phase differences
 * @param   ant_attr    Pointer to antenna attributes structure containing antenna positions and type
 * @param   lut_attr    Pointer to LUT attributes structure containing reference data and parameters
 * @param   azi_result  Pointer to store the calculated azimuth angle in degrees
 * @param   ele_result  Pointer to store the calculated elevation angle in degrees
 * @return  CB_AOA_STATUS  EN_AOA_OK on success, EN_AOA_ERROR on invalid antenna type
 */
CB_AOA_STATUS cb_uwbaoa_lut_full3d( stAOA_CompensatedData* AOA_PD, const st_antenna_attribute_3d* ant_attr, const cb_uwbaoa_lut_attribute_st* lut_attr,
                                    float* azi_result, float* ele_result);
/**
 * @brief Compensate 3D AoA phase differences with antenna pair biases
 * @details This function compensates raw 3D phase difference data using specified bias values for each antenna pair.
 *          The biases account for systematic phase offsets between antenna pairs.
 * @param[in] pdoa_raw Raw 3D phase difference data containing measurements between antenna pairs
 * @param[in] pd01_bias Phase difference bias between antenna 0 and 1 (in degrees)
 * @param[in] pd02_bias Phase difference bias between antenna 0 and 2 (in degrees)
 * @param[in] pd12_bias Phase difference bias between antenna 1 and 2 (in degrees)
 * @return stAOA_CompensatedData Structure containing the bias-compensated phase differences
 */
stAOA_CompensatedData cb_uwbaoa_pdoa_biascomp(cb_uwbsystem_pdoa_3ddata_st pdoa_raw, float pd01_bias, float pd02_bias, float pd12_bias);

/**
 * @brief Calculate 2D Angle of Arrival (AOA) using lookup table method
 * @details This function calculates the 2D AOA by using phase differences and lookup tables (LUT)
 *          to estimate azimuth angle at a given elevation angle.
 *
 * @param[in] pd_azi Pointer to phase difference for azimuth calculation
 * @param[in] ele_ref Pointer to reference elevation angle in degrees
 * @param[in] ant_attr Pointer to 2D antenna attributes structure containing antenna positions and type
 * @param[in] lut_attr Pointer to LUT attributes structure containing reference data and parameters
 * @param[out] azi_result Pointer to store the calculated azimuth angle in degrees
 * @return CB_AOA_STATUS  EN_AOA_OK on success, EN_AOA_ERROR on invalid parameters
 */
CB_AOA_STATUS cb_uwbaoa_lut_full2d( float* pd_azi, float* ele_ref, const st_antenna_attribute_2d* ant_attr, const cb_uwbaoa_lut_attribute_st* lut_attr, float* azi_result);
/**
 * @brief Judge if the AOA is out of FOV (Field of View)
 * @details This function determines if the calculated Angle of Arrival (AOA) falls outside the 
 *          defined Field of View (FOV) by comparing the compensated phase differences against 
 *          the FOV boundaries defined in the lookup tables. Only works for antenna type 0 
 *          (A at top, B and C at bottom) and type 2 (A and C at top, B at bottom). All other 
 *          antenna types are treated as out-of-FOV.
 *
 *          Type 0:            Type 2:
 *             A               A     C
 *          B     C               B
 *
 * @param[in] fov_list Pointer to array containing FOV boundary definitions
 * @param[in] ant_attr Pointer to antenna attributes structure containing antenna positions and type
 * @param[in] LUT_attr Pointer to LUT attributes structure containing reference data and parameters
 * @param[in] AOA_PD Pointer to structure containing compensated phase differences between antenna pairs
 * @return uint8_t Returns 1 if AOA is outside FOV or unsupported antenna type, 0 if within FOV
 */
uint8_t cb_uwbaoa_detect_angle_inversion(const float* fov_list, const st_antenna_attribute_3d* ant_attr, const cb_uwbaoa_fov_attribute_st* FOV_attr, stAOA_CompensatedData* AOA_PD);
#endif /*_CB_AOA_H*/
