## Overview
Hodor is a project from University of Rochester with the goal of isolating data-plane libraries from the applications that use them and enabling sharing of system services. This repository is an implementation of Hodor-PKU which uses Intel Protection Keys for User-space (PKU) for isolation.

This work was published on ATC'19:

_Hodor: Intra-Process Isolation for High-Throughput Data Plane Libraries_
https://www.usenix.org/conference/atc19/presentation/hedayati-hodor

## Requirements

Hodor-PKU requires processors with PKU feature (tested on Skylake-SP).
