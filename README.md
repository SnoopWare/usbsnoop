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

Note you must build 32-bit version for 32-bit Windows and 64-bit for 64-bit Windows, respectively. This is due to bundling of drivers for the corresponding architecture within the USBSnoop executable. 32-bit drivers won't work on 64-bit Windows and vise versa.

In order to get the actual code and configure the build rules, run the following commands from the command prompt:

32-bit:

```
git clone https://github.com/dmikushin/usbsnoop.git
cd usbsnoop
git submodule init
git submodule update
mkdir build
cd build
cmake ..
```

64-bit:

```
git clone https://github.com/dmikushin/usbsnoop.git
cd usbsnoop
git submodule init
git submodule update
mkdir build
cd build
cmake -DCMAKE_GENERATOR_PLATFORM=x64 ..
```

Then open the generated Visual Studio solution and build it. During build, password inputs will be prompted for generation and use of SSL certificates. For local use you may choose a simple password, e.g. "x".

The resulting executable with (architecture suffix appended) will be placed into `build\$(Configuration)`, e.g. into `build\Debug`.

## Deployment

The target system must be configured to enable installation of self-signed drivers. Do the following from the Administrator command prompt:

```
Bcdedit.exe -set TESTSIGNING ON
```

The USBSnoop executable is supposed to be always run as Administrator, as it has to deal with system service management. In order to run (e.g. Debug) USBSnoop for within Visual Studio, it also has to be started on behalf of Administrator. 

## LEGAL

This package is provided as is, no warranties are expressed or implied, no liability whatsoever is assumed. If this program burns down your house or puts your fish on fire, it's all your fault.

## Developers & Contributors

 * Tom & Roland of Wingman Team <tom@wingmanteam.com>, <roland@wingmanteam.com>
 * Benoit Papillault <benoit.papillault@free.fr>
 * Graeme W. Gill <graeme@argyllcms.com>

