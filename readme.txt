06 January 1994  

How to compile:
---------------
  xmkmf
  make depend
  make

For Silicon Graphics: the man page xsnow.z can be moved 
to /usr/catman/u_man/cat1/X11/xsnow.z

VMS
---
This distribution contains a file 'makefile.com' to build Xsnow on
VMS systems, and 'xsnow.hlp'. To build type '@makefile' in the
xsnow directory. Another makefile.com is makefile2.com 

Virtual window managers
-----------------------
For compatibility with virtual window managers Xsnow uses 'vroot.h',
an include file that makes X programs compatible with window managers
like swm and tvtwm. The complete vroot distribution is available from
'ftp.x.org' in the contrib directory as 'vroot.shar.Z'.
Vroot was written by Andreas Stolcke <stolcke@ICSI.Berkeley.EDU>, 9/7/90
and is copyright 1991 by Andreas Stolcke, copyright 1990 by Solbourne 
Computer Inc.

Update history
--------------
 1.22 Really building up snow at bottom of screen
 1.22 A choice of Santa's. Thanks to Holger Veit (Holger.Veit@gmd.de)
      for Santa no. 2. Santa 1 was derived from 2 by scaling down.
 1.23 Aaaargh! gnu C doesn't like the initialization of the new
      Santa pixmaps. Extra braces do the trick, which is so often the case
 1.24 Deleted PaintSnowAtBottom. It's more of a surprise this way.
 1.24 Bug removed with windows off-screen to the left
 1.25 Rudolf's red nose. Silly. Grmbll.
 1.25 Bug with -nokeepsnowonwindows removed
 1.27 Bug with erasing snow on top of windows removed
      Erasing of snow at bottom of screen improved in case of expose event
 1.30 Performance improved. Wind.
 1.31 Make.com and xsnow.hlp for VMS
 1.32 unsigned long snowDelay changed to long, as strtoul is troublesome

Merry X-mas!

Rick Jansen
--
rick@sara.nl
---------------------------------------------------

p.s.
I have kept XSNOW, which I have found in the DECus archives:
https://www.digiater.nl/openvms/decus/
