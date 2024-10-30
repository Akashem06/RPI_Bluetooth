#include "gatt.h"
#include "mm.h"

static GATTService gatt_services[MAX_SERVICES];
static uint8_t service_count = 0;
static uint16_t next_handle = 1;

static GATTEventCallback gatt_event_callback = NULL;

static GATTService* find_service_by_uuid(uint16_t uuid) {
    for (int i = 0; i < service_count; i++) {
        if (gatt_services[i].uuid == uuid) {
            return &gatt_services[i];
        }
    }
    return NULL;
}

static GATTCharacteristic* find_characteristic_by_uuid(GATTService *service, uint16_t uuid) {
    if (!service) return NULL;
    
    for (int i = 0; i < service->characteristic_count; i++) {
        if (service->characteristics[i].uuid == uuid) {
            return &service->characteristics[i];
        }
    }
    return NULL;
}

static GATTCharacteristic* find_characteristic_by_handle(uint16_t handle) {
    for (int i = 0; i < service_count; i++) {
        for (int j = 0; j < gatt_services[i].characteristic_count; j++) {
            if (gatt_services[i].characteristics[j].handle == handle ||
                gatt_services[i].characteristics[j].value_handle == handle) {
                return &gatt_services[i].characteristics[j];
            }
        }
    }
    return NULL;
}

// GATT Server Functions Implementation
GATTError GATT_init(void) {
    service_count = 0;
    next_handle = 1;
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
    gatt_services[service_count].characteristic_count = 0;

    service_count++;
    return GATT_ERROR_SUCCESS;
}

GATTError GATT_add_characteristic(uint16_t service_uuid, uint16_t char_uuid,
                                GATTCharacteristicProperties properties,
                                GATTCharacteristicPermissions permissions,
                                uint8_t *initial_value, uint16_t value_length) {
    
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

GATTError GATT_update_characteristic_value(uint16_t service_uuid, uint16_t char_uuid,
                                         uint8_t *value, uint16_t length) {
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

GATTError GATT_send_notification(uint16_t connection_handle, uint16_t char_handle,
                               uint8_t *value, uint16_t length) {
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

    uint8_t packet[MAX_VALUE_LENGTH + 3];
    packet[0] = ATT_HANDLE_VALUE_NOTIFICATION;
    packet[1] = char_handle & 0xFF;
    packet[2] = (char_handle >> 8) & 0xFF;
    memcpy(&packet[3], value, length);

    HCIAsyncData acl_data = {
        .connection_handle = connection_handle,
        .pb_flag = 0,
        .bc_flag = 0,
        .data_total_length = length + 3,
        .data = packet
    };

    return (HCI_send_async_data(&acl_data) == HCI_ERROR_SUCCESS) ? 
           GATT_ERROR_SUCCESS : GATT_ERROR_INSUFFICIENT_RESOURCES;
}