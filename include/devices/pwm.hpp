#pragma once
#include "providers/pwm/ipwm.hpp"
#include <memory>
#include <string>
#include <map>

namespace Devices::Pwm {

class PwmChannel
{
	friend class PwmController;
public:
	void setRange(uint32_t range) NOEXCEPT;
	[[nodiscard]] uint32_t getRange() const NOEXCEPT;

	void setData(uint32_t data) NOEXCEPT;
    [[nodiscard]] uint32_t getData() const NOEXCEPT;

	void setPolarity(Polarity polarity) NOEXCEPT;
    [[nodiscard]] Polarity getPolarity() const NOEXCEPT;

    void enable(bool enable) NOEXCEPT;
    [[nodiscard]] bool isRunning() const NOEXCEPT;

    [[nodiscard]] int channel() const NOEXCEPT;

private:
	PwmChannel(Devices::Pwm::Provider::IPwmChannelProvider* impl)
	    : _provider(impl)
    {
    }

	std::unique_ptr<Devices::Pwm::Provider::IPwmChannelProvider> _provider;
};

class PwmController
{
	friend class PwmProvider;
public:
	std::shared_ptr<PwmChannel> open(int channel);
	bool tryOpen(int channel, std::shared_ptr<PwmChannel>* out) noexcept;

    [[nodiscard]] std::string name() const;
    [[nodiscard]] int count() const noexcept;

	static std::shared_ptr<PwmController> getDefault();

private:
	explicit PwmController(std::unique_ptr<Devices::Pwm::Provider::IPwmControllerProvider> impl) :
		_provider(std::move(impl)) {}

	std::unique_ptr<Devices::Pwm::Provider::IPwmControllerProvider> _provider;
    static std::map<int, std::weak_ptr<PwmChannel>> access;
};

using ControllerList = std::vector<std::shared_ptr<PwmController>>;
class PwmProvider
{
public:
    [[nodiscard]] static ControllerList getControllers(Devices::Pwm::Provider::IPwmProvider* p);
    [[nodiscard]] static ControllerList getControllers(Devices::Pwm::Provider::IPwmProvider* p, const std::string& name);
};

}