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

typedef enum
{
  EN_ANT_GEO_RightTriangle,       /**< Right Angle */
  EN_ANT_GEO_2D,                  /**< 2D */
  EN_ANT_GEO_IsocelesTriangle,    /**< Isoceles */
} en_SpecificAntennaGeometry;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

typedef struct
{  
  float azimuth_bias;
  float elevation_bias;
} stAOA_BiasData;

/**
 * @brief Structure for calculated elevation and azimuth phase differences.
 */
typedef struct
{  
  float azimuth;
  float elevation;
  float pdResidue[3];
} __attribute__((aligned(4))) stAOA_CompensatedData;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/**
 * @brief Calculate the 3D Angle of Arrival (AoA) based on phase differences and receiver geometry.
 * @details This function computes the 3D angle of arrival by calculating both azimuth and elevation angles using phase differences
 *          between signals received at multiple receivers. The receiver geometry, including height and width between the receivers,
 *          is considered during the calculation.
 * @param[in] PhaseDiff_readings Pointer to a stPDOA_Data structure containing phase differences between various pairs of receivers.
 * @param[out] azi_result Pointer to a float where the calculated azimuth angle (in degrees) will be stored.
 * @param[out] ele_result Pointer to a float where the calculated elevation angle (in degrees) will be stored.
 * @return CB_AOA_STATUS Status of the AoA calculation.
 *         - EN_AOA_OK: Calculation was successful.
 *         - EN_AOA_ERROR: Error occurred during calculation.
 */
CB_AOA_STATUS cb_uwbaoa_antenna_0_0_3d(stAOA_CompensatedData* AOA_PD, float* azi_result, float* ele_result);

/**
 * @brief Compensate 3D AoA phase difference with azimuth and elevation biases.
 * @details This function compensates 3D phase difference data using specified azimuth and elevation phase difference biases.
 * @param[in] pdoa_raw Raw 3D phase difference data.
 * @param[in] azi_pd_bias The azimuth phase difference bias (in degrees).
 * @param[in] ele_pd_bias The elevation phase difference bias (in degrees).
 * @return The compensated 3D AoA data, including both azimuth and elevation angles.
 */
stAOA_CompensatedData cb_uwbaoa_antenna_0_0_3d_biascomp(cb_uwbsystem_pdoa_3ddata_st pdoa_raw, float azi_pd_bias, float ele_pd_bias);

CB_AOA_STATUS cb_uwbaoa_antenna_2_0_3d_a(stAOA_CompensatedData* AOA_PD, float* azi_result, float* ele_result);
stAOA_CompensatedData cb_uwbaoa_antenna_2_0_3d_biascomp(cb_uwbsystem_pdoa_3ddata_st pdoa_raw, float azi_pd_bias, float ele_pd_bias);

#endif /*_CB_AOA_H*/
