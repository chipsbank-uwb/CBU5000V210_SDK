/**
 * @file    AppSysUartCommander.h
 * @brief   [SYSTEM] Header file for the UART Commander Application Module
 * @details This header file declares the UART Commander module and includes necessary
 *          headers for function prototypes, definitions, enums, structs/unions, and global variables.
 * @author  Chipsbank
 * @date    2024
 */
 
#ifndef __APP_SYS_UART_COMMANDER_H
#define __APP_SYS_UART_COMMANDER_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include "APP_CompileOption.h"
#include "APP_common.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------
#define OFF_CMD (0u)
#define PERIODIC_RX_CMD (1u)
#define PERIODIC_TX_CMD (2u)

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------
typedef void (*ptrFunction)(uint32_t const argc, uint32_t  *args);

typedef struct {
    char command;
    ptrFunction handler;
} app_uart_cmd_st;

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------

/**
 * @brief Handles UART command processing for UWB TX and RX operation modes.
 *
 * This function processes UART commands to configure and execute UWB 
 * transmissions (TX) or receptions (RX). Depending on the operation mode, 
 * it sets up the required packet configurations, payloads, and ports, and 
 * triggers the appropriate UWB operations.
 *
 * @param[in] argc The number of arguments passed to the function:
 *                 - For TX: Must be 1.
 *                 - For RX: Must be 2.
 * @param[in] args A pointer to the array of arguments:
 *                 - For TX:
 *                   - args[0]: UWB operation mode (`1` for TX).
 *                 - For RX:
 *                   - args[0]: UWB operation mode (`2` for RX).
 */
void APP_UART_Func_a(uint32_t const argc, uint32_t  *args);

/**
 * @brief Handles UART command processing for PDOA operation modes.
 *
 * This function processes UART commands to configure and execute PDOA 
 * operations. Depending on the mode (Suspend, RX, or TX), it sets up the 
 * required configurations and triggers the appropriate operations.
 *
 * @param[in] argc The number of arguments passed to the function:
 *                 - For Suspend: Must be 1.
 *                 - For RX: Must be 2 or 3 (for extended configuration).
 *                 - For TX: Must be 2 or 3 (for extended configuration).
 * @param[in] args A pointer to the array of arguments:
 *                 - args[0]: PDOA operation mode:
 *                   - `0`: Suspend
 *                   - `1`: Receive (RX)
 *                   - `2`: Transmit (TX)
 *                 - args[1]: Number of PDOA internal cycles:
 *                   - `1`: One cycle
 *                   - `3`: Three cycles
 *                   - `5`: Five cycles
 *                 - args[2]: (Optional, for RX or TX extended configuration)
 *                   - RX: Logging option
 *                   - TX: Reserved argument (used to configure extended arguments)
 */
 
void APP_UART_Func_b(uint32_t const argc, uint32_t  *args);

/**
 * @brief Handles UART command processing for AOA (Angle of Arrival) operation modes.
 *
 * This function processes UART commands to configure and execute AOA 
 * operations, including TX, RX, and Suspend modes. It initializes the required 
 * packet configurations and sets up the AOA parameters such as internal cycles 
 * and biases for azimuth and elevation.
 *
 * @param[in] argc The number of arguments passed to the function:
 *                 - For Suspend: Must be 1.
 *                 - For TX: Must be 2.
 *                 - For RX: Must be 4.
 * @param[in] args A pointer to the array of arguments:
 *                 - args[0]: AOA operation mode:
 *                   - `0`: Suspend
 *                   - `1`: Receive (RX)
 *                   - `2`: Transmit (TX)
 *                 - args[1]: Number of PDOA internal cycles:
 *                   - `1`: One cycle
 *                   - `3`: Three cycles
 *                   - `5`: Five cycles
 *                 - args[2] (RX only): Azimuth bias (in degrees, signed integer)
 *                 - args[3] (RX only): Elevation bias (in degrees, signed integer)
 */
void APP_UART_Func_c(uint32_t const argc, uint32_t  *args);

/**
 * @brief Handles UART command processing for DSTWR (Distance Two-Way Ranging) operation modes.
 *
 * This function processes UART commands to configure and execute DSTWR 
 * operations, including Suspend, Responder, and Initiator modes. It initializes 
 * the configuration for ranging operations and supports extended configurations 
 * for additional parameters.
 *
 * @param[in] argc The number of arguments passed to the function:
 *                 - For Suspend: Must be 1.
 *                 - For Responder or Initiator: Must be 3 or more (extended configuration).
 * @param[in] args A pointer to the array of arguments:
 *                 - args[0]: DSTWR operation mode:
 *                   - `0`: Suspend
 *                   - `1`: Responder
 *                   - `2`: Initiator
 *                 - args[1]: Number of ranging cycles
 *                 - args[2]: Ranging bias (signed integer)
 *                 - args[3..8] (Optional, extended configuration):
 *                   - Extended arguments for advanced packet configuration
 *                   - args[8]: Logging option (unsigned 8-bit integer)
 */
void APP_UART_Func_d(uint32_t const argc, uint32_t  *args);

/**
 * @brief Handles UART command processing for RNGAOA (Ranging with Angle of Arrival) operations.
 *
 * This function processes UART commands to configure and execute RNGAOA operations, 
 * including Suspend, Responder, and Initiator modes. It supports configuration of 
 * frequency, packet count, ranging and angle biases, and DS-PDOA activation.
 *
 * @param[in] argc The number of arguments passed to the function:
 *                 - For Suspend: Must be 1.
 *                 - For Responder or Initiator: Must be 7.
 * @param[in] args A pointer to the array of arguments:
 *                 - args[0]: RNGAOA operation mode:
 *                   - `0`: Suspend
 *                   - `1`: Responder
 *                   - `2`: Initiator
 *                 - args[1]: Frequency (Hz):
 *                   - `10`: 10 Hz
 *                   - `20`: 20 Hz
 *                   - `50`: 50 Hz
 *                 - args[2]: Number of packets per PDOA cycle:
 *                   - `1`: One internal cycle
 *                   - `3`: Three internal cycles
 *                   - `5`: Five internal cycles
 *                 - args[3]: Ranging bias (double, in meters)
 *                 - args[4]: Azimuth bias (float, in degrees)
 *                 - args[5]: Elevation bias (float, in degrees)
 *                 - args[6]: DS-PDOA activation:
 *                   - `0`: Disable
 *                   - `1`: Activate
 */
void APP_UART_Func_e(uint32_t const argc, uint32_t  *args);

/**
 * @brief TRX-Periodic Command parser for customer.
 * 
 * @param args Pointer to the array of arguments.
 */
void APP_UART_Func_f(uint32_t const argc, uint32_t  *args);

/**
 * @brief Radar Command parser for customer.
 * 
 * @param args Pointer to the array of arguments.
 */
void APP_UART_Func_g(uint32_t const argc, uint32_t  *args);

/**
 * @brief RX Packet Error Rate (RX PER) Command parser for internal test.
 * 
 * @param args Pointer to the array of arguments.
 */
void APP_UART_Func_h(uint32_t const argc, uint32_t  *args);

/**
 * @brief Handles UART command processing for UWB preamble scanning operation mode.
 *
 * This function processes UART commands by configuring UWB preamble scanning
 * based on the given arguments. It initializes the UWB operation mode, sets up 
 * the preamble scanning parameters, and triggers the appropriate UWB operations 
 * based on the mode (RX or TX). 
 *
 * @param[in] argc The number of arguments passed to the function. Minimum value is 1.
 * @param[in] args A pointer to the array of arguments:
 *            - args[0]: UWB operation mode (must be cast to uint8_t).
 *            - args[1]: PSR mode in case of RX
 *            - args[2]: Scan duration in case of RX
 */
void APP_UART_Func_i(uint32_t const argc, uint32_t  *args);

/**
 * @brief Configures the UWB system parameters via UART command.
 * 
 * This function parses the provided arguments and sets the UWB system 
 * configuration. It also retrieves and prints the updated configuration.
 *
 * @param argc Number of arguments provided.
 * @param args Pointer to the array of arguments.
 * 
 * @details Usage: `s,arg0,arg1,arg2,arg3,arg4,arg5,arg6`
 * - `arg0`: TX Power Code. Refer to the power code table.
 * - `arg1`: TX & RX Packet Mode:
 *   - 0: BPRF_0681  (m5p1)
 *   - 1: BPRF_0850  (m3p2)
 *   - 2: BPRF_0850A (m1p2) - not supported yet
 *   - 3: BPRF_0681A (m5p2) - not supported yet
 *   - 4: HPRF_6810  (m4p1)
 *   - 5: HPRF_7800  (m4p2)
 *   - 6: HPRF_27200 (m4p3)
 *   - 7: HPRF_32100 (m4p4)
 * - `arg2`: TX & RX STS Mode:
 *   - 0: SP0, 1 STS Segment
 *   - 1: SP1, 1 STS Segment
 *   - 2: SP3, 1 STS Segment
 * - `arg3`: TX & RX Preamble Code Index. Range: [9, 32]. Default: {9: BPRF}, {25: HPRF}.
 * - `arg4`: TX & RX SFD Sequence Index. Range: [0, 4]. Default: 2.
 * - `arg5`: RX Operational Mode:
 *   - 0: Sensitivity mode
 *   - 1: Coexist mode
 * - `arg6`: Antenna Index.
 *
 * Usage: ``s`` with no other arguments to view the current configurations
 */
void APP_UART_Func_s(uint32_t const argc, uint32_t  *args);

/**
 * @brief   Prints the version of the CB Library.
 * 
 * This function prints the version of the CB Library in the format "CB LIB version: X.Y.Z",
 * where X is the major version, Y is the minor version, and Z is the patch version.
 * 
 * @note    This function requires CB_LIB_MAJOR_VERSION, CB_LIB_MINOR_VERSION, and CB_LIB_PATCH_VERSION
 *          to be defined appropriately.
 */
void APP_UART_PrintCBLibVersion(void);

/**
 * @brief   Prints the application name
 * 
 * This function prints name of application example [APP_UWB_EXAMPLE]
 * 
 */
void APP_UART_PrintAppName(void);

#endif /*__APP_SYS_UART_COMMANDER_H*/
