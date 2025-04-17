/**
 * @file    AppDemo.c
 * @brief   Customer-Specified DFU Use Case Example
 * @details 
 * 
 * @author  Chipsbank
 * @date    2024
 */
//-------------------------------
// INCLUDE SECTION
//-------------------------------
#include <assert.h>
#include "CB_system.h"
#include "CB_efuse.h"
#include "CB_SleepDeepSleep.h"

#include "dfu_uart.h"
#include "dfu_handler.h"
#include "dfu_blesvc.h"
// BLE controller
#include "CB_ble.h"
// BLE nimble host
#include "nimble/nimble_port.h"
// BLE applications
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "ble_hs_id_priv.h"

#define APP_DEMO_LOG_ENABLE APP_FALSE
#if (APP_DEMO_LOG_ENABLE == APP_TRUE)
  #include "app_uart.h"
  #define LOG(...) app_uart_printf(__VA_ARGS__)
#else
  #define LOG(...)
#endif
//-------------------------------
// CONFIGURATION SECTION
//-------------------------------
#define BLE_DEVICE_NAME "CB_dfu"

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
uint8_t g_own_addr_type;

//-------------------------------
// FUNCTION PROTOTYPE SECTION
//-------------------------------


static int app_ble_gap_event(struct ble_gap_event *event, void *arg);

/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
static void
app_ble_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

    /*
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Advertising tx power.
     *     o 128 bit UUID
     */

    memset(&fields, 0, sizeof fields);
    

    /* Advertise two flags:
     *     o Discoverability in forthcoming advertisement (general)
     *     o BLE-only (BR/EDR unsupported).
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    /* Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assiging the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
//    fields.tx_pwr_lvl_is_present = 1;
//    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

//    fields.uuids128 = BLE_UUID128(&gatt_svr_svc_uart_uuid.u);
//    fields.num_uuids128 = 1;
//    fields.uuids128_is_complete = 1;
    fields.name = (uint8_t *)ble_svc_gap_device_name();
    fields.name_len = strlen((char *)fields.name);
    fields.name_is_complete = 1;
    
    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        return;
    }

    memset(&fields, 0, sizeof fields);
    fields.name = (uint8_t *)ble_svc_gap_device_name();
    fields.name_len = strlen((char *)fields.name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_rsp_set_fields(&fields);
    if (rc != 0) {
        return;
    }

    /* Begin advertising. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    adv_params.itvl_min = BLE_GAP_ADV_ITVL_MS(50);
    adv_params.itvl_max = BLE_GAP_ADV_ITVL_MS(50);
    rc = ble_gap_adv_start(g_own_addr_type , NULL,BLE_HS_FOREVER,
                           &adv_params, app_ble_gap_event, NULL);
    if (rc != 0) {
        return;
    }
}



/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleuart uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unuesd by
 *                                  bleuart.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int
app_ble_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed. */
        if (event->connect.status == 0) {
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
            dfu_blesvc_set_conn_handle(event->connect.conn_handle);
            
            rc = ble_gattc_exchange_mtu(event->connect.conn_handle, NULL, NULL);
            if (rc != 0) {
                LOG("ERROR: Failed to negotiate MTU; rc = %d\n", rc);
            }
        }

        if (event->connect.status != 0) {
            /* Connection failed; resume advertising. */
            app_ble_advertise();
        }
        LOG("BLE_GAP_EVENT_CONNECT \r\n");

        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        /* Connection terminated; resume advertising. */
        app_ble_advertise();
        LOG("BLE_GAP_EVENT_DISCONNECT \r\n");
        return 0;


    case BLE_GAP_EVENT_ADV_COMPLETE:
        /* Advertising terminated; resume advertising. */
        
        LOG("BLE_GAP_EVENT_ADV_COMPLETE \r\n");
        #if 0
        cb_deep_sleep_control(1);
        #else
        app_ble_advertise();
        #endif
        return 0;

    case BLE_GAP_EVENT_REPEAT_PAIRING:
        /* We already have a bond with the peer, but it is attempting to
         * establish a new secure link.  This app sacrifices security for
         * convenience: just throw away the old bond and accept the new link.
         */
        LOG("BLE_GAP_EVENT_REPEAT_PAIRING \r\n");
        /* Delete the old bond. */
        rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
        assert(rc == 0);
        ble_store_util_delete_peer(&desc.peer_id_addr);

        /* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
         * continue with the pairing operation.
         */
        return BLE_GAP_REPEAT_PAIRING_RETRY;
    
    case BLE_GAP_EVENT_MTU:
        LOG("[Check] enter BLE_GAP_EVENT_MTU \n");
        break;
    case BLE_GAP_EVENT_PHY_UPDATE_COMPLETE:
        LOG("[Check] enter BLE_GAP_EVENT_PHY_UPDATE_COMPLETE \n");
        break;
    
    case BLE_GAP_EVENT_CONN_UPDATE:
        LOG("[Check] enter BLE_GAP_EVENT_CONN_UPDATE \n");
        break;
    case BLE_GAP_EVENT_CONN_UPDATE_REQ:
        LOG("[INFO] updating conncetion parameters...\n");
//        event->conn_update_req.conn_handle = conn_handle;
//        event->conn_update_req.peer_params = NULL;
//        LOG("[INFO] connection parameters updated!\n");
        break;
    }

    return 0;
}


static void
app_ble_on_sync(void)
{
    int rc;
    ble_addr_t addr;
    uint32_t chipId = cb_efuse_read_chip_id();
    memset(&addr,0,sizeof(addr));
    memcpy(&addr.val,&chipId,sizeof(uint32_t));
    
    //static address
    addr.val[5] |= 0xc0;

//    rc = ble_hs_id_gen_rnd(1, &addr);
//    assert(rc == 0);
//    ble_hs_id_set_pub(&addr.val);
    rc = ble_hs_id_set_rnd((uint8_t*)&addr.val);
    assert(rc == 0);
    LOG("set addr: %02X %02X %02X %02X %02X %02X",
    addr.val[0],addr.val[1],addr.val[2],addr.val[3],addr.val[4],addr.val[5]);
    
//    rc = ble_hs_util_ensure_addr(0);
    rc = ble_hs_id_infer_auto(0, &g_own_addr_type);
    assert(rc == 0);
    
    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(g_own_addr_type, addr_val, NULL);
    assert(rc == 0);
    LOG("read addr: %02X %02X %02X %02X %02X %02X",
    addr_val[0],addr_val[1],addr_val[2],addr_val[3],addr_val[4],addr_val[5]);
    
    /* Begin advertising. */
    app_ble_advertise();
}

static void
app_ble_on_reset(int reason)
{
    LOG("[INFO] Resetting state; reason=0x%x\n", reason);
    if(reason == 0x603) //hardware reset
    {
      cb_system_delay_in_ms(10);
      cb_deep_sleep_control(1);
    }
}
void app_ble_dfu_app_init(void)
{
    int rc;
    /* Initialize the BLE host. */
    ble_hs_cfg.sync_cb = app_ble_on_sync;
    //   ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
    ble_hs_cfg.reset_cb =  app_ble_on_reset;

    rc = dfu_blesvc_gatt_svr_init();
    assert(rc == 0);
    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set(BLE_DEVICE_NAME);
    assert(rc == 0);

}

void app_ble_task_controller(void *pvParameters)
{
    while (1)
    {
        CB_BLE_loop();
    }
}

void app_ble_task_host(void *pvParameters)
{
    cb_system_delay_in_ms(200);
    struct ble_npl_event *ev;
    while (1)
    {
        ev = ble_npl_eventq_get(nimble_port_get_dflt_eventq(), BLE_NPL_TIME_FOREVER);
        ble_npl_event_run(ev);
    }
}


void app_task_uart_dfu(void *pvParameters)
{
    dfu_uart_init();
    while (1)
    {
        dfu_uart_polling();
    }
}


void APP_DEMO_Init(void)
{
    BaseType_t rt;
    // Init BLE Controller
    CB_BLE_Init();

    // Init nimble host
    nimble_port_init();

    // Init apps
    app_ble_dfu_app_init();
    
    // Active Periodic RC Calibration every 1 second interval for BLE Connection Stability
    if (cb_system_start_periodic_rc_calibration(1000)!= APP_TRUE) //if RC_calibration request is failed due to calibration in-progress.
    {
      cb_system_stop_rc_calibration(); //stop the previous RC_calibration request.
      cb_system_start_periodic_rc_calibration(1000);//start a periodic RC_calibration request with 1 second interval.
    }

    // Create task for BLE host
    rt = xTaskCreate((TaskFunction_t)app_ble_task_host, "Task: BLE Host", 1000, NULL, 2, NULL);
    if(rt != pdPASS)
    {
        LOG("xTaskCreate fail \n")
    }
    // Create task fro BLE controller
    rt = xTaskCreate((TaskFunction_t)app_ble_task_controller, "Task: BLE Controller", 3000, NULL, 4, NULL);
    if(rt != pdPASS)
    {
        LOG("xTaskCreate fail \n")
    }
    // application
    rt = xTaskCreate((TaskFunction_t)app_task_uart_dfu, "Task: BLE Application", 1024, NULL, 1, NULL);
    if(rt != pdPASS)
    {
        LOG("xTaskCreate fail \n")
    }
    // Start the scheduler
    vTaskStartScheduler();
    
    while(1);
}


void APP_DEMO_Run(void)
{}
