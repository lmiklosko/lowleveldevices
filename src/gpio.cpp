#include "devices/gpio.hpp"
#include "ilowleveldevices.hpp"
#include "exceptions.hpp"

using namespace Devices;
using namespace Devices::Gpio;
using namespace Devices::Gpio::Provider;

// -------------------------------------- Pin -------------------------------------------

PinValue GpioPin::read() const
{
	return _provider->read();
}
void GpioPin::write(PinValue val)
{
	_provider->write(val);
}

PinDriveMode GpioPin::getDriveMode() const
{
	return _provider->getDriveMode();
}
void GpioPin::setDriveMode(PinDriveMode mode)
{
	_provider->setDriveMode(mode);
}

void GpioPin::enableInterrupt(PinEdge edge, std::function<void(GpioPin*, PinEdge)> callback)
{
	_provider->enableInterrupt(edge, [this, callback](PinEdge edge) {
		callback(this, edge);
	});
}

int GpioPin::pinNumber() const noexcept
{
	return _provider->pinNumber();
}

// ----------------------------------- Controller ---------------------------------------

/* static */ std::map<int, std::weak_ptr<GpioPin>> GpioController::access;

GpioController::~GpioController()
{
	/* remove weak references for expired pins */
	for (auto it = access.begin(); it != access.end(); )
	{
		if (it->second.expired())
		{
			it = access.erase(it);
		}
		else
		{
			++it;
		}
	}
}

std::shared_ptr<GpioPin> GpioController::open(int pin)
{
	auto it = access.find(pin);
	if (it != access.end() && !it->second.expired())
	{
		throw LLD::access_violation_exception{};
	}

	std::shared_ptr<GpioPin> tmp(new GpioPin(_impl->open(pin)));
	access[pin] = tmp;
	return tmp;
}
bool GpioController::tryOpen(int pin, std::shared_ptr<GpioPin>* out) noexcept
{
	try
	{
		*out = open(pin);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

int GpioController::count() const noexcept
{
	return _impl->count();
}
std::string GpioController::name() const
{
	return _impl->name();
}

/* static */ std::shared_ptr<GpioController> GpioController::getDefault()
{
	auto tmp = LowLevelDevicesController::defaultProvider->GetGpioController();
	return std::shared_ptr<GpioController>(new GpioController(std::move(tmp)));
}

// ------------------------------------ Provider ----------------------------------------

/* static */ ControllerList GpioProvider::getControllers(IGpioProvider* p)
{
	auto ictrl = p->getControllers();
	ControllerList out;
	for (auto& i : ictrl)
		out.emplace_back(new GpioController(std::move(i)));
	return out;
}

/* static */ ControllerList GpioProvider::getControllers(IGpioProvider* p, const std::string& name)
{
	auto ictrl = p->getControllers(name);
	ControllerList out;
	for (auto& i : ictrl)
		out.emplace_back(new GpioController(std::move(i)));
	return out;
}