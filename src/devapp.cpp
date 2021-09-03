#include "clock.hpp"
#include <exception>
#include <iostream>
#include <thread>
#include <chrono>
#include <ilowleveldevices.hpp>

#include "providers/gpio/cdevgpioprovider.hpp"
#include "providers/gpio/dmagpioprovider.hpp"
#include "gpio.hpp"
static std::mutex m;
int main()
{
    using namespace Clocks;
    using namespace Devices::Gpio;
    using namespace Devices::Gpio::Provider;

    Devices::LowLevelDevicesController::enableTrace = true;

    try
    {
        auto controller = GpioProvider::getControllers(DMAGpioProvider::getInstance(), "gpiochip0")[0];
        auto pin12 = controller->open(12);
        pin12->setDriveMode(PinDriveMode::InputPullDown);
//        pin12->enableInterrupt(PinEdge::Falling, [](GpioPin*, PinEdge){
//            std::lock_guard lock(m);
//            std::cout << "Pin12 interrupt" << std::endl;
//        });

        auto pin13 = controller->open(13);
        pin13->setDriveMode(PinDriveMode::InputPullUp);
//        pin13->enableInterrupt(PinEdge::Falling, [](GpioPin*, PinEdge){
//            std::lock_guard lock(m);
//            std::cout << "Pin13 interrupt" << std::endl;
//        });

        while (1)
        {
//            pin13->write(pin13->read() == PinValue::High ? PinValue::Low : PinValue::High);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

    }
    catch (const std::exception & e)
    {
        std::cerr << "[ ERROR ] " << e.what() << std::endl;
    }

	return 0;
}