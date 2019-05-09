# Qt-ENVI-Numpy-Viewer
Viewer for hyperspectral Numpy cubes.

This viewer opens practically all numpy files, that has at least two dimensions. Using a slider it is possible to scroll through the channels. It features an envi to numpy converter, that converts raw ENVI shots to numpy (even white/black-ref support).

This tool might also be useful for people working with DICOM. If you convert a DICOM image to a numpy array, and then save it, then this tool is useful to scroll through slices.

Can convert BIL ENVI files from e.g. "Lumo scanner", Specim FX17, Specim FX10, etc.
You can also compare the spectrum of a pixel by left/right clicking in the canvas.

Three functions were kindly borrowed from K. Dijkstra's DDSL.

The INIReader origins from the link below, and is modified to parse hdr files instead.
https://github.com/benhoyt/inih

Prerequisites:
 - Qt
 - QtCharts (add during installation, or with maintenance tool)

Prebuilt binaries:
https://github.com/ArendJanKramer/Qt-ENVI-Numpy-Viewer/releases/

![main window](https://raw.githubusercontent.com/ArendJanKramer/Qt-ENVI-Numpy-Viewer/master/artwork/Screenshot%20mainwindow.png)
![spectrum](https://raw.githubusercontent.com/ArendJanKramer/Qt-ENVI-Numpy-Viewer/master/artwork/Screenshot%20spectogram.png)
