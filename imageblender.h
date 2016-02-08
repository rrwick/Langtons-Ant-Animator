#ifndef IMAGEBLENDER_H
#define IMAGEBLENDER_H

#include <QImage>

class ImageBlender
{
public:
    ImageBlender(bool * runningFlagP);
    ~ImageBlender();
    void initialize(int arraySizeP);
    void addImage(QImage imageToAdd, int i);
    QImage blendImages();

    QImage * arrayToBlend;

private:
    int arraySize;
    double arraySizeDouble;
    QSize imageSize;
    bool initialized;
    bool * runningFlag;
};

#endif // IMAGEBLENDER_H
