#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include <QtWidgets/QWidget>
#include <QtGui>
#include "antdirection.h"

namespace Ui {
class StateWidget;
}



class StateWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit StateWidget(QWidget *parent = 0);
    ~StateWidget();

    QColor color;
    AntDirection direction;
    bool initialized;

    void initialize(int numberToSet, AntDirection directionToSet, QColor colorToSet);
    void changeColor(QColor colorToSet);
    
private slots:
    void colorButtonPushed();
    void rightSelected();
    void leftSelected();
    void backSelected();

signals:
    void colorChanged();
    void directionChanged();

private:
    Ui::StateWidget *ui;
};

#endif // STATEWIDGET_H
