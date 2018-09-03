VArchC
=====

**ArchC-based Approximate Computing Framework**

VArchC is a framework built to test, validate and evaluate Approximate Computing techniques.

ArchC is a powerful and modern open-source architecture description language designed at University of Campinas by the ArchC team in the Computer Systems Laboratory, Institute of Computing.

License
-------
 - ArchC tools are provided under the GNU GPL license.
   See [Copying](COPYING) file for details on this license.

 - ArchC utility library, i.e. all files stored in the src/aclib
   directory of the ArchC source tree, are provided under the GNU LGPL
   license. See the [Copying Lib](COPYING.LIB) file for details on this license.

Dependencies
------------

1. GNU autotools:
..- m4
..- autoconf
..- automake
..- libtool

2. Build tools:
..- GCC
..- GNU Make

3. GNU Bison

4. GNU Flex

Build and Install
------------
1. Create a directory to store VArchC and access it

2. Build and install SystemC
```bash
wget http://www.accellera.org/images/downloads/standards/systemc/systemc-2.3.1.tgz
tar -xvf systemc-2.3.1.tgz
cd systemc-2.3.1
./configure --prefix=$(pwd)/../SystemC/
make
make install
cd ../
```

3. Build and install ArchC
```bash
git clone https://github.com/VArchC/ArchC.git ArchC-SRC
cd ArchC-SRC
./autogen.sh
./configure --prefix=$(pwd)/../ArchC --with-systemc=$(pwd)/../SystemC/
make
make install
```

4. Setup environment
```bash
source env.sh
```
We recommend to add this line to your ~/.bashrc to set the ArchC environment automatically.
```bash
echo "source env.sh" >> ~/.bashrc
source ~/.bashrc
```
5. ArchC installation is ready. You need a Processor Model to get started.

Processor Models
------------

VArchC needs vanilla-ArchC to be slightly modified for compatibility. Currently the following models are available:

| Model             | Repository                         |
|-------------------|------------------------------------|
| MIPS32r2          | https://github.com/VArchC/MIPS32r2 |

Get a model of your choice to get started.
