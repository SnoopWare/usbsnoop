==========================================================================
SnoopyPro 0.20 Readme.txt
==========================================================================


==========================================================================
WHAT IS SNOOPYPRO?
==========================================================================

SnoopyPro is a tool for advanced USB programmers. It allows you to record
each URB sent to and received from a USB device. This traces can be saved,
loaded, edited, printed and combined into new traces.

WARNING: You might damage your system with this tool. Don't use it if
you don't know what you're doing!!!! We're not responsible for anything
that happens to you, your system, your devices, your marriage, etc. etc.


==========================================================================
SUPPORTED OPERATING SYSTEMS:
==========================================================================

Tested by the authors on Windows 98, Windows 2000.


==========================================================================
INSTALLATION/USE:
==========================================================================

1. Run SnoopyPro.exe from whereever you have saved it.
2. Open up the USB devices window with F2.
3. Choose 'Unpack Drivers' from the 'File' menu.
4. Choose 'Install Service' from the 'File' menu.
5. Locate the device you want to sniff.
6. Right-click on it and choose 'Install and Restart'.
7. Wait for the magic to happen...


==========================================================================
FAQ:
==========================================================================

Q: Is this a cool piece of software?
A: Yes.


==========================================================================
RECENT CHANGES:
==========================================================================

[X] Added endpoint number as a column in the log.

[X] Added some analyzing log functionality: should figure out better
    which URBs are IN or OUT or INOUT and determines endpoint addresses

[X] Fixed isochronous IN transferbuffer handling: correctly gathers
    the individual buffers with the (variable) length

[X] Now adding the filter in front of the LowerFilters registry
    entry, so it's loaded first.

[X] Timestamping mess: should store plug-in stamp in front
    of trace and UI can either offset or not...
    -> allows comparison of timestamps across traces 

[X] Figure out why the update cmd ui in the devices dialog
    doesn't work... because the updatecmdui gets called on
    command invocation as well... doh

[X] sister device objects on newly arrived devices... wuah!


==========================================================================
TODO/BUGS:
==========================================================================

[ ] Fix About Box - there should be a way to display a nice
    little hand while over the url... (except comctl32.dll
    version 6 -> ICC_LINK_CLASS :-( )

[ ] Implement a retry mechanims in USBSnoop to go out and
    look for USBSnpys on other occasions than just load

[ ] Implement nasty behaviour of stealing the USBD entry
    points and inserting the sniffer filter into chains of
    'impaired' drivers.

[ ] Implement additional column called 'Endpoint' and correctly
    decode In/Out flags of the URBs...

[ ] Don't free the snoop buffer over in the Snpys bridge. Use
    two buffers instead.. might be a bit slower on connect, but
    it's way cleaner.

[ ] Opening log windows after a log has been stopped could yield
    shadow buttons. New log windows shouldn't have the VID/PID
    display -> or it should be grayed, filled in on the first
    paste and checks could be performed on subsequent pastes...
    hmmmm


==========================================================================
CONTACT INFO:
==========================================================================

Download latest version from:
http://home.jps.net/~koma

Send email to:
tom@wingmanteam.com
roland@wingmanteam.com

Send postcards to:
Logitech Inc.
Cool Driver Room
6505 Kaiser Drive
Fremont, CA 94555
USA
