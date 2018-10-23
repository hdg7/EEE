# EEE

This version works in CentOS 6.5, we do not provide support to any different platform.

In order to compile the modified UPX version, you need to perform the following steps:

First, install UCL compressor from:

http://www.oberhumer.com/opensource/ucl/download/

This part of the project contains the code of the different versions of UPX packer.

To compile it you can just do it, but the stub is independent, to compile the stub you need the build tools.

Add the build tools to the path:
```
PATH=$PATH:/path/to/build/tools/
```
Go to stub folder. I recommend you to comment the win64 amd64 stub in the Makefile. Then, run:

```
make all
```
If you find problems compiling the stub, like Bad ELF interpreter, you need to install glib:
```
sudo yum install glibc
sudo yum install glibc.i686
```
In Debian 9:
```
apt-get install libc6
apt-get install libc6.i386
```
To reduce space I eliminated some compiled files from the stub, so you need to recompile it. 

## Classifiers

You need to set a classifier that EEE will attack. The classifier needs to read the piece of malware and extract its features. We provide a small implementation of structural entropy from Sorokin:

Sorokin, I. (2011). Comparing files using structural entropy. Journal in computer virology, 7(4), 259.

Once you have trained the classifier you need to create variants with EEE and inject them.