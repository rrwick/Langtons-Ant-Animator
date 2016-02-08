#ifndef GRID_H
#define GRID_H

#include <QtGui>

class Grid
{
public:
    Grid(int pixelWidthP, int pixelHeightP, int squareSizeP, QColor fillColor);
    ~Grid();

    QImage * gridImage;
    void drawSquare(int column, int row, QColor color);
    void changeImageSize(int pixelWidthP, int pixelHeightP, int squareSizeP, QColor fillColor);
    void changeSquareSize(int squareSizeP, QColor fillColor);
    void fillImage(QColor fillColor);
    int rowCount;
    int columnCount;

private:
    int pixelWidth;
    int pixelHeight;
    int squareSize;

};

#endif // GRID_H
