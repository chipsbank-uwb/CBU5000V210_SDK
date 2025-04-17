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

#ifndef H_BLE_MONITOR_
#define H_BLE_MONITOR_

#include <syscfg/syscfg.h>
#include "nimble/transport.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_MONITOR     (MYNEWT_VAL(BLE_MONITOR_RTT) || \
                         MYNEWT_VAL(BLE_MONITOR_UART))

#if BLE_MONITOR
int ble_monitor_out(int c);
int ble_monitor_log(int level, const char *fmt, ...);
#else
static inline int
ble_monitor_out(int c)
{
    (void)c;
    return 0;
}
static inline int
ble_monitor_log(int level, const char *fmt, ...)
{
    (void)level;
    (void)fmt;
    return 0;
}

//not dynamic should alloc new memory and not use static buffer
// static uint8_t host_to_controller_buffer[296] = {0};

static inline int
ble_transport_to_ll_cmd(void *buf)
{
    return ble_transport_to_ll_cmd_impl(buf);
    // ble_buff_hdr_t *hci_host_event = (ble_buff_hdr_t*) hci_alloc_msg();

    // hci_host_event->data_size = 3 + ((uint8_t *)buf)[2];

    // memcpy(host_to_controller_buffer, buf, hci_host_event->data_size);

    // hci_host_event->buff_start = host_to_controller_buffer;

    // hci_host_event->ble_hdr_flags = BLE_BUFF_HDR_EVNT_CMD_PCKT;

    // emngr_post_event(HCI_HANDLER, hci_host_event);

    // return 0;
}

static inline int
ble_transport_to_ll_acl(struct os_mbuf *om)
{
    return ble_transport_to_ll_acl_impl(om);

    // ble_buff_hdr_t *hci_host_event = (ble_buff_hdr_t*) hci_alloc_msg();

    // hci_host_event->buff_start = om->om_data;

    // hci_host_event->data_size = OS_MBUF_PKTLEN(om);

    // hci_host_event->ble_hdr_flags = BLE_BUFF_HDR_ACL_DATA_PCKT;

    // emngr_post_event(HCI_HANDLER, hci_host_event);

    // return 0;
}

static inline int
ble_transport_to_ll_iso(struct os_mbuf *om)
{
    return ble_transport_to_ll_iso_impl(om);

    // ble_buff_hdr_t *hci_host_event = (ble_buff_hdr_t*) hci_alloc_msg();

    // hci_host_event->buff_start = om->om_data;

    // hci_host_event->data_size = OS_MBUF_PKTLEN(om);

    // hci_host_event->ble_hdr_flags = BLE_BUFF_HDR_ACL_DATA_PCKT;

    // emngr_post_event(HCI_HANDLER, hci_host_event);

    // return 0;
}

static inline int
ble_transport_to_hs_evt(void *buf)
{
    return ble_transport_to_hs_evt_impl(buf);
}

static inline int
ble_transport_to_hs_acl(struct os_mbuf *om)
{
    return ble_transport_to_hs_acl_impl(om);
}

static inline int
ble_transport_to_hs_iso(struct os_mbuf *om)
{
    return ble_transport_to_hs_iso_impl(om);
}
#endif /* BLE_MONITOR */

#ifdef __cplusplus
}
#endif

#endif
