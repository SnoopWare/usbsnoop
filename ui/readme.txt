Instruction to compile files in the ui directory
------------------------------------------------------------------------------
14/07/2001 v1.2 Benoit PAPILLAULT <benoit.papillault@free.fr> 
19/03/2001 v1.1 Benoit PAPILLAULT <benoit.papillault@free.fr>

* Contents:

You should file the following files in this source package:

readme.txt        this file
resource.h
sniffusb.cpp
sniffusb.dsp
sniffusb.dsw
sniffusb.h
sniffusb.rc
sniffusbdlg.cpp
sniffusbdlg.h
stdafx.cpp
stdafx.h

* Compile:

You should compile those source files either in "Win32 Debug" mode
or in "Win32 Release" mode.
Don't forget to install "Windows 2000 DDK" available at the microsoft web
site:

	http://www.microsoft.com/ddk/W2kDDK.htm
	
Edit the project settings to point to the include path of your
installation. If you install it under C:\NTDDK as me, you should have:

C++/Preprocessor/additionnal include directories : C:\NTDDK\inc

Do not change the library path.

If your compile is ok, the file "sniffusb.exe" will be available.


