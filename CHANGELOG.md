## [0.0.3] - Sep 03 2021
### <span style="color:green">Added</span>
* GPIO Character device support.
  * The official way of manipulating GPIOs on linux
  * Supports Input / Ouput / OpenSource / OpenDrain configurations with Pull Up/Down resistors
  * Event detection running on a separate thread using poll
  * Provides non-shared access across all running processes
  * Implemented using GPIOv2 ABIx  
  
  
* IGpioController interface has been extended
  * Adds support for set/get hysteresis and drive strength
    * Per bank bases.  
      Bank1:  GPIO 0-27  
      Bank2:  GPIO 28-45  
      Bank3:  GPIO 46-57
  * Currently, supported only in DMA Gpio provider  
  

* Traceability
  * New header file trace.hpp has been added as a helper macro to print file,line,threadID,timestamp
    along with custom text.
  * Can be turned on/off (default: off) by setting LowLevelDevicesController::enableTrace flag.

### Changed
* ilowleveldevices.hpp does not include all relevant interface files, instead uses forward declaration.
* Added [[nodiscard]] qualifiers inside igpio.hpp file.
* bcm_host adjusted for new PUP_PDN_CNTRL register

### <span style="color:FireBrick"> Removed </span>
* The Interrupt support in DMA Gpio provider has been removed.   
  AS linux kernel is cleaning the event register inside ISR which means 
  we are not able to access EV register from userspace


## [0.0.2] - Aug 28 2021
### <span style="color:green">Added</span>
* Introducing changelog
* Added a2w to the bcm_host
* CMake now supports pkg file generation

### Changed
* When GPIO pin channel goes out of scope, it is restored
  to the default configuration - input, no-pull, disabled interrupt. 
  Same goes for the PWM channel - disabling it, and setting range to 0.  
  **Note: *This does not affect clock settings***.
* ClockManager::GetClockFrequency now reads PLL clocks directly
  from the registers instead of a clk_summary file.

### <span style="color:FireBrick"> Removed </span>
* ClockManager no longer enumerates all system clocks. Only peripheral
  channels of all PLLs (when applicable) are supported.
  
---
<sup>1</sup>*While technically possible to enumerate all system clocks, there is no point
to support other muxes when working with peripherals.*

## [minor.major.patch] - Mon 03 2021
### <span style="color:green">Added</span>
### Changed
### <span style="color:FireBrick"> Removed </span>
