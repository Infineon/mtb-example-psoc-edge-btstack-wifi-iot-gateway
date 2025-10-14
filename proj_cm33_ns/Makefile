################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Makefile for configuring the Cortex-M33 core project
#
################################################################################
# \copyright
# (c) 2025, Infineon Technologies AG, or an affiliate of Infineon
# Technologies AG.  SPDX-License-Identifier: Apache-2.0
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

include ../common.mk

################################################################################
# Basic Configuration
################################################################################

# Name of application (used to derive name of final linked file).
#
# If APPNAME is edited, ensure to update or regenerate launch
# configurations for your IDE.
APPNAME=proj_cm33_ns

# If set to "true" or "1", display full command-lines when building.
VERBOSE=

# Set BT feature baudrate to 3M 
DEFINES+=CYBSP_BT_PLATFORM_CFG_BAUD_FEATURE=3000000 

# Use application as BLE NODE
USE_BLE_NODE = 0

################################################################################
# Advanced Configuration
################################################################################

# Enable optional code that is ordinarily disabled by default.
#
# Available components depend on the specific targeted hardware and firmware
# in use. In general, if you have
#
#    COMPONENTS=foo bar
#
# ... then code in directories named COMPONENT_foo and COMPONENT_bar will be
# added to the build
#
COMPONENTS+=

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS= 

CORE=CM33
CORE_NAME=CM33_0

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
SOURCES+=

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES+=

# Select softfp or hardfp floating point. Default is softfp.
VFP_SELECT+=

# Additional / custom C compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CFLAGS+=

# Additional / custom C++ compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CXXFLAGS+=

# Additional / custom assembler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
ASFLAGS+=

# Additional / custom linker flags.
LDFLAGS+=

# Additional / custom libraries to link in to the application.
LDLIBS+=

# Path to the linker script to use (if empty, use the default linker script).
LINKER_SCRIPT=

# Custom pre-build commands to run.
PREBUILD=

# Custom post-build commands to run.
POSTBUILD=

################################################################################
# Advanced Custom Configuration
################################################################################

ifeq ($(USE_BLE_NODE),1)
# Search for components folder
SEARCH+=../components

# Include the below components
COMPONENTS+=FREERTOS WICED_BLE BLE_NODE

# Add additional defines to the build process (without a leading -D).
DEFINES+=CY_RETARGET_IO_CONVERT_LF_TO_CRLF

# Ignore WiFi and MQTT libraries if app is used in BLE_NODE 
# configuration
CY_IGNORE+=$(SEARCH_wifi-core-freertos-lwip-mbedtls)/
CY_IGNORE+=$(SEARCH_wifi-host-driver)/
CY_IGNORE+=$(SEARCH_wifi-resources)/
CY_IGNORE+=$(SEARCH_wpa3-external-supplicant)/
CY_IGNORE+=$(SEARCH_wifi-connection-manager)/
CY_IGNORE+=$(SEARCH_whd-bsp-integration)/
CY_IGNORE+=$(SEARCH_secure-sockets)/
CY_IGNORE+=$(SEARCH_lwip)/
CY_IGNORE+=$(SEARCH_lwip-network-interface-integration)/
CY_IGNORE+=$(SEARCH_lwip-freertos-integration)/
CY_IGNORE+=$(SEARCH_ifx-mbedtls)/
CY_IGNORE+=$(SEARCH_cy-mbedtls-acceleration)/
CY_IGNORE+=$(SEARCH_connectivity-utilities)/
CY_IGNORE+=$(SEARCH_aws-iot-device-sdk-port)/
CY_IGNORE+=$(SEARCH_aws-iot-device-sdk-embedded-C)/
CY_IGNORE+=$(SEARCH_mqtt)/

else
# Include the below components
COMPONENTS+=FREERTOS LWIP MBEDTLS WCM SECURE_SOCKETS WICED_BLE

# Add below define
DEFINES+=IOT_GATEWAY

# Custom configuration of mbedtls library.
MBEDTLSFLAGS = MBEDTLS_USER_CONFIG_FILE='"configs/mbedtls_user_config.h"'

# Add additional defines to the build process (without a leading -D).
DEFINES+=$(MBEDTLSFLAGS) CYBSP_WIFI_CAPABLE CY_RETARGET_IO_CONVERT_LF_TO_CRLF

# Default configuration of mbedtls library.
DEFINES+=MBEDTLS_CONFIG_FILE='"mbedtls/mbedtls_config.h"'
# PSA configuration of mbedtls library.
DEFINES+=MBEDTLS_PSA_CRYPTO_CONFIG_FILE='"configs/ifx_psa_crypto_config.h"'

ifeq ($(TOOLCHAIN),LLVM_ARM)
DEFINES+=CY_MQTT_EVENT_THREAD_STACK_SIZE=10*1024
endif

# Define the following macro in the application's Makefile to mandatorily disable the custom 
# configuration header file.
DEFINES += HTTP_DO_NOT_USE_CUSTOM_CONFIG
endif

################################################################################
# Paths
################################################################################

# Relative path to the project directory (default is the Makefile's directory).
#
# This controls where automatic source code discovery looks for code.
CY_APP_PATH=

# Relative path to the shared repo location.
#
# All .mtb files have the format, <URI>#<COMMIT>#<LOCATION>. If the <LOCATION> 
# field begins with $$ASSET_REPO$$, then the repo is deposited in the path 
# specified by the CY_GETLIBS_SHARED_PATH variable. The default location is one 
# directory level above the current app directory.
# This is used with CY_GETLIBS_SHARED_NAME variable, which specifies the 
# directory name.
CY_GETLIBS_SHARED_PATH=../..

# Directory name of the shared repo location.
#
CY_GETLIBS_SHARED_NAME=mtb_shared

include $(CY_TOOLS_DIR)/make/start.mk
