#include "ahci.h"
#include "../lib/string.h"
#include "../lib/print.h"
#include "../kernel.h"

// See ahci.h for more info about struct member names

static void traceAHCI(const char* str) 
{
    term_write(str, strlen(str));
}

// Check device type
static int checkType(HBA_PORT* port)
{
	uint32_t ssts = port->ssts;
 
	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;
 
	if (det != HBA_PORT_DET_PRESENT)	// Check drive status
		return AHCI_DEV_NULL;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return AHCI_DEV_NULL;
 
	switch (port->sig)
	{
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        default:
            return AHCI_DEV_SATA;
	}
}
 
void probePort(HBA_MEM *abar)
{
	// Search disk in implemented ports
	uint32_t pi = abar->pi;
    for (size_t i = 0; i < 32; i++) 
	{
		if (pi & 1)
		{
            switch (checkType(&abar->ports[i])) 
            {
                case AHCI_DEV_SATA:
                    traceAHCI("SATA drive found at port \n");
                    break;
                case AHCI_DEV_SATAPI:
                    traceAHCI("SATAPI drive found at port \n");
                    break;
                case AHCI_DEV_SEMB:
                    traceAHCI("SEMB drive found at port \n");
                    break;
                case AHCI_DEV_PM:
                    traceAHCI("PM drive found at port \n");
                    break;
			    default:
                    traceAHCI("No drive found at port \n");
            }
            printNumber(i);
		}
		pi >>= 1;
	}
} 

// Start command engine
void startCMD(HBA_PORT *port)
{
	// Wait until CR (bit15) is cleared
	while (port->cmd & HBA_PxCMD_CR);
 
	// Set FRE (bit4) and ST (bit0)
	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST; 
}
 
// Stop command engine
void stopCMD(HBA_PORT *port)
{
	// Clear ST (bit0)
	port->cmd &= ~HBA_PxCMD_ST;
 
	// Clear FRE (bit4)
	port->cmd &= ~HBA_PxCMD_FRE;
 
	// Wait until FR (bit14), CR (bit15) are cleared
	while (true)
	{
		if (port->cmd & HBA_PxCMD_FR)
			continue;
		if (port->cmd & HBA_PxCMD_CR)
			continue;
		break;
	}
}

void portRebase(HBA_PORT *port, int port_no)
{
	stopCMD(port);	// Stop command engine
 
	// Command list offset: 1K*port_no
	// Command list entry size = 32
	// Command list entry maximum count = 32
	// Command list maximum size = 32*32 = 1K per port
	port->clb = AHCI_BASE + (port_no << 10);
	port->clbu = 0;
	memset((void*) (port->clb), 0, 1024);
 
	// FIS offset: 32K + 256*port_no
	// FIS entry size = 256 bytes per port
	port->fb = AHCI_BASE + (32 << 10) + (port_no << 8);
	port->fbu = 0;
	memset((void*) (port->fb), 0, 256);
 
	// Command table offset: 40K + 8K*port_no
	// Command table size = 256*32 = 8K per port
	HBA_CMD_HEADER* cmd_header = (HBA_CMD_HEADER*) (port->clb);
	for (int i = 0; i < 32; i++)
	{
		cmd_header[i].prdtl = 8;	// 8 prdt entries per command table
                                    // 256 bytes per command table, 64+16+48+16*8

		// Command table offset: 40K + 8K*port_no + cmd_header_index*256
		cmd_header[i].ctba = AHCI_BASE + (40 << 10) + (port_no << 13) + (i << 8);
		cmd_header[i].ctbau = 0;
		memset((void*) cmd_header[i].ctba, 0, 256);
	}
 
	startCMD(port);	// Start command engine
}
 
// Find a free command list slot
int findCMDSlot(HBA_PORT* port, size_t cmd_slots)
{
	// If not set in SACT and CI, the slot is free
	uint32_t slots = port->sact | port->ci;
    for (int i = 0; i < cmd_slots; i++)
	{
		if (!(slots & 1))
			return i;
		slots >>= 1;
	}
	traceAHCI("Cannot find free command list entry\n");
	return -1;
}

bool read(HBA_PORT *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t* buf)
{
	port->is = (uint32_t) -1;		// Clear pending interrupt bits
                                    //
	int spin = 0; // Spin lock timeout counter
	int slot = findCMDSlot(port, 32);

	if (slot == -1)
		return false;
 
	HBA_CMD_HEADER* cmd_header = (HBA_CMD_HEADER*) port->clb;
	cmd_header += slot;
	cmd_header->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);	// Command FIS size
	cmd_header->w = 0;		// Read from device
	cmd_header->prdtl = (uint16_t) ((count-1) >> 4) + 1;	// PRDT entries count
 
	HBA_CMD_TBL* cmd_tbl = (HBA_CMD_TBL*) (cmd_header->ctba);
	memset(cmd_tbl, 0, sizeof(HBA_CMD_TBL) +
 		(cmd_header->prdtl - 1) * sizeof(HBA_PRDT_ENTRY));
 
    size_t i;
	// 8K bytes (16 sectors) per PRDT
	for (i = 0; i < cmd_header->prdtl - 1; i++)
	{
		cmd_tbl->prdt_entry[i].dba = (uint32_t) buf;
		cmd_tbl->prdt_entry[i].dbc = 8*1024 - 1;	// 8K bytes (this value should always be set to 1 less than the actual value)
		cmd_tbl->prdt_entry[i].i = 1;
		buf += 4 * 1024;	// 4K words
		count -= 16;	// 16 sectors
	}
	// Last entry
	cmd_tbl->prdt_entry[i].dba = (uint32_t) buf;
	cmd_tbl->prdt_entry[i].dbc = (count << 9) - 1;	// 512 bytes per sector
	cmd_tbl->prdt_entry[i].i = 1;
 
	// Setup command
	FIS_REG_H2D* cmd_fis = (FIS_REG_H2D*) (&cmd_tbl->cfis);
 
	cmd_fis->fis_type = FIS_TYPE_REG_H2D;
	cmd_fis->c = 1;	// Command
	cmd_fis->command = ATA_CMD_READ_DMA_EX;
 
	cmd_fis->lba0 = (uint8_t) startl;
	cmd_fis->lba1 = (uint8_t) (startl >> 8);
	cmd_fis->lba2 = (uint8_t) (startl >> 16);
	cmd_fis->device = 1 << 6;	// LBA mode
 
	cmd_fis->lba3 = (uint8_t) (startl >> 24);
	cmd_fis->lba4 = (uint8_t) starth;
	cmd_fis->lba5 = (uint8_t) (starth >> 8);
 
	cmd_fis->countl = count & 0xFF;
	cmd_fis->counth = (count >> 8) & 0xFF;
 
	// The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		traceAHCI("Port is hung\n");
		return false;
	}
 
	port->ci = 1 << slot;	// Issue command
 
	// Wait for completion
	while (true)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if (!(port->ci & (1 << slot)))
			break;

		if (port->is & HBA_PxIS_TFES)	// Task file error
		{
			traceAHCI("Read disk error\n");
			return false;
		}
	}
 
	// Check again
	if (port->is & HBA_PxIS_TFES)
	{
		traceAHCI("Read disk error\n");
		return false;
	}
 
	return true;
}

bool write(HBA_PORT *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t* buf)
{
	port->is = (uint32_t) -1;

	int spin = 0;
	int slot = findCMDSlot(port, 32);
;
	if (slot == -1)
		return false;
 
	HBA_CMD_HEADER* cmd_header = (HBA_CMD_HEADER*) port->clb;
	cmd_header += slot;
	cmd_header->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
	cmd_header->w = 1;		// Write to device
	cmd_header->prdtl = (uint16_t) ((count - 1) >> 4) + 1;
 
	HBA_CMD_TBL* cmd_tbl = (HBA_CMD_TBL*) (cmd_header->ctba);
	memset(cmd_tbl, 0, sizeof(HBA_CMD_TBL) +
 		(cmd_header->prdtl - 1) * sizeof(HBA_PRDT_ENTRY));
 
    size_t i;

	for (i = 0; i < cmd_header->prdtl - 1; i++)
	{
		cmd_tbl->prdt_entry[i].dba = (uint32_t) buf;
		cmd_tbl->prdt_entry[i].dbc = 8*1024 - 1;
		cmd_tbl->prdt_entry[i].i = 1;
		buf += 4 * 1024;
		count -= 16;
	}

	cmd_tbl->prdt_entry[i].dba = (uint32_t) buf;
	cmd_tbl->prdt_entry[i].dbc = (count << 9) - 1;	// 512 bytes per sector
	cmd_tbl->prdt_entry[i].i = 1;
 
	// Setup command
	FIS_REG_H2D* cmd_fis = (FIS_REG_H2D*) (&cmd_tbl->cfis);
 
	cmd_fis->fis_type = FIS_TYPE_REG_H2D;
	cmd_fis->c = 1;
	cmd_fis->command = ATA_CMD_WRITE_DMA_EX;
 
	cmd_fis->lba0 = (uint8_t) startl;
	cmd_fis->lba1 = (uint8_t) (startl >> 8);
	cmd_fis->lba2 = (uint8_t) (startl >> 16);
	cmd_fis->device = 1 << 6;
 
	cmd_fis->lba3 = (uint8_t) (startl >> 24);
	cmd_fis->lba4 = (uint8_t) starth;
	cmd_fis->lba5 = (uint8_t) (starth >> 8);
 
	cmd_fis->countl = count & 0xFF;
	cmd_fis->counth = (count >> 8) & 0xFF;
 
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
		spin++;

	if (spin == 1000000)
	{
		traceAHCI("Port is hung\n");
		return false;
	}
 
	port->ci = 1 << slot;
 
	while (true)
	{
		if (!(port->ci & (1 << slot)))
			break;

		if (port->is & HBA_PxIS_TFES)
		{
			traceAHCI("Write disk error\n");
			return false;
		}
	}
 
	if (port->is & HBA_PxIS_TFES)
	{
		traceAHCI("Write disk error\n");
		return false;
	}
 
	return true;
}
