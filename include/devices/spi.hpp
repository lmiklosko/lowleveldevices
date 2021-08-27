#pragma once
#ifndef I2C_HPP
#define I2C_HPP

#include <stdint.h>
#include <memory>
#include <vector>
#include "providers/spi/ispi.hpp"

namespace Devices
{
namespace Spi
{

class SpiDevice
{
public:
	std::size_t read(uint8_t* buffer, std::size_t size);
	std::size_t write(uint8_t* buffer, std::size_t size);
	std::size_t transferFullDuplex(uint8_t* writeBuffer, std::size_t writeLen, uint8_t* readBuffer, std::size_t readLen);

	SpiConnectionSettings getConnectionSettings() const;

private:
	SpiDevice(Devices::Spi::Provider::ISpiDeviceProvider* p) : _provider(std::make_unique(p)) {}
	std::unique_ptr<Devices::Spi::Provider::ISpiDeviceProvider> _provider;
};

class SpiController
{
public:
	SpiDevice* getDevice(SpiConnectionSettings settings);
	
	SpiBusinfo getBusInfo() const;
	std::string busName() const;

private:
	SpiController(Devices::Spi::Provider::ISpiControllerProvider* p) : _provider(std::make_unique(p)) {}
	std::unique_ptr<Devices::Spi::Provider::ISpiControllerProvider> _provider;
};

using ControllerList = std::vector<std::shared_ptr<SpiController>>;
class SpiProvider
{
	static ControllerList getControllers() const;
	static ControllerList getControllers(const std::string& name);
};

}
}

#endif // I2C_HPP