/**
 * @file    ftm_uwb_cal.h
 * @brief   UWB Calibration Module Header
 * @details This header file contains function prototypes and data structures related to factory calibration.
 * @author  Chipsbank
 * @date    2024
 */
 
#ifndef __FTM_UWB_CAL_H_
#define __FTM_UWB_CAL_H_

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Common.h"
//#include "AppUwbCommTrx.h"

#include "ftm_cal_nvm.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define CB_RADAR_DEBUG CB_TRUE

//-------------------------------
// ENUM SECTION
//-------------------------------
/*typedef enum {
  EN_CAL_OK = 0,
  EN_CAL_FAILED = 1,
} enCALReturnCode;*/

typedef enum {
  Tx = 0,
  Rx = 1,
  RNGAOATx = 2,
  RNGAOARx = 3,
  Close = 4,
} enCALOperationMode;

typedef enum {
  enSingleAntenna1_RX = 0x01,
  enSingleAntenna2_RX = 0x02,
  enSingleAntenna3_RX = 0x04,
  enDualAntenna1_2_RX = 0x03,
  enDualAntenna2_3_RX = 0x06,
  enDualAntenna1_3_RX = 0x05,
  enTripleAntenna1_2_3_RX = 0x07,
} enCALRxChannel;

typedef enum {
  Responder,
  Initiator,
} enCALRNGAOAID;

typedef enum {
  On = 1,
  Off = 0,
} enSwtich;

typedef enum {
  EN_CAL_IRQ_RX0_DONE,
} enCALIrqNotify;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

typedef struct {
  int16_t dis;
  int16_t aoah;
  int16_t aoav;
  int16_t pdoah;
  int16_t pdoav;
  int16_t avg_rsl1;
  int16_t avg_rsl2;
  int16_t avg_rsl3;
} stRNGAOARx;


//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

enCALReturnCode ftm_uwb_cal_set_tx_packets(uint32_t TxNumOfPacket);
enCALReturnCode ftm_uwb_cal_set_tx_interval(uint16_t TxInterval);
enCALReturnCode ftm_uwb_cal_set_tx_onoff(enSwtich status);
enCALReturnCode ftm_uwb_cal_set_rx_channel(enCALRxChannel RxChannel);
enCALReturnCode ftm_uwb_cal_set_rx_onoff(enSwtich status);
enCALReturnCode ftm_uwb_cal_get_rx_packets(uint32_t* RxNumOfPacket);
enCALReturnCode ftm_cal_rngaoa_SetID(uint32_t RNGAOAID);
enCALReturnCode ftm_cal_rngaoa_ReadID(uint32_t* RNGAOAID);
enCALReturnCode ftm_cal_rngaoa_SetFreq(uint8_t RNGAOAFreq);
enCALReturnCode ftm_cal_rngaoa_ReadFreq(uint8_t* RNGAOAFreq);
enCALReturnCode ftm_uwb_cal_set_rngaoa_tx_onoff(enSwtich status);
enCALReturnCode ftm_uwb_cal_set_rngaoa_rx_channel(enCALRxChannel RxChannel);
enCALReturnCode ftm_uwb_cal_set_rngaoa_rx_onoff(enSwtich status);
enCALReturnCode ftm_uwb_cal_get_rngaoa_rx_resulf(stRNGAOARx* stRNGAOARxDataContainer);

void ftm_uwb_cal_periodic_tx(void);
void ftm_uwb_cal_periodic_tx_Stop(void);
void ftm_uwb_cal_periodic_tx_done_irq_unlimited_callback(void);
void ftm_uwb_cal_periodic_tx_done_irq_callback(void);
void ftm_uwb_cal_comm_rx(void);
void ftm_uwb_cal_comm_rx_Stop(void);
void ftm_uwb_cal_comm_rx_dene_irq_calllback(void);

void ftm_uwb_cal_rngaoa_task_polling();
#endif /**__FTM_UWB_CAL_H_*/
