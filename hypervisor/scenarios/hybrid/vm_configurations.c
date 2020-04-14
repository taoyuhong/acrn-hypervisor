/*
 * Copyright (C) 2018 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <vm_config.h>
#include <vuart.h>
#include <pci_dev.h>
#include <pci_devices.h>
#include <dm/vpci.h>

struct acrn_vm_pci_dev_config vm0_pci_devs[3] = {
	{
		.emu_type = PCI_DEV_TYPE_HVEMUL,
		.vbdf.bits = {.b = 0x00U, .d = 0x00U, .f = 0x00U},
		.vdev_ops = &vhostbridge_ops,
	},
	{
		.emu_type = PCI_DEV_TYPE_PTDEV,
		.vbdf.bits = {.b = 0x00U, .d = 0x01U, .f = 0x00U},
		NON_VOLATILE_MEMORY_CONTROLLER_0
	},
	{
		.emu_type = PCI_DEV_TYPE_PTDEV,
		.vbdf.bits = {.b = 0x00U, .d = 0x02U, .f = 0x00U},
		ETHERNET_CONTROLLER_1
	},
};

struct acrn_vm_config vm_configs[CONFIG_MAX_VM_NUM] = {
	{	/* VM0 */
		CONFIG_PRE_STD_VM(1),
		.name = "ACRN PRE-LAUNCHED VM0",
		.cpu_affinity = VM0_CONFIG_CPU_AFFINITY,
		//.guest_flags = (GUEST_FLAG_RT | GUEST_FLAG_LAPIC_PASSTHROUGH),
		.guest_flags = 0U,
		.memory = {
			.start_hpa = VM0_CONFIG_MEM_START_HPA,
			.size = VM0_CONFIG_MEM_SIZE,
		},
		.os_config = {
			.name = "ClearLinux",
			.kernel_type = KERNEL_BZIMAGE,
			.kernel_mod_tag = "RT_bzImage",
			.bootargs = "console=tty0 console=ttyS0,115200n8 earlyprintk=ttyS0 root=/dev/nvme0n1p3 rw rootwait ignore_loglevel no_timer_check consoleblank=0 x2apic_phys hvlog=2M@0x1FE00000 memmap=2M$0x1FE00000  maxcpus=4 nohpet tsc=reliable processor.max_cstate=0 intel_idle.max_cstate=0  mce=ignore_ce audit=0 isolcpus=nohz,domain,1 nohz_full=1 rcu_nocbs=1 nosoftlockup idle=poll irqaffinity=0 nopcid ",
		},
		.vuart[0] = {
			.type = VUART_LEGACY_PIO,
			.addr.port_base = COM1_BASE,
			.irq = COM1_IRQ,
		},
		.vuart[1] = {
			.type = VUART_LEGACY_PIO,
			.addr.port_base = COM2_BASE,
			.irq = COM2_IRQ,
			.t_vuart.vm_id = 1U,
			.t_vuart.vuart_id = 1U,
		},
		.pci_dev_num = 3,
		.pci_devs = vm0_pci_devs,
	},
	{	/* VM1 */
		CONFIG_SOS_VM,
		.name = "ACRN SOS VM",
		.guest_flags = 0UL,
		.memory = {
			.start_hpa = 0UL,
			.size = CONFIG_SOS_RAM_SIZE,
		},
		.os_config = {
			.name = "ACRN Service OS",
			.kernel_type = KERNEL_BZIMAGE,
			.kernel_mod_tag = "SOS_bzImage",
			.bootargs = SOS_VM_BOOTARGS,
		},
		.vuart[0] = {
			.type = VUART_LEGACY_PIO,
			.addr.port_base = SOS_COM1_BASE,
			.irq = SOS_COM1_IRQ,
		},
		.vuart[1] = {
			.type = VUART_LEGACY_PIO,
			.addr.port_base = SOS_COM2_BASE,
			.irq = SOS_COM2_IRQ,
			.t_vuart.vm_id = 0U,
			.t_vuart.vuart_id = 1U,
		},
	},
	{	/* VM2 */
		CONFIG_POST_STD_VM(1),
		.cpu_affinity = VM2_CONFIG_CPU_AFFINITY,
		.vuart[0] = {
			.type = VUART_LEGACY_PIO,
			.addr.port_base = COM1_BASE,
			.irq = COM1_IRQ,
		},
		.vuart[1] = {
			.type = VUART_LEGACY_PIO,
			.addr.port_base = INVALID_COM_BASE,
		}
	}
};
