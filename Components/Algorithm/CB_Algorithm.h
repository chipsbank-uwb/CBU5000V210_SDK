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
// CONFIGURATION SECTION
//-------------------------------

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

typedef enum {
  EN_PDOA_2D_CALTYPE = 0, 
  EN_PDOA_3D_CALTYPE = 1
} enUwbPdoaCalType;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
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
cb_uwbsystem_rx_tsutimestamp_st cb_uwbalg_ge_trx_tsu(cb_uwbsystem_rx_tsustatus_st* p_rxTsuStatus, cb_uwbsystem_rx_cir_iqdata_st* p_cirRegisterData, uint16_t cirCtlIdx);

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
double cb_uwbalg_prop_calculation(cb_uwbsystem_rangingtroundtreply_st* result1, cb_uwbsystem_rangingtroundtreply_st* result2);

/**
 * @brief Performs CIR (Channel Impulse Response) ranging.
 *
 * This function calculates the CIR ranging parameters using CIR data.
 *
 * @param p_rx_ranging Pointer to the structure where ranging parameters will be stored.
 * @param p_cirRegisterData Pointer to the array containing the CIR data.
 */
double cb_uwbalg_cir_ranging (cb_uwbsystem_rx_rangingparam_st* p_rx_ranging, cb_uwbsystem_rx_cir_iqdata_st* p_cirRegisterData, uint16_t cirCtlIdx);

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
uint8_t cb_uwbalg_cir_quality_check(cb_uwbsystem_rx_cir_iqdata_st* p_cirRegisterData);

/**
  * 
  * @brief Computes the Phase Difference of Arrival (PDOA) using the Cordic algorithm.
  * @details This function computes the Phase Difference of Arrival (PDOA) between two UWB receiver
  * antennas using the Cordic algorithm. It takes the y and x components of the vector representing
  * the phase difference, along with the interpolation factor, as input.
  * @param y The y-component of the vector representing the phase difference.
  * @param x The x-component of the vector representing the phase difference.
  * @param inter The interpolation factor for Cordic algorithm.
  * @return The computed PDOA value in radian.
  */

double cb_uwbalg_pdoa_cordic_vector(int32_t y, int32_t x, uint8_t inter);

/**
  * @brief Estimates the Phase Difference of Arrival (PDOA) between two UWB receiver antennas.
  * @details This function estimates the Phase Difference of Arrival (PDOA) between two UWB receiver
  * antennas using the phase information from received signals. It takes the phase angles and amplitude
  * ratio of the two antennas, along with the packet count, as input.
  * @param poa_deg1 The phase angle of the first antenna in degrees.
  * @param poa_deg2 The phase angle of the second antenna in degrees.
  * @return The estimated PDOA value in degrees.
  */
double cb_uwbalg_pdoa_estimation(double poa_deg1, double poa_deg2);

/**
  * 
  * @brief Computes the magnitude of complex IQ data and finds the index corresponding to the maximum magnitude.
  * @details This function takes an array of complex IQ data and its length as input, computes the magnitude of 
  * each IQ pair, and finds the index corresponding to the maximum magnitude.
  * @param Data Pointer to an array of complex IQ data.
  * @param NumDataSet The number of elements in the array.
  * @return The index corresponding to the maximum magnitude in the array.
  */
uint32_t cb_uwbalg_pdoa_find_max_mag_index(cb_uwbsystem_rx_cir_iqdata_st* Data,uint32_t NumDataSet);

/**
  * @brief Performs post-processing on the received Channel Impulse Response (CIR) data.
  * @details This function performs post-processing on the received Channel Impulse Response (CIR)
  * data to extract relevant information for PDOA calculation.
  * @return A structure containing the processed results of Channel Impulse Response (CIR) data.
  */
cb_uwbalg_poa_outputperpacket_st cb_uwbalg_pdoa_cir_post_processing(enUwbPdoaCalType CIR_CalculationType, uint8_t PackageNum, const uint8_t numRxUsed, const cb_uwbsystem_rx_cir_iqdata_st* cirRegisterData, uint16_t cirDataSize);

#endif /*__CB_ALGORITHM_H*/
