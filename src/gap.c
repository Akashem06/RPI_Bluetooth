#include "gap.h"

#include "hci.h"
#include "hci_defs.h"
#include "mem_utils.h"

static GAPConnection connections[MAX_CONNECTIONS];
static GAPEventCallback gap_event_callback = NULL;
static uint8_t connection_count = 0;

GAPError GAP_init(GAPEventCallback event_callback, uint8_t *bt_addr) {
  if (bt_addr == NULL) {
    return GAP_ERROR_INVALID_PARAMETERS;
  }

  for (int i = 0; i < MAX_CONNECTIONS; i++) {
    connections[i].connected = false;
    connections[i].services_discovered = false;
    connections[i].connection_handle = 0;
  }

  gap_event_callback = event_callback;
  connection_count = 0U;

  HCIError status = HCI_set_bt_addr(bt_addr);

  if (status != HCI_ERROR_SUCCESS) {
    return GAP_ERROR_HCI_ERROR;
  }

  return GAP_ERROR_SUCCESS;
}

GAPError GAP_deinit(void) {
  GAP_stop_advertising();
  GAP_stop_scanning();

  for (int i = 0; i < MAX_CONNECTIONS; i++) {
    if (connections[i].connected) {
      GAP_disconnect(connections[i].connection_handle);
    }
  }

  gap_event_callback = NULL;
  connection_count = 0;

  return GAP_ERROR_SUCCESS;
}

GAPError GAP_set_device_name(const char *name) {
  static uint8_t params[248] = { 0 };

  size_t name_len = 0;
  while (name[name_len] != 0 && name_len < sizeof(params)) {
    name_len++;
  }

  memcpy(params, name, name_len);
  memset(params + name_len, 0U, 248 - name_len);

  HCICommand cmd = { .op_code.raw = CMD_BT_WRITE_LOCAL_NAME, .parameter_length = sizeof(params), .parameters = params };

  HCIError hci_status = HCI_send_command(&cmd);
  if (hci_status != HCI_ERROR_SUCCESS) {
    return GAP_ERROR_HCI_ERROR;
  }

  HCI_wait_response();

  /* Update the advertising data as well */

  static uint8_t adv_data[31] = { 0 };
  uint8_t adv_idx = 0;

  /* Add flags */
  adv_data[adv_idx++] = 2;    /* Length of flags field */
  adv_data[adv_idx++] = 0x01; /* Flags type */
  adv_data[adv_idx++] = 0x06; /* Flags value (LE General Discoverable + BR/EDR Not Supported) */

  /* Add name (truncate if too long) */
  uint8_t available_space = 31 - adv_idx - 2;
  uint8_t actual_name_len = (name_len > available_space) ? available_space : name_len;

  adv_data[adv_idx++] = actual_name_len + 1;
  adv_data[adv_idx++] = 0x09;

  memcpy(&adv_data[adv_idx], name, actual_name_len);
  adv_idx += actual_name_len;

  GAPError gap_status = GAP_set_advertising_data(adv_data, adv_idx);

  if (gap_status != GAP_ERROR_SUCCESS) {
    return gap_status;
  }
  return GAP_ERROR_SUCCESS;
}

GAPError GAP_start_advertising(uint16_t interval_ms, bool connectable) {
  uint8_t zero_addr[6] = { 0 };

  HCIError status =
      HCI_BLE_set_advertising_param(interval_ms,                                                      /* min interval */
                                    interval_ms,                                                      /* max interval */
                                    connectable ? ADV_TYPE_UNDIRECT_CONN : ADV_TYPE_UNDIRECT_NONCONN, /* advertising type */
                                    ADV_OWN_ADDR_PUBLIC,                                              /* own address type */
                                    ADV_DIR_ADDR_PUBLIC,                              /* direct address type */
                                    zero_addr,                                        /* direct address */
                                    ADV_CHANNEL_37 | ADV_CHANNEL_38 | ADV_CHANNEL_39, /* channel map */
                                    ADV_FILTER_POLICY_ALLOW_ALL                       /* filter policy */
      );

  if (status != HCI_ERROR_SUCCESS) {
    return GAP_ERROR_HCI_ERROR;
  }

  uint8_t enable_val = 1U;
  HCICommand adv_enable_cmd = { .op_code.raw = CMD_BLE_SET_ADVERTISE_ENABLE,
                                .parameter_length = 1,
                                .parameters = (uint8_t *){ &enable_val } };

  status = HCI_send_command(&adv_enable_cmd);

  if (status != HCI_ERROR_SUCCESS) {
    return GAP_ERROR_HCI_ERROR;
  }

  HCI_wait_response();
  return GAP_ERROR_SUCCESS;
}

GAPError GAP_set_advertising_data(uint8_t *adv_data, uint8_t adv_data_len) {
  if (adv_data == NULL || adv_data_len > 32) {
    return GAP_ERROR_INVALID_PARAMETERS;
  }

  /* Data goes out of scope when function returns unless declared as a static variable. */
  static uint8_t data[32] = { 0 };
  data[0] = adv_data_len;
  memcpy(&data[1], adv_data, adv_data_len);

  HCICommand cmd = { .op_code.raw = CMD_BLE_SET_ADVERTISING_DATA, .parameter_length = sizeof(data), .parameters = data };

  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return GAP_ERROR_HCI_ERROR;
  }

  HCI_wait_response();
  return GAP_ERROR_SUCCESS;
}

GAPError GAP_stop_advertising(void) {
  uint8_t disable_val = 0U;
  HCICommand adv_disable_cmd = { .op_code.raw = CMD_BLE_SET_ADVERTISE_ENABLE,
                                 .parameter_length = 1,
                                 .parameters = (uint8_t *){ &disable_val } };

  HCIError status = HCI_send_command(&adv_disable_cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return GAP_ERROR_HCI_ERROR;
  }

  HCI_wait_response();
  return GAP_ERROR_SUCCESS;
}

GAPError GAP_set_advertising_parameters(uint8_t type, uint16_t min_interval_ms, uint16_t max_interval_ms,
                                        uint8_t channel_map, uint8_t filter_policy) {
  if (min_interval_ms > max_interval_ms || min_interval_ms < 20 || max_interval_ms > 10240 || (channel_map & 0x07) == 0) {
    return GAP_ERROR_INVALID_PARAMETERS;
  }

  uint8_t zero_addr[6] = { 0 };

  HCIError status = HCI_BLE_set_advertising_param(min_interval_ms,     /* min interval */
                                                  max_interval_ms,     /* max interval */
                                                  type,                /* advertising type */
                                                  ADV_OWN_ADDR_PUBLIC, /* own address type */
                                                  ADV_DIR_ADDR_PUBLIC, /* direct address type */
                                                  zero_addr,           /* direct address */
                                                  channel_map,         /* channel map */
                                                  filter_policy        /* filter policy */
  );

  if (status != HCI_ERROR_SUCCESS) {
    return GAP_ERROR_HCI_ERROR;
  }

  return GAP_ERROR_SUCCESS;
}

GAPError GAP_start_scanning(uint16_t interval_ms, uint16_t window_ms) {
  if (window_ms > interval_ms || interval_ms < 2.5 || interval_ms > 10240) {
    return GAP_ERROR_INVALID_PARAMETERS;
  }

  HCIError status = HCI_BLE_set_scan_parameters(SCAN_ACTIVE,             /* scan type */
                                                interval_ms,             /* scan interval */
                                                window_ms,               /* scan window */
                                                SCAN_PUBLIC_DEVICE_ADDR, /* own address type */
                                                SCAN_ACCEPT_ALL          /* filter policy */
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

GAPError GAP_set_scan_response_data(uint8_t *scan_data, uint8_t scan_data_len) {
  if (scan_data == NULL || scan_data_len > 31) {
    return GAP_ERROR_INVALID_PARAMETERS;
  }

  static uint8_t data[32] = { 0 };
  data[0] = scan_data_len;

  memcpy(&data[1], scan_data, scan_data_len);

  HCICommand cmd = { .op_code.raw = CMD_BLE_SET_SCAN_RESPONSE_DATA, .parameter_length = sizeof(data), .parameters = data };

  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return GAP_ERROR_HCI_ERROR;
  }

  HCI_wait_response();
  return GAP_ERROR_SUCCESS;
}

GAPError GAP_set_scan_parameters(bool active, bool filter_duplicates, uint8_t filter_policy) {
  (void)filter_duplicates;
  uint8_t scan_type = active ? SCAN_ACTIVE : SCAN_PASSIVE;

  HCIError status = HCI_BLE_set_scan_parameters(scan_type,               /* scan type */
                                                100,                     /* scan interval (default 100ms) */
                                                50,                      /* scan window (default 50ms) */
                                                SCAN_PUBLIC_DEVICE_ADDR, /* own address type */
                                                filter_policy            /* filter policy */
  );

  if (status != HCI_ERROR_SUCCESS) {
    return GAP_ERROR_HCI_ERROR;
  }

  return GAP_ERROR_SUCCESS;
}

GAPError GAP_connect(uint8_t *peer_addr, uint16_t scan_interval_ms, uint16_t scan_window_ms) {
  HCIError status = HCI_BLE_create_connection(scan_interval_ms,                    /* scan interval */
                                              scan_window_ms,                      /* scan window */
                                              CONN_INITIATOR_FILTER_LIST_NOT_USED, /* filter policy */
                                              CONN_PEER_PUBLIC_DEVICE_ADDRESS,     /* peer address type */
                                              peer_addr,                           /* peer address */
                                              CONN_OWN_PUBLIC_DEVICE_ADDRESS,      /* own address type */
                                              50,                                  /* min connection interval (ms) */
                                              100,                                 /* max connection interval (ms) */
                                              0,                                   /* connection latency */
                                              2000                                 /* supervision timeout (ms) */
  );

  if (status != HCI_ERROR_SUCCESS) {
    return GAP_ERROR_HCI_ERROR;
  }

  return GAP_ERROR_SUCCESS;
}

GAPError GAP_disconnect(uint16_t connection_handle) {
  int conn_idx = -1;

  for (int i = 0; i < MAX_CONNECTIONS; i++) {
    if (connections[i].connected && connections[i].connection_handle == connection_handle) {
      conn_idx = i;
      break;
    }
  }

  if (conn_idx == -1) {
    return GAP_ERROR_INVALID_PARAMETERS;
  }

  HCIError status = HCI_disconnect(connection_handle, CONN_DISCONNECT_REMOTE_USER_TERMINATED);
  if (status != HCI_ERROR_SUCCESS) {
    return GAP_ERROR_HCI_ERROR;
  }

  return GAP_ERROR_SUCCESS;
}

GAPError GAP_update_connection_parameters(uint16_t connection_handle, uint16_t min_interval_ms, uint16_t max_interval_ms,
                                          uint16_t latency, uint16_t timeout_ms) {
  int conn_idx = -1;
  for (int i = 0; i < MAX_CONNECTIONS; i++) {
    if (connections[i].connected && connections[i].connection_handle == connection_handle) {
      conn_idx = i;
      break;
    }
  }

  if (conn_idx == -1) {
    return GAP_ERROR_INVALID_PARAMETERS;
  }

  /* Validate Parameters */
  if (min_interval_ms > max_interval_ms || timeout_ms < 100 || timeout_ms > 32000 || latency > 500) {
    return GAP_ERROR_INVALID_PARAMETERS;
  }

  HCIError status = HCI_BLE_connection_update(connection_handle, min_interval_ms, max_interval_ms, latency, timeout_ms);

  if (status != HCI_ERROR_SUCCESS) {
    return GAP_ERROR_HCI_ERROR;
  }

  return GAP_ERROR_SUCCESS;
}

GAPError GAP_set_appearance(uint16_t appearance) {
  (void)appearance;
  return GAP_ERROR_SUCCESS;
}

GAPError GAP_get_connection_info(uint16_t connection_handle, GAPConnection *connection) {
  (void)connection_handle;
  (void)connection;

  return GAP_ERROR_SUCCESS;
}

GAPError GAP_set_preferred_mtu(uint16_t mtu) {
  (void)mtu;
  return GAP_ERROR_SUCCESS;
}
