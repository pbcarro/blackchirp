#include "motorzplot.h"

#include <QMenu>
#include <QMouseEvent>

MotorZPlot::MotorZPlot(QWidget *parent) : MotorSpectrogramPlot(parent)
{
    setName(QString("motorZPlot"));
    setAxis(QwtPlot::yLeft,BlackChirp::MotorY);
    setAxis(QwtPlot::xBottom,BlackChirp::MotorZ);
}

void MotorZPlot::buildContextMenu(QMouseEvent *me)
{
    QMenu *m = contextMenu();

    m->popup(me->globalPos());
}
