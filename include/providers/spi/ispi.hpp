#pragma once
#include <vector>
#include <string>

namespace Devices::Spi {
	enum class SpiMode
	{
		SpiMode0 = 0,
		SpiMode1,
		SpiMode2,
		SpiMode3
	};
	struct SpiConnectionSettings
	{
		uint8_t  chipSelect;
		uint32_t clockFrequency;
		uint8_t  dataBitLength;
		SpiMode  mode;

		explicit SpiConnectionSettings(uint8_t chipSelectLine) : chipSelect(chipSelectLine), clockFrequency(0),
			dataBitLength(0), mode(SpiMode::SpiMode0) {}
	};
	struct SpiBusInfo
	{
		uint8_t chipSelectLineCount;
		uint32_t maxClockFrequency;
		uint32_t minClockFrequency;
	};

namespace Provider{

class ISpiDeviceProvider
{
public:
	virtual ~ISpiDeviceProvider() = default;

	virtual std::size_t read(uint8_t* buffer, std::size_t toRead) = 0;
	virtual std::size_t write(uint8_t* buffer, std::size_t toWrite) = 0;
	virtual std::size_t transferFullDuplex(uint8_t* writeBuffer, std::size_t writeBufferLen,
		uint8_t* readBuffer, std::size_t readBufferLen) = 0;

	virtual SpiConnectionSettings getConnectionSettings() const = 0;
};

class ISpiControllerProvider
{
public:
	virtual ~ISpiControllerProvider() {}

	virtual ISpiDeviceProvider* getDevice(SpiConnectionSettings settings) = 0;
	virtual SpiBusInfo getBusInfo() const = 0;
	virtual std::string busName() const = 0;
};

using ControllerProviderList = std::vector<ISpiControllerProvider*>;
class ISpiProvider
{
public:
	virtual ~ISpiProvider() {}

	virtual ControllerProviderList getControllers() const = 0;
	virtual ControllerProviderList getControllers(std::string const&) const = 0;
};

}
}