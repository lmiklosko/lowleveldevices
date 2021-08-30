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

### <span style="color:orange"> Removed </span>
* ClockManager no longer enumerates all system clocks. Only peripheral
  channels of all PLLs (when applicable) are supported.
