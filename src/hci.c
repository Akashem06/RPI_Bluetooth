#include "hci.h"
#include "hardware.h"
#include <string.h>

static HCIState hci_state = HCI_STATE_IDLE;
static uint16_t hci_command_opcode = 0;

/***************************************************************************************
 * Packet serialization
 **************************************************************************************/
int HCI_encode_packet(HCIPacket packet_type, void *packet_data, uint8_t *buffer, uint16_t buffer_size) {
    uint16_t encoded_length = 0U;

    switch(packet_type) {
        case HCI_COMMAND_PACKET: {
            HCICommand *cmd = (HCICommand *)packet_data;
            if (buffer_size < 4 + cmd->paramter_length) {
                return 0;
            }

            buffer[0] = packet_type;
            buffer[1] = cmd->op_code.bit.command & 0xFF;
            buffer[2] = ((cmd->op_code.bit.command >> 8) & 0x03) | (cmd->op_code.bit.group << 2);
            buffer[3] = cmd->paramter_length;
            memcpy(&buffer[4], cmd->parameters, cmd->paramter_length);

            encoded_length = 4 + cmd->paramter_length;
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
    uint16_t packet_len = HCI_encode_packet(HCI_COMMAND_PACKET, data, packet, sizeof(packet));

    return HCI_ERROR_SUCCESS;
}

/***************************************************************************************
 * Event Handlers
 **************************************************************************************/

void HCI_handle_command_complete_event(uint8_t *parameters, uint8_t parameter_length) {
    if (parameter_length < 3) {
        HCI_handle_error(HCI_ERROR_INVALID_PARAMETERS);
        return;
    }

}

void HCI_handle_command_status_event(uint8_t *parameters, uint8_t parameter_length) {
    if (parameter_length < 4) {
        HCI_handle_error(HCI_ERROR_INVALID_PARAMETERS);
        return;
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
        .paramter_length = 0U,
        .parameters = NULL,
    };

    HCI_Error status = HCI_send_command(&reset_command);
    while (hci_state != HCI_STATE_ON);
}

HCIState HCI_get_state(void) {
    return hci_state;
}

void HCI_set_state(HCIState new_state) {
    hci_state = new_state;
}