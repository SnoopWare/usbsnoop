
There appear to be three components:

SnoopyPro:	The user application

USBSnoop:	The Kernel driver that captures the URBs,
			and is installed in a given devices USB driver chain.


USBSnpys:
	
	Windows 2K WDM driver bridge that accesses the
	captured URBs for the application.

	Code for USBSnpys is in:

	shared/USBSnoopies_Common.inl
	USBSnpys/DriverEntry.cpp
	shared/RingBuffer.cpp

USBSnpyV:	(Not used - Windows 98 VxD driver bridge)


Changes:

2008/12/13: GWG

	Removed Win98 VxD driver, since DDK isn't available.

	Increased buffer in SnoopyPro/USBLogDoc.cpp GetNewURBSFromSniffer() line 205
	to avoid lockup with big USB packets.

	Bumped version number to 0.23G to label change.

	Fixed formatting slightly to make blank lines sane,
	and better deliniate each packet.

2010:
	V 0.26g

	Added 64 bit driver support. As a result, the DDK needs to be
	used to build 64 bit USBSnoop, before the main application is built.

	To build it:


	Do this by opening a 64 bit DDK checked build envirenment window, and:

		cd USBSnoop
		make


		cd ..\USBSnpys
		make

	Do this by opening a 64 bit DDK free build envirenment window, and:

		cd USBSnoop
		make


		cd ..\USBSnpys
		make

	Open the project file in VC++.

	Set the following active configurations and build them:

		UsbSnoopy - Win32 Debug
		UsbSnoopy - Win32 Release
		USBSnpys - Win32 Debug
		USBSnpys - Win32 Release
		SnoopyPro - Win32 Release


	To run it on a 64 bit Vista or Win7 machine, you need to
	boot while pressing F8, and then continue with the option that
	allows you to run unsigned drivers. Then run SnoopyPro as administrator.
	It can be a bit flaky, and can sometimes take multiple attempts to get
	it all working. Sometimes the order seems to matter (ie. of installing
	the Snoopy bridge, enabling snooping of the particular device, and actually
	plugging the device in.) 

