Instruction to compile files in the filter directory
------------------------------------------------------------------------------
14/07/2001 v1.2 Benoit PAPILLAULT <benoit.papillault@free.fr> 
19/03/2001 v1.1 Benoit PAPILLAULT <benoit.papillault@free.fr>

* Contents:

You should file the following files in this source package:

Driver.h
DriverEntry.cpp  source code for the filter
readme.txt       this file
RemoveLock.cpp   source code needed for Windows 98
stddcls.cpp
stddcls.h
usbsnoop.dsp     Visual C++ 6.0 Project File
usbsnoop.dsw	 Visual C++ 6.0 Project Workspace

* Compilation for Windows 2000, using Visual C++ 6.0:

You have to select the configuration "Win32 Debug win2k" in "Build" /
"Set Active Configuration" menu.

Don't forget to install "Windows 2000 DDK" available at the microsoft web
site:

	http://www.microsoft.com/ddk/installW2k.asp
	
Edit the project settings to point to the include & library path of your
installation. If you install it under C:\NTDDK (which is the default), you
should have:

C++/Preprocessor/additionnal include directories : C:\NTDDK\inc
Link/Input/Additionnal library path              : C:\NTDDK\libchk\i386

If your compile process is ok, the file "usbsnoop.sys" will be copied to your
C:\WINNT\System32\drivers\

* Compilation for Windows 98, using Visual C++ 6.0:

You have to select the configuration "Win32 Debug win98" in "Build" /
"Set Active Configuration" menu.

Don't forget to install "Windows 98 DDK" available at the microsoft web
site:

	http://www.microsoft.com/ddk/install98ddk.asp
	
Edit the project settings to point to the include & library path of your
installation. If you install it under C:\98DDK (which is the default), you
should have:

C++/Preprocessor/additionnal include directories : C:\98DDK\inc\win98
Link/Input/Additionnal library path              : C:\98DDK\lib\i386\checked

If your compile process is ok, the file "usbsnoop.sys" will be copied to your
C:\WINNT\System32\drivers\

* Note:

You can compiler the Windows 98 version under Windows 200 and the Windows 
2000 version under Windows 98. Just select the appropriate configuration.


