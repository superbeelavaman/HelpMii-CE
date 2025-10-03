# HelpMii
## The Wii Linux Support Program.
HelpMii was made by Techflash, Tech64, and other contributors.  
This port was made by SuperbeeLavaman.  

This version is approaching a useable point. It is still recommended to use the original [Wii Version](https://github.com/Wii-Linux/Helpmii) if you need assistance with Wii-Linux.  
  
This program is licensed under the terms of the GNU General Public License, version 2.  
You may find these terms in the LICENSE file, which is additionally included in the build files.

### Known Bugs

- `[` is treated as an escape character by default, meaning it will print the default escape code of theta instead
- The guide may contain other typos, although I tried to correct all typos I found in the original guide, which have been merged upstream.

### Building

Get the [CE Toolchain](https://github.com/CE-Programming/toolchain).  
Follow their listed instructions linked by their README to get instructions to set it up.  
Enter the directory where you downloaded this to. Run `make` to compile the program without the guide or `make GUIDE=1` to compile with the guide.  The current estimated sizes are 6.5kB without the guide and 36kB with the guide.  
Use a program like TI-Connect CE or TiLP to copy the program to your calc.  