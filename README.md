*NOTE*: This repository is here only for historical purposes (ie, it hasn't been updated since 2006). It's a clone from [http://sourceforge.net/projects/saturntools](http://sourceforge.net/projects/saturntools).


Ops for linux is a port of the windows program "ops". Both programs
are designed to unlock a CVS Puredigital camcorder and allow people to
download movies from it, upload movies to it, and generally interact
with the camcorder.

Both programs are under the GPL license (see COPYING) and a lot of
this program's usb functionality is borrowed in many ways from Ops for
windows. This is the beauty of using portable libraries like libusb.

This program requires libusb and GTK+ 2.x. This program might work
under windows but as of now you should consider such functionality to be
broken and use regular Ops for windows instead. While libusb and GTK+ 2.x
exist for both systems, GTK+ 2.x imposes some restrictions on threads in
windows which may cause graphical refresh (temporary) problems or they
might the program to lock up during things like file transfer. The file
Makefile.win32 was written for a cross-compiler on linux compiling for
mingw32 windows. If it's useful to you, grand, but don't expect it to
work without any editing.

Ops may compile under a Mac OS X. There's been no test of that yet,
but big endian code which supports the Mac processor has recently been
added. Please get back to us (saturntools-ops-linux@lists.sourceforge.net)
if you get the program working on any platform other than linux on
an i386.


GUI mode
~~~~~~~~
Click "Open Camcorder" then "Unlock" before doing anything else. In
general please don't poke around with bugs in the implementation (ie,
don't upload and download at the same time or you're asking for trouble,
for instance.) Don't unplug your camcorder until the program closes.


Batch-run mode
~~~~~~~~~~~~~~
To download all movies on the camcoder non-interactively, use the "-d" flag.
To erase all movies from the camcorder non-interactively, use the "-f" flag.

Combining these two flags will download, then erase the movies on the
camcorder (regardless of order on the command line.)

Use the "-h" flag for more up-to-date help info.


DISCLAIMER
~~~~~~~~~~
Anything you do with this program is your sole responsibility, even if
there's an obvious bug which fries your camcorder that seems to imply
some kind of incompetence on our part, even if the buttons seem to imply
that when you push them, they'll work the way they say they will.

This camcorder wasn't built to be used like this. This program may
implement functions based on an ignorant understanding of how it works,
leading to a situation where anything may happen. Be careful and use
common sense.

You may send email to the mailing list address a few paragraphs up if
you want to contact the developers. We would be pleased to hear about
successes, failures, good ideas, contributions, anything at all except
spam. (Basically, keep discussion relevant to the program.)

