/**
 * @file AppSysIrqCallback.c
 * @brief [SYSTEM] Functions for managing IRQ callback registration and execution.
 * @details This file contains functions for registering, deregistering, and executing IRQ callback functions.
 *          Callbacks can be registered for specific IRQ entry numbers, and when the corresponding IRQ occurs,
 *          the registered callback functions are executed.
 * @author Chipsbank
 * @date 2024
 */
 
//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "AppSysIrqCallback.h"
#include "CB_Uart.h"
#include "CB_crypto.h"
#include "CB_uwb.h"   /**!< For Ranging Callbacks */
#include "CB_timer.h" /**!< For Timer Callbacks */
#include "CB_gpio.h"  /**!< For GPIO Callbacks */
#include "CB_wdt.h"
#include "CB_crc.h"
#include "CB_dma.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define APP_SYS_IRQCALLBACK_UARTPRINT_ENABLE APP_TRUE
#if (APP_SYS_IRQCALLBACK_UARTPRINT_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define app_sys_irqcallback_print(...) app_uart_printf(__VA_ARGS__)
#else
  #define app_sys_irqcallback_print(...)
#endif

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
// Define a structure for a node in the callback list
typedef struct callback_node {
    irq_callback_t callback;
    struct callback_node* next;
} callback_node_t;

// Define a structure for a callback list
typedef struct {
    callback_node_t* head;
} callback_list_t;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
void APP_IRQ_CallBack(enIrqEntry entryNumber);

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
static callback_list_t callback_lists[EN_IRQENTRY_MAX_NUMBER];  // Array to store callback lists for each IRQ

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------
/**
 * @brief Registers a callback function for a specific IRQ entry number.
 * 
 * This function registers a callback function to be called when the corresponding IRQ occurs.
 * 
 * @param entryNumber The entry number of the IRQ.
 * @param callback The callback function to be registered.
 */
void app_irq_register_irqcallback(enIrqEntry entryNumber, irq_callback_t callback)
{
  if (entryNumber >= EN_IRQENTRY_MAX_NUMBER) 
  {
    app_sys_irqcallback_print("register_irq_callback: Invalid Entry Number\n");
    return;
  }
  
  // Check if the callback is already registered for this IRQ
  callback_node_t* current = callback_lists[entryNumber].head;
  while (current != NULL) 
  {
    if (current->callback == callback) 
    {
      // Callback already registered, no need to register again
      app_sys_irqcallback_print("Callback already registered for Entry %d\n", entryNumber);
      return;
      }
    current = current->next;
  }  
  
  // Create a new node for the callback
  callback_node_t* new_node = (callback_node_t*)malloc(sizeof(callback_node_t));
  if (new_node == NULL) 
  {
    app_sys_irqcallback_print("Memory allocation failed\n");
    return;
  }
  new_node->callback = callback;
  new_node->next = NULL;
  
  // Add the new node to the beginning of the callback list for the IRQ
  if (callback_lists[entryNumber].head == NULL) 
  {
    // First callback for this IRQ
    callback_lists[entryNumber].head = new_node;
  } 
  else 
  {
    // Add the new node to the beginning of the list
    new_node->next = callback_lists[entryNumber].head;
    callback_lists[entryNumber].head = new_node;
  }
}

/**
 * @brief Deregisters a callback function for a specific IRQ entry number.
 * 
 * This function deregisters a previously registered callback function for the specified IRQ entry number.
 * 
 * @param entryNumber The entry number of the IRQ.
 * @param callback The callback function to be deregistered.
 */
void app_irq_deregister_irqcallback(enIrqEntry entryNumber, irq_callback_t callback) 
{
  if (entryNumber >= EN_IRQENTRY_MAX_NUMBER) 
  {
    app_sys_irqcallback_print("register_irq_callback: Invalid Entry Number\n");
    return;
  }
  callback_node_t* current = callback_lists[entryNumber].head;
  callback_node_t* prev = NULL;
  
  // Traverse the list to find the callback to deregister
  while (current != NULL) 
  {
    if (current->callback == callback)
    {
      // Found the callback to deregister
      if (prev == NULL) 
      {
        // Callback to deregister is at the head of the list
        callback_lists[entryNumber].head = current->next;
      } 
      else 
      {
        // Callback to deregister is in the middle or end of the list
        prev->next = current->next;
      }
      free(current); // Free the memory allocated for the node
      return;
    }
    // Move to the next node
    prev = current;
    current = current->next;
  }
  // If the callback is not found
  app_sys_irqcallback_print("Callback not found for Entry Number %d\n", entryNumber);
}

/**
 * @brief Executes the registered callback functions for a specific IRQ entry number.
 * 
 * This function executes all the registered callback functions associated with the specified IRQ entry number.
 * 
 * @param entryNumber The entry number of the IRQ.
 */
void APP_IRQ_CallBack(enIrqEntry entryNumber) 
{
  if (entryNumber >= EN_IRQENTRY_MAX_NUMBER) 
  {
    app_sys_irqcallback_print("register_irq_callback: Invalid Entry Number\n");
    return;
  }
  callback_node_t* current = callback_lists[entryNumber].head;
  while (current != NULL) 
  {
    current->callback();
    current = current->next;
  }
}

void cb_uwbapp_tx_done_irqhandler(void)                { APP_IRQ_CallBack(EN_IRQENTRY_UWB_TX_DONE_APP_IRQ);          }
void cb_uwbapp_tx_sfd_mark_irqhandler(void)            { APP_IRQ_CallBack(EN_IRQENTRY_UWB_TX_SFD_MARK_APP_IRQ);      }
void cb_uwbapp_rx0_done_irqcb(void)                    { APP_IRQ_CallBack(EN_IRQENTRY_UWB_RX0_DONE_APP_IRQ);         }
void cb_uwbapp_rx0_preamble_detected_irqcb(void)       { APP_IRQ_CallBack(EN_IRQENTRY_UWB_RX0_PD_DONE_APP_IRQ);      }
void cb_uwbapp_rx0_sfd_detected_irqcb(void)            { APP_IRQ_CallBack(EN_IRQENTRY_UWB_RX0_SFD_DET_DONE_APP_IRQ); }
void cb_uwbapp_rx1_done_irqhandler(void)               { APP_IRQ_CallBack(EN_IRQENTRY_UWB_RX1_DONE_APP_IRQ);         }
void cb_uwbapp_rx1_preamble_detected_irqhandler(void)  { APP_IRQ_CallBack(EN_IRQENTRY_UWB_RX1_PD_DONE_APP_IRQ);      }
void cb_uwbapp_rx1_sfd_detected_irqcb(void)            { APP_IRQ_CallBack(EN_IRQENTRY_UWB_RX1_SFD_DET_DONE_APP_IRQ); }
void cb_uwbapp_rx2_done_irqcb(void)                    { APP_IRQ_CallBack(EN_IRQENTRY_UWB_RX2_DONE_APP_IRQ);         }
void cb_uwbapp_rx2_preamble_detected_irqhandler(void)  { APP_IRQ_CallBack(EN_IRQENTRY_UWB_RX2_PD_DONE_APP_IRQ);      }
void cb_uwbapp_rx2_sfd_detected_irqcb(void)            { APP_IRQ_CallBack(EN_IRQENTRY_UWB_RX2_SFD_DET_DONE_APP_IRQ); }
void cb_uwbapp_rx_sts_cir_end_irqhandler(void)         { APP_IRQ_CallBack(EN_IRQENTRY_UWB_RX_STS_CIR_END_APP_IRQ);   }
void cb_uwbapp_rx_phr_detected_irqhandler(void)        { APP_IRQ_CallBack(EN_IRQENTRY_UWB_RX_PHR_DETECTED_APP_IRQ);  }
void cb_uwbapp_rx_done_irqhandler(void)                { APP_IRQ_CallBack(EN_IRQENTRY_UWB_RX_DONE_APP_IRQ); }

void cb_gpio_app_irq_callback(void)                  { APP_IRQ_CallBack(EN_IRQENTRY_GPIO_APP_IRQ);    }

void cb_timer_0_app_irq_callback(void)               { APP_IRQ_CallBack(EN_IRQENTRY_TIMER_0_APP_IRQ); }
void cb_timer_1_app_irq_callback(void)               { APP_IRQ_CallBack(EN_IRQENTRY_TIMER_1_APP_IRQ); }
void cb_timer_2_app_irq_callback(void)               { APP_IRQ_CallBack(EN_IRQENTRY_TIMER_2_APP_IRQ); }
void cb_timer_3_app_irq_callback(void)               { APP_IRQ_CallBack(EN_IRQENTRY_TIMER_3_APP_IRQ); }

void cb_wdt_nmi_app_irq_callback(void)               { APP_IRQ_CallBack(EN_IRQENTRY_WDT_APP_IRQ);     }

void cb_uart_0_rxb_full_app_irq_callback(void)       { app_uart_0_rxb_full_callback();  } /* Direct callback here for improved IRQ processing time in current SDK */
void cb_uart_0_txb_empty_app_irq_callback(void)      { APP_IRQ_CallBack(EN_IRQENTRY_UART_0_TXB_EMPTY_APP_IRQ); }
void cb_uart_1_rxb_full_app_irq_callback(void)       { APP_IRQ_CallBack(EN_IRQENTRY_UART_1_RXB_FULL_APP_IRQ);  }
void cb_uart_1_txb_empty_app_irq_callback(void)      { APP_IRQ_CallBack(EN_IRQENTRY_UART_1_TXB_EMPTY_APP_IRQ); }

void cb_crypto_app_irq_callback(void)                { APP_IRQ_CallBack(EN_IRQENTRY_CRYPTO_APP_IRQ);}
																								     
void cb_crc_app_irq_callback(void)                   { APP_IRQ_CallBack(EN_IRQENTRY_CRC_APP_IRQ);}

void cb_dma_app_irq_callback(void)                   { APP_IRQ_CallBack(EN_IRQENTRY_DMA_APP_IRQ);          }
void cb_dma_channel_0_app_irq_callback(void)         { APP_IRQ_CallBack(EN_IRQENTRY_DMA_CHANNEL_0_APP_IRQ);}
void cb_dma_channel_1_app_irq_callback(void)         { APP_IRQ_CallBack(EN_IRQENTRY_DMA_CHANNEL_1_APP_IRQ);}
void cb_dma_channel_2_app_irq_callback(void)         { APP_IRQ_CallBack(EN_IRQENTRY_DMA_CHANNEL_2_APP_IRQ);}

void cb_i2c_app_irq_callback(void)                   { APP_IRQ_CallBack(EN_IRQENTRY_I2C_APP_IRQ);          }
