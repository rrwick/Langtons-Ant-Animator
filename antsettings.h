#ifndef ANTSETTINGS_H
#define ANTSETTINGS_H

#include <QtWidgets>
#include "antdirection.h"
#include "statewidget.h"

class AntSettings
{

public:
    //Default constructor - sets things to default values
    AntSettings();

    //functions
    void saveToFile(QString filename, StateWidget * stateArray);
    void loadFromFile(QString filename, StateWidget ** stateArrayPointer);
    void loadOneSetting(QString * settingLine, StateWidget ** stateArrayPointer);

    //The setting members are public to prevent having to use set/get functions.
    int stateCount;
    int firstRandom;
    int lastRandom;
    int pixelWidth;
    int pixelHeight;
    int cellSize;
    int gridBuffer;
    int startingDirection;
    int startingColumn;
    int startingRow;
    bool showCounter;
    int counterLocation;
    bool showRules;
    int rulesLocation;
    QFont counterFont;
    bool showAntColor;
    QColor antColor;
    int stepsPerUpdate;
    int delayPerUpdate;
    int stepsPerSample;
    int samplesPerFrame;
    int frameCount;
    bool saveZeroFrame;
    int searchSteps;
    bool includeBack;

    //This setting, the time counter, is not saved and loaded like the rest of the settings.  It is
    //part of this class so it can exist in just one place: MainWindow and AntGrid objects will both
    //be able to view it.
    int time;

};

#endif // ANTSETTINGS_H
