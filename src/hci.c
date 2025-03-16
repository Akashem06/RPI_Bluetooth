#include "hci.h"

#include <string.h>

#include "hardware_bl.h"
#include "log.h"
#include "log_bl.h"

#define MAX_PACKET_SIZE 256

static HCIState hci_state = HCI_STATE_IDLE;
static bool waiting_response = false;

static HW_RXState rx_state = HW_RX_STATE_WAIT_TYPE;
static uint8_t rx_buffer[MAX_PACKET_SIZE];
static uint8_t rx_count = 0;
static uint8_t rx_expected = 0;

extern char _binary_BCM4345C0_hcd_start[];
extern char _binary_BCM4345C0_hcd_end[];
extern size_t _binary_BCM4345C0_hcd_size;

uint8_t *bcm4345c0_fw_ptr = (uint8_t *)_binary_BCM4345C0_hcd_start;
uint8_t *bcm4345c0_fw_end = (uint8_t *)_binary_BCM4345C0_hcd_end;
size_t bcm4345c0_fw_size = (size_t)(&_binary_BCM4345C0_hcd_size);

/***************************************************************************************
 * Helper function
 **************************************************************************************/

static void wait_hci_response() {
  waiting_response = true;
  while (waiting_response) {
  }
}

// static bool is_timeout(uint32_t start_time, uint32_t timeout_ms) {
//   return (hw_get_time_ms() - start_time) > timeout_ms;
// }

/***************************************************************************************
 * State handling
 **************************************************************************************/

HCIState HCI_get_state(void) {
  return hci_state;
}

void HCI_set_state(HCIState new_state) {
  hci_state = new_state;
}

/***************************************************************************************
 * Packet serialization
 **************************************************************************************/
int HCI_encode_packet(HCIPacket packet_type, void *packet_data, uint8_t *buffer, uint16_t buffer_size) {
  uint16_t encoded_length = 0U;

  switch (packet_type) {
    case HCI_COMMAND_PACKET: {
      HCICommand *cmd = (HCICommand *)packet_data;
      if (buffer_size < 4 + cmd->parameter_length) {
        return 0;
      }

      buffer[0] = packet_type;
      buffer[1] = cmd->op_code.raw & 0xFF;
      buffer[2] = (cmd->op_code.raw >> 8) & 0xFF;
      buffer[3] = cmd->parameter_length;
      memcpy(&buffer[4], cmd->parameters, cmd->parameter_length);

      encoded_length = 4 + cmd->parameter_length;
      break;
    }
    case HCI_ASYNC_DATA_PACKET: {
      HCIAsyncData *acl = (HCIAsyncData *)packet_data;
      if (buffer_size < 5 + acl->data_total_length) {
        return 0;
      }
      buffer[0] = packet_type;
      buffer[1] = acl->connection_handle & 0xFF;
      buffer[2] = ((acl->connection_handle >> 8) & 0x0F) | (acl->pb_flag << 4) | (acl->bc_flag << 6);
      buffer[3] = acl->data_total_length & 0xFF;
      buffer[4] = (acl->data_total_length >> 8) & 0xFF;
      memcpy(&buffer[5], acl->data, acl->data_total_length);
      encoded_length = 5 + acl->data_total_length;
      break;
    }
    default:
      break;
  }
  return encoded_length;
}

HCIError HCI_decode_packet(uint8_t *buffer, uint16_t buffer_size, uint8_t *packet_type, void *packet_data) {
  if (buffer_size < 1) {
    return HCI_ERROR_INVALID_PARAMETERS;
  }
  *packet_type = buffer[0];

  switch (*packet_type) {
    case HCI_EVENT_PACKET: {
      if (buffer_size < 3) {
        return HCI_ERROR_INVALID_PARAMETERS;
      }
      HCIEvent *event = (HCIEvent *)packet_data;
      event->event_code = buffer[1];
      event->parameter_total_length = buffer[2];
      if (buffer_size < 3 + event->parameter_total_length) {
        return HCI_ERROR_INVALID_PARAMETERS;
      }
      event->parameters = &buffer[3];
      return 3 + event->parameter_total_length;
    }
    case HCI_ASYNC_DATA_PACKET: {
      if (buffer_size < 5) {
        return HCI_ERROR_INVALID_PARAMETERS;
      }
      HCIAsyncData *acl = (HCIAsyncData *)packet_data;
      acl->connection_handle = buffer[1] | ((buffer[2] & 0x0F) << 8);
      acl->pb_flag = (buffer[2] >> 4) & 0x03;
      acl->bc_flag = (buffer[2] >> 6) & 0x03;
      acl->data_total_length = buffer[3] | (buffer[4] << 8);
      if (buffer_size < 5 + acl->data_total_length) {
        return HCI_ERROR_INVALID_PARAMETERS;
      }
      acl->data = &buffer[5];
      return 5 + acl->data_total_length;
    }

    default:
      return HCI_ERROR_UNKNOWN_PACKET_TYPE;
  }
  return HCI_ERROR_SUCCESS;
}

/***************************************************************************************
 * Error Handler
 **************************************************************************************/

void HCI_handle_error(uint8_t error_code) {
  char *error_message;
  switch (error_code) {
    case HCI_ERROR_INVALID_OPCODE:
      error_message = "Invalid Op Code";
      break;
    case HCI_ERROR_INVALID_EVENT:
      error_message = "Invalid Event";
      break;
    case HCI_ERROR_UNKNOWN_COMMAND:
      error_message = "Unkown Command";
      break;
    case HCI_ERROR_INVALID_PARAMETERS:
      error_message = "Invalid Parameters";
      break;
    case HCI_ERROR_COMMAND_TIMEOUT:
      error_message = "Command Timeout";
      break;
    case HCI_ERROR_BUFFER_OVERFLOW:
      error_message = "Buffer Overflow";
      break;
    case HCI_ERROR_UNSUPPORTED_GROUP:
      error_message = "Unsupported Op Group";
      break;
    case HCI_ERROR_MEMORY_ALLOCATION_FAILED:
      error_message = "Memory Allocation Failed";
      break;
    case HCI_ERROR_INTERNAL_ERROR:
      error_message = "Internal Error";
      break;
    case HCI_ERROR_BUSY:
      error_message = "Busy";
      break;
    case HCI_ERROR_UNSUPPORTED_VERSION:
      error_message = "Unsupported Version";
      break;
    case HCI_ERROR_UNKNOWN_PACKET_TYPE:
      error_message = "Unknown Packet Type";
      break;
    default:
      error_message = "Unknown Error";
      break;
  }
  log_bl_error(error_message);
}

/***************************************************************************************
 * HCI command and data transmission
 **************************************************************************************/
HCIError HCI_send_command(HCICommand *cmd) {
  uint8_t packet[MAX_PACKET_SIZE];
  uint16_t packet_len = HCI_encode_packet(HCI_COMMAND_PACKET, cmd, packet, sizeof(packet));
  if (packet_len == 0) {
    return HCI_ERROR_INVALID_PARAMETERS;
  }

  hw_transmit_buffer(packet, packet_len);

  return HCI_ERROR_SUCCESS;
}

HCIError HCI_send_async_data(HCIAsyncData *data) {
  uint8_t packet[MAX_PACKET_SIZE];
  uint16_t packet_len = HCI_encode_packet(HCI_ASYNC_DATA_PACKET, data, packet, sizeof(packet));

  if (packet_len == 0) {
    return HCI_ERROR_INVALID_PARAMETERS;
  }

  hw_transmit_buffer(packet, packet_len);

  return HCI_ERROR_SUCCESS;
}

/***************************************************************************************
 * Async Data Handlers
 **************************************************************************************/

void HCI_handle_async_data(HCIAsyncData *data) {
  (void)data;
}

/***************************************************************************************
 * Event Handlers
 **************************************************************************************/

void HCI_handle_command_complete_event(uint8_t *parameters, uint8_t parameter_length) {
  if (parameter_length < 4) {
    HCI_handle_error(HCI_ERROR_INVALID_PARAMETERS);
    return;
  }

  uint8_t num_cmd_packets = parameters[0];
  (void)num_cmd_packets;
  uint16_t op_code = parameters[1] | (parameters[2] << 8);
  uint8_t status = parameters[3];
  waiting_response = false;

  if (status != HCI_ERROR_SUCCESS) {
    HCI_handle_error(status);
    return;
  }

  switch (op_code) {
    case CMD_BT_RESET:
      HCI_set_state(HCI_STATE_ON);
      break;

    case CMD_BT_READ_REMOTE_VERSION_INFORMATION:
      HCI_set_state(HCI_STATE_ON);
      break;

    case CMD_BLE_SET_ADVERTISE_ENABLE:
      /* This command toggles the advertising. */
      if (hci_state == HCI_STATE_ADVERTISING) {
        HCI_set_state(HCI_STATE_ON);
      } else {
        HCI_set_state(HCI_STATE_ADVERTISING);
      }
      break;

    case CMD_BLE_SET_SCAN_ENABLE:
      /* This command toggles scanning. */
      if (hci_state == HCI_STATE_SCANNING) {
        HCI_set_state(HCI_STATE_ON);
      } else {
        HCI_set_state(HCI_STATE_SCANNING);
      }
      break;

    case CMD_BLE_SET_RANDOM_ADDRESS:
      HCI_set_state(HCI_STATE_ON);
      break;

    case CMD_BLE_SET_SCAN_PARAMETERS:
      HCI_set_state(HCI_STATE_ON);
      break;

    case CMD_BT_READ_BD_ADDR:
      HCI_set_state(HCI_STATE_ON);
      break;

    default:
      break;
  }
}

void HCI_handle_command_status_event(uint8_t *parameters, uint8_t parameter_length) {
  if (parameter_length < 4) {
    HCI_handle_error(HCI_ERROR_INVALID_PARAMETERS);
    return;
  }

  uint8_t status = parameters[0];
  uint8_t num_cmd_packets = parameters[1];
  (void)num_cmd_packets;
  uint16_t op_code = parameters[2] | (parameters[3] << 8);

  if (status != HCI_ERROR_SUCCESS) {
    HCI_handle_error(status);
    return;
  }

  switch (op_code) {
    case CMD_BLE_CREATE_CONNECTION:
      HCI_set_state(HCI_STATE_CONNECTING);
      break;

    case CMD_BT_DISCONNECT:
      HCI_set_state(HCI_STATE_DISCONNECTED);
      break;
  }
}

void HCI_handle_disconnection_complete_event(uint8_t *parameters, uint8_t parameter_length) {
  (void)parameters;
  (void)parameter_length;
}

void HCI_handle_connection_complete_event(uint8_t *parameters, uint8_t parameter_length) {
  (void)parameters;
  (void)parameter_length;
}

void HCI_handle_BLE_connection_complete(uint8_t *subevent_parameters, uint8_t subevent_length) {
  (void)subevent_parameters;
  (void)subevent_length;
}

void HCI_handle_BLE_connection_update_complete(uint8_t *subevent_parameters, uint8_t subevent_length) {
  (void)subevent_parameters;
  (void)subevent_length;
}

void HCI_handle_BLE_enhanced_connection_complete(uint8_t *subevent_parameters, uint8_t subevent_length) {
  (void)subevent_parameters;
  (void)subevent_length;
}

void HCI_handle_event(HCIEvent *event) {
  switch (event->event_code) {
    case EVNT_BT_COMMAND_COMPLETE:
      HCI_handle_command_complete_event(event->parameters, event->parameter_total_length);
      break;
    case EVNT_BT_COMMAND_STATUS:
      HCI_handle_command_status_event(event->parameters, event->parameter_total_length);
      break;
    case EVNT_BLE_EVENT_CODE:
      uint8_t subevent_code = event->parameters[0];
      uint8_t *subevent_parameters = &event->parameters[1];
      uint16_t subevent_length = event->parameter_total_length - 1;

      switch (subevent_code) {
        case SUB_EVNT_BLE_CONNECTION_COMPLETE:
          HCI_handle_BLE_connection_complete(subevent_parameters, subevent_length);
          break;

        case SUB_EVNT_BLE_CONNECTION_UPDATE_COMPLETE:
          HCI_handle_BLE_connection_update_complete(subevent_parameters, subevent_length);
          break;

        case SUB_EVNT_BLE_ENHANCED_CONNECTION_COMPLETED:
          HCI_handle_BLE_enhanced_connection_complete(subevent_parameters, subevent_length);
          break;

        default:
          HCI_handle_error(HCI_ERROR_INVALID_EVENT);
          break;
      }
      break;
    case EVNT_BT_DISCONNECTION_COMPLETE:
      break;
    default:
      HCI_handle_error(HCI_ERROR_INVALID_EVENT);
      break;
  }
}

/***************************************************************************************
 * Hardware RX handling
 **************************************************************************************/

void HCI_handle_hw_rx(uint8_t byte) {
  rx_buffer[rx_count++] = byte;
  switch (rx_state) {
    case HW_RX_STATE_WAIT_TYPE:
      rx_count = 1;
      switch (byte) {
        case HCI_EVENT_PACKET:
          rx_state = HW_RX_STATE_WAIT_EVENT_HEADER;
          rx_expected = 2;
          break;
        case HCI_ASYNC_DATA_PACKET:
          rx_state = HW_RX_STATE_WAIT_ASYNC_HEADER;
          rx_expected = 4;
          break;
        default:
          rx_count = 0;
      }
      break;

    case HW_RX_STATE_WAIT_EVENT_HEADER:
      /* We received the event code + length. */
      if (rx_count == 3) {
        rx_expected = rx_buffer[2] + 3;
        rx_state = HW_RX_STATE_WAIT_PAYLOAD;
      }
      break;

    case HW_RX_STATE_WAIT_ASYNC_HEADER:
      if (rx_count == 5) {
        uint16_t data_total_length = rx_buffer[3] | (rx_buffer[4] << 8);
        rx_expected = data_total_length + 5;
        rx_state = HW_RX_STATE_WAIT_PAYLOAD;
      }
      break;

    case HW_RX_STATE_WAIT_PAYLOAD:
      if (rx_count == rx_expected) {
        /* Call handler function. */
        if (rx_buffer[0] == HCI_EVENT_PACKET) {
          HCIEvent event = { .event_code = rx_buffer[1],
                             .parameter_total_length = rx_buffer[2],
                             .parameters = &rx_buffer[3] };
          HCI_handle_event(&event);
        } else if (rx_buffer[0] == HCI_ASYNC_DATA_PACKET) {
          HCIAsyncData async_data = { .connection_handle = (rx_buffer[1] & 0xFF) | ((rx_buffer[2] << 8) & 0x0F),
                                      .pb_flag = (rx_buffer[2] >> 4) & 0x03,
                                      .bc_flag = (rx_buffer[2] >> 6) & 0x03,
                                      .data_total_length = rx_buffer[3] | (rx_buffer[4] << 8),
                                      .data = &rx_buffer[5] };

          HCI_handle_async_data(&async_data);
        }
        rx_state = HW_RX_STATE_WAIT_TYPE;
        rx_count = 0;
        rx_expected = 0;
      }
      break;

    default:
      break;
  }
}

uint8_t HCI_buffer_space() {
  return MAX_PACKET_SIZE - rx_count;
}

/***************************************************************************************
 * Advertising handling
 **************************************************************************************/

HCIError HCI_BLE_set_advertising_param(uint16_t adv_interval_min, uint16_t adv_interval_max, Adv_Type adv_type,
                                       Adv_OwnAddressType own_address_type, Adv_DirectAddressType direct_address_type,
                                       uint8_t *direct_address, Adv_ChannelMap adv_channel_map,
                                       Adv_FilterPolicy adv_filter_policy) {
  if (direct_address == NULL) {
    return HCI_ERROR_INVALID_PARAMETERS;
  }
  /* Convert milliseconds to bluetooth units. */
  adv_interval_min = (uint16_t)((adv_interval_min * 16) / 10);
  adv_interval_max = (uint16_t)((adv_interval_max * 16) / 10);

  uint8_t adv_params[15] = { (uint8_t)(adv_interval_min & 0xFF),
                             (uint8_t)((adv_interval_min >> 8) & 0xFF),
                             (uint8_t)(adv_interval_max & 0xFF),
                             (uint8_t)((adv_interval_max >> 8) & 0xFF),
                             adv_type,            /* Advertising type: Connectable, non-connectable. */
                             own_address_type,    /* Own address type. */
                             direct_address_type, /* Direct address type. */
                             direct_address[0],
                             direct_address[1],
                             direct_address[2], /* Direct address. */
                             direct_address[3],
                             direct_address[4],
                             direct_address[5],
                             adv_channel_map, /* Broadcast on x channels */
                             adv_filter_policy };

  HCICommand cmd = { .op_code.raw = CMD_BLE_SET_ADVERTISING_PARAMETERS,
                     .parameter_length = sizeof(adv_params),
                     .parameters = adv_params };

  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return status;
  }

  wait_hci_response();
  return status;
}

HCIError HCI_BLE_set_advertising_data(uint8_t *adv_data, uint8_t adv_data_len) {
  /* Data goes out of scope when function returns unless declared as a static variable. */
  static uint8_t data[32] = { 0 };
  data[0] = adv_data_len;
  memcpy(&data[1], adv_data, adv_data_len);

  HCICommand cmd = { .op_code.raw = CMD_BLE_SET_ADVERTISING_DATA, .parameter_length = sizeof(data), .parameters = data };

  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return status;
  }

  wait_hci_response();
  return status;
}

HCIError HCI_BLE_set_advertising_enable(bool enable) {
  uint8_t enable_val = enable ? 1U : 0U;
  HCICommand adv_enable_cmd = { .op_code.raw = CMD_BLE_SET_ADVERTISE_ENABLE,
                                .parameter_length = 1,
                                .parameters = (uint8_t *){ &enable_val } };

  HCIError status = HCI_send_command(&adv_enable_cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return status;
  }

  wait_hci_response();
  return status;
}

/***************************************************************************************
 * Scanning handling
 **************************************************************************************/

HCIError HCI_BLE_set_scan_parameters(Scan_Type scan_type, uint16_t scan_interval_ms, uint16_t scan_window_ms,
                                     Scan_OwnAddressType own_address_type, Scan_FilterPolicy scanning_filter_policy) {
  /* Convert milliseconds to bluetooth units. */
  uint16_t scan_interval = (uint16_t)((scan_interval_ms * 16) / 10);
  uint16_t scan_window = (uint16_t)((scan_window_ms * 16) / 10);

  uint8_t params[7];
  params[0] = scan_type;
  params[1] = scan_interval & 0xFF;
  params[2] = (scan_interval >> 8) & 0xFF;
  params[3] = scan_window & 0xFF;
  params[4] = (scan_window >> 8) & 0xFF;
  params[5] = own_address_type;
  params[6] = scanning_filter_policy;

  HCICommand cmd = { .op_code.raw = CMD_BLE_SET_SCAN_PARAMETERS, .parameter_length = sizeof(params), .parameters = params };

  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return status;
  }

  wait_hci_response();
  return status;
}

HCIError HCI_BLE_set_scan_enable(bool enable, bool filter_duplicates) {
  uint8_t params[2];
  params[0] = enable ? 0x01 : 0x00;
  params[1] = filter_duplicates ? 0x01 : 0x00;

  HCICommand cmd = { .op_code.raw = CMD_BLE_SET_SCAN_ENABLE, .parameter_length = sizeof(params), .parameters = params };

  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return status;
  }

  wait_hci_response();
  return status;
}

/***************************************************************************************
 * Connection handling
 **************************************************************************************/

HCIError HCI_BLE_create_connection(uint16_t scan_interval_ms, uint16_t scan_window_ms,
                                   Conn_InitiatorFilterPolicy filter_policy, Conn_PeerAddressType peer_address_type,
                                   uint8_t *peer_address, Conn_OwnAddressType own_address_type,
                                   uint16_t conn_interval_min_ms, uint16_t conn_interval_max_ms, uint16_t conn_latency,
                                   uint16_t supervision_timeout_ms) {
  /* Convert milliseconds to bluetooth units */
  uint16_t scan_interval = (uint16_t)((scan_interval_ms * 16) / 10);
  uint16_t scan_window = (uint16_t)((scan_window_ms * 16) / 10);
  uint16_t conn_interval_min = (uint16_t)((conn_interval_min_ms * 16) / 10);
  uint16_t conn_interval_max = (uint16_t)((conn_interval_max_ms * 16) / 10);

  uint8_t params[25] = {
    scan_interval & 0xFF,
    (scan_interval >> 8) & 0xFF,
    scan_window & 0xFF,
    (scan_window >> 8) & 0xFF,
    filter_policy,
    peer_address_type,
    peer_address[0],
    peer_address[1],
    peer_address[2], /* Peer address */
    peer_address[3],
    peer_address[4],
    peer_address[5],
    own_address_type,
    conn_interval_min & 0xFF,
    (conn_interval_min >> 8) & 0xFF,
    conn_interval_max & 0xFF,
    (conn_interval_max >> 8) & 0xFF,
    conn_latency & 0xFF,
    (conn_latency >> 8) & 0xFF,
    supervision_timeout_ms & 0xFF,
    (supervision_timeout_ms >> 8) & 0xFF,
    0x00,
    0x00, /* Minimum CE length */
    0x00,
    0x00 /* Maximum CE length */
  };

  HCICommand cmd = { .op_code.raw = CMD_BLE_CREATE_CONNECTION, .parameter_length = sizeof(params), .parameters = params };

  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return status;
  }

  wait_hci_response();
  return status;
}

HCIError HCI_BLE_connection_update(uint16_t connection_handle, uint16_t conn_interval_min_ms, uint16_t conn_interval_max_ms,
                                   uint16_t conn_latency, uint16_t supervision_timeout_ms) {
  /* Convert milliseconds to bluetooth units */
  uint16_t conn_interval_min = (uint16_t)((conn_interval_min_ms * 16) / 10);
  uint16_t conn_interval_max = (uint16_t)((conn_interval_max_ms * 16) / 10);

  uint8_t params[14] = {
    connection_handle & 0xFF,
    (connection_handle >> 8) & 0xFF,
    conn_interval_min & 0xFF,
    (conn_interval_min >> 8) & 0xFF,
    conn_interval_max & 0xFF,
    (conn_interval_max >> 8) & 0xFF,
    conn_latency & 0xFF,
    (conn_latency >> 8) & 0xFF,
    supervision_timeout_ms & 0xFF,
    (supervision_timeout_ms >> 8) & 0xFF,
    0x00,
    0x00, /* Minimum CE length */
    0x00,
    0x00 /* Maximum CE length */
  };

  HCICommand cmd = { .op_code.raw = CMD_BLE_CONNECTION_UPDATE, .parameter_length = sizeof(params), .parameters = params };

  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return status;
  }

  wait_hci_response();
  return status;
}

HCIError HCI_disconnect(uint16_t connection_handle, Conn_DisconnectReason reason) {
  uint8_t params[3];
  params[0] = connection_handle & 0xFF;
  params[1] = (connection_handle >> 8) & 0xFF;
  params[2] = reason;

  HCICommand cmd = { .op_code.raw = CMD_BT_DISCONNECT, .parameter_length = sizeof(params), .parameters = params };

  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return status;
  }

  wait_hci_response();
  return status;
}

/***************************************************************************************
 * Set Event Mask
 **************************************************************************************/

HCIError HCI_BLE_set_event_mask(uint8_t mask) {
  uint8_t params[8] = { mask, 0, 0, 0, 0, 0, 0, 0 };

  HCICommand cmd = { .op_code.raw = CMD_BLE_SET_EVENT_MASK, .parameter_length = sizeof(params), .parameters = params };

  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return status;
  }

  wait_hci_response();
  return status;
}

/***************************************************************************************
 * Set name
 **************************************************************************************/

HCIError HCI_set_local_name(const char *name) {
  static uint8_t params[248] = { 0 };

  size_t name_len = 0;
  while (name[name_len] != 0 && name_len < sizeof(params)) {
    name_len++;
  }

  memcpy(params, name, name_len);
  memset(params + name_len, 0U, 248 - name_len);

  HCICommand cmd = { .op_code.raw = CMD_BT_WRITE_LOCAL_NAME, .parameter_length = sizeof(params), .parameters = params };

  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return status;
  }

  wait_hci_response();

  static uint8_t adv_data[31] = { 0 };
  adv_data[0] = 2;     // Length of flags field
  adv_data[1] = 0x01;  // Flags type
  adv_data[2] = 0x06;  // Flags value

  adv_data[3] = name_len + 1;  // Length of name field + type byte
  adv_data[4] = 0x09;          // Complete Local Name type
  memcpy(&adv_data[5], name, name_len);

  uint8_t total_len = 5 + name_len;
  status = HCI_BLE_set_advertising_data(adv_data, total_len);

  return status;
}

/***************************************************************************************
 * BCM4345 firmware handling
 **************************************************************************************/

HCIError HCI_bcm4345_load_firmware(void) {
  const uint32_t CHUNK_DELAY_MS = 1;
  const uint32_t FIRMWARE_BOOT_DELAY_MS = 250;

  HCICommand cmd = { .op_code.raw = CMD_BROADCOM_DOWNLOAD_MINIDRIVER, .parameter_length = 0, .parameters = NULL };

  // Send minidriver and wait for response
  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return status;
  }

  wait_hci_response();

  hw_delay_ms(100);  // Give chip time to process minidriver

  // Validate firmware
  if (bcm4345c0_fw_size != ((uint64_t)bcm4345c0_fw_end - (uint64_t)bcm4345c0_fw_ptr)) {
    return HCI_ERROR_INTERNAL_ERROR;
  }

  if (bcm4345c0_fw_ptr[0] != 0x4c) {
    return HCI_ERROR_INTERNAL_ERROR;
  }

  // Process firmware chunks
  while (bcm4345c0_fw_ptr < bcm4345c0_fw_end) {
    if (bcm4345c0_fw_ptr + 3 > bcm4345c0_fw_end) {
      return HCI_ERROR_BUFFER_OVERFLOW;
    }

    // Extract OpCode and ParamLength
    uint16_t fw_op_code = bcm4345c0_fw_ptr[0] | (bcm4345c0_fw_ptr[1] << 8);
    bcm4345c0_fw_ptr += 2;
    uint8_t fw_parameter_length = bcm4345c0_fw_ptr[0];
    bcm4345c0_fw_ptr += 1;

    if (bcm4345c0_fw_ptr + fw_parameter_length > bcm4345c0_fw_end) {
      return HCI_ERROR_BUFFER_OVERFLOW;
    }

    cmd.op_code.raw = fw_op_code;
    cmd.parameter_length = fw_parameter_length;
    cmd.parameters = bcm4345c0_fw_ptr;

    // Send firmware chunk
    status = HCI_send_command(&cmd);
    if (status != HCI_ERROR_SUCCESS) {
      return status;
    }

    wait_hci_response();

    bcm4345c0_fw_ptr += fw_parameter_length;
    hw_delay_ms(CHUNK_DELAY_MS);
  }

  hw_delay_ms(FIRMWARE_BOOT_DELAY_MS);

  return status;
}

HCIError HCI_bcm4345_set_baudrate(uint32_t baudrate) {
  uint8_t params[6];
  params[0] = baudrate & 0xFF;
  params[1] = (baudrate >> 8) & 0xFF;
  params[2] = (baudrate >> 16) & 0xFF;
  params[3] = (baudrate >> 24) & 0xFF;
  params[4] = 0x00;
  params[5] = 0x00;

  HCICommand cmd = { .op_code.raw = CMD_BROADCOM_UPDATE_BAUDRATE, .parameter_length = 6, .parameters = params };

  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return status;
  }

  wait_hci_response();
  return status;
}

/***************************************************************************************
 * BCM435C0 Status
 **************************************************************************************/

HCIError HCI_get_module_status(BCM4345C0Info *info) {
  if (info == NULL) {
    return HCI_ERROR_INVALID_PARAMETERS;
  }

  HCICommand cmd = { .op_code.raw = CMD_BT_READ_LOCAL_VERSION_INFORMATION, .parameter_length = 0, .parameters = NULL };

  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return status;
  }

  wait_hci_response();

  info->hci_version = rx_buffer[7];
  info->hci_revision = rx_buffer[8] | (rx_buffer[9] << 8);
  info->lmp_version = rx_buffer[10];
  info->manufacturer = rx_buffer[11] | (rx_buffer[12] << 8);
  info->lmp_subversion = rx_buffer[13] | (rx_buffer[14] << 8);

  return status;
}

void HCI_print_module_status(BCM4345C0Info *info) {
  if (info == NULL) return;

  log_bl_debug("Bluetooth Module Status:\n\r");
  log_bl_debug("----------------------\n\r");
  log_bl_debug("HCI Version: %d\n\r", info->hci_version);
  log_bl_debug("HCI Revision: %d\n\r", info->hci_revision);
  log_bl_debug("LMP Version: %d\n\r", info->lmp_version);
  log_bl_debug("Manufacturer: %d\n\r", info->manufacturer);
  log_bl_debug("LMP Subversion: %d\n\n\r", info->lmp_subversion);
}

/***************************************************************************************
 * Bluetooth Address Setters/Getters
 **************************************************************************************/

HCIError HCI_set_bt_addr(uint8_t *bt_addr) {
  if (bt_addr == NULL) {
    return HCI_ERROR_INVALID_PARAMETERS;
  }

  uint8_t reversed_bt_addr[6];
  for (uint8_t i = 0; i < sizeof(reversed_bt_addr); i++) {
    reversed_bt_addr[i] = bt_addr[5 - i];
  }

  HCICommand cmd = { .op_code.raw = CMD_BROADCOM_WRITE_BD_ADDR, .parameter_length = 6, .parameters = reversed_bt_addr };

  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return status;
  }

  wait_hci_response();
  return status;
}

HCIError HCI_get_bt_addr(uint8_t *bt_addr) {
  if (bt_addr == NULL) {
    return HCI_ERROR_INVALID_PARAMETERS;
  }

  HCICommand cmd = { .op_code.raw = CMD_BT_READ_BD_ADDR, .parameter_length = 0, .parameters = NULL };

  HCIError status = HCI_send_command(&cmd);
  if (status != HCI_ERROR_SUCCESS) {
    return status;
  }

  wait_hci_response();

  memcpy(bt_addr, &rx_buffer[7], sizeof(bt_addr));

  return status;
}

/***************************************************************************************
 * Bluetooth Init/Reset
 **************************************************************************************/

HCIError HCI_init(void) {
  HCIError status;
  hw_init();

  status = HCI_reset();
  hw_delay_ms(150);
  if (status != HCI_ERROR_SUCCESS) return status;

  status = HCI_bcm4345_load_firmware();
  if (status != HCI_ERROR_SUCCESS) {
    log_bl_error("Failed load firmwre %d\r\n", status);
    return status;
  }
  hw_delay_ms(1000);
  hci_state = HCI_STATE_ON;

  /* Reset */
  return status;
}

HCIError HCI_reset(void) {
  HCICommand reset_command = {
    .op_code.raw = CMD_BT_RESET,
    .parameter_length = 0U,
    .parameters = NULL,
  };

  HCIError status = HCI_send_command(&reset_command);
  waiting_response = true;
  while (hci_state != HCI_STATE_ON);
  return status;
}
