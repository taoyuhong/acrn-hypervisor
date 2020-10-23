/*
 * Copyright (C) 2020 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VUART_PCI_H
#define VUART_PCI_H

#define COM_VENDOR	0x9710U
#define COM_DEV		0x9900U

extern const struct pci_vdev_ops vuart_pci_ops;
void trigger_vuart_msi(struct pci_vdev *vdev);
int32_t create_vuart_vdev(struct acrn_vm *vm, struct acrn_emul_dev *dev);
int32_t destroy_vuart_vdev(struct acrn_vm *vm, struct acrn_emul_dev *dev);

#define pr_vpci(FMT, ...) do { \
	struct acrn_vm *vm = vpci2vm(vdev->vpci); \
        pr_warn("vm%d vuart %02x:%02x.%x " FMT, \
                        vm->vm_id, vdev->bdf.bits.b, vdev->bdf.bits.d, vdev->bdf.bits.f, \
                        __VA_ARGS__); }while(0)

#endif
