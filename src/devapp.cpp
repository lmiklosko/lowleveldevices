#include "clock.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <ilowleveldevices.hpp>

#include "gpio.hpp"
#include "exceptions.hpp"


int main()
{
    using namespace Clocks;
    using namespace Devices::Gpio;
    using namespace Devices::Gpio::Provider;

    Devices::LowLevelDevicesController::enableTrace = true;

    try
    {
        auto controller = GpioController::getDefault();
        auto pin12 = controller->open(12);
        pin12->setDriveMode(PinDriveMode::Output);

        for (int i = 0; i < 10; )
        {
            pin12->write(pin12->read() == PinValue::High ? PinValue::Low : PinValue::High);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

    }
    catch (const std::exception & e)
    {
        std::cerr << "[ ERROR ] " << e.what() << std::endl;
    }

	return 0;
}