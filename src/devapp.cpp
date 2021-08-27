#include "devices/pwm.hpp"
#include "devices/gpio.hpp"
#include "clock.hpp"
#include <exception>
#include <iostream>

#include "dmagpioprovider.hpp"
using namespace std::chrono_literals;

int main()
{
    using namespace Devices;
    using namespace Devices::Gpio;
    using namespace Devices::Pwm;

    using namespace Clocks;

    try
    {
        ClockManager::SetPWMClock(ClockSource::Oscillator, 2, 0);

        auto gpio = GpioController::getDefault()->open(12);
        gpio->setDriveMode(PinDriveMode::Pwm);
        gpio->write(PinValue::High);

        auto channel = PwmController::getDefault()->open(0);
        channel->setRange(75);
        channel->setData(15);
        channel->enable(true);

        GpioProvider::getControllers(Gpio::Provider::DMAGpioProvider::getInstance());
    }
    catch (const std::exception & e)
    {
        std::cerr << "[ ERROR ] " << e.what() << std::endl;
    }

	return 0;
}