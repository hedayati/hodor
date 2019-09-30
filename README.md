## Overview
Hodor is a project from University of Rochester with the goal of isolating data-plane libraries from the applications that use them and enabling sharing of system services. This repository is an implementation of Hodor-PKU which uses Intel Protection Keys for User-space (PKU) for isolation.

This work was published on ATC'19:

_Hodor: Intra-Process Isolation for High-Throughput Data Plane Libraries_
https://www.usenix.org/conference/atc19/presentation/hedayati-hodor

## Requirements

Hodor-PKU requires processors with PKU feature (tested on Skylake-SP).

## How to Run Hodor-PKU

After cloning the repository, you need to configure, build and boot into the Hodor kernel (if you plan to run inside a VM, `hodor-pku-vm.config` may be good configuration to start from):
```
git clone https://github.com/hedayati/hodor
cd hodor/linux-4.15
mkdir -p ~/builds
make O=~/builds/linux-hodor allyesconfig
cp ./hodor-pku-vm.config ~/builds/linux-hodor/.config
cd ~/builds/linux-hodor
make menuconfig
make -jX bzImage modules
sudo make install modules_install
```

At this step you should be able to build the module, the library and tests:
```
make
```

and, load the module:
```
sudo insmod kern/hodor.ko
```

To ensure everything woks, run `bench_trampoline`:
```
cd tests
sudo LD_LIBRARY_PATH=. ./bench_trampoline
```

Look at `example` and `apps` to see how Hodor can be instructed to load a protected library.
