usbsnoop version 1.3
15/07/2001 benoit.papillault@free.fr
http://benoit.papillault.free.fr/speedtouch/sniff-src-1.3.zip

Three modifications have been made :
- added DumpBuffer() method to dump a buffer, with a call to KdPrint() per
    line, instead of per byte
- added a PipeHandle/Endpoint matching table, which is used to display
    endpoint numbers instead of Windows PipeHandle
- corrected display of URB_FUNCTION_SELECT_CONFIGURATION. Analize was wrong
    in case of several interfaces.

usbsnoop version 1.2
14/07/2001 benoit.papillault@free.fr
http://benoit.papillault.free.fr/speedtouch/sniff-2000.html

* what is it?

usbsnoop is a package designed to log USB packets going from your Windows
device drvier to your hardware device and vice-versa. It works both under
Windows NT 2000 and Windows 98 (via compilation #define).

* how this is done?

Previous version was using a WDM filter driver, which can filter all traffic
flowing between the selected USB device driver (ECI USB modem driver for
instance) and USBD (which drivers the USB bus). This version use the same
principles, but work even if the driver above was not passing USB request
down to the driver stack. Obviously, it's based on Windows internals and
might therefore not be portable.

* installation

- copy usbsnoop.sys to your c:\winnt\system32\drivers or
c:\win98\system32\drivers.

* usage

- launch usbsniff.exe (The source included in this package only works for
Windows NT 2000, look at http://www.jps.net/koma/ for binaries for Windows 98)
A list of USB device should displayed, select the one you want to spy and
click on "Install".

- launch dbgview.exe, this tool will let you see all USB traffic going on.
You can later save the window content to a file.

- unplug & replug your hardware device at this stage, and you should see USB 
traffic starting.

------------------------------------------------------------------------------
usbsnoop version 1.1
19/03/2001 benoit.papillault@free.fr
http://benoit.papillault.free.fr/speedtouch/sniff-2000.html

* what is it?

usbsnoop is a package design to log USB packet going from your Windows device
driver to your harware device and vice-versa. It works under Windows NT 2000 
only.

* how this is done?

It is done using a WDM filter driver, which is inserted between USBD and your 
driver.

* installation

- copy usbsnoop.sys to your c:\winnt\system32\drivers directory. This is needed 
in this version as it looks like a convenient location for usbsnoop.sys, but 
this step may be removed from a next release.

- launch dbgview.exe, this tool will let you see all USB traffic going on.

- launch usbsniff.exe, agree to install usbsnoop service. A list of USB device 
should displayed, select the one you want to spy and click on "Install". 

Important: UNPLUG & RE-PLUG your device at this stage, and you should see USB 
traffic starting.

------------------------------------------------------------------------------
SniffUSB - USB packet watcher 0.1
http://www.jps.net/koma/
tom@wingmanteam.com

** What is it?

SniffUSB is a packet watcher for Windows 98. It's a combination
of a kernel mode filter and a UI to catch the watched traffic.

** How does it work?

The kernel mode filter slips in between a USB client driver
and USBD.sys, logging everything that's going on without
touching anything. It's invisible to USB client drivers.

The UI will connect to the filter and retrieve the watched
traffic, allowing further analysis, filtering etc...

** Limitations/known bugs/missing stuff

As of today (2/25/2000), the filter works and can be used.
However, it does *not* collect any data at all; it simply
spits out stuff to the debugger (or a debugging message
logging facility if no debugger is installed). Therefore,
a release build doesn't do anything at all, and included
in this package is a debug build.

The UI does not attempt to connect to the filter yet. It
is useful for installing/removing the filter only.

To catch the debug output, a debugger like SoftIce is needed.
If this is not available, any debugging message hooker can
be used. Included in the package is dbgview, made by Mark
Russinovich at http://www.sysinternals.com. The filter
can produce *lots* of debug output at times, which might
make the debug viewer appear to be locked up. If it doesn't
react anymore, give it some time (possibly up to a few minutes).
If you're fed up, just kill it.

** How do I use it?!?

Here's what you have been waiting for - the "install":

-- One-time installation
1. Make sure you're running Windows 98 or 98 Second Edition
   (won't work properly on Win2K - yet)
2. Copy the file "UsbSnoop.sys" from the filter\Debug directory
   to your <windir>\System32\Drivers directory
3. Copy ui\Release\SniffUSB.exe to a convenient location (desktop?)
4. Copy the entire dbgview\ directory to a convenient location

-- Device setup
1. Make sure you have your device plugged in at least once.
   To ensure consistent results, make sure you won't plug it into
   another port during the logging sessions (the filter is registered
   on a per-port basis, and any new port will not be using the filter)
2. Run dbgview.exe. You will get a window which shows all debug
   output of everything that produces debug output.
3. Run SniffUSB.exe. You will get a list of USB devices with
   their description and a note if the filter is installed or not.
4. Right-click on the entry you want to sniff, select "Install".
   This will register the filter for this device on all ports
   that have been ever used so far.
5. At this point, the device needs to be unplugged/re-plugged;
   as filters get loaded as part of the PnP AddDevice process, which
   only takes place during plug-in. If you dare, you can right-click
   the entry, and select "Replug" - this might crash your machine.
   You can also just unplug/replug it. You should start seeing debug
   output on the viewer.
6. Analyze what you see ;-)

-- Filter uninstall
1. Run SniffUSB.exe.
2. Right-click entry which you want the filter removed for, select
   "Uninstall". That's it.

Comments, suggestions, bug fixes, to roland@wingmanteam.com (mostly
UI related stuff), or/and tom@wingmanteam.com (mostly filter related
stuff).
Flames to /dev/null (or, on an MS platform, NUL:)

------- Because we live in such a strange world:

As usual, the legal stuff - this package is provided as is, no warranties
are expressed or implied, no liability whatsoever is assumed, if this
program burns down your house of puts your fish on fire, it's all your
fault.

Simply put - you're on your own. M'kay?
