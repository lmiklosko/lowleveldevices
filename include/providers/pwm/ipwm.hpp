#pragma once
#include <cstdint>
#include <vector>
#include <memory>

#define NOEXCEPT noexcept

namespace Devices::Pwm
{

	enum class [[maybe_unused]] Polarity
	{
		ActiveHigh = 0,
		ActiveLow = 1
	};

	namespace Provider
	{

        class IPwmChannelProvider
        {
        public:
            virtual ~IPwmChannelProvider() = default;

            virtual void setRange(uint32_t) NOEXCEPT = 0;
            [[nodiscard]] virtual uint32_t getRange() const NOEXCEPT = 0;

            virtual void setData(uint32_t) NOEXCEPT = 0;
            [[nodiscard]] virtual uint32_t getData() const NOEXCEPT = 0;

            virtual void setPolarity(Polarity polarity) NOEXCEPT = 0;
            [[nodiscard]] virtual Polarity getPolarity() const NOEXCEPT = 0;

            virtual void enable(bool en) NOEXCEPT = 0;
            [[nodiscard]] virtual bool isRunning() const NOEXCEPT = 0;

            [[nodiscard]] virtual int channel() const NOEXCEPT = 0;
        };

        class IPwmControllerProvider
        {
        public:
            virtual ~IPwmControllerProvider() = default;

            virtual IPwmChannelProvider* open(int channel) = 0;

            [[nodiscard]] virtual const char* name() const NOEXCEPT = 0;
            [[nodiscard]] virtual int  count() const NOEXCEPT = 0;
        };

        using ControllerProviderList = std::vector<std::unique_ptr<IPwmControllerProvider>>;
        class IPwmProvider
        {
        public:
            virtual ~IPwmProvider() = default;

            [[nodiscard]] virtual ControllerProviderList getControllers() const = 0;
            [[nodiscard]] virtual ControllerProviderList getControllers(const char*) const = 0;
        };
    }
}