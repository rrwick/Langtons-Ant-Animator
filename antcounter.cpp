#include "antcounter.h"

AntCounter::AntCounter(Grid *displayGridP, AntSettings *settingsP, StateWidget *stateArrayP)
{
    //Store the pointers to the Grid and AntSettings objects and the StateWidget array
    displayGrid = displayGridP;
    settings = settingsP;
    stateArray = stateArrayP;

    //initialize the max values to zero
    maxWidthSoFar = 0;
    maxHeightSoFar = 0;

    //Set up the pen that will be used to draw the outlines around the counter and rules
    pen.setWidth(2);
    pen.setJoinStyle(Qt::MiterJoin);
}



void AntCounter::paintCountAndRules()
{
    //Make a painter for the image
    QPainter painter(displayGrid->gridImage);

    //Set the painter to use the defined pen
    painter.setPen(pen);

    //Set the painter's font
    painter.setFont(settings->counterFont);

    //Make a QFontMetrics object that will be used to determine the string sizes
    QFontMetrics fontMetrics(settings->counterFont);

    //Draw the counter onto the image, if that setting is on
    if (settings->showCounter)
    {
        //Set up the string to be painted on
        QString timeText = addCommasToNumber(settings->time);

        drawTextOnImage(timeText, settings->counterLocation, true, &painter, &fontMetrics);
    }

    //Draw the rules onto the image, if that setting is on
    if (settings->showRules)
    {
        //Set up the string to be painted on
        QString ruleText = getStateList();

        drawTextOnImage(ruleText, settings->rulesLocation, false, &painter, &fontMetrics);
    }
}





void AntCounter::drawTextOnImage(QString text, int location, bool onlyGrow, QPainter * painter, QFontMetrics * fontMetrics)
{
    //Use the QFontMetrics object to get a rectangle for the size of the string
    QRect textRect = fontMetrics->boundingRect(text);

    if (onlyGrow)
    {
        //Ensure that the size hasn't gotten any smaller!  The size of the counter box is only allowed to grow until it is reset.
        if (textRect.width()<maxWidthSoFar)
            textRect.setWidth(maxWidthSoFar);
        else
            maxWidthSoFar = textRect.width();
        if (textRect.height()<maxHeightSoFar)
            textRect.setHeight(maxHeightSoFar);
        else
            maxHeightSoFar = textRect.height();
    }

    //Create a new rectangle to make the outline around the text
    QRect outlineRect(0, 0, textRect.width()+10, textRect.height()+8);

    //Enlarge the text rectangle a bit so the text is less likely to be cut-off
    textRect.setWidth(textRect.width()+8);

    //Move the rectangles to the correct place in the image
    switch (location)
    {
    case 0: //top-left corner
        outlineRect.moveTopLeft(QPoint(16, 16));
        textRect.moveTopLeft(QPoint(20, 20));
        break;
    case 1: //top-right corner
        outlineRect.moveTopRight(QPoint(settings->pixelWidth-16, 16));
        textRect.moveTopRight(QPoint(settings->pixelWidth-20, 20));
        break;
    case 2: //bottom-left corner
        outlineRect.moveBottomLeft(QPoint(16, settings->pixelHeight-16));
        textRect.moveBottomLeft(QPoint(20, settings->pixelHeight-20));
        break;
    case 3: //bottom-right corner
        outlineRect.moveBottomRight(QPoint(settings->pixelWidth-16, settings->pixelHeight-16));
        textRect.moveBottomRight(QPoint(settings->pixelWidth-20, settings->pixelHeight-20));
        break;
    }

    //Fill the outline box with white
    painter->fillRect(outlineRect, QColor(255,255,255));

    //Draw the outline of the outline box with the new pen
    painter->drawRect(outlineRect);

    //Paint the text onto the image
    switch (location)
    {
    case 0: //top-left corner
        painter->drawText(textRect, Qt::AlignLeft, text);
        break;
    case 1: //top-right corner
        painter->drawText(textRect, Qt::AlignRight, text);
        break;
    case 2: //bottom-left corner
        painter->drawText(textRect, Qt::AlignLeft, text);
        break;
    case 3: //bottom-right corner
        painter->drawText(textRect, Qt::AlignRight, text);
        break;
    }


}





QString AntCounter::addCommasToNumber(int numberNeedingCommas)
{
    QString numberString = QString::number(numberNeedingCommas);

    for (int i = numberString.length() - 3; i > 0; i -= 3)
        numberString.insert(i, ",");

    return numberString;
}





QString AntCounter::getStateList()
{
    QString stateList = "";
    for (int i=0; i < settings->stateCount; i++)
    {
        switch (stateArray[i].direction)
        {
        case antRight:
            stateList += "R";
            break;
        case antLeft:
            stateList += "L";
            break;
        case antBack:
            stateList += "B";
            break;
        }
    }

    return stateList;
}




void AntCounter::reset()
{
    maxWidthSoFar = 0;
    maxHeightSoFar = 0;
}




//Since the stateArray pointer will change when the user alters the number of states (the array is actually
//deleted and recreated), it is necessary to have this function so the new pointer can be given to the AntCounter
//object.
void AntCounter::updateStateArrayPointer(StateWidget * stateArrayP)
{
    stateArray = stateArrayP;
}
