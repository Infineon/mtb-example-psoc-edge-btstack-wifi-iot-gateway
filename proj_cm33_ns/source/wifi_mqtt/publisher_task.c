/* Enable this file if application is used as IOT Gateway */
#ifdef IOT_GATEWAY 

/******************************************************************************
* File Name:   publisher_task.c
*
* Description: This file contains the task that sets up the user button GPIO 
*              for the publisher and publishes MQTT messages on the topic
*              'MQTT_PUB_TOPIC' to control a device that is actuated by the
*              subscriber task. The file also contains the ISR that notifies
*              the publisher task about the new device state to be published.
*
* Related Document: See README.md
*
********************************************************************************
* (c) 2025-2026, Infineon Technologies AG, or an affiliate of Infineon
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
#include "publisher_task.h"
#include "app_bt.h"
#include "mqtt_client_config.h"
#include "mqtt_task.h"
#include "subscriber_task.h"
#include "publisher_task.h"
#include "app_bt.h"
#include "subscriber_task.h"
#include "cybsp.h"
#include "FreeRTOS.h"

/* Task header files */
#include "cy_mqtt_api.h"
#include "retarget_io_init.h"

/******************************************************************************
* Macros
******************************************************************************/
/* Interrupt priority for User Button Input. */
#define BTN1_INTERRUPT_PRIORITY         (7U)

/* The maximum number of times each PUBLISH in this example will be retried. */
#define PUBLISH_RETRY_LIMIT             (10U)

/* A PUBLISH message is retried if no response is received within this 
 * time (in milliseconds).
 */
#define PUBLISH_RETRY_MS                (1000U)

/* Queue length of a message queue that is used to communicate with the 
 * publisher task.
 */
#define PUBLISHER_TASK_QUEUE_LENGTH     (3U)
#define DEBOUNCE_TIME_MS                (2U)
#define LED_OFF                         (0U)

/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Global Variables
*******************************************************************************/
extern int led_status;

/* FreeRTOS task handle for this task. */
TaskHandle_t publisher_task_handle;

volatile bool button_debouncing = false;
volatile uint32_t button_debounce_timestamp = 0;

/* Handle of the queue holding the commands for the publisher task */
QueueHandle_t publisher_task_q;

/* Structure to store publish message information. */
cy_mqtt_publish_info_t publish_info =
{
    .qos = (cy_mqtt_qos_t) MQTT_MESSAGES_QOS,
    .topic = MQTT_PUB_TOPIC,
    .topic_len = (sizeof(MQTT_PUB_TOPIC) - 1),
    .retain = false,
    .dup = false
};

/* Interrupt config structure */
cy_stc_sysint_t intrCfg =
{
    .intrSrc = CYBSP_USER_BTN_IRQ,
    .intrPriority = BTN1_INTERRUPT_PRIORITY
};
/*******************************************************************************
* Function Name: button_interrupt_handler
********************************************************************************
* Summary:
*  GPIO interrupt handler.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void button_interrupt_handler(void)
{
    if (Cy_GPIO_GetInterruptStatus(CYBSP_USER_BTN1_PORT, CYBSP_USER_BTN1_PIN))
    {
        Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN1_PORT, CYBSP_USER_BTN1_PIN);
        NVIC_ClearPendingIRQ(CYBSP_USER_BTN1_IRQ);

        if (!button_debouncing)
        {
            /* Set the debouncing flag */
            button_debouncing = true;

            /* Record the current timestamp */
            button_debounce_timestamp = (uint32_t) (xTaskGetTickCount() *
                    portTICK_PERIOD_MS);
        }

        if (button_debouncing && (((xTaskGetTickCount() * portTICK_PERIOD_MS)) -
                button_debounce_timestamp >= DEBOUNCE_TIME_MS * portTICK_PERIOD_MS))
        {
            button_debouncing = false;
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            publisher_data_t publisher_q_data;

            /* Assign the publish command to be sent to the publisher task. */
            publisher_q_data.cmd = PUBLISH_MQTT_MSG;

            /* Assign the publish message payload so that the device state toggles. */
            if (LED_OFF == led_status)
            {
                publisher_q_data.data = (char *)MQTT_DEVICE_OFF_MESSAGE;
            }
            else
            {
                publisher_q_data.data = (char *)MQTT_DEVICE_ON_MESSAGE;
            }

            /* Send the command and data to publisher task over the queue */
            xQueueSendFromISR(publisher_task_q, &publisher_q_data,
                    &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    /* CYBSP_USER_BTN1 (SW2) and CYBSP_USER_BTN2 (SW4) share the same port and
     * hence they share the same NVIC IRQ line. Since both the buttons are
     * configured for falling edge interrupt in the BSP, pressing any button
     * will trigger the execution of this ISR. Therefore, we must clear the
     * interrupt flag of the user button (CYBSP_USER_BTN2) to avoid issues in
     * case if user presses BTN2 by mistake.
     */
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN2_PORT, CYBSP_USER_BTN2_PIN);
    NVIC_ClearPendingIRQ(CYBSP_USER_BTN2_IRQ);
}

/*******************************************************************************
* Function Name: user_button_init
********************************************************************************
*
* Summary:
*  Initialize the button with Interrupt
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void user_button_init(void)
{
    cy_en_sysint_status_t btn_interrupt_init_status ;

    /* CYBSP_USER_BTN1 (SW2) and CYBSP_USER_BTN2 (SW4) share the same port and
    * hence they share the same NVIC IRQ line. Since both are configured in the BSP
    * via the Device Configurator, the interrupt flags for both the buttons are set
    * right after they get initialized through the call to cybsp_init(). The flags
    * must be cleared otherwise the interrupt line will be constantly asserted.
    */
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN1_PORT,CYBSP_USER_BTN1_PIN);
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN2_PORT,CYBSP_USER_BTN2_PIN);
    NVIC_ClearPendingIRQ(CYBSP_USER_BTN1_IRQ);
    NVIC_ClearPendingIRQ(CYBSP_USER_BTN2_IRQ);

    /* Initialize the interrupt and register interrupt callback */
    btn_interrupt_init_status = Cy_SysInt_Init(&intrCfg, &button_interrupt_handler);

    /* Button interrupt initialization failed. Stop program execution. */
    if(CY_SYSINT_SUCCESS != btn_interrupt_init_status)
    {
        handle_app_error();
    }

    /* Enable the interrupt in the NVIC */
    NVIC_EnableIRQ(intrCfg.intrSrc);
}

/******************************************************************************
 * Function Name: publisher_init
 ******************************************************************************
 * Summary:
 *  Function that initializes and sets-up the user button GPIO pin along with  
 *  its interrupt.
 * 
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 ******************************************************************************/
static void publisher_init(void)
{
    /* Initialize the user button GPIO */
    user_button_init();
}

/*******************************************************************************
* Function Name: publisher_deinit
********************************************************************************
* Summary:
*  Disables the user button interrupt
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void publisher_deinit(void)
{
    NVIC_DisableIRQ(intrCfg.intrSrc);
}

/*******************************************************************************
* Function Name: publisher_task
********************************************************************************
* Summary:
*  Task that sets up the user button GPIO for the publisher and publishes
*  MQTT messages to the broker. The user button init and deinit operations,
*  and the MQTT publish operation is performed based on commands sent by other
*  tasks and callbacks over a message queue.
*
* Parameters:
*  void *pvParameters : Task parameter defined during task creation (unused)
*
* Return:
*  void
*
*******************************************************************************/
void publisher_task(void *pvParameters)
{
    /* Status variable */
    cy_rslt_t result;

    publisher_data_t publisher_q_data;

    /* Command to the MQTT client task */
    mqtt_task_cmd_t mqtt_task_cmd;

    /* To avoid compiler warnings */
    CY_UNUSED_PARAMETER(pvParameters);

    /* Initialize and set-up the user button GPIO. */
    publisher_init();

    /* Create a message queue to communicate with other tasks and callbacks. */
    publisher_task_q = xQueueCreate(PUBLISHER_TASK_QUEUE_LENGTH,
            sizeof(publisher_data_t));

    while (true)
    {
        /* Wait for commands from other tasks and callbacks. */
        if (pdTRUE == xQueueReceive(publisher_task_q, &publisher_q_data,
                portMAX_DELAY))
        {
            switch(publisher_q_data.cmd)
            {
                case PUBLISHER_INIT:
                {
                    /* Initialize and set-up the user button GPIO. */
                    publisher_init();
                    break;
                }

                case PUBLISHER_DEINIT:
                {
                    /* Deinit the user button GPIO and corresponding interrupt. */
                    publisher_deinit();
                    break;
                }

                case PUBLISH_MQTT_MSG:
                {
                    /* Publish the data received over the message queue. */
                    publish_info.payload = publisher_q_data.data;
                    publish_info.payload_len = strlen(publish_info.payload);

                    printf("\nPublisher: Publishing '%s' on the topic '%s'\n",
                           (char *) publish_info.payload, publish_info.topic);

                    result = cy_mqtt_publish(mqtt_connection, &publish_info);

                    if (result != CY_RSLT_SUCCESS)
                    {
                        printf("  Publisher: MQTT Publish failed with error "
                                "0x%0X.\n\n", (int)result);

                        /* Communicate the publish failure with the the MQTT
                         * client task.
                         */
                        mqtt_task_cmd = HANDLE_MQTT_PUBLISH_FAILURE;
                        xQueueSend(mqtt_task_q, &mqtt_task_cmd, portMAX_DELAY);
                    }

                    break;
                }

                default:
                    break;
            }
        }
    }
}

#endif /* Enable if defined IOT_GATEWAY */


/* [] END OF FILE */