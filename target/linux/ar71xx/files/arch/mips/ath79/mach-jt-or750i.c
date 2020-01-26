/*
 * Joy-IT JT-OR750i board support
 *
 * Copyright (C) 2019 Vincent Wiemann <vcw@derowe.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/platform_device.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"

/* ath10k GPIO is not used as a WLAN-LED, but as a status LED with default
 * state "on" together with the inverted red status LED for signalling e.g.
 * failsafe mode
 */
#define OR750I_GPIO_LED_STATUS_GREEN	506
#define OR750I_GPIO_LED_STATUS_RED	13
#define OR750I_GPIO_LED_LAN1		16
#define OR750I_GPIO_LED_LAN2		15
#define OR750I_GPIO_LED_LAN3		14
#define OR750I_GPIO_LED_WAN		4

#define OR750I_GPIO_BTN_RESET		17

#define OR750I_KEYS_POLL_INTERVAL	20 /* msec */
#define OR750I_KEYS_DEBOUNCE_INTERVAL	(3 * OR750I_KEYS_POLL_INTERVAL)

#define OR750I_WMAC2G_CALDATA_OFFSET	0x1000
#define OR750I_WMAC5G_CALDATA_OFFSET	0x5000

static struct gpio_led or750i_gpio_leds[] __initdata = {
	{
		.name		= "jt-or750i:green:lan1",
		.gpio		= OR750I_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "jt-or750i:green:lan2",
		.gpio		= OR750I_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "jt-or750i:green:lan3",
		.gpio		= OR750I_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "jt-or750i:green:wan",
		.gpio		= OR750I_GPIO_LED_WAN,
		.active_low	= 1,
	}, {	/* Inverted active_low, because a red status LED looks broken */
		.name		= "jt-or750i:inv_red:status",
		.gpio		= OR750I_GPIO_LED_STATUS_RED,
		.active_low	= 0,
		.default_state	= 0,
	}
};

static struct gpio_keys_button or750i_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= OR750I_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= OR750I_GPIO_BTN_RESET,
		.active_low		= 1,
	}
};

static void __init or750i_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	/* WAN */
	ath79_init_mac(ath79_eth0_data.mac_addr, art, 0);
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.speed = SPEED_100;
	ath79_register_eth(0);

	/* LAN */
	ath79_init_mac(ath79_eth1_data.mac_addr, art + 6, 0);
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_register_eth(1);

	ath79_register_gpio_keys_polled(-1, OR750I_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(or750i_gpio_keys),
					or750i_gpio_keys);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(or750i_gpio_leds),
				 or750i_gpio_leds);

	ath79_register_usb();

	ath79_register_wmac(art + OR750I_WMAC2G_CALDATA_OFFSET, NULL);

	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_JT_OR750I, "JT-OR750I", "Joy-IT JT-OR750i", or750i_setup);
