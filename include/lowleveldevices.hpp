#pragma once
#include "ilowleveldevices.hpp"

struct DefaultAggregateProvider final : public Devices::ILowLevelDevicesAggregateProvider
{
	std::unique_ptr<Devices::Gpio::Provider::IGpioControllerProvider> GetGpioController() const;
    std::unique_ptr<Devices::Spi::Provider::ISpiControllerProvider> GetSpiController() const;
    std::unique_ptr<Devices::I2c::Provider::II2cControllerProvider> GetI2cController() const;
    std::unique_ptr<Devices::Pwm::Provider::IPwmControllerProvider> GetPwmController() const;
};
