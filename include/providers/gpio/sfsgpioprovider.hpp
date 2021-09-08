#pragma once

#include "igpio.hpp"
#include "threading/poller.hpp"

namespace Devices::Gpio::Provider {

class SFSGpioProvider final : public IGpioProvider
{
public:
	[[nodiscard]] ControllerProviderList getControllers() const override;
	[[nodiscard]] ControllerProviderList getControllers(std::string const&) const override;

	static SFSGpioProvider* getInstance() noexcept;

private:
    ~SFSGpioProvider() = default;
};	

class SFSGpioControllerProvider final : public IGpioControllerProvider
{
	friend class SFSGpioProvider;
public:
    [[nodiscard]] IGpioPinProvider* open(int) override;

    void setDriveStrength(unsigned bank, PinDriveStrength driveStrength) override;
    [[nodiscard]] PinDriveStrength getDriveStrength(unsigned bank) const override;

    void setHysteresis(unsigned bank, bool enabled) override;
    [[nodiscard]] bool getHysteresis(unsigned bank) const override;

    [[nodiscard]] int base() const override;
    [[nodiscard]] int count() const override;
    [[nodiscard]] std::string name() const override;

private:
	std::string _target;

	explicit SFSGpioControllerProvider(const std::string& _name);
};

class SFSGpioPinProvider final : public IGpioPinProvider
{
	friend class SFSGpioControllerProvider;
public:
	[[nodiscard]] PinValue read() const override;
	void write(PinValue) override;

	[[nodiscard]] PinDriveMode getDriveMode() const override;
	void setDriveMode(PinDriveMode) override;

	void enableInterrupt(PinEdge, Isr) override;
	[[nodiscard]] int pinNumber() const noexcept override { return _pin; }

private:
	int _pin;
	const std::string _fd;
	int _ed{-1};

	explicit SFSGpioPinProvider(int pin);
	~SFSGpioPinProvider();

	static void readEvent(int, const Isr&);
	static Poller<Isr> poller;
};

}
