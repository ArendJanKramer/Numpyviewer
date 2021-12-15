# Numpy Viewer for Hyperspectral ENVI, and Tensors
Viewer for multi dimensional Numpy arrays for Windows, macOS and Linux.

This viewer opens practically all numpy files, that has at least two dimensions. Using a slider it is possible to scroll through the batches and channels.
This tool might also be useful for people working with DICOM. If you convert a DICOM image to a numpy array, and then save it, then this tool is useful to scroll through slices.


Get it now on Snapcraft, or find prebuilt binaries for Windows, mac or AppImage on the Releases page
[![Numpy viewer](https://snapcraft.io/numpyviewer/badge.svg)](https://snapcraft.io/numpyviewer)

## Hyperspectral ENVI support
It features an ENVI to numpy converter, that converts raw ENVI shots to numpy (even white/black-ref support).
Tested on BIL ENVI files from the Lumo scanner series, Specim FX17, Specim FX10, etc.
You can also compare the spectrum of a pixel by left/right clicking in the canvas.

![main window](https://raw.githubusercontent.com/ArendJanKramer/Numpyviewer/master/artwork/Screenshot%20mainwindow.png)
![envi converter](https://raw.githubusercontent.com/ArendJanKramer/Numpyviewer/master/artwork/Screenshot%20envi.png)
![spectrum](https://raw.githubusercontent.com/ArendJanKramer/Numpyviewer/master/artwork/Screenshot%20spectogram.png)

## Multidimensional Tensor support
The numpy viewer supports all numpy array that have at least two dimensions. Tensors usually have four dimensions, and this viewer allows you to select a batch and a channel.
If desired, it can also merge different slices into one RGB view, or it can assign unique colors to each value in the canvas.

![tensor](https://raw.githubusercontent.com/ArendJanKramer/Numpyviewer/master/artwork/Screenshot%20tensor.png)
![instances](https://raw.githubusercontent.com/ArendJanKramer/Numpyviewer/master/artwork/Screenshot%20instances.png)

## NPZ Support
The numpy viewer supports all npz files. It allows to select the instances within the npz file. 
Then, the appropriate array will be loaded and all features can then be used. 

![npz](https://raw.githubusercontent.com/ArendJanKramer/Numpyviewer/master/artwork/Screenshot%20npz.png)



## Scrolling through the slices -- "horizontal scroll"
The numpy viewer supports fast and easy scrolling through slices using a Horizontal Scroll. This may be very useful to review 3D images, or to quickly scan through various examples in your batch or dataset. 

For scrolling through the slices, you can either use your hardware `horizontal scroll` mouse wheel, or a combination of `Alt`+`scroll wheel` *(the standard one)*. 

The default scrolling speed is 1 slice *"per step"*, in order to be able to reach any slice you want. However, this may be too slow and tedious if you want to find some particular slide far away from the begining -- and that's why there is also a "Fast mode". Just hold down the `Shift` key while you are scrolling and you will scroll 3x faster. 




## Contributes
Three functions were kindly borrowed from K. Dijkstra's DDSL.

The INIReader origins from the link below, and is modified to parse hdr files instead.
https://github.com/benhoyt/inih

## Build
Prerequisites:
 - Qt 5
 - QtCharts

Prebuilt binaries:
https://github.com/ArendJanKramer/Numpyviewer/releases/
