/****************************************************************************
**
** Copyright (C) 2014 Digia Plc
** All rights reserved.
** For any questions to Digia, please use contact form at http://qt.digia.com
**
** This file is part of the QtDataVisualization module.
**
** Licensees holding valid Qt Enterprise licenses may use this file in
** accordance with the Qt Enterprise License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.
**
** If you have questions regarding the use of this file, please use
** contact form at http://qt.digia.com
**
****************************************************************************/

#ifndef SCATTERDATAMODIFIER_H
#define SCATTERDATAMODIFIER_H

#include <QtDataVisualization/q3dscatter.h>
#include <QtDataVisualization/qscatter3dseries.h>

#include <QFont>
#include <QDebug>
#include <QTimer>
#include <QLabel>

using namespace QtDataVisualization;

class ScatterDataModifier : public QObject
{
    Q_OBJECT
public:
    explicit ScatterDataModifier(Q3DScatter *scatter);
    ~ScatterDataModifier();

    void addData();
    void changeStyle();
    void changePresetCamera();
    void changeTheme();
    void changeLabelStyle();
    void changeFont(const QFont &font);
    void changeFontSize(int fontSize);
    void changePointSize(int pointSize);
    void setBackgroundEnabled(int enabled);
    void setGridEnabled(int enabled);
    void setMinX(int min);
    void setMinY(int min);
    void setMinZ(int min);
    void setMaxX(int max);
    void setMaxY(int max);
    void setMaxZ(int max);
    void setAspectRatio(int ratio);
    void setHorizontalAspectRatio(int ratio);
    void start();
    void massiveDataTest();
    void massiveTestScroll();
    void massiveTestAppendAndScroll();
    void setFpsMeasurement(bool enable);
    void setFpsLabel(QLabel *fpsLabel) { m_fpsLabel = fpsLabel; }
    void testItemChanges();
    void testAxisReverse();

public slots:
    void changeShadowQuality(int quality);
    void shadowQualityUpdatedByVisual(QAbstract3DGraph::ShadowQuality shadowQuality);
    void clear();
    void resetAxes();
    void addOne();
    void addBunch();
    void insertOne();
    void insertBunch();
    void changeOne();
    void changeBunch();
    void removeOne();
    void removeBunch();
    void timeout();
    void startStopTimer();
    void selectItem();
    void handleSelectionChange(int index);
    void setGradient();
    void clearSeriesData();
    void addSeries();
    void removeSeries();
    void toggleSeriesVisibility();
    void changeSeriesName();
    void handleAxisXChanged(QValue3DAxis *axis);
    void handleAxisYChanged(QValue3DAxis *axis);
    void handleAxisZChanged(QValue3DAxis *axis);
    void handleFpsChange(qreal fps);
    void changeLabelRotation(int rotation);
    void changeRadialLabelOffset(int offset);
    void toggleAxisTitleVisibility(bool enabled);
    void toggleAxisTitleFixed(bool enabled);
    void renderToImage();
    void togglePolar(bool enable);
    void toggleStatic(bool enable);
    void toggleOrtho(bool enable);
    void setCameraTargetX(int value);
    void setCameraTargetY(int value);
    void setCameraTargetZ(int value);
    void setGraphMargin(int value);

signals:
    void shadowQualityChanged(int quality);

private:
    QVector3D randVector();
    QScatter3DSeries *createAndAddSeries();
    void populateFlatSeries(QScatter3DSeries *series, int rows, int columns, float value);
    void populateRisingSeries(QScatter3DSeries *series, int rows, int columns, float minValue,
                              float maxValue);
    void deleteAxis(QValue3DAxis *axis);

    Q3DScatter *m_chart;
    int m_fontSize;
    QTimer m_timer;
    int m_loopCounter;
    int m_selectedItem;
    QScatter3DSeries *m_targetSeries;
    QScatterDataArray m_massiveTestCacheArray;
    QLabel *m_fpsLabel;
    QVector3D m_cameraTarget;

};

#endif
