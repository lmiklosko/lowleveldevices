# Low level devices
A Linux "userspace drivers" for controlling General Purpose Input Output (GPIO)

## Features
* Straightforward C++ interface
* Low level device access
* Selectable providers - You decide, whether you want to use Direct Memory Access for controlling the GPIOs,  
SysFS interface (deprecated), or Character Device (/dev/gpiochip) v1 and v2.
    * By using CDev provider, you can create virtual controllers (using GPIO Aggregator) with different privileges for each process.
    * Interrupts with timestamps using the monotonic clock, without missing a single event!
* Supports GPIO, GPCLK, PWM (and more in the future).
* Detect all available clocks and their respectable frequencies on the system.
* Control drive strength, select PWM/GPCLK clock, dividers and range on your own.
* Enumerate available controllers on the board.
* AND MANY MORE TO COME

## Notes
This is still under development, so please take it with a grain of salt.  
Any comments, changes, suggestions are welcome.

## Examples

### Default GPIO Controller
By using Devices::Gpio::GpioController::getDefault() function, you automatically get the default GPIO controller on the device (the main gpio block on the RPi) using Direct Memory Access provider (direct registers manipulation).

Using a controller, you can open a pin for yourself, set drive mode, drive strength, level and many more.
```
using namespace Devices;
using namespace Devices::Gpio;

try
{
    auto gpio = GpioController::getDefault()->open(12);
    gpio->setDriveMode(PinDriveMode::Output);
    gpio->write(PinValue::High);
}
catch (lld::access_violation_exception const& e)
{
    /* Already opened */
}
```

### Custom provider
There might be cases for which the DMA Provider is not well suited, for example handling lots of interrupts
in a timely manner. For this you'd be better off using Character device provider instead, which uses
new /dev/gpiochipX interface.

```
using namespace Devices;
using namespace Devices::Gpio;
using namespace Devices::Gpio::Provider;

/* Enumerate all available controllers, supporting character device interface */
auto controllers = GpioProvider::getControllers(Gpio::Provider::CDevGpioProvider::getInstance());
if (!controllers.empty())
{
    /* Use the first available - default on RPi */
    auto gpio = controllers[0]->open(12);
    
    ... Use gpio as you would in the first example
}
```

### Running hardware PWM
Set the GPIO as a PWM output. Get the default PWM controller (PWM0 for the RPi) open channel X, set desired parameters and youre ready to go.
```
ClockManager::SetPWMClock(ClockSource::PLLD, 2, 0);

auto gpio = GpioController::getDefault()->open(12);
gpio->setDriveMode(PinDriveMode::Pwm);

auto channel = PwmController::getDefault()->open(0);
channel->setRange(75);
channel->setData(15);
channel->enable(true);
```

## Like what you see?
Consider helping out the project by your contribution comments or suggestions.