/* Enable this file if application is used as IOT Gateway */
#ifdef IOT_GATEWAY 

/******************************************************************************
* File Name:   subscriber_task.c
*
* Description: This file contains the task that initializes the user LED GPIO,
*              subscribes to the topic 'MQTT_SUB_TOPIC', and actuates the user LED
*              based on the notifications received from the MQTT subscriber
*              callback.
*
* Related Document: See README.md
*
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
#include "subscriber_task.h"
#include "cybsp.h"
#include "string.h"
#include "app_bt.h"
#include "mqtt_client_config.h"
#include "mqtt_task.h"
#include "FreeRTOS.h"

/* Task header files */
#include "cy_mqtt_api.h"
#include "cy_retarget_io.h"
#include "wiced_bt_dev.h"
#include "retarget_io_init.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* Maximum number of retries for MQTT subscribe operation */
#define MAX_SUBSCRIBE_RETRIES                   (3U)

/* Time interval in milliseconds between MQTT subscribe retries. */
#define MQTT_SUBSCRIBE_RETRY_INTERVAL_MS        (1000U)

/* The number of MQTT topics to be subscribed to. */
#define SUBSCRIPTION_COUNT                      (1U)

/* Queue length of a message queue that is used to communicate with the 
 * subscriber task.
 */
#define SUBSCRIBER_TASK_QUEUE_LENGTH            (1U)
#define TURN_ON_USER_LED_CMD                    (1U)
#define TURN_OFF_USER_LED_CMD                   (0U)
#define STRNG_CMP_SUCCESS                       (0U)

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Task handle for this task. */
TaskHandle_t subscriber_task_handle;

/* Handle of the queue holding the commands for the subscriber task */
QueueHandle_t subscriber_task_q;

/* Variable to denote the current state of the user LED that is also used by 
 * the publisher task.
 */
uint32_t current_device_state = DEVICE_OFF_STATE;

/* Configure the subscription information structure. */
static cy_mqtt_subscribe_info_t subscribe_info =
{
    .qos = (cy_mqtt_qos_t) MQTT_MESSAGES_QOS,
    .topic = MQTT_SUB_TOPIC,
    .topic_len = (sizeof(MQTT_SUB_TOPIC) - 1)
};

wiced_result_t wiced_result;
/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/*******************************************************************************
* Function Name: subscribe_to_topic
********************************************************************************
* Summary:
*  Function that subscribes to the MQTT topic specified by the macro
*  'MQTT_SUB_TOPIC'. This operation is retried a maximum of
*  'MAX_SUBSCRIBE_RETRIES' times with interval of
*  'MQTT_SUBSCRIBE_RETRY_INTERVAL_MS' milliseconds.
*
* Parameters:
*  void
*
* Return:
*  void
*
******************************************************************************/
static void subscribe_to_topic(void)
{
    /* Status variable */
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* Command to the MQTT client task */
    mqtt_task_cmd_t mqtt_task_cmd;

    /* Subscribe with the configured parameters. */
    for (uint32_t retry_count = 0; retry_count < MAX_SUBSCRIBE_RETRIES; retry_count++)
    {
        result = cy_mqtt_subscribe(mqtt_connection, &subscribe_info, SUBSCRIPTION_COUNT);
        if (result == CY_RSLT_SUCCESS)
        {
            printf("\nMQTT client subscribed to the topic '%.*s' successfully.\n",
                    subscribe_info.topic_len, subscribe_info.topic);
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(MQTT_SUBSCRIBE_RETRY_INTERVAL_MS));
    }

    if (CY_RSLT_SUCCESS != result)
    {
        printf("\nMQTT Subscribe failed with error 0x%0X after %d retries...\n\n",
               (int)result, MAX_SUBSCRIBE_RETRIES);

        /* Notify the MQTT client task about the subscription failure */
        mqtt_task_cmd = HANDLE_MQTT_SUBSCRIBE_FAILURE;
        xQueueSend(mqtt_task_q, &mqtt_task_cmd, portMAX_DELAY);
    }
}

/*******************************************************************************
* Function Name: unsubscribe_from_topic
********************************************************************************
* Summary:
*  Function that unsubscribes from the topic specified by the macro
*  'MQTT_SUB_TOPIC'.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void unsubscribe_from_topic(void)
{
    cy_rslt_t result = cy_mqtt_unsubscribe(mqtt_connection,
                             (cy_mqtt_unsubscribe_info_t *) &subscribe_info,
                              SUBSCRIPTION_COUNT);

    if (CY_RSLT_SUCCESS != result)
    {
        printf("MQTT Unsubscribe operation failed with error 0x%0X!\n", (int)result);
    }
}

/*******************************************************************************
* Function Name: subscriber_task
********************************************************************************
* Summary:
*  Task that sets up the user LED GPIO, subscribes to the specified MQTT topic,
*  and controls the user LED based on the received commands over the message
*  queue. The task can also unsubscribe from the topic based on the commands
*  via the message queue.
*
* Parameters:
*  void *pvParameters : Task parameter defined during task creation (unused)
*
* Return:
*  void
*
*******************************************************************************/
void subscriber_task(void *pvParameters)
{
    subscriber_data_t subscriber_q_data;

    /* To avoid compiler warnings */
    (void) pvParameters;

    /* Subscribe to the specified MQTT topic. */
    subscribe_to_topic();

    /* Create a message queue to communicate with other tasks and callbacks. */
    subscriber_task_q = xQueueCreate(SUBSCRIBER_TASK_QUEUE_LENGTH,
            sizeof(subscriber_data_t));

    while (true)
    {
        /* Wait for commands from other tasks and callbacks. */
        if (pdTRUE == xQueueReceive(subscriber_task_q, &subscriber_q_data,
                portMAX_DELAY))
        {
            switch(subscriber_q_data.cmd)
            {
                case SUBSCRIBE_TO_TOPIC:
                {
                    subscribe_to_topic();
                    break;
                }

                case UNSUBSCRIBE_FROM_TOPIC:
                {
                    unsubscribe_from_topic();
                    break;
                }

                case UPDATE_DEVICE_STATE:
                {
                    /* Update the current device state extern variable. */
                    current_device_state = subscriber_q_data.data;
                    break;
                }

                default:
                    break;
            }
        }
    }
}

/*******************************************************************************
* Function Name: mqtt_subscription_callback
********************************************************************************
* Summary:
*  Callback to handle incoming MQTT messages. This callback prints the
*  contents of the incoming message and informs the subscriber task, via a
*  message queue, to turn on / turn off the device based on the received
*  message.
*
* Parameters:
*  cy_mqtt_publish_info_t *received_msg_info : Information structure of the
*                                              received MQTT message
*
* Return:
*  void
*
*******************************************************************************/
void mqtt_subscription_callback(cy_mqtt_publish_info_t *received_msg_info)
{
    /* Received MQTT message */
    const char *received_msg = received_msg_info->payload;
    int received_msg_len = received_msg_info->payload_len;

    /* Data to be sent to the subscriber task queue. */
    subscriber_data_t subscriber_q_data;

    printf("  \nSubsciber: Incoming MQTT message received:\n"
           "    Publish topic name: %.*s\n"
           "    Publish QoS: %d\n"
           "    Publish payload: %.*s\n",
           received_msg_info->topic_len, received_msg_info->topic,
           (int) received_msg_info->qos,
           (int) received_msg_info->payload_len, (const char *)received_msg_info->payload);

    /* Assign the device state depending on the received MQTT message. */
    if ((strlen("TURN ON USER LED") == received_msg_len) &&
        (strncmp("TURN ON USER LED", received_msg, received_msg_len) ==
                STRNG_CMP_SUCCESS))
    {
        send_led_command(TURN_ON_USER_LED_CMD);
    }
    else if ((strlen("TURN OFF USER LED") == received_msg_len) &&
             (strncmp("TURN OFF USER LED", received_msg, received_msg_len) ==
                     STRNG_CMP_SUCCESS))
    {
        send_led_command(TURN_OFF_USER_LED_CMD);
    }

    /* Send the command and data to subscriber task queue */
    xQueueSend(subscriber_task_q, &subscriber_q_data, portMAX_DELAY);
}

#endif /* Enable if defined IOT_GATEWAY */


/* [] END OF FILE */
