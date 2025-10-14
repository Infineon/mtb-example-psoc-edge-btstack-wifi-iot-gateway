/* Enable this file if application is used as IOT Gateway */
#ifdef IOT_GATEWAY 

/******************************************************************************
* File Name: wifi_config.h
*
* Description: This file contains the configuration macros required for the
*              Wi-Fi connection.
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

#ifndef WIFI_CONFIG_H_
#define WIFI_CONFIG_H_

#include "cy_wcm.h"

/*******************************************************************************
* Macros
********************************************************************************/
/* SSID of the Wi-Fi Access Point to which the MQTT client connects. */
#define WIFI_SSID                        "MY_WIFI_SSID"

/* Passkey of the above mentioned Wi-Fi SSID. */
#define WIFI_PASSWORD                    "MY_WIFI_PASSWORD"

/* Security type of the Wi-Fi access point. See 'cy_wcm_security_t' structure
 * in "cy_wcm.h" for more details.
 */
#define WIFI_SECURITY                     CY_WCM_SECURITY_WPA2_AES_PSK

/* Maximum Wi-Fi re-connection limit. */
#define MAX_WIFI_CONN_RETRIES             (120u)

/* Wi-Fi re-connection time interval in milliseconds. */
#define WIFI_CONN_RETRY_INTERVAL_MS       (5000)

#endif /* WIFI_CONFIG_H_ */

#endif /* Enable if defined IOT_GATEWAY */