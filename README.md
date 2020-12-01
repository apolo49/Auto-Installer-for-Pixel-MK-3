# Auto-Installer for Pixel MK 3

This is a _basic_ installer for a Minecraft modpack for me and my friends.
It is very unoptimised and probably quite unstable and prone to injections, currently I haven't tested it on any OS other than Windows 10 and have only compiled it using VS2019.
The program itself, if you are smart enough, would be very easy to generalise as everything is grouped into general functions and there are few hardcoded values.


### TODO:
- ~~Add stb image to load in an icon for the GLFW window.~~
- Generalise using JSON files and _possibly_ upload as an alternative to Technic or FTB, as a native way to install modpacks.
	-  Remove hardcoded progress multiplier in extract zip function.
- Remove all system calls for a more secure and faster execution.
- ~~Test compilation using GCC and other compilers.~~
- ~~Make a CMake generator.~~
- Optimise, Optimise, Optimise.
- Remove unnecessary legacy variables and package more of same-types into vectors or other containers.
- Improve the download progress struct.
- Find a better extraction algorithm.
- Find out more about Java on MacOS ~~and Linux (specifically Debain-based)~~ as documentation is scarce.
- Windows has CreateProcess function, Unix equivalent for Linux __(Possibly posix_spawn())__ and MacOS???
- Make mainloop function smaller
- Test on MacOS.
 
