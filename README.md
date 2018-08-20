#UPX

This vresion works in CentOS 6.5, we do not provide support to any different platform.

This part of the project contains the code of the different versions of UPx packer.

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
To reduce space I eliminated some compiled files from the stub, so you need to recompile it. 
