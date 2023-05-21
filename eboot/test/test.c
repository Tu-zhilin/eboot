#include "eboot.h"
#include "eboot_cfg.h"
#include "fal.h"

extern uint16_t eboot_calc(uint16_t crc, uint8_t value);
extern int32_t eboot_fw_hdr_check(struct eboot_app_hdr* app_hdr);

int32_t test_eboot_fw_hdr_check()
{
	uint16_t crc = EBOOT_CRC_WORD;
	struct eboot_app_hdr app_hdr = { 0 };

	app_hdr.magic_word = EBOOT_MAGIC_WORD;
	app_hdr.version = 0x00000001;
	app_hdr.product_code = EBOOT_PRODUCT_CODE;
	app_hdr.code_size = 0x00000400;
	app_hdr.p_crc = 0xFFFF;

	for (uint8_t i = 0; i < sizeof(struct eboot_app_hdr) - 2; i++)
	{
		crc = eboot_calc(crc, *((uint8_t *)&app_hdr + i));
	}
	app_hdr.hdr_crc = crc;

	if (eboot_fw_hdr_check(&app_hdr) < 0)
	{
		EBOOT_LOG("[eboot_fw_hdr_check] test failed\r\n");
		return -1;
	}

	return 0;
}

void eboot_test()
{
	test_eboot_fw_hdr_check();
}