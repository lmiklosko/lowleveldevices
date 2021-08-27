#pragma once
#include <string>

namespace Devices::I2c::Provider{

class II2cDeviceProvider
{
public:
	virtual void read(uint8_t* ptr, std::size_t size) = 0;
	virtual void write(uint8_t* ptr, std::size_t size) = 0;

	virtual std::string deviceId() const = 0;
};

class II2cControllerProvider
{
public:
};

class II2cProvider
{
};

}