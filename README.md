# B.Choppr
An audio stream chopping LV2 plugin.

Description: B.Choppr cuts the audio input stream into a repeated sequence of up to 16 chops.
Each chop can be leveled up or down (gating). B.Choppr is the successor of B.Slizr.

B.Choppr is in development and should not be used for production, yet. Forward and backward
compatibility are not guaranteed at this point of development.

Installation
------------

Build your own binaries in the following three steps.

Step 1: [Download the latest published version](https://github.com/sjaehn/BChoppr/releases) of B.Choppr. Or clone or
[download the master](https://github.com/sjaehn/BChoppr/archive/master.zip) of this repository.

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


Running
-------
After the installation Ardour, Carla, and any other LV2 host should automatically detect B.Choppr.

If jalv is installed, you can also call it

```
jalv.gtk https://www.jahnichen.de/plugins/lv2/BChoppr
```

to run it stand-alone and connect it to the JACK system.

Notes:

* **Jack transport is required to get information about beat / position**

Usage
-----
The plugin slices a stereo input, amplifies or silences the individual slices and send the whole
sequence to the output. Although this affects only the audio signal, it needs a playback running
(Jack transport).

The interface is divided into three parts: step controls, monitor and step shape.

Step controls
* Sequences per bar : Number of sequences in one bar (1..8)
* Number of steps : Number of steps in one sequence (1..16)
* Step markers: Defines the size of each step
* Step level control : Sound level for each individual step

Monitor
* On/Off switch: Switches monitor and monitor <-> plugin communication on/off. Reduces CPU load.
* Monitor : Visualization (l + r signal) the input / output signal for a whole sequence. Use mouse wheel or drag to zoom in or out.

Step shape
* Attack
* Decay
* Monitor : Visualization of a single step
