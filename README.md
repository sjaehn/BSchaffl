# B.Schaffl

Pattern-controlled MIDI amp & time stretch plugin to produce shuffle / swing effects. (Experimental)

Description: TODO

**Warning: Experimental software. Not intended for production use, yet. No guarantees.**


## Installation

Build your own binaries in the following three steps.

Step 1: [Download the latest published version](https://github.com/sjaehn/BSchaffl/releases) of B.Schaffl. Or clone or
[download the master](https://github.com/sjaehn/BSchaffl/archive/master.zip) of this repository.

Step 2: Install pkg-config and the development packages for x11, cairo, and lv2 if not done yet. On
Debian-based systems you may run:
```
sudo apt-get install pkg-config libx11-dev libcairo2-dev lv2-dev
```

Step 3: Building and installing into the default lv2 directory (/usr/lib/lv2/) is easy. Simply call:

```
make
sudo make install
```

from the directory where you downloaded the repository files. For installation into an
alternative directory (e.g., /usr/local/lib/lv2), change the variable `PREFIX` while installing:

```
sudo make install PREFIX=/usr/local
```


## Running

After the installation Ardour, Carla, and any other LV2 host should automatically detect B.Schaffl.

If jalv is installed, you can also call it using one of the supported jalv flavours (jalv.gtk,
jalv.gtk3, jalv.qt4, jalv.qt5), like

```
jalv.gtk https://www.jahnichen.de/plugins/lv2/BSchaffl
```

to run it (pseudo) stand-alone and connect it to the JACK system.

Note: **Jack transport is required to get information about beat / position in the beat(s) or bar(s)
mode!**


## Usage

TODO


## TODO's

* Show warning if latency exceeds maximum
* Option to amplify NOTE_OFF velocity in the same way as NOTE_ON velocity
* Select between: stretch note position only / stretch note length only / stretch note position & length
* Option to change output step
* MIDI channel filter option
* MIDI message filter option
* Velocity swing option
* Step shape editor
* Write plugin manual
* Make tutorial video
* Link manual & tutorial in plugin


## Links

TODO
