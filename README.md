# Counting Food Grains with a Computer Vision Application
In this demo app, I'll show some processing steps required for counting objects. Let's say you need to count food grains, like rice or beans, or maybe you need to detect nonstandard sizes for quality control.

The code for this application is written in **C++**, with the **Qt5** library for building the GUI interface, and **OpenCV 4** for image processing.

For this computer vision application, we will apply the following steps:

- load image
- convert to grayscale
- binarize the image (that is, make the foreground white and the background black)
- if necessary, clean small noise applying morphological operators like erosion/dilation
- count the objects with a [connected-component labeling algorithm](https://en.wikipedia.org/wiki/Connected-component_labeling)

There are two examples images in `docs`:

- [50 grains of rice](docs/rice-count-50-640x480-IMG_7802.jpg)
- [31 grains of red bean](docs/red-beans-count-31_640x480-IMG_7804.jpg)

[Tutorial](docs/TUTORIAL.md)
