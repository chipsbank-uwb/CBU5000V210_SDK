/**
 * @file    tdoa_config.h
 * @brief   TDOA runtime configuration interface.
 * @details Declares the shared g_tdoa_config object and the role-override hook
 *          that each example implements locally (see tdoa_role_apply_default_config).
 * @author  Chipsbank
 * @date    2026
 */

#ifndef __TDOA_CONFIG_H
#define __TDOA_CONFIG_H

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <stdint.h>
#include "tdoa_common.h"

//-------------------------------
// DEFINE SECTION
//-------------------------------

//-------------------------------
// ENUM SECTION
//-------------------------------

//-------------------------------
// STRUCT/UNION SECTION
//-------------------------------

//-------------------------------
// GLOBAL VARIABLE SECTION
//-------------------------------
extern tdoa_runtime_config_t g_tdoa_config;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
/**
 * @brief Load the TDOA runtime configuration on startup.
 * @details Applies the generic defaults first, then calls the weakly-linked
 *          tdoa_role_apply_default_config() so each role example can override
 *          identity and anchor layout without modifying the common source.
 */
void tdoa_config_init(void);

#endif /* __TDOA_CONFIG_H */
