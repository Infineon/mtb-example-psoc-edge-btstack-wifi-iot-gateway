# PSOC&trade; Edge MCU: Bluetooth&reg; LE Wi-Fi IoT gateway 

This code example demonstrates implementing a Bluetooth&reg; Wi-Fi IoT gateway using the MQTT client library and Bluetooth&reg; host stack on PSOC&trade; Edge MCU coupled with AIROC&trade; CYW55513 Wi-Fi & Bluetooth&reg; combo chip in the ModusToolbox&trade; software environment. It uses the AWS IoT Device SDK library (MQTT and Thing Shadows) to publish and subscribe with the AWS cloud. The PSOC&trade; Edge E84 kit receives cloud commands, controls USER LED 1 on a Bluetooth&reg; LE node via a GATT connection, and sends the LED state back to the cloud based on a user button press.

This code example implements both IoT gateway and Bluetooth&reg; LE node implementations in a single application which can be switched using a simple Makefile variable.

This code example has a three project structure: CM33 secure, CM33 non-secure, and CM55 projects. All three projects are programmed to the external QSPI flash and executed in Execute in Place (XIP) mode. Extended boot launches the CM33 secure project from a fixed location in the external flash, which then configures the protection settings and launches the CM33 non-secure application. Additionally, CM33 non-secure application enables CM55 CPU and launches the CM55 application.

[View this README on GitHub.](https://github.com/Infineon/mtb-example-psoc-edge-btstack-wifi-iot-gateway)

[Provide feedback on this code example.](https://cypress.co1.qualtrics.com/jfe/form/SV_1NTns53sK2yiljn?Q_EED=eyJVbmlxdWUgRG9jIElkIjoiQ0UyNDIxNTgiLCJTcGVjIE51bWJlciI6IjAwMi00MjE1OCIsIkRvYyBUaXRsZSI6IlBTT0MmdHJhZGU7IEVkZ2UgTUNVOiBCbHVldG9vdGgmcmVnOyBMRSBXaS1GaSBJb1QgZ2F0ZXdheSIsInJpZCI6InN1cmVzaGt1bWFyYSIsIkRvYyB2ZXJzaW9uIjoiMS4wLjAiLCJEb2MgTGFuZ3VhZ2UiOiJFbmdsaXNoIiwiRG9jIERpdmlzaW9uIjoiTUNEIiwiRG9jIEJVIjoiSUNXIiwiRG9jIEZhbWlseSI6IlBTT0MifQ==)

See the [Design and implementation](docs/design_and_implementation.md) for the functional description of this code example.


## Requirements

- [ModusToolbox&trade;](https://www.infineon.com/modustoolbox) v3.6 or later (tested with v3.6)
- Board support package (BSP) minimum required version: 1.0.0
- Programming language: C
- Associated parts: All [PSOC&trade; Edge MCU](https://www.infineon.com/products/microcontroller/32-bit-psoc-arm-cortex/32-bit-psoc-edge-arm) parts


## Supported toolchains (make variable 'TOOLCHAIN')

- GNU Arm&reg; Embedded Compiler v14.2.1 (`GCC_ARM`) – Default value of `TOOLCHAIN`
- Arm&reg; Compiler v6.22 (`ARM`)
- IAR C/C++ Compiler v9.50.2 (`IAR`)
- LLVM Embedded Toolchain for Arm&reg; v19.1.5 (`LLVM_ARM`)


## Supported kits (make variable 'TARGET')

- [PSOC&trade; Edge E84 Evaluation Kit](https://www.infineon.com/KIT_PSE84_EVAL) (`KIT_PSE84_EVAL_EPC2`) – Default value of `TARGET`
- [PSOC&trade; Edge E84 Evaluation Kit](https://www.infineon.com/KIT_PSE84_EVAL) (`KIT_PSE84_EVAL_EPC4`)


## Hardware setup

This example uses the board's default configuration. See the kit user guide to ensure that the board is configured correctly.

Ensure the following jumper and pin configuration on board.
- BOOT SW must be in the HIGH/ON position
- J20 and J21 must be in the tristate/not connected (NC) position

To verify the output, two PSOC&trade; Edge E84 Evaluation kits are required: one for the IOT Gateway and the other for Bluetooth&reg; LE node.
Program the Bluetooth&reg; LE node on the second kit.


## Software setup

See the [ModusToolbox&trade; tools package installation guide](https://www.infineon.com/ModusToolboxInstallguide) for information about installing and configuring the tools package.

Install a terminal emulator if you do not have one. Instructions in this document use [Tera Term](https://teratermproject.github.io/index-en.html).

This example requires no additional software or tools.


## Operation

See [Using the code example](docs/using_the_code_example.md) for instructions on creating a project, opening it in various supported IDEs, and performing tasks, such as building, programming, and debugging the application within the respective IDEs.

> **Note:** The code example requires you to program another PSOC&trade; Edge E84 Evaluation Kit to act as a Bluetooth&reg; LE node to communicate with. Build and program the same application after updating the `USE_BLE_NODE` variable to '1' in *Makefile* of the *proj_cm33_ns* folder.

1. Connect the board to your PC using the provided USB cable through the KitProg3 USB connector

2. Update the user configuration files based on the following:

   1. **Wi-Fi configuration:** Set the Wi-Fi credentials in *wifi_config.h* file and modify the `WIFI_SSID`, `WIFI_PASSWORD`, and `WIFI_SECURITY` macros to match the Wi-Fi network you want to connect

   2. **MQTT configuration:** Set up the MQTT client and configure the credentials in the *mqtt_client_config.h* file. Some of the important configuration macros are:

      - `MQTT_BROKER_ADDRESS`: Hostname of the MQTT broker

      - `MQTT_PORT`: Port number to use for the MQTT connection. As specified by the Internet Assigned Numbers Authority (IANA), port numbers assigned to the MQTT protocol are '1883' for non-secure connections and '8883' for secure connections. However, MQTT brokers may use other ports. Configure this macro as specified by the MQTT broker

      - `MQTT_SECURE_CONNECTION`: Set this macro as '1' if you need to establish a secure (transport layer security (TLS)) connection to the MQTT broker; otherwise set it as '0'

      - `MQTT_USERNAME` and `MQTT_PASSWORD`: User name and password for client authentication and authorization if required by the MQTT broker. Note that this information is generally not encrypted and the password is sent in plain text. Therefore, this is not a recommended method of client authentication

      - `CLIENT_CERTIFICATE` and `CLIENT_PRIVATE_KEY`: Certificate and private key of the MQTT client used for client authentication. Note that these macros are applicable only when `MQTT_SECURE_CONNECTION` is set as '1'

      - `ROOT_CA_CERTIFICATE`: Root CA certificate of the MQTT broker

      See [Setting up the MQTT broker](#setting-up-the-mqtt-broker) section to learn how to configure these macros for AWS IoT MQTT broker

      For a full list of configuration macros used in this code example, see the [Wi-Fi and MQTT configuration macros](#wi-fi-and-mqtt-configuration-macros) section

   3. **Other configuration files:** You can optionally modify the configuration macros in the following file according to your application:

      - *core_mqtt_config.h*, used by the [MQTT library](https://github.com/Infineon/mqtt)

3. Set up the Device Shadow service and to subscribe and publish to 'ledstatus' topic. See the [AWS Device Shadow](https://docs.aws.amazon.com/iot/latest/developerguide/device-shadow-document.html) for details on using the Device Shadow service. Additionally, see the [AN238090 - Connecting to cloud services using ModusToolbox&trade;](https://www.infineon.com/AN238090) application note on how to setup MQTT client and Device Shadow services

4. Open a terminal program and select the KitProg3 COM port. Set the serial port parameters to 8N1 and 115200 baud

5. After programming, the application starts automatically. Observe the messages stating application initialization, Bluetooth&reg; stack initialization, Bluetooth&reg; LE advertisement start, Wi-Fi initializations, MQTT initialization and successful connection, and subscription to 'ledstatus' topic as shown in **Figure 1**

   **Figure 1. Terminal output on program startup**

   ![](images/iot_gateway_app_start.png)

    Now the Bluetooth&reg; advertisement would have started with the name "IOT Gateway" and MQTT connection is successful, and it is subscribed to topic named 'ledstatus'

6. Navigate to *Makefile* of the *proj_cm33_ns* folder and update the `USE_BLE_NODE` variable to '1'. Build and program the application to another PSOC&trade; Edge E84 Evaluation Kit, which will act as a Bluetooth&reg; LE node

7. After successful programming, the Bluetooth&reg; LE node scans for the device with name "IOT Gateway" and gets connected as shown in **Figure 2**

   **Figure 2. Terminal output on Bluetooth&reg; LE node program startup**

   ![](images/ble_node_app_start.png)

8. Navigate to the AWS website and select the Things and Shadows, which is set up as shown in **Figure 3**

   **Figure 3. Sample Things and Shadows selection**

   ![](images/things_and_device_shadows.png)

9. Subscribe to 'ledstatus' topic as shown in **Figure 4** 

   **Figure 4. Subscribe to ledstatus topic**

   ![](images/subscribe_to_topic.png)

10. Navigate to **Publish to a topic** and then select the subscribed topic and **Publish** the message with `TURN ON USER LED` payload as shown in **Figure 5**

    **Figure 5. Publish to ledstatus topic**

    ![](images/publish_to_topic.png)

    Observe message from subscriber to publish the payload and PSOC&trade; Edge device sends notification to the Bluetooth&reg; LE Node to TURN ON USER LED as shown in **Figure 6**

    **Figure 6. Send notification to Bluetooth&reg; LE node**

    ![](images/send_notification.png)

    Observe the USER LED 1 on the Bluetooth&reg; LE Node turning ON

    Similarly, publish with `TURN OFF USER LED` to turn the USER LED 1 on the Bluetooth&reg; LE node off

11. Press the **USER Button 1 (SW2)** on the primary kit (IoT gateway) to publish the status of the USER LED 1 of the Bluetooth&reg; LE node to the AWS cloud as shown in **Figure 7** and **Figure 8**

    **Figure 7. LED status publish logs**

    ![](images/led_status_publish_logs.png)

    **Figure 8. LED status publish on cloud**

    ![](images/led_status_on_cloud.png)


### Configuring the MQTT client


#### Wi-Fi and MQTT configuration macros

**Table 1. Wi-Fi and MQTT configuration macros**

Macro                               |  Description
:---------------------------------- | :------------------------
**Wi-Fi connection configurations**  |  In *proj_cm33_ns* > *source* > *wifi_mqtt* > *wifi_config.h*
`WIFI_SSID`       | SSID of the Wi-Fi AP to which the MQTT client connects
`WIFI_PASSWORD`   | Passkey/password for the Wi-Fi SSID specified above
`WIFI_SECURITY`   | Security type of the Wi-Fi AP. See `cy_wcm_security_t` structure in *cy_wcm.h* file for details
`MAX_WIFI_CONN_RETRIES`   | Maximum number of retries for Wi-Fi connection
`WIFI_CONN_RETRY_INTERVAL_MS`   | Time interval in milliseconds between successive Wi-Fi connection retries
**MQTT connection configurations**  |  In *proj_cm33_ns* > *source* > *wifi_mqtt* > *mqtt_client_config.h*
`MQTT_BROKER_ADDRESS`      | Hostname of the MQTT broker
`MQTT_PORT`                | Port number to use for the MQTT connection. As specified by IANA, port numbers assigned for MQTT protocol is '1883' for non-secure connections and '8883' for secure connections. However, MQTT brokers may use other ports. Configure this macro as specified by the MQTT broker
`MQTT_SECURE_CONNECTION`   | Set this macro to '1' if you need to establish a secure (TLS) connection to the MQTT broker; else `0`.
`MQTT_USERNAME` <br> `MQTT_PASSWORD`   | User name and password for client authentication and authorization, if required by the MQTT broker. However, note that this information is generally not encrypted and the password is sent in plain text. Therefore, this is not a recommended method of client authentication
**MQTT client certificate configurations**  |  In *proj_cm33_ns* > *source* > *wifi_mqtt* > *mqtt_client_config.h*
`CLIENT_CERTIFICATE` <br> `CLIENT_PRIVATE_KEY`  | Certificate and private key of the MQTT client used for client authentication. Note that these macros are applicable only when `MQTT_SECURE_CONNECTION` is set to '1'
`ROOT_CA_CERTIFICATE`      |  Root CA certificate of the MQTT broker
**MQTT message configurations**    |  In *mqtt_client_config.h*
`MQTT_PUB_TOPIC`           | MQTT topic to which the messages are published by the Publisher task to the MQTT broker
`MQTT_SUB_TOPIC`           | MQTT topic to which the subscriber task subscribes. The MQTT broker sends the messages to the subscriber that are published in this (or equivalent) topic
`MQTT_MESSAGES_QOS`        | The quality of service (QoS) level to be used by the publisher and subscriber. Valid choices are '0', '1', and '2'
`ENABLE_LWT_MESSAGE`       | Set this macro to '1' if you want to use the Last Will and Testament (LWT) option; else '0'. <br> LWT is an MQTT message published by the MQTT broker on the specified topic if the MQTT connection is unexpectedly closed. This configuration is sent to the MQTT broker during the MQTT connect operation; the MQTT broker publishes the Will message on the Will topic when it recognizes an unexpected disconnection from the client
`MQTT_WILL_TOPIC_NAME` <br> `MQTT_WILL_MESSAGE`   | The MQTT topic and message for the LWT option described above. These configurations are applicable only when `ENABLE_LWT_MESSAGE` is set to '1'
`MQTT_DEVICE_ON_MESSAGE` <br> `MQTT_DEVICE_OFF_MESSAGE`  | The MQTT messages that control the device (LED) state in this code example
**Other MQTT client configurations**    |  In *proj_cm33_ns* > *source* > *wifi_mqtt* > *mqtt_client_config.h*
`GENERATE_UNIQUE_CLIENT_ID`   | Every active MQTT connection must have a unique client identifier. If this macro is set to '1', the device generates a unique client identifier by appending a timestamp to the string specified by the `MQTT_CLIENT_IDENTIFIER` macro. This feature is useful if you are using the same code on multiple kits simultaneously
`MQTT_CLIENT_IDENTIFIER`     | The client identifier (client ID) string to use during MQTT connection. If `GENERATE_UNIQUE_CLIENT_ID` is set to '1', a timestamp is appended to this macro value and used as the client ID; else, the value specified for this macro is directly used as the client ID
`MQTT_CLIENT_IDENTIFIER_MAX_LEN`   | The longest client identifier that an MQTT server must accept (as defined by the MQTT 3.1.1 spec) is 23 characters. However, some MQTT brokers support longer client IDs. Configure this macro as per the MQTT broker specification
`MQTT_TIMEOUT_MS`            | Timeout in milliseconds for MQTT operations in this example
`MQTT_KEEP_ALIVE_SECONDS`    | The keepalive interval in seconds used for MQTT ping request
`MQTT_ALPN_PROTOCOL_NAME`   | The application layer protocol negotiation (ALPN) protocol name to use that is supported by the MQTT broker in use. Note that this is an optional macro for most use cases. <br> As per IANA, the port numbers assigned to MQTT protocol are '1883' for non-secure connections and '8883' for secure connections. In some cases, you need to use other ports for MQTT, like port 443 (reserved for HTTPS). ALPN is an extension to TLS that allows many protocols to be used over a secure connection
`MQTT_SNI_HOSTNAME`   | The server name indication (SNI) hostname to use during the TLS connection as specified by the MQTT broker. <br> SNI is an extension to the TLS protocol. As required by some MQTT brokers, SNI typically includes the hostname in the "Client Hello" message sent during the TLS handshake
`MQTT_NETWORK_BUFFER_SIZE`   | A network buffer is allocated for sending and receiving MQTT packets over the network. Specify the size of this buffer using this macro. Note that the minimum buffer size is defined by the `CY_MQTT_MIN_NETWORK_BUFFER_SIZE` macro in the MQTT library
`MAX_MQTT_CONN_RETRIES`   | Maximum number of retries for MQTT connection
`MQTT_CONN_RETRY_INTERVAL_MS`   | Time interval (in milliseconds) between successive MQTT connection retries

<br>


#### Setting up the MQTT broker

<details><summary><b>AWS IoT MQTT</b></summary>

 1. Set up the MQTT device (also known as a **Thing**) in the AWS IoT core as described in the [Getting started with AWS IoT tutorial](https://docs.aws.amazon.com/iot/latest/developerguide/iot-gs.html)

      > **Note:** While setting up your device, ensure that the policy associated with this device permits all MQTT operations (**iot:Connect**, **iot:Publish**, **iot:Receive**, and **iot:Subscribe**) for the resource used by this device. For testing purposes, it is recommended to have the following policy document, which allows all **MQTT Policy Actions** on all **Amazon Resource Names (ARNs)**
   
      ```
      {
         "Version": "2012-10-17",
         "Statement": [
               {
                  "Effect": "Allow",
                  "Action": "iot:*",
                  "Resource": "*"
               }
         ]
      }
      ```

 2. In the *mqtt_client_config.h* file, set `MQTT_BROKER_ADDRESS` to your custom endpoint on the **Settings** page of the AWS IoT console. This has the 'ABCDEFG1234567.iot.<region>.amazonaws.com' format

 3. Set the macros `MQTT_PORT` to '8883' and `MQTT_SECURE_CONNECTION` to '1' in the *mqtt_client_config.h* file

 4. Download the following certificates and keys that are created and activated in the previous step:

       - A certificate for the AWS IoT Thing: *xxxxxxxxxx.cert.pem*
       - A public key: *xxxxxxxxxx.public.key*
       - A private key: *xxxxxxxxxx.private.key*
       - Root CA "RSA 2048 bit key: Amazon Root CA 1" for AWS IoT from [CA certificates for server authentication](https://docs.aws.amazon.com/iot/latest/developerguide/server-authentication.html#server-authentication-certs)

 5. Using these certificates and keys, enter the following parameters in the *mqtt_client_config.h* file in the privacy-enhanced mail (PEM) format:
       - `CLIENT_CERTIFICATE`: *xxxxxxxxxx.cert.pem*
       - `CLIENT_PRIVATE_KEY`: *xxxxxxxxxx.private.key*
       - `ROOT_CA_CERTIFICATE`: Root CA certificate

    Either convert the values to strings manually following the format shown in the *mqtt_client_config.h* file or use the [amazon-freertos](https://github.com/Infineon/amazon-freertos/blob/master/tools/certificate_configuration/PEMfileToCString.html) HTML utility to convert the certificates and keys from the PEM to C string format. Clone the repository from GitHub to use the utility

</details>


### Wi-Fi throughput

This code example is configured to run on the CM33 core at a frequency of 200 MHz, out of the external flash memory. However, this setup may result in lower throughput compared to running the code in internal memory (SRAM).

For optimal performance, it is recommended to run the code example on the CM55 core at 400 MHz, leveraging the internal memory (i.e., System SRAM/SoCMEM). For guidance on achieving better throughput, see the [Wi-Fi Bluetooth&reg; tester](https://github.com/Infineon/mtb-psoc-edge-wifi-bluetooth-tester) application.


## Related resources

Resources  | Links
-----------|----------------------------------
Application notes  | [AN235935](https://www.infineon.com/AN235935) – Getting started with PSOC&trade; Edge E8 MCU on ModusToolbox&trade; software <br> [AN236697](https://www.infineon.com/AN236697) – Getting started with PSOC&trade; MCU and AIROC&trade; Connectivity devices <br> [AN238090](https://www.infineon.com/AN238090) – Connecting to cloud services using ModusToolbox&trade;
Code examples  | [Using ModusToolbox&trade;](https://github.com/Infineon/Code-Examples-for-ModusToolbox-Software) on GitHub
Device documentation | [PSOC&trade; Edge MCU datasheets](https://www.infineon.com/products/microcontroller/32-bit-psoc-arm-cortex/32-bit-psoc-edge-arm#documents) <br> [PSOC&trade; Edge MCU reference manuals](https://www.infineon.com/products/microcontroller/32-bit-psoc-arm-cortex/32-bit-psoc-edge-arm#documents)
Development kits | Select your kits from the [Evaluation board finder](https://www.infineon.com/cms/en/design-support/finder-selection-tools/product-finder/evaluation-board)
Libraries  | [mtb-dsl-pse8xxgp](https://github.com/Infineon/mtb-dsl-pse8xxgp) – Device support library for PSE8XXGP <br> [retarget-io](https://github.com/Infineon/retarget-io) – Utility library to retarget STDIO messages to a UART port <br> [wifi-core-freertos-lwip-mbedtls](https://github.com/Infineon/wifi-core-freertos-lwip-mbedtls) -This repo includes core components needed for Wi-Fi connectivity support. The library bundles FreeRTOS, lwIP TCP/IP stack, Mbed TLS for security, Wi-Fi host driver (WHD), Wi-Fi Connection Manager (WCM), secure sockets, connectivity utilities, and configuration files <br> [btstack-integration](https://github.com/Infineon/btstack-integration) – The btstack-integration hosts platform adaptation layer (porting layer) between AIROC&trade; BTSTACK and Infineon's different hardware platforms
Tools  | [ModusToolbox&trade;](https://www.infineon.com/modustoolbox) – ModusToolbox&trade; software is a collection of easy-to-use libraries and tools enabling rapid development with Infineon MCUs for applications ranging from wireless and cloud-connected systems, edge AI/ML, embedded sense and control, to wired USB connectivity using PSOC&trade; Industrial/IoT MCUs, AIROC&trade; Wi-Fi and Bluetooth&reg; connectivity devices, XMC&trade; Industrial MCUs, and EZ-USB&trade;/EZ-PD&trade; wired connectivity controllers. ModusToolbox&trade; incorporates a comprehensive set of BSPs, HAL, libraries, configuration tools, and provides support for industry-standard IDEs to fast-track your embedded application development

<br>


## Other resources

Infineon provides a wealth of data at [www.infineon.com](https://www.infineon.com) to help you select the right device, and quickly and effectively integrate it into your design.


## Document history

Document title: *CE242158* – *PSOC&trade; Edge MCU: Bluetooth&reg; LE Wi-Fi IoT gateway*

 Version | Description of change
 ------- | ---------------------
 1.0.0   | New code example <br> GitHub release 
<br>


All referenced product or service names and trademarks are the property of their respective owners.

The Bluetooth&reg; word mark and logos are registered trademarks owned by Bluetooth SIG, Inc., and any use of such marks by Infineon is under license.

PSOC&trade;, formerly known as PSoC&trade;, is a trademark of Infineon Technologies. Any references to PSoC&trade; in this document or others shall be deemed to refer to PSOC&trade;.

---------------------------------------------------------

© Cypress Semiconductor Corporation, 2025. This document is the property of Cypress Semiconductor Corporation, an Infineon Technologies company, and its affiliates ("Cypress").  This document, including any software or firmware included or referenced in this document ("Software"), is owned by Cypress under the intellectual property laws and treaties of the United States and other countries worldwide.  Cypress reserves all rights under such laws and treaties and does not, except as specifically stated in this paragraph, grant any license under its patents, copyrights, trademarks, or other intellectual property rights.  If the Software is not accompanied by a license agreement and you do not otherwise have a written agreement with Cypress governing the use of the Software, then Cypress hereby grants you a personal, non-exclusive, nontransferable license (without the right to sublicense) (1) under its copyright rights in the Software (a) for Software provided in source code form, to modify and reproduce the Software solely for use with Cypress hardware products, only internally within your organization, and (b) to distribute the Software in binary code form externally to end users (either directly or indirectly through resellers and distributors), solely for use on Cypress hardware product units, and (2) under those claims of Cypress's patents that are infringed by the Software (as provided by Cypress, unmodified) to make, use, distribute, and import the Software solely for use with Cypress hardware products.  Any other use, reproduction, modification, translation, or compilation of the Software is prohibited.
<br>
TO THE EXTENT PERMITTED BY APPLICABLE LAW, CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH REGARD TO THIS DOCUMENT OR ANY SOFTWARE OR ACCOMPANYING HARDWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  No computing device can be absolutely secure.  Therefore, despite security measures implemented in Cypress hardware or software products, Cypress shall have no liability arising out of any security breach, such as unauthorized access to or use of a Cypress product. CYPRESS DOES NOT REPRESENT, WARRANT, OR GUARANTEE THAT CYPRESS PRODUCTS, OR SYSTEMS CREATED USING CYPRESS PRODUCTS, WILL BE FREE FROM CORRUPTION, ATTACK, VIRUSES, INTERFERENCE, HACKING, DATA LOSS OR THEFT, OR OTHER SECURITY INTRUSION (collectively, "Security Breach").  Cypress disclaims any liability relating to any Security Breach, and you shall and hereby do release Cypress from any claim, damage, or other liability arising from any Security Breach.  In addition, the products described in these materials may contain design defects or errors known as errata which may cause the product to deviate from published specifications. To the extent permitted by applicable law, Cypress reserves the right to make changes to this document without further notice. Cypress does not assume any liability arising out of the application or use of any product or circuit described in this document. Any information provided in this document, including any sample design information or programming code, is provided only for reference purposes.  It is the responsibility of the user of this document to properly design, program, and test the functionality and safety of any application made of this information and any resulting product.  "High-Risk Device" means any device or system whose failure could cause personal injury, death, or property damage.  Examples of High-Risk Devices are weapons, nuclear installations, surgical implants, and other medical devices.  "Critical Component" means any component of a High-Risk Device whose failure to perform can be reasonably expected to cause, directly or indirectly, the failure of the High-Risk Device, or to affect its safety or effectiveness.  Cypress is not liable, in whole or in part, and you shall and hereby do release Cypress from any claim, damage, or other liability arising from any use of a Cypress product as a Critical Component in a High-Risk Device. You shall indemnify and hold Cypress, including its affiliates, and its directors, officers, employees, agents, distributors, and assigns harmless from and against all claims, costs, damages, and expenses, arising out of any claim, including claims for product liability, personal injury or death, or property damage arising from any use of a Cypress product as a Critical Component in a High-Risk Device. Cypress products are not intended or authorized for use as a Critical Component in any High-Risk Device except to the limited extent that (i) Cypress's published data sheet for the product explicitly states Cypress has qualified the product for use in a specific High-Risk Device, or (ii) Cypress has given you advance written authorization to use the product as a Critical Component in the specific High-Risk Device and you have signed a separate indemnification agreement.
<br>
Cypress, the Cypress logo, and combinations thereof, ModusToolbox, PSoC, CAPSENSE, EZ-USB, F-RAM, and TRAVEO are trademarks or registered trademarks of Cypress or a subsidiary of Cypress in the United States or in other countries. For a more complete list of Cypress trademarks, visit www.infineon.com. Other names and brands may be claimed as property of their respective owners.