#include "statewidget.h"
#include "ui_statewidget.h"

#include <QColorDialog>

StateWidget::StateWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StateWidget)
{
    ui->setupUi(this);

    //set default values
    ui->groupBox->setTitle("0");
    color = qRgb(0,0,0);
    direction = antRight;
    initialized = false;

    //set up connections
    connect(ui->colorButton, SIGNAL(clicked()), this, SLOT(colorButtonPushed()));
    connect(ui->rightButton, SIGNAL(clicked()), this, SLOT(rightSelected()));
    connect(ui->leftButton, SIGNAL(clicked()), this, SLOT(leftSelected()));
    connect(ui->backButton, SIGNAL(clicked()), this, SLOT(backSelected()));
}




StateWidget::~StateWidget()
{
    delete ui;
}




void StateWidget::initialize(int numberToSet, AntDirection directionToSet, QColor colorToSet)
{
    //set the title of the group box to the number
    ui->groupBox->setTitle(QString::number(numberToSet));

    //set the ant's direction to the passed direction
    direction = directionToSet;

    //set the initialized bool to true
    initialized = true;

    //make the radio buttons reflect the direction
    switch (direction)
    {
    case antRight:
        ui->rightButton->setChecked(true);
        ui->leftButton->setChecked(false);
        ui->backButton->setChecked(false);
        break;
    case antLeft:
        ui->rightButton->setChecked(false);
        ui->leftButton->setChecked(true);
        ui->backButton->setChecked(false);
        break;
    case antBack:
        ui->rightButton->setChecked(false);
        ui->leftButton->setChecked(false);
        ui->backButton->setChecked(true);
        break;
    }

    //Change the state's color and make the button that color
    changeColor(colorToSet);

}





void StateWidget::colorButtonPushed()
{
    QColor chosenColor = QColorDialog::getColor(color, 0, "Choose a State Colour");

    if (chosenColor.isValid()) //if the user clicked OK instead of Cancel
    {
        //Change the state's color and make the button that color
        changeColor(chosenColor);

        //Emit a signal that will make MainWindow redraw the image using the new colors
        emit colorChanged();
    }
}





void StateWidget::rightSelected()
{
    direction = antRight;
    emit directionChanged();
}

void StateWidget::leftSelected()
{
    direction = antLeft;
    emit directionChanged();
}

void StateWidget::backSelected()
{
    direction = antBack;
    emit directionChanged();
}




void StateWidget::changeColor(QColor colorToSet)
{
    //set the state's color to the passed color
    color = colorToSet;

    //make the button reflect the color
    const QString COLOR_STYLE("QPushButton { background-color : %1 }");
    ui->colorButton->setStyleSheet(COLOR_STYLE.arg(color.name()));

}

