# Golgi Linux extension

Golgi is primarily intended to be run on embedded devices and their associated prototyping boards - e.g. Arduino, MediaTek, Intel Galileo/Edison, Gainspan, etc. However, some customers may wish to run on other platforms. In order to expose this general architecture support Golgi is designed so that its data handling and network layers are logically separate. This allows developers to write network layers for their specific platform to plug into the Golgi data handling layer. The purpose of the Golgi Linux extension is to allow developers to use Golgi on Linux based platforms - e.g. Raspberry Pi (Raspbian), OpenWrt, 32-bit Linux OS, etc. 

# Usage

To use the Golgi Linux extension download the code and include it in your build area. The extension consists of the following files:

- GolgiNetLinux.cpp
- GolgiNetLinux.h
- GolgiWrapper.cpp
- GolgiWrapper.h
- golgi_fixup.cpp
- golgi_fixup.h

You will also need a copy of libgolgi.cpp and libgolgi.h for your application. From the [Golgi Developer Portal](https://devs.golgi.io) select your application and generate and download the "Other" embedded code.

In your main file call GolgiSetup(char *golgi_id, bool useEncryption) to initialise Golgi. Then use the GolgiSelect function as part of a standard select loop. GolgiSelect takes identical parameters to a system select call but manages the process of adding the Golgi file descriptors to the read and write sets as well as processing operations on those descriptors.
