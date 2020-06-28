# B.Schaffl

Pattern-controlled MIDI amp & time stretch plugin to produce shuffle / swing effects. (Experimental)

**Warning: Experimental software. Not intended for production use, yet. No guarantees.**

![screenshot](https://raw.githubusercontent.com/sjaehn/BSchaffl/master/doc/screenshot.png "Screenshot from B.Schaffl")


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

B.Schaffl is a MIDI converting plugin. It needs a MIDI input and a MIDI output.

The graphical user interface is divided into thee parts. In the top right is a visualization of the
full sequence from MIDI input (IN), over controls to change the step amplification (AMP) and markers
to stretch or compress the step size (STR) to MIDI output (OUT).

In the bottom right are controllers to directly effect the MIDI editing (sequence size, nr of steps,
amp swing, amp randomness, amp process, step swing, step swing randomness, step swing process auto
markers). The left part contains a menu for global settings (MIDI channel filter, MIDI message filter,
smart quantization, latency).

### Sequence size

Set the size of a full sequence ranging from 1/8 to 4 second(s), beat(s), or bar(s). You need Jack
transport to get position information to use beat(s) or bar(s) as base.


### Number of steps

You can divide each sequence into up to 16 steps.


### Amp swing

Step amplification swing. This feature can be used to either reduce velocity of MIDI notes for odd
steps or for even steps. Values lower than 1 (down to 1/128) mean reduction of odd step MIDI velocity.
Values higher than 1 (up to 128) mean reduction of even step MIDI velocity.


### Amp randomness

Symmetrically randomizes the amplification for each MIDI note velocity. An amp randomization value of
0.0 means no randomization. An amp randomization value of 1.0 means full randomization and the
resulting MIDI note velocity will be in the range between 0 and two times the unrandomized velocity.


### Amp process

Applies plugin amp effects (amp sliders, amp swing, amp randomness) to the MIDI note velocity.
A value of 0.0 results in unchanged MIDI note velocity. A value of
1.0 fully applies the plugin amp effects to the MIDI note velocity. Also values outside the 0.0 .. 1.0
range are allowed. A value of -1.0 inversely applies the plugin amp effects to the MIDI note velocity.
And a value of 2.0 applies the double plugin amp effects to the MIDI note velocity.


### Step swing

Step stretch swing. This feature can be used to compress or to stretch even and odd steps. Values
lower than 1 (down to 1/3) mean compression of odd steps for the benefit of the following even step.
Values higher than 1 (up to 3) mean stretching of odd steps to the disadvantage of the following even
step.


### Step swing randomness

Randomizes the start and end positions for each step to both directions. A step swing randomization
value of 0.0 means no randomization. A step swing randomization value of 0.5 means half randomization
and the step start and end positions are left or rightward-shifted in the range between 0 and the
half distance to the respective neighbor step.


### Step swing process

Applies plugin amp effects (amp sliders, amp swing, amp randomness) to the step stretch.
A value of 0.0 results in unchanged temporal MIDI signal positions. A value of
1.0 fully applies the plugin step stretch effects to the respective MIDI signal positions.


### Step stretch Markers

Drag the stretch markers to manually compress or stretch the step size.

All stretch markers are set to "auto" by default. This means that all markers are homogeneously
distributed and are automatically re-positioned after changes. The markers status is changed to
"manual" once a marker is dragged. You can also change the markers status by right-clicking on it or by
resetting all markers by pressing the "auto markers" button.


### Auto markers

Sets all markers to auto positioning.


### Step amp controls

Drag (or scroll) the sliders to reduce the velocity of MIDI notes for the respective step.


### MIDI channel filter

Select the MIDI channels to be processed by the plugin. All non-selected channels are bypassed.


### MIDI message filter

Select the MIDI messages (MIDI status byte) to be processed by the plugin. All non-selected messages
are bypassed.


### Smart quantization

During live playing and even if in computer-generated sequences, MIDI signals often do not exactly fit
into the host-communicated beat/bar pattern. The smart quantization algorithm synchronizes not exactly
fitting MIDI input signals (e.g., notes) with the step pattern if the signal is within a user-defined
range from the step start or end.

It is recommended to set the range (in steps) to the half size of the shortest expected note for the
start.

MIDI input signals can be synchronized just by assignment to a step or by fitting into a step or both.
Assignment means that the MIDI input signal is re-assigned to the just starting step if a step starts
within the range. Fitting means that all MIDI input signals except NOTE_OFF are moved to the start of
a step if the start is within the range. NOTE_OFF signals are moved to the end of a step if the
end is within the range. You should combine both for quantization.


### Latency

Latency is required if a marker is automatically or manually dragged to the left. The plugin itself
calculates the latency by default. This value (in ms) is shown in the top right of the plugin.
Alternatively, the user may manually set a fixed latency of up to 192000 frames.

Latency-correction of time/position data (time, bar, beat) is required for the proper work of B.Schaffl.
However, hosts may differently handle latency and time/position data. Some correct it by themselves (as
in Ardour 6) others not (Ardour 5). Switch on latency-correction of time/position if the host doesn't
correct it by itself.


## Acknowledgments

* Bart Brouns for the original idea plus subsequent ideas about principle and features
* Robin Gareus for ideas to technically solve *rubato*-related problems


## What's new

* Randomize amp
* Randomize stretch
* Process amount for amp and stretch


## TODO's

* Handle changes which effect step stretch during playback
* Option to amplify NOTE_OFF velocity in the same way as NOTE_ON velocity
* Select between: stretch note position only / stretch note length only / stretch note position & length
* Option to change output step
* Step shape editor
* Make tutorial video
* Link tutorial in plugin


## Links

* Preview: https://www.youtube.com/watch?v=5oPtBPAgvmY
