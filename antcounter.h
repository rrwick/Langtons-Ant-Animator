#ifndef ANTCOUNTER_H
#define ANTCOUNTER_H

#include "grid.h"
#include "antsettings.h"
#include "statewidget.h"

class AntCounter
{
public:
    AntCounter(Grid * displayGridP, AntSettings * settingsP, StateWidget * stateArrayP);

    void paintCountAndRules();
    void drawTextOnImage(QString text, int location, bool onlyGrow, QPainter *painter, QFontMetrics *fontMetrics);
    static QString addCommasToNumber(int numberNeedingCommas);
    QString getStateList();
    void reset();
    void updateStateArrayPointer(StateWidget * stateArrayP);

private:
    Grid * displayGrid;
    AntSettings * settings;
    StateWidget * stateArray;

    QPainter painter;
    QPen pen;

    int maxWidthSoFar;
    int maxHeightSoFar;
};

#endif // ANTCOUNTER_H
