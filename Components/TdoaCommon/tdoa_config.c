/**
 * @file    tdoa_config.c
 * @brief   TDOA runtime configuration implementation.
 * @details Ships a single shared g_tdoa_config object and a weak role hook so
 *          each example project can override identity and anchor layout
 *          without forking common source.
 * @author  Chipsbank
 * @date    2026
 */

//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "tdoa_config.h"
#include "CB_flash.h"
#include "CB_crc.h"
#include "CB_Uart.h"
#include "CB_system.h"
#include "app_uart.h"
#include "NonLIB_sharedUtils.h"

//-------------------------------
// CONFIGURATION SECTION
//-------------------------------

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

tdoa_runtime_config_t g_tdoa_config;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------
static void tdoa_config_set_defaults(void);

//-------------------------------
// FUNCTION BODY SECTION
//-------------------------------

/**
 * @brief Weak default for the role-specific override hook.
 * @details Each example overrides this in its own role file to set device ID,
 *          slot, and anchor layout. When no role file is linked, defaults apply.
 */
__WEAK void tdoa_role_apply_default_config(tdoa_runtime_config_t* config)
{
    (void)config;
}

static void tdoa_config_set_defaults(void)
{
    memset(&g_tdoa_config, 0, sizeof(g_tdoa_config));

    g_tdoa_config.device_id    = TDOA_DEFAULT_RESPONDER1_ID;
    g_tdoa_config.slot         = TDOA_DEFAULT_RESPONDER1_ID;
    g_tdoa_config.t_prop       = TDOA_DEFAULT_T_PROP_NS;
    g_tdoa_config.bias         = TDOA_DEFAULT_BIAS_CM;
    g_tdoa_config.calib_source = 0;  /* 0 = DEFAULT */

    /* Default anchor layout: (0,0), (0,1), (1,0), (1,1) metres. */
    g_tdoa_config.anchor_x[0] = 0.0f;  g_tdoa_config.anchor_y[0] = 0.0f;
    g_tdoa_config.anchor_x[1] = 0.0f;  g_tdoa_config.anchor_y[1] = 1.0f;
    g_tdoa_config.anchor_x[2] = 1.0f;  g_tdoa_config.anchor_y[2] = 0.0f;
    g_tdoa_config.anchor_x[3] = 1.0f;  g_tdoa_config.anchor_y[3] = 1.0f;
    g_tdoa_config.anchor_count = TDOA_MAX_ANCHORS;

    /* Let each role file override identity and anchor layout locally. */
    tdoa_role_apply_default_config(&g_tdoa_config);
}

void tdoa_config_init(void)
{
    tdoa_config_set_defaults();
    app_uart_printf("TDOA: config loaded from defaults (id=%u slot=%u)\r\n",
                    g_tdoa_config.device_id, g_tdoa_config.slot);
}

