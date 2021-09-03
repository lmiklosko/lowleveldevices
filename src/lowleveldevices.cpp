#include "lowleveldevices.hpp"
#include "dmagpioprovider.hpp"
#include "dmapwmprovider.hpp"

#include "exceptions.hpp"

using namespace Devices;

/* static */ ILowLevelDevicesAggregateProvider* LowLevelDevicesController::defaultProvider =
    []() noexcept -> DefaultAggregateProvider* {
        try
        {
            static DefaultAggregateProvider defaultProvider;
            return &defaultProvider;
        }
        catch (...)
        {
            return nullptr;
        }
    }();
/* static */ bool LowLevelDevicesController::enableTrace = false;

// ----------------------------------------------------------------------------

std::unique_ptr<Devices::Gpio::Provider::IGpioControllerProvider> DefaultAggregateProvider::GetGpioController() const 
{
    auto dmaControllers = Devices::Gpio::Provider::DMAGpioProvider::getInstance()->getControllers();
    if (!dmaControllers.empty())
    {
        return std::move(dmaControllers[0]);
    }

    throw LLD::no_controller_exception{};
}

std::unique_ptr<Devices::Spi::Provider::ISpiControllerProvider> DefaultAggregateProvider::GetSpiController() const
{
    throw LLD::not_supported_exception{};
}

std::unique_ptr<Devices::I2c::Provider::II2cControllerProvider> DefaultAggregateProvider::GetI2cController() const
{
    throw LLD::not_supported_exception{};
}

std::unique_ptr<Devices::Pwm::Provider::IPwmControllerProvider> DefaultAggregateProvider::GetPwmController() const
{
    auto pwmControllers = Devices::Pwm::Provider::DMAPwmProvider::getInstance()->getControllers();
    if (!pwmControllers.empty())
    {
        return std::move(pwmControllers[0]);
    }

    throw LLD::no_controller_exception{};
}