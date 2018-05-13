#ifndef BACKUP_SRAM
#define  BACKUP_SRAM


void disable_backup_sram();
void enable_backup_sram();


#define BACKUP_SRAM_START 	BKPSRAM_BB_BASE
#define BACKUP_SRAM_LEN		0x1000	/* (4kB) */


#endif /* BACKUP_SRAM */
