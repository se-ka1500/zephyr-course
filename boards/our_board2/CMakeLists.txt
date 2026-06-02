# Copyright (c) 2024 Custom Vendor
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_BOARD_OUR_BOARD2_ESP32S3_PROCPU)
  zephyr_library()
  zephyr_library_sources(board.c)
endif()
