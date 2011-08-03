/*
	LPCXpresso memory definitions
*/

MEMORY
{
  MFlash512 (rx) : ORIGIN = 0x0, LENGTH = 0x80000 /* 512k */
  RamLoc32 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x8000 /* 32k */
  RamAHB32 (rwx) : ORIGIN = 0x2007c000, LENGTH = 0x8000 /* 32k */
  CANAccFilterRAM (rwx) : ORIGIN = 0x40038000, LENGTH = 0x800 /* 2k */
}