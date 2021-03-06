#ifndef ATA_H
#define ATA_H

#include <drivers/pci.h>
#include <stddef.h>
#include <stdint.h>

typedef struct ata_drive_s
{
	uint32_t base;
	uint32_t ctrl;

	int id;
	int mode;
	int type;
	int slave;

	int capabilities;
	int command_set; /* 28bit or 48 bit */
	int signature;
	int max_lba;

	char model[41];

} ata_drive_t;

int init_ata(pci_device_t *pci_dev);

#endif