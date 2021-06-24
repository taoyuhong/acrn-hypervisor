/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <types.h>
#include <asm/vm_config.h>
#include <asm/guest/vm.h>
#include <vacpi.h>
#include <logmsg.h>
#include <asm/guest/ept.h>

static void *get_acpi_mod_entry(struct acrn_vm *vm, const char *signature)
{
	struct acpi_table_xsdt *xsdt;
	uint32_t i, entry_cnt = 0U;
	struct acpi_table_header *header = NULL, *find = NULL;

	xsdt = vm->sw.acpi_info.src_addr + VIRT_XSDT_ADDR - VIRT_ACPI_DATA_ADDR;
	entry_cnt = (xsdt->header.length - sizeof(xsdt->header)) / (sizeof(xsdt->table_offset_entry[0]));

	for (i = 0; i < entry_cnt; i++) {
		header =
		    vm->sw.acpi_info.src_addr + xsdt->table_offset_entry[i] -
		    VIRT_ACPI_DATA_ADDR;
		if (strncmp(header->signature, signature, ACPI_NAME_SIZE) == 0) {
			find = header;
			break;
		}
	}

	return find;
}

static void tpm2_fixup(struct acrn_vm *vm)
{
	struct acpi_table_tpm2 *tpm2 = NULL, *native = NULL;
	struct acrn_vm_config *config = get_vm_config(vm->vm_id);
	bool need_fix = false;
	uint8_t checksum;

	tpm2 = get_acpi_mod_entry(vm, ACPI_SIG_TPM2);
	native = get_acpi_tbl(ACPI_SIG_TPM2);

	if (config->pt_tpm2) {
		if ((tpm2 != NULL) && (native != NULL)) {
			/* Native has different start method */
			need_fix = tpm2->start_method != native->start_method;

			/* Native has event log */
			if (native->header.length ==
			    sizeof(struct acpi_table_tpm2)) {
				need_fix |= tpm2->header.length == 0x34U;
				need_fix |= strncmp((char *)tpm2->start_method_spec_para, (char *)native->start_method_spec_para,
					    sizeof(tpm2->start_method_spec_para)) != 0;
				need_fix |= tpm2->laml != native->laml;
				need_fix |= tpm2->lasa != native->lasa;
			}

			if (need_fix) {
				pr_err("%s tpm2 fix start method and event log field", __FUNCTION__);
				tpm2->start_method = native->start_method;
				tpm2->header.length = native->header.length;
				tpm2->header.revision = native->header.revision;
				memcpy_s(&native->start_method_spec_para, sizeof(native->start_method_spec_para),
					 &tpm2->start_method_spec_para, sizeof(native->start_method_spec_para));

				ept_del_mr(vm, (uint64_t *)vm->arch_vm.nworld_eptp, config->mmiodevs[0].mmiores[1].base_gpa,
						config->mmiodevs[0].mmiores[1].size);  

				tpm2->laml = native->laml;
				tpm2->lasa = 0x40890000U;

				tpm2->header.checksum = 0;
				checksum = calculate_checksum8(tpm2, sizeof(struct acpi_table_tpm2));
				tpm2->header.checksum = checksum;

				config->mmiodevs[0].mmiores[1].base_hpa = native->lasa;
				config->mmiodevs[0].mmiores[1].base_gpa = 0x40890000U;
				config->mmiodevs[0].mmiores[1].size = tpm2->laml;

				ept_add_mr(vm, (uint64_t *)vm->arch_vm.nworld_eptp,config->mmiodevs[0].mmiores[1].base_hpa, 
					       config->mmiodevs[0].mmiores[1].base_gpa, config->mmiodevs[0].mmiores[1].size, 
				       		 EPT_RWX | EPT_UNCACHED);	       
			}
		} else {
			pr_err("VM or native can't find TPM2 ACPI table");
		}
	}
}

void vm_info_fixup(struct acrn_vm *vm)
{
	if (is_prelaunched_vm(vm)) {
		tpm2_fixup(vm);
	}
}

