#pragma once
#include <vector>
#include <string_view>

namespace Clocks
{
    enum class ClockSource
    {
        Disabled = 0,
        /** 19.2 MHz oscillator clock. Unlikely to change. */
        Oscillator,
        PLLA = 4,
        /** Main CPU clock (default 1.2 GHz), changes with overclock settings */
        PLLC,
        /** 500 MHz fixed clock. Unlikely to change */
        PLLD,
        /** 216 MHz HDMI auxiliary clock. */
        HDMIAuxiliary
    };

    static constexpr auto maxClockNameSize = 20;
    struct ClockInfo
    {
        char clock_name[maxClockNameSize];
        unsigned long clock_rate;
    };

    class ClockManager
    {
    public:
        ClockManager() = delete;

        static std::vector<ClockInfo> GetClockFrequencies();
        static unsigned long GetClockFrequency(std::string_view clockName);

        static void SetPCMClock(ClockSource source, int integerDiv, int fractDiv);
        static void SetPWMClock(ClockSource source, int integerDiv, int fractDiv);
        static void SetGPIOClock(int index, ClockSource source, int integerdiv, int fractDiv);
    };
};