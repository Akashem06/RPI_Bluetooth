#pragma once

#include <stdint.h>

#include "hci.h"

#define MAX_SERVICES 10
#define MAX_CHARACTERISTICS_PER_SERVICE 10
#define MAX_VALUE_LENGTH 512

#define GATT_PRIMARY_SERVICE_UUID 0x2800
#define GATT_SECONDARY_SERVICE_UUID 0x2801
#define GATT_CHARACTERISTIC_UUID 0x2803

typedef enum {
  ATT_ERROR_RESPONSE = 0x01,
  ATT_EXCHANGE_MTU_REQUEST = 0x02,
  ATT_EXCHANGE_MTU_RESPONSE = 0x03,
  ATT_FIND_INFORMATION_REQUEST = 0x04,
  ATT_FIND_INFORMATION_RESPONSE = 0x05,
  ATT_FIND_BY_TYPE_VALUE_REQUEST = 0x06,
  ATT_FIND_BY_TYPE_VALUE_RESPONSE = 0x07,
  ATT_READ_BY_TYPE_REQUEST = 0x08,
  ATT_READ_BY_TYPE_RESPONSE = 0x09,
  ATT_READ_REQUEST = 0x0A,
  ATT_READ_RESPONSE = 0x0B,
  ATT_READ_BLOB_REQUEST = 0x0C,
  ATT_READ_BLOB_RESPONSE = 0x0D,
  ATT_READ_MULTIPLE_REQUEST = 0x0E,
  ATT_READ_MULTIPLE_RESPONSE = 0x0F,
  ATT_WRITE_REQUEST = 0x12,
  ATT_WRITE_RESPONSE = 0x13,
  ATT_WRITE_COMMAND = 0x52,
  ATT_HANDLE_VALUE_NOTIFICATION = 0x1B,
  ATT_HANDLE_VALUE_INDICATION = 0x1D,
  ATT_HANDLE_VALUE_CONFIRMATION = 0x1E
} ATT_PDUType;

typedef enum {
  GATT_PROP_BROADCAST = 0x01,
  GATT_PROP_READ = 0x02,
  GATT_PROP_WRITE_NO_RESP = 0x04,
  GATT_PROP_WRITE = 0x08,
  GATT_PROP_NOTIFY = 0x10,
  GATT_PROP_INDICATE = 0x20,
  GATT_PROP_AUTH_SIGNED_WRITE = 0x40,
  GATT_PROP_EXTENDED_PROPS = 0x80
} GATTCharacteristicProperties;

typedef enum {
  GATT_PERM_NONE = 0x00,
  GATT_PERM_READ = 0x01,
  GATT_PERM_WRITE = 0x02,
  GATT_PERM_READ_ENC = 0x04,
  GATT_PERM_WRITE_ENC = 0x08
} GATTCharacteristicPermissions;

typedef enum {
  GATT_EVENT_READ_REQUEST,
  GATT_EVENT_WRITE_REQUEST,
  GATT_EVENT_NOTIFICATION,
  GATT_EVENT_INDICATION
} GATTEventType;

typedef enum {
  GATT_ERROR_SUCCESS = 0x00,
  GATT_ERROR_INVALID_HANDLE = 0x01,
  GATT_ERROR_READ_NOT_PERMITTED = 0x02,
  GATT_ERROR_WRITE_NOT_PERMITTED = 0x03,
  GATT_ERROR_INVALID_PDU = 0x04,
  GATT_ERROR_INSUFFICIENT_AUTH = 0x05,
  GATT_ERROR_REQUEST_NOT_SUPPORTED = 0x06,
  GATT_ERROR_INVALID_OFFSET = 0x07,
  GATT_ERROR_INSUFFICIENT_AUTHORIZATION = 0x08,
  GATT_ERROR_PREPARE_QUEUE_FULL = 0x09,
  GATT_ERROR_ATTRIBUTE_NOT_FOUND = 0x0A,
  GATT_ERROR_ATTRIBUTE_NOT_LONG = 0x0B,
  GATT_ERROR_INSUFFICIENT_KEY_SIZE = 0x0C,
  GATT_ERROR_INVALID_VALUE_LENGTH = 0x0D,
  GATT_ERROR_UNLIKELY = 0x0E,
  GATT_ERROR_INSUFFICIENT_ENCRYPTION = 0x0F,
  GATT_ERROR_UNSUPPORTED_GROUP_TYPE = 0x10,
  GATT_ERROR_INSUFFICIENT_RESOURCES = 0x11
} GATTError;

typedef struct {
  GATTEventType type;
  uint16_t connection_handle;
  uint16_t attribute_handle;
  uint16_t offset;
  uint16_t length;
  uint8_t *data;
} GATTEvent;

typedef struct {
  uint16_t handle;
  uint16_t uuid;
  GATTCharacteristicProperties properties;
  GATTCharacteristicPermissions permissions;
  uint16_t value_handle;
  uint8_t value[MAX_VALUE_LENGTH];
  uint16_t value_length;
} GATTCharacteristic;

typedef struct {
  uint16_t handle;
  uint16_t uuid;
  bool is_primary;
  GATTCharacteristic characteristics[MAX_CHARACTERISTICS_PER_SERVICE];
  uint8_t characteristic_count;
} GATTService;

typedef void (*GATTEventCallback)(GATTEvent *event);

/**
 * @brief
 * @details
 * @param
 * @return
 */
GATTError GATT_init(void);

/**
 * @brief
 * @details
 * @param
 * @return
 */
GATTError GATT_register_service(uint16_t uuid, bool is_primary);

/**
 * @brief
 * @details
 * @param
 * @return
 */
GATTError GATT_add_characteristic(uint16_t service_uuid, uint16_t char_uuid,
                                  GATTCharacteristicProperties properties,
                                  GATTCharacteristicPermissions permissions, uint8_t *initial_value,
                                  uint16_t value_length);

/**
 * @brief
 * @details
 * @param
 * @return
 */
GATTError GATT_update_characteristic_value(uint16_t service_uuid, uint16_t char_uuid,
                                           uint8_t *value, uint16_t length);

/**
 * @brief
 * @details
 * @param
 * @return
 */
GATTError GATT_read_characteristic_value(uint16_t service_uuid, uint16_t char_uuid, uint8_t *buffer,
                                         uint16_t *length);
/**
 * @brief
 * @details
 * @param
 * @return
 */
GATTError GATT_send_notification(uint16_t connection_handle, uint16_t char_handle, uint8_t *value,
                                 uint16_t length);

/**
 * @brief
 * @details
 * @param
 * @return
 */
GATTError GATT_send_indication(uint16_t connection_handle, uint16_t char_handle, uint8_t *value,
                               uint16_t length);

/**
 * @brief
 * @details
 * @param
 * @return
 */
GATTError GATT_discover_services(uint16_t connection_handle);

/**
 * @brief
 * @details
 * @param
 * @return
 */
GATTError GATT_discover_characteristics(uint16_t connection_handle, uint16_t start_handle,
                                        uint16_t end_handle);

/**
 * @brief
 * @details
 * @param
 * @return
 */
GATTError GATT_read_characteristic(uint16_t connection_handle, uint16_t char_handle);

/**
 * @brief
 * @details
 * @param
 * @return
 */
GATTError GATT_write_characteristic(uint16_t connection_handle, uint16_t char_handle,
                                    uint8_t *value, uint16_t length);

/**
 * @brief
 * @details
 * @param
 * @return
 */
void GATT_register_event_handler(GATTEventCallback callback);

/**
 * @brief
 * @details
 * @param
 * @return
 */
void GATT_handle_hci_event(HCIEvent *event);

/**
 * @brief
 * @details
 * @param
 * @return
 */
void GATT_handle_acl_data(HCIAsyncData *acl_data);