###########################
Seek Thermal Imager Library
###########################


Introduction
############

This library can be used to retrieve frames of Seek cameras.


Usage
#####

See the ``seek-test.cpp`` example file to see how this is used.

At the moment, there is no thermal information provided by the
library... that's TODO_.

The example can be plugged onto ffmpeg like this:

.. code:: sh

   ./build/seek-test \
    | ffplay -i - -f rawvideo -video_size 208x156 -pixel_format gray16le


Installation
############

The library uses `the waf build
system<http://code.google.com/p/waf/>`_ to compile.
This is done using something like (assuming you want to install to /usr):

.. code:: sh

   curl http://ftp.waf.io/pub/release/waf-1.8.5 > waf
   python waf configure --prefix /usr
   python waf
   sudo python waf install --destdir /


Issues
######

The project home is on github: https://github.com/zougloub/libseek
and issues can be reported there.

Alternately, you can contact `me <mailto:cJ-libseek@zougloub.eu>`_ directly.

Credits
#######

Thanks to `Stephen Stair <https://github.com/sgstair>`_ who put
working code on github, and to the rest of the people discussing on
the `Yet another cheap thermal imager incoming.. Seek Thermal thread
of the EEVblog Electronics Community Forum
<http://www.eevblog.com/forum/testgear/yet-another-cheap-thermal-imager-incoming>`_
which was about the only thing I found when looking for existing stuff
for my camera, once I could plug it on a real computer.


Legal
#####

The code uses the MIT License as this is used by the winusbdotnet
project which was partly slurped in here.
See the ``LICENSE`` file along with the code.

The source code doesn't contain per-file headers, but this is no
excuse for being evil.

The library was developed with interoperability in mind, ie. the goal
of providing a working PC interface to the imager, which is yet
another USB device, for the people who may have smashed their phone's
glass.


Development
###########


Contributing
************

Hey, this is on github, you know what to do!


TODO
****

- Thermal information in the frame (min/max values)
- Bad Pixel Compensation
- Higher-level wrapper?
- Movement-based super-resolution?
