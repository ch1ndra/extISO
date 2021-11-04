# extISO
extISO is a small utility program to extract the contents of an iso file without a need for mounting. It also includes a small C library that provides various functions to read the contents of an ISO9660-formatted file.

# Features
+ Fully compatible with the original ISO9600 specifications
+ Small and portable
+ Includes a C library that can be linked with other C programs
+ Highly readable code
+ Can be used as an IS09660 filesystem driver
+ New features, functionalites and extensions can be easily added

# Limitations
- Newer ISO9660 extensions are not supported (for e.g. UDF)
- Older ISO9660 extensions are partially supported (for e.g. Rock Ridge)

# Requirements
* Requires GCC (and the accompanying toolchain)
* Requires GNU make

# Instructions
* To compile/build the iso9660 library, run:
make iso9660lib

* To compile/build extiso, run:
make extiso

# Usage examples
* ./extiso test.iso gives you a peek into the contents of "test.iso"
* ./extiso test.iso abc.txt extracts the "abc.txt" file from the root directory inside "test.iso"
* ./extiso test.iso xyz/abc.txt extracts the "abc.txt" file from the "xyz" directory inside "test.iso"
* You can specify any number of files that you want extracted from the command line

# That's all folks.
