#include "grid.h"

Grid::Grid(int pixelWidthP, int pixelHeightP, int squareSizeP, QColor fillColor)
{
    //Set the data members to the passed parameters.
    pixelWidth = pixelWidthP;
    pixelHeight = pixelHeightP;
    squareSize = squareSizeP;

    //Calculate the number of rows and columns, including any that are partially-visible.  I.e. round
    //the number of rows and columns up.
    rowCount = pixelHeight/squareSize;
    if (pixelHeight%squareSize > 0) //if there is a remainder, add one (serves to round up)
        rowCount++;
    columnCount = pixelWidth/squareSize;
    if (pixelWidth%squareSize > 0) //if there is a remainder, add one (serves to round up)
        columnCount++;

    //Create the image that will show the grid!
    gridImage = new QImage(pixelWidth, pixelHeight, QImage::Format_RGB32);
    gridImage->fill(fillColor);

}




Grid::~Grid()
{
    delete gridImage;
}




//This function colors a square to the passed color.  The column and row parameters start at zero, like
//C++ arrays and like the setPixel functions uses as well.
void Grid::drawSquare(int column, int row, QColor color)
{
    //Quit if the square being drawn isn't visible.  The "or equal to" is required because the column
    //and row counts start at zero.  For example, if columnCount = 3 and I try to draw to column number
    //3 (the 4th column), this code will quit.
    if ( (column < 0)||(row < 0)||(column >= columnCount)||(row >= rowCount) )
        return;

    int firstPixelColumn = column*squareSize;
    int lastPixelColumnPlusOne = firstPixelColumn+squareSize;

    int firstPixelRow = row*squareSize;
    int lastPixelRowPlusOne = firstPixelRow+squareSize;

    //If either of the lastPlusOne variables is greater than the corresponding dimension of the image,
    //change them to stay within the image.  This should keep us from trying to draw pixels that are outside
    //the image and make it possible to draw partial squares.
    if (lastPixelColumnPlusOne>pixelWidth)
        lastPixelColumnPlusOne=pixelWidth;
    if (lastPixelRowPlusOne>pixelHeight)
        lastPixelRowPlusOne=pixelHeight;

    for (int i=firstPixelColumn; i<lastPixelColumnPlusOne; i++)
    {
        for (int j=firstPixelRow; j<lastPixelRowPlusOne; j++)
        {
            gridImage->setPixel(i, j, color.rgb() );
        }
    }
}



//This function is for when the pixel size of the image is changed.  It just deletes the image and then
//does exactly what the constructor does to start from scratch.
void Grid::changeImageSize(int pixelWidthP, int pixelHeightP, int squareSizeP, QColor fillColor)
{
    delete gridImage;

    //Set the data members to the passed parameters.
    pixelWidth = pixelWidthP;
    pixelHeight = pixelHeightP;
    squareSize = squareSizeP;

    //Calculate the number of rows and columns, including any that are partially-visible.  I.e. round
    //the number of rows and columns up.
    rowCount = pixelHeight/squareSize;
    if (pixelHeight%squareSize > 0) //if there is a remainder, add one (serves to round up)
        rowCount++;
    columnCount = pixelWidth/squareSize;
    if (pixelWidth%squareSize > 0) //if there is a remainder, add one (serves to round up)
        columnCount++;

    //Create the image that will show the grid!
    gridImage = new QImage(pixelWidth, pixelHeight, QImage::Format_RGB32);
    gridImage->fill(fillColor);

}


//If only the square size is changed, then it is necessary to delete and recreate the image.
void Grid::changeSquareSize(int squareSizeP, QColor fillColor)
{
    squareSize = squareSizeP;

    //Calculate the number of rows and columns, including any that are partially-visible.  I.e. round
    //the number of rows and columns up.
    rowCount = pixelHeight/squareSize;
    if (pixelHeight%squareSize > 0) //if there is a remainder, add one (serves to round up)
        rowCount++;
    columnCount = pixelWidth/squareSize;
    if (pixelWidth%squareSize > 0) //if there is a remainder, add one (serves to round up)
        columnCount++;

    //Redraw the image
    gridImage->fill(fillColor);

}





void Grid::fillImage(QColor fillColor)
{
    //Redraw the image
    gridImage->fill(fillColor);
}
