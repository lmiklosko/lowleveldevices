#pragma once
#include <memory>
#include "ipwm.hpp"
#include "ispi.hpp"
#include "ii2c.hpp"
#include "igpio.hpp"

namespace Devices
{

struct ILowLevelDevicesAggregateProvider
{
	[[nodiscard]] virtual std::unique_ptr<Devices::Gpio::Provider::IGpioControllerProvider> GetGpioController() const = 0;
	[[nodiscard]] virtual std::unique_ptr<Devices::Spi::Provider::ISpiControllerProvider> GetSpiController() const = 0;
	[[nodiscard]] virtual std::unique_ptr<Devices::I2c::Provider::II2cControllerProvider> GetI2cController() const = 0;
	[[nodiscard]] virtual std::unique_ptr<Devices::Pwm::Provider::IPwmControllerProvider> GetPwmController() const = 0;
};

struct LowLevelDevicesController
{
	static ILowLevelDevicesAggregateProvider* defaultProvider;
};

}