/*******************************************************************************
* File Name: ble_node.c
*
* Description: This source file contains the btstack and gatt callback events
*              for ble node usage of the IOT Gateway application in the CM33
*              CPU for ModusToolbox.
*
* Related Document: See README.md
*
********************************************************************************
* (c) 2025, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
* 
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "wiced_bt_stack.h"
#include "cyabs_rtos_impl.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <string.h>
#include "cybt_platform_trace.h"
#include "wiced_memory.h"
#include "stdio.h"
#include "cycfg_gatt_db.h"
#include "cycfg_bt_settings.h"
#include "cycfg_gap.h"
#include "wiced_bt_dev.h"
#include "cybsp_bt_config.h"
#include "retarget_io_init.h"
#include "app_bt_utils.h"
#include "ble_node.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define CONNECTION_ID_INIT_VALUE             (0U)
#define NOTIFY_FLAG_INIT_VALUE               (0U)
#define INIT_VALUE                           (0U)
#define DEVICE_NAME_LEN                      (15U)
#define STRING_CMP_SUCCESS                   (0U)
#define INDEX_VAL                            (0U)
#define USER_LED_ON                          (1U)
#define USER_LED_OFF                         (0U)
#define UUID_INDEX_SIZE                      (16U)
#define UUID_INDEX_SIZE_MINUS_ONE            (15U)
#define SERVICE_DISCOVERY_S_HANDLE           (0x01U)
#define SERVICE_DISCOVERY_E_HANDLE           (0xFFFFU)

/*******************************************************************************
* Variable Definitions
*******************************************************************************/
typedef struct
{
    uint16_t ble_start_handle;
    uint16_t ble_end_handle;
    uint16_t ble_char_handle;
    uint16_t ble_char_val_handle;
    uint16_t ble_cccd_handle;
    bool ble_service_found;
} ble_discovery_data_t;

/* UUID of the custom LED TX service */
uint8_t LED_TXServiceUuid[UUID_INDEX_SIZE]    = {
        0x4F, 0xF5, 0x90, 0xBD, 0x8F, 0xA9, 0x40, 0x10, \
        0xA4, 0xC2, 0x2F, 0x0D, 0xE5, 0x1B, 0xFE, 0x18  \
};

/* UUID of the TX attribute of the custom LED TX service */
uint8_t LED_TXAttrUuid[UUID_INDEX_SIZE]        = {
        0x20, 0xBC, 0x81, 0xB1, 0xE4, 0x3E, 0x4E, 0x73, \
        0x84, 0x43, 0x5F, 0x09, 0x91, 0x23, 0x02, 0x30  \
};

/* UUID of the RX attribute of the custom LED RX service */
uint8_t LED_RXAttrUuid[UUID_INDEX_SIZE]        = {
        0xDE, 0xE8, 0xB3, 0x67, 0x37, 0x48, 0x4C, 0x11, \
        0xA2, 0x98, 0x45, 0x3A, 0x3E, 0xD6, 0x75, 0x97  \
};

static uint16_t bt_connection_id = CONNECTION_ID_INIT_VALUE;
static ble_discovery_data_t ble_discovery_data;
uint8_t notification_flag = NOTIFY_FLAG_INIT_VALUE;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void ble_app_init(void);
static void scan_result_cback(wiced_bt_ble_scan_results_t *p_scan_result,
        uint8_t *p_adv_data );

/* GATT Event Callback Functions */
static wiced_bt_gatt_status_t ble_app_connect_handler
(wiced_bt_gatt_connection_status_t *p_conn_status);
static wiced_bt_gatt_status_t ble_app_gatt_event_callback
        (wiced_bt_gatt_evt_t  event, wiced_bt_gatt_event_data_t *p_event_data);
static wiced_bt_gatt_status_t  ble_app_service_discovery_handler
    (wiced_bt_gatt_discovery_complete_t *discovery_complete);
static wiced_bt_gatt_status_t  ble_app_discovery_result_handler
    (wiced_bt_gatt_discovery_result_t *discovery_result);

/*******************************************************************************
* Function Name: void scan_result_cback
********************************************************************************
* Summary:
*   This function is registered as a callback to handle the scan results.
*   When the desired device is found, it will try to establish connection with
*   that device.
*
* Parameters:
*   wiced_bt_ble_scan_results_t *p_scan_result: Details of the new device found.
*   uint8_t                     *p_adv_data      : Advertisement data.
*
* Return:
*   None
*
*******************************************************************************/
void scan_result_cback(wiced_bt_ble_scan_results_t *p_scan_result,
        uint8_t *p_adv_data )
{
    wiced_result_t result = WICED_BT_SUCCESS;
    uint8_t length = INIT_VALUE;
    uint8_t *adv_name;
    uint8_t client_device_name[DEVICE_NAME_LEN] = {'I','O','T',' ','G','a','t','e',\
            'w','a','y','\0'};

    if (p_scan_result)
    {
        adv_name = wiced_bt_ble_check_advertising_data(p_adv_data,
                BTM_BLE_ADVERT_TYPE_NAME_COMPLETE, &length);

        if(NULL != adv_name)
        {
            /* Check if the peer device's name is "IOT Gateway" */
            if(STRING_CMP_SUCCESS == memcmp(adv_name, client_device_name,
                    strlen((const char *)client_device_name)))
            {
                printf("\nFound the peer device! BD Addr: ");
                print_bd_address(p_scan_result->remote_bd_addr);

                /* Device found. Stop scan. */
                if((result = wiced_bt_ble_scan(BTM_BLE_SCAN_TYPE_NONE, WICED_TRUE,
                        scan_result_cback))!= WICED_BT_SUCCESS)
                {
                    printf("\r\nScan off status %d\r\n", result);
                }
                else
                {
                    printf("\r\nScan completed\r\n");
                }

                printf("\r\nInitiating connection\r\n");

                /* Initiate the connection */
                if(wiced_bt_gatt_le_connect(p_scan_result->remote_bd_addr,
                        p_scan_result->ble_addr_type,
                        BLE_CONN_MODE_HIGH_DUTY,
                        WICED_TRUE)!= WICED_TRUE)
                {
                    printf("\r\nwiced_bt_gatt_connect failed\r\n");
                }
            }
            else
            {
                printf("BD Addr: ");
                print_bd_address(p_scan_result->remote_bd_addr);
                return;
            }
        }
    }
}

/*******************************************************************************
* Function Name: app_bt_management_callback()
********************************************************************************
* Summary:
*   This is a Bluetooth stack event handler function to receive management
*   events from the Bluetooth LE stack and process as per the application.
*
* Parameters:
*   wiced_bt_management_evt_t event : Bluetooth LE event code of one byte length
*   wiced_bt_management_evt_data_t *p_event_data: Pointer to Bluetooth LE
*                                                 management event structures
*
* Return:
*  wiced_result_t: Error code from WICED_RESULT_LIST or BT_RESULT_LIST
*
*******************************************************************************/
wiced_result_t app_bt_management_callback(wiced_bt_management_evt_t event,
        wiced_bt_management_evt_data_t *p_event_data)
{
    wiced_result_t wiced_result = WICED_BT_SUCCESS;

    switch (event)
    {
        case BTM_ENABLED_EVT:

            /* Bluetooth Controller and Host Stack Enabled */
            if (WICED_BT_SUCCESS == p_event_data->enabled.status)
            {
                /* Perform application-specific initialization */
                ble_app_init();
            }
            else
            {
                printf( "Bluetooth Disabled \n" );
            }

            break;

        case BTM_BLE_CONNECTION_PARAM_UPDATE:
            printf("Connection parameter update status:%d, Connection Interval: %d,"
                    " Connection Latency: %d, Connection Timeout: %d\n",
                    p_event_data->ble_connection_param_update.status,
                    p_event_data->ble_connection_param_update.conn_interval,
                    p_event_data->ble_connection_param_update.conn_latency,
                    p_event_data->ble_connection_param_update.supervision_timeout);
            break;
        
        default: 
            break;
            
    }

    return wiced_result;
}

/*******************************************************************************
* Function Name: ble_app_init()
********************************************************************************
* Summary:
*   This function handles application level initialization tasks and is called
*   from the BT management callback once the Bluetooth LE stack enabled event
*   (BTM_ENABLED_EVT) is triggered. This function is executed in the
*   BTM_ENABLED_EVT management callback.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
static void ble_app_init(void)
{
    wiced_bt_gatt_status_t gatt_status;
    wiced_result_t result = WICED_BT_SUCCESS;

    printf("\r\nDiscover device with \"IOT Gateway\" name\r\n");

    /* Register with BT stack to receive GATT callback */
    gatt_status = wiced_bt_gatt_register(ble_app_gatt_event_callback);
    printf("GATT event Handler registration status: %s \n",
            get_bt_gatt_status_name(gatt_status));

    /* Initialize GATT Database */
    gatt_status = wiced_bt_gatt_db_init(gatt_database, gatt_database_len, NULL);
    printf("GATT database initialization status: %s \n",
            get_bt_gatt_status_name(gatt_status));

    result = wiced_bt_ble_scan(BTM_BLE_SCAN_TYPE_HIGH_DUTY, WICED_TRUE,
            scan_result_cback);
    if ((WICED_BT_PENDING == result) || (WICED_BT_BUSY == result))
    {
        printf("\r\nScanning.....\n");
    }
    else
    {
        printf("\rError: Starting scan failed. Error code: %d\n", result);
    }

    UNUSED_VARIABLE(gatt_status);
}


/*******************************************************************************
* Function Name: ble_app_gatt_event_callback()
********************************************************************************
* Summary:
*   GATT Callback function registered with Bluetooth stack to handle GATT events.
*
* Parameters:
*   wiced_bt_gatt_evt_t event : Bluetooth LE GATT event code of one byte length
*   wiced_bt_gatt_event_data_t *p_event_data : Pointer to Bluetooth LE GATT
*                                              event structures
* Return:
*  wiced_bt_gatt_status_t: See possible status codes in wiced_bt_gatt_status_e
*                          in wiced_bt_gatt.h
*******************************************************************************/
static wiced_bt_gatt_status_t
ble_app_gatt_event_callback(wiced_bt_gatt_evt_t event,
        wiced_bt_gatt_event_data_t *p_event_data)
{
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_SUCCESS;

    /* Call the appropriate callback function based on the GATT event type, and
     * pass the relevant event parameters to the callback function */
    switch ( event )
    {
        case GATT_CONNECTION_STATUS_EVT:
            gatt_status = ble_app_connect_handler(&p_event_data->connection_status);
            break;

        case GATT_DISCOVERY_RESULT_EVT:
            gatt_status = ble_app_discovery_result_handler
                (&p_event_data->discovery_result);
            break;

        case GATT_DISCOVERY_CPLT_EVT:
            gatt_status = ble_app_service_discovery_handler
                (&p_event_data->discovery_complete);
            break;

        case GATT_OPERATION_CPLT_EVT:
            switch (p_event_data->operation_complete.op)
            {
                case GATTC_OPTYPE_NOTIFICATION:

                    /* Function call to print notification data */
                    if(USER_LED_OFF == p_event_data->operation_complete.response_data.att_value.
                            p_data[INDEX_VAL])
                    {
                        printf("Turning OFF USER LED 1\n\n");
                        Cy_GPIO_Write(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_NUM,
                                USER_LED_OFF);

                    }
                    else if (USER_LED_ON == p_event_data->operation_complete.response_data.att_value.
                            p_data[INDEX_VAL])
                    {
                        printf("Turning ON USER LED 1\n\n");
                        Cy_GPIO_Write(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_NUM,
                                USER_LED_ON);
                    }

                    break;
            }

            break;

        case GATT_APP_BUFFER_TRANSMITTED_EVT:
        {
            vPortFree(p_event_data->buffer_xmitted.p_app_data);
            break;
        }

        default:
            gatt_status = WICED_BT_GATT_SUCCESS;
            break;
    }

    return gatt_status;
}

/*******************************************************************************
* Function Name:  ble_app_connect_handler()
********************************************************************************
* Summary:
*   This function handles connection status changes.
*
* Parameters:
*   wiced_bt_gatt_connection_status_t *p_conn_status : Pointer to data that has
*                                                      connection details
*
* Return:
*  wiced_bt_gatt_status_t: See possible status codes in wiced_bt_gatt_status_e
*                          in wiced_bt_gatt.h
*
*******************************************************************************/
static wiced_bt_gatt_status_t ble_app_connect_handler
    (wiced_bt_gatt_connection_status_t *p_conn_status)
{
    wiced_result_t result;
    wiced_bt_gatt_status_t gatt_status =  WICED_BT_GATT_SUCCESS;
    wiced_bt_gatt_discovery_param_t service_discovery_setup; 
    memset(&service_discovery_setup, INIT_VALUE, sizeof(wiced_bt_gatt_discovery_param_t));

    if ( NULL != p_conn_status )
    {

        if ( p_conn_status->connected )
        {
            /* Device has connected */
            printf("Device Connected \r\n" );
            printf("Connection ID '%d' \n", p_conn_status->conn_id );

            /* Store the connection ID */
            bt_connection_id = p_conn_status->conn_id;

            /* Send GATT service discovery request */
            service_discovery_setup.s_handle = SERVICE_DISCOVERY_S_HANDLE;
            service_discovery_setup.e_handle = SERVICE_DISCOVERY_E_HANDLE;
            service_discovery_setup.uuid.len = LEN_UUID_128;

            for(int8 i = INIT_VALUE;i < UUID_INDEX_SIZE;i++)
            {
                service_discovery_setup.uuid.uu.uuid128[i] =
                        LED_TXServiceUuid[UUID_INDEX_SIZE_MINUS_ONE-i];
            }

            gatt_status = wiced_bt_gatt_client_send_discover(bt_connection_id,
                    GATT_DISCOVER_SERVICES_BY_UUID, &service_discovery_setup);

            if(WICED_BT_GATT_SUCCESS != gatt_status)
            {
                printf("GATT Discovery request failed. Error code: %d, "
                        "Conn id: %d\n", gatt_status, bt_connection_id);
            }
            else
            {
                printf("Service Discovery Started\n");
            }
        }
        else
        {
            /* Device has disconnected */
            printf("Disconnected : BDA " );
            print_bd_address(p_conn_status->bd_addr);
            printf("Connection ID '%d', Reason '%s'\n", p_conn_status->conn_id,
                    get_bt_gatt_disconn_reason_name(p_conn_status->reason));

            /* Set the connection id to zero to indicate disconnected state */
            bt_connection_id = CONNECTION_ID_INIT_VALUE;

            /* Service discovery is performed upon reconnection, so reset the
             * status of service found flag */
            notification_flag = false;

            /*restart the scan*/
            result = wiced_bt_ble_scan(BTM_BLE_SCAN_TYPE_HIGH_DUTY,
                    WICED_TRUE, scan_result_cback);

            if(WICED_BT_PENDING != result)
            {
                printf("Cannot restart scanning. Error: %d \n", result);
            }
            else
            {
                printf("\r\nScanning.....\n");
            }
        }
    }
    else
    {
        gatt_status = WICED_BT_GATT_ERROR;
    }

    return gatt_status;
}

/*******************************************************************************
* Function Name:  ble_app_discovery_result_handler()
********************************************************************************
* Summary:
*   This function handles discovery results from bluetooth stack.
*
* Parameters:
*   wiced_bt_gatt_discovery_result_t *discovery_result : Pointer to data
*   that has discovery details.
*
* Return:
*  wiced_bt_gatt_status_t: See possible status codes in wiced_bt_gatt_status_e
*                          in wiced_bt_gatt.h
*
*******************************************************************************/
static wiced_bt_gatt_status_t
    ble_app_discovery_result_handler(wiced_bt_gatt_discovery_result_t *discovery_result)
{
    wiced_bt_gatt_status_t gatt_status =  WICED_BT_GATT_SUCCESS;
    wiced_bt_gatt_discovery_type_t discovery_type;
    discovery_type = discovery_result->discovery_type;

    switch (discovery_type)
    {
        case GATT_DISCOVER_SERVICES_BY_UUID:

            ble_discovery_data.ble_start_handle = discovery_result->discovery_data.
                    group_value.s_handle;
            ble_discovery_data.ble_end_handle = discovery_result->discovery_data.
                    group_value.e_handle;
            printf("LED_TX Service Found\n");
            break;

        case GATT_DISCOVER_CHARACTERISTICS:

            ble_discovery_data.ble_char_handle = discovery_result->
                    discovery_data.characteristic_declaration.handle;
            ble_discovery_data.ble_char_val_handle = discovery_result
                    ->discovery_data.characteristic_declaration.val_handle;
            break;

        case GATT_DISCOVER_CHARACTERISTIC_DESCRIPTORS:

            if(UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION ==
                    discovery_result->discovery_data.char_descr_info.type.uu.uuid16)
            {
                ble_discovery_data.ble_cccd_handle = discovery_result
                        ->discovery_data.char_descr_info.handle;
                notification_flag = true;
            }
            break;

        default:
            break;
    }

    return gatt_status;
}

/*******************************************************************************
* Function Name:  ble_app_service_discovery_handler()
********************************************************************************
* Summary:
*   This function handles service discovery for custom LED Service.
*
* Parameters:
*   wiced_bt_gatt_discovery_complete_t *discovery_complete : Pointer to data
*   that has type and status of the completed discovery.
*
* Return:
*  wiced_bt_gatt_status_t: See possible status codes in wiced_bt_gatt_status_e
*                          in wiced_bt_gatt.h
*
*********************************************************************************/
static wiced_bt_gatt_status_t ble_app_service_discovery_handler
    (wiced_bt_gatt_discovery_complete_t *discovery_complete)
{
    wiced_bt_gatt_status_t gatt_status =  WICED_BT_GATT_SUCCESS;
    wiced_bt_gatt_discovery_param_t char_discovery_setup;
    memset(&char_discovery_setup, INIT_VALUE, sizeof(wiced_bt_gatt_discovery_param_t));
    wiced_bt_gatt_discovery_type_t discovery_type;
    discovery_type = discovery_complete->discovery_type;

    switch (discovery_type)
    {
        case GATT_DISCOVER_SERVICES_BY_UUID:
            char_discovery_setup.s_handle = ble_discovery_data.ble_start_handle;
            char_discovery_setup.e_handle = ble_discovery_data.ble_end_handle;

            for(int8 i=INIT_VALUE;i<UUID_INDEX_SIZE;i++)
            {
                char_discovery_setup.uuid.uu.uuid128[i] =
                        LED_RXAttrUuid[UUID_INDEX_SIZE_MINUS_ONE - i];
            }

            gatt_status = wiced_bt_gatt_client_send_discover(bt_connection_id,
                    GATT_DISCOVER_CHARACTERISTICS, &char_discovery_setup);

            if(WICED_BT_GATT_SUCCESS != gatt_status)
                printf("GATT characteristics discovery failed! Error code = %d\n",
                        gatt_status);
            else
                printf("GATT characteristics discovery success \n");

            break;

        case GATT_DISCOVER_CHARACTERISTICS:
            char_discovery_setup.s_handle = ble_discovery_data.ble_start_handle;
            char_discovery_setup.e_handle = ble_discovery_data.ble_end_handle;
            char_discovery_setup.uuid.uu.uuid16 =
                    UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION;

            gatt_status = wiced_bt_gatt_client_send_discover(bt_connection_id,
                    GATT_DISCOVER_CHARACTERISTIC_DESCRIPTORS,
                    &char_discovery_setup);

            if(WICED_BT_GATT_SUCCESS != gatt_status)
                printf("GATT CCCD discovery failed! Error code = %d\n", gatt_status);

            break;

        default:
            break;
    }

    return gatt_status;
}

/* END OF FILE [] */