/* L4Xpresso memory area definitions 
*/

INCLUDE "../mk/leo4_mem.ld"

ENTRY (__l4_start)

/*
 * Memory Relocation 
 * 
 * +-------------------+	0x000000000 
 * +  isr_vectors      + 
 * +--------------------	0x000001000	(PROC_TABLE_BASE)
 * +  proc_table       |
 * +--------------------	0x000002000 (KERNEL_BASE)
 * +  text / data      +    30k
 * +--------------------	0x000008000 (USERSPACE_BASE)
 * |....Userspace......|
 * ~~~~~~~~~~~~~~~~~~~~~
 *
 * +-------------------+	0x100000000 
 * +  stack            + 2k	
 * +-------------------+	0x100000800 (+KERNEL_STACK)
 * +  data / bss       + 10k
 * ~~~~~~~~~~~~~~~~~~~~~	0x100003000 (+KERNEL_DATA)
 * |....Userspace......|
 * ~~~~~~~~~~~~~~~~~~~~~    0x10007c000 (+USER_DATA) 
 * +  KIP              + 1k
 * +-------------------+
 *
 * 
 * +-------------------+	0x2007C000
 * +  AHB mem          +   
 * +-------------------+
 * 
 *
 */


PROC_TABLE_BASE = 0x00000800;
KERNEL_BASE = 0x00001000;
USERSPACE_BASE = 0x000008000;

KERNEL_STACK_SIZE    = 0x00000400;

KERNEL_STACK_ADDR = 0x10000000;
KERNEL_DATA_ADDR =  0x10000400;
KERNEL_KIP_ADDR =   0x10007e00;

GROUP(libcr_c.a libcr_eabihelpers.a) 

SECTIONS {
	.text 0x00000000:
	{
		KEEP(*(.isr_vector))
		. = PROC_TABLE_BASE;
		*(.proctable*)
		. = KERNEL_BASE;
		kernel_text_start = .;
		*(.text*)
		*(.rodata*)
		kernel_text_end = .;
	} > MFlash512
	
	.user_text 0x00008000 : ALIGN(256)
	{
		user_text_start = .;
		*(.user_text*)
		user_text_end = .;
	} > MFlash512
	
	.stack :
	{
		. += KERNEL_STACK_SIZE;
		kernel_stack_addr = .;
	} > RamLoc32
	
	.data :
	AT( ADDR(.text) + SIZEOF(.text) )
	{
		kernel_data_start = .;
		kip_start = .;
		./kernel/src/start.o (kip)
		kip_end = .;
		/*Only kernel data*/
		*(.data*)
		kernel_data_end = .;
	} >RamLoc32

	/* zero initialized data */
	.bss :
	{
		kernel_bss_start = .;
		*(.bss*)
	    *(.ktable*)			/* Statically allocated kernel table*/
		*(COMMON)
		kernel_bss_end = .;
	} > RamLoc32
	
	.user_data : 
	AT( ADDR(.user_text) + SIZEOF(.user_text) )
	ALIGN(256)
	{
		user_data_start = .;
		*(.user_data*)
		user_data_end = .;
	} > RamLoc32 
	
	.user_bss : ALIGN(256)
	{
		user_bss_start = .;
		*(.user_bss*)
		user_bss_end = .;
	} > RamLoc32

	.data_AHB :
	{
		kernel_ahb_start = .;
		*(.bitmap*)
		kernel_ahb_end = .;
	} > RamAHB32
}