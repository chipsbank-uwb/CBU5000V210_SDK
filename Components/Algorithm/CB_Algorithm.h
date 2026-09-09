/**
 * @file    CB_Algorithm.h
 * @brief   Contains algorithm functions
 * @details This file contains algorithm functions
 * @author  Chipsbank
 * @date    2024
 */

#ifndef __CB_ALGORITHM_H
#define __CB_ALGORITHM_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_system_types.h"
#include <stdint.h>

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define CIR_REGISTER_256_SAMPLES_SIZE   256

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct
{  
  volatile double rx0;
  volatile double rx1;
  volatile double rx2;
}cb_uwbalg_poa_outputperpacket_st;

// AOA STRUCTS
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
// END AOA STRUCTS

//-------------------------------
// ENUM SECTION
//-------------------------------
typedef enum {
  EN_PDOA_2D_CALTYPE = 0, 
  EN_PDOA_3D_CALTYPE = 1
} enUwbPdoaCalType;

typedef enum
{
  EN_ALG_OK     = 0x00,   /**< Result Ok */
  EN_ALG_ERROR  = 0x01,   /**< Result Error */
} CB_ALG_STATUS;

// AOA ENUMS
typedef enum
{
  EN_AOA_OK     = 0x00,   /**< Result Ok */
  EN_AOA_ERROR  = 0x01,   /**< Result Error */
} CB_AOA_STATUS;
// END AOA ENUMS

//-----------------------------------------
// Typedefs for function pointer signatures
//-----------------------------------------
/**
 * @brief Calculates the receiver timestamp using the TSU (Timestamp Unit) status and CIR (Channel Impulse Response) data.
 *
 * This function computes the receiver timestamp by combining the SFD (Start of Frame Delimiter) timestamp,
 * offset, and buffer sample values with CIR data and synchronization indices. It uses these values to 
 * generate a precise timestamp of when the receiver detected the signal.
 *
 * @param[in]  p_rxTsuStatus          Pointer to a `cb_uwbsystem_rx_tsustatus_st` structure containing the receiver's TSU status information.
 * @param[in]  p_cirRegisterData     Pointer to an array of CIR register data.
 *
 * @return     The computed receiver timestamp as a `cb_uwbsystem_rx_tsutimestamp_st` value. This timestamp represents the time 
 *             when the receiver detected the signal, adjusted by the provided CIR data and synchronization indices.
 */
typedef cb_uwbsystem_rx_tsutimestamp_st
        (*uwbalg_get_rx_tsu_fn)(cb_uwbsystem_rx_tsustatus_st* p_rxTsuStatus,
                          cb_uwbsystem_rx_cir_iqdata_st* p_cirRegisterData,
                          uint16_t cirCtlIdx);

/**
 * @brief Check the quality of the Channel Impulse Response (CIR).
 *
 * This function evaluates the quality of the CIR by analyzing the in-phase (I) and 
 * quadrature (Q) register data. It computes the magnitude of the CIR values, calculates 
 * noise thresholds, and checks for leading edges, peak shapes, and multipath environments 
 * to determine the overall CIR quality.

 * @param p_cirRegisterData Pointer to an array containing the CIR register data.
 * 
 * @return A quality flag indicating the quality of the CIR, where lower values indicate better quality.
 */
typedef uint8_t (*uwbalg_cir_quality_check_fn)(cb_uwbsystem_rx_cir_iqdata_st*);

/**
 * @brief Performs CIR (Channel Impulse Response) ranging.
 *
 * This function calculates the CIR ranging parameters using CIR data.
 *
 * @param p_rx_ranging Pointer to the structure where ranging parameters will be stored.
 * @param p_cirRegisterData Pointer to the array containing the CIR data.
 */
typedef double  (*uwbalg_cir_ranging_fn)(cb_uwbsystem_rx_rangingparam_st*,
                                  cb_uwbsystem_rx_cir_iqdata_st*,
                                  uint16_t);

/**
  * @brief Performs post-processing on the received Channel Impulse Response (CIR) data.
  * @details This function performs post-processing on the received Channel Impulse Response (CIR)
  * data to extract relevant information for PDOA calculation.
  * @return A structure containing the processed results of Channel Impulse Response (CIR) data.
  */
typedef cb_uwbalg_poa_outputperpacket_st
        (*uwbalg_cir_pdoa_postproc_fn)(enUwbPdoaCalType       CIR_CalculationType,
                                uint8_t                PackageNum,
                                const uint8_t          numRxUsed,
                                const cb_uwbsystem_rx_cir_iqdata_st* cirRegisterData,
                                uint16_t               cirDataSize);

/**
 * @brief Calculates a specific property based on two sets of ranging results.
 *
 * This function performs a calculation using the round-trip and reply times from two different sets of ranging 
 * results. The formula used in the calculation involves multiplying and subtracting the round-trip and reply 
 * times from both results, and then dividing by the sum of these times. The result is a double-precision 
 * floating-point value.
 *
 * @param[in]  result1  Pointer to the first `cb_uwbsystem_rangingtroundtreply_st` structure containing the first set of
 *                      ranging results. It should include values for T_round and T_reply.
 * @param[in]  result2  Pointer to the second `cb_uwbsystem_rangingtroundtreply_st` structure containing the second set of
 *                      ranging results. It should also include values for T_round and T_reply.
 *
 * @return The calculated property as a double-precision floating-point value.
 */                                
typedef double
        (*uwbalg_prop_calculation_fn)(cb_uwbsystem_rangingtroundtreply_st* result1,
                               cb_uwbsystem_rangingtroundtreply_st* result2);

/**
  * @brief Estimates the Phase Difference of Arrival (PDOA) between two UWB receiver antennas.
  * @details This function estimates the Phase Difference of Arrival (PDOA) between two UWB receiver
  * antennas using the phase information from received signals. It takes the phase angles and amplitude
  * ratio of the two antennas, along with the packet count, as input.
  * @param poa_deg1 The phase angle of the first antenna in degrees.
  * @param poa_deg2 The phase angle of the second antenna in degrees.
  * @return The estimated PDOA value in degrees.
  */
typedef double
        (*uwbalg_pdoa_estimation_fn)(double poa_deg1, double poa_deg2);

/**
 * @brief Initialize the quality flag model interpreter.
 *
 * This function loads either the quantized (int8) or float32 version of the
 * quality model depending on the build flag. It sets up the operator
 * resolver, allocates tensor arenas if initialization succeeds.
 *
 * @note Must be called once before invoking cb_uwbalg_tf_quality_flag_check().
 *
 * @return CB_ALG_STATUS  EN_ALG_OK on success, EN_ALG_ERROR failed initialized
 */
typedef CB_ALG_STATUS
        (*uwbalg_tf_quality_flag_init_fn)(void);

/**
 * @brief Run quality flag inference on feature vector.
 *
 * This function checks that the interpreter has been successfully
 * initialized, validates input/output tensors, loads the 23-element
 * feature vector into the model, runs inference, and output the
 * predicted class index (0-3).
 * @param[out] result Pointer to Predicted class index in range [0,3] on success.
 * @return CB_ALG_STATUS  EN_ALG_OK on success, EN_ALG_ERROR failed invoking
 */
typedef CB_ALG_STATUS
        (*uwbalg_tf_quality_flag_check_fn)(uint8_t* result);

/**
 * @brief Evaluate similarity between STS CIR and RX CIR.
 *
 * @return uint8_t 
 *         - CB_LIB_TRUE (1): Pass
 *         - CB_LIB_FALSE (0): Fail
 */
typedef uint8_t
        (*uwbalg_sts_evaluation_fn)(void);

// AOA
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
typedef CB_AOA_STATUS
(*uwbalg_aoa_lut_full3d_fn)(stAOA_CompensatedData* AOA_PD,
                     const st_antenna_attribute_3d* ant_attr,
                     const cb_uwbaoa_lut_attribute_st* lut_attr,
                     float* azi_result, float* ele_result);

/**
 * @brief   HS-AOA 3D LUT estimator (1-NN in PDOA space + two-neighbor interpolation).
 * @param   AOA_PD      Compensated phase differences (PD01/02/12), degrees
 * @param   lut_attr    Grid metadata + lut_data (size_col >= 3)
 * @param   azi_result  Estimated azimuth (deg)
 * @param   ele_result  Estimated elevation (deg)
 * @return  EN_AOA_OK on success, EN_AOA_ERROR on invalid parameters
 */
typedef CB_AOA_STATUS 
(*uwbalg_aoa_lut_hs_fn)(stAOA_CompensatedData* AOA_PD,
                const cb_uwbaoa_lut_attribute_st* lut_attr,
                float* azi_result,
                float* ele_result);       
                     
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
typedef stAOA_CompensatedData
(*uwbalg_aoa_pdoa_biascomp_fn)(cb_uwbsystem_pdoa_3ddata_st pdoa_raw,
                        float pd01_bias, float pd02_bias, float pd12_bias);

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
typedef CB_AOA_STATUS
(*uwbalg_aoa_lut_full2d_fn)(float* pd_azi, float* ele_ref,
                     const st_antenna_attribute_2d* ant_attr,
                     const cb_uwbaoa_lut_attribute_st* lut_attr,
                     float* azi_result);

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
typedef uint8_t
(*uwbalg_aoa_detect_angle_inversion_fn)(const float* fov_list,
                                 const st_antenna_attribute_3d* ant_attr,
                                 const cb_uwbaoa_fov_attribute_st* FOV_attr,
                                 stAOA_CompensatedData* AOA_PD);

/**
 * @brief Simple wrapper for arm_cfft_f32 with FFT size selection.
 *
 * @param[in]     fft_len      FFT length
 * @param[in,out] pSrc         Pointer to complex data buffer (interleaved real/imag)
 * @param[in]     ifftFlag     IFFT flag (0=FFT, 1=IFFT)
 * @param[in]     doBitReverse Bit reverse flag (0=no, 1=yes)
 */
// RADAR
typedef void
        (*uwbalg_fft_fn)(cb_uwbradar_en fft_len, float* pSrc,
                  uint8_t ifftFlag, uint8_t doBitReverse);

cb_uwbsystem_rx_tsutimestamp_st cb_uwbalg_get_rx_tsu_lemm(cb_uwbsystem_rx_tsustatus_st* p_rxTsuStatus, cb_uwbsystem_rx_cir_iqdata_st* p_cirRegisterData, uint16_t cirCtlIdx, int scale_factor, int le_noise_th_ip, int mm_option);

        
//-----------------------------------------
// ALG function pointer
//-----------------------------------------
uwbalg_get_rx_tsu_fn            cb_getfn_uwbalg_get_rx_tsu                  (void);
uwbalg_cir_quality_check_fn      cb_getfn_uwbalg_cir_quality_check            (void);
uwbalg_cir_ranging_fn            cb_getfn_uwbalg_cir_ranging                  (void);
uwbalg_cir_pdoa_postproc_fn      cb_getfn_uwbalg_cir_pdoa_cir_post_processing (void);
uwbalg_prop_calculation_fn       cb_getfn_uwbalg_prop_calculation             (void);
uwbalg_pdoa_estimation_fn        cb_getfn_uwbalg_pdoa_estimation              (void);
uwbalg_tf_quality_flag_init_fn   cb_getfn_uwbalg_tf_quality_flag_init         (void);
uwbalg_tf_quality_flag_check_fn  cb_getfn_uwbalg_tf_quality_flag_check        (void);
uwbalg_sts_evaluation_fn         cb_getfn_uwbalg_sts_evaluation               (void);

uwbalg_aoa_lut_full2d_fn              cb_getfn_uwbalg_aoa_lut_full2d             (void);
uwbalg_aoa_lut_full3d_fn              cb_getfn_uwbalg_aoa_lut_full3d             (void);
uwbalg_aoa_pdoa_biascomp_fn           cb_getfn_uwbalg_aoa_pdoa_biascomp          (void);

uwbalg_aoa_detect_angle_inversion_fn  cb_getfn_uwbalg_aoa_detect_angle_inversion (void);
uwbalg_fft_fn   cb_getfn_uwbalg_fft (void);

#endif /*__CB_ALGORITHM_H*/
