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

## Counting red beans

For another image, with 31 red beans, the process is almost the same, but we need to adjust the threshold value and invert background/foreground. Let's check it.

Opening the image, you will find that there are some shadows in the corners as well as glare on the beans.

![Red beans image!](load-red-beans-image.png?raw=true)

Converting to grayscale, now we see a difference.

![Grayscale beans!](grayscale-red-beans-image.png?raw=true)

We can apply the threshold operation with the default value of 128.

![Threshold 128 beans!](threshold-128-red-beans-image.png?raw=true)

We can see a lot of noise in the corners. So, it is better to adjust the threshold. Undoing the operation, we can set the threshold for 95 (in a future version, I'll add a histogram; there are also automatic thresholding algorithms). 

![Set threshold 95 to beans!](set-threshold-95-red-beans-image.png?raw=true)

![Threshold 95 beans!](threshold-95-red-beans-image.png?raw=true)

The connected-component labeling algorithm assumes that the foregound is white and the background is black, so we need to invert the image.

![Invert beans!](invert-red-beans-image.png?raw=true)

And now, the result for counting the beans. It is a good result, with 3 errors from "glued" beans, 1 error from the glare (I was expecting more problems with it), and 1 in the upper left corner. Try to improve the results adjusting the threshold, and applying erosion and dilation!

![Count beans!](count-red-beans-image.png?raw=true)

There are more advanced techniques to apply, but I hope you've learning the basics with this tutorial.
