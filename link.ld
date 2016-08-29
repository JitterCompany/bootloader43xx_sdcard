
MEMORY
{
  /* Define each memory region */
  Flash_M0 (rx) : 	ORIGIN = 0x1b000000, LENGTH = 0x80000
  Flash_M4 (rx) : 	ORIGIN = 0x1a000000, LENGTH = 0x80000
  RAM_M0 (rwx) : 	ORIGIN = 0x10000000, LENGTH = 0x8000
  RAM_M4 (rwx) : 	ORIGIN = 0x10080000, LENGTH = 0xA000
  SharedRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x8000
  RAM_device_id (rw) : ORIGIN = 0x20008000, LENGTH = 0x10
  RAM_M4_global (rwx) : ORIGIN = 0x20008010, LENGTH = 0x7FF0
}

/* Define a symbol for the top of each memory region */
__top_Flash_M0 = ORIGIN(Flash_M0) + LENGTH(Flash_M0);
__top_Flash_M4 = ORIGIN(Flash_M4) + LENGTH(Flash_M4);
__top_RAM_M0 = ORIGIN(RAM_M0) + LENGTH(RAM_M0);
__top_RAM_M4 = ORIGIN(RAM_M4) + LENGTH(RAM_M4);
__top_SharedRAM = ORIGIN(SharedRAM) + LENGTH(SharedRAM);
__top_RAM_device_id = ORIGIN(RAM_device_id) + LENGTH(RAM_device_id);
__top_RAM_M4_global = ORIGIN(SharedRAM) + LENGTH(SharedRAM);


SECTIONS
{
	/* BSS section for SharedRam */
	.shared_bss (NOLOAD) : ALIGN(4)
	{
        __shared_RAM_begin__ = ABSOLUTE(.) ;
	} > SharedRAM
		. = __top_SharedRAM ;
		__shared_RAM_end__ = ABSOLUTE(.);

 	/* BSS section for RAM_device_id */
	.device_id_bss (NOLOAD) : ALIGN(4)
	{
        __RAM_device_id_begin__ = ABSOLUTE(.) ;
        *(.bss.$device_id*)
        . = ALIGN(4) ;
	} > RAM_device_id
		. = __top_RAM_device_id ;
		__RAM_device_id_end__ = ABSOLUTE(.);

 	/* BSS section for RAM_M4_global */
	.m4_global_bss (NOLOAD) : ALIGN(4)
	{
        __RAM_M4_global_begin__ = ABSOLUTE(.) ;
        *(.bss.$m4_global*)
        . = ALIGN(4) ;
	} > RAM_M4_global
		. = __top_RAM_M4_global ;
		__RAM_M4_global_end__ = ABSOLUTE(.);
}

