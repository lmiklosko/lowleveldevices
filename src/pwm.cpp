#include <exceptions.hpp>
#include "devices/pwm.hpp"
#include "ilowleveldevices.hpp"

using namespace Devices;
using namespace Devices::Pwm;

void PwmChannel::setRange(uint32_t range) NOEXCEPT
{
	_provider->setRange(range);
}
uint32_t PwmChannel::getRange() const NOEXCEPT
{
	return _provider->getRange();
}

void PwmChannel::setData(uint32_t data) NOEXCEPT
{
	_provider->setData(data);
}
uint32_t PwmChannel::getData() const NOEXCEPT
{
	return _provider->getData();
}

void PwmChannel::setPolarity(Polarity polarity) NOEXCEPT
{
	_provider->setPolarity(polarity);
}
Polarity PwmChannel::getPolarity() const NOEXCEPT
{
	return _provider->getPolarity();
}

void PwmChannel::enable(bool en) NOEXCEPT
{
    _provider->enable(en);
}

bool PwmChannel::isRunning() const NOEXCEPT
{
    return _provider->isRunning();
}

int PwmChannel::channel() const NOEXCEPT
{
    return _provider->channel();
}

// --------------------------------------------------------------------------------------
/* static */ std::map<int, std::weak_ptr<PwmChannel>> PwmController::access{};
std::shared_ptr<PwmChannel> PwmController::open(int channel)
{
    auto it = access.find(channel);
    if (it != access.end() && !it->second.expired())
    {
        throw LLD::access_violation_exception{};
    }

    std::shared_ptr<PwmChannel> tmp(new PwmChannel(_provider->open(channel)));
    access[channel] = tmp;
	return tmp;
}

bool PwmController::tryOpen(int channel, std::shared_ptr<PwmChannel>* out) noexcept
{
	try
	{
		*out = open(channel);
		return true;
	}
	catch (...)
	{
	    *out = nullptr;
		return false;
	}
}

std::string PwmController::name() const
{
	return _provider->name();
}
int PwmController::count() const noexcept
{
	return _provider->count();
}

/* static */ std::shared_ptr<PwmController> PwmController::getDefault()
{
    auto ctrl = LowLevelDevicesController::defaultProvider->GetPwmController();
    return std::shared_ptr<PwmController>{new PwmController(std::move(ctrl))};
}

// ----------------------------------------------------------------------------

/* static */ Devices::Pwm::ControllerList PwmProvider::getControllers(
	Devices::Pwm::Provider::IPwmProvider* p)
{
	auto iCtrl = p->getControllers();
	ControllerList out;
	for (auto& i : iCtrl)
		out.emplace_back(new PwmController(std::move(i)));
	return out;
}

/* static */ Devices::Pwm::ControllerList PwmProvider::getControllers(
	Devices::Pwm::Provider::IPwmProvider* p,
	const std::string& name)
{
	auto iCtrl = p->getControllers(name.c_str());
	ControllerList out;
	for (auto& i : iCtrl)
		out.emplace_back(new PwmController(std::move(i)));
	return out;
}