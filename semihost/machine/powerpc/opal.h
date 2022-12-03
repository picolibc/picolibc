/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2022 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <stdio.h>

extern void *__opal_base, *__opal_entry;

int
opal_call(void *r3, void *r4, void *r5, int call_r6, void *base_r7, void *entry_r8);

int
opal_console_write(int terminal, size_t len, const char *base);

void _ATTRIBUTE((__noreturn__))
opal_cec_power_down(uint64_t request);

#define OPAL_TERMINAL_DEFAULT   0

/* status codes */
#define OPAL_SUCCESS		0
#define OPAL_PARAMETER		-1
#define OPAL_BUSY		-2
#define OPAL_PARTIAL		-3
#define OPAL_CONSTRAINED	-4
#define OPAL_CLOSED		-5
#define OPAL_HARDWARE		-6
#define OPAL_UNSUPPORTED	-7
#define OPAL_PERMISSION		-8
#define OPAL_NO_MEM		-9
#define OPAL_RESOURCE		-10
#define OPAL_INTERNAL_ERROR	-11
#define OPAL_BUSY_EVENT		-12
#define OPAL_HARDWARE_FROZEN	-13
#define OPAL_WRONG_STATE	-14
#define OPAL_ASYNC_COMPLETION	-15
#define OPAL_EMPTY		-16
#define OPAL_I2C_TIMEOUT	-17
#define OPAL_I2C_INVALID_CMD	-18
#define OPAL_I2C_LBUS_PARITY	-19
#define OPAL_I2C_BKEND_OVERRUN	-20
#define OPAL_I2C_BKEND_ACCESS	-21
#define OPAL_I2C_ARBT_LOST	-22
#define OPAL_I2C_NACK_RCVD	-23
#define OPAL_I2C_STOP_ERR	-24
#define OPAL_XSCOM_BUSY		OPAL_BUSY
#define OPAL_XSCOM_CHIPLET_OFF	OPAL_WRONG_STATE
#define OPAL_XSCOM_PARTIAL_GOOD	-25
#define OPAL_XSCOM_ADDR_ERROR	-26
#define OPAL_XSCOM_CLOCK_ERROR	-27
#define OPAL_XSCOM_PARITY_ERROR	-28
#define OPAL_XSCOM_TIMEOUT	-29
#define OPAL_XSCOM_CTR_OFFLINED	-30
#define OPAL_XIVE_PROVISIONING	-31
#define OPAL_XIVE_FREE_ACTIVE	-32
#define OPAL_TIMEOUT		-33

/* API Tokens (in r0) */
#define OPAL_INVALID_CALL		       -1
#define OPAL_TEST				0
#define OPAL_CONSOLE_WRITE			1
#define OPAL_CONSOLE_READ			2
#define OPAL_RTC_READ				3
#define OPAL_RTC_WRITE				4
#define OPAL_CEC_POWER_DOWN			5
#define OPAL_CEC_REBOOT				6
#define OPAL_READ_NVRAM				7
#define OPAL_WRITE_NVRAM			8
#define OPAL_HANDLE_INTERRUPT			9
#define OPAL_POLL_EVENTS			10
#define OPAL_PCI_SET_HUB_TCE_MEMORY		11 /* Removed, p5ioc only */
#define OPAL_PCI_SET_PHB_TCE_MEMORY		12 /* Removed, p5ioc only */
#define OPAL_PCI_CONFIG_READ_BYTE		13
#define OPAL_PCI_CONFIG_READ_HALF_WORD  	14
#define OPAL_PCI_CONFIG_READ_WORD		15
#define OPAL_PCI_CONFIG_WRITE_BYTE		16
#define OPAL_PCI_CONFIG_WRITE_HALF_WORD		17
#define OPAL_PCI_CONFIG_WRITE_WORD		18
#define OPAL_SET_XIVE				19
#define OPAL_GET_XIVE				20
#define OPAL_GET_COMPLETION_TOKEN_STATUS	21 /* obsolete */
#define OPAL_REGISTER_OPAL_EXCEPTION_HANDLER	22
#define OPAL_PCI_EEH_FREEZE_STATUS		23
#define OPAL_PCI_SHPC				24
#define OPAL_CONSOLE_WRITE_BUFFER_SPACE		25
#define OPAL_PCI_EEH_FREEZE_CLEAR		26
#define OPAL_PCI_PHB_MMIO_ENABLE		27
#define OPAL_PCI_SET_PHB_MEM_WINDOW		28
#define OPAL_PCI_MAP_PE_MMIO_WINDOW		29
#define OPAL_PCI_SET_PHB_TABLE_MEMORY		30 /* never implemented */
#define OPAL_PCI_SET_PE				31
#define OPAL_PCI_SET_PELTV			32
#define OPAL_PCI_SET_MVE			33
#define OPAL_PCI_SET_MVE_ENABLE			34
#define OPAL_PCI_GET_XIVE_REISSUE		35 /* never implemented */
#define OPAL_PCI_SET_XIVE_REISSUE		36 /* never implemented */
#define OPAL_PCI_SET_XIVE_PE			37
#define OPAL_GET_XIVE_SOURCE			38
#define OPAL_GET_MSI_32				39
#define OPAL_GET_MSI_64				40
#define OPAL_START_CPU				41
#define OPAL_QUERY_CPU_STATUS			42
#define OPAL_WRITE_OPPANEL			43 /* unimplemented */
#define OPAL_PCI_MAP_PE_DMA_WINDOW		44
#define OPAL_PCI_MAP_PE_DMA_WINDOW_REAL		45
/* 46 is unused */
/* 47 is unused */
/* 48 is unused */
#define OPAL_PCI_RESET				49
#define OPAL_PCI_GET_HUB_DIAG_DATA		50
#define OPAL_PCI_GET_PHB_DIAG_DATA		51
#define OPAL_PCI_FENCE_PHB			52
#define OPAL_PCI_REINIT				53
#define OPAL_PCI_MASK_PE_ERROR			54
#define OPAL_SET_SLOT_LED_STATUS		55
#define OPAL_GET_EPOW_STATUS			56
#define OPAL_SET_SYSTEM_ATTENTION_LED		57
#define OPAL_RESERVED1				58
#define OPAL_RESERVED2				59
#define OPAL_PCI_NEXT_ERROR			60
#define OPAL_PCI_EEH_FREEZE_STATUS2		61
#define OPAL_PCI_POLL				62
#define OPAL_PCI_MSI_EOI			63
#define OPAL_PCI_GET_PHB_DIAG_DATA2		64
#define OPAL_XSCOM_READ				65
#define OPAL_XSCOM_WRITE			66
#define OPAL_LPC_READ				67
#define OPAL_LPC_WRITE				68
#define OPAL_RETURN_CPU				69
#define OPAL_REINIT_CPUS			70
#define OPAL_ELOG_READ				71
#define OPAL_ELOG_WRITE				72
#define OPAL_ELOG_ACK				73
#define OPAL_ELOG_RESEND			74
#define OPAL_ELOG_SIZE				75
#define OPAL_FLASH_VALIDATE			76
#define OPAL_FLASH_MANAGE			77
#define OPAL_FLASH_UPDATE			78
#define OPAL_RESYNC_TIMEBASE			79
#define OPAL_CHECK_TOKEN			80
#define OPAL_DUMP_INIT				81
#define OPAL_DUMP_INFO				82
#define OPAL_DUMP_READ				83
#define OPAL_DUMP_ACK				84
#define OPAL_GET_MSG				85
#define OPAL_CHECK_ASYNC_COMPLETION		86
#define OPAL_SYNC_HOST_REBOOT			87
#define OPAL_SENSOR_READ			88
#define OPAL_GET_PARAM				89
#define OPAL_SET_PARAM				90
#define OPAL_DUMP_RESEND			91
#define OPAL_ELOG_SEND				92	/* Deprecated */
#define OPAL_PCI_SET_PHB_CAPI_MODE		93
#define OPAL_DUMP_INFO2				94
#define OPAL_WRITE_OPPANEL_ASYNC		95
#define OPAL_PCI_ERR_INJECT			96
#define OPAL_PCI_EEH_FREEZE_SET			97
#define OPAL_HANDLE_HMI				98
#define OPAL_CONFIG_CPU_IDLE_STATE		99
#define OPAL_SLW_SET_REG			100
#define OPAL_REGISTER_DUMP_REGION		101
#define OPAL_UNREGISTER_DUMP_REGION		102
#define OPAL_WRITE_TPO				103
#define OPAL_READ_TPO				104
#define OPAL_GET_DPO_STATUS			105
#define OPAL_OLD_I2C_REQUEST			106	/* Deprecated */
#define OPAL_IPMI_SEND				107
#define OPAL_IPMI_RECV				108
#define OPAL_I2C_REQUEST			109
#define OPAL_FLASH_READ				110
#define OPAL_FLASH_WRITE			111
#define OPAL_FLASH_ERASE			112
#define OPAL_PRD_MSG				113
#define OPAL_LEDS_GET_INDICATOR			114
#define OPAL_LEDS_SET_INDICATOR			115
#define OPAL_CEC_REBOOT2			116
#define OPAL_CONSOLE_FLUSH			117
#define OPAL_GET_DEVICE_TREE			118
#define OPAL_PCI_GET_PRESENCE_STATE		119
#define OPAL_PCI_GET_POWER_STATE		120
#define OPAL_PCI_SET_POWER_STATE		121
#define OPAL_INT_GET_XIRR			122
#define	OPAL_INT_SET_CPPR			123
#define OPAL_INT_EOI				124
#define OPAL_INT_SET_MFRR			125
#define OPAL_PCI_TCE_KILL			126
#define OPAL_NMMU_SET_PTCR			127
#define OPAL_XIVE_RESET				128
#define OPAL_XIVE_GET_IRQ_INFO			129
#define OPAL_XIVE_GET_IRQ_CONFIG		130
#define OPAL_XIVE_SET_IRQ_CONFIG		131
#define OPAL_XIVE_GET_QUEUE_INFO		132
#define OPAL_XIVE_SET_QUEUE_INFO		133
#define OPAL_XIVE_DONATE_PAGE			134
#define OPAL_XIVE_ALLOCATE_VP_BLOCK		135
#define OPAL_XIVE_FREE_VP_BLOCK			136
#define OPAL_XIVE_GET_VP_INFO			137
#define OPAL_XIVE_SET_VP_INFO			138
#define OPAL_XIVE_ALLOCATE_IRQ			139
#define OPAL_XIVE_FREE_IRQ			140
#define OPAL_XIVE_SYNC				141
#define OPAL_XIVE_DUMP				142
#define OPAL_XIVE_GET_QUEUE_STATE		143 /* Get END state */
#define OPAL_XIVE_SET_QUEUE_STATE		144 /* Set END state */
#define OPAL_SIGNAL_SYSTEM_RESET		145
#define OPAL_NPU_INIT_CONTEXT			146
#define OPAL_NPU_DESTROY_CONTEXT		147
#define OPAL_NPU_MAP_LPAR			148
#define OPAL_IMC_COUNTERS_INIT			149
#define OPAL_IMC_COUNTERS_START			150
#define OPAL_IMC_COUNTERS_STOP			151
#define OPAL_GET_POWERCAP			152
#define OPAL_SET_POWERCAP			153
#define OPAL_GET_POWER_SHIFT_RATIO		154
#define OPAL_SET_POWER_SHIFT_RATIO		155
#define OPAL_SENSOR_GROUP_CLEAR			156
#define OPAL_PCI_SET_P2P			157
#define OPAL_QUIESCE				158
#define OPAL_NPU_SPA_SETUP			159
#define OPAL_NPU_SPA_CLEAR_CACHE		160
#define OPAL_NPU_TL_SET				161
#define OPAL_SENSOR_READ_U64			162
#define OPAL_SENSOR_GROUP_ENABLE		163
#define OPAL_PCI_GET_PBCQ_TUNNEL_BAR		164
#define OPAL_PCI_SET_PBCQ_TUNNEL_BAR		165
#define OPAL_HANDLE_HMI2			166
#define OPAL_NX_COPROC_INIT			167
#define OPAL_NPU_SET_RELAXED_ORDER		168
#define OPAL_NPU_GET_RELAXED_ORDER		169
#define OPAL_XIVE_GET_VP_STATE			170 /* Get NVT state */
#define OPAL_NPU_MEM_ALLOC			171
#define OPAL_NPU_MEM_RELEASE			172
#define OPAL_MPIPL_UPDATE			173
#define OPAL_MPIPL_REGISTER_TAG			174
#define OPAL_MPIPL_QUERY_TAG			175
#define OPAL_SECVAR_GET				176
#define OPAL_SECVAR_GET_NEXT			177
#define OPAL_SECVAR_ENQUEUE_UPDATE		178
#define OPAL_PHB_SET_OPTION			179
#define OPAL_PHB_GET_OPTION			180
#define OPAL_LAST				180

