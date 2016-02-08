#include "imageblender.h"
#include <QtWidgets>

ImageBlender::ImageBlender(bool * runningFlagP)
{
    //Store the running flag - the image blender will only work when this is set to true
    runningFlag = runningFlagP;

    //Set initialized to false so the image blender won't work until it is initialized
    initialized = false;
}





ImageBlender::~ImageBlender()
{
    //If the image blender has been previously initialized, delete the image array now
    if (initialized)
        delete [] arrayToBlend;
}





void ImageBlender::initialize(int arraySizeP)
{
    //If the image blender has been previously initialized, delete the image array now
    if (initialized)
        delete [] arrayToBlend;

    //Store arguments in data members
    arraySize = arraySizeP;

    //check to make sure arraySize is a valid number
    if (arraySize < 1)
        return;

    //Create the image array
    arrayToBlend = new QImage [arraySize];

    //store arraySize as a double - will be used in averaging later
    arraySizeDouble = double(arraySize);

    //Label the image blender as initialized - allows it to work.
    initialized = true;
}





void ImageBlender::addImage(QImage imageToAdd, int i)
{
    arrayToBlend[i] = imageToAdd;
}





QImage ImageBlender::blendImages()
{
    //Quit with a null image if the image blender hasn't been initialized
    if (!initialized)
        return QImage();

    //Note the size of the first image in the array
    imageSize = arrayToBlend[0].size();

    //Quit with a null image if either the height or width is zero
    if (imageSize.isEmpty())
        return QImage();

    //loop through each image in the array to check its size
    for (int i = 0; i < arraySize; i++)
    {
        //If there is a size discrepancy, return a null image
        if ( !(arrayToBlend[i].size() == imageSize) )
            return QImage();

        //If any image has a format other than QImage::Format_RGB32, return a null image
        if ( !(arrayToBlend[i].format() == QImage::Format_RGB32) )
            return QImage();
    }

    //If the code got to here, everything should be okay and the blending can now begin.

    //create a new image to hold the blend
    QImage blendedImage(imageSize, QImage::Format_RGB32);

    //these variables will hold the totals for each pixel
    double redTotal, greenTotal, blueTotal;

    //these variables will hold the averages for each pixel
    double redAverage, greenAverage, blueAverage;

    //this variable holds the color for a pixel
    QRgb pixelColor;

    //Loop through the pixels
    for (int i = 0; i < imageSize.width(); i++)
    {
        //Process events so the GUI stays responsive while the image blending occurs.
        QCoreApplication::processEvents();

        //Quit with a null image if the running flag is false
        if (!(*runningFlag))
            return QImage();

        for (int j = 0; j < imageSize.height(); j++)
        {
            //reset the total variables
            redTotal = 0;
            greenTotal = 0;
            blueTotal = 0;

            //loop through each image and add to the totals
            for (int k = 0; k < arraySize; k++)
            {
                pixelColor = arrayToBlend[k].pixel(i, j);
                redTotal += qRed(pixelColor);
                greenTotal += qGreen(pixelColor);
                blueTotal += qBlue(pixelColor);
            }

            //divide by the number of images to get an average
            redAverage = redTotal/arraySizeDouble;
            greenAverage = greenTotal/arraySizeDouble;
            blueAverage = blueTotal/arraySizeDouble;

            //set the pixel in the blended image (adding 0.5 is to get rounding right because the conversion from double to int will truncate)
            blendedImage.setPixel(i, j, qRgb(int(redAverage+0.5), int(greenAverage+0.5), int(blueAverage+0.5)));
        }
    }

    return blendedImage;
}
