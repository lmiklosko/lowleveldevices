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

	void enableInterrupt(PinEdge, _Isr) override;
	[[nodiscard]] int pinNumber() const noexcept override { return _pin; }

	template<typename Rep, typename Period>
	static void setPollingAccuracy(std::chrono::duration<Rep, Period> const& accuracy)
    {
	    _poll.pollingAccuracy = std::min(_poll.pollingAccuracy, accuracy);
    }

private:
	int _pin, _pinBank;
	uint32_t _pinBit;

	explicit DMAGpioPinProvider(int pin) :
		_pin(pin), _pinBank(pin/32), _pinBit(1 << (pin % 32))
	{
	}

    static struct poll {
        std::thread instance{};
        std::map<int, std::function<void(PinEdge)>> map{};
        std::chrono::microseconds pollingAccuracy{1000};
        std::atomic_bool running{true};
    } _poll;
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
