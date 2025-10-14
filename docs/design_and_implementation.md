[Click here](../README.md) to view the README.

## Design and implementation

The design of this application is minimalistic to get started with code examples on PSOC&trade; Edge MCU devices. All PSOC&trade; Edge E84 MCU applications have a dual-CPU three-project structure to develop code for the CM33 and CM55 cores. The CM33 core has two separate projects for the secure processing environment (SPE) and non-secure processing environment (NSPE). A project folder consists of various subfolders, each denoting a specific aspect of the project. The three project folders are as follows:

**Table 1. Application projects**

Project | Description
--------|------------------------
*proj_cm33_s* | Project for CM33 secure processing environment (SPE)
*proj_cm33_ns* | Project for CM33 non-secure processing environment (NSPE)
*proj_cm55* | CM55 project

<br>

In this code example, at device reset, the secure boot process starts from the ROM boot with the secure enclave (SE) as the root of trust (RoT). From the secure enclave, the boot flow is passed on to the system CPU subsystem where the secure CM33 application starts. After all necessary secure configurations, the flow is passed on to the non-secure CM33 application. Resource initialization for this example is performed by this CM33 non-secure project. It configures the system clocks, pins, clock to peripheral connections, and other platform resources. It then enables the CM55 core using the `Cy_SysEnableCM55()` function and the CM55 core is subsequently put to DeepSleep mode.

In the CM33 non-secure application, the clocks and system resources are initialized by the BSP initialization function. The retarget-io middleware is configured to use the debug UART. BTSTACK is initialized and callback functions are registered. Events handlers are written to handle the BTSTACK callback events. A task is created to handle the MQTT client implementation and connection. 

This code example implements both IoT gateway and Bluetooth&reg; LE node implementations in a single application, which can be switched using a simple Makefile variable.

This example implements a cloud-connected PSOC&trade; Edge Kit that bridges MQTT-based cloud commands to a Bluetooth&reg; LE peripheral device for LED control, and publishes LED status back to the cloud.

On startup, the PSOC&trade; Edge Kit establishes a secure MQTT session with the target broker, subscribes to command topics, and publishes device telemetry and status to well-defined topics. In parallel, the same PSOC&trade; Edge Kit operates as a Bluetooth&reg; LE Central that discovers and connects to a peripheral Bluetooth&reg; LE device (another PSOC&trade; Edge Kit), which exposes a custom GATT service. This service provides characteristics for LED control and LED status. During connection setup, the Central performs service and characteristic discovery.

Cloud-originated commands received via MQTT are translated by the Central into GATT operations on the Peripheral. Specifically, when a command instructs the LED to turn ON or OFF, the Central issues a notification to the LED control characteristic on the Peripheral’s custom GATT service and the USER LED 1 on the peripheral device (Bluetooth&reg; LE node) is controlled based on the notification from the central device. 

<br>