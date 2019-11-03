# B.Slizr
formerly known as B.Slicer

Description: LV2 audio effect plugin for sequenced slicing of stereo audio input signals. Each slice can be levelled up or down to get a step sequencer-like effect.

![screenshot](https://raw.githubusercontent.com/sjaehn/bmusic-bslicer/master/Screenshot.png "Screenshot from B.Slizr")

Installation
------------
a) Install the bslizr package for your system
* [Arch linux](https://www.archlinux.org/packages/community/x86_64/bslizr/) by David Runge
* [NixOS](https://github.com/NixOS/nixpkgs/blob/master/pkgs/applications/audio/bslizr/default.nix) by Bart Brouns
* Check https://repology.org/project/bslizr/versions for other systems

b) Build your own binaries in the following three steps.

Step 1: [Download the latest published version](https://github.com/sjaehn/BSlizr/releases) of B.Slizr. Or clone or
[download the master](https://github.com/sjaehn/BSlizr/archive/master.zip) of this repository.

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
After the installation Ardour, Carla, and any other LV2 host should automatically detect B.Slizr.

If jalv is installed, you can also call it

```
jalv.gtk https://www.jahnichen.de/plugins/lv2/BSlizr
```

to run it stand-alone and connect it to the JACK system.

Notes:

* **Jack transport is required to get information about beat / position**
* If you use an older version (until v0.3) of this plugin, use the URI https://www.jahnichen.de/plugins/lv2/BSlicer instead.

What's new
-----------
* Jack transport status notification
* Downsize on small screens
* New install method

Usage
-----
The plugin slices a stereo input, amplifies or silences the individual slices and send the whole sequence to the output. Although this affects only the audio signal, it needs a playback running (Jack transport).

The interface is divided into three parts: step controls, monitor and step shape.

Step controls
* Sequences per bar : Number of sequences in one bar (1..8)
* Number of steps : Number of steps in one sequence (1..16)
* Step control : Coefficient for sound reduction for each individual step

Monitor
* On/Off switch: Switches monitor and monitor <-> plugin communication on/off. Reduces CPU load.
* Scale : Scales the visualization of the input / output signal
* Monitor : Visualization (l + r signal) the input / output signal for a whole sequence

Step shape
* Attack
* Decay
* Monitor : Visualization of a single step

Links
-----
* Video: https://www.youtube.com/watch?v=gLI2ozKdNDk
