//
// Created by Lukas Miklosko on 8/16/21.
//

#ifndef LIGHTNING_BCM_HOST_HPP
#define LIGHTNING_BCM_HOST_HPP

#include <cstdio>
#include <cstdint>

#define _C
#define _R
#define _W
#define _RW
#define _RS _C

#define BCM_PASSWORD 0x5A000000

/**
 *	Direct memory access channel register map
 *
 */
struct dma_channel_t
{
    _RW uint32_t controlStatus;
    _RW uint32_t controlBlockAddress;
    _RW uint32_t transferInformation;
    _RW uint32_t sourceAddress;
    _RW uint32_t destinationAddress;
    _RW uint32_t transferLength;
    _RW uint32_t d2Stride;
    _RW uint32_t nextCBAddress;
    _RW uint32_t debug;
    _RS  uint32_t RESERVED_0[55];
};
static_assert(sizeof(dma_channel_t) == 0x100);

/**
 *	Direct memory access controller block
 *
 *	@address: 0x00007000
 *	@for: Extended PWM
 *
 */
struct __attribute__((packed)) dma_base_t
{
    _RW dma_channel_t channel[15];
    _RS  uint32_t RESERVED_0[56];
    _RW uint32_t INT_STATUS;
    _RS  uint32_t RESERVED_4[3];
    _RW uint32_t ENABLE;
};

/**
 *	Power management block
 *
 *	@address:	0x00100000
 *	@for: GPIO PADs
 */
struct power_management_t
{
    _RW uint32_t GNRIC;
    _RW uint32_t AUDIO;
    _RS uint32_t UNUSED0[4];
    _RW uint32_t STATUS;
    _RW uint32_t RSTC;
    _RW uint32_t RSTS; // 20
    _RW uint32_t WDOG;
    _RW uint32_t PADS0; // 28
    _RW uint32_t PADS[5];   // 2c
    _RS uint32_t UNUSED1[1];
    _RW uint32_t CAM[2];
    _RW uint32_t CCP2TX;
    _RW uint32_t DSI[2];
    _RW uint32_t HDMI;
    _RW uint32_t USB;
    _RW uint32_t PXLDO;
    _RW uint32_t PXBG;
    _RW uint32_t DFT;
    _RW uint32_t SMPS;
    _RW uint32_t XOSC;
    _RW uint32_t SPAREW;
    _C  uint32_t SPARER;
    _RW uint32_t AVS_RSTDR;
    _RW uint32_t AVS_STAT;
    _RW uint32_t AVS_EVENT;
    _RW uint32_t AVS_INTEN;
    _RS  uint8_t UNUSED2[0x70];
    _RW uint32_t DUMMY;
    _RS  uint8_t UNUSED3[0x8];
    _RW uint32_t IMAGE;
    _RW uint32_t GRAFX;
    _RW uint32_t PROC;
};

/**
 *	Clock manager block
 *
 *	@address: 0x00101000
 *	@for: PWM, GPCLK
 *
 */
struct clock_management_t
{
    _RW uint32_t GNRICCTL;
    _RW uint32_t GNRICDIV;
    _RW uint32_t VPUCTL;
    _RW uint32_t VPUDIV;
    _RW uint32_t SYSCTL;
    _R  uint32_t SYSDIV;
    _RW uint32_t PERIACTL;
    _R  uint32_t PERIADIV;
    _RW uint32_t PERIICTL;
    _R  uint32_t PERIIDIV;
    _RW uint32_t H264CTL;
    _RW uint32_t H264DIV;
    _RW uint32_t ISPCTL;
    _RW uint32_t ISPDIV;
    _RW uint32_t V3DCTL;
    _RW uint32_t V3DDIV;
    _RW uint32_t CAM0CTL;
    _RW uint32_t CAM0DIV;
    _RW uint32_t CAM1CTL;
    _RW uint32_t CAM1DIV;
    _RW uint32_t CCP2CTL;
    _R  uint32_t CCP2DIV;
    _RW uint32_t DSI0ECTL;
    _RW uint32_t DSI0EDIV;
    _RW uint32_t DSI0PCTL;
    _R  uint32_t DSI0PDIV;
    _RW uint32_t DPICTL;
    _RW uint32_t DPIDIV;    // 6C
    struct {
        _RW uint32_t CTL;
        _RW uint32_t DIV;
    } GP[3];
    _RW uint32_t HSMCTL;
    _RW uint32_t HSMDIV;
    _RW uint32_t OTPCTL;
    _RW uint32_t OTPDIV;
    _RW uint32_t PCMCTL;
    _RW uint32_t PCMDIV;
    _RW uint32_t PWMCTL;
    _RW uint32_t PWMDIV;    // a4
    _RW uint32_t SLIMCTL;
    _RW uint32_t SLIMDIV;
    _RW uint32_t SMICTL;
    _RW uint32_t SMIDIV;    // b4
    _RS uint32_t UNUSED0[2];
    _RW uint32_t TCNTCTL;
    _RW uint32_t TCNTCNT;
    _RW uint32_t TECCTL;
    _RW uint32_t TECDIV;
    _RW uint32_t TD0CTL;
    _RW uint32_t TD0DIV;
    _RW uint32_t TD1CTL;
    _RW uint32_t TD1DIV;
    _RW uint32_t TSENSCTL;
    _RW uint32_t TSENSDIV;
    _RW uint32_t TIMERCTL;
    _RW uint32_t TIMERDIV;
    _RW uint32_t UARTCTL;   // f0
    _RW uint32_t UARTDIV;
    _RW uint32_t VECCTL;
    _RW uint32_t VECDIV;
    _RW uint32_t OSCCOUNT;  // 100
    _RW uint32_t PLLA;
    _RW uint32_t PLLC;
    _RW uint32_t PLLD;
    _RW uint32_t PLLH;
    _RW uint32_t LOCK;
    _RW uint32_t EVENT;
    _RW uint32_t INTEN;
    _RW uint32_t DSI0HSCK;
    _RW uint32_t CKSM;
    _RW uint32_t OSCFREQI;
    _RW uint32_t OSCFREQF;
    _RW uint32_t PLLTCTL;   // 130
    _RW uint32_t PLLTCNT0;
    _RW uint32_t PLLTCNT1;
    _RW uint32_t PLLTCNT2;
    _RW uint32_t PLLTCNT3;
    _RW uint32_t TDCLKEN;
    _RW uint32_t BURSTCTL;
    _RW uint32_t BURSTCNT;  // 14c
    _RS uint32_t UNUSED1[2];
    _RW uint32_t DSI1ECTL;
    _RW uint32_t DSI1EDIV;
    _RW uint32_t DSI1PCTL;  // 160
    _R  uint32_t DSI1PDIV;
    _RW uint32_t DFTCTL;
    _RW uint32_t DFTDIV;
    _RW uint32_t PLLB;      // 170
    _RS uint32_t UNUSED2[7];
    _RW uint32_t PULSECTL;  // 190
    _RW uint32_t PULSEDIV;
    _RS uint32_t UNUSED3[4];
    _RW uint32_t SDCCTL;    // 1a8
    _RW uint32_t SDCDIV;
    _RW uint32_t ARMCTL;    // 1b0
    _R  uint32_t ARMDIV;
    _RW uint32_t AVEOCTL;
    _RW uint32_t AVEODIV;
    _RW uint32_t EMMCCTL;
    _RW uint32_t EMMCDIV;   // 1d4
};

struct pll_ctl {
    uint32_t val;
    uint32_t RESERVED[7];
};
struct pll_frac {
    uint32_t val;
    uint32_t RESERVED[7];
};
struct pll_per {
    uint32_t val;
    uint32_t RESERVED[7];
};
/**
 *  A2W Block.
 *
 *  Offsets taken from RPi Linux Kernel:
 *      https://github.com/raspberrypi/linux/blob/7fb9d006d3ff3baf2e205e0c85c4e4fd0a64fcd0/drivers/clk/bcm/clk-bcm2835.c
 *      https://elinux.org/BCM2835_registers#A2W
 *  Failed to find relevant datasheet.
 *
 *  @address 0x00102000
 *  @for: pll clocks
 *  @note There are some defined registers inside RESERVED blocks, but we don't use them or we failed to find the
 *  register description - for sake of simplicity we didn't defined them here.
 */
struct a2w_base_t
{
    _C  uint32_t RESERVED0[0x40];
    _RW pll_ctl PLLx_CTRL[4];
    _C  uint32_t RESERVED1[4];
    _RW uint32_t XOSC_CTRL;
    _C  uint32_t RESERVED2[0x13];
    _RW pll_ctl PLLB_CTRL;
    _RW pll_frac PLLx_FRAC[4];
    _RW uint32_t RESERVED3[0x17];
    _RW uint32_t PLLB_FRAC;
    _C  uint32_t RESERVED4[0x17];
    _RW uint32_t PLLH_AUX;
    _C  uint32_t RESERVED5[0x70];
    _RW pll_per PLLx_PER[3];
};

/**
 *	General Purpose Input Output controller block
 *
 *	@address 0x00200000
 *	@for: gpio, pwm, i2c, spi, onewire
 *
 */
struct gpio_base_t
{
    _RW uint32_t GPFSEL[6];
    _RS uint32_t RESERVED_0;
    _W  uint32_t GPSET[2];
    _RS uint32_t RESERVED_1;
    _W	uint32_t GPCLR[2];
    _RS uint32_t RESERVED_2;
    _R  uint32_t GPLEV[2];
    _RS uint32_t RESERVED_3;    // 3c
    _RW uint32_t GPEDS[2];
    _RS uint32_t RESERVED_4;
    _RW	uint32_t GPREN[2];
    _RS uint32_t RESERVED_5;
    _RW uint32_t GPFEN[2];
    _RS uint32_t RESERVED_6;    // 60
    _RW uint32_t GPHEN[2];
    _RS uint32_t RESERVED_7;
    _RW uint32_t GPLEN[2];
    _RS uint32_t RESERVED_8;
    _RW uint32_t GPAREN[2];
    _RS uint32_t RESERVED_9;
    _RW uint32_t GPAFEN[2];
    _RS uint32_t RESERVED_10;
    _RW uint32_t GPPUD;
    _RW uint32_t GPPUDCLK[2];
};

/**
 *	Pulse Code Modulation controller block
 *
 *	@address 0x7e203000
 *	@for dmapwm
 *
 */
struct pcm_base_t
{
    _RW uint32_t CS_A;
    _RW uint32_t FIFO_A;
    _RW uint32_t MODE_A;
    _RW uint32_t RXC_A;
    _RW uint32_t TXC_A;
    _RW uint32_t DREQ_A;
    _RW uint32_t INTEN_A;
    _RW uint32_t INTSTC_A;
    _RW uint32_t GRAY;
};

/**
 *	Pulse width modulation block
 *
 *	@address 0x0020C000
 *	@for pwm
 *
 */
struct pwm_base_t
{
    _RW uint32_t CTL;
    _RW uint32_t STA;
    _RW uint32_t DMAC;

    _RS uint32_t RESERVED_4;

    volatile struct pwm_channel_t {
        _RW uint32_t RNG;
        _RW uint32_t DAT;
        _RW uint32_t FIF;
        _RS uint32_t RESERVED;
    } CHANNEL[2];
};

/* Check random addresses */
static_assert(offsetof(clock_management_t, PWMDIV) == 0xa4);
static_assert(offsetof(clock_management_t, PLLA) == 0x104);
static_assert(offsetof(clock_management_t, PLLB) == 0x170);

static_assert(offsetof(gpio_base_t, RESERVED_3) == 0x3c);
static_assert(offsetof(gpio_base_t, RESERVED_6) == 0x60);

static_assert(offsetof(pwm_base_t, CTL) == 0x0);
static_assert(offsetof(pwm_base_t, CHANNEL[0].RNG) == 0x10);
static_assert(offsetof(pwm_base_t, CHANNEL[1].RNG) == 0x20);


static_assert(offsetof(a2w_base_t, PLLx_CTRL) == 0x100);
static_assert(offsetof(a2w_base_t, XOSC_CTRL) == 0x190);
static_assert(offsetof(a2w_base_t, PLLB_CTRL) == 0x1e0);
static_assert(offsetof(a2w_base_t, PLLx_FRAC) == 0x200);
static_assert(offsetof(a2w_base_t, PLLx_PER) == 0x500);

#define CLK_CTL_BUSY    (1 <<7)
#define CLK_CTL_KILL    (1 <<5)
#define CLK_CTL_ENAB    (1 <<4)
#define CLK_CTL_SRC(x) ((x)<<0)

#define CLK_DIV_DIVI(x) ((x)<<12)
#define CLK_DIV_DIVF(x) ((x)<< 0)

#define A2W_PLL_CTRL_NDIV_BIT  0
#define A2W_PLL_CTRL_NDIV_MASK 0x3ff
#define A2W_PLL_CTRL_PDIV_BIT 12
#define A2W_PLL_CTRL_PDIV_MASK 0x7000

#define A2W_PLL_FRAC_FRAC_BIT 0
#define A2W_PLL_FRAC_FRAC_MASK 0x000fffff

#define A2W_PLL_CN_DIV_BIT 0
#define A2W_PLL_CN_DIV_MASK 0x000000ff

#define A2W_XOSC_CTRL_PLLAEN 0x00000040
#define A2W_XOSC_CTRL_PLLBEN 0x00000080
#define A2W_XOSC_CTRL_PLLCEN 0x00000001
#define A2W_XOSC_CTRL_PLLDEN 0x00000020

#define PWM_CTL_MSEN1 (1<<7)
#define PWM_CTL_PWEN1 (1<<0)

unsigned bcm_getPeripheralAddress();
unsigned bcm_getPeripheralSize();

static constexpr std::size_t bcm_numPwmControllers = 1;

[[maybe_unused]] volatile dma_base_t* bcm_dmaPerip();
[[maybe_unused]] volatile power_management_t* bcm_pmPerip();
[[maybe_unused]] volatile clock_management_t* bcm_clkPerip();
[[maybe_unused]] volatile gpio_base_t* bcm_gpioPerip();
[[maybe_unused]] volatile pcm_base_t* bcm_pcmPerip();
[[maybe_unused]] volatile pwm_base_t* bcm_pwmPerip(std::size_t idx);
[[maybe_unused]] volatile a2w_base_t* bcm_a2wPerip();


#endif //LIGHTNING_BCM_HOST_HPP
