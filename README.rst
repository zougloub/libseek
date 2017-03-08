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


The library performs bad pixels correction based on a list of bad
pixels; that list can be obtained by running:

.. code:: sh

   # Get 100 raw frames... move the camera around while this runs
   ./build/seek-test-calib

   # Generate bad pixels images (calib-avg.png, calib-std.png, calib-bpc-dead.png)
   ./test-calib.py

   # Generate bad pixels correction structure
   ./seek_bpc.py


Installation
############

The library uses `the waf build system <http://code.google.com/p/waf/>`_
to compile.
This is done using something like (assuming you want to install to /usr):

.. code:: sh

   curl https://waf.io/waf-1.8.14 > waf
   python waf configure --prefix /usr
   python waf
   sudo python waf install --destdir /

The test scripts in Python use numpy and OpenCV, so you should have
those dependencies installed to run the scripts.


Issues
######

The project home is on github: https://github.com/zougloub/libseek
and issues can be reported there.


Credits
#######

This library is using bits and pieces from work done by other people.

Thanks to `Stephen Stair <https://github.com/sgstair>`_ who put
working code on github, and to the rest of the people discussing on
the `Yet another cheap thermal imager incoming.. Seek Thermal thread
of the EEVblog Electronics Community Forum
<http://www.eevblog.com/forum/testgear/yet-another-cheap-thermal-imager-incoming/>`_
which was about the only thing I found when looking for existing stuff
for my camera, once I could plug it on a real computer.

Thanks to `David Tulloh <https://github.com/lod>`_ for creating and maintaining
https://github.com/lod/seek-thermal-documentation/


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

- Bad Pixel Compensation - Calibration clean-up

  Bad pixels (and bad clusters) are corrected by the core library,
  but the code used to generate the correction information is not
  that straightforward to use.

- gstreamer source

- Thermal information in the frame (retrieve min/max values)

- Absolute temperature reading

- Higher-level wrapper?

- Movement-based super-resolution?


Device Information
******************


Camera Information
==================

The camera uses a microbolometer array of 12 µm pixels.

It has some kind of shutter, used to perform Flat Field Correction
regularly
(http://www.flir.com/cvs/cores/knowledgebase/index.cfm?CFTREEITEMKEY=342&view=35774,
http://www.google.ca/patents/US8373757)
and making the camera alternatively provide shutter images and scene
images (shutter operating once every 23 pictures after start-up, and
also generating one more unusable frame before the calibration frame).

Issues have been reported with the FFC, and a thermal gradient can be
seen through the image.

The sensor array has a fraction (TODO provide) of what we'll call
*black pixels*, pixels that carry no usable data by design.
They are thought to exist to improve the SNR.

There are also bad pixels, which are relatively frequent, at least on
my unit... where I can see TODO of them.
Thus, some kind of compensation needs to be performed on these bad pixels.


Reading from the Camera
=======================

``lsusb`` says::

  Bus 002 Device 118: ID 289d:0010
  Device Descriptor:
    bLength                18
    bDescriptorType         1
    bcdUSB               2.00
    bDeviceClass            0 (Defined at Interface level)
    bDeviceSubClass         0
    bDeviceProtocol         0
    bMaxPacketSize0        64
    idVendor           0x289d
    idProduct          0x0010
    bcdDevice            1.00
    iManufacturer           1 Seek Thermal
    iProduct                2 PIR206 Thermal Camera
    iSerial                 5 @Ă耀
    bNumConfigurations      1
    Configuration Descriptor:
      bLength                 9
      bDescriptorType         2
      wTotalLength           64
      bNumInterfaces          2
      bConfigurationValue     1
      iConfiguration          0
      bmAttributes         0x80
        (Bus Powered)
      MaxPower              100mA
      Interface Descriptor:
        bLength                 9
        bDescriptorType         4
        bInterfaceNumber        0
        bAlternateSetting       0
        bNumEndpoints           2
        bInterfaceClass       255 Vendor Specific Class
        bInterfaceSubClass    240
        bInterfaceProtocol      0
        iInterface              3 iAP Interface
        Endpoint Descriptor:
          bLength                 7
          bDescriptorType         5
          bEndpointAddress     0x01  EP 1 OUT
          bmAttributes            2
            Transfer Type            Bulk
            Synch Type               None
            Usage Type               Data
          wMaxPacketSize     0x0200  1x 512 bytes
          bInterval               0
        Endpoint Descriptor:
          bLength                 7
          bDescriptorType         5
          bEndpointAddress     0x81  EP 1 IN
          bmAttributes            2
            Transfer Type            Bulk
            Synch Type               None
            Usage Type               Data
          wMaxPacketSize     0x0200  1x 512 bytes
          bInterval               0
      Interface Descriptor:
        bLength                 9
        bDescriptorType         4
        bInterfaceNumber        1
        bAlternateSetting       0
        bNumEndpoints           0
        bInterfaceClass       255 Vendor Specific Class
        bInterfaceSubClass    240
        bInterfaceProtocol      1
        iInterface              4 com.thermal.pir206.1
      Interface Descriptor:
        bLength                 9
        bDescriptorType         4
        bInterfaceNumber        1
        bAlternateSetting       1
        bNumEndpoints           2
        bInterfaceClass       255 Vendor Specific Class
        bInterfaceSubClass    240
        bInterfaceProtocol      1
        iInterface              4 com.thermal.pir206.1
        Endpoint Descriptor:
          bLength                 7
          bDescriptorType         5
          bEndpointAddress     0x02  EP 2 OUT
          bmAttributes            2
            Transfer Type            Bulk
            Synch Type               None
            Usage Type               Data
          wMaxPacketSize     0x0200  1x 512 bytes
          bInterval               0
        Endpoint Descriptor:
          bLength                 7
          bDescriptorType         5
          bEndpointAddress     0x82  EP 2 IN
          bmAttributes            2
            Transfer Type            Bulk
            Synch Type               None
            Usage Type               Data
          wMaxPacketSize     0x0200  1x 512 bytes
          bInterval               0
  Device Qualifier (for other device speed):
    bLength                10
    bDescriptorType         6
    bcdUSB               2.00
    bDeviceClass            0 (Defined at Interface level)
    bDeviceSubClass         0
    bDeviceProtocol         0
    bMaxPacketSize0        64
    bNumConfigurations      1
  Device Status:     0x0000
    (Bus Powered)

This library is using the first interface ``iAP Interface``.

The communication protocol is pretty simple, but there's no point (?)
to understand it in order to write something usable.
The camera is autonomous at providing data, after an initial configuration
consisting in a handful of commands, and a "send me data now" request
it will provide image frames.

There are different type of frames, that are identified by an in-band
status byte located at position 20:

- Regular frames (code 3)
- Flat Field Calibration frames (code 1)
- Drift Calibration frames (code 4 or 10)
- Unusable frames (code 6), probably because the shutter is in progress
- TBD (code 8)
- TBD (code 7)
- ...

The raw frame data contains regular "holes", values that are "black
pixels" by design.
The missing values are reconstructed using interpolation from
neighboring cells.
The locations are predicted, but it's also possible to identify them
because the values are also missing in calibration frames.

Special frame locations:

- At position 2, something that looks like it is related to the device
  temperature.

- At position 20, the frame code

- At position 80, a frame counter.

