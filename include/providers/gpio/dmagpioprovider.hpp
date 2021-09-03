#pragma once

#include "igpio.hpp"
#include <memory>
#include <atomic>
#include <cstdint>
#include <thread>
#include <map>

namespace Devices::Gpio::Provider
{
class DMAGpioPinProvider;
class DMAGpioControllerProvider final : public IGpioControllerProvider
{
	friend class DMAGpioProvider;
public:
	IGpioPinProvider* open(int) override;

	void setDriveStrength(unsigned bank, PinDriveStrength strength) override;
	[[nodiscard]] PinDriveStrength getDriveStrength(unsigned int bank) const override;

	void setHysteresis(unsigned int bank, bool enabled) override;
	[[nodiscard]] bool getHysteresis(unsigned int bank) const override;

	[[nodiscard]] int base() const override;
	[[nodiscard]] int count() const override;
	[[nodiscard]] std::string name() const override;

private:
	/* virtual */ ~DMAGpioControllerProvider() override = default;
};

class DMAGpioPinProvider final : public IGpioPinProvider
{
	friend class DMAGpioControllerProvider;
public:
	[[nodiscard]] PinValue read() const override;
	void write(PinValue) override;

	[[nodiscard]] PinDriveMode getDriveMode() const override;
	void setDriveMode(PinDriveMode) override;

	void enableInterrupt(PinEdge, Isr) override;
	[[nodiscard]] int pinNumber() const noexcept override { return _pin; }

    ~DMAGpioPinProvider();

private:
	int _pin, _pinBank;
	uint32_t _pinBit;

	explicit DMAGpioPinProvider(int pin) :
		_pin(pin), _pinBank(pin/32), _pinBit(1 << (pin % 32))
	{
	}
};

class DMAGpioProvider final : public IGpioProvider
{
public:
	[[nodiscard]] ControllerProviderList getControllers() const override;
	[[nodiscard]] ControllerProviderList getControllers(std::string const&) const override;

	static DMAGpioProvider* getInstance() noexcept;

private:
	DMAGpioProvider() = default;
	~DMAGpioProvider() override = default;
};
}
