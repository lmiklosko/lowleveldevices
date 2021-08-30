#pragma once

#include "providers/gpio/igpio.hpp"
#include <memory>
#include <map>

namespace Devices::Gpio
{

class GpioPin
{
	friend class GpioController;
public:
	[[nodiscard]] PinValue read() const;
	void write(PinValue val);

    [[nodiscard]] PinDriveMode getDriveMode() const;
    void setDriveMode(PinDriveMode mode);

    void enableInterrupt(PinEdge edge, std::function<void(GpioPin*, PinEdge)> callback);
	[[nodiscard]] int pinNumber() const noexcept;

private:
	explicit GpioPin(Devices::Gpio::Provider::IGpioPinProvider* impl) :
		_provider(impl) { }

	std::unique_ptr<Devices::Gpio::Provider::IGpioPinProvider> _provider;
};

class GpioController
{
	friend class GpioProvider;
public:
	/* virtual */ ~GpioController();

    [[nodiscard]] std::shared_ptr<GpioPin> open(int pin);
    [[nodiscard]] bool tryOpen(int pin, std::shared_ptr<GpioPin>* out) noexcept;

    void setDriveStrength(unsigned bank, PinDriveStrength strength);
    [[nodiscard]] PinDriveStrength getDriveStrength(unsigned int bank) const;

    void setHysteresis(unsigned int bank, bool enabled);
    [[nodiscard]] bool getHysteresis(unsigned int bank) const;

    [[nodiscard]] int count() const noexcept;
    [[nodiscard]] std::string name() const;

	static std::shared_ptr<GpioController> getDefault();

private:
	explicit GpioController(std::unique_ptr<Devices::Gpio::Provider::IGpioControllerProvider> impl) :
		_impl(std::move(impl)) {}

	std::unique_ptr<Devices::Gpio::Provider::IGpioControllerProvider> _impl;
	static std::map<int, std::weak_ptr<GpioPin>> access;
};

using ControllerList = std::vector<std::shared_ptr<GpioController>>;
class GpioProvider
{
public:
    [[nodiscard]] static ControllerList getControllers(Devices::Gpio::Provider::IGpioProvider* p);
    [[nodiscard]] static ControllerList getControllers(Devices::Gpio::Provider::IGpioProvider* p, std::string const& name);
};

}