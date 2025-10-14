/* Enable this file if application is used as IOT Gateway */
#ifdef IOT_GATEWAY 

/*******************************************************************************
* File Name: bt_app.c
*
* Description: This source file contains the btstack and gatt callback events
*              of the IOT Gateway application in the CM33
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
/* Header file includes */
#include "app_bt.h"
#include "wiced_bt_types.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define CONNECTION_ID_INIT_VALUE             (0U)
#define EXT_ATTR_INIT_VAL                    (0U)
#define CONN_ID_INIT_VAL                     (0U)
#define HANDLE_INIT_VAL                      (0U)
#define LENGTH_INIT_VAL                      (0U)
#define NOT_FILLED                           (0U)
#define SCAN_RESPONSE_NO_OF_ELEMENTS         (1U)
#define LED_STATUS_INIT_VAL                  (0U)
#define LED_ON                               (1U)
#define LED_OFF                              (0U)

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
int led_status = LED_STATUS_INIT_VAL;
static uint16_t  bt_connection_id = CONNECTION_ID_INIT_VALUE;
static uint8_t led_on = LED_ON;
static uint8_t led_off = LED_OFF;

typedef struct
{
    wiced_bt_device_address_t remote_addr;  /* remote peer device address */
    uint32_t  timer_count_s;                /* count of sec since the start
                                             * of seconds timer
                                             */
    uint16_t  conn_id;                      /* connection ID of the peer */
    uint16_t  peer_mtu;                     /* peer MTU */
} bt_state_t;

/* Holds the global state of the proximity application */
bt_state_t bt_state;

/* This enumeration combines the advertising, connection states from two different
 * callbacks to maintain the status in a single state variable
 */
typedef enum
{
    APP_BT_ADV_OFF_CONN_OFF,
    APP_BT_ADV_ON_CONN_OFF,
    APP_BT_ADV_OFF_CONN_ON
} app_bt_adv_conn_mode_t;

app_bt_adv_conn_mode_t app_bt_adv_conn_state = APP_BT_ADV_OFF_CONN_OFF;

/*******************************************************************************
* Function Declarations
*******************************************************************************/
typedef void (*pfn_free_buffer_t)(uint8_t *);
wiced_bt_dev_status_t wiced_bt_ble_set_raw_scan_response_data
(uint8_t num_elem, wiced_bt_ble_advert_elem_t *p_data);

/*******************************************************************************
* Function Definitions
*******************************************************************************/
/*******************************************************************************
* Function Name: app_bt_adv_stop_handler
********************************************************************************
* This function handles advertisement stop event.
*
* Parameters:
*   None
*
* Return:
*  None
*
*******************************************************************************/
void app_bt_adv_stop_handler(void)
{
    wiced_result_t result;

    /* Check if device connected or not */
    if (!bt_state.conn_id)
    {
        /* Start Undirected LE Advertisement */
        result =  wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH,
                  BLE_ADDR_PUBLIC, NULL);
        if(result != WICED_BT_SUCCESS)
            printf("Advertisement start failed :%d\r\n", result);
    }
    else
    {
        printf("Stop Advertisement\r\n");
    }

    UNUSED_VARIABLE(result);
}

/*******************************************************************************
* Function Name: send_led_command
********************************************************************************
* This function sends notification to the BLE node to turn ON/OFF the user led.
*
* Parameters:
*  int arg : argument to turn off/on the user led.
*
* Return:
*  None
*
*******************************************************************************/
void send_led_command(int arg)
{
    wiced_bt_gatt_status_t status = 0;

    if(LED_ON == arg)
    {
        led_status = LED_ON;
        printf("\nSend notification to BLE Node to TURN ON USER LED\n");

        /* GATT Server Send Notification */
        status = wiced_bt_gatt_server_send_notification(bt_connection_id,
                HDLC_LED_TX_LED_BLINK_VALUE, MAX_LEN_LED_TX_LED_BLINK, &led_on,
                NULL);
    }
    else if(LED_OFF == arg)
    {

        led_status = LED_OFF;
        printf("\nSend notification to BLE Node to TURN OFF USER LED\n");

        /* GATT Server Send Notification */
        status = wiced_bt_gatt_server_send_notification(bt_connection_id,
                HDLC_LED_TX_LED_BLINK_VALUE, MAX_LEN_LED_TX_LED_BLINK, &led_off,
                NULL);
    }

    if(status != WICED_BT_GATT_SUCCESS)
    {
        printf("\n\r Notification send failed \n\r");
    }
}


/*******************************************************************************
* Function Name: le_app_write_handler
********************************************************************************
* Summary:
*   This function handles Write Requests received from the client device
*
* Parameters:
*  conn_id          Connection ID
*  opcode           LE GATT request type opcode
*  p_write_req      Pointer to LE GATT write request
*  len_req          Length of data requested
*  *p_error_handle  Pointer to error handle
*
* Return:
*  wiced_bt_gatt_status_t: See possible status codes in wiced_bt_gatt_status_e
*  in wiced_bt_gatt.h
*
********************************************************************************/
static wiced_bt_gatt_status_t le_app_write_handler(uint16_t conn_id,
        wiced_bt_gatt_opcode_t opcode, wiced_bt_gatt_write_req_t *p_write_req,
        uint16_t len_req, uint16_t *p_error_handle)
{
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_INVALID_HANDLE;
    *p_error_handle = p_write_req->handle;

    return (gatt_status);
}

/*******************************************************************************
* Function Name : le_app_find_by_handle
* ******************************************************************************
* Summary :
*  Find attribute description by handle
*
* Parameter:
*  handle - handle to look up
*
* Return:
*  gatt_db_lookup_table_t - pointer containing handle data
*
*******************************************************************************/
static gatt_db_lookup_table_t  *le_app_find_by_handle(uint16_t handle)
{
    int iteration;
    for (iteration = EXT_ATTR_INIT_VAL; iteration < app_gatt_db_ext_attr_tbl_size;
            iteration++)
    {
        if (handle == app_gatt_db_ext_attr_tbl[iteration].handle )
        {
            break;
        }
    }

    return (&app_gatt_db_ext_attr_tbl[iteration]);
}

/*******************************************************************************
* Function Name: le_app_read_handler
********************************************************************************
* Summary:
*   This function handles Read Requests received from the client device
*
* Parameters:
*  conn_id          Connection ID
*  opcode           LE GATT request type opcode
*  p_read_req       Pointer to read request containing the handle to read
*  len_req          Length of data requested
*  *p_error_handle  Pointer to error handle
*
* Return:
*  wiced_bt_gatt_status_t: See possible status codes in wiced_bt_gatt_status_e
*  in wiced_bt_gatt.h
*
*******************************************************************************/
static wiced_bt_gatt_status_t le_app_read_handler( uint16_t conn_id,
             wiced_bt_gatt_opcode_t opcode, wiced_bt_gatt_read_t *p_read_req,
             uint16_t len_req, uint16_t *p_error_handle)
{
    gatt_db_lookup_table_t *puAttribute;
    int  attr_len_to_copy;
    uint8_t *from;
    int to_send;
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;
    *p_error_handle = p_read_req->handle;
    puAttribute = le_app_find_by_handle(p_read_req->handle);

    if ( NULL == puAttribute )
    {
        status = WICED_BT_GATT_INVALID_HANDLE;
    }
    else
    {
        attr_len_to_copy = puAttribute->cur_len;

        if (p_read_req->offset >= puAttribute->cur_len)
        {
            status = WICED_BT_GATT_INVALID_OFFSET;
        }
        else
        {
            to_send = MIN(len_req, attr_len_to_copy - p_read_req->offset);
            from = ((uint8_t *)puAttribute->p_data) + p_read_req->offset;
            status = wiced_bt_gatt_server_send_read_handle_rsp(conn_id, opcode,
                    to_send, from, NULL);
        }
    }

    return status;
}

/*******************************************************************************
* Function Name: le_app_connect_handler
********************************************************************************
* Summary:
*  This callback function handles connection status changes.
*
* Parameters:
*  wiced_bt_gatt_connection_status_t *p_conn_status  : Pointer to data that has
*                                                       connection details
*
* Return:
*  wiced_bt_gatt_status_t: See possible status codes in wiced_bt_gatt_status_e
*                          in wiced_bt_gatt.h
*
*******************************************************************************/
static wiced_bt_gatt_status_t le_app_connect_handler
             (wiced_bt_gatt_connection_status_t *p_conn_status)
{
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_SUCCESS ;
    if ( NULL != p_conn_status )
    {
        if ( p_conn_status->connected )
        {
            /* Device has connected */
            printf("\nConnected to BLE Node device: BDA " );
            print_bd_address(p_conn_status->bd_addr);
            printf("Connection ID '%d' \n", p_conn_status->conn_id );

            /* Store the connection ID */
            bt_connection_id = p_conn_status->conn_id;

            /* Update the adv/conn state */
            app_bt_adv_conn_state = APP_BT_ADV_OFF_CONN_ON;

            wiced_bt_start_advertisements(BTM_BLE_ADVERT_OFF,
                    BLE_ADDR_PUBLIC, NULL);
        }
        else
        {
            /* Device has disconnected */
            printf("Disconnected : BDA " );
            print_bd_address(p_conn_status->bd_addr);
            printf("Connection ID '%d', Reason '%s'\n", p_conn_status->conn_id,
                    get_bt_gatt_disconn_reason_name(p_conn_status->reason) );

            /* Set the connection id to zero to indicate disconnected state */
            bt_connection_id = CONN_ID_INIT_VAL;

            /* Restart the advertisements */
            wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH,
                    BLE_ADDR_PUBLIC, NULL);

            /* Update the adv/conn state */
            app_bt_adv_conn_state = APP_BT_ADV_ON_CONN_OFF;

        }
    }

    return gatt_status;
}

/*******************************************************************************
* Function Name: app_free_buffer
********************************************************************************
* Summary:
*  This function frees up the memory buffer.
*
* Parameters:
*  uint8_t *p_data: Pointer to the buffer to be free
*
* Return:
*  None
*******************************************************************************/
static void app_free_buffer(uint8_t *p_buf)
{
    vPortFree(p_buf);
}

/*******************************************************************************
* Function Name: app_alloc_buffer
********************************************************************************
* Summary:
*  This function allocates a memory buffer.
*
* Parameters:
*  int len: Length to allocate
*
* Return:
*  None
*******************************************************************************/
static void* app_alloc_buffer(int len)
{
    return pvPortMalloc(len);
}

/*******************************************************************************
* Function Name: app_bt_gatt_req_read_by_type_handler
********************************************************************************
* Function Description:
*  Process read-by-type request from peer device
*
* Parameters:
*  conn_id          Connection ID
*  opcode           LE GATT request type opcode
*  p_read_req       Pointer to read request containing the handle to read
*  len_requested    Length of data requested
*  *p_error_handle  Pointer to error handle
*
* Return:
*  wiced_bt_gatt_status_t  LE GATT status
*
*******************************************************************************/
static wiced_bt_gatt_status_t
app_bt_gatt_req_read_by_type_handler(uint16_t conn_id,
                                     wiced_bt_gatt_opcode_t opcode,
                                     wiced_bt_gatt_read_by_type_t *p_read_req,
                                     uint16_t len_requested,
                                     uint16_t *p_error_handle)
{
    int filled = NOT_FILLED;
    gatt_db_lookup_table_t *puAttribute;
    uint16_t last_handle = HANDLE_INIT_VAL;
    uint16_t attr_handle = p_read_req->s_handle;
    uint8_t *p_rsp = app_alloc_buffer(len_requested);
    uint8_t pair_len = LENGTH_INIT_VAL;
    int used_len = LENGTH_INIT_VAL;
    wiced_bt_gatt_status_t status = WICED_BT_GATT_SUCCESS;

    if (NULL == p_rsp)
    {
        printf("No memory, len_requested: %d!!\r\n",len_requested);
        status = WICED_BT_GATT_INSUF_RESOURCE;
    }
    else
    {
        /* Read by type returns all attributes of the specified type,
         * between the start and end handles
         */
        while (WICED_TRUE)
        {
            *p_error_handle = attr_handle;
            last_handle = attr_handle;
            attr_handle = wiced_bt_gatt_find_handle_by_type(attr_handle,
                                                            p_read_req->e_handle,
                                                           &p_read_req->uuid);
            if (HANDLE_INIT_VAL == attr_handle )
                break;

            if ( NULL == (puAttribute = le_app_find_by_handle(attr_handle)))
            {
                printf("found type but no attribute for %d \r\n",last_handle);
                status = WICED_BT_GATT_INVALID_HANDLE;
                break;
            }

            filled = wiced_bt_gatt_put_read_by_type_rsp_in_stream(p_rsp +
                         used_len, len_requested - used_len, &pair_len,
                         attr_handle, puAttribute->cur_len, puAttribute->p_data);

            if (NOT_FILLED == filled)
            {
                break;
            }

            used_len += filled;

            /* Increment starting handle for next search to one past current */
            attr_handle++;
        }

        if (LENGTH_INIT_VAL == used_len)
        {
           printf("attr not found  start_handle: 0x%04x  end_handle: 0x%04x  "
                   "Type: 0x%04x\r\n", p_read_req->s_handle, p_read_req->e_handle,
                   p_read_req->uuid.uu.uuid16);
            status = WICED_BT_GATT_INVALID_HANDLE;
        }
        else
        {
            /* Send the response */
            status = wiced_bt_gatt_server_send_read_by_type_rsp(conn_id,
                    opcode, pair_len, used_len, p_rsp, (void *)app_free_buffer);
        }
    }

    if (p_rsp != NULL)
    {
        app_free_buffer(p_rsp);
    }

    return status;
}

/*******************************************************************************
* Function Name: le_app_server_handler
********************************************************************************
* Summary:
*   This function handles GATT server events from the BT stack.
*
* Parameters:
*  p_attr_req:      Pointer to LE GATT connection status
*  *p_error_handle  Pointer to error handle
*
* Return:
*  wiced_bt_gatt_status_t: See possible status codes in wiced_bt_gatt_status_e
*                          in wiced_bt_gatt.h
*
********************************************************************************/
static wiced_bt_gatt_status_t le_app_server_handler
                  (wiced_bt_gatt_attribute_request_t *p_attr_req,
                   uint16_t *p_error_handle)
{
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_SUCCESS;

    switch ( p_attr_req->opcode )
    {
        case GATT_REQ_READ_BLOB:

             /* Attribute read request */
            gatt_status = le_app_read_handler(p_attr_req->conn_id,
                                              p_attr_req->opcode,
                                              &p_attr_req->data.read_req,
                                              p_attr_req->len_requested,
                                              p_error_handle);
             break;

        case GATT_CMD_WRITE:

             /* Attribute write request */
            gatt_status = le_app_write_handler(p_attr_req->conn_id,
                                               p_attr_req->opcode,
                                               &p_attr_req->data.write_req,
                                               p_attr_req->len_requested,
                                               p_error_handle );

             break;

        case GATT_REQ_MTU:
            gatt_status = wiced_bt_gatt_server_send_mtu_rsp(p_attr_req->conn_id,
                                                       p_attr_req->data.remote_mtu,
                                                       CY_BT_RX_PDU_SIZE);
             break;

        case GATT_HANDLE_VALUE_NOTIF:
                    printf("Notfication send complete\n");
             break;

        case GATT_REQ_READ_BY_TYPE:
            gatt_status = app_bt_gatt_req_read_by_type_handler(p_attr_req->conn_id,
                            p_attr_req->opcode, &p_attr_req->data.read_by_type,
                            p_attr_req->len_requested, p_error_handle);
             break;

        default:
                printf("ERROR: Unhandled GATT Connection Request case: %d\n",
                        p_attr_req->opcode);
                gatt_status = WICED_BT_GATT_ERROR;
                break;
    }

    return gatt_status;
}

/******************************************************************************
* Function Name: le_app_gatt_event_callback
*******************************************************************************
* Summary:
*   This function handles GATT events from the BT stack.
*
* Parameters:
*   wiced_bt_gatt_evt_t event                : LE GATT event code of one byte
*                                              length
*   wiced_bt_gatt_event_data_t *p_event_data : Pointer to LE GATT event structures
*
* Return:
*  wiced_bt_gatt_status_t: See possible status codes in wiced_bt_gatt_status_e
*                          in wiced_bt_gatt.h
*
*******************************************************************************/
static wiced_bt_gatt_status_t le_app_gatt_event_callback
            (wiced_bt_gatt_evt_t event, wiced_bt_gatt_event_data_t *p_event_data)
{
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_SUCCESS;
    wiced_bt_gatt_attribute_request_t *p_attr_req =
            &p_event_data->attribute_request;
    uint16_t error_handle = HANDLE_INIT_VAL;

    /* Call the appropriate callback function based on the GATT event type,
     * and pass the relevant event
     * parameters to the callback function
     */
    switch ( event )
    {
        case GATT_CONNECTION_STATUS_EVT:
            gatt_status = le_app_connect_handler(&p_event_data->connection_status);
            break;

        case GATT_ATTRIBUTE_REQUEST_EVT:
            gatt_status = le_app_server_handler(p_attr_req,
                                                &error_handle );
            if(WICED_BT_GATT_SUCCESS != gatt_status)
            {
               wiced_bt_gatt_server_send_error_rsp(p_attr_req->conn_id,
                    p_attr_req->opcode, error_handle, gatt_status);
            }

            break;

        case GATT_GET_RESPONSE_BUFFER_EVT:
            /* GATT buffer request, typically sized to max of bearer mtu - 1 */
            p_event_data->buffer_request.buffer.p_app_rsp_buffer =
            app_alloc_buffer(p_event_data->buffer_request.len_requested);
            p_event_data->buffer_request.buffer.p_app_ctxt =
                    (void *)app_free_buffer;
            gatt_status = WICED_BT_GATT_SUCCESS;
            break;

        case GATT_APP_BUFFER_TRANSMITTED_EVT:
            /* GATT buffer transmitted event,
             * check \ref wiced_bt_gatt_buffer_transmitted_t
             */
            {
            pfn_free_buffer_t pfn_free = (pfn_free_buffer_t)p_event_data
                    ->buffer_xmitted.p_app_ctxt;

            /* If the buffer is dynamic, the context will point to a
             * function to free it.
             */
            if (pfn_free)
            {
                pfn_free(p_event_data->buffer_xmitted.p_app_data);
            }

            gatt_status = WICED_BT_GATT_SUCCESS;
            }

            break;

        default:
            gatt_status = WICED_BT_GATT_ERROR;
               break;
    }

    return gatt_status;
}

/*******************************************************************************
* Function Name: le_app_init
********************************************************************************
* Summary:
*  This function handles application level initialization tasks and is called
*  from the BT management callback once the LE stack enabled event
*  (BTM_ENABLED_EVT) is triggered.
*  This function is executed in the BTM_ENABLED_EVT management callback.
*******************************************************************************/
static void le_app_init(void)
{
    wiced_result_t wiced_result;
    cy_rslt_t cy_result = CY_RSLT_SUCCESS;
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_SUCCESS;

    /* Set raw response packet data */
    cy_result = wiced_bt_ble_set_raw_scan_response_data
            (SCAN_RESPONSE_NO_OF_ELEMENTS,cy_bt_scan_resp_packet_data);

    if (WICED_SUCCESS != cy_result)
    {
        printf("Set response data failed\n");
    }

    /* Register with BT stack to receive GATT callback */
    gatt_status = wiced_bt_gatt_register(le_app_gatt_event_callback);
    printf("GATT event Handler registration status: %s \n",
            get_bt_gatt_status_name(gatt_status));

    /* Initialize GATT Database */
    gatt_status = wiced_bt_gatt_db_init(gatt_database,
            gatt_database_len, NULL);
    printf("GATT database initialization status: %s \n",
            get_bt_gatt_status_name(gatt_status));

    /* Start Undirected LE Advertisements on device startup. */
    wiced_result = wiced_bt_start_advertisements
            (BTM_BLE_ADVERT_UNDIRECTED_HIGH, BLE_ADDR_PUBLIC, NULL);

    printf("\nStarted undirected advertisements with the name 'IOT Gateway' \n\n");

    /* Failed to start advertisement. Stop program execution */
    if (WICED_BT_SUCCESS != wiced_result)
    {
        printf("failed to start advertisement! \n");
        handle_app_error();
    }

}

/*******************************************************************************
* Function Name: app_bt_management_callback
********************************************************************************
* Summary:
*   This is a Bluetooth stack event handler function to receive management events 
*   from the LE stack and process as per the application.
*
* Parameters:
*   wiced_bt_management_evt_t event             : LE event code of one byte length
*   wiced_bt_management_evt_data_t *p_event_data: Pointer to LE management event 
*                                                 structures
*
* Return:
*  wiced_result_t: Error code from WICED_RESULT_LIST or BT_RESULT_LIST
*
*******************************************************************************/
wiced_result_t app_bt_management_callback(wiced_bt_management_evt_t event,
        wiced_bt_management_evt_data_t *p_event_data)
{
    wiced_result_t wiced_result = WICED_BT_SUCCESS;
    wiced_bt_ble_advert_mode_t *p_mode;

    switch (event)
    {
        case BTM_ENABLED_EVT:

            /* Bluetooth Controller and Host Stack Enabled */
            if (WICED_BT_SUCCESS == p_event_data->enabled.status)
            {
                /* Perform application-specific initialization */
                le_app_init();
            }
            else
            {
                printf("Bluetooth enable failed \r\n");
            }

            break;

        case BTM_DISABLED_EVT:
            break;

        case BTM_BLE_ADVERT_STATE_CHANGED_EVT:
            p_mode = &p_event_data->ble_advert_state_changed;

            if (*p_mode == BTM_BLE_ADVERT_OFF)
            {
                app_bt_adv_stop_handler();
            }

            break;

        case BTM_BLE_CONNECTION_PARAM_UPDATE:
            printf("Connection parameter update status:%d, Connection Interval: "
                    "%d, Connection Latency: %d, Connection Timeout: %d\n",
                    p_event_data->ble_connection_param_update.status,
                    p_event_data->ble_connection_param_update.conn_interval,
                    p_event_data->ble_connection_param_update.conn_latency,
                    p_event_data->ble_connection_param_update.supervision_timeout);
            break;

        case BTM_BLE_PHY_UPDATE_EVT:
            printf("Selected TX PHY - %dM\r\n Selected RX PHY - %dM\r\n",
                    p_event_data->ble_phy_update_event.tx_phy,
                    p_event_data->ble_phy_update_event.rx_phy);
            break;

        default:
            break;
    }

    return wiced_result;
}

#endif /* Enable if defined IOT_GATEWAY */


/* [] END OF FILE */