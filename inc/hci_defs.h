#pragma once

typedef enum {
    /* Bluetooth Commands. */
    CMD_BT_DISCONNECT                                               = 0x0406,
    CMD_BT_READ_REMOTE_VERSION_INFORMATION                          = 0x041D,
    CMD_BT_SET_EVENT_MASK                                           = 0x0C01,
    CMD_BT_RESET                                                    = 0x0C03,
    CMD_BT_READ_TRANSMIT_POWER_LEVEL                                = 0x0C2D,
    CMD_BT_SET_CONTROLLER_TO_HOST_FLOW_CONTROL                      = 0x0C31,
    CMD_BT_HOST_BUFFER_SIZE                                         = 0x0C33,
    CMD_BT_HOST_NUMBER_OF_COMPLETED_PACKETS                         = 0x0C35,
    CMD_BT_SET_EVENT_MASK_PAGE_2                                    = 0x0C63,
    CMD_BT_READ_AUTHENTICATED_PAYLOAD_TIMEOUT                       = 0x0C7B,
    CMD_BT_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT                      = 0x0C7C,
    CMD_BT_READ_LOCAL_VERSION_INFORMATION                           = 0x1001,
    CMD_BT_READ_LOCAL_SUPPORTED_COMMANDS                            = 0x1002,
    CMD_BT_READ_LOCAL_SUPPORTED_FEATURES                            = 0x1003,
    CMD_BT_READ_BD_ADDR                                             = 0x1009,
    CMD_BT_READ_RSSI                                                = 0x1405,

    /* BLE Commands. */
    CMD_BLE_SET_EVENT_MASK                                          = 0x2001,
    CMD_BLE_READ_BUFFER_SIZE                                        = 0x2002,
    CMD_BLE_READ_LOCAL_SUPPORTED_FEATURES                           = 0x2003,
    CMD_BLE_SET_RANDOM_ADDRESS                                      = 0x2005,
    CMD_BLE_SET_ADVERTISING_PARAMETERS                              = 0x2006,
    CMD_BLE_READ_ADVERTISING_CHANNEL_TX_POWER                       = 0x2007,
    CMD_BLE_SET_ADVERTISING_DATA                                    = 0x2008,
    CMD_BLE_SET_SCAN_RESPONSE_DATA                                  = 0x2009,
    CMD_BLE_SET_ADVERTISE_ENABLE                                    = 0x200A,
    CMD_BLE_SET_SCAN_PARAMETERS                                     = 0x200B,
    CMD_BLE_SET_SCAN_ENABLE                                         = 0x200C,
    CMD_BLE_CREATE_CONNECTION                                       = 0x200D,
    CMD_BLE_CREATE_CONNECTION_CANCEL                                = 0x200E,
    CMD_BLE_READ_WHITE_LIST_SIZE                                    = 0x200F,
    CMD_BLE_CLEAR_WHITE_LIST                                        = 0x2010,
    CMD_BLE_ADD_DEVICE_TO_WHITE_LIST                                = 0x2011,
    CMD_BLE_REMOVE_DEVICE_FROM_WHITE_LIST                           = 0x2012,
    CMD_BLE_CONNECTION_UPDATE                                       = 0x2013,
    CMD_BLE_SET_HOST_CHANNEL_CLASSIFICATION                         = 0x2014,
    CMD_BLE_READ_CHANNEL_MAP                                        = 0x2015,
    CMD_BLE_READ_REMOTE_USED_FEATURES                               = 0x2016,
    CMD_BLE_ENCRYPT                                                 = 0x2017,
    CMD_BLE_RAND                                                    = 0x2018,
    CMD_BLE_START_ENCRYPTION                                        = 0x2019,
    CMD_BLE_LONG_TERM_KEY_REQUEST_REPLY                             = 0x201A,
    CMD_BLE_LONG_TERM_KEY_REQUEST_NEGATIVE_REPLY                    = 0x201B,
    CMD_BLE_READ_SUPPORTED_STATES                                   = 0x201C,
    CMD_BLE_RECEIVER_TEST                                           = 0x201D,
    CMD_BLE_TRANSMITTER_TEST                                        = 0x201E,
    CMD_BLE_TEST_END                                                = 0x201F,
    CMD_BLE_REMOTE_CONNECTION_PARAMETER_REQUEST_REPLY               = 0x2020,
    CMD_BLE_REMOTE_CONNECTION_PARAMETER_REQUEST_NEGATIVE_REPLY      = 0x2021,
    CMD_BLE_SET_DATA_LENGTH                                         = 0x2022,
    CMD_BLE_READ_SUGGESTED_DEFAULT_DATA_LENGTH                      = 0x2023,
    CMD_BLE_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH                     = 0x2024,
    CMD_BLE_READ_LOCAL_P256_PUBLIC_KEY                              = 0x2025,
    CMD_BLE_GENERATE_DHKEY                                          = 0x2026,
    CMD_BLE_ADD_DEVICE_TO_RESOLVING_LIST                            = 0x2027,
    CMD_BLE_REMOVE_DEVICE_FROM_RESOLVING_LIST                       = 0x2028,
    CMD_BLE_CLEAR_RESOLVING_LIST                                    = 0x2029,
    CMD_BLE_READ_RESOLVING_LIST_SIZE                                = 0x202A,
    CMD_BLE_READ_PEER_RESOLVABLE_ADDRESS                            = 0x202B,
    CMD_BLE_READ_LOCAL_RESOLVABLE_ADDRESS                           = 0x202C,
    CMD_BLE_SET_ADDRESS_RESOLUTION_ENABLE                           = 0x202D,
    CMD_BLE_SET_RESOLVABLE_PRIVATE_ADDRESS_TIMEOUT                  = 0x202E,
    CMD_BLE_READ_MAXIMUM_DATA_LENGTH                                = 0x202F,

    /* Broadcom Vendor Commands. */
    CMD_BROADCOM_SET_SLEEP_MODE                                     = 0xFC27,
    CMD_BROADCOM_WRITE_SCO_PCM_INT_PARAM                            = 0xFC1C,
    CMD_BROADCOM_WRITE_PCM_DATA_FORMAT_PARAM                        = 0xFC1E,
    CMD_BROADCOM_ENABLE_WBS                                         = 0xFC7E,
    CMD_BROADCOM_SET_TX_POWER                                       = 0xFC0C,
    CMD_BROADCOM_ENABLE_UART_TL                                     = 0xFC09,
    CMD_BROADCOM_WRITE_BD_ADDR                                      = 0xFC01,
    CMD_BROADCOM_READ_VERBOSE_CONFIG_VERSION                        = 0xFC79,
    CMD_BROADCOM_WRITE_I2SPCM_INTERFACE_PARAM                       = 0xFC6D,
    CMD_BROADCOM_ENABLE_RF_CALIBRATION                              = 0xFC28,
    CMD_BROADCOM_SET_UART_BAUD_RATE                                 = 0xFC18,
    CMD_BROADCOM_DOWNLOAD_MINIDRIVER                                = 0xFC2E,
    CMD_BROADCOM_LAUNCH_RAM                                         = 0xFC4E,
    CMD_BROADCOM_WRITE_RAM                                          = 0xFC4C,
    CMD_BROADCOM_UPDATE_BAUDRATE                                    = 0xFC77,
    CMD_BROADCOM_COEX_WRITE_WIMAX_CONFIGURATION                     = 0xFC7A,
    CMD_BROADCOM_SET_COEXISTENCE_PARAMETERS                         = 0xFC0A,
    CMD_BROADCOM_ENABLE_CUSTOMER_SPECIFIC_FEATURE                   = 0xFC6E,
    
} HCI_CommandCode;

typedef enum {
    EVNT_BT_DISCONNECTION_COMPLETE                                  = 0x05,
    EVNT_BT_ENCRYPTION_CHANGE                                       = 0x08,
    EVNT_BT_READ_REMOTE_VERSION_INFO_COMPLETE                       = 0x0C,
    EVNT_BT_COMMAND_COMPLETE                                        = 0x0E,
    EVNT_BT_COMMAND_STATUS                                          = 0x0F,
    EVNT_BT_HARDWARE_ERROR                                          = 0x10,
    EVNT_BT_NUMBER_OF_COMPLETED_PACKETS                             = 0x13,
    EVNT_BT_DATA_BUFFER_OVERFLOW                                    = 0x1A,
    EVNT_BT_ENCRYPTION_KEY_REFRESH_COMPLETE                         = 0x30,
    EVNT_BLE_EVENT_CODE                                             = 0x3E,
    EVNT_BT_AUTHENTICATED_PAYLOAD_TIMEOUT_EXPIRED                   = 0x57
} HCI_EventCode;

typedef enum {
    SUB_EVNT_BLE_CONNECTION_COMPLETE = 0x01,
    SUB_EVNT_BLE_ADVERTISING_REPORT,
    SUB_EVNT_BLE_CONNECTION_UPDATE_COMPLETE,
    SUB_EVNT_BLE_READ_REMOTE_USED_FEATURES_COMPLETE,
    SUB_EVNT_BLE_LONG_TERM_KEY_REQUESTED,
    SUB_EVNT_BLE_REMOTE_CONNECTION_PARAMETER_REQUEST,
    SUB_EVNT_BLE_DATA_LENGTH_CHANGE,
    SUB_EVNT_BLE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE,
    SUB_EVNT_BLE_GENERATE_DHKEY_COMPLETE,
    SUB_EVNT_BLE_ENHANCED_CONNECTION_COMPLETED,
    SUB_EVNT_BLE_DIRECT_ADVERTISING_REPORT
} HCI_SubEventCode;

typedef enum {
    HCI_COMMAND_PACKET                                              = 0x01,
    HCI_ASYNC_DATA_PACKET,
    HCI_SYNC_DATA_PACKET,
    HCI_EVENT_PACKET,
    HCI_EXTENDED_COMMAND_PACKET
} HCIPacket;

/***************************************************************************************
 * Advertising defs
 **************************************************************************************/

typedef enum {
    ADV_TYPE_UNDIRECT_CONN,                 /** Connectable undirected advertising */
    ADV_TYPE_DIRECT_CONN,                   /** Connectable directed advertising */
    ADV_TYPE_UNDIRECT_SCAN,                 /** Scannable undirected advertising */
    ADV_TYPE_UNDIRECT_NONCONN               /** Non-connectable undirected advertising */
} Adv_Type;

typedef enum {
    ADV_OWN_ADDR_PUBLIC,                        /** Public Device Address */
    ADV_OWN_ADDR_RANDOM,                        /** Random Device Address */
} Adv_OwnAddressType;

typedef enum {
    ADV_DIR_ADDR_PUBLIC,                        /** Public Device Address */
    ADV_DIR_ADDR_RANDOM,                        /** Random Device Address */
} Adv_DirectAddressType;

typedef enum {
    ADV_CHANNEL_37 = 0x01,                      /** Channel 37 enabled */
    ADV_CHANNEL_38 = 0x02,                      /** Channel 38 enabled */
    ADV_CHANNEL_39 = 0x04,                      /** Channel 39 enabled */
} Adv_ChannelMap;

typedef enum {
    ADV_FILTER_POLICY_ALLOW_ALL = 0x00,         /** Accept all connections */
    ADV_FILTER_POLICY_ALLOW_SCAN = 0x01,        /** Accept scan requests, but not connections */
    ADV_FILTER_POLICY_ALLOW_CONN = 0x02,        /** Accept connection requests, but not scans */
    ADV_FILTER_POLICY_ALLOW_NONE = 0x03,        /** Reject all scan and connection requests */
} Adv_FilterPolicy;