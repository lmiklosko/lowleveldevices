//
// Created by Lukas Miklosko on 8/16/21.
//

#include "dmapwmprovider.hpp"
#include "bcm_host.hpp"

#include <string>
#include <stdexcept>

using namespace Devices;
using namespace Devices::Pwm;
using namespace Devices::Pwm::Provider;

ControllerProviderList DMAPwmProvider::getControllers() const
{
    ControllerProviderList list;
    for (std::size_t i = 0; i < bcm_numPwmControllers; ++i)
    {
        try
        {
            bcm_pwmPerip(i);
            list.emplace_back(new DMAPwmControllerProvider(i));
        }
        catch (std::bad_alloc const&)
        {
            throw;
        }
        catch (...)
        {
        }
    }
    return list;
}

ControllerProviderList DMAPwmProvider::getControllers(const char *) const
{
    return getControllers();
}

/* static */ DMAPwmProvider * DMAPwmProvider::getInstance() noexcept
{
    static DMAPwmProvider provider;
    return &provider;
}

IPwmChannelProvider* DMAPwmControllerProvider::open(int channel)
{
    if (channel < 0 || channel >= count())
    {
        throw std::range_error("\"channel < 0 || channel >= count()\" with " + std::to_string(channel));
    }

    return new DMAPwmChannelProvider(id, channel);
}

const char* DMAPwmControllerProvider::name() const noexcept
{
    return "DMA PWM Controller";
}

int DMAPwmControllerProvider::count() const noexcept
{
    return 2;
}

void DMAPwmChannelProvider::setRange(uint32_t range) noexcept
{
    bcm_pwmPerip(controllerID)->CHANNEL[channel()].RNG = range;
}
uint32_t DMAPwmChannelProvider::getRange() const noexcept
{
    return bcm_pwmPerip(controllerID)->CHANNEL[channel()].RNG;
}

void DMAPwmChannelProvider::setData(uint32_t data) noexcept
{
    bcm_pwmPerip(controllerID)->CHANNEL[channel()].DAT = data;
}
uint32_t DMAPwmChannelProvider::getData() const noexcept
{
    return bcm_pwmPerip(controllerID)->CHANNEL[channel()].DAT;
}

void DMAPwmChannelProvider::setPolarity(Polarity polarity) noexcept
{
    auto ptr = bcm_pwmPerip(controllerID);
    ptr->CTL = (ptr->CTL & ~(1 << (4 + 8*channel()))) |
            ((static_cast<int>(polarity) & 0x01) << (4 + 8*channel()));
}
Polarity DMAPwmChannelProvider::getPolarity() const noexcept
{
    auto pol = bcm_pwmPerip(controllerID)->CTL & (1 << (4 + 8 * channel()));
    return static_cast<Polarity>(pol);
}

void DMAPwmChannelProvider::enable(bool en) noexcept
{
    auto ptr = bcm_pwmPerip(controllerID);
    if (en)
    {
        ptr->CTL |= (PWM_CTL_PWEN1 | PWM_CTL_MSEN1) << (8 * channel());
    }
    else
    {
        ptr->CTL &= ~((PWM_CTL_PWEN1 | PWM_CTL_MSEN1) << (8 * channel()));
    }
}
[[nodiscard]] bool DMAPwmChannelProvider::isRunning() const noexcept
{
    return (bcm_pwmPerip(controllerID)->STA & (0x200 << channel())) != 0;
}