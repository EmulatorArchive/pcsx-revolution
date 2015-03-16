# Compile #

  * Download latest [Devkitpro](http://wiibrew.org/wiki/DevkitPro) and install it.
  * Download latest zlib library from [portlibs](http://sourceforge.net/projects/devkitpro/files/portlibs/ppc/) and extract it into devkitPro/portlibs/ppc.
    1. **Windows**:
      * Download [TortoiseSVN](http://tortoisesvn.net/downloads).
      * Create folder, right click on it -> TortoiseSVN -> checkout -> http://pcsx-revolution.googlecode.com/svn/trunk/
      * Run Programmer's Notepad (installed with devkitPro)
      * File->Open, find project folder and open Gamecube/Makefile.
      * Tools->make.
    1. **Unix**:
      * Install subversion.
      * svn checkout http://pcsx-revolution.googlecode.com/svn/trunk/ pcsx-revolution-read-only
      * "cd pcsx-revolution-read-only/Gamecube && make" (or "make gc" for GC version). Or you can modify build.sh script for your system.

# Launch #
File organisation:
```
[sd:]&[usb:]
+-apps
|  +-pcsx-r
|     |-boot.dol
|     |-meta.xml
|     |-icon.png

[sd:]&[usb:]
+-pcsx-r
|  +-bios
|     |-scph1001.bin
|  |-memcards
|  |-games

[smb:]
+-pcsx-r
|  |-games
```
  * Put **scph1001.bin** in bios dir.
  * Memory cards will be created automatically on the first launch.
  * Default folder for games is sd:/pcsx-r/games.
  * Supported types: iso, bin, bin+cue, bin+toc, img, mdf.
  * All config files stored in sd:/pcsx-r.

  * Don't forget to configure PAD before launch a game.

# SMB #

SMB config (smb.ini) should be placed in pcsx-r directory.

smb.ini should contain (without "%"):
```
ip = %ip of your share%
user = %username of the share%
pwd = %password%
share = %share name%
```