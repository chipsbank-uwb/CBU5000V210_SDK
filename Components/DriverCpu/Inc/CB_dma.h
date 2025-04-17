/**
 * @file    CB_dma.h
 * @brief   Header file for DMA communication module.
 * @details This file defines structures, constants, and function prototypes for the DMA.
 *          Function prototypes are provided for initializing DMA, reconfiguring DMA settings, 
 *          transmitting data via DMA, and handling DMA interrupts.
 * @author  Chipsbank
 * @date    2024
 */
#ifndef INC_DMA_H_
#define INC_DMA_H_

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_Common.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------

typedef enum
{
  EN_DMA_CHANNEL_0 = 0,
  EN_DMA_CHANNEL_1 = 1, 
  EN_DMA_CHANNEL_2 = 2
} enDMAChannel;

typedef enum
{
  EN_DMA_IRQ_DISABLE = 0,
  EN_DMA_IRQ_ENABLE = 1,
} enDMAIRQ;

typedef enum
{
  EN_DMA_SRC_ADDR_INC_DISABLE = 0,
  EN_DMA_SRC_ADDR_INC_ENABLE = 1,
} enDMASrcInc;


typedef enum 
{
  EN_DMA_DEST_ADDR_INC_DISABLE = 0,
  EN_DMA_DEST_ADDR_INC_ENABLE = 1
} enDMADestInc;

typedef enum 
{
  EN_DMA_DATA_WIDTH_BYTE = 0, // 8 bit
  EN_DMA_DATA_WIDTH_HWORD = 1, // 16 bit
  EN_DMA_DATA_WIDTH_WORD = 2 // 32 bit
} enDMADataWidth;

typedef enum 
{
  EN_DMA_CONTINUOUS_MODE_DISABLE = 0,
  EN_DMA_CONTINUOUS_MODE_ENABLE = 1 // transfer size ignored when continuous mode enabled
} enDMAContinuousMode;

typedef enum
{
  EN_DMA_TRANSFER_TYPE_MULTI = 0,   // transfer will not stop as long as flow control is asserted
  EN_DMA_TRANSFER_TYPE_SINGLE = 1   // transfer will start on positive edge of flow control enable, then stop after 1 data transfered
} enDMATransferType;

// useful if Source and Destination is of two different peripheral/address type
// or if user is operating in LLi (Linked-list) mode, where different LLi packet
// require different flow-control
typedef enum 
{
  EN_DMA_FLOW_CONTROL_SRC_DISABLE = 0,
  EN_DMA_FLOW_CONTROL_SRC_SEL_0   = 1,
  EN_DMA_FLOW_CONTROL_SRC_SEL_1   = 2,
  EN_DMA_FLOW_CONTROL_SRC_SEL_2   = 3,
  EN_DMA_FLOW_CONTROL_SRC_SEL_3   = 4,
  EN_DMA_FLOW_CONTROL_SRC_SEL_4   = 5,
  EN_DMA_FLOW_CONTROL_SRC_SEL_5   = 6,
  EN_DMA_FLOW_CONTROL_SRC_SEL_6   = 7,
  EN_DMA_FLOW_CONTROL_SRC_SEL_7   = 8
} enDMAFlowControlSrc;

typedef enum
{
  EN_DMA_FLOW_CONTROL_DEST_DISABLE = 0,
  EN_DMA_FLOW_CONTROL_DEST_SEL_0   = 1,
  EN_DMA_FLOW_CONTROL_DEST_SEL_1   = 2,
  EN_DMA_FLOW_CONTROL_DEST_SEL_2   = 3,
  EN_DMA_FLOW_CONTROL_DEST_SEL_3   = 4,
  EN_DMA_FLOW_CONTROL_DEST_SEL_4   = 5,
  EN_DMA_FLOW_CONTROL_DEST_SEL_5   = 6,
  EN_DMA_FLOW_CONTROL_DEST_SEL_6   = 7,
  EN_DMA_FLOW_CONTROL_DEST_SEL_7   = 8
} enDMAFlowControlDest;

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef struct {
  enDMAChannel   DMAChannel;
  enDMAIRQ       IRQEnable;
  
  uint32_t      SrcAddr;
  uint32_t      DestAddr;

  uint32_t      DataLen;

  enDMADataWidth  DataWidth;

  enDMASrcInc   SrcInc;    // enable/disable source address incrementation, based on source data width
  enDMADestInc  DestInc;   // enable/disable destination address incrementation, based on destnation data width

  enDMAContinuousMode ContinuousMode;   // only take effect when both SrcInc and DestInc is disabled

  // only on V3
  enDMATransferType     TransferType;

  enDMAFlowControlSrc   FlowControlSrc;
  enDMAFlowControlDest  FlowControlDest;
} stDMAConfig;

struct stDMALinkedListNode {
  stDMAConfig DMAConfig;      // need this
  uint32_t LLiConfig[5];  // internally edited
  struct stDMALinkedListNode *NextNode; // need this
};

struct stDMALinkedListHead {
  stDMAConfig DMAConfig;
  struct stDMALinkedListNode *NextNode;
};

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void cb_dma_irqhandler(void);
void cb_dma_app_irq_callback(void);

void cb_dma_channel_0_app_irq_callback(void);
void cb_dma_channel_0_mid_irq_callback(void);
void cb_dma_channel_1_app_irq_callback(void);
void cb_dma_channel_1_mid_irq_callback(void);
void cb_dma_channel_2_app_irq_callback(void);
void cb_dma_channel_2_mid_irq_callback(void);

/**
 * @brief Initialize DMA module.
 * @details This function turns on the DMA enable bits and enable DMA's interrupts system-wise.
 */
void cb_dma_init(void);

/**
 * @brief Deinitialize DMA module.
 * @details This function turns off the DMA enable bits and disable DMA's interrupt system-wise
 */
void cb_dma_deinit(void);

/**
 * @brief Enables the DMA module.
 * @details This function enables the DMA module by setting the DMA enable bit in the DMA control register.
 */
void cb_dma_enable(void);

/**
 * @brief Disables the DMA module.
 * @details This function disables the DMA module by clearing the DMA enable bit in the DMA control register.
 */
void cb_dma_disable(void);

/**
 * @brief Initializes a DMA configuration structure with default values.
 * @details This function sets all members of the DMA configuration structure to their default values.
 * @param DMAConfig Pointer to the DMA configuration structure to be initialized.
 */
void cb_dma_struct_init(stDMAConfig *DMAConfig);

/**
 * @brief Configures the DMA channel based on the provided configuration.
 * @details This function sets up the DMA channel according to the provided configuration structure.
 *          It configures the interrupt, source and destination addresses, address increments, data width,
 *          transfer size, continuous mode, and flow control.
 * @param DMAConfig Pointer to the DMA configuration structure.
 */
void cb_dma_setup(stDMAConfig *DMAConfig);

/**
 * @brief Initializes the DMA linked list for a specified channel.
 * @details This function sets up the DMA linked list for a specified channel based on the provided configuration.
 *          It configures the DMA channel using the `cb_dma_setup` function and sets the next linked list item (LLI).
 * @param DMALLiConfig Pointer to the DMA linked list configuration structure.
 */
void cb_dma_lli_init(struct stDMALinkedListHead *DMALLiConfig);

/**
 * @brief Sets up a DMA linked list node with the provided configuration.
 * @details This function configures a DMA linked list node based on the provided configuration structure.
 *          It sets the source and destination addresses, address increments, data width, transfer size, and flow control.
 * @param DMALLiConfig Pointer to the DMA linked list head configuration structure.
 * @param LLiNode Pointer to the DMA linked list node to be configured.
 */
void cb_dma_lli_setup(struct stDMALinkedListHead *DMALLiConfig, struct stDMALinkedListNode *LLiNode);

/**
 * @brief Enables a specified DMA channel.
 * @details This function enables a specified DMA channel based on the provided configuration.
 *          It selects the appropriate DMA channel and sets the channel configuration to enable it.
 * @param DMAConfig Pointer to the DMA configuration structure.
 */
void cb_dma_enable_channel(stDMAConfig *DMAConfig);

/**
 * @brief Disables a specified DMA channel.
 * @details This function disables a specified DMA channel based on the provided configuration.
 *          It selects the appropriate DMA channel, clears pending DMA requests, and disables the channel.
 * @param DMAConfig Pointer to the DMA configuration structure.
 */
void cb_dma_disable_channel(stDMAConfig *DMAConfig);

/**
 * @brief Sets the source request for a specified DMA channel.
 * @details This function configures the source request for a specified DMA channel based on the provided configuration.
 *          It sets the appropriate bit in the DMA request register to enable the source request for the specified channel.
 * @param DMAConfig Pointer to the DMA configuration structure.
 */
void cb_dma_set_src_request(stDMAConfig *DMAConfig);

/**
 * @brief Clears the source request for a specified DMA channel.
 * @details This function clears the source request for a specified DMA channel based on the provided configuration.
 *          It clears the appropriate bit in the DMA request register to disable the source request for the specified channel.
 * @param DMAConfig Pointer to the DMA configuration structure.
 */
void cb_dma_clear_src_request(stDMAConfig *DMAConfig);

/**
 * @brief Sets the destination request for a specified DMA channel.
 * @details This function configures the destination request for a specified DMA channel based on the provided configuration.
 *          It sets the appropriate bit in the DMA request register to enable the destination request for the specified channel.
 * @param DMAConfig Pointer to the DMA configuration structure.
 */
void cb_dma_set_dest_request(stDMAConfig *DMAConfig);

/**
 * @brief Clears the destination request for a specified DMA channel.
 * @details This function clears the destination request for a specified DMA channel based on the provided configuration.
 *          It clears the appropriate bit in the DMA request register to disable the destination request for the specified channel.
 * @param DMAConfig Pointer to the DMA configuration structure.
 */
void cb_dma_clear_dest_request(stDMAConfig *DMAConfig);

/**
 * @brief Checks if a specified DMA channel is idle.
 * @details This function checks if a specified DMA channel is idle by examining the channel idle status register.
 * @param DMAConfig Pointer to the DMA configuration structure.
 * @return Returns CB_TRUE if the channel is idle, otherwise returns CB_FALSE.
 */
uint8_t cb_dma_is_channel_idle(stDMAConfig *DMAConfig);

/**
 * @brief Configures DMA interrupts.
 * @param DMA_INT Specifies the interrupt type (e.g., EN_DMA_INT_READY_CHANNEL_0, EN_DMA_INT_ERROR_CHANNEL_1).
 * @param NewState The new state for the interrupt (ENABLE to enable, DISABLE to disable).
 */
void cb_dma_int_config(uint32_t DMA_INT, uint8_t NewState);

/**
 * @brief   Get and clear the status of DMA interrupts.
 * @return  The status of the specified DMA interrupts. Each bit corresponds to the status of one interrupt source.
 */
uint32_t cb_dma_get_and_clear_it_status(void);

/**
 * @brief   Clear all DMA requests for a specified channel.
 * @param[in] Channel Specifies the DMA channel to clear requests for. Use the following values:
 * @return  None
 */
void cb_dma_clear_request(enDMAChannel Channel);

#endif /*INC_DMA_H_*/
