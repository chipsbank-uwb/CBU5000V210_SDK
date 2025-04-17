/**
 * @file    app_dma.c
 * @brief   [CPU Subsystem] DMA Application Module
 * @details This module provides functions for DMA examples.
 * @author  Chipsbank
 * @date    2024
 */

#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "app_dma.h"
#include "AppSysIrqCallback.h"
#include "CB_scr.h"
#include "CB_iomux.h"
#include "CB_Uart.h"
#include "CB_timer.h"
#include "CB_dma.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_CPU_DMA_UARTPRINT_ENABLE APP_TRUE
#if (APP_CPU_DMA_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_dma_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_dma_print(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define TIMER_TEST_TIMEOUT_CASES   3

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static stDMAConfig dma_channel_0_config;
static stDMAConfig dma_channel_1_config;
// static stDMAConfig dma_channel_12_config;

// linked list example only contains 2 nodes.
static struct stDMALinkedListHead head_node_data;
static struct stDMALinkedListNode node_data;
static struct stDMALinkedListNode node2_data;


// For peripheral <-> RAM example
static  stTimerSetUp timer_setup;

static uint8_t dma_destination_memory_4[12] __attribute__((aligned(4)));

static uint8_t dma_source_memory_1[0xC8]    __attribute__((aligned(4)))= 
{
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
    0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
    0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
    0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
    0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
    0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7
};

static uint8_t dma_destination_memory_1[0xC8] __attribute__((aligned(4)));

static uint16_t dma_destination_memory_3[10]  __attribute((aligned(4)));

static uint32_t dma_source_memory_2[10] __attribute__((aligned(4))) =
{
    0x1234ABCD, 0xABCDDCBA, 0x11112222, 0x12341234, 0xFFFFFFFF, 0xFEDCBA12, 0x12344321, 0x99998888, 0xC0CCC0CC, 0xFFFFFFFF
};

static uint32_t dma_destination_memory_2[10] __attribute__((aligned(4)));

static uint16_t dma_source_memory_3[10] __attribute__((aligned(4))) = 
{
    0xFFFF, 0x1234, 0xABCD, 0x4321, 0xBEEF,0xCCDA, 0xC0CC, 0xFACC, 0x9876, 0x11FF 
};

static uint8_t dma_source_memory_4[12] __attribute((aligned(4))) = 
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06,0xAB, 0xCD, 0x8D, 0x7F, 0xFF, 0xAE
};

volatile uint32_t timer_value;
static uint32_t current_src_addr;
static uint32_t current_dest_addr;
static uint32_t current_size;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Initializes the DMA module and sets up interrupt callbacks.
 *
 * This function performs the following tasks:
 * - Initializes the DMA module using `cb_dma_init()`.
 * - Registers an interrupt callback for DMA interrupts using `app_irq_deregister_irqcallback()`.
 * - Clears the destination memory arrays (`dma_destination_memory_1`, `dma_destination_memory_2`,
 *   `dma_destination_memory_3`, `dma_destination_memory_4`) to ensure they are empty before use.
 */
void app_dma_init(void)
{
    cb_dma_init();
    app_irq_deregister_irqcallback(EN_IRQENTRY_DMA_APP_IRQ, app_amd_irq_callback);

    memset(dma_destination_memory_1, 0, sizeof(dma_destination_memory_1));
    memset(dma_destination_memory_2, 0, sizeof(dma_destination_memory_2));
    memset(dma_destination_memory_3, 0, sizeof(dma_destination_memory_3));
    memset(dma_destination_memory_4, 0, sizeof(dma_destination_memory_4));
}

/**
 * @brief Handles the DMA interrupt callback.
 */
void app_amd_irq_callback(void) 
{
    app_dma_print("\nCurrent source address:  0x%X\n", current_src_addr);
    app_dma_print("\nCurrent dest address:    0x%X\n", current_dest_addr);

    int result = memcmp((const void*)current_src_addr, (const void*)current_dest_addr, current_size);

    if (result == 0) 
    {
      app_dma_print("\nContents matched   -  SUCCESS\n");
    }
    else 
    {
      app_dma_print("\nContents mismatch  - FAILURE\n");
    }
}

/**
 * @brief Demonstrates a simple RAM-to-RAM DMA transfer.
 *
 * This function sets up and performs a DMA transfer from a source memory array
 * (`dma_source_memory_2`) to a destination memory array (`dma_destination_memory_2`).
 * It configures DMA Channel 0 with the necessary parameters, enables the channel,
 * and waits for the transfer to complete. After the transfer, it prints the contents
 * of the source and destination arrays before and after the transfer to verify the
 * correctness of the data transfer.
 */
void app_dma_ram_to_ram_example(void) 
{
    // Set current source and destination addresses and data length
    current_src_addr  = (uint32_t) dma_source_memory_2;
    current_dest_addr = (uint32_t) dma_destination_memory_2;
    current_size = (uint32_t) sizeof(dma_source_memory_2);

    // Configure DMA Channel 0
    dma_channel_0_config.DMAChannel      = EN_DMA_CHANNEL_0;                  // Set DMA channel to Channel 0
    dma_channel_0_config.IRQEnable       = EN_DMA_IRQ_ENABLE;                 // Enable DMA interrupt
    dma_channel_0_config.SrcAddr         = (uint32_t) current_src_addr;       // Set source address
    dma_channel_0_config.DestAddr        = (uint32_t) current_dest_addr;      // Set destination address
    dma_channel_0_config.DataLen         = (uint32_t) current_size;           // Set data length
    dma_channel_0_config.DataWidth       = EN_DMA_DATA_WIDTH_WORD;            // Set data width to word (32 bits)
    dma_channel_0_config.SrcInc          = EN_DMA_SRC_ADDR_INC_ENABLE;        // Enable source address increment
    dma_channel_0_config.DestInc         = EN_DMA_DEST_ADDR_INC_ENABLE;       // Enable destination address increment
    dma_channel_0_config.ContinuousMode  = EN_DMA_CONTINUOUS_MODE_DISABLE;    // Disable continuous mode
    dma_channel_0_config.TransferType    = EN_DMA_TRANSFER_TYPE_MULTI;        // Set transfer type to multi
    dma_channel_0_config.FlowControlSrc  = EN_DMA_FLOW_CONTROL_SRC_SEL_0;     // Set source flow control selection
    dma_channel_0_config.FlowControlDest = EN_DMA_FLOW_CONTROL_DEST_SEL_1;    // Set destination flow control selection

    // Setup and enable DMA channel
    cb_dma_setup(&dma_channel_0_config);                                     // Setup DMA configuration
    cb_dma_enable_channel(&dma_channel_0_config);                            // Enable DMA channel

    // Print contents of the source array before transfer
    app_dma_print("\nContents in the second test source array:\n");
    for (uint32_t i = 0; i < current_size / sizeof(uint32_t); i++) 
    {
        app_dma_print("0x%X\n", dma_source_memory_2[i]);
    }

    // Print contents of the destination array before transfer
    app_dma_print("\nContents in the second test destination array BEFORE TRANSFER:\n");
    for (uint32_t i = 0; i < current_size / sizeof(uint32_t); i++) 
    {
        app_dma_print("0x%X\n", dma_destination_memory_2[i]);
    }

    // Set destination and source requests to start the transfer
    cb_dma_set_dest_request(&dma_channel_0_config);                          // Set destination request
    app_dma_print("\nDest DMA request SET\n");
    app_dma_print("\nSetting source DMA request. Transfer should begin...\n");
    cb_dma_set_src_request(&dma_channel_0_config);                           // Set source request

    // Wait for the DMA transfer to complete
    while (cb_dma_is_channel_idle(&dma_channel_0_config) != CB_TRUE);

    // Print contents of the destination array after transfer
    app_dma_print("\nContents in the second test destination array AFTER TRANSFER:\n");
    for (uint32_t i = 0; i < current_size / sizeof(uint32_t); i++) 
    {
        app_dma_print("0x%X\n", dma_destination_memory_2[i]);
    }

    // Disable the DMA channel
    cb_dma_disable_channel(&dma_channel_0_config);                           // Disable DMA channel
}

/**
 * @brief Demonstrates a DMA linked-list transfer.
 *
 * This function sets up and performs a DMA transfer using a linked list of data packets.
 * Each data packet transfer is initialized with specific configurations, and the linked list
 * allows the use of a single DMA channel for multiple data transfers without re-configuring
 * the channel each time. The function configures DMA Channel 2, sets up the linked list nodes,
 * and performs the transfer. After the transfer, it verifies the correctness of the data transfer
 * by comparing the source and destination arrays.
 */
void app_dma_ram_to_ram_lli(void)
{
    // Configure the head node of the linked list
    head_node_data.DMAConfig.DMAChannel = EN_DMA_CHANNEL_2;                    // Set DMA channel to Channel 2
    head_node_data.DMAConfig.IRQEnable  = EN_DMA_IRQ_DISABLE;                  // Disable DMA interrupt
    head_node_data.DMAConfig.SrcAddr    = (uint32_t) dma_source_memory_1;      // Set source address
    head_node_data.DMAConfig.DestAddr   = (uint32_t) dma_destination_memory_1; // Set destination address
    head_node_data.DMAConfig.DataLen    = (uint32_t) sizeof(dma_source_memory_1); // Set data length
    head_node_data.DMAConfig.DataWidth  = EN_DMA_DATA_WIDTH_BYTE;             // Set data width to byte (8 bits)
    head_node_data.DMAConfig.SrcInc     = EN_DMA_SRC_ADDR_INC_ENABLE;         // Enable source address increment
    head_node_data.DMAConfig.DestInc    = EN_DMA_DEST_ADDR_INC_ENABLE;        // Enable destination address increment
    head_node_data.DMAConfig.ContinuousMode = EN_DMA_CONTINUOUS_MODE_DISABLE; // Disable continuous mode
    head_node_data.DMAConfig.FlowControlSrc = EN_DMA_FLOW_CONTROL_SRC_SEL_0;  // Set source flow control selection
    head_node_data.DMAConfig.FlowControlDest = EN_DMA_FLOW_CONTROL_DEST_SEL_1; // Set destination flow control selection
    head_node_data.NextNode = &node_data;                                    // Link to the next node

    // Configure the first node of the linked list
    node_data.DMAConfig.SrcAddr    = (uint32_t) dma_source_memory_3;         // Set source address
    node_data.DMAConfig.DestAddr   = (uint32_t) dma_destination_memory_3;    // Set destination address
    node_data.DMAConfig.DataLen    = (uint32_t) sizeof(dma_source_memory_3); // Set data length
    node_data.DMAConfig.DataWidth  = EN_DMA_DATA_WIDTH_HWORD;                // Set data width to half-word (16 bits)
    node_data.DMAConfig.SrcInc     = EN_DMA_SRC_ADDR_INC_ENABLE;             // Enable source address increment
    node_data.DMAConfig.DestInc    = EN_DMA_DEST_ADDR_INC_ENABLE;            // Enable destination address increment
    node_data.DMAConfig.ContinuousMode = EN_DMA_CONTINUOUS_MODE_DISABLE;     // Disable continuous mode
    node_data.DMAConfig.FlowControlSrc = EN_DMA_FLOW_CONTROL_SRC_SEL_2;      // Set source flow control selection
    node_data.DMAConfig.FlowControlDest = EN_DMA_FLOW_CONTROL_DEST_SEL_3;     // Set destination flow control selection
    node_data.NextNode = &node2_data;                                       // Link to the next node

    // Configure the second node of the linked list
    node2_data.DMAConfig.SrcAddr    = (uint32_t) dma_source_memory_4;         // Set source address
    node2_data.DMAConfig.DestAddr   = (uint32_t) dma_destination_memory_4;    // Set destination address
    node2_data.DMAConfig.DataLen    = (uint32_t) sizeof(dma_source_memory_4); // Set data length
    node2_data.DMAConfig.DataWidth  = EN_DMA_DATA_WIDTH_WORD;                 // Set data width to word (32 bits)
    node2_data.DMAConfig.SrcInc     = EN_DMA_SRC_ADDR_INC_ENABLE;             // Enable source address increment
    node2_data.DMAConfig.DestInc    = EN_DMA_DEST_ADDR_INC_ENABLE;            // Enable destination address increment
    node2_data.DMAConfig.ContinuousMode = EN_DMA_CONTINUOUS_MODE_DISABLE;     // Disable continuous mode
    node2_data.DMAConfig.FlowControlSrc = EN_DMA_FLOW_CONTROL_SRC_SEL_4;      // Set source flow control selection
    node2_data.DMAConfig.FlowControlDest = EN_DMA_FLOW_CONTROL_DEST_SEL_5;     // Set destination flow control selection
    node2_data.NextNode = NULL;                                              // End of linked list

    // Enable the DMA channel
    cb_dma_enable_channel(&(head_node_data.DMAConfig));                       // Enable DMA channel

    // Initialize and setup the linked list
    cb_dma_lli_init(&head_node_data);                                         // Initialize linked list
    cb_dma_lli_setup(&head_node_data, &node_data);                            // Setup first node
    cb_dma_lli_setup(&head_node_data, &node2_data);                           // Setup second node

    // Set destination and source requests to start the transfer
    cb_dma_set_dest_request(&(head_node_data.DMAConfig));                     // Set destination request for head node
    cb_dma_set_dest_request(&(node_data.DMAConfig));                          // Set destination request for first node
    cb_dma_set_dest_request(&(node2_data.DMAConfig));                         // Set destination request for second node

    cb_dma_set_src_request(&(head_node_data.DMAConfig));                      // Set source request for head node
    cb_dma_set_src_request(&(node_data.DMAConfig));                           // Set source request for first node
    cb_dma_set_src_request(&(node2_data.DMAConfig));                          // Set source request for second node

    // Wait for the DMA transfer to complete
    while (cb_dma_is_channel_idle(&(head_node_data.DMAConfig)) != CB_TRUE);

    // Verify the contents of the first transfer
    app_dma_print("\nSource addr 1st LLi -   0x%X \n", (uint32_t)dma_source_memory_1);
    app_dma_print("\nDest addr 1st LLi   -   0x%X\n", (uint32_t) dma_destination_memory_1);

    int result = memcmp(dma_source_memory_1, dma_destination_memory_1, sizeof(dma_source_memory_1));
    if (result == 0)
    {
        app_dma_print("\n--- SUCCESS ---\n");
    }
    else 
    {
        app_dma_print("\n--- FAILURE ---\n");
    }

    // Verify the contents of the second transfer
    app_dma_print("\nSource addr 2nd LLi -   0x%X \n", (uint32_t)dma_source_memory_3);
    app_dma_print("\nDest addr 2nd LLi   -   0x%X\n", (uint32_t) dma_destination_memory_3);

    result = memcmp(dma_source_memory_3, dma_destination_memory_3, sizeof(dma_source_memory_3));
    if (result == 0)
    {
        app_dma_print("\n--- SUCCESS ---\n");
    }
    else 
    {
        app_dma_print("\n--- FAILURE ---\n");
    }

    // Verify the contents of the third transfer
    app_dma_print("\nSource addr 3rd LLi -   0x%X \n", (uint32_t)dma_source_memory_4);
    app_dma_print("\nDest addr 3rd LLi   -   0x%X\n", (uint32_t) dma_destination_memory_4);

    result = memcmp(dma_source_memory_4, dma_destination_memory_4, sizeof(dma_source_memory_4));
    if (result == 0)
    {
        app_dma_print("\n--- SUCCESS ---\n");
    }
    else 
    {
        app_dma_print("\n--- FAILURE ---\n");
    }

    // Disable the DMA channel
    cb_dma_disable_channel(&(head_node_data.DMAConfig));                       // Disable DMA channel
}

/**
 * @brief Demonstrates the use of DMA with peripheral functions.
 *
 * This function initializes UART and DMA modules, then runs a DMA transfer example.
 */
void app_peripheral_dma_demo_init(void)
{
    // Initialize UART: GPIO00-RX, GPIO01-TX, BPR=115200, Stop bit = 1, BitOrder = LSB, Parity = NONE
    app_uart_init();  

    // Print power-on reset message
    app_dma_print("\n\nPower-on reset\n\n");

    // Initialize DMA module
    app_dma_init();

    // Execute DMA transfer example
    app_dma_ram_to_ram_example();
}
