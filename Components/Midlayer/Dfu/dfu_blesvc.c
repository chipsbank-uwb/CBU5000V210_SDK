/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sysinit/sysinit.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "os/endian.h"
#include "dfu_blesvc.h"
#include "dfu_handler.h"

/* ble uart attr read handle */
uint16_t g_dfu_attr_read_handle;

/* ble uart attr write handle */
uint16_t g_dfu_attr_write_handle;

/* Pointer to a console buffer */
//char *console_buf;

uint16_t g_console_conn_handle;

#if (LOG_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define LOG(...) app_uart_printf(__VA_ARGS__)
#else
  #define LOG(...)
#endif



void dfu_blesvc_responder(uint16_t command, uint8_t *buf, uint8_t len);
static int
gatt_svr_chr_access_uart_write(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        /* Service: uart */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_uart_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) { {
            .uuid = &gatt_svr_chr_uart_read_uuid.u,
            .val_handle = &g_dfu_attr_read_handle,
            .access_cb = gatt_svr_chr_access_uart_write,
            .flags = BLE_GATT_CHR_F_NOTIFY,
        }, {
            /* Characteristic: Write */
            .uuid = &gatt_svr_chr_uart_write_uuid.u,
            .access_cb = gatt_svr_chr_access_uart_write,
            .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
            .val_handle = &g_dfu_attr_write_handle,
        }, {
            0, /* No more characteristics in this service */
        } },
    },

    {
        0, /* No more services */
    },
};

int dfu_blesvc_notify(uint8_t* buf, uint16_t size)
{
  struct os_mbuf *om;
  om = ble_hs_mbuf_from_flat(buf, size);
  if (!om) {
      return -1;
  }
  ble_gatts_notify_custom(g_console_conn_handle,
                          g_dfu_attr_read_handle, om);
  
  return 0;
}
  

/**
 * @brief Process receive buffer.
 * 
 * This function processes the BLE receive buffer by separating the command
 * and arguments, converting argument strings to integers, and then executing
 * the corresponding command handler function.
 * 
 * @param ptr_buffer Pointer to the BLE receive buffer.
 * @param len             The data len of buffer.
 */
void dfu_blesvc_process_buffer(uint8_t* ptr_buffer, uint8_t len)
{
    uint16_t command;
    uint8_t  cmdType;
    uint16_t  lenData;
    uint8_t  *prtData;
    uint8_t checksun = 0;

    if(len > DEF_HEADER_SIZE)
    {
        command = ((uint16_t)ptr_buffer[DEF_CMD_POS]<<8) | ptr_buffer[DEF_CMD_POS+1];
        cmdType = ptr_buffer[DEF_RESP_POS]; //Req:0x00 | Resp:0x01
        lenData = ptr_buffer[DEF_DL_POS];

        #if (DEF_DL_SIZE == 2) 
        lenData += (uint16_t)DFU_Rxbuf[DEF_DL_POS+1]<<8;
        lenData -= (DEF_0008_SIZE+DEF_CMD_SIZE+DEF_RESP_SIZE);
        #endif

        prtData = &ptr_buffer[DEF_DATA_POS];
        if(cmdType != 0x00)
        {
            LOG("command type error\n");
            return; //command Type error
        }
        uint8_t cb_done = APP_FALSE;
        //OTA command
        cb_done = dfu_halder_polling(command,prtData,lenData,dfu_blesvc_responder);
    }
}


void dfu_blesvc_responder(uint16_t command, uint8_t *buf, uint8_t len)
{
    static uint8_t cmd_respond_buf[32];
    static uint8_t seq = 0;
    //memset(cmd_respond_buf,0,DUF_TX_BUF_SIZE);
    cmd_respond_buf[DEF_RXMARKER_POS] = DEF_RXMARKER_VAL;
    #if defined DEF_SEQ_POS
    cmd_respond_buf[DEF_SEQ_POS] = ++seq;
    #endif
    cmd_respond_buf[DEF_DL_POS] = len;
    #if (DEF_DL_SIZE == 2)
    cmd_respond_buf[DEF_DL_POS] += (DEF_0008_SIZE+DEF_CMD_SIZE+DEF_RESP_SIZE);
    cmd_respond_buf[DEF_DL_POS+1] = 0; //DL_H
    #endif
    #if defined DEF_0008_POS
    cmd_respond_buf[DEF_0008_POS] = DEF_0008_L_VAL;
    cmd_respond_buf[DEF_0008_POS+1] = DEF_0008_H_VAL; 
    #endif
    cmd_respond_buf[DEF_CMD_POS] = command>>8;
    cmd_respond_buf[DEF_CMD_POS+1] = command&0xff;
    cmd_respond_buf[DEF_RESP_POS] = 0x01; //Req:0x00 | Resp:0x01
    if(len > 0)
    {
        memcpy(&cmd_respond_buf[DEF_DATA_POS],buf,len);
    }
    uint8_t checksun = 0;
    for (uint8_t i = 1; i < (len+DEF_HEADER_SIZE); i++)
    {
        checksun += cmd_respond_buf[i];
    }
    len = len+DEF_HEADER_SIZE;
    cmd_respond_buf[len] = checksun;
    len++;
    dfu_blesvc_notify(cmd_respond_buf,len);
} 

static int
gatt_svr_chr_access_uart_write(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    struct os_mbuf *om = ctxt->om;
    switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            if(attr_handle == g_dfu_attr_write_handle)
            {
                dfu_blesvc_process_buffer((uint8_t *)om->om_data, om->om_len);
            }
              return 0;
        default:
            assert(0);
            return BLE_ATT_ERR_UNLIKELY;
    }
}

/**
 * bleuart GATT server initialization
 *
 * @param eventq
 * @return 0 on success; non-zero on failure
 */
int
dfu_blesvc_gatt_svr_init(void)
{
    int rc;
    dfu_halder_init(NULL);
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        goto err;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

err:
    return rc;
}

/**
 * Sets the global connection handle
 *
 * @param connection handle
 */
void
dfu_blesvc_set_conn_handle(uint16_t conn_handle) {
    g_console_conn_handle = conn_handle;
}




