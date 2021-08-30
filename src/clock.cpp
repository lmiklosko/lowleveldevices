#include <cstdint>
#include <thread>
#include <chrono>

#include "clock.hpp"
#include "bcm_host.hpp"
#include "exceptions.hpp"


using namespace Clocks;
using namespace std::chrono_literals;

static void setClock(volatile uint32_t& ctl, volatile uint32_t& div, int source, int divi, int divf)
{
    /* kill the clock if busy, anything else isn't reliable - pigpio.c */
    if (ctl & CLK_CTL_BUSY)
    {
        do
        {
            ctl = BCM_PASSWORD | CLK_CTL_KILL;
        }
        while (ctl & CLK_CTL_BUSY);
    }

    div = (BCM_PASSWORD | CLK_DIV_DIVI(divi) | CLK_DIV_DIVF(divf));
    std::this_thread::sleep_for(10us);

    ctl = (BCM_PASSWORD | CLK_CTL_SRC(source));
    std::this_thread::sleep_for(10us);

    ctl |= (BCM_PASSWORD | CLK_CTL_ENAB);
}

/* static */ void Clocks::ClockManager::SetPWMClock(ClockSource source, int integerDiv, int fractDiv)
{
    if (integerDiv < 2 || integerDiv > 256)
    {
        throw LLD::invalid_argument_exception("Clocks::ClockManager::SetPWMClock()",
                                              "integerDiv < 2 || integerDiv > 256",
                                              std::to_string(integerDiv));
    }
    if (fractDiv < 0 || fractDiv > ((1 << 12)-1))
    {
        throw LLD::invalid_argument_exception("Clocks::ClockManager::SetPWMClock()",
                                              "fractDiv < 0 || fractDiv > ((1 << 12)-1)",
                                              std::to_string(fractDiv));
    }

    // TODO: Address multiple pwm controllers
    auto pwm0 = bcm_pwmPerip(0);
    auto clk = bcm_clkPerip();

    /* Preserve configuration of the PWM */
    auto cfg = pwm0->CTL;

    setClock(clk->PWMCTL, clk->PWMDIV, static_cast<int>(source), integerDiv, fractDiv);

    /* Restore */
    pwm0->CTL = cfg;
}

/* static */ void Clocks::ClockManager::SetPCMClock(ClockSource source, int integerDiv, int fractDiv)
{
    if (integerDiv < 2 || integerDiv > 256)
    {
        throw LLD::invalid_argument_exception("Clocks::ClockManager::SetPCMClock()",
                                              "integerDiv < 2 || integerDiv > 256",
                                              std::to_string(integerDiv));
    }
    if (fractDiv < 0 || fractDiv > ((1 << 12)-1))
    {
        throw LLD::invalid_argument_exception("Clocks::ClockManager::SetPCMClock()",
                                              "fractDiv < 0 || fractDiv > ((1 << 12)-1)",
                                              std::to_string(fractDiv));
    }

    auto clk = bcm_clkPerip();
    setClock(clk->PCMCTL, clk->PCMDIV, static_cast<int>(source), integerDiv, fractDiv);
}

void Clocks::ClockManager::SetGPIOClock(int index, ClockSource source, int integerDiv, int fractDiv)
{
    if (integerDiv < 2 || integerDiv > 256)
    {
        throw LLD::invalid_argument_exception("Clocks::ClockManager::SetGPIOClock()",
                                              "integerDiv < 2 || integerDiv > 256",
                                              std::to_string(integerDiv));
    }
    if (fractDiv < 0 || fractDiv > ((1 << 12)-1))
    {
        throw LLD::invalid_argument_exception("Clocks::ClockManager::SetGPIOClock()",
                                              "fractDiv < 0 || fractDiv > ((1 << 12)-1)",
                                              std::to_string(fractDiv));
    }

    auto clk = bcm_clkPerip();
    setClock(clk->GP[index].CTL, clk->GP[index].DIV, static_cast<int>(source), integerDiv, fractDiv);
}

unsigned long Clocks::ClockManager::GetClockFrequency(ClockSource clockSource)
{
    /* Each PLL is a fractional N frequency synthesizer that can generate N/M times the crystal oscillator
     * frequency (XOSC). The integer part of N is controlled by the NDIV field of the A2W_PLLx_CTRL register;
     * the fractional part is stored in A2W_PLLx_FRAC. The M refers to the PDIV field of A2W_PLLx_CTRL. */

    /* All SoC clocks are derived from a crystal oscillator (54.0 MHz for RPi4, 19.2 MHz for all other models). */
    static const auto crystalFreq = []{
        if (bcm_getPeripheralAddress() == 0xfe000000) {
            return 54000000.0;
        }
        else {
            return 19200000.0;
        }
    }();

    /* pll enabled bits are scattered across the XOSC_CTRL register */
    static constexpr uint32_t enBit[] = {
            A2W_XOSC_CTRL_PLLAEN,
            A2W_XOSC_CTRL_PLLCEN,
            A2W_XOSC_CTRL_PLLDEN
    };

    switch (clockSource)
    {
        case ClockSource::Disabled:     return 0;
        case ClockSource::Oscillator:   return static_cast<unsigned long>(crystalFreq);
        default:
            break;
    }

    auto a2w = bcm_a2wPerip();
    auto idx = static_cast<int>(clockSource) - 4;

    /* If clock has been disabled, return 0 instead */
    if (!(a2w->XOSC_CTRL & enBit[idx]))
    {
        return 0;
    }

    auto ndiv = (a2w->PLLx_CTRL[idx].val & A2W_PLL_CTRL_NDIV_MASK) >> A2W_PLL_CTRL_NDIV_BIT;
    auto pdiv = (a2w->PLLx_CTRL[idx].val & A2W_PLL_CTRL_PDIV_MASK) >> A2W_PLL_CTRL_PDIV_BIT;
    auto frac = (a2w->PLLx_FRAC[idx].val & A2W_PLL_FRAC_FRAC_MASK) >> A2W_PLL_FRAC_FRAC_BIT;

    auto freq = crystalFreq * ndiv / pdiv;
    freq += frac * crystalFreq / (A2W_PLL_FRAC_FRAC_MASK + 1);

    /* PLLH channel divisor is defined separately as it is not a PER channel but HDMI Aux instead. */
    auto cdiv = ((clockSource == ClockSource::PLLH ? a2w->PLLH_AUX : a2w->PLLx_PER[idx].val) & A2W_PLL_CN_DIV_MASK) >> A2W_PLL_CN_DIV_BIT;
    return static_cast<unsigned long>(freq / cdiv);
}