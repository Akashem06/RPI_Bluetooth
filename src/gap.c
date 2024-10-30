#include "gap.h"
#include "hci.h"
#include "hci_defs.h"
#include "mm.h"

static GAPConnection connections[MAX_CONNECTIONS];
static GAPEventCallback gap_event_callback = NULL;
static uint8_t connection_count = 0;

GAPError GAP_init(GAPEventCallback event_callback, uint8_t *bt_addr) {
    if (bt_addr == NULL) {
        return GAP_ERROR_INVALID_PARAMETERS;
    }
    size_t addr_size = 0;
    while(bt_addr[addr_size] != 0 && addr_size < 6) {
        addr_size++;
    }

    if (addr_size <= 6) {
        return GAP_ERROR_INVALID_PARAMETERS;
    }

    gap_event_callback = event_callback;
    connection_count = 0;
    
    HCIError status = HCI_set_bt_addr(bt_addr);
    if (status != HCI_ERROR_SUCCESS) {
        return GAP_ERROR_HCI_ERROR;
    }
    
    return GAP_ERROR_SUCCESS;
}

GAPError GAP_set_device_name(const char* name) {
    HCIError status = HCI_set_local_name(name);
    if (status != HCI_ERROR_SUCCESS) {
        return GAP_ERROR_HCI_ERROR;
    }
    return GAP_ERROR_SUCCESS;
}

GAPError GAP_start_advertising(uint16_t interval_ms, bool connectable) {
    uint8_t zero_addr[6] = {0};

    HCIError status = HCI_BLE_set_advertising_param(
        interval_ms,                                                        /* min interval */
        interval_ms,                                                        /* max interval */
        connectable ? ADV_TYPE_UNDIRECT_CONN : ADV_TYPE_UNDIRECT_NONCONN,   /* advertising type */
        ADV_OWN_ADDR_PUBLIC,                                                /* own address type */
        ADV_DIR_ADDR_PUBLIC,                                                /* direct address type */
        zero_addr,                                                          /* direct address */
        ADV_CHANNEL_37 | ADV_CHANNEL_38 | ADV_CHANNEL_39,                   /* channel map */
        ADV_FILTER_POLICY_ALLOW_ALL                                         /* filter policy */
    );
    
    if (status != HCI_ERROR_SUCCESS) {
        return GAP_ERROR_HCI_ERROR;
    }

    status = HCI_BLE_set_advertising_enable(true);
    if (status != HCI_ERROR_SUCCESS) {
        return GAP_ERROR_HCI_ERROR;
    }
    
    return GAP_ERROR_SUCCESS;
}

GAPError GAP_stop_advertising(void) {
    HCIError status = HCI_BLE_set_advertising_enable(false);
    if (status != HCI_ERROR_SUCCESS) {
        return GAP_ERROR_HCI_ERROR;
    }
    return GAP_ERROR_SUCCESS;
}

GAPError GAP_start_scanning(uint16_t interval_ms, uint16_t window_ms) {
    // Set scan parameters
    HCIError status = HCI_BLE_set_scan_parameters(
        SCAN_ACTIVE,                /* scan type */
        interval_ms,                /* scan interval */
        window_ms,                  /* scan window */
        SCAN_PUBLIC_DEVICE_ADDR,    /* own address type */
        SCAN_ACCEPT_ALL             /* filter policy */
    );
    
    if (status != HCI_ERROR_SUCCESS) {
        return GAP_ERROR_HCI_ERROR;
    }

    status = HCI_BLE_set_scan_enable(true, true);
    if (status != HCI_ERROR_SUCCESS) {
        return GAP_ERROR_HCI_ERROR;
    }
    
    return GAP_ERROR_SUCCESS;
}

GAPError GAP_stop_scanning(void) {
    HCIError status = HCI_BLE_set_scan_enable(false, false);
    if (status != HCI_ERROR_SUCCESS) {
        return GAP_ERROR_HCI_ERROR;
    }
    return GAP_ERROR_SUCCESS;
}

GAPError GAP_connect(uint8_t* peer_addr, uint16_t scan_interval_ms, uint16_t scan_window_ms) {
    HCIError status = HCI_BLE_create_connection(
        scan_interval_ms,                       /* scan interval */
        scan_window_ms,                         /* scan window */
        CONN_INITIATOR_FILTER_LIST_NOT_USED,    /* filter policy */
        CONN_PEER_PUBLIC_DEVICE_ADDRESS,        /* peer address type */
        peer_addr,                              /* peer address */
        CONN_OWN_PUBLIC_DEVICE_ADDRESS,         /* own address type */
        50,                                     /* min connection interval (ms) */
        100,                                    /* max connection interval (ms) */
        0,                                      /* connection latency */
        2000                                    /* supervision timeout (ms) */
    );
    
    if (status != HCI_ERROR_SUCCESS) {
        return GAP_ERROR_HCI_ERROR;
    }
    
    return GAP_ERROR_SUCCESS;
}

GAPError GAP_disconnect(uint16_t connection_handle) {
    HCIError status = HCI_disconnect(connection_handle, CONN_DISCONNECT_REMOTE_USER_TERMINATED);
    if (status != HCI_ERROR_SUCCESS) {
        return GAP_ERROR_HCI_ERROR;
    }
    return GAP_ERROR_SUCCESS;
}