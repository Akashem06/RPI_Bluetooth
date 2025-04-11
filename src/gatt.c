#include "gatt.h"

#include "hci_defs.h"
#include "mem_utils.h"

static GATTService gatt_services[MAX_SERVICES];
static uint8_t service_count = 0U;
static uint16_t next_handle = 1U;

static GATTEventCallback gatt_event_callback = NULL;

static GATTService *find_service_by_uuid(uint16_t uuid) {
  for (uint8_t i = 0; i < service_count; i++) {
    if (gatt_services[i].uuid == uuid) {
      return &gatt_services[i];
    }
  }
  return NULL;
}

static GATTCharacteristic *find_characteristic_by_uuid(GATTService *service, uint16_t uuid) {
  if (!service) return NULL;

  for (uint8_t i = 0; i < service->characteristic_count; i++) {
    if (service->characteristics[i].uuid == uuid) {
      return &service->characteristics[i];
    }
  }
  return NULL;
}

static GATTCharacteristic *find_characteristic_by_handle(uint16_t handle) {
  for (uint8_t i = 0; i < service_count; i++) {
    for (uint8_t j = 0; j < gatt_services[i].characteristic_count; j++) {
      if (gatt_services[i].characteristics[j].handle == handle ||
          gatt_services[i].characteristics[j].value_handle == handle) {
        return &gatt_services[i].characteristics[j];
      }
    }
  }
  return NULL;
}

GATTError GATT_init(void) {
  service_count = 0;
  next_handle = 1;
  gatt_event_callback = NULL;
  return GATT_ERROR_SUCCESS;
}

GATTError GATT_deinit(void) {
  for (uint8_t i = 0; i < MAX_SERVICES; i++) {
    memzero((uint64_t)&gatt_services[i], sizeof(GATTService));
  }

  gatt_event_callback = NULL;

  return GATT_ERROR_SUCCESS;
}

GATTError GATT_register_service(uint16_t uuid, bool is_primary) {
  if (service_count >= MAX_SERVICES) {
    return GATT_ERROR_INSUFFICIENT_RESOURCES;
  }

  gatt_services[service_count].uuid = uuid;
  gatt_services[service_count].is_primary = is_primary;
  gatt_services[service_count].handle = next_handle++;
  gatt_services[service_count].characteristic_count = 0U;

  service_count++;
  return GATT_ERROR_SUCCESS;
}

GATTError GATT_remove_service(uint16_t service_uuid) {
  GATTService *service = find_service_by_uuid(service_uuid);

  if (!service) {
    return GATT_ERROR_INVALID_HANDLE;
  }

  uint8_t service_index = 0;

  for (; service_index < service_count; service_index++) {
    if (gatt_services[service_index].uuid == service_uuid) {
      break;
    }
  }

  if (service_index >= service_count) {
    return GATT_ERROR_INVALID_HANDLE;
  }

  /* Shift all services after this one */
  for (uint8_t i = service_index; i < service_count - 1U; i++) {
    gatt_services[i] = gatt_services[i + 1];
  }

  service_count--;
  return GATT_ERROR_SUCCESS;
}

GATTError GATT_add_characteristic(uint16_t service_uuid, uint16_t char_uuid, GATTCharacteristicProperties properties,
                                  GATTCharacteristicPermissions permissions, uint8_t *initial_value, uint16_t value_length) {
  GATTService *service = find_service_by_uuid(service_uuid);
  if (!service) {
    return GATT_ERROR_INVALID_HANDLE;
  }

  if (service->characteristic_count >= MAX_CHARACTERISTICS_PER_SERVICE) {
    return GATT_ERROR_INSUFFICIENT_RESOURCES;
  }

  if (value_length > MAX_VALUE_LENGTH) {
    return GATT_ERROR_INVALID_VALUE_LENGTH;
  }

  GATTCharacteristic *characteristic = &service->characteristics[service->characteristic_count];
  characteristic->uuid = char_uuid;
  characteristic->properties = properties;
  characteristic->permissions = permissions;
  characteristic->handle = next_handle++;
  characteristic->value_handle = next_handle++;
  characteristic->value_length = value_length;

  if (initial_value && value_length > 0) {
    memcpy(characteristic->value, initial_value, value_length);
  }

  service->characteristic_count++;
  return GATT_ERROR_SUCCESS;
}

GATTError GATT_update_characteristic_value(uint16_t service_uuid, uint16_t char_uuid, uint8_t *value, uint16_t length) {
  if (!value || length > MAX_VALUE_LENGTH) {
    return GATT_ERROR_INVALID_VALUE_LENGTH;
  }

  GATTService *service = find_service_by_uuid(service_uuid);
  if (!service) {
    return GATT_ERROR_INVALID_HANDLE;
  }

  GATTCharacteristic *characteristic = find_characteristic_by_uuid(service, char_uuid);

  if (!characteristic) {
    return GATT_ERROR_INVALID_HANDLE;
  }

  memcpy(characteristic->value, value, length);
  characteristic->value_length = length;
  return GATT_ERROR_SUCCESS;
}

GATTError GATT_read_characteristic_value(uint16_t service_uuid, uint16_t char_uuid, uint8_t *buffer, uint16_t *length) {
  if (!buffer && !length) {
    return GATT_ERROR_INVALID_PARAMETER;
  }

  GATTService *service = find_service_by_uuid(service_uuid);

  if (!service) {
    return GATT_ERROR_INVALID_HANDLE;
  }

  GATTCharacteristic *characteristic = find_characteristic_by_uuid(service, char_uuid);

  if (!characteristic) {
    return GATT_ERROR_INVALID_HANDLE;
  }

  memcpy(buffer, characteristic->value, characteristic->value_length);
  *length = characteristic->value_length;

  return GATT_ERROR_SUCCESS;
}

GATTError GATT_send_notification(uint16_t connection_handle, uint16_t char_handle, uint8_t *value, uint16_t length) {
  if (!value || length > MAX_VALUE_LENGTH) {
    return GATT_ERROR_INVALID_VALUE_LENGTH;
  }

  GATTCharacteristic *characteristic = find_characteristic_by_handle(char_handle);

  if (!characteristic) {
    return GATT_ERROR_INVALID_HANDLE;
  }

  if (!(characteristic->properties & GATT_PROP_NOTIFY)) {
    return GATT_ERROR_REQUEST_NOT_SUPPORTED;
  }

  static uint8_t packet[MAX_VALUE_LENGTH + 3];
  packet[0] = ATT_HANDLE_VALUE_NOTIFICATION;
  packet[1] = char_handle & 0xFFU;
  packet[2] = (char_handle >> 8U) & 0xFFU;
  memcpy(&packet[3], value, length);

  HCIAsyncData acl_data = {
    .connection_handle = connection_handle, .pb_flag = 0, .bc_flag = 0, .data_total_length = length + 3U, .data = packet
  };

  return (HCI_send_async_data(&acl_data) == HCI_ERROR_SUCCESS) ? GATT_ERROR_SUCCESS : GATT_ERROR_INSUFFICIENT_RESOURCES;
}

GATTError GATT_send_indication(uint16_t connection_handle, uint16_t char_handle, uint8_t *value, uint16_t length) {
  if (!value || length > MAX_VALUE_LENGTH) {
    return GATT_ERROR_INVALID_VALUE_LENGTH;
  }

  GATTCharacteristic *characteristic = find_characteristic_by_handle(char_handle);

  if (!characteristic) {
    return GATT_ERROR_INVALID_HANDLE;
  }

  if (!(characteristic->properties & GATT_PROP_INDICATE)) {
    return GATT_ERROR_REQUEST_NOT_SUPPORTED;
  }

  static uint8_t packet[MAX_VALUE_LENGTH + 3];
  packet[0] = ATT_HANDLE_VALUE_INDICATION;
  packet[1] = char_handle & 0xFFU;
  packet[2] = (char_handle >> 8U) & 0xFFU;
  memcpy(&packet[3], value, length);

  HCIAsyncData acl_data = {
    .connection_handle = connection_handle, .pb_flag = 0, .bc_flag = 0, .data_total_length = length + 3U, .data = packet
  };

  return (HCI_send_async_data(&acl_data) == HCI_ERROR_SUCCESS) ? GATT_ERROR_SUCCESS : GATT_ERROR_INSUFFICIENT_RESOURCES;
}

GATTError GATT_discover_services(uint16_t connection_handle) {
  static uint8_t packet[7];

  /* https://community.nxp.com/t5/Wireless-MCU/Bluetooth-Low-Energy-How-to-discover-all-Primary-Services-on-a/m-p/378974 */
  packet[0] = ATT_READ_BY_GROUP_TYPE_REQUEST;
  packet[1] = 0x00U; /* Start handle lower byte */
  packet[2] = 0x01U; /* Start handle upper byte */
  packet[3] = 0xFFU; /* End handle lower byte */
  packet[4] = 0xFFU; /* End handle upper byte */
  packet[5] = GATT_PRIMARY_SERVICE_UUID & 0xFFU;
  packet[6] = (GATT_PRIMARY_SERVICE_UUID >> 8U) & 0xFFU;

  HCIAsyncData acl_data = {
    .connection_handle = connection_handle, .pb_flag = 0, .bc_flag = 0, .data_total_length = sizeof(packet), .data = packet
  };

  if (HCI_send_async_data(&acl_data) != HCI_ERROR_SUCCESS) {
    return GATT_ERROR_INSUFFICIENT_RESOURCES;
  }

  return GATT_ERROR_SUCCESS;
}

GATTError GATT_discover_characteristics(uint16_t connection_handle, uint16_t start_handle, uint16_t end_handle) {
  static uint8_t packet[7];

  packet[0] = ATT_READ_BY_TYPE_REQUEST;
  packet[1] = start_handle & 0xFFU;         /* Start handle lower byte */
  packet[2] = (start_handle >> 8U) & 0xFFU; /* Start handle upper byte */
  packet[3] = end_handle & 0xFFU;           /* End handle lower byte */
  packet[4] = (end_handle >> 8U) & 0xFFU;   /* End handle upper byte */
  packet[5] = GATT_CHARACTERISTIC_UUID & 0xFFU;
  packet[6] = (GATT_CHARACTERISTIC_UUID >> 8U) & 0xFFU;

  HCIAsyncData acl_data = {
    .connection_handle = connection_handle, .pb_flag = 0, .bc_flag = 0, .data_total_length = sizeof(packet), .data = packet
  };

  if (HCI_send_async_data(&acl_data) != HCI_ERROR_SUCCESS) {
    return GATT_ERROR_INSUFFICIENT_RESOURCES;
  }

  return GATT_ERROR_SUCCESS;
}

GATTError GATT_subscribe_characteristic(uint16_t connection_handle, uint16_t char_handle,
                                        GATTNotificationType notification_type) {
  GATTCharacteristic *characteristic = find_characteristic_by_handle(char_handle);

  if (!characteristic) {
    return GATT_ERROR_INVALID_HANDLE;
  }

  if (!(characteristic->properties & (GATT_PROP_INDICATE | GATT_PROP_NOTIFY))) {
    return GATT_ERROR_REQUEST_NOT_SUPPORTED;
  }

  /* CCCD Handle is 2 after the char handle (declaration, value, then description) */
  uint16_t cccd_handle = char_handle + 2;
  uint8_t cccd_value[2] = { 0U };

  if (notification_type == GATT_NOTIFY) {
    /* Enable notifications: 0x0001 */
    cccd_value[0] = 0x01U;
    cccd_value[1] = 0x00U;
  } else if (notification_type == GATT_INDICATE) {
    /* Enable indications: 0x0002 */
    cccd_value[0] = 0x02U;
    cccd_value[1] = 0x00U;
  } else {
    return GATT_ERROR_INVALID_PARAMETER;
  }

  static uint8_t packet[5];
  packet[0] = ATT_WRITE_REQUEST;
  packet[1] = cccd_handle & 0xFFU;
  packet[2] = (cccd_handle >> 8U) & 0xFFU;
  packet[3] = cccd_value[0];
  packet[4] = cccd_value[1];

  HCIAsyncData acl_data = {
    .connection_handle = connection_handle, .pb_flag = 0, .bc_flag = 0, .data_total_length = sizeof(packet), .data = packet
  };

  if (HCI_send_async_data(&acl_data) != HCI_ERROR_SUCCESS) {
    return GATT_ERROR_INSUFFICIENT_RESOURCES;
  }

  return GATT_ERROR_SUCCESS;
}

GATTError GATT_unsubscribe_characteristic(uint16_t connection_handle, uint16_t char_handle) {
  GATTCharacteristic *characteristic = find_characteristic_by_handle(char_handle);

  if (!characteristic) {
    return GATT_ERROR_INVALID_HANDLE;
  }

  if (!(characteristic->properties & (GATT_PROP_INDICATE | GATT_PROP_NOTIFY))) {
    return GATT_ERROR_REQUEST_NOT_SUPPORTED;
  }

  /* CCCD Handle is 2 after the char handle (declaration, value, then description) */
  uint16_t cccd_handle = char_handle + 2;
  uint8_t cccd_value[2] = { 0U, 0U };

  static uint8_t packet[5];
  packet[0] = ATT_WRITE_REQUEST;
  packet[1] = cccd_handle & 0xFFU;
  packet[2] = (cccd_handle >> 8U) & 0xFFU;
  packet[3] = cccd_value[0];
  packet[4] = cccd_value[1];

  HCIAsyncData acl_data = {
    .connection_handle = connection_handle, .pb_flag = 0, .bc_flag = 0, .data_total_length = sizeof(packet), .data = packet
  };

  if (HCI_send_async_data(&acl_data) != HCI_ERROR_SUCCESS) {
    return GATT_ERROR_INSUFFICIENT_RESOURCES;
  }

  return GATT_ERROR_SUCCESS;
}

GATTError GATT_read_characteristic(uint16_t connection_handle, uint16_t char_handle) {
  GATTCharacteristic *characteristic = find_characteristic_by_handle(char_handle);

  if (!characteristic) {
    return GATT_ERROR_INVALID_HANDLE;
  }

  if (!(characteristic->properties & GATT_PROP_READ)) {
    return GATT_ERROR_REQUEST_NOT_SUPPORTED;
  }

  static uint8_t packet[3];
  packet[0] = ATT_READ_REQUEST;
  packet[1] = char_handle & 0xFF;
  packet[2] = (char_handle >> 8) & 0xFF;

  HCIAsyncData acl_data = {
    .connection_handle = connection_handle, .pb_flag = 0, .bc_flag = 0, .data_total_length = sizeof(packet), .data = packet
  };

  if (HCI_send_async_data(&acl_data) != HCI_ERROR_SUCCESS) {
    return GATT_ERROR_INSUFFICIENT_RESOURCES;
  }

  return GATT_ERROR_SUCCESS;
}

GATTError GATT_write_characteristic(uint16_t connection_handle, uint16_t char_handle, uint8_t *value, uint16_t length) {
  if (!value || length > MAX_VALUE_LENGTH) {
    return GATT_ERROR_INVALID_PARAMETER;
  }

  GATTCharacteristic *characteristic = find_characteristic_by_handle(char_handle);

  if (!characteristic) {
    return GATT_ERROR_INVALID_HANDLE;
  }

  if (!(characteristic->properties & GATT_PROP_WRITE)) {
    return GATT_ERROR_REQUEST_NOT_SUPPORTED;
  }

  static uint8_t packet[MAX_VALUE_LENGTH];
  packet[0] = ATT_WRITE_REQUEST;
  packet[1] = char_handle & 0xFFU;
  packet[2] = (char_handle >> 8U) & 0xFFU;
  memcpy(&packet[3], value, length);

  HCIAsyncData acl_data = {
    .connection_handle = connection_handle, .pb_flag = 0, .bc_flag = 0, .data_total_length = 3 + length, .data = packet
  };

  if (HCI_send_async_data(&acl_data) != HCI_ERROR_SUCCESS) {
    return GATT_ERROR_INSUFFICIENT_RESOURCES;
  }

  return GATT_ERROR_SUCCESS;
}

GATTError GATT_read_descriptor(uint16_t connection_handle, uint16_t desc_handle) {
  static uint8_t packet[3];
  packet[0] = ATT_READ_REQUEST;
  packet[1] = desc_handle & 0xFFU;
  packet[2] = (desc_handle >> 8U) & 0xFFU;

  HCIAsyncData acl_data = {
    .connection_handle = connection_handle, .pb_flag = 0, .bc_flag = 0, .data_total_length = sizeof(packet), .data = packet
  };

  if (HCI_send_async_data(&acl_data) != HCI_ERROR_SUCCESS) {
    return GATT_ERROR_INSUFFICIENT_RESOURCES;
  }

  return GATT_ERROR_SUCCESS;
}

GATTError GATT_write_descriptor(uint16_t connection_handle, uint16_t desc_handle, uint8_t *value, uint16_t length) {
  if (!value || length > MAX_VALUE_LENGTH) {
    return GATT_ERROR_INVALID_PARAMETER;
  }

  static uint8_t packet[MAX_VALUE_LENGTH];
  packet[0] = ATT_WRITE_REQUEST;
  packet[1] = desc_handle & 0xFFU;
  packet[2] = (desc_handle >> 8U) & 0xFFU;
  memcpy(&packet[3], value, length);

  HCIAsyncData acl_data = {
    .connection_handle = connection_handle, .pb_flag = 0, .bc_flag = 0, .data_total_length = 3 + length, .data = packet
  };

  if (HCI_send_async_data(&acl_data) != HCI_ERROR_SUCCESS) {
    return GATT_ERROR_INSUFFICIENT_RESOURCES;
  }

  return GATT_ERROR_SUCCESS;
}

GATTError GATT_exchange_mtu(uint16_t connection_handle, uint16_t client_mtu) {
  if (client_mtu < ATT_DEFAULT_MTU) {
    return GATT_ERROR_INVALID_PARAMETER;
  }

  static uint8_t packet[3];
  packet[0] = ATT_EXCHANGE_MTU_REQUEST;
  packet[1] = client_mtu & 0xFFU;
  packet[2] = (client_mtu >> 8U) & 0xFFU;

  HCIAsyncData acl_data = {
    .connection_handle = connection_handle, .pb_flag = 0, .bc_flag = 0, .data_total_length = sizeof(packet), .data = packet
  };

  if (HCI_send_async_data(&acl_data) != HCI_ERROR_SUCCESS) {
    return GATT_ERROR_INSUFFICIENT_RESOURCES;
  }

  return GATT_ERROR_SUCCESS;
}

void GATT_register_event_handler(GATTEventCallback callback) {
  gatt_event_callback = callback;
}

void GATT_handle_hci_event(HCIEvent *event) {
  if (!event) {
    return;
  }

  switch (event->event_code) {
    case EVNT_BT_DISCONNECTION_COMPLETE:
      if (gatt_event_callback) {
        GATTEvent gatt_event = { .type = GATT_EVENT_DISCONNECTION_COMPLETE,
                                 .connection_handle = event->parameters[1] | (event->parameters[2] << 8U),
                                 .attribute_handle = 0,
                                 .offset = 0,
                                 .length = 0,
                                 .data = NULL };
        gatt_event_callback(&gatt_event);
      }
      break;

    case EVNT_BT_ENCRYPTION_CHANGE:
      if (gatt_event_callback) {
        GATTEvent gatt_event = { .type = GATT_EVENT_ENCRYPTION_CHANGE,
                                 .connection_handle = event->parameters[1] | (event->parameters[2] << 8U),
                                 .attribute_handle = 0,
                                 .offset = 0,
                                 .length = 1,
                                 .data = &event->parameters[3] };
        gatt_event_callback(&gatt_event);
      }
      break;

    default:
      break;
  }
}

void GATT_handle_acl_data(HCIAsyncData *acl_data) {
  if (!acl_data || !acl_data->data || acl_data->data_total_length < 1) {
    return;
  }

  if (acl_data->data_total_length >= 5) {
    uint16_t l2cap_length = acl_data->data[0] | (acl_data->data[1] << 8);
    uint16_t l2cap_cid = acl_data->data[2] | (acl_data->data[3] << 8);

    if (l2cap_cid == L2CAP_ATT_CID) {
      GATT_process_att_packet(acl_data->connection_handle, &acl_data->data[4], l2cap_length);
    }
  }
}

void GATT_process_att_packet(uint16_t connection_handle, uint8_t *packet, uint16_t length) {
  if (!packet || length < 1) {
    return;
  }

  uint8_t opcode = packet[0];

  switch (opcode) {
    case ATT_ERROR_RESPONSE:
      if (length < 5) return;

      uint16_t handle = packet[2] | (packet[3] << 8U);

      if (gatt_event_callback) {
        GATTEvent event = { .type = GATT_EVENT_ERROR,
                            .connection_handle = connection_handle,
                            .attribute_handle = handle,
                            .offset = 0,
                            .length = length - 1,
                            .data = &packet[1] };
        gatt_event_callback(&event);
      }
      break;

    case ATT_EXCHANGE_MTU_RESPONSE:
      if (length < 3) return;

      uint16_t server_mtu = packet[1] | (packet[2] << 8U);

      if (gatt_event_callback) {
        GATTEvent event = { .type = GATT_EVENT_MTU_EXCHANGE,
                            .connection_handle = connection_handle,
                            .attribute_handle = 0,
                            .offset = 0,
                            .length = 2,
                            .data = &packet[1] };
        event.params.mtu_exchange.mtu = server_mtu;
        gatt_event_callback(&event);
      }
      break;

    case ATT_READ_RESPONSE:
      if (gatt_event_callback) {
        GATTEvent event = { .type = GATT_EVENT_READ_RESPONSE,
                            .connection_handle = connection_handle,
                            .attribute_handle = 0,
                            .offset = 0,
                            .length = length - 1,
                            .data = &packet[1] };
        gatt_event_callback(&event);
      }
      break;

    case ATT_READ_BY_TYPE_RESPONSE:
      /* Characteristic discovery */
      if (length < 2) return;

      uint8_t data_length = packet[1];

      if (gatt_event_callback) {
        GATTEvent event = { .type = GATT_EVENT_READ_BY_TYPE_RESPONSE,
                            .connection_handle = connection_handle,
                            .attribute_handle = 0, /* Stored in the data */
                            .offset = 0,
                            .length = length - 1,
                            .data = &packet[1] };
        gatt_event_callback(&event);
      }
      break;

    case ATT_READ_BY_GROUP_TYPE_RESPONSE:
      /* Service discovery */
      if (length < 2) return;

      data_length = packet[1];

      if (gatt_event_callback) {
        GATTEvent event = { .type = GATT_EVENT_READ_BY_GROUP_TYPE_RESPONSE,
                            .connection_handle = connection_handle,
                            .attribute_handle = 0, /* Stored in the data */
                            .offset = 0,
                            .length = length - 1,
                            .data = &packet[1] };

        if (data_length >= 6) {
          /* Process if there is atleast 6 bytes (From 16-bit UUID format) */
          for (uint16_t i = 2; i < length; i += data_length) {
            if (i + data_length <= length) {
              event.params.service_discovery.start_handle = packet[i] | (packet[i + 1] << 8U);
              event.params.service_discovery.end_handle = packet[i + 2] | (packet[i + 3] << 8U);
              event.params.service_discovery.uuid = packet[i + 4] | (packet[i + 5] << 8U);
              event.params.service_discovery.is_primary = true;

              gatt_event_callback(&event);
            }
          }
        }
      }
      break;

    case ATT_WRITE_RESPONSE:
      if (gatt_event_callback) {
        GATTEvent event = { .type = GATT_EVENT_WRITE_RESPONSE,
                            .connection_handle = connection_handle,
                            .attribute_handle = 0,
                            .offset = 0,
                            .length = 0,
                            .data = NULL };
        gatt_event_callback(&event);
      }
      break;

    case ATT_HANDLE_VALUE_NOTIFICATION:
      if (length < 3) return;

      uint16_t char_handle = packet[1] | (packet[2] << 8);

      if (gatt_event_callback) {
        GATTEvent event = { .type = GATT_EVENT_NOTIFICATION,
                            .connection_handle = connection_handle,
                            .attribute_handle = char_handle,
                            .offset = 0,
                            .length = length - 3,
                            .data = &packet[3] };
        gatt_event_callback(&event);
      }
      break;

    case ATT_HANDLE_VALUE_INDICATION:
      if (length < 3) return;

      char_handle = packet[1] | (packet[2] << 8);

      uint8_t confirm_packet[1] = { ATT_HANDLE_VALUE_CONFIRMATION };

      HCIAsyncData confirm_data = {
        .connection_handle = connection_handle, .pb_flag = 0, .bc_flag = 0, .data_total_length = 1, .data = confirm_packet
      };

      HCI_send_async_data(&confirm_data);

      if (gatt_event_callback) {
        GATTEvent event = { .type = GATT_EVENT_INDICATION,
                            .connection_handle = connection_handle,
                            .attribute_handle = char_handle,
                            .offset = 0,
                            .length = length - 3,
                            .data = &packet[3] };
        gatt_event_callback(&event);
      }
      break;

    case ATT_FIND_INFORMATION_RESPONSE:
      if (length < 2) return;

      if (gatt_event_callback) {
        GATTEvent event = { .type = GATT_EVENT_FIND_INFORMATION_RESPONSE,
                            .connection_handle = connection_handle,
                            .attribute_handle = 0, /* Stored in the data */
                            .offset = 0,
                            .length = length - 1,
                            .data = &packet[1] };
        gatt_event_callback(&event);
      }
      break;

    case ATT_FIND_BY_TYPE_VALUE_RESPONSE:
      if (gatt_event_callback) {
        GATTEvent event = { .type = GATT_EVENT_FIND_BY_TYPE_VALUE_RESPONSE,
                            .connection_handle = connection_handle,
                            .attribute_handle = 0, /* Stored in the data */
                            .offset = 0,
                            .length = length - 1,
                            .data = &packet[1] };
        gatt_event_callback(&event);
      }
      break;

    default:
      if (gatt_event_callback) {
        GATTEvent event = { .type = GATT_EVENT_UNKNOWN,
                            .connection_handle = connection_handle,
                            .attribute_handle = 0,
                            .offset = 0,
                            .length = length,
                            .data = packet };
        gatt_event_callback(&event);
      }
      break;
  }
}
