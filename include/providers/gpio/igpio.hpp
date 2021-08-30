#pragma once

#include <functional>
#include <string>
#include <vector>
#include <memory>

namespace Devices::Gpio {
	enum class PinValue
	{
		Low,
		High
	};
	enum class PinDriveMode
	{
		Input,
		InputPullUp,
		InputPullDown,
		Output,
		OpenDrain,
		OpenSource,
		Pwm,
		Clock,
		AlternateFunction
	};
	enum class PinEdge
	{
		None,
		Rising,
		Falling,
		Both
	};
	enum class PinDriveStrength
    {
        s2mA = 0,
        s4mA,
        s6mA,
        s8mA,
        s10mA,
        s12mA,
        s14mA,
        s16mA
    };

	using _Isr = std::function<void(PinEdge)>;

namespace Provider{

class IGpioPinProvider
{
public:
	virtual ~IGpioPinProvider() {}

	virtual PinValue read() const = 0;
	virtual void write(PinValue) = 0;

	virtual PinDriveMode getDriveMode() const = 0;
	virtual void setDriveMode(PinDriveMode) = 0;

	virtual void enableInterrupt(PinEdge, std::function<void(PinEdge)>) = 0;
	virtual int pinNumber() const noexcept = 0;
};

class IGpioControllerProvider
{
public:
	virtual ~IGpioControllerProvider() {}

	virtual IGpioPinProvider* open(int) = 0;

	virtual void setDriveStrength(unsigned bank, PinDriveStrength driveStrength) = 0;
	virtual PinDriveStrength getDriveStrength(unsigned bank) const = 0;

	virtual void setHysteresis(unsigned bank, bool enabled) = 0;
	virtual bool getHysteresis(unsigned bank) const = 0;

	virtual int base() const = 0;
	virtual int count() const = 0;
	virtual std::string name() const = 0;
};

using ControllerProviderList = std::vector<std::unique_ptr<IGpioControllerProvider>>;
class IGpioProvider
{
public:
	virtual ~IGpioProvider() {}

	virtual ControllerProviderList getControllers() const = 0;
	virtual ControllerProviderList getControllers(std::string const&) const = 0;
};

}
}