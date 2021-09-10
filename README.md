# B.Schaffl

Groove quantizer LV2 MIDI plugin.

B.Schaffl is a slider / shape-controlled MIDI amp & time stretch plugin to vitalize sequencer-controlled MIDI
instruments and to produce shuffle / swing effects.

Key features:
* MIDI velocity amplification and timing manipulation plugin
* Swing and shuffle rhythms
* Pre-generator dynamics
* *Tempo rubato*
* Pattern (sliders) or shape-controlled
* MIDI filters
* Smart quantization
* Group / link individual instances of B.Schaffl
* Free and open source

![screenshot](https://raw.githubusercontent.com/sjaehn/BSchaffl/master/doc/screenshot.png "Screenshot from B.Schaffl")


## Installation

### a) Install the bschaffl package for your system

* [Arch user repository](https://aur.archlinux.org/packages/bschaffl.lv2-git) by Milkii Brewster, SpotlightKid
* [Debian / LibraZiK](https://librazik.tuxfamily.org/doc3/logiciels/bschaffl) by Olivier Humbert
* [Guix](https://guix.gnu.org/packages/bschaffl-1.2.0/)
* [NixOS](https://github.com/NixOS/nixpkgs/blob/release-20.09/pkgs/applications/audio/bschaffl/default.nix#L23) by Bart Brouns
* [openSUSE](https://software.opensuse.org/package/BSchaffl)
* Check https://repology.org/project/bschaffl/versions for other systems

Note: This will NOT necessarily install the latest version of B.Schaffl. The version provided depends on the packagers.

### b) Use the latest provided binaries

Unpack the provided bschaffl-\*.zip or bschaffl-\*.tar.xz from the latest release and 
copy the BSchaffl.lv2 folder to your lv2 directory (depending on your system settings,
~/.lv2/, /usr/lib/lv2/, /usr/local/lib/lv2/, or ...).

### c) Build your own binaries in the following three steps.

Step 1: [Download the latest published version](https://github.com/sjaehn/BSchaffl/releases) of B.Schaffl. Or clone or
[download the master](https://github.com/sjaehn/BSchaffl/archive/master.zip) of this repository.

Step 2: Install pkg-config and the development packages for x11, cairo, and lv2 if not done yet. If you
don't have already got the build tools (compilers, make, libraries) then install them too.

On Debian-based systems you may run:
```
sudo apt-get install build-essential
sudo apt-get install pkg-config libx11-dev libcairo2-dev lv2-dev
```

On Arch-based systems you may run:
```
sudo pacman -S base-devel
sudo pacman -S pkg-config libx11 cairo lv2
```

Step 3: Building and installing into the default lv2 directory (/usr/local/lib/lv2/) is easy using `make` and
`make install`. Simply call:
```
make
sudo make install
```

**Optional:** Standard `make` and `make install` parameters are supported. Alternatively, you may build a debugging version using
`make CPPFLAGS+=-g`. For installation into an alternative directory (e.g., /usr/lib/lv2/), change the
variable `PREFIX` while installing: `sudo make install PREFIX=/usr`. If you want to freely choose the
install target directory, change the variable `LV2DIR` (e.g., `make install LV2DIR=~/.lv2`) or even define
`DESTDIR`.

**Optional:** Further supported parameters include `LANGUAGE` (usually two letters code) to change the GUI
language (see customize).


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

B.Schaffl is a MIDI converting plugin. It needs a MIDI input and a MIDI output. Thus, it must be
placed **before** an instrument (synth or sampler).

The graphical user interface is divided into thee parts. In the top right you can find the core of
the plugin, the sequence editor. The sequence editor contains a visualization of the
full sequence from MIDI input (IN), over controls (or a shape) to change the step amplification (AMP)
and markers to stretch or shrink the step size (STR) to MIDI output (OUT).

In the bottom right are controllers to directly effect the MIDI editing (sequence size, nr of steps,
amp swing, amp randomness, amp process, step swing, step swing randomness, step swing process). The
left part contains a menu for global settings (MIDI channel filter, MIDI message filter,
smart quantization, latency).


### Sequence editor

#### Amp control mode

The sequence editor supports two different modes. Select between step controlled (sliders) and shape
controlled mode. The step controlled mode uses constant amp values for the whole respective step. If
you want more flexibility and change the amp value during a step (e. g., for accents), you can use
the shape mode.


#### Slider-controlled amp mode: Step amp controls

Drag (or scroll) the sliders to reduce the velocity of MIDI notes for the respective step.


#### Shape-controlled amp mode: Amp shape

Alternatively, draw a shape for the reduction of velocity of MIDI notes. Use tools (see toolbox) to
set different types of nodes, to cut, copy or paste nodes, to undo, redo or reset changes, and to hide,
show or snap to the grid.


#### Step stretch markers

Drag the stretch markers to manually shrink or stretch the step size.

All stretch markers are set to "auto" by default. This means that all markers are homogeneously
distributed and are automatically re-positioned after changes. The markers status is changed to
"manual" once a marker is dragged. You can also change the markers status by right-clicking on it or by
setting all markers by pressing the "All markers" buttons in the toolbox.


#### Toolbox

| Symbols | Description |
| --- | --- |
| ![convert_to_sliders](https://raw.githubusercontent.com/sjaehn/BSchaffl/master/inc/convert_to_steps.png "Convert to sliders") ![convert_to_shape](https://raw.githubusercontent.com/sjaehn/BSchaffl/master/inc/convert_to_linear_shape.png "Convert to shape") | Convert shape to sliders or vice versa |
| ![marker_tools](https://raw.githubusercontent.com/sjaehn/BSchaffl/master/inc/markers_tb.png "All marker tools") | Set all markers to auto or manual |
| ![node_tools](https://raw.githubusercontent.com/sjaehn/BSchaffl/master/inc/shape_tools_tb.png "Node tools") | Node tools (only in shape-controlled mode)|
| ![edit_tools](https://raw.githubusercontent.com/sjaehn/BSchaffl/master/inc/edit_tb.png "Edit tools") | Edit tools: cut, copy and paste (only in shape-controlled mode)|   
| ![history_tools](https://raw.githubusercontent.com/sjaehn/BSchaffl/master/inc/history_tb.png "History") | History: reset, undo and redo |
| ![grid_tools](https://raw.githubusercontent.com/sjaehn/BSchaffl/master/inc/grid_tb.png "Grid") | Grid: show grid or snap to grid |


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


### MIDI channel filter

Select the MIDI channels to be processed by the plugin. All non-selected channels are bypassed.


### MIDI message filter

Select the MIDI messages (MIDI status byte) to be processed by the plugin. All non-selected messages
are bypassed.


### MIDI note options

#### Keep note position

Keeps the note position unaffected from step stretch. Take care, this may cause overlapping notes.


#### Keep note duration

Keeps the note duration unaffected from step stretch. Take care, this may cause overlapping notes.


#### If notes overlap

B.Schaffl may temporarily produce overlapping notes if either the note position or the note duration is
kept unaffected from step stretch. Overlapping MIDI notes usually end at the first MIDI NOTE_OFF. You
may change this behavior to merge both notes or to split to two (ore more) discreet notes.


#### Note off amp

There are two options to amplify the NOTE_OFF velocity signal. Either amplification with the same value
as the NOTE_ON signal or amplification with the amp value for the step/position of the NOTE_OFF signal.


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
Alternatively, the user may manually set a fixed latency of up to 192000 frames. It is recommended to
manually set this parameter to the maximally expected latency if you automate and/or change step markers,
step swing, and/or step swing process during playback.

Latency-correction of time/position data (time, bar, beat) is required for the proper work of B.Schaffl.
However, hosts may differently handle latency and time/position data. Some correct it by themselves (as
in Ardour 6) others not (Ardour 5). Switch on latency-correction of time/position if the host doesn't
correct it by itself.


### 🔗 Shared data

If you use multiple instances of B.Schaffl you may be interested in sharing the plugin settings between
the instances. Click on one of the four shared data fields and the respective plugin instance is linked
to the shared data. The plugin instance data are copied to the shared data field if no other plugin
instance is linked to this shared data field before (otherwise *vise versa*).

If you now click on the same shared data box in another plugin instance, both
plugin instances get the same data.

Click again on the selected box to unlink the plugin instance from
shared data. The plugin now shows the host-provided data.

Note: Shared data are unlinked from host automation.


## Customize

You can create customized builds of B.Schaffl using the parameter `LANGUAGE` (e.g., `make LANGUAGE=DE`).
To create a new language pack, copy `src/Locale_EN.hpp` and edit the text for the respective definitions.
But do not change or delete any definition symbol!


## What's new

* Faster due to compiler optimizations (`-O3 -ffast-math`)
* Binary compatibility improved
  * Static libs
  * Strip symbols by default
* Binary packages provided
* Support build parameter LANGUAGE
* Add EN and DE locales
* Bugfixes
* Enable web browser call (help, tutorial) on fork-blocking hosts


## Acknowledgments

* Bart Brouns for the original idea plus subsequent ideas about principle and features
* Robin Gareus for ideas to technically solve *rubato*-related problems
* Jatin Chowdhury for ideas to technically solve shared data storage


## Links

* Preview: https://www.youtube.com/watch?v=5oPtBPAgvmY
* Features/Tutorial: https://www.youtube.com/watch?v=CBwkYDk5reU
