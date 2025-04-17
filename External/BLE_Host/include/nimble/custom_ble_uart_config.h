#ifndef BLE_UART_CONFIG_H
#define BLE_UART_CONFIG_H

// UART configuration for BLE transport
#define BLE_TRANSPORT_UART_PORT         1        // UART index to use for HCI interface
#define BLE_TRANSPORT_UART_BAUDRATE     19200  // Baudrate on UART
#define BLE_TRANSPORT_UART_DATA_BITS    8        // Number of data bits on UART
#define BLE_TRANSPORT_UART_STOP_BITS    1        // Number of stop bits on UART

// Parity configuration
#define BLE_TRANSPORT_UART_PARITY_NONE  0
#define BLE_TRANSPORT_UART_PARITY_EVEN  1
#define BLE_TRANSPORT_UART_PARITY_ODD   2
#define BLE_TRANSPORT_UART_PARITY       BLE_TRANSPORT_UART_PARITY_NONE

// Flow control configuration
#define BLE_TRANSPORT_UART_FLOW_CONTROL_NONE    0
#define BLE_TRANSPORT_UART_FLOW_CONTROL_RTSCTS 1
#define BLE_TRANSPORT_UART_FLOW_CONTROL        BLE_TRANSPORT_UART_FLOW_CONTROL_RTSCTS

#endif // BLE_UART_CONFIG_H