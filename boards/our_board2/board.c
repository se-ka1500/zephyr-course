/*
 * Copyright (c) 2024 Custom Vendor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static int board_our_board2_init(void)
{
	printk("Board Initialized\n");
	return 0;
}

SYS_INIT(board_our_board2_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
