/*
 * Copyright (C) 2023 Espressif Systems (Shanghai) CO LTD
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef REGISTER_ACCESS_H
#define REGISTER_ACCESS_H

#define WRITE_REGISTER(addr, val) (*((volatile uint32_t *)(addr))) = (uint32_t)(val)
#define READ_REGISTER(addr) (*((volatile uint32_t *)(addr)))

#endif // REGISTER_ACCESS_H
