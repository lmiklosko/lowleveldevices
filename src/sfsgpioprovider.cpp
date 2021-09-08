#include "sfsgpioprovider.hpp"
#include "filesystem.hpp"
#include "exceptions.hpp"

#include <fstream>
#include <fcntl.h>

using namespace Devices;
using namespace Devices::Gpio;
using namespace Devices::Gpio::Provider;

namespace fs = std::filesystem;
using namespace LLD;

/* static */ Poller<Isr> SFSGpioPinProvider::poller(SFSGpioPinProvider::readEvent);

#define SFS_GPIO_PATH "/sys/class/gpio/"
Devices::Gpio::Provider::SFSGpioPinProvider::SFSGpioPinProvider(int pin)
	: _pin(pin), _fd(SFS_GPIO_PATH "gpio" + std::to_string(pin))
{
}

Devices::Gpio::Provider::SFSGpioPinProvider::~SFSGpioPinProvider()
{
    if (_ed != -1)
    {
        enableInterrupt(PinEdge::None, nullptr);
        close(_ed);
    }

	std::ofstream ofs(SFS_GPIO_PATH "unexport");
	ofs << _pin;
}

PinValue Devices::Gpio::Provider::SFSGpioPinProvider::read() const
{
	std::ifstream fs(_fd + "/value");
	int in;

	fs >> in;
	return in ? PinValue::High : PinValue::Low;
}

void Devices::Gpio::Provider::SFSGpioPinProvider::write(PinValue value)
{
	std::ofstream ofs(_fd + "/value");
	ofs << (value == PinValue::High ? 1 : 0);
}

PinDriveMode Devices::Gpio::Provider::SFSGpioPinProvider::getDriveMode() const
{
	std::ifstream ifs(_fd + "/direction");
	std::string dir;

	ifs >> dir;
	return dir == "out" ? PinDriveMode::Output : PinDriveMode::Input;
}

void Devices::Gpio::Provider::SFSGpioPinProvider::setDriveMode(PinDriveMode mode)
{
	if (mode != PinDriveMode::Input && mode != PinDriveMode::Output)
	{
		throw LLD::not_supported_exception();
	}

	std::ofstream ofs(_fd + "/direction");
	ofs << (mode == PinDriveMode::Input ? "in" : "out");
}

void Devices::Gpio::Provider::SFSGpioPinProvider::enableInterrupt(PinEdge edge, std::function<void(PinEdge)> callback)
{
	std::ofstream ofs(_fd + "/edge");
	switch (edge)
	{
    case PinEdge::None:    ofs << "none"; break;
	case PinEdge::Rising:  ofs << "rising"; break;
	case PinEdge::Falling: ofs << "falling"; break;
	case PinEdge::Both:    ofs << "both"; break;
	}

	if (_ed == -1 && edge != PinEdge::None)
    {
        _ed = open((_fd + "/edge").c_str(), O_RDWR);
        if (_ed == -1)
        {
            throw LLD::access_exception();
        }


        poller.addDescriptor(_ed, callback);
    }
	else if (_ed != -1 && (edge == PinEdge::None || !callback))
    {
	    poller.removeDescriptor(_ed);
    }
}

void SFSGpioPinProvider::readEvent(int fd, const Isr& cb)
{
    char buffer;

    lseek(fd, 0, SEEK_SET);
    while (::read(fd, &buffer, 1) < 0) {
        if (errno != EINTR && errno != EIO)
        {
            TRACE("Unexpected error occurred during read (",fd,"): ", errno, " - ", strerror(errno));
            return;
        }
    }

    std::invoke(cb, buffer == 0 ? PinEdge::Falling : PinEdge::Rising);
}

// ---------------------------------- FSF GPIO CONTROLLER PROVIDER -----------------------------------

Devices::Gpio::Provider::SFSGpioControllerProvider::SFSGpioControllerProvider(const std::string& name)
	: _target(name)
{
	if (!fs::exists(name))
	{
		throw not_found_exception {};
	}

	if (!fs::is_directory(name) && !fs::is_symlink(name))
	{
		throw not_supported_exception {};
	}
}

std::string Devices::Gpio::Provider::SFSGpioControllerProvider::name() const
{
	std::ifstream ifs(_target + "/label");
	std::string label;

	ifs >> label;
	return label;
}

int Devices::Gpio::Provider::SFSGpioControllerProvider::count() const
{
	std::ifstream ifs(_target + "/ngpio");
	int ngpio;

	ifs >> ngpio;
	return ngpio;
}

int Devices::Gpio::Provider::SFSGpioControllerProvider::base() const
{
	std::ifstream ifs(_target + "/base");
	int base;

	ifs >> base;
	return base;
}

IGpioPinProvider* Devices::Gpio::Provider::SFSGpioControllerProvider::open(int num)
{
	int base = this->base();
	int ngpio = count();
	
	if (num < base || num > (base + ngpio))
	{
		throw invalid_argument_exception("SFSGpioControllerProvider::open", 
			"num < base || num > (base + ngpio)",
			std::to_string(num));
	}
	
	std::ofstream ofs(SFS_GPIO_PATH "export");
	ofs << num;
	ofs.close();

	if (!fs::exists(SFS_GPIO_PATH "gpio" + std::to_string(num)))
	{
		throw LLD::access_exception{};
	}

	return new SFSGpioPinProvider(num);
}

void Devices::Gpio::Provider::SFSGpioControllerProvider::setDriveStrength(unsigned, PinDriveStrength)
{
    throw LLD::not_supported_exception();
}
PinDriveStrength Devices::Gpio::Provider::SFSGpioControllerProvider::getDriveStrength(unsigned) const
{
    throw LLD::not_supported_exception();
}

void Devices::Gpio::Provider::SFSGpioControllerProvider::setHysteresis(unsigned, bool)
{
    throw LLD::not_supported_exception();
}
bool Devices::Gpio::Provider::SFSGpioControllerProvider::getHysteresis(unsigned) const
{
    throw LLD::not_supported_exception();
}

// -------------------------------------- SFS GPIO PROVIDER ------------------------------------------

ControllerProviderList Devices::Gpio::Provider::SFSGpioProvider::getControllers() const
{
	ControllerProviderList list;
	if (fs::exists(SFS_GPIO_PATH))
	{
		for (auto& p : fs::directory_iterator(SFS_GPIO_PATH))
		{
			if (p.path().filename().string().substr(0, 8) == "gpiochip")
			{
				try 
				{
					list.emplace_back(new SFSGpioControllerProvider(p.path().string()));
				}
				catch (...)
				{ /* Do nothing */ }
			}
		}
	}
	return list;
}

ControllerProviderList Devices::Gpio::Provider::SFSGpioProvider::getControllers(std::string const& chip) const
{
	ControllerProviderList list;
	if (fs::exists(SFS_GPIO_PATH))
	{
		for (auto& p : fs::directory_iterator(SFS_GPIO_PATH))
		{
			if (p.path().filename().string() == chip)
			{
				try 
				{
					list.emplace_back(new SFSGpioControllerProvider(p.path().string()));
				}
				catch (...) { /* do nothing */ }
			}
		}
	}
	return list;
}

SFSGpioProvider* Devices::Gpio::Provider::SFSGpioProvider::getInstance() noexcept
{
	static SFSGpioProvider provider;
	return &provider;
}
