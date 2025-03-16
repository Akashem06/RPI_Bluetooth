#pragma once

#include <stdint.h>

#include "hci.h"

#define MAX_SERVICES 10                    /**< Maximum number of services that can be registered */
#define MAX_CHARACTERISTICS_PER_SERVICE 10 /**< Maximum number of characteristics per service */
#define MAX_VALUE_LENGTH 128               /**< Maximum length of characteristic value */

#define GATT_PRIMARY_SERVICE_UUID 0x2800   /**< UUID for primary service declaration */
#define GATT_SECONDARY_SERVICE_UUID 0x2801 /**< UUID for secondary service declaration */
#define GATT_CHARACTERISTIC_UUID 0x2803    /**< UUID for characteristic declaration */

/** Default MTU size for ATT protocol */
#define ATT_DEFAULT_MTU 23

/** Maximum supported MTU size */
#define ATT_MAX_MTU 517

/* L2CAP packet with ATT protocol CID */
#define L2CAP_ATT_CID 0x0004

typedef enum {
  ATT_ERROR_RESPONSE = 0x01,              /**< Error Response */
  ATT_EXCHANGE_MTU_REQUEST = 0x02,        /**< Exchange MTU Request */
  ATT_EXCHANGE_MTU_RESPONSE = 0x03,       /**< Exchange MTU Response */
  ATT_FIND_INFORMATION_REQUEST = 0x04,    /**< Find Information Request */
  ATT_FIND_INFORMATION_RESPONSE = 0x05,   /**< Find Information Response */
  ATT_FIND_BY_TYPE_VALUE_REQUEST = 0x06,  /**< Find By Type Value Request */
  ATT_FIND_BY_TYPE_VALUE_RESPONSE = 0x07, /**< Find By Type Value Response */
  ATT_READ_BY_TYPE_REQUEST = 0x08,        /**< Read By Type Request */
  ATT_READ_BY_TYPE_RESPONSE = 0x09,       /**< Read By Type Response */
  ATT_READ_REQUEST = 0x0A,                /**< Read Request */
  ATT_READ_RESPONSE = 0x0B,               /**< Read Response */
  ATT_READ_BLOB_REQUEST = 0x0C,           /**< Read Blob Request */
  ATT_READ_BLOB_RESPONSE = 0x0D,          /**< Read Blob Response */
  ATT_READ_MULTIPLE_REQUEST = 0x0E,       /**< Read Multiple Request */
  ATT_READ_MULTIPLE_RESPONSE = 0x0F,      /**< Read Multiple Response */
  ATT_READ_BY_GROUP_TYPE_REQUEST = 0x10,  /**< Read By Group Type Request */
  ATT_READ_BY_GROUP_TYPE_RESPONSE = 0x11, /**< Read By Group Type Response */
  ATT_WRITE_REQUEST = 0x12,               /**< Write Request */
  ATT_WRITE_RESPONSE = 0x13,              /**< Write Response */
  ATT_WRITE_COMMAND = 0x52,               /**< Write Command */
  ATT_SIGNED_WRITE_COMMAND = 0xD2,        /**< Signed Write Command */
  ATT_PREPARE_WRITE_REQUEST = 0x16,       /**< Prepare Write Request */
  ATT_PREPARE_WRITE_RESPONSE = 0x17,      /**< Prepare Write Response */
  ATT_EXECUTE_WRITE_REQUEST = 0x18,       /**< Execute Write Request */
  ATT_EXECUTE_WRITE_RESPONSE = 0x19,      /**< Execute Write Response */
  ATT_HANDLE_VALUE_NOTIFICATION = 0x1B,   /**< Handle Value Notification */
  ATT_HANDLE_VALUE_INDICATION = 0x1D,     /**< Handle Value Indication */
  ATT_HANDLE_VALUE_CONFIRMATION = 0x1E    /**< Handle Value Confirmation */
} ATT_PDUType;

typedef enum {
  GATT_PROP_BROADCAST = 0x01,         /**< Broadcast property */
  GATT_PROP_READ = 0x02,              /**< Read property */
  GATT_PROP_WRITE_NO_RESP = 0x04,     /**< Write Without Response property */
  GATT_PROP_WRITE = 0x08,             /**< Write property */
  GATT_PROP_NOTIFY = 0x10,            /**< Notify property */
  GATT_PROP_INDICATE = 0x20,          /**< Indicate property */
  GATT_PROP_AUTH_SIGNED_WRITE = 0x40, /**< Authenticated Signed Writes property */
  GATT_PROP_EXTENDED_PROPS = 0x80     /**< Extended Properties */
} GATTCharacteristicProperties;

typedef enum {
  GATT_PERM_NONE = 0x00,         /**< No permissions */
  GATT_PERM_READ = 0x01,         /**< Read permission */
  GATT_PERM_WRITE = 0x02,        /**< Write permission */
  GATT_PERM_READ_ENC = 0x04,     /**< Encrypted read permission */
  GATT_PERM_WRITE_ENC = 0x08,    /**< Encrypted write permission */
  GATT_PERM_READ_AUTHEN = 0x10,  /**< Authenticated read permission */
  GATT_PERM_WRITE_AUTHEN = 0x20, /**< Authenticated write permission */
  GATT_PERM_READ_AUTHOR = 0x40,  /**< Authorized read permission */
  GATT_PERM_WRITE_AUTHOR = 0x80  /**< Authorized write permission */
} GATTCharacteristicPermissions;

typedef enum {
  GATT_EVENT_READ_REQUEST,                /**< Read request event */
  GATT_EVENT_WRITE_REQUEST,               /**< Write request event */
  GATT_EVENT_NOTIFICATION,                /**< Notification event */
  GATT_EVENT_INDICATION,                  /**< Indication event */
  GATT_EVENT_CONNECTION_COMPLETE,         /**< Connection complete event */
  GATT_EVENT_DISCONNECTION_COMPLETE,      /**< Disconnection complete event */
  GATT_EVENT_MTU_EXCHANGE_COMPLETE,       /**< MTU exchange complete event */
  GATT_EVENT_SERVICE_DISCOVERED,          /**< Service discovered event */
  GATT_EVENT_CHARACTERISTIC_DISCOVERED,   /**< Characteristic discovered event */
  GATT_EVENT_READ_RESPONSE,               /**< Read response event */
  GATT_EVENT_WRITE_RESPONSE,              /**< Write response event */
  GATT_EVENT_READ_BY_TYPE_RESPONSE,       /**< Read by type response event */
  GATT_EVENT_READ_BY_GROUP_TYPE_RESPONSE, /**< Read by group type response event */
  GATT_EVENT_FIND_INFORMATION_RESPONSE,   /**< Find information response event */
  GATT_EVENT_FIND_BY_TYPE_VALUE_RESPONSE, /**< Find by type value response event */
  GATT_EVENT_ERROR,                       /**< Error response event */
  GATT_EVENT_MTU_EXCHANGE,                /**< MTU exchange event */
  GATT_EVENT_ENCRYPTION_CHANGE,           /**< Encryption change event */
  GATT_EVENT_DISCONNECTION,               /**< Disconnection event */
  GATT_EVENT_UNKNOWN                      /**< Unknown GATT event */
} GATTEventType;

typedef enum {
  GATT_ERROR_SUCCESS = 0x00,                    /**< Success */
  GATT_ERROR_INVALID_HANDLE = 0x01,             /**< Invalid handle */
  GATT_ERROR_READ_NOT_PERMITTED = 0x02,         /**< Read not permitted */
  GATT_ERROR_WRITE_NOT_PERMITTED = 0x03,        /**< Write not permitted */
  GATT_ERROR_INVALID_PDU = 0x04,                /**< Invalid PDU */
  GATT_ERROR_INSUFFICIENT_AUTH = 0x05,          /**< Insufficient authentication */
  GATT_ERROR_REQUEST_NOT_SUPPORTED = 0x06,      /**< Request not supported */
  GATT_ERROR_INVALID_OFFSET = 0x07,             /**< Invalid offset */
  GATT_ERROR_INSUFFICIENT_AUTHORIZATION = 0x08, /**< Insufficient authorization */
  GATT_ERROR_PREPARE_QUEUE_FULL = 0x09,         /**< Prepare queue full */
  GATT_ERROR_ATTRIBUTE_NOT_FOUND = 0x0A,        /**< Attribute not found */
  GATT_ERROR_ATTRIBUTE_NOT_LONG = 0x0B,         /**< Attribute not long */
  GATT_ERROR_INSUFFICIENT_KEY_SIZE = 0x0C,      /**< Insufficient encryption key size */
  GATT_ERROR_INVALID_VALUE_LENGTH = 0x0D,       /**< Invalid attribute value length */
  GATT_ERROR_UNLIKELY = 0x0E,                   /**< Unlikely error */
  GATT_ERROR_INSUFFICIENT_ENCRYPTION = 0x0F,    /**< Insufficient encryption */
  GATT_ERROR_UNSUPPORTED_GROUP_TYPE = 0x10,     /**< Unsupported group type */
  GATT_ERROR_INSUFFICIENT_RESOURCES = 0x11,     /**< Insufficient resources */
  GATT_ERROR_DB_OUT_OF_SYNC = 0x12,             /**< Database out of sync */
  GATT_ERROR_VALUE_NOT_ALLOWED = 0x13,          /**< Value not allowed */
  GATT_ERROR_APPLICATION = 0x80,                /**< Application error */
  GATT_ERROR_INVALID_PARAMETER = 0x81,          /**< Invalid parameter */
  GATT_ERROR_OUT_OF_MEMORY = 0x82,              /**< Out of memory */
  GATT_ERROR_NOT_INITIALIZED = 0x83,            /**< GATT not initialized */
  GATT_ERROR_BUSY = 0x84,                       /**< GATT busy */
  GATT_ERROR_TIMEOUT = 0x85                     /**< Operation timed out */
} GATTError;

typedef enum {
  GATT_NOTIFY = 0x01U,  /**< Enable notifications */
  GATT_INDICATE = 0x02U /**< Enable indications */
} GATTNotificationType;

/**
 * @brief   Structure representing a GATT event
 */
typedef struct {
  GATTEventType type;         /**< Type of the event */
  uint16_t connection_handle; /**< Connection handle */
  uint16_t attribute_handle;  /**< Attribute handle */
  uint16_t offset;            /**< Offset for read/write operations */
  uint16_t length;            /**< Length of data */
  uint8_t *data;              /**< Pointer to data */

  union {
    struct {
      uint16_t mtu; /**< Negotiated MTU size */
    } mtu_exchange;

    struct {
      uint16_t start_handle; /**< Service start handle */
      uint16_t end_handle;   /**< Service end handle */
      uint16_t uuid;         /**< Service UUID */
      bool is_primary;       /**< Whether service is primary */
    } service_discovery;

    struct {
      uint16_t handle;                         /**< Characteristic handle */
      uint16_t value_handle;                   /**< Characteristic value handle */
      uint16_t uuid;                           /**< Characteristic UUID */
      GATTCharacteristicProperties properties; /**< Characteristic properties */
    } characteristic_discovery;
  } params; /**< Additional parameters for specific events */
} GATTEvent;

/**
 * @brief   Structure representing a GATT characteristic
 * @details Stores the characteristic declaration and value
 */
typedef struct {
  uint16_t handle;                           /**< Declaration handle */
  uint16_t uuid;                             /**< UUID of the characteristic */
  GATTCharacteristicProperties properties;   /**< Properties of the characteristic */
  GATTCharacteristicPermissions permissions; /**< Permissions of the characteristic */
  uint16_t value_handle;                     /**< Value handle */
  uint8_t value[MAX_VALUE_LENGTH];           /**< Characteristic value buffer */
  uint16_t value_length;                     /**< Current length of the value */
} GATTCharacteristic;

/**
 * @brief   Structure representing a GATT service
 */
typedef struct {
  uint16_t handle;                                                     /**< Service declaration handle */
  uint16_t uuid;                                                       /**< UUID of the service */
  bool is_primary;                                                     /**< Whether service is primary */
  GATTCharacteristic characteristics[MAX_CHARACTERISTICS_PER_SERVICE]; /**< Array of characteristics */
  uint8_t characteristic_count;                                        /**< Number of characteristics in the service */
  uint16_t end_handle;                                                 /**< Service end handle */
} GATTService;

/**
 * @typedef GATTEventCallback
 * @brief   Callback function type for GATT events
 * @param   event Pointer to the event structure
 */
typedef void (*GATTEventCallback)(GATTEvent *event);

/**
 * @brief   Initialize the GATT module
 * @details Sets up internal data structures and prepares the GATT module for operation
 * @return  GATT_ERROR_SUCCESS if successful, or an appropriate error code
 */
GATTError GATT_init(void);

/**
 * @brief   Deinitialize the GATT module
 * @details Cleans up resources and prepares the GATT module for shutdown
 * @return  GATT_ERROR_SUCCESS if successful, or an appropriate error code
 */
GATTError GATT_deinit(void);

/**
 * @brief   Register a service in the local GATT database
 * @details Creates a new service with the specified UUID and adds it to the local GATT database
 * @param   uuid UUID of the service to register
 * @param   is_primary true if this is a primary service, false for secondary service
 * @return  GATT_ERROR_SUCCESS if successful, or an appropriate error code
 */
GATTError GATT_register_service(uint16_t uuid, bool is_primary);

/**
 * @brief   Remove a registered service from the local GATT database
 * @details Removes a service and all its characteristics from the local GATT database
 * @param   service_uuid UUID of the service to remove
 * @return  GATT_ERROR_SUCCESS if successful, or an appropriate error code
 */
GATTError GATT_remove_service(uint16_t service_uuid);

/**
 * @brief   Add a characteristic to a registered service
 * @details Creates and adds a characteristic to the specified service
 * @param   service_uuid UUID of the service to which the characteristic should be added
 * @param   char_uuid UUID of the characteristic to add
 * @param   properties Bit mask of the characteristic properties
 * @param   permissions Bit mask of the characteristic permissions
 * @param   initial_value Pointer to the initial value of the characteristic
 * @param   value_length Length of the initial value
 * @return  GATT_ERROR_SUCCESS if successful, or an appropriate error code
 */
GATTError GATT_add_characteristic(uint16_t service_uuid, uint16_t char_uuid, GATTCharacteristicProperties properties,
                                  GATTCharacteristicPermissions permissions, uint8_t *initial_value, uint16_t value_length);

/**
 * @brief   Update the value of a local characteristic
 * @details Updates the value of a characteristic in the local GATT database
 * @param   service_uuid UUID of the service containing the characteristic
 * @param   char_uuid UUID of the characteristic to update
 * @param   value Pointer to the new value
 * @param   length Length of the new value
 * @return  GATT_ERROR_SUCCESS if successful, or an appropriate error code
 */
GATTError GATT_update_characteristic_value(uint16_t service_uuid, uint16_t char_uuid, uint8_t *value, uint16_t length);

/**
 * @brief   Read the value of a local characteristic
 * @details Retrieves the value of a characteristic from the local GATT database
 * @param   service_uuid UUID of the service containing the characteristic
 * @param   char_uuid UUID of the characteristic to read
 * @param   buffer Pointer to the buffer where the value will be stored
 * @param   length Pointer to store the length of the read value; on input contains buffer size
 * @return  GATT_ERROR_SUCCESS if successful, or an appropriate error code
 */
GATTError GATT_read_characteristic_value(uint16_t service_uuid, uint16_t char_uuid, uint8_t *buffer, uint16_t *length);

/**
 * @brief   Send a notification for a characteristic value
 * @details Sends a notification of a characteristic value change to a connected device
 * @param   connection_handle Connection handle of the target device
 * @param   char_handle Handle of the characteristic whose value has changed
 * @param   value Pointer to the new value to notify
 * @param   length Length of the new value
 * @return  GATT_ERROR_SUCCESS if successful, or an appropriate error code
 */
GATTError GATT_send_notification(uint16_t connection_handle, uint16_t char_handle, uint8_t *value, uint16_t length);

/**
 * @brief   Send an indication for a characteristic value
 * @details Sends an indication of a characteristic value change to a connected device and waits for
 * confirmation
 * @param   connection_handle Connection handle of the target device
 * @param   char_handle Handle of the characteristic whose value has changed
 * @param   value Pointer to the new value to indicate
 * @param   length Length of the new value
 * @return  GATT_ERROR_SUCCESS if successful, or an appropriate error code
 */
GATTError GATT_send_indication(uint16_t connection_handle, uint16_t char_handle, uint8_t *value, uint16_t length);

/**
 * @brief   Discover services on a remote device
 * @details Initiates GATT service discovery procedure on a connected remote device
 * @param   connection_handle Connection handle of the remote device
 * @return  GATT_ERROR_SUCCESS if discovery initiated successfully, or an appropriate error code
 */
GATTError GATT_discover_services(uint16_t connection_handle);

/**
 * @brief   Discover characteristics within a service range
 * @details Initiates GATT characteristic discovery procedure within the specified handle range
 * @param   connection_handle Connection handle of the remote device
 * @param   start_handle Start handle for the discovery range
 * @param   end_handle End handle for the discovery range
 * @return  GATT_ERROR_SUCCESS if discovery initiated successfully, or an appropriate error code
 */
GATTError GATT_discover_characteristics(uint16_t connection_handle, uint16_t start_handle, uint16_t end_handle);

/**
 * @brief   Subscribe to notifications or indications on a remote characteristic
 * @details Writes to the Client Characteristic Configuration Descriptor to enable
 * notifications/indications
 * @param   connection_handle Connection handle of the remote device
 * @param   char_handle Handle of the characteristic to subscribe to
 * @param   notification_type 1 for notifications, 2 for indications, 3 for both
 * @return  GATT_ERROR_SUCCESS if subscribe request sent successfully, or an appropriate error code
 */
GATTError GATT_subscribe_characteristic(uint16_t connection_handle, uint16_t char_handle,
                                        GATTNotificationType notification_type);

/**
 * @brief   Unsubscribe from notifications or indications on a remote characteristic
 * @details Writes to the Client Characteristic Configuration Descriptor to disable
 * notifications/indications
 * @param   connection_handle Connection handle of the remote device
 * @param   char_handle Handle of the characteristic to unsubscribe from
 * @return  GATT_ERROR_SUCCESS if unsubscribe request sent successfully, or an appropriate error
 * code
 */
GATTError GATT_unsubscribe_characteristic(uint16_t connection_handle, uint16_t char_handle);

/**
 * @brief   Read a characteristic value from a remote device
 * @details Sends a read request for a characteristic value on a connected remote device
 * @param   connection_handle Connection handle of the remote device
 * @param   char_handle Handle of the characteristic to read
 * @return  GATT_ERROR_SUCCESS if read request sent successfully, or an appropriate error code
 */
GATTError GATT_read_characteristic(uint16_t connection_handle, uint16_t char_handle);

/**
 * @brief   Write a value to a characteristic on a remote device
 * @details Sends a write request for a characteristic value on a connected remote device
 * @param   connection_handle Connection handle of the remote device
 * @param   char_handle Handle of the characteristic to write
 * @param   value Pointer to the value to write
 * @param   length Length of the value to write
 * @return  GATT_ERROR_SUCCESS if write request sent successfully, or an appropriate error code
 */
GATTError GATT_write_characteristic(uint16_t connection_handle, uint16_t char_handle, uint8_t *value, uint16_t length);

/**
 * @brief   Read a descriptor value from a remote device
 * @details Sends a read request for a descriptor value on a connected remote device
 * @param   connection_handle Connection handle of the remote device
 * @param   desc_handle Handle of the descriptor to read
 * @return  GATT_ERROR_SUCCESS if read request sent successfully, or an appropriate error code
 */
GATTError GATT_read_descriptor(uint16_t connection_handle, uint16_t desc_handle);

/**
 * @brief   Write a value to a descriptor on a remote device
 * @details Sends a write request for a descriptor value on a connected remote device
 * @param   connection_handle Connection handle of the remote device
 * @param   desc_handle Handle of the descriptor to write
 * @param   value Pointer to the value to write
 * @param   length Length of the value to write
 * @return  GATT_ERROR_SUCCESS if write request sent successfully, or an appropriate error code
 */
GATTError GATT_write_descriptor(uint16_t connection_handle, uint16_t desc_handle, uint8_t *value, uint16_t length);

/**
 * @brief   Exchange MTU with a remote device
 * @details Initiates the MTU exchange procedure with a connected remote device
 * @param   connection_handle Connection handle of the remote device
 * @param   client_mtu Client's desired MTU size
 * @return  GATT_ERROR_SUCCESS if exchange initiated successfully, or an appropriate error code
 */
GATTError GATT_exchange_mtu(uint16_t connection_handle, uint16_t client_mtu);

/**
 * @brief   Register a callback function for GATT events
 * @details Sets the callback function that will be called when GATT events occur
 * @param   callback Pointer to the callback function to register
 * @return  None
 */
void GATT_register_event_handler(GATTEventCallback callback);

/**
 * @brief   Process HCI events related to GATT
 * @details Handles HCI events that may affect GATT operations
 * @param   event Pointer to the HCI event to process
 * @return  None
 */
void GATT_handle_hci_event(HCIEvent *event);

/**
 * @brief   Process ACL data packets related to GATT
 * @details Handles ACL data packets that contain ATT/GATT protocol data
 * @param   acl_data Pointer to the ACL data packet to process
 * @return  None
 */
void GATT_handle_acl_data(HCIAsyncData *acl_data);

/**
 * @brief   Process incoming ATT packet
 * @details Handles an ATT protocol packet received from a remote device
 * @param   connection_handle Connection handle of the remote device
 * @param   packet Pointer to the ATT packet data
 * @param   length Length of the ATT packet
 * @return  None
 */
void GATT_process_att_packet(uint16_t connection_handle, uint8_t *packet, uint16_t length);
