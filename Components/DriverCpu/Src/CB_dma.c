/**
 * @file    CB_dma.c
 * @brief   DMA driver implementation.
 * @details This file contains the implementation for DMA driver.
 * @author  Chipsbank
 * @date    2024
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "CB_dma.h"
#include "CB_scr.h"
#include "string.h"
#include "CB_PeripheralPhyAddrDataBase.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define BYTE_SIZE 8
#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-variable"
#endif
//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static stDMA_TypeDef *pDMA = (stDMA_TypeDef* )DMA_BASE_ADDR;
static stDMAChannel_TypeDef *pDMA_CHANNEL0 = (stDMAChannel_TypeDef* )DMA_CHANNEL0_BASE_ADDR;
static stDMAChannel_TypeDef *pDMA_CHANNEL1 = (stDMAChannel_TypeDef* )DMA_CHANNEL1_BASE_ADDR;
static stDMAChannel_TypeDef *pDMA_CHANNEL2 = (stDMAChannel_TypeDef* )DMA_CHANNEL2_BASE_ADDR;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void cb_dma_channel_0_irq_callback(void);
void cb_dma_channel_1_irq_callback(void);
void cb_dma_channel_2_irq_callback(void);
void cb_dma_irq_callback(void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief   DMA interrupt handler.
 * @details This function handles DMA interrupts.
 */
void cb_dma_irqhandler(void)
{
    uint32_t irq_status = pDMA->irq_status;

    if ((irq_status & DMA_CH_READY_IRQ_CH_0_MSK) == DMA_CH_READY_IRQ_CH_0)
    {
    pDMA->dma_req_reg &= ~(0x000000FFUL);   // clear all channel 0 DMA requests
    cb_dma_channel_0_irq_callback();
    } 

    if ((irq_status & DMA_CH_READY_IRQ_CH_1_MSK) == DMA_CH_READY_IRQ_CH_1)
    {
    pDMA->dma_req_reg &= ~(0x0000FF00UL);   // clear all channel 1 DMA requests
    cb_dma_channel_1_irq_callback();
    }

    if ((irq_status & DMA_CH_READY_IRQ_CH_2_MSK) == DMA_CH_READY_IRQ_CH_2)
    {
    pDMA->dma_req_reg &= ~(0x00FF0000UL);   // clear all channel 2 DMA requests
    cb_dma_channel_2_irq_callback();
    }

    cb_dma_irq_callback();
}

/**
 * @brief Callback function for DMA channel 0 interrupt.
 */void cb_dma_channel_0_irq_callback(void)
{
    cb_dma_channel_0_app_irq_callback();
}

/**
 * @brief Weak implementation of the application-specific callback for DMA channel 0.
 */
__WEAK void cb_dma_channel_0_app_irq_callback(void)
{
}

/**
 * @brief Callback function for DMA channel 1 interrupt.
 */
void cb_dma_channel_1_irq_callback(void)
{
    cb_dma_channel_1_app_irq_callback();
}

/**
 * @brief Weak implementation of the application-specific callback for DMA channel 1.
 */
__WEAK void cb_dma_channel_1_app_irq_callback(void)
{
}

/**
 * @brief Callback function for DMA channel 2 interrupt.
 */
void cb_dma_channel_2_irq_callback(void)
{
    cb_dma_channel_2_app_irq_callback();
}

/**
 * @brief Weak implementation of the application-specific callback for DMA channel 2.
 */__WEAK void cb_dma_channel_2_app_irq_callback(void)
{
}

/**
 * @brief General DMA interrupt callback.
 */
void cb_dma_irq_callback(void)
{
    cb_dma_app_irq_callback();
}

/**
 * @brief Weak implementation of the application-specific callback for general DMA interrupts.
 */
__WEAK void cb_dma_app_irq_callback(void)
{
}


/**
 * @brief Initialize DMA module.
 * @details This function turns on the DMA enable bits and enable DMA's interrupts system-wise.
 */
void cb_dma_init(void)
{
    /*DMA Module Power On and Reset Release*/
    cb_scr_dma_module_on();

    /*Enable DMA Module*/
    pDMA->dma_en |= DMA_ENABLE;

    /*Clear DMA Request*/
    pDMA->dma_req_reg = 0x00000000UL;

    /*NVIC IRQ Enable */
    NVIC_EnableIRQ(DMA_IRQn);
}

/**
 * @brief Deinitialize DMA module.
 * @details This function turns off the DMA enable bits and disable DMA's interrupt system-wise
 */
void cb_dma_deinit(void)
{
    /*Clear DMA Request*/
    pDMA->dma_req_reg = 0x00000000UL;

    /*NVIC IRQ Disable */
    NVIC_DisableIRQ(DMA_IRQn);

    /*Disable CRC Module*/
    pDMA->dma_en &= ~(DMA_ENABLE);

    /*Disable DMA Module*/
    cb_scr_dma_module_off();
}

/**
 * @brief Enables the DMA module.
 * @details This function enables the DMA module by setting the DMA enable bit in the DMA control register.
 */
void cb_dma_enable(void)
{
    /*Enable DMA Module*/
    pDMA->dma_en |= DMA_ENABLE;
}


/**
 * @brief Disables the DMA module.
 * @details This function disables the DMA module by clearing the DMA enable bit in the DMA control register.
 */
void cb_dma_disable(void)
{
    /*Enable DMA Module*/
    pDMA->dma_en &= ~(DMA_ENABLE);
}


/**
 * @brief Initializes a DMA configuration structure with default values.
 * @details This function sets all members of the DMA configuration structure to their default values.
 * @param DMAConfig Pointer to the DMA configuration structure to be initialized.
 */
void cb_dma_struct_init(stDMAConfig *DMAConfig)
{
    if (DMAConfig == NULL)
    {
        return; // Return if the input pointer is NULL
    }

    // Initialize each member of the DMA configuration structure
    DMAConfig->DMAChannel     = EN_DMA_CHANNEL_0;    // Default to channel 0
    DMAConfig->IRQEnable      = EN_DMA_IRQ_ENABLE;     // Enable IRQ by default
    DMAConfig->SrcAddr        = 0;                     // Source address set to 0
    DMAConfig->DestAddr       = 0;                     // Destination address set to 0
    DMAConfig->DataLen        = 0;                     // Data length set to 0
    DMAConfig->DataWidth      = EN_DMA_DATA_WIDTH_WORD; // Data width set to word (32 bits)
    DMAConfig->SrcInc         = EN_DMA_SRC_ADDR_INC_ENABLE; // Source address increment enabled
    DMAConfig->DestInc        = EN_DMA_DEST_ADDR_INC_ENABLE; // Destination address increment enabled
    DMAConfig->ContinuousMode = EN_DMA_CONTINUOUS_MODE_DISABLE; // Continuous mode disabled
    DMAConfig->TransferType   = EN_DMA_TRANSFER_TYPE_MULTI; // Multi transfer type
    DMAConfig->FlowControlSrc = EN_DMA_FLOW_CONTROL_SRC_SEL_0; // Flow control source set to 0
    DMAConfig->FlowControlDest = EN_DMA_FLOW_CONTROL_DEST_SEL_1; // Flow control destination set to 1
}



/**
 * @brief Configures the DMA channel based on the provided configuration.
 * @details This function sets up the DMA channel according to the provided configuration structure.
 *          It configures the interrupt, source and destination addresses, address increments, data width,
 *          transfer size, continuous mode, and flow control.
 * @param DMAConfig Pointer to the DMA configuration structure.
 */
void cb_dma_setup(stDMAConfig *DMAConfig) 
{ 
    stDMAChannel_TypeDef *DMA_CHANNEL;

    /* NULL check */
    if (DMAConfig == NULL)
    {
        return;
    }

    // Configure the channel ready interrupt
    if (DMAConfig->IRQEnable == EN_DMA_IRQ_ENABLE)
    {
        pDMA->irq_en |= (0x1UL << DMAConfig->DMAChannel);  // Enable interrupt for the specified channel
    }
    else 
    {
        pDMA->irq_en &= ~(0x1UL << DMAConfig->DMAChannel); // Disable interrupt for the specified channel
    }

    // Set default flow control wait states (no use case for this yet)
    pDMA->ChFcWs = 0x00000003UL;

    // Select the DMA channel based on the configuration and clear pending DMA requests
    switch (DMAConfig->DMAChannel)
    {
        case EN_DMA_CHANNEL_0:
            DMA_CHANNEL = pDMA_CHANNEL0;
            break;
        case EN_DMA_CHANNEL_1:
            DMA_CHANNEL = pDMA_CHANNEL1;
            break;
        case EN_DMA_CHANNEL_2:
            DMA_CHANNEL = pDMA_CHANNEL2;
            break;
        default:
            DMA_CHANNEL = pDMA_CHANNEL0; // Default to channel 0 if invalid channel is specified
            break;
    }
    // Assign source and destination addresses
    DMA_CHANNEL->ChannelSrcAddr   = DMAConfig->SrcAddr;
    DMA_CHANNEL->ChannelDestAddr  = DMAConfig->DestAddr;

    // Clear previous configurations
    DMA_CHANNEL->ChannelCtrl &= 0x00000000UL;

    // Configure source address increment
    switch (DMAConfig->SrcInc)
    {
        case EN_DMA_SRC_ADDR_INC_DISABLE:
            DMA_CHANNEL->ChannelCtrl &= ~(DMA_SINC); // Disable source address increment
            break;
        case EN_DMA_SRC_ADDR_INC_ENABLE:
            DMA_CHANNEL->ChannelCtrl |= DMA_SINC;   // Enable source address increment
            break;
    }

    // Configure destination address increment
    switch (DMAConfig->DestInc)
    {
        case EN_DMA_DEST_ADDR_INC_DISABLE:
            DMA_CHANNEL->ChannelCtrl &= ~(DMA_DINC); // Disable destination address increment
            break;
        case EN_DMA_DEST_ADDR_INC_ENABLE:
            DMA_CHANNEL->ChannelCtrl |= DMA_DINC;   // Enable destination address increment
            break;
    }

    // Configure data width for both source and destination
    switch (DMAConfig->DataWidth)
    {
        case EN_DMA_DATA_WIDTH_BYTE:
            DMA_CHANNEL->ChannelCtrl &= ~(DMA_SWIDTH); // Set source data width to byte
            DMA_CHANNEL->ChannelCtrl &= ~(DMA_DWIDTH); // Set destination data width to byte
            break;
        case EN_DMA_DATA_WIDTH_HWORD:
            DMA_CHANNEL->ChannelCtrl |= (0x1UL << DMA_SWIDTH_POS); // Set source data width to half-word
            DMA_CHANNEL->ChannelCtrl |= (0x1UL << DMA_DWIDTH_POS); // Set destination data width to half-word
            break;
        case EN_DMA_DATA_WIDTH_WORD:
            DMA_CHANNEL->ChannelCtrl |= (0x2UL << DMA_SWIDTH_POS); // Set source data width to word
            DMA_CHANNEL->ChannelCtrl |= (0x2UL << DMA_DWIDTH_POS); // Set destination data width to word
            break;
    }

    // Configure transfer size
    DMA_CHANNEL->ChannelCtrl |= ((DMAConfig->DataLen << DMA_TRANSFER_SIZE_POS) & DMA_TRANSFER_SIZE_MSK);

    // Configure continuous mode (ignoring transfer size)
    switch (DMAConfig->ContinuousMode)
    {
        case EN_DMA_CONTINUOUS_MODE_DISABLE:
            DMA_CHANNEL->ChannelCtrl &= ~(DMA_CONTINUOUS); // Disable continuous mode
            break;
        case EN_DMA_CONTINUOUS_MODE_ENABLE:
            DMA_CHANNEL->ChannelCtrl |= DMA_CONTINUOUS;   // Enable continuous mode
            break;
    }

    // Configure flow control (only applicable on V3)
    #if 0
    if (DMAConfig->TransferType == EN_DMA_TRANSFER_TYPE_SINGLE) 
    {
        DMA_CHANNEL->ChannelCtrl |= ((DMAConfig->TransferType << DMA_FC_SRC_POSEDGE_POS) & DMA_FC_SRC_POSEDGE_MSK) 
                                | ((DMAConfig->TransferType << DMA_FC_DEST_POSEDGE_POS) & DMA_FC_DEST_POSEDGE_MSK);
    }
    #endif

    // Configure flow control source and destination selections
    DMA_CHANNEL->ChannelFcSrcSel = ((((uint32_t) DMAConfig->FlowControlSrc) << DMA_CH_FC_SRC_SEL_POS) & DMA_CH_FC_SRC_SEL_MSK)  
                                | ((((uint32_t) DMAConfig->FlowControlDest) << DMA_CH_FC_DEST_SEL_POS) & DMA_CH_FC_DEST_SEL_MSK);
}


/**
 * @brief Initializes the DMA linked list for a specified channel.
 * @details This function sets up the DMA linked list for a specified channel based on the provided configuration.
 *          It configures the DMA channel using the `cb_dma_setup` function and sets the next linked list item (LLI).
 * @param DMALLiConfig Pointer to the DMA linked list configuration structure.
 */
void cb_dma_lli_init(struct stDMALinkedListHead *DMALLiConfig) 
{
    stDMAChannel_TypeDef *DMA_CHANNEL;

    /* NULL check */
    if (DMALLiConfig == NULL)
    {
        return; // Return if the input pointer is NULL
    }

    // Configure the DMA channel using the provided configuration
    cb_dma_setup(&(DMALLiConfig->DMAConfig));

    // Select the DMA channel based on the configuration
    switch (DMALLiConfig->DMAConfig.DMAChannel)
    {
        case EN_DMA_CHANNEL_0:
            DMA_CHANNEL = pDMA_CHANNEL0;
            break;
        case EN_DMA_CHANNEL_1:
            DMA_CHANNEL = pDMA_CHANNEL1;
            break;
        case EN_DMA_CHANNEL_2:
            DMA_CHANNEL = pDMA_CHANNEL2;
            break;
        default:
            DMA_CHANNEL = pDMA_CHANNEL0; // Default to channel 0 if invalid channel is specified
            break;
    }

    // Set the next linked list item (LLI) address
    if (DMALLiConfig->NextNode == NULL)
    {
        DMA_CHANNEL->ChannelLLIAddr = 0x00000000UL; // No next node, set to 0
    }
    else 
    {
        DMA_CHANNEL->ChannelLLIAddr = (uint32_t) DMALLiConfig->NextNode->LLiConfig; // Set the address of the next node
    }
}

/**
 * @brief Sets up a DMA linked list node with the provided configuration.
 * @details This function configures a DMA linked list node based on the provided configuration structure.
 *          It sets the source and destination addresses, address increments, data width, transfer size, and flow control.
 * @param DMALLiConfig Pointer to the DMA linked list head configuration structure.
 * @param LLiNode Pointer to the DMA linked list node to be configured.
 */
void cb_dma_lli_setup(struct stDMALinkedListHead *DMALLiConfig, struct stDMALinkedListNode *LLiNode)
{
    /* NULL check */
    if (DMALLiConfig == NULL || LLiNode == NULL)
    {
        return; // Return if either input pointer is NULL
    }

    // Transfer the DMA channel from the head configuration to the node configuration
    LLiNode->DMAConfig.DMAChannel = DMALLiConfig->DMAConfig.DMAChannel;

    /* Assign source and destination addresses */
    LLiNode->LLiConfig[0] = LLiNode->DMAConfig.SrcAddr;
    LLiNode->LLiConfig[1] = LLiNode->DMAConfig.DestAddr;

    // Clear previous configurations
    LLiNode->LLiConfig[3] &= 0x00000000UL;

    // Configure source address increment
    switch (LLiNode->DMAConfig.SrcInc)
    {
        case EN_DMA_SRC_ADDR_INC_DISABLE:
            LLiNode->LLiConfig[3] &= ~(DMA_SINC); // Disable source address increment
            break;
        case EN_DMA_SRC_ADDR_INC_ENABLE:
            LLiNode->LLiConfig[3] |= DMA_SINC;   // Enable source address increment
            break;
    }

    // Configure destination address increment
    switch (LLiNode->DMAConfig.DestInc)
    {
        case EN_DMA_DEST_ADDR_INC_DISABLE:
            LLiNode->LLiConfig[3] &= ~(DMA_DINC); // Disable destination address increment
            break;
        case EN_DMA_DEST_ADDR_INC_ENABLE:
            LLiNode->LLiConfig[3] |= DMA_DINC;   // Enable destination address increment
            break;
    }

    // Configure data width for both source and destination
    switch (LLiNode->DMAConfig.DataWidth)
    {
        case EN_DMA_DATA_WIDTH_BYTE:
            LLiNode->LLiConfig[3] &= ~(DMA_SWIDTH); // Set source data width to byte
            LLiNode->LLiConfig[3] &= ~(DMA_DWIDTH); // Set destination data width to byte
            break;
        case EN_DMA_DATA_WIDTH_HWORD:
            LLiNode->LLiConfig[3] |= (0x1UL << DMA_SWIDTH_POS); // Set source data width to half-word
            LLiNode->LLiConfig[3] |= (0x1UL << DMA_DWIDTH_POS); // Set destination data width to half-word
            break;
        case EN_DMA_DATA_WIDTH_WORD:
            LLiNode->LLiConfig[3] |= (0x2UL << DMA_SWIDTH_POS); // Set source data width to word
            LLiNode->LLiConfig[3] |= (0x2UL << DMA_DWIDTH_POS); // Set destination data width to word
            break;
    }

    // Configure transfer size
    LLiNode->LLiConfig[3] |= ((LLiNode->DMAConfig.DataLen << DMA_TRANSFER_SIZE_POS) & DMA_TRANSFER_SIZE_MSK);

    // Configure flow control (only applicable on V3)
    #if 0
    LLiNode->LLiConfig[4] |= ((LLiNode->DMAConfig.TransferType << DMA_FC_SRC_POSEDGE_POS) & DMA_FC_SRC_POSEDGE_MSK) 
                            | ((LLiNode->DMAConfig.TransferType << DMA_FC_DEST_POSEDGE_POS) & DMA_FC_DEST_POSEDGE_MSK);
    #endif

    // Configure flow control source and destination selections
    LLiNode->LLiConfig[4] = ((((uint32_t) LLiNode->DMAConfig.FlowControlSrc) << DMA_CH_FC_SRC_SEL_POS) & DMA_CH_FC_SRC_SEL_MSK)  
                            | ((((uint32_t) LLiNode->DMAConfig.FlowControlDest) << DMA_CH_FC_DEST_SEL_POS) & DMA_CH_FC_DEST_SEL_MSK);

    // Set the next linked list item (LLI) address
    if (LLiNode->NextNode == NULL)
    {
        LLiNode->LLiConfig[2] = 0x00000000UL; // No next node, set to 0
    }
    else 
    {
        LLiNode->LLiConfig[2] = (uint32_t) LLiNode->NextNode->LLiConfig; // Set the address of the next node
    }
}


/**
 * @brief Enables a specified DMA channel.
 * @details This function enables a specified DMA channel based on the provided configuration.
 *          It selects the appropriate DMA channel and sets the channel configuration to enable it.
 * @param DMAConfig Pointer to the DMA configuration structure.
 */
void cb_dma_enable_channel(stDMAConfig *DMAConfig)
{
    stDMAChannel_TypeDef *DMA_CHANNEL;

    /* NULL check */
    if (DMAConfig == NULL)
    {
        return; // Return if the input pointer is NULL
    }

    // Select the DMA channel based on the configuration
    switch (DMAConfig->DMAChannel)
    {
        case EN_DMA_CHANNEL_0:
            DMA_CHANNEL = pDMA_CHANNEL0;
            break;
        case EN_DMA_CHANNEL_1:
            DMA_CHANNEL = pDMA_CHANNEL1;
            break;
        case EN_DMA_CHANNEL_2:
            DMA_CHANNEL = pDMA_CHANNEL2;
            break;
        default:
            DMA_CHANNEL = pDMA_CHANNEL0; // Default to channel 0 if invalid channel is specified
            break;
    }

    // Enable the selected DMA channel
    DMA_CHANNEL->Channelcfg = DMA_CH_CFG_EN;
}

/**
 * @brief Disables a specified DMA channel.
 * @details This function disables a specified DMA channel based on the provided configuration.
 *          It selects the appropriate DMA channel, clears pending DMA requests, and disables the channel.
 * @param DMAConfig Pointer to the DMA configuration structure.
 */
void cb_dma_disable_channel(stDMAConfig *DMAConfig)
{
    stDMAChannel_TypeDef *DMA_CHANNEL;

    /* NULL check */
    if (DMAConfig == NULL)
    {
        return; // Return if the input pointer is NULL
    }

    // Select the DMA channel based on the configuration and clear pending DMA requests
    switch (DMAConfig->DMAChannel)
    {
        case EN_DMA_CHANNEL_0:
            DMA_CHANNEL = pDMA_CHANNEL0;
            pDMA->dma_req_reg &= ~(0x000000FFUL); // Clear all channel 0 DMA requests
            break;
        case EN_DMA_CHANNEL_1:
            DMA_CHANNEL = pDMA_CHANNEL1;
            pDMA->dma_req_reg &= ~(0x0000FF00UL); // Clear all channel 1 DMA requests
            break;
        case EN_DMA_CHANNEL_2:
            DMA_CHANNEL = pDMA_CHANNEL2;
            pDMA->dma_req_reg &= ~(0x00FF0000UL); // Clear all channel 2 DMA requests
            break;
        default:
            DMA_CHANNEL = pDMA_CHANNEL0; // Default to channel 0 if invalid channel is specified
            pDMA->dma_req_reg &= ~(0x000000FFUL); // Clear all channel 0 DMA requests
            break;
    }

    // Disable the selected DMA channel
    DMA_CHANNEL->Channelcfg &= ~(DMA_CH_CFG_EN);
}

/**
 * @brief Sets the source request for a specified DMA channel.
 * @details This function configures the source request for a specified DMA channel based on the provided configuration.
 *          It sets the appropriate bit in the DMA request register to enable the source request for the specified channel.
 * @param DMAConfig Pointer to the DMA configuration structure.
 */
void cb_dma_set_src_request(stDMAConfig *DMAConfig)
{
    uint32_t temp = 0x00000000UL;

    /* NULL check */
    if (DMAConfig == NULL)
    {
        return; // Return if the input pointer is NULL
    }

    // Set flow control channel if not disabled
    if (DMAConfig->FlowControlSrc != EN_DMA_FLOW_CONTROL_SRC_DISABLE)
    {
        temp = (0x1UL) << (DMAConfig->FlowControlSrc - 1);
    }
    else
    {
        return; // Return if flow control source is disabled
    }

    // Correspond flow control to DMA channel
    switch (DMAConfig->DMAChannel)
    {
        case EN_DMA_CHANNEL_0:
            temp = temp << 0U; // Shift for channel 0
            break;
        case EN_DMA_CHANNEL_1:
            temp = temp << 8U; // Shift for channel 1
            break;
        case EN_DMA_CHANNEL_2:
            temp = temp << 16U; // Shift for channel 2
            break;
        default:
            return; // Invalid channel, return without setting request
    }

    // Set the source request in the DMA request register
    pDMA->dma_req_reg |= temp;
}

/**
 * @brief Clears the source request for a specified DMA channel.
 * @details This function clears the source request for a specified DMA channel based on the provided configuration.
 *          It clears the appropriate bit in the DMA request register to disable the source request for the specified channel.
 * @param DMAConfig Pointer to the DMA configuration structure.
 */
void cb_dma_clear_src_request(stDMAConfig *DMAConfig)
{
    uint32_t temp = 0x00000000UL;

    /* NULL check */
    if (DMAConfig == NULL)
    {
        return; // Return if the input pointer is NULL
    }

    // Set flow control channel if not disabled
    if (DMAConfig->FlowControlSrc != EN_DMA_FLOW_CONTROL_SRC_DISABLE)
    {
        temp = (0x1UL) << (DMAConfig->FlowControlSrc - 1);
    }
    else
    {
        return; // Return if flow control source is disabled
    }

    // Correspond flow control to DMA channel
    switch (DMAConfig->DMAChannel)
    {
    case EN_DMA_CHANNEL_0:
        temp = temp << 0U; // Shift for channel 0
        break;
    case EN_DMA_CHANNEL_1:
        temp = temp << 8U; // Shift for channel 1
        break;
    case EN_DMA_CHANNEL_2:
        temp = temp << 16U; // Shift for channel 2
        break;
    default:
        return; // Invalid channel, return without clearing request
    }

    // Clear the source request in the DMA request register
    pDMA->dma_req_reg &= ~temp;
}

/**
 * @brief Sets the destination request for a specified DMA channel.
 * @details This function configures the destination request for a specified DMA channel based on the provided configuration.
 *          It sets the appropriate bit in the DMA request register to enable the destination request for the specified channel.
 * @param DMAConfig Pointer to the DMA configuration structure.
 */
void cb_dma_set_dest_request(stDMAConfig *DMAConfig)
{
    uint32_t temp = 0x00000000UL;
    /* Null check */
    if (DMAConfig == NULL)
    {
        return; // Return if the input pointer is NULL
    }

    // Set flow control channel if not disabled
    if (DMAConfig->FlowControlDest != EN_DMA_FLOW_CONTROL_DEST_DISABLE)
    {
        temp = (0x1UL) << (DMAConfig->FlowControlDest - 1);
    }
    else
    {
        return; // Return if flow control destination is disabled
    }

    // Correspond flow control to DMA channel
    switch (DMAConfig->DMAChannel)
    {
    case EN_DMA_CHANNEL_0:
        temp = temp << 0U; // Shift for channel 0
        break;
    case EN_DMA_CHANNEL_1:
        temp = temp << 8U; // Shift for channel 1
        break;
    case EN_DMA_CHANNEL_2:
        temp = temp << 16U; // Shift for channel 2
        break;
    default:
        return; // Invalid channel, return without setting request
    }

    // Set the destination request in the DMA request register
    pDMA->dma_req_reg |= temp;
}

/**
 * @brief Clears the destination request for a specified DMA channel.
 * @details This function clears the destination request for a specified DMA channel based on the provided configuration.
 *          It clears the appropriate bit in the DMA request register to disable the destination request for the specified channel.
 * @param DMAConfig Pointer to the DMA configuration structure.
 */
void cb_dma_clear_dest_request(stDMAConfig *DMAConfig)
{
    uint32_t temp = 0x00000000UL;
    /* Null check */
    if (DMAConfig == NULL)
    {
        return; // Return if the input pointer is NULL
    }

    // Set flow control channel if not disabled
    if (DMAConfig->FlowControlDest != EN_DMA_FLOW_CONTROL_DEST_DISABLE)
    {
        temp = (0x1UL) << (DMAConfig->FlowControlDest - 1);
    }
    else
    {
        return; // Return if flow control destination is disabled
    }
    // Correspond flow control to DMA channel
    switch (DMAConfig->DMAChannel)
    {
    case EN_DMA_CHANNEL_0:
        temp = temp << 0U; // Shift for channel 0
        break;
    case EN_DMA_CHANNEL_1:
        temp = temp << 8U; // Shift for channel 1
        break;
    case EN_DMA_CHANNEL_2:
        temp = temp << 16U; // Shift for channel 2
        break;
    default:
        return; // Invalid channel, return without clearing request
    }
    // Clear the destination request in the DMA request register
    pDMA->dma_req_reg &= ~temp;
}

/**
 * @brief Checks if a specified DMA channel is idle.
 * @details This function checks if a specified DMA channel is idle by examining the channel idle status register.
 * @param DMAConfig Pointer to the DMA configuration structure.
 * @return Returns CB_TRUE if the channel is idle, otherwise returns CB_FALSE.
 */
uint8_t cb_dma_is_channel_idle(stDMAConfig *DMAConfig)
{
    /* Null check */
    if (DMAConfig == NULL)
    {
    return CB_FALSE; // Return false if the input pointer is NULL
    }

    // Check the idle status of the specified DMA channel
    switch (DMAConfig->DMAChannel)
    {
    case EN_DMA_CHANNEL_0:
        return ((pDMA->ch_idle & 0x00000001UL) == 0x00000001UL) ? CB_TRUE : CB_FALSE; // Check channel 0
    case EN_DMA_CHANNEL_1:
        return ((pDMA->ch_idle & 0x00000002UL) == 0x00000002UL) ? CB_TRUE : CB_FALSE; // Check channel 1
    case EN_DMA_CHANNEL_2:
        return ((pDMA->ch_idle & 0x00000004UL) == 0x00000004UL) ? CB_TRUE : CB_FALSE; // Check channel 2
    default:
        return CB_FALSE; // Invalid channel, return false
    }
}


/**
 * @brief Configures DMA interrupts.
 * @param DMA_INT Specifies the interrupt type (e.g., EN_DMA_INT_READY_CHANNEL_0, EN_DMA_INT_ERROR_CHANNEL_1).
 * @param NewState The new state for the interrupt (ENABLE to enable, DISABLE to disable).
 */
void cb_dma_int_config(uint32_t DMA_INT, uint8_t NewState)
{
    /* Configure the interrupt based on the specified type */
    if(NewState == CB_TRUE)
    {
    pDMA->irq_en |= DMA_INT;  // Enable the specified interrupt
    }
    else
    {
    pDMA->irq_en &= ~DMA_INT; // Disable the specified interrupt
    }
}

/**
 * @brief   Get and clear the status of DMA interrupts.
 * @return  The status of the specified DMA interrupts. Each bit corresponds to the status of one interrupt source.
 */
uint32_t cb_dma_get_and_clear_it_status(void)
{
    uint16_t irqstatus =  pDMA->irq_status;
    return irqstatus ;
}

/**
 * @brief   Clear all DMA requests for a specified channel.
 * @param[in] Channel Specifies the DMA channel to clear requests for. Use the following values:
 * @return  None
 */
void cb_dma_clear_request(enDMAChannel Channel)
{
    // Check for valid Channel value
    if (Channel > 3)
    {
        return; // Invalid Channel value
    }
    // Clear all DMA requests for the specified channel
    pDMA->dma_req_reg &= ~(0xFFUL << (8 * Channel));
}

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif
