/*
 * SoC-specific lowlevel code for tms320dm365 and similar chips
 * Actually used for booting from NAND with nand_spl.
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <common.h>
#include <nand.h>
#include <ns16550.h>
#include <post.h>
#include <asm/arch/dm365_lowlevel.h>
#include <asm/arch/hardware.h>

void dm365_waitloop(unsigned long loopcnt)
{
	unsigned long	i;

	for (i = 0; i < loopcnt; i++)
		asm("   NOP");
}

int dm365_pll1_init(unsigned long pllmult, unsigned long prediv)
{
	unsigned int clksrc = 0x0;

	/* Power up the PLL */
	clrbits_le32(&dv_pll0_regs->pllctl, PLLCTL_PLLPWRDN);

	clrbits_le32(&dv_pll0_regs->pllctl, PLLCTL_RES_9);
	setbits_le32(&dv_pll0_regs->pllctl, clksrc << 8);

	/*
	 * Set PLLENSRC '0', PLL Enable(PLLEN) selection is controlled
	 * through MMR
	 */
	clrbits_le32(&dv_pll0_regs->pllctl, PLLCTL_PLLENSRC);

	/* Set PLLEN=0 => PLL BYPASS MODE */
	clrbits_le32(&dv_pll0_regs->pllctl, PLLCTL_PLLEN);

	dm365_waitloop(150);

	 /* PLLRST=1(reset assert) */
	setbits_le32(&dv_pll0_regs->pllctl, PLLCTL_PLLRST);

	dm365_waitloop(300);

	/*Bring PLL out of Reset*/
	clrbits_le32(&dv_pll0_regs->pllctl, PLLCTL_PLLRST);

	/* Program the Multiper and Pre-Divider for PLL1 */
	writel(pllmult, &dv_pll0_regs->pllm);
	writel(prediv, &dv_pll0_regs->prediv);

	/* Assert TENABLE = 1, TENABLEDIV = 1, TINITZ = 1 */
	writel(PLLSECCTL_STOPMODE | PLLSECCTL_TENABLEDIV | PLLSECCTL_TENABLE |
		PLLSECCTL_TINITZ, &dv_pll0_regs->secctl);
	/* Assert TENABLE = 1, TENABLEDIV = 1, TINITZ = 0 */
	writel(PLLSECCTL_STOPMODE | PLLSECCTL_TENABLEDIV | PLLSECCTL_TENABLE,
		&dv_pll0_regs->secctl);
	/* Assert TENABLE = 0, TENABLEDIV = 0, TINITZ = 0 */
	writel(PLLSECCTL_STOPMODE, &dv_pll0_regs->secctl);
	/* Assert TENABLE = 0, TENABLEDIV = 0, TINITZ = 1 */
	writel(PLLSECCTL_STOPMODE | PLLSECCTL_TINITZ, &dv_pll0_regs->secctl);

	/* Program the PostDiv for PLL1 */
	writel(0x8000, &dv_pll0_regs->postdiv);

	/* Post divider setting for PLL1 */
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV1, &dv_pll0_regs->plldiv1);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV2, &dv_pll0_regs->plldiv2);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV3, &dv_pll0_regs->plldiv3);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV4, &dv_pll0_regs->plldiv4);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV5, &dv_pll0_regs->plldiv5);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV6, &dv_pll0_regs->plldiv6);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV7, &dv_pll0_regs->plldiv7);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV8, &dv_pll0_regs->plldiv8);
	writel(CONFIG_SYS_DM36x_PLL1_PLLDIV9, &dv_pll0_regs->plldiv9);

	dm365_waitloop(300);

	/* Set the GOSET bit */
	writel(PLLCMD_GOSET, &dv_pll0_regs->pllcmd); /* Go */

	dm365_waitloop(300);

	/* Wait for PLL to LOCK */
	while (!((readl(&dv_sys_module_regs->pll0_config) & PLL0_LOCK)
		== PLL0_LOCK))
		;

	/* Enable the PLL Bit of PLLCTL*/
	setbits_le32(&dv_pll0_regs->pllctl, PLLCTL_PLLEN);

	return 0;
}

int dm365_pll2_init(unsigned long pllm, unsigned long prediv)
{
	unsigned int clksrc = 0x0;

	/* Power up the PLL*/
	clrbits_le32(&dv_pll1_regs->pllctl, PLLCTL_PLLPWRDN);

	/*
	 * Select the Clock Mode as Onchip Oscilator or External Clock on
	 * MXI pin
	 * VDB has input on MXI pin
	 */
	clrbits_le32(&dv_pll1_regs->pllctl, PLLCTL_RES_9);
	setbits_le32(&dv_pll1_regs->pllctl, clksrc << 8);

	/*
	 * Set PLLENSRC '0', PLL Enable(PLLEN) selection is controlled
	 * through MMR
	 */
	clrbits_le32(&dv_pll1_regs->pllctl, PLLCTL_PLLENSRC);

	/* Set PLLEN=0 => PLL BYPASS MODE */
	clrbits_le32(&dv_pll1_regs->pllctl, PLLCTL_PLLEN);

	dm365_waitloop(50);

	 /* PLLRST=1(reset assert) */
	setbits_le32(&dv_pll1_regs->pllctl, PLLCTL_PLLRST);

	dm365_waitloop(300);

	/* Bring PLL out of Reset */
	clrbits_le32(&dv_pll1_regs->pllctl, PLLCTL_PLLRST);

	/* Program the Multiper and Pre-Divider for PLL2 */
	writel(pllm, &dv_pll1_regs->pllm);
	writel(prediv, &dv_pll1_regs->prediv);

	writel(0x8000, &dv_pll1_regs->postdiv);

	/* Assert TENABLE = 1, TENABLEDIV = 1, TINITZ = 1 */
	writel(PLLSECCTL_STOPMODE | PLLSECCTL_TENABLEDIV | PLLSECCTL_TENABLE |
		PLLSECCTL_TINITZ, &dv_pll1_regs->secctl);
	/* Assert TENABLE = 1, TENABLEDIV = 1, TINITZ = 0 */
	writel(PLLSECCTL_STOPMODE | PLLSECCTL_TENABLEDIV | PLLSECCTL_TENABLE,
		&dv_pll1_regs->secctl);
	/* Assert TENABLE = 0, TENABLEDIV = 0, TINITZ = 0 */
	writel(PLLSECCTL_STOPMODE, &dv_pll1_regs->secctl);
	/* Assert TENABLE = 0, TENABLEDIV = 0, TINITZ = 1 */
	writel(PLLSECCTL_STOPMODE | PLLSECCTL_TINITZ, &dv_pll1_regs->secctl);

	/* Post divider setting for PLL2 */
	writel(CONFIG_SYS_DM36x_PLL2_PLLDIV1, &dv_pll1_regs->plldiv1);
	writel(CONFIG_SYS_DM36x_PLL2_PLLDIV2, &dv_pll1_regs->plldiv2);
	writel(CONFIG_SYS_DM36x_PLL2_PLLDIV3, &dv_pll1_regs->plldiv3);
	writel(CONFIG_SYS_DM36x_PLL2_PLLDIV4, &dv_pll1_regs->plldiv4);
	writel(CONFIG_SYS_DM36x_PLL2_PLLDIV5, &dv_pll1_regs->plldiv5);

	/* GoCmd for PostDivider to take effect */
	writel(PLLCMD_GOSET, &dv_pll1_regs->pllcmd);

	dm365_waitloop(150);

	/* Wait for PLL to LOCK */
	while (!((readl(&dv_sys_module_regs->pll1_config) & PLL1_LOCK)
		== PLL1_LOCK))
		;

	dm365_waitloop(4100);

	/* Enable the PLL2 */
	setbits_le32(&dv_pll1_regs->pllctl, PLLCTL_PLLEN);

	/* do this after PLL's have been set up */
	writel(CONFIG_SYS_DM36x_PERI_CLK_CTRL,
		&dv_sys_module_regs->peri_clkctl);

	return 0;
}

int dm365_ddr_setup(void)
{
	lpsc_on(DAVINCI_LPSC_DDR_EMIF);
	clrbits_le32(&dv_sys_module_regs->vtpiocr,
		VPTIO_IOPWRDN | VPTIO_CLRZ | VPTIO_LOCK | VPTIO_PWRDN);

	/* Set bit CLRZ (bit 13) */
	setbits_le32(&dv_sys_module_regs->vtpiocr, VPTIO_CLRZ);

	/* Check VTP READY Status */
	while (!(readl(&dv_sys_module_regs->vtpiocr) & VPTIO_RDY))
		;

	/* Set bit VTP_IOPWRDWN bit 14 for DDR input buffers) */
	setbits_le32(&dv_sys_module_regs->vtpiocr, VPTIO_IOPWRDN);

	/* Set bit LOCK(bit7) */
	setbits_le32(&dv_sys_module_regs->vtpiocr, VPTIO_LOCK);

	/*
	 * Powerdown VTP as it is locked (bit 6)
	 * Set bit VTP_IOPWRDWN bit 14 for DDR input buffers)
	 */
	setbits_le32(&dv_sys_module_regs->vtpiocr,
		VPTIO_IOPWRDN | VPTIO_PWRDN);

	/* Wait for calibration to complete */
	dm365_waitloop(150);

	/* Set the DDR2 to synreset, then enable it again */
	lpsc_syncreset(DAVINCI_LPSC_DDR_EMIF);
	lpsc_on(DAVINCI_LPSC_DDR_EMIF);

	writel(CONFIG_SYS_DM36x_DDR2_DDRPHYCR, &dv_ddr2_regs_ctrl->ddrphycr);

	/* Program SDRAM Bank Config Register */
	writel((CONFIG_SYS_DM36x_DDR2_SDBCR | DV_DDR_BOOTUNLOCK),
		&dv_ddr2_regs_ctrl->sdbcr);
	writel((CONFIG_SYS_DM36x_DDR2_SDBCR | DV_DDR_TIMUNLOCK),
		&dv_ddr2_regs_ctrl->sdbcr);

	/* Program SDRAM Timing Control Register1 */
	writel(CONFIG_SYS_DM36x_DDR2_SDTIMR, &dv_ddr2_regs_ctrl->sdtimr);
	/* Program SDRAM Timing Control Register2 */
	writel(CONFIG_SYS_DM36x_DDR2_SDTIMR2, &dv_ddr2_regs_ctrl->sdtimr2);

	writel(CONFIG_SYS_DM36x_DDR2_PBBPR, &dv_ddr2_regs_ctrl->pbbpr);

	writel(CONFIG_SYS_DM36x_DDR2_SDBCR, &dv_ddr2_regs_ctrl->sdbcr);

	/* Program SDRAM Refresh Control Register */
	writel(CONFIG_SYS_DM36x_DDR2_SDRCR, &dv_ddr2_regs_ctrl->sdrcr);

	lpsc_syncreset(DAVINCI_LPSC_DDR_EMIF);
	lpsc_on(DAVINCI_LPSC_DDR_EMIF);

	return 0;
}

void dm365_vpss_sync_reset(void)
{
	unsigned int PdNum = 0;

	/* VPSS_CLKMD 1:1 */
	setbits_le32(&dv_sys_module_regs->vpss_clkctl,
		VPSS_CLK_CTL_VPSS_CLKMD);

	/* LPSC SyncReset DDR Clock Enable */
	writel(((readl(&dv_psc_regs->mdctl[47]) & ~PSC_MD_STATE_MSK) |
		PSC_SYNCRESET), &dv_psc_regs->mdctl[47]);

	writel((1 << PdNum), &dv_psc_regs->ptcmd);

	while (!(((readl(&dv_psc_regs->ptstat) >> PdNum) & PSC_GOSTAT) == 0))
		;
	while (!((readl(&dv_psc_regs->mdstat[47]) &  PSC_MD_STATE_MSK) ==
		PSC_SYNCRESET))
		;
}

void dm365_por_reset(void)
{
	if (readl(&dv_pll0_regs->rstype) & 3)
		dm365_vpss_sync_reset();
}

void dm365_psc_init(void)
{
	unsigned char i = 0;
	unsigned char lpsc_start;
	unsigned char lpsc_end, lpscgroup, lpscmin, lpscmax;
	unsigned int  PdNum = 0;

	lpscmin = 0;
	lpscmax = 2;

	for (lpscgroup = lpscmin; lpscgroup <= lpscmax; lpscgroup++) {
		if (lpscgroup == 0) {
			lpsc_start = 0; /* Enabling LPSC 3 to 28 SCR first */
			lpsc_end   = 28;
		} else if (lpscgroup == 1) { /* Skip locked LPSCs [29-37] */
			lpsc_start = 38;
			lpsc_end   = 47;
		} else {
			lpsc_start = 50;
			lpsc_end   = 51;
		}

		/* NEXT=0x3, Enable LPSC's */
		for (i = lpsc_start; i <= lpsc_end; i++)
			setbits_le32(&dv_psc_regs->mdctl[i], 0x3);

		/*
		 * Program goctl to start transition sequence for LPSCs
		 * CSL_PSC_0_REGS->PTCMD = (1<<PdNum); Kick off Power
		 * Domain 0 Modules
		 */
		writel((1 << PdNum), &dv_psc_regs->ptcmd);

		/*
		 * Wait for GOSTAT = NO TRANSITION from PSC for Powerdomain 0
		 */
		while (!(((readl(&dv_psc_regs->ptstat) >> PdNum) & PSC_GOSTAT)
			== 0))
			;

		/* Wait for MODSTAT = ENABLE from LPSC's */
		for (i = lpsc_start; i <= lpsc_end; i++)
			while (!((readl(&dv_psc_regs->mdstat[i]) &
				PSC_MD_STATE_MSK) == 0x3))
				;
	}
}

static void dm365_emif_init(void)
{
	writel(CONFIG_SYS_DM36x_AWCCR, &davinci_emif_regs->awccr);
	writel(CONFIG_SYS_DM36x_AB1CR, &davinci_emif_regs->ab1cr);

	setbits_le32(&davinci_emif_regs->nandfcr, 1);

	writel(CONFIG_SYS_DM36x_AB2CR, &davinci_emif_regs->ab2cr);

	return;
}

void dm365_pinmux_ctl(unsigned long offset, unsigned long mask,
	unsigned long value)
{
	clrbits_le32(&dv_sys_module_regs->pinmux[offset], mask);
	setbits_le32(&dv_sys_module_regs->pinmux[offset], (mask & value));
}

__attribute__((weak))
void board_gpio_init(void)
{
	return;
}

#if defined(CONFIG_POST)
int post_log(char *format, ...)
{
	return 0;
}
#endif

void dm36x_lowlevel_init(ulong bootflag)
{
	/*
	 * copied from arch/arm/cpu/arm926ejs/start.S
	 *
	 * flush v4 I/D caches
	 */
	asm("mov	r0, #0");
	asm("mcr	p15, 0, r0, c7, c7, 0");	/* flush v3/v4 cache */
	asm("mcr	p15, 0, r0, c8, c7, 0");	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	asm("mrc	p15, 0, r0, c1, c0, 0");
	/* clear bits 13, 9:8 (--V- --RS) */
	asm("bic	r0, r0, #0x00002300");
	/* clear bits 7, 2:0 (B--- -CAM) */
	asm("bic	r0, r0, #0x00000087");
	/* set bit 2 (A) Align */
	asm("orr	r0, r0, #0x00000002");
	/* set bit 12 (I) I-Cache */
	asm("orr	r0, r0, #0x00001000");
	asm("mcr	p15, 0, r0, c1, c0, 0");

	/* Mask all interrupts */
	writel(0x04, &dv_aintc_regs->intctl);
	writel(0x0, &dv_aintc_regs->eabase);
	writel(0x0, &dv_aintc_regs->eint0);
	writel(0x0, &dv_aintc_regs->eint1);

	/* Clear all interrupts */
	writel(0xffffffff, &dv_aintc_regs->fiq0);
	writel(0xffffffff, &dv_aintc_regs->fiq1);
	writel(0xffffffff, &dv_aintc_regs->irq0);
	writel(0xffffffff, &dv_aintc_regs->irq1);

	/* System PSC setup - enable all */
	dm365_psc_init();

	/* Setup Pinmux */
	dm365_pinmux_ctl(0, 0xFFFFFFFF, CONFIG_SYS_DM36x_PINMUX0);
	dm365_pinmux_ctl(1, 0xFFFFFFFF, CONFIG_SYS_DM36x_PINMUX1);
	dm365_pinmux_ctl(2, 0xFFFFFFFF, CONFIG_SYS_DM36x_PINMUX2);
	dm365_pinmux_ctl(3, 0xFFFFFFFF, CONFIG_SYS_DM36x_PINMUX3);
	dm365_pinmux_ctl(4, 0xFFFFFFFF, CONFIG_SYS_DM36x_PINMUX4);

	/* PLL setup */
	dm365_pll1_init(CONFIG_SYS_DM36x_PLL1_PLLM,
		CONFIG_SYS_DM36x_PLL1_PREDIV);
	dm365_pll2_init(CONFIG_SYS_DM36x_PLL2_PLLM,
		CONFIG_SYS_DM36x_PLL2_PREDIV);

	/* GPIO setup */
	board_gpio_init();

	NS16550_init((NS16550_t)(CONFIG_SYS_NS16550_COM1),
			CONFIG_SYS_NS16550_CLK / 16 / CONFIG_BAUDRATE);

	/*
	 * Fix Power and Emulation Management Register
	 * see sprufh2.pdf page 38 Table 22
	 */
	writel(0x0000e003, (CONFIG_SYS_NS16550_COM1 + 0x30));
	puts("ddr init\n");
	dm365_ddr_setup();

	puts("emif init\n");
	dm365_emif_init();

#if defined(CONFIG_POST)
	/*
	 * Do memory tests, calls arch_memory_failure_handle()
	 * if error detected.
	 */
	memory_post_test(0);
#endif
}