## Counting Rice

Let's start with a image with 50 rice grains.

Opening the image, you will find that there is a clear separation between the background and the foreground.

![Rice image!](load-rice-image.png?raw=true)

Converting to grayscale, we don't see much visual modification, but the 3 color channels are mixed into only one.

![Grayscale rice!](grayscale-rice-image.png?raw=true)

Now, we transform the grayscale image into a binary (only black and white) image, through a threshold operation. That is, if the intensity level of a pixel is less than a threshold value, it will be considered black, otherwise white. The default value in this application is 128 (from 0 to 255). Using the default threshold, we obtain a clear separation between the background and the foreground.

![Threshold 128 rice!](threshold-128-rice-image.png?raw=true)

After that, we can apply a connected-component labeling algorithm that search all the image for blobs (white pixels that are connected somehow). This application draws a circle for each detected object. If the area of the object is more than 30% smaller or bigger than mean area, the circle is red, indicating lower confidence in the detection. If the area is between this 30% limits, the circle is green. The result for the rice image is the following.

![Count rice!](count-rice-image.png?raw=true)

So, we see that two clusters each with 2 grains are wrongly detected as one grain. We can improve the detection trying to dissolve the link between the grains with a morphological operators called erosion. It is like using a rubber around the borders of the object. We undo the counting (Ctrl+Z) and apply twice the erosion. 

![Eroded rice image!](eroded2x-rice-image.png?raw=true)

Applying again the connected-component labeling, we have only one missed count. You could try more erosion, combine erosion with dilation, or simply filter for a range of area values.

![Count eroded rice image!](count-eroded-rice-image.png?raw=true)

