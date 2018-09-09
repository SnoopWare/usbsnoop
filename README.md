## What is USBSnoop?

USBSnoop is a package designed to log USB packets going from your Windows device driver to your hardware device and vice-versa. The latest version is designed to work on modern 32-bit and 64-bit Windows systems.

USBSnoop is a tool for advanced USB programmers. It allows you to record each URB sent to and received from a USB device. This traces can be saved, loaded, edited, printed and combined into new traces.

## How this is done?

Original version was using a WDM filter driver, which can filter all traffic flowing between the selected USB device driver (ECI USB modem driver for instance) and USBD (which drivers the USB bus). Later versions use the same principles, but work even if the driver above was not passing USB request down to the driver stack. Obviously, it's based on Windows internals and might therefore not be portable.

## Code structure

 * SnoopyPro: the user application
 * USBSnoop: the Kernel driver that captures the URBs, and is installed in a given devices USB driver chain.
 * USBSnpys: Windows 2K WDM driver bridge that accesses the captured URBs for the application.

Code for USBSnpys is in:

```
shared/USBSnoopies_Common.inl
USBSnpys/DriverEntry.cpp
shared/RingBuffer.cpp
```

## Building

Prerequisites:

 * Git for Windows
 * Visual Studio 2017
 * Windows Driver Kit (WDK): https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
 * CMake for Windows

Run the following commands from the command line:

```
git clone https://github.com/dmikushin/usbsnoop.git
cd usbsnoop
git submodule init
git submodule update
mkdir build
cd build
cmake ..
```

Then open the generated Visual Studio solution and build it. The resulting executable will be placed in `build\$(Configuration)`, e.g. in `build\Debug`.

## Deployment

To run it on a 64 bit Vista or Win7 machine, you need to boot while pressing F8, and then continue with the option that allows you to run unsigned drivers. Then run SnoopyPro as administrator. It can be a bit flaky, and can sometimes take multiple attempts to get it all working. Sometimes the order seems to matter (ie. of installing the Snoopy bridge, enabling snooping of the particular device, and actually plugging the device in.) 

## LEGAL

This package is provided as is, no warranties are expressed or implied, no liability whatsoever is assumed. If this program burns down your house or puts your fish on fire, it's all your fault.

## Developers & Contributors

 * Tom & Roland of Wingman Team <tom@wingmanteam.com>, <roland@wingmanteam.com>
 * Benoit Papillault <benoit.papillault@free.fr>
 * Graeme W. Gill <graeme@argyllcms.com>

