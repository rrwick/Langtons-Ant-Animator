#include "antsettings.h"

//default constructor - sets data members to default values
AntSettings::AntSettings()
{
    stateCount = 2;
    firstRandom = 1;
    lastRandom = 2;
    pixelWidth = 1280;
    pixelHeight = 720;
    cellSize = 5;
    gridBuffer = 200;
    startingDirection = 0; //up
    startingColumn = 128;
    startingRow = 72;
    showCounter = true;
    counterLocation = 3;
    showRules = false;
    rulesLocation = 0;
    showAntColor = true;
    antColor = QColor(255, 0, 0);
    stepsPerUpdate = 1;
    delayPerUpdate = 0;
    stepsPerSample = 10;
    samplesPerFrame = 100;
    frameCount = 300;
    saveZeroFrame = true;
    searchSteps = 1000000;
    includeBack = false;

    QFont loadFont;
    loadFont.fromString("Consolas,26,-1,5,50,0,0,0,0,0");
    counterFont = loadFont;

    time = 0;

}





//This function saves all settings to a file, both the ones stored in the AntSettings object and the
//ones in the stateArray array.  I have no need to alter the contents of the stateArray array, so I
//can just use the normal pointer approach that allows me to access an array.  This differs from the
//loadFromFile function below where I do need to change the array's contents and therefore need to
//use a pointer to a pointer.
void AntSettings::saveToFile(QString fileName, StateWidget * stateArray)
{
    //Create the file object using the name and path provided by the user
    QFile saveFile(fileName);

    //Try to open the file
    if (saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //If opening was successful...

        //Create an output stream object
        QTextStream outputStream(&saveFile);

        //Define a delimiter
        QString delimiter = "=";

        //Output all of the settings to the file!

        //These settings are from the settings docking widget (everything but colors and directions of states)
        //format: setting name=setting value
        //example: number of states=2
        outputStream << "number of states" << delimiter << stateCount << endl;
        outputStream << "first state to randomize" << delimiter << firstRandom << endl;
        outputStream << "last state to randomize" << delimiter << lastRandom << endl;
        outputStream << "pixel width" << delimiter << pixelWidth << endl;
        outputStream << "pixel height" << delimiter << pixelHeight << endl;
        outputStream << "cell size" << delimiter << cellSize << endl;
        outputStream << "grid buffer" << delimiter << gridBuffer << endl;
        outputStream << "starting direction" << delimiter << startingDirection << endl;
        outputStream << "starting column" << delimiter << startingColumn << endl;
        outputStream << "starting row" << delimiter << startingRow << endl;
        outputStream << "show counter" << delimiter << showCounter << endl;
        outputStream << "counter location" << delimiter << counterLocation << endl;
        outputStream << "show rules" << delimiter << showRules << endl;
        outputStream << "rules location" << delimiter << rulesLocation << endl;
        outputStream << "counter font" << delimiter << counterFont.toString() << endl;
        outputStream << "show ant color" << delimiter << showAntColor << endl;
        outputStream << "ant color" << delimiter << antColor.red() << "," << antColor.green() << "," << antColor.blue() << endl;
        outputStream << "steps per update" << delimiter << stepsPerUpdate << endl;
        outputStream << "delay per update" << delimiter << delayPerUpdate << endl;
        outputStream << "steps per sample" << delimiter << stepsPerSample << endl;
        outputStream << "samples per frame" << delimiter << samplesPerFrame << endl;
        outputStream << "frame count" << delimiter << frameCount << endl;
        outputStream << "save zero frame" << delimiter << saveZeroFrame << endl;
        outputStream << "search step count" << delimiter << searchSteps << endl;
        outputStream << "include back" << delimiter << includeBack << endl;

        //This code stores the values in the state widgets
        //format: stateNumber=direction=red,green,blue
        //example: 1=right=255,127,220
        for (int i=0; i<stateCount; i++)
        {
            //output the state number
            //This uses the displayed number (starts at 1), not the array index (starts at 0)
            outputStream << i+1 << delimiter;

            //output the state direction
            if (stateArray[i].direction == antRight)
                outputStream << "right" << delimiter;
            if (stateArray[i].direction == antLeft)
                outputStream << "left" << delimiter;
            if (stateArray[i].direction == antBack)
                outputStream << "back" << delimiter;

            //output the state color
            outputStream << stateArray[i].color.red() << ",";
            outputStream << stateArray[i].color.green() << ",";
            outputStream << stateArray[i].color.blue() << endl;
        }


    }

    //The QFile destructor will close the file automatically
}





//This function loads settings from a passed file.  It also needs to access the stateArray so it can
//load values into the states.  See the loadOneSetting definition for why it has to be a pointer to
//a pointer.
void AntSettings::loadFromFile(QString fileName, StateWidget ** stateArrayPointer)
{

    //Open the file
    QFile loadFile(fileName);

    //If the file can't be opened, quit
    if (!loadFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    //Create the input stream and a QString to hold the lines
    QTextStream inputStream(&loadFile);
    QString settingLine;

    //Loop through each line in the file and call a function to save the settings
    while (!inputStream.atEnd())
    {
        settingLine = inputStream.readLine();
        loadOneSetting(&settingLine, stateArrayPointer);
    }

}





//This function interprets a single line of a settings file and makes the appropriate changes to
//either one of the members of the AntSettings object or to StateWidget stored in stateArray.
//I need to pass stateArray as a pointer to a pointer.  This is because an array's address is a
//pointer, but since I am going to be deleting and recreating the stateArray array, I will need to
//change that pointer.  So getting a pointer to a pointer allows this code to change the pointer
//that indicates where the stateArray is stored.
void AntSettings::loadOneSetting(QString * settingLine, StateWidget ** stateArrayPointer)
{

    //if the line has a single equals sign, it is a regular setting (not a state color or direction)
    if ( settingLine->count('=') == 1 )
    {
        QString settingName = settingLine->section("=", 0, 0);
        QString settingValue = settingLine->section("=", 1);

        if (settingName == "number of states")
        {
            stateCount = settingValue.toInt();

            //Delete the existing StateWidget array.
            delete [] *stateArrayPointer;

            //Recreate the StateWidget array to the loaded size
            *stateArrayPointer = new StateWidget [stateCount];

            //Initialise each StateWidget to some default value
            for (int i=0; i<stateCount; i++)
            {
                (*stateArrayPointer)[i].initialize(i+1, antRight, qRgb(0, 0, 0));
            }

        }
        if (settingName == "first state to randomize")
            firstRandom = settingValue.toInt();
        if (settingName == "last state to randomize")
            lastRandom = settingValue.toInt();
        if (settingName == "pixel width")
            pixelWidth = settingValue.toInt();
        if (settingName == "pixel height")
            pixelHeight = settingValue.toInt();
        if (settingName == "cell size")
            cellSize = settingValue.toInt();
        if (settingName == "grid buffer")
            gridBuffer = settingValue.toInt();
        if (settingName == "starting direction")
            startingDirection = settingValue.toInt();
        if (settingName == "starting column")
            startingColumn = settingValue.toInt();
        if (settingName == "starting row")
            startingRow = settingValue.toInt();
        if (settingName == "show counter")
            showCounter = settingValue.toInt();
        if (settingName == "counter location")
            counterLocation = settingValue.toInt();
        if (settingName == "show rules")
            showRules = settingValue.toInt();
        if (settingName == "rules location")
            rulesLocation = settingValue.toInt();
        if (settingName == "counter font")
            counterFont.fromString(settingValue);
        if (settingName == "show ant color")
            showAntColor = settingValue.toInt();
        if (settingName == "ant color")
        {
            QString redString = settingValue.section(",", 0, 0);
            QString greenString = settingValue.section(",", 1, 1);
            QString blueString = settingValue.section(",", 2, 2);
            int red = redString.toInt();
            int green = greenString.toInt();
            int blue = blueString.toInt();
            antColor = QColor(red, green, blue);
        }
        if (settingName == "steps per update")
            stepsPerUpdate = settingValue.toInt();
        if (settingName == "delay per update")
            delayPerUpdate = settingValue.toInt();
        if (settingName == "steps per sample")
            stepsPerSample = settingValue.toInt();
        if (settingName == "samples per frame")
            samplesPerFrame = settingValue.toInt();
        if (settingName == "frame count")
            frameCount = settingValue.toInt();
        if (settingName == "save zero frame")
            saveZeroFrame = settingValue.toInt();
        if (settingName == "search step count")
            searchSteps = settingValue.toInt();
        if (settingName == "include back")
            includeBack = settingValue.toInt();
    }

    //if the line has two equals signs, it is a state setting
    if ( settingLine->count('=') == 2 )
    {
        QString numberString = settingLine->section("=", 0, 0);
        int number = numberString.toInt();

        QString directionString = settingLine->section("=", 1, 1);
        AntDirection direction;
        if (directionString=="right")
            direction = antRight;
        if (directionString=="left")
            direction = antLeft;
        if (directionString=="back")
            direction = antBack;

        QString colorString = settingLine->section("=", 2, 2);
        QString redString = colorString.section(",", 0, 0);
        QString greenString = colorString.section(",", 1, 1);
        QString blueString = colorString.section(",", 2, 2);
        int red = redString.toInt();
        int green = greenString.toInt();
        int blue = blueString.toInt();

        //initialise the state to these values
        (*stateArrayPointer)[number-1].initialize(number, direction, QColor(red, green, blue));

    }

}
