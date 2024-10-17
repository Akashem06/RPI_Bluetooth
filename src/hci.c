#include "hci.h"
#include "hardware.h"
#include <string.h>

static HCIState hci_state = HCI_STATE_IDLE;
static uint16_t hci_command_opcode = 0;

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

    switch(packet_type) {
        case HCI_COMMAND_PACKET: {
            HCICommand *cmd = (HCICommand *)packet_data;
            if (buffer_size < 4 + cmd->parameter_length) {
                return 0;
            }

            buffer[0] = packet_type;
            buffer[1] = cmd->op_code.bit.command & 0xFF;
            buffer[2] = ((cmd->op_code.bit.command >> 8) & 0x03) | (cmd->op_code.bit.group << 2);
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

HCI_Error HCI_decode_packet(uint8_t *buffer, uint16_t buffer_size, uint8_t *packet_type, void *packet_data) {
    if (buffer_size < 1) {
        return HCI_ERROR_INVALID_PARAMETERS;
    }
    *packet_type = buffer[0];

    switch(*packet_type) {
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
    const char *error_message;
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
    
}

/***************************************************************************************
 * HCI command and data transmission
 **************************************************************************************/
HCI_Error HCI_send_command(HCICommand *cmd) {
    uint8_t packet[MAX_PACKET_SIZE];
    uint16_t packet_len = HCI_encode_packet(HCI_COMMAND_PACKET, cmd, packet, sizeof(packet));

    if (packet_len == 0) {
        return HCI_ERROR_INVALID_PARAMETERS;
    }

    hw_transmit_buffer(packet, packet_len);

    hci_command_opcode = (cmd->op_code.bit.group << 10) | cmd->op_code.bit.command;
    hci_state = HCI_STATE_WAITING_RESPONSE;

    return HCI_ERROR_SUCCESS;
}

HCI_Error HCI_send_async_data(HCIAsyncData *data) {
    uint8_t packet[MAX_PACKET_SIZE];
    uint16_t packet_len = HCI_encode_packet(HCI_ASYNC_DATA_PACKET, data, packet, sizeof(packet));

    if (packet_len == 0) {
        return HCI_ERROR_INVALID_PARAMETERS;
    }

    hw_transmit_buffer(packet, packet_len);

    return HCI_ERROR_SUCCESS;
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
    uint16_t op_code = parameters[1] | (parameters[2] << 8);
    uint8_t status = parameters[3];

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

        case CMD_BT_READ_LOCAL_VERSION_INFORMATION:
            HCI_set_state(HCI_STATE_ON);
            break;

        case CMD_BLE_READ_LOCAL_SUPPORTED_FEATURES:
            HCI_set_state(HCI_STATE_ON);
            break;

        case CMD_BLE_SET_EVENT_MASK:
            HCI_set_state(HCI_STATE_ON);
            break;

        case CMD_BLE_SET_RANDOM_ADDRESS:
            HCI_set_state(HCI_STATE_ON);
            break;

        case CMD_BLE_SET_SCAN_PARAMETERS:
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

void HCI_handle_event(HCIEvent *event) {
    switch (event->event_code) {
        case EVNT_BT_COMMAND_COMPLETE:
            HCI_handle_command_complete_event(event->parameters, event->parameter_total_length);
            break;
        case EVNT_BT_COMMAND_STATUS:
            HCI_handle_command_status_event(event->parameters, event->parameter_total_length);
            break;
        default:
            HCI_handle_error(HCI_ERROR_INVALID_EVENT);
            break;
    }
}

/***************************************************************************************
 * Advertising handling
 **************************************************************************************/

HCI_Error HCI_BLE_set_advertising_param(uint16_t adv_interval_min, uint16_t adv_interval_max, 
                                        Adv_Type adv_type, Adv_OwnAddressType own_address_type,
                                        Adv_DirectAddressType direct_address_type, uint8_t *direct_address,
                                        Adv_ChannelMap adv_channel_map, Adv_FilterPolicy adv_filter_policy) {

    /* Convert milliseconds to bluetooth units. */
    adv_interval_min = (uint16_t)(adv_interval_min / 0.625);
    adv_interval_max = (uint16_t)(adv_interval_max / 0.625);

    uint8_t adv_params[15] = {
        (uint8_t)(adv_interval_min & 0xFF), (uint8_t)((adv_interval_min >> 8) & 0xFF),
        (uint8_t)(adv_interval_max & 0xFF), (uint8_t)((adv_interval_max >> 8) & 0xFF),
        adv_type,                                                   /* Advertising type: Connectable, non-connectable. */
        own_address_type,                                           /* Own address type. */
        direct_address_type,                                        /* Direct address type. */
        direct_address[0], direct_address[1], direct_address[2],    /* Direct address. */
        direct_address[3], direct_address[4], direct_address[5],
        adv_channel_map,                                            /* Broadcast on x channels */
        adv_filter_policy
    };

    HCICommand cmd = {
        .op_code = CMD_BLE_SET_ADVERTISING_PARAMETERS,
        .parameter_length = sizeof(adv_params),
        .parameters = adv_params
    };
    
    return HCI_send_command(&cmd);
}

HCI_Error HCI_BLE_set_advertising_data(uint8_t *adv_data, uint8_t adv_data_len) {

    uint8_t data[32] = {0};
    data[0] = adv_data_len;
    memcpy(&data[1], adv_data, adv_data_len);

    HCICommand cmd = {
        .op_code = CMD_BLE_SET_ADVERTISING_DATA,
        .parameter_length = sizeof(data),
        .parameters = data
    };
    
    return HCI_send_command(&cmd);
}

/***************************************************************************************
 * Scanning handling
 **************************************************************************************/

HCI_Error HCI_LE_SetScanParameters(uint8_t scan_type, uint16_t scan_interval, uint16_t scan_window,
                                   uint8_t own_address_type, uint8_t scanning_filter_policy) {

}

/***************************************************************************************
 * BCM4345 firmware handling
 **************************************************************************************/

HCI_Error HCI_bcm4345_load_firmware(void) {
    HCICommand cmd = {
        .op_code.raw = CMD_BROADCOM_DOWNLOAD_MINIDRIVER,
        .parameter_length = 0,
        .parameters = NULL
    };

    HCI_Error status = HCI_send_command(&cmd);
    if (status != HCI_ERROR_SUCCESS) {
        return status;
    }

    extern uint8_t _binary_BCM4345C0_hcd_start[];
    extern uint8_t _binary_BCM4345C0_hcd_end[];
    extern uint32_t _binary_BCM4345C0_hcd_size;

    uint8_t *firmware_ptr = _binary_BCM4345C0_hcd_start;
    uint8_t *firmware_end = firmware_ptr + _binary_BCM4345C0_hcd_size;
    
    while (firmware_ptr < firmware_end) {
        uint16_t fw_op_code = firmware_ptr[0] | (firmware_ptr[1] << 8);
        firmware_ptr += 2;

        uint8_t fw_parameter_length = firmware_ptr[0];
        firmware_ptr += 1;

        if (firmware_ptr + fw_parameter_length > firmware_end) {
            return HCI_ERROR_BUFFER_OVERFLOW;
        }

        cmd.op_code.raw = fw_op_code;
        cmd.parameter_length = fw_parameter_length;
        cmd.parameters = firmware_ptr;
        
        status = HCI_send_command(&cmd);
        if (status != HCI_ERROR_SUCCESS) {
            return status;
        }

        firmware_ptr += fw_parameter_length;
    }

    cmd.op_code.raw = CMD_BROADCOM_LAUNCH_RAM;
    cmd.parameter_length = 0;
    cmd.parameters = NULL;
    
    status = HCI_send_command(&cmd);
    if (status != HCI_ERROR_SUCCESS) {
        return status;
    }

    hw_delay_ms(1000);
}

HCI_Error HCI_bcm4345_set_baudrate(uint32_t baudrate) {
    uint8_t params[6];
    params[0] = baudrate & 0xFF;
    params[1] = (baudrate >> 8) & 0xFF;
    params[2] = (baudrate >> 16) & 0xFF;
    params[3] = (baudrate >> 24) & 0xFF;
    params[4] = 0x00;
    params[5] = 0x00;

    HCICommand cmd = {
        .op_code.raw = CMD_BROADCOM_UPDATE_BAUDRATE,
        .parameter_length = 6,
        .parameters = params
    };

    return HCI_send_command(&cmd);
}

HCI_Error HCI_set_bt_addr(uint8_t *bt_addr) {
    if (bt_addr == NULL) {
        return HCI_ERROR_INVALID_PARAMETERS;
    }

    uint8_t reversed_bt_addr[6];
    for (uint8_t i = 0; i < sizeof(reversed_bt_addr); i++) {
        reversed_bt_addr[i] = bt_addr[5 - i];
    }

    HCICommand cmd = {
        .op_code.raw = CMD_BROADCOM_WRITE_BD_ADDR,
        .parameter_length = 6,
        .parameters = reversed_bt_addr
    };

    return HCI_send_command(&cmd);
}

/***************************************************************************************
 * Bluetooth Init/Reset
 **************************************************************************************/
HCI_Error HCI_init(void) {
    hw_init();
    hci_state = HCI_STATE_ON;
    return HCI_ERROR_SUCCESS;
}

HCI_Error HCI_reset(void) {
    HCICommand reset_command = {
        .op_code.raw = CMD_BT_RESET,
        .parameter_length = 0U,
        .parameters = NULL,
    };

    HCI_Error status = HCI_send_command(&reset_command);
    while (hci_state != HCI_STATE_ON);
}
