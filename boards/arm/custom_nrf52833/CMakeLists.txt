# Application-level CMakeLists.txt

# SPDX-License-Identifier: Apache-2.0

message("Custom nRF52833 board")

zephyr_include_directories(.)

dts_include_directories(${ZEPHYR_BASE}/dts/arm)