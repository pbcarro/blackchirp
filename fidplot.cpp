#include "fidplot.h"
#include <math.h>
#include <qwt6/qwt_plot_canvas.h>
#include <QDebug>
#include <QPalette>
#include <QSettings>
#include <QApplication>
#include <QMenu>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QWidgetAction>
#include <QFormLayout>

FidPlot::FidPlot(QWidget *parent) :
    QwtPlot(parent), d_ftEndAtFidEnd(true)
{
    //make axis label font smaller
    this->setAxisFont(QwtPlot::xBottom,QFont(tr("sans-serif"),8));
    this->setAxisFont(QwtPlot::yLeft,QFont(tr("sans-serif"),8));

    //build axis titles with small font. The <html> etc. tags are needed to display the mu character
    QwtText blabel(tr("<html><body>Time (&mu;s)</body></html>"));
    blabel.setFont(QFont(tr("sans-serif"),8));
    this->setAxisTitle(QwtPlot::xBottom,blabel);

    QwtText llabel(tr("FID"));
    llabel.setFont(QFont(tr("sans-serif"),8));
    this->setAxisTitle(QwtPlot::yLeft,llabel);

    this->setAxisAutoScale(QwtPlot::yLeft,true);
    this->setAxisAutoScale(QwtPlot::xBottom,true);

    this->setAxisScaleDraw(QwtPlot::yLeft,new SciNotationScaleDraw());


    d_curve = new QwtPlotCurve();
//	curveData->setRenderHint(QwtPlotItem::RenderAntialiased,true);
    QPalette pal;
    QPen p;
    QSettings s;
    p.setColor(s.value(tr("fidcolor"),pal.text().color()).value<QColor>());
    p.setWidth(1);
    d_curve->setPen(p);
    d_curve->attach(this);

    QwtPlotMarker *chirpStartMarker = new QwtPlotMarker();
    p.setColor(pal.text().color());
    chirpStartMarker->setLineStyle(QwtPlotMarker::VLine);
    chirpStartMarker->setLinePen(p);
    QwtText csLabel(QString("Chirp Start"));
    csLabel.setFont(QFont(QString("sans serif"),6));
    chirpStartMarker->setLabel(csLabel);
    chirpStartMarker->setLabelOrientation(Qt::Vertical);
    chirpStartMarker->setLabelAlignment(Qt::AlignBottom|Qt::AlignRight);
    d_chirpMarkers.first = chirpStartMarker;

    QwtPlotMarker *chirpEndMarker = new QwtPlotMarker();
    chirpEndMarker->setLineStyle(QwtPlotMarker::VLine);
    chirpEndMarker->setLinePen(p);
    QwtText ceLabel(QString("Chirp End"));
    ceLabel.setFont(QFont(QString("sans serif"),6));
    chirpEndMarker->setLabel(ceLabel);
    chirpEndMarker->setLabelOrientation(Qt::Vertical);
    chirpEndMarker->setLabelAlignment(Qt::AlignTop|Qt::AlignRight);
    d_chirpMarkers.second = chirpEndMarker;

    QwtPlotMarker *ftStartMarker = new QwtPlotMarker();
    ftStartMarker->setLineStyle(QwtPlotMarker::VLine);
    ftStartMarker->setLinePen(p);
    QwtText ftsLabel(QString(" FT Start "));
    ftsLabel.setFont(QFont(QString("sans serif"),6));
    ftsLabel.setBackgroundBrush(QPalette().window());
    ftsLabel.setColor(QPalette().text().color());
    ftStartMarker->setLabel(ftsLabel);
    ftStartMarker->setLabelOrientation(Qt::Vertical);
    ftStartMarker->setLabelAlignment(Qt::AlignBottom|Qt::AlignRight);
    ftStartMarker->setValue(0.0,0.0);
    ftStartMarker->attach(this);
    d_ftMarkers.first = ftStartMarker;

    QwtPlotMarker *ftEndMarker = new QwtPlotMarker();
    ftEndMarker->setLineStyle(QwtPlotMarker::VLine);
    ftEndMarker->setLinePen(p);
    QwtText fteLabel(QString(" FT End "));
    fteLabel.setFont(QFont(QString("sans serif"),6));
    fteLabel.setBackgroundBrush(QPalette().window());
    fteLabel.setColor(QPalette().text().color());
    ftEndMarker->setLabel(fteLabel);
    ftEndMarker->setLabelOrientation(Qt::Vertical);
    ftEndMarker->setLabelAlignment(Qt::AlignTop|Qt::AlignLeft);
    ftEndMarker->setValue(0.0,0.0);
    ftEndMarker->attach(this);
    d_ftMarkers.second = ftEndMarker;

}

void FidPlot::receiveData(const Fid f)
{
    if(f.size() < 2)
        return;

    if(d_ftEndAtFidEnd)
    {
        d_ftMarkers.second->setValue(f.spacing()*f.size()*1e6,0.0);
        d_ftEndAtFidEnd = false; //unset this so that the marker isn't repositioned at every new FID
    }

    d_currentFid = f;
    replot();
}

void FidPlot::filterData()
{
    if(d_currentFid.size() < 2)
        return;

    QVector<QPointF> fidData = d_currentFid.toXY();

    double firstPixel = 0.0;
    double lastPixel = canvas()->width();
    QwtScaleMap map = canvasMap(QwtPlot::xBottom);
//    double scaleMin = map.invTransform(firstPixel);
//    double scaleMax = map.invTransform(lastPixel);

    QVector<QPointF> filtered;

    //find first data point that is in the range of the plot
    int dataIndex = 0;
    while(dataIndex+1 < fidData.size() && map.transform(fidData.at(dataIndex).x()*1e6) < firstPixel)
        dataIndex++;

    //add the previous point to the filtered array
    //this will make sure the curve always goes to the edge of the plot
    if(dataIndex-1 >= 0)
        filtered.append(fidData.at(dataIndex-1));

    //at this point, dataIndex is at the first point within the range of the plot. loop over pixels, compressing data
    for(double pixel = firstPixel; pixel<lastPixel; pixel+=1.0)
    {
        double min = fidData.at(dataIndex).y(), max = min;
        int minIndex = dataIndex, maxIndex = dataIndex;
//        double upperLimit = map.invTransform(pixel+1.0);
        int numPnts = 0;
        while(dataIndex+1 < fidData.size() && map.transform(fidData.at(dataIndex).x()*1e6) < pixel+1.0)
        {
            if(fidData.at(dataIndex).y() < min)
            {
                min = fidData.at(dataIndex).y();
                minIndex = dataIndex;
            }
            if(fidData.at(dataIndex).y() > max)
            {
                max = fidData.at(dataIndex).y();
                maxIndex = dataIndex;
            }
            dataIndex++;
            numPnts++;
        }
        if(numPnts == 1)
            filtered.append(fidData.at(dataIndex-1));
        else if (numPnts > 1)
        {
            QPointF first(map.invTransform(pixel),fidData.at(minIndex).y());
            QPointF second(map.invTransform(pixel),fidData.at(maxIndex).y());
            filtered.append(first);
            filtered.append(second);
        }
    }

    if(dataIndex < fidData.size())
    {
        QPointF p = fidData.at(dataIndex);
        p.setX(p.x()*1e6);
        filtered.append(p);
    }

    //assign data to curve object
    d_curve->setSamples(filtered);
}

void FidPlot::initialize(double chirpStart, double chirpEnd, bool displayMarkers)
{
    if(displayMarkers)
    {
        d_chirpMarkers.first->setValue(chirpStart,0.0);
        d_chirpMarkers.first->attach(this);
        d_chirpMarkers.second->setValue(chirpEnd,0.0);
        d_chirpMarkers.second->attach(this);
    }
    else
    {
        d_chirpMarkers.first->detach();
        d_chirpMarkers.second->detach();
    }

    setFtStart(0.0);
    setFtEnd(0.0);
    d_ftEndAtFidEnd = true;
}

void FidPlot::setFtStart(double start)
{
    if(start < d_ftMarkers.second->value().x() && start >= 0.0)
    {
        d_ftMarkers.first->setValue(start,0.0);
        emit ftStartChanged(start);
    }
    else
        emit overrideStart(d_ftMarkers.first->value().x());

    QwtPlot::replot();
}

void FidPlot::setFtEnd(double end)
{
    if(end > d_ftMarkers.first->value().x() && end <= d_currentFid.spacing()*d_currentFid.size()*1e6)
    {
        d_ftMarkers.second->setValue(end,0.0);
        emit ftEndChanged(end);
    }
    else
        emit overrideEnd(d_ftMarkers.second->value().x());

    QwtPlot::replot();
}

void FidPlot::replot()
{
    if(d_currentFid.size()>0)
    {
        if(fabs(axisScaleDiv(QwtPlot::xBottom).upperBound()-d_currentFid.spacing()*d_currentFid.size()*1e6) > 0.001)
        {
            setAxisScale(QwtPlot::xBottom,0.0,d_currentFid.spacing()*d_currentFid.size()*1e6);
            //x axes need to be updated before data are refiltered
            updateAxes();
            //updateAxes() doesn't reprocess the scale labels, which might need to be recalculated.
            //this line will take care of that and avoid plot glitches
            QApplication::sendPostedEvents(this,QEvent::LayoutRequest);
        }

        filterData();
    }

    QwtPlot::replot();
}

void FidPlot::contextMenuEvent(QContextMenuEvent *ev)
{
    if(d_currentFid.size()<2)
        return;

    QMenu *menu = new QMenu();
    connect(menu,&QMenu::aboutToHide,menu,&QMenu::deleteLater);

    QWidgetAction *wa = new QWidgetAction(menu);
    QWidget *w = new QWidget(menu);
    QFormLayout *fl = new QFormLayout(w);



    QDoubleSpinBox *startBox = new QDoubleSpinBox();
    startBox->setMinimum(0.0);
    startBox->setDecimals(2);
    startBox->setMaximum(d_currentFid.size()*d_currentFid.spacing()*1e6);
    startBox->setSingleStep(0.05);
    startBox->setValue(d_ftMarkers.first->value().x());
    startBox->setKeyboardTracking(false);
    connect(startBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&FidPlot::setFtStart);
    connect(this,&FidPlot::overrideStart,startBox,&QDoubleSpinBox::setValue);
    startBox->setSuffix(QString::fromUtf8(" μs"));
    fl->addRow(QString("FT Start"),startBox);

    QDoubleSpinBox *endBox = new QDoubleSpinBox();
    endBox->setMinimum(0.0);
    endBox->setDecimals(2);
    endBox->setMaximum(d_currentFid.size()*d_currentFid.spacing()*1e6);
    endBox->setSingleStep(0.05);
    endBox->setValue(d_ftMarkers.second->value().x());
    endBox->setKeyboardTracking(false);
    connect(endBox,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,&FidPlot::setFtEnd);
    connect(this,&FidPlot::overrideEnd,endBox,&QDoubleSpinBox::setValue);
    endBox->setSuffix(QString::fromUtf8(" μs"));
    fl->addRow(QString("FT End"),endBox);

    w->setLayout(fl);
    wa->setDefaultWidget(w);
    menu->addAction(wa);

    menu->popup(ev->globalPos());

}

void FidPlot::resizeEvent(QResizeEvent *e)
{
    QwtPlot::resizeEvent(e);

    if(d_currentFid.size()>0)
        replot();
}