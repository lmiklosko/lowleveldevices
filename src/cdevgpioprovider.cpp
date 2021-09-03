#include "cdevgpioprovider.hpp"
#include "filesystem.hpp"
#include "exceptions.hpp"
#include "trace.hpp"

#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/gpio.h>

using namespace Devices;
using namespace Devices::Gpio;
using namespace Devices::Gpio::Provider;
namespace fs = std::filesystem;

#define CDEV_BASE_PATH "/dev/"

// ----------------------------------------------------------------------------

ControllerProviderList Devices::Gpio::Provider::CDevGpioProvider::getControllers(std::string const& chip) const
{
	ControllerProviderList list;
	if (fs::exists(CDEV_BASE_PATH))
	{
		for (auto const& p : fs::directory_iterator(CDEV_BASE_PATH))
		{
			if (p.path().filename().string() == chip)
			{
				list.emplace_back(new CDevGpioControllerProvider(p.path().string()));
			}
		}
	}
	return list;
}

ControllerProviderList Devices::Gpio::Provider::CDevGpioProvider::getControllers() const
{
	ControllerProviderList list;
	if (fs::exists(CDEV_BASE_PATH))
	{
		for (auto& p : fs::directory_iterator(CDEV_BASE_PATH))
		{
			if (p.path().filename().string().substr(0, 8) == "gpiochip")
			{
				list.emplace_back(new CDevGpioControllerProvider(p.path().string()));
			}
		}
	}
	return list;
}

CDevGpioProvider* Devices::Gpio::Provider::CDevGpioProvider::getInstance() noexcept
{
	static CDevGpioProvider provider;
	return &provider;
}

// ------------------------------------ CDevGpioControllerProvider -------------------------------------

Devices::Gpio::Provider::CDevGpioControllerProvider::CDevGpioControllerProvider(const std::string& path)
	: _fd(0)
{
	_fd = ::open(path.c_str(), O_RDWR);
	if (_fd == -1)
	{
		throw LLD::access_exception{};
	}

	TRACE("Controller open: ",_fd);
}
CDevGpioControllerProvider::~CDevGpioControllerProvider()
{
    TRACE("Closing controller: ", _fd);
    close(_fd);
}

int Devices::Gpio::Provider::CDevGpioControllerProvider::count() const
{
	struct gpiochip_info info{};
	int rv = ioctl(_fd, GPIO_GET_CHIPINFO_IOCTL, &info);
	if (rv == -1)
	{
		throw LLD::ioctl_exception{ _fd, "GPIO_GET_CHIPINFO_IOCTL" };
	}

	return static_cast<int>(info.lines);
}

std::string Devices::Gpio::Provider::CDevGpioControllerProvider::name() const
{
	struct gpiochip_info info{};
	int rv = ioctl(_fd, GPIO_GET_CHIPINFO_IOCTL, &info);
	if (rv == -1)
	{
		throw LLD::ioctl_exception{ _fd, "GPIO_GET_CHIPINFO_IOCTL" };
	}

	return info.name;
}

IGpioPinProvider* Devices::Gpio::Provider::CDevGpioControllerProvider::open(int pin)
{
    struct gpio_v2_line_info info{};
    info.offset = pin;

    /* Retrieve information about requested pin */
    if (ioctl(_fd, GPIO_V2_GET_LINEINFO_IOCTL, &info) < 0)
    {
        throw LLD::ioctl_exception( _fd, "GPIO_V2_GET_LINEINFO_IOCTL");
    }

    /* Already used by other application */
    if (info.flags & GPIO_V2_LINE_FLAG_USED)
    {
        throw LLD::access_violation_exception{};
    }

	return new CDevGpioPinProvider(_fd, pin);
}

void Devices::Gpio::Provider::CDevGpioControllerProvider::setDriveStrength(unsigned, PinDriveStrength)
{
    throw LLD::not_supported_exception();
}
PinDriveStrength Devices::Gpio::Provider::CDevGpioControllerProvider::getDriveStrength(unsigned) const
{
    throw LLD::not_supported_exception();
}

void Devices::Gpio::Provider::CDevGpioControllerProvider::setHysteresis(unsigned, bool)
{
    throw LLD::not_supported_exception();
}
bool Devices::Gpio::Provider::CDevGpioControllerProvider::getHysteresis(unsigned) const
{
    throw LLD::not_supported_exception();
}

// --------------------------------------------------------------------------------------------------------
/* static */ CDevGpioPinProvider::__event_handler_t CDevGpioPinProvider::__event_handler{};
Devices::Gpio::Provider::CDevGpioPinProvider::CDevGpioPinProvider(int fd, int pin)
	: _pin(pin), _cd(fd), _pd(0)
{
    struct gpio_v2_line_request req{};
    req.offsets[0] = pin;
    strncpy(req.consumer, "lowleveldevices", GPIO_MAX_NAME_SIZE);
    req.config.flags = GPIO_V2_LINE_FLAG_INPUT | GPIO_V2_LINE_FLAG_BIAS_DISABLED;
    req.num_lines = 1;

    if (ioctl(_cd, GPIO_V2_GET_LINE_IOCTL, &req) < 0)
    {
        throw LLD::ioctl_exception(_cd, "GPIO_V2_GET_LINE_IOCTL");
    }

    _pd = req.fd;
}

Devices::Gpio::Provider::CDevGpioPinProvider::~CDevGpioPinProvider()
{
    enableInterrupt(PinEdge::None, nullptr);
    close(_pd);
}

PinValue Devices::Gpio::Provider::CDevGpioPinProvider::read() const
{
    struct gpio_v2_line_values val{};
    val.mask = 1;

    if (ioctl(_pd, GPIO_V2_LINE_GET_VALUES_IOCTL, &val) < 0)
    {
        throw LLD::ioctl_exception(_pd, "GPIO_V2_LINE_GET_VALUES_IOCTL");
    }

    return val.bits & 0b1 ? PinValue::High : PinValue::Low;
}

void Devices::Gpio::Provider::CDevGpioPinProvider::write(PinValue out)
{
    struct gpio_v2_line_values val{};
    val.mask = 1;
    val.bits = (out == PinValue::High ? 1 : 0);

    if (ioctl(_pd, GPIO_V2_LINE_SET_VALUES_IOCTL, &val) < 0)
    {
        throw LLD::ioctl_exception(_pd, "GPIO_V2_LINE_GET_VALUES_IOCTL");
    }
}

PinDriveMode Devices::Gpio::Provider::CDevGpioPinProvider::getDriveMode() const
{
    struct gpio_v2_line_info info{};
    info.offset = _pin;

    /* Retrieve information about requested pin */
    if (ioctl(this->_cd, GPIO_V2_GET_LINEINFO_IOCTL, &info) < 0)
    {
        throw LLD::ioctl_exception( _cd, "GPIO_V2_GET_LINEINFO_IOCTL");
    }

    if (info.flags & GPIO_V2_LINE_FLAG_INPUT)
    {
        if (info.flags & GPIO_V2_LINE_FLAG_BIAS_PULL_UP)
        {
            return PinDriveMode::InputPullUp;
        }
        else if (info.flags & GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN)
        {
            return PinDriveMode::InputPullDown;
        }
        return PinDriveMode::Input;
    }
    else if (info.flags & GPIO_V2_LINE_FLAG_OUTPUT)
    {
        return PinDriveMode::Output;
    }
    else if (info.flags & GPIO_V2_LINE_FLAG_OPEN_DRAIN)
    {
        return PinDriveMode::OpenDrain;
    }
    else if (info.flags & GPIO_V2_LINE_FLAG_OPEN_SOURCE)
    {
        return PinDriveMode::OpenSource;
    }
    throw std::runtime_error("Unknown PinDriveMode configuration");
}

void Devices::Gpio::Provider::CDevGpioPinProvider::setDriveMode(PinDriveMode mode)
{
	struct gpio_v2_line_config cfg{};
	switch (mode)
    {
    case PinDriveMode::OpenSource:
        cfg.flags = GPIO_V2_LINE_FLAG_OPEN_SOURCE | GPIO_V2_LINE_FLAG_BIAS_DISABLED;
        break;

    case PinDriveMode::OpenDrain:
        cfg.flags = GPIO_V2_LINE_FLAG_OPEN_DRAIN | GPIO_V2_LINE_FLAG_BIAS_DISABLED;
        break;

    case PinDriveMode::Output:
        cfg.flags = GPIO_V2_LINE_FLAG_OUTPUT | GPIO_V2_LINE_FLAG_BIAS_DISABLED;
        cfg.attrs[0].mask=1;
        cfg.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES;
        cfg.attrs[0].attr.values = (this->read() == PinValue::High);
        cfg.num_attrs = 1;
        break;

    case PinDriveMode::InputPullUp:
        cfg.flags = GPIO_V2_LINE_FLAG_INPUT | GPIO_V2_LINE_FLAG_BIAS_PULL_UP;
        break;

    case PinDriveMode::InputPullDown:
        cfg.flags = GPIO_V2_LINE_FLAG_INPUT | GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN;
        break;

    case PinDriveMode::Input:
        cfg.flags = GPIO_V2_LINE_FLAG_INPUT | GPIO_V2_LINE_FLAG_BIAS_DISABLED;
        break;

    default:
        throw LLD::not_supported_exception{};
    }

    if (ioctl(_pd, GPIO_V2_LINE_SET_CONFIG_IOCTL, &cfg) < 0)
    {
        throw LLD::ioctl_exception(_pd, "GPIO_V2_LINE_SET_CONFIG_IOCTL");
    }
}

/* static */ std::unique_lock<std::mutex> Devices::Gpio::Provider::CDevGpioPinProvider::notifyWorker()
{
    TRACE("Requesting worker to stop polling");

    ::write(__event_handler.fd[1], "suse\0", 5);
    std::unique_lock lock(__event_handler.mutex);

    return lock;
}

#define ALL(x) x.begin(), x.end()
void Devices::Gpio::Provider::CDevGpioPinProvider::enableInterrupt(PinEdge edge, std::function<void(PinEdge)> callback)
{
    /* Stop watching for an interrupt */
	if (!callback || edge == PinEdge::None)
	{
	    if (auto it = std::find_if(ALL(__event_handler.storage), [fd = this->_pd](struct pollfd i){ return i.fd == fd; });
	        it != __event_handler.storage.end())
        {
	        /* Request worker to stop watching file descriptors */
	        auto lock = notifyWorker();

            /* Poll call returned, we can now modify pollfd structure. First we need to acquire lock to inform other thread
             * about change in pollfd structure. */
            __event_handler.callbacks.erase(it->fd);
            __event_handler.storage.erase(it);
        }

	    /* Give control back to the worker */
	    __event_handler.cv.notify_all();

	    /* If the storage is empty (size == 1 due to the synchronization pipe), the thread will terminate - wait for that to happen */
	    if (__event_handler.storage.size() == 1 && __event_handler.worker.joinable())
        {
	        __event_handler.worker.join();
        }
		return;
	}

	/* Reconfigure pin. */
    struct gpio_v2_line_config cfg{};
    switch (getDriveMode())
    {
        case PinDriveMode::Input:           cfg.flags = GPIO_V2_LINE_FLAG_INPUT; break;
        case PinDriveMode::InputPullDown:   cfg.flags = GPIO_V2_LINE_FLAG_INPUT | GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN; break;
        case PinDriveMode::InputPullUp:     cfg.flags = GPIO_V2_LINE_FLAG_INPUT | GPIO_V2_LINE_FLAG_BIAS_PULL_UP; break;
        default:
            /* TODO: It would be good to throw something like invalid configuration exception in the future */
            throw LLD::lowleveldevices_exception();
    }

    switch (edge)
    {
        case PinEdge::Rising:  cfg.flags |= GPIO_V2_LINE_FLAG_EDGE_RISING;  break;
        case PinEdge::Falling: cfg.flags |= GPIO_V2_LINE_FLAG_EDGE_FALLING; break;
        case PinEdge::Both:    cfg.flags |= GPIO_V2_LINE_FLAG_EDGE_RISING | GPIO_V2_LINE_FLAG_EDGE_FALLING; break;
        default:
            /* We already handled PinEdge::None. No action needed */
            break;
    }

    if (ioctl(_pd, GPIO_V2_LINE_SET_CONFIG_IOCTL, &cfg) < 0)
    {
        throw LLD::ioctl_exception(_pd, "GPIO_V2_LINE_SET_CONFIG_IOCTL");
    }

    /* joinable is supposed to return true if there is a running tread not joined yet - thus we can use it to indicate
     * if the thread has already been spawned */
    if (__event_handler.worker.joinable())
    {
        {
            /* Request worker to stop watching file descriptors */
            auto lock = notifyWorker();

            /* If pollfd entry does not exist yet, add. */
            if (std::find_if(ALL(__event_handler.storage), [fd = _pd](struct pollfd &i) { return i.fd == fd; }) ==
                __event_handler.storage.end()) {
                __event_handler.storage.push_back({_pd, POLLPRI | POLLIN, 0});
            }
            __event_handler.callbacks[_pd] = callback;
        }

        /* Give control back to the worker */
        __event_handler.cv.notify_all();
    }
    else
    {
        pipe(__event_handler.fd);

        /* thread does not exists yet, it is safe to assume that entries doesn't as well.
         * First entry is always the input pipe for synchronization */
        __event_handler.storage.push_back({__event_handler.fd[0], POLLPRI | POLLIN, 0});
        __event_handler.storage.push_back({_pd, POLLPRI | POLLIN, 0});
        __event_handler.callbacks[_pd] = callback;

        __event_handler.worker = std::thread([]{
            std::unique_lock lock(__event_handler.mutex);
            while (true)
            {
                if (int rv = poll(__event_handler.storage.data(), __event_handler.storage.size(), -1); rv > 0)
                {
                    /* Pipe sync requested */
                    if (__event_handler.storage.begin()->revents)
                    {
                        /* clear the buffer */
                        char buff[0x10];
                        ::read(__event_handler.fd[0], buff, sizeof(buff));

                        TRACE("Giving control of storage away...");

                        /* let the main thread take control of __event_handler.storage */
                        __event_handler.cv.wait(lock);

                        if (__event_handler.storage.size() == 1)
                        {
                            /* Nothing to watch for, close pipes and exit */
                            TRACE("Terminating worker thread!");

                            close(__event_handler.fd[0]);
                            close(__event_handler.fd[1]);
                            __event_handler.storage.clear();

                            break;
                        }

                        TRACE("Restarting poll with ", __event_handler.storage.size() - 1, " descriptors");

                        /* Reevaluate the poll in case the descriptor which was ready for read was removed */
                        continue;
                    }

                    for (auto& pfd : __event_handler.storage)
                    {
                        if (pfd.revents)
                        {
                            gpio_v2_line_event event{};
                            while (poll(&pfd, 1, 0) != 0 &&
                                   sizeof(event) == ::read(pfd.fd, &event, sizeof(event)))
                            {
                                std::invoke(__event_handler.callbacks.at(pfd.fd),
                                            event.id == GPIO_V2_LINE_EVENT_FALLING_EDGE ? PinEdge::Falling : PinEdge::Rising);
                            }
                        }
                    }
                }
                else if (errno == EINTR)
                {
                    /* An interrupt came from an external source, continue */
                    continue;
                }
                else
                { TRACE("Invalid POLL call: ", rv, " ", errno); }
            }
        });
    }
}