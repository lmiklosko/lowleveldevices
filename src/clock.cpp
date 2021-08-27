#include <cstdint>
#include <thread>
#include <chrono>
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>

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

/*
 * I would really like to investigate how to get these from the registers instead of the linux filesystem
 * although I can't seem to find a relevant documentation regarding PLLs on the internet. Mailbox interface
 * does not provide the complete information either.
 */
std::vector<ClockInfo> Clocks::ClockManager::GetClockFrequencies()
{
    std::vector<ClockInfo> clocks;

    std::ifstream fs("/sys/kernel/debug/clk/clk_summary");
    if (!fs)
    {
        throw LLD::access_exception{};
    }

    /* Skip first 3 lines */
    std::string line;
    for (int i = 0; i < 3; ++i)
    {
        std::getline(fs, line);
    }

    /* Sure, non-standard, but better than plain std::operator>> in my opinion */
    ClockInfo ci{}; int dummy1{}, dummy2{}, dummy3{};
    while (std::getline(fs, line))
    {
        sscanf(line.c_str(), "%s%d%d%d%lu", ci.clock_name, &dummy1, &dummy2, &dummy3, &ci.clock_rate);
        clocks.push_back(ci);
    }

    return clocks;
}

unsigned long Clocks::ClockManager::GetClockFrequency(std::string_view clockName)
{
    auto frequencies = GetClockFrequencies();
    for (auto const& i : frequencies)
    {
        if (clockName == i.clock_name)
        {
            return i.clock_rate;
        }
    }

    throw LLD::not_found_exception{};
}

void Clocks::ClockManager::SetGPIOClock(int index, ClockSource source, int integerDiv, int fractDiv)
{
//    int divi = 19200000 / freq ;
//    int divr = 19200000 % freq ;
//    auto divf = (int)((double)divr * 4096.0 / 19200000.0);

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
