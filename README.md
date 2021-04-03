# pandOS
pandOS is an educational operating system designed for µMPS architectures.
This project is an implementation of pandOS by Luca Donno, Antonio Lopez, Samuele Marro and Edoardo Merli at University of Bologna.

The documentation is available [here](https://pandos.readthedocs.io/).

## Install

Clone the repository, then run:

```bash
cd pandOS
sudo ./build.sh [phase1|phase2]
```

## How to Use

In order to run this operating system a µMPS emulator is required.
This project is designed with [µMPS3](https://github.com/virtualsquare/umps3)'s emulator in mind.

After installing µMPS3, open the emulator and create a new machine configuration. Click on the gear icon
and set:
* Core file: the path to pandOS/build/kernel.core.umps
* Symbol table: the path to pandOS/build/kernel.stab.umps

Turn on the emulator, open Terminal 0 (Windows->Terminal 0) and start the machine.
