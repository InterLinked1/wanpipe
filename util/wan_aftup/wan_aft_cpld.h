#ifndef __WAN_AFT_CPLD_H
# define __WAN_AFT_CPLD_H


#define MEMORY_TYPE_SRAM	0x00
#define MEMORY_TYPE_FLASH	0x01
#define MASK_MEMORY_TYPE_SRAM	0x10
#define MASK_MEMORY_TYPE_FLASH	0x20

#define DEF_SECTOR_FLASH	0x00
#define USER_SECTOR_FLASH	0x01
#define MASK_DEF_SECTOR_FLASH	0x00
#define MASK_USER_SECTOR_FLASH	0x04

#define WAN_AFTUP_A101		0x01
#define WAN_AFTUP_A102		0x02
#define WAN_AFTUP_A104		0x04
#define WAN_AFTUP_A300		0x08

typedef struct {
	void	*private;
	int	adptr_type;
} wan_aft_cpld_t;

#endif /* __WAN_AFT_CPLD_H*/
