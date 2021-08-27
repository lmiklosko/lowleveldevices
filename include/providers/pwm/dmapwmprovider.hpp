//
// Created by Lukas Miklosko on 8/16/21.
//
#pragma once
#include "ipwm.hpp"

namespace Devices::Pwm::Provider
{
    class DMAPwmProvider : public IPwmProvider
    {
    public:
        /* virtual */ [[nodiscard]] ControllerProviderList getControllers() const final;
        /* virtual */ [[nodiscard]] ControllerProviderList getControllers(const char*) const final;

        static DMAPwmProvider* getInstance() noexcept;
    };

    class DMAPwmControllerProvider : public IPwmControllerProvider
    {
    public:
        explicit DMAPwmControllerProvider(int id) : id(id)
        {
        }

        /* virtual */ IPwmChannelProvider* open(int channel) override;

        /* virtual */ [[nodiscard]] const char* name() const NOEXCEPT override;
        /* virtual */ [[nodiscard]] int  count() const NOEXCEPT override;

    private:
        int id;
    };

    class DMAPwmChannelProvider : public IPwmChannelProvider
    {
    public:
        DMAPwmChannelProvider(int controller, int channel)
            : controllerID(controller), channelID(channel)
        {
        }

        /* virtual */ void setRange(uint32_t) NOEXCEPT override;
        /* virtual */ [[nodiscard]] uint32_t getRange() const NOEXCEPT override;

        /* virtual */ void setData(uint32_t) NOEXCEPT override;
        /* virtual */ [[nodiscard]] uint32_t getData() const NOEXCEPT override;

        /* virtual */ void setPolarity(Polarity polarity) NOEXCEPT override;
        /* virtual */ [[nodiscard]] Polarity getPolarity() const NOEXCEPT override;

        /* virtual */ void enable(bool en) NOEXCEPT override;
        /* virtual */ [[nodiscard]] bool isRunning() const NOEXCEPT override;

        /* virtual */ [[nodiscard]] int channel() const NOEXCEPT override { return channelID; }

    private:
        int controllerID;
        int channelID;
    };
}

