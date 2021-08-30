#pragma once
#include <vector>
#include <string_view>

namespace Clocks
{
    /**
     * These clocks are peripheral muxes clock sources. This means that PLLx refer to the
     * PER channel of the respective PLL.
     */
    enum class ClockSource
    {
        Disabled = 0,
        Oscillator,
        PLLA = 4,
        PLLC,
        PLLD,
        PLLH
    };

    class ClockManager
    {
    public:
        ClockManager() = delete;

        /** @returns 0 if the clock has been disabled, or actual frequency of a @ref ClockSource */
        static unsigned long GetClockFrequency(ClockSource clockSource);

        static void SetPCMClock(ClockSource source, int integerDiv, int fractDiv);
        static void SetPWMClock(ClockSource source, int integerDiv, int fractDiv);
        static void SetGPIOClock(int index, ClockSource source, int integerdiv, int fractDiv);
    };
};