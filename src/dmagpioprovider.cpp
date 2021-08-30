
#include "dmagpioprovider.hpp"
#include "exceptions.hpp"
#include <map>
#include <vector>
#include <algorithm>
#include <array>
#include <functional>

#include <thread>
#include <chrono>
#include <atomic>

#include "bcm_host.hpp"


using namespace Devices;
using namespace Devices::Gpio;
using namespace Devices::Gpio::Provider;

// --------------------------------------------------------------------------------------

/* static */ DMAGpioPinProvider::poll DMAGpioPinProvider::_poll{};

// --------------------------------------------------------------------------------------

DMAGpioProvider* DMAGpioProvider::getInstance() noexcept
{
	static DMAGpioProvider _provider;
	return &_provider;
}

ControllerProviderList DMAGpioProvider::getControllers() const
{
	ControllerProviderList list;
	try
	{
		list.emplace_back(new DMAGpioControllerProvider());
	}
    catch (std::bad_alloc const&)
    {
        throw;
    }
    catch (...)
    {
    }
	return list;
}

ControllerProviderList DMAGpioProvider::getControllers(std::string const&) const
{
	return getControllers();
}

// --------------------------------------------------------------------------------------

IGpioPinProvider* DMAGpioControllerProvider::open(int pin)
{
	if (pin >= (base() + count()) || pin < base())
	{
		throw std::range_error("\'pin >= (base() + count()) || pin < base()\' condition not met.");
	}

	return new DMAGpioPinProvider(pin);
}

void DMAGpioControllerProvider::setDriveStrength(unsigned bank, PinDriveStrength strength)
{
    auto pm = bcm_pmPerip();
    pm->PADS[bank % 2] = (pm->PADS[bank % 2] & ~(0x7)) | (static_cast<uint8_t>(strength) & 0x7);
}
[[nodiscard]] PinDriveStrength DMAGpioControllerProvider::getDriveStrength(unsigned int bank) const
{
    auto pm = bcm_pmPerip();
    return static_cast<PinDriveStrength>(pm->PADS[bank % 2] & 0x7);
}

void DMAGpioControllerProvider::setHysteresis(unsigned int bank, bool enabled)
{
    auto pm = bcm_pmPerip();
    if (enabled)
    {
        pm->PADS[bank % 2] |= 1 << 3;
    }
    else
    {
        pm->PADS[bank % 2] &= ~(1 << 3);
    }
}
bool DMAGpioControllerProvider::getHysteresis(unsigned int bank) const
{
    auto pm = bcm_pmPerip();
    return (pm->PADS[bank % 2] & (1 << 3)) != 0;
}

int DMAGpioControllerProvider::base() const
{
	return 0;
}

int DMAGpioControllerProvider::count() const
{
	return 53;
}

std::string DMAGpioControllerProvider::name() const
{
	return "Direct memory access General Purpose Input Output controller";
}

// --------------------------------------------------------------------------------------

DMAGpioPinProvider::~DMAGpioPinProvider()
{
    setDriveMode(PinDriveMode::Input);
    enableInterrupt(PinEdge::None, nullptr);
}

PinValue DMAGpioPinProvider::read() const
{
    if (bcm_gpioPerip()->GPLEV[_pinBank] & _pinBit)
    {
        return PinValue::High;
    }
    else
    {
        return PinValue::Low;
    }
}

void DMAGpioPinProvider::write(PinValue val)
{
	if (val == PinValue::High)
	{
		bcm_gpioPerip()->GPSET[_pinBank] = _pinBit;
	}
	else
	{
		bcm_gpioPerip()->GPCLR[_pinBank] = _pinBit;
	}
}
static const std::map<int, std::vector<std::pair<uint8_t,PinDriveMode>>> altMap =
{
    {4, {
        {0, PinDriveMode::Clock}    /* GPCLK0 */
    }},
    {5, {
        {0, PinDriveMode::Clock}    /* GPCLK1 */
    }},
    {5, {
        {0, PinDriveMode::Clock}    /* GPCLK2 */
    }},

    {12, {
        {0, PinDriveMode::Pwm}  /* PWM0_0 */
    }},
    {13, {
        {0, PinDriveMode::Pwm}  /* PWM0_1 */
    }},

    {18, {
        {5, PinDriveMode::Pwm}  /* PWM0_0 */
    }},
    {19, {
        {5, PinDriveMode::Pwm}  /* PWM0_1 */
    }},
    {20, {
        {5, PinDriveMode::Clock}    /* GPCLK0 */
    }},
    {21, {
        {5, PinDriveMode::Clock}    /* GPCLK1 */
    }},

    {32, {
        {0, PinDriveMode::Clock}    /* GPCLK0 */
    }},
    {34, {
        {0, PinDriveMode::Clock}    /* GPCLK0 */
    }},

    {40, {
         {0, PinDriveMode::Pwm}     /* PWM1_0 */
    }},
    {41, {
         {0, PinDriveMode::Pwm}     /* PWM1_1 */
    }},
    {42, {
         {0, PinDriveMode::Clock}   /* GPCLK1 */
    }},
    {43, {
         {0, PinDriveMode::Clock}   /* GPCLK2 */
    }},
    {44, {
         {0, PinDriveMode::Clock}   /* GPCLK1 */
    }},
    {45, {
         {0, PinDriveMode::Pwm}     /* PWM0_1 */
    }},
};
PinDriveMode DMAGpioPinProvider::getDriveMode() const
{
    auto mode = bcm_gpioPerip()->GPFSEL[_pin / 10] & (0x7 << (_pin % 10));
    mode >>= (_pin % 10);

    switch (mode)
    {
    case 0b000:
        return PinDriveMode::Input;

    case 0b001:
        return PinDriveMode::Output;

    default:
        if (altMap.find(_pin) != altMap.end())
        if (auto item = std::find_if(altMap.at(_pin).begin(), altMap.at(_pin).end(),
                                     [mode](auto pair){return pair.first == mode;}); item != altMap.at(_pin).end())
        {
            return item->second;
        }
        return PinDriveMode::AlternateFunction;
    }
}

void DMAGpioPinProvider::setDriveMode(PinDriveMode mode)
{
    constexpr auto setFS = [](int pin, int val){
        const auto bank = pin/10;
        const auto offset = (3 * (pin % 10));

        auto ptr = bcm_gpioPerip();
        ptr->GPFSEL[bank] &= ~(0x7 << offset);
        ptr->GPFSEL[bank] |= val << offset;
    };
    auto setPU = [this](bool up, bool down){
        auto ptr = bcm_gpioPerip();

        ptr->GPPUD = (up ? 0b10 : 0) | (down ? 0b01 : 0);
        std::this_thread::sleep_for(std::chrono::nanoseconds(150));
        ptr->GPPUDCLK[_pinBank] = _pinBit;
        std::this_thread::sleep_for(std::chrono::nanoseconds(150));
        ptr->GPPUDCLK[_pinBank] = _pinBit;
        ptr->GPPUD = 0;
    };
    static constexpr std::array<uint8_t, 6> altBits = {0b100, 0b101, 0b110, 0b111, 0b011, 0b010};

	switch (mode)
	{
	case PinDriveMode::Input:
		setFS(_pin, 0b000);
		setPU(false, false);
		break;

	case PinDriveMode::InputPullUp:
		setFS(_pin, 0b000);
		setPU(true, false);
		break;

	case PinDriveMode::InputPullDown:
	    setFS(_pin, 0b000);
	    setPU(false, true);
		break;

	case PinDriveMode::Output:
		setPU(false, false);
		setFS(_pin, 0b001);
		break;

    case PinDriveMode::Clock:
    case PinDriveMode::Pwm:
        if (altMap.find(_pin) != altMap.end())
        if (auto item = std::find_if(altMap.at(_pin).begin(), altMap.at(_pin).end(),
                                     [mode](auto pair){return pair.second == mode;}); item != altMap.at(_pin).end())
        {
            setFS(_pin, altBits[item->first]);
            break;
        }
    [[fallthrough]];

	default:
		// not supported
		throw LLD::not_supported_exception{};
	}
}

// ------------------------ Interrupt handling -----------------------

void DMAGpioPinProvider::enableInterrupt(PinEdge edge, _Isr fn)
{
    auto ptr = bcm_gpioPerip();
    if (edge == PinEdge::None)
    {
        ptr->GPREN[_pinBank] &= ~_pinBit;
        ptr->GPFEN[_pinBank] &= ~_pinBit;

        /* Detach interrupt */
        auto it = _poll.map.find(pinNumber());
        if (it != _poll.map.end())
        {
            _poll.map.erase(it);
        }

        if (_poll.map.empty())
        {
            _poll.running = false;
            _poll.instance.join();
        }
        return;
    }
    else if (edge == PinEdge::Falling)
    {
        ptr->GPREN[_pinBank] &= ~_pinBit;
        ptr->GPFEN[_pinBank] |= _pinBit;
    }
    else if (edge == PinEdge::Rising)
    {
        ptr->GPREN[_pinBank] |= _pinBit;
        ptr->GPFEN[_pinBank] &= ~_pinBit;
    }
    else
    {
        ptr->GPREN[_pinBank] |= _pinBit;
        ptr->GPFEN[_pinBank] |= _pinBit;
    }

    if (!_poll.instance.joinable() && !_poll.map.empty())
    {
        _poll.map[pinNumber()] = std::move(fn);
        _poll.instance = std::thread([]{
            auto ptr = bcm_gpioPerip();
            while (_poll.running)
            {
                auto evt = ptr->GPEDS[0] + (ptr->GPEDS[1] * 4294967296ll);
                int offset = 0;
                while (evt && _poll.running)
                {
                    if (evt & 1 && _poll.map.find(offset) != _poll.map.end())
                    {
                        std::invoke(
                                _poll.map[offset],
                                (ptr->GPLEV[offset/32] & (offset % 32)) ? PinEdge::Rising : PinEdge::Falling);
                    }

                    ++offset;
                    evt >>= 1;
                }

                std::this_thread::sleep_for(_poll.pollingAccuracy);
            }
        });
    }
    else
    {
        /* Even if entry already exists, we want to override isr */
        _poll.map[pinNumber()] = fn;
    }
}

