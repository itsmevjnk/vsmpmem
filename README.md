# vsmpmem
`vsmpmem` is a "smart" 8-bit parallel ROM/RAM simulation model for Labcenter Proteus.

This model allows for (EEP)ROM and SRAM chips of arbitrary sizes to be simulated by detecting address pins included in the chip (eg. `A0` to `A14` would indicate a 32Kx8 chip), as well as the existence of the `/WE` (write enable) pin on RAM chips.

## Installation
The model can be compiled using Visual Studio 2022. Compiling for `Win32` is recommended over `x64` as this allows the model to be used on both 32-bit and 64-bit installations of Proteus.

The simulator can theoretically be compiled on Unix with MinGW and CMake; however, CMake builds are currently broken.

## Usage
The `LIBRARY` directory provides a number of 27C EPROM parts (27C16 to 27C080) for reference.

A custom ROM or SRAM part can be created following these pin naming requirements:
* The address pins must be named `An`, with `n` beginning from 0.
* Data pins `D0` through `D7` must be present.
* An active-low chip enable pin named `$CE$` must be present.
* An active-low output enable pin named `$OE$` or `$OE$/VPP` must be present.
* For SRAM chips, an active-low write enable pin named `$WE$` must be present.

The following properties/definitions are **required**:
* `PRIMITIVE` (string): set to `DIGITAL,CHIP`.
* `MODDLL` (string): set to the VSM model file name/path (absolute or relative to the `MODELS` directory in your Proteus installation); eg. `vsmpmem.dll`.

The part may also provide the following properties:
* `FILE` (file name): for providing the ROM image or initial RAM dump to be loaded when simulation starts.
* `BASE` (integer): the base offset of `FILE`, defaults to 0x0000.
* `SHIFT` (integer): the address shift of `FILE` in bits, defaults to 0.
* `INITVAL` (integer): the initial/padding value for the ROM/SRAM, defaults to 0xFF.

The `BASE` and `SHIFT` properties can be used to simulate 16-bit ROM banks; eg. one may use two 8-bit ROM chips with `SHIFT` set to 1 (causing the image's second bytes to be read), and `BASE` set to 0x0000 and 0x0001 (so one chip has all the low bytes and another has all the high bytes).

Please note that EPROM programming is not currently being simulated.

## Contributing
Pull requests and discussions/bug reports through [Issues](https://github.com/itsmevjnk/vsmpmem/issues) are welcome.
