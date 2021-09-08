#pragma once

#include "igpio.hpp"
#include "threading/poller.hpp"

namespace Devices::Gpio::Provider {

class CDevGpioControllerProvider final : public IGpioControllerProvider
{
	friend class CDevGpioProvider;
public:
    [[nodiscard]] IGpioPinProvider* open(int) override;

    void setDriveStrength(unsigned bank, PinDriveStrength driveStrength) override;
    [[nodiscard]] PinDriveStrength getDriveStrength(unsigned bank) const override;

    void setHysteresis(unsigned bank, bool enabled) override;
    [[nodiscard]] bool getHysteresis(unsigned bank) const override;

    [[nodiscard]] int base() const override { return -1; }
    [[nodiscard]] int count() const override;
    [[nodiscard]] std::string name() const override;

    ~CDevGpioControllerProvider();

private:
	explicit CDevGpioControllerProvider(const std::string& path);


	int _fd;
};

class CDevGpioPinProvider final : public IGpioPinProvider
{
	friend class CDevGpioControllerProvider;
public:
	[[nodiscard]] PinValue read() const override;
	void write(PinValue) override;

	[[nodiscard]] PinDriveMode getDriveMode() const override;
	void setDriveMode(PinDriveMode) override;

	void enableInterrupt(PinEdge, std::function<void(PinEdge)>) override;
	[[nodiscard]] int pinNumber() const noexcept override { return _pin; }

private:
    CDevGpioPinProvider(int fd, int pin);
    ~CDevGpioPinProvider();

    int _pin;
	int _cd, _pd;

	static Poller<Isr> poller;
	static void readEvent(int fd, Isr const& callback);
};

class CDevGpioProvider final : public IGpioProvider
{
public:
	[[nodiscard]] ControllerProviderList getControllers() const override ;
	[[nodiscard]] ControllerProviderList getControllers(std::string const&) const override;

	static CDevGpioProvider* getInstance() noexcept;

private:
	CDevGpioProvider() = default;
};

}
