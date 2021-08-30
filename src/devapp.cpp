#include "clock.hpp"
#include <exception>
#include <iostream>

int main()
{
    using namespace Devices;
    using namespace Devices::Gpio;
    using namespace Devices::Pwm;

    using namespace Clocks;

    try
    {
        std::cout
            << "PLLA: " << ClockManager::GetClockFrequency(ClockSource::PLLA) << '\n'
            << "PLLC: " << ClockManager::GetClockFrequency(ClockSource::PLLC) << '\n'
            << "PLLD: " << ClockManager::GetClockFrequency(ClockSource::PLLD) << std::endl;
    }
    catch (const std::exception & e)
    {
        std::cerr << "[ ERROR ] " << e.what() << std::endl;
    }

	return 0;
}