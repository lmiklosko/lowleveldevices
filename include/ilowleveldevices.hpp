#pragma once
#include <memory>

namespace Devices
{
namespace Gpio::Provider{
    class IGpioControllerProvider;
}
namespace Spi::Provider{
    class ISpiControllerProvider;
}
namespace I2c::Provider{
    class II2cControllerProvider;
}
namespace Pwm::Provider{
    class IPwmControllerProvider;
}

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
	static bool enableTrace;
};

}