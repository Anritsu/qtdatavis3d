/****************************************************************************
**
** Copyright (C) 2013 Digia Plc
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef SURFACE3DCONTROLLER_P_H
#define SURFACE3DCONTROLLER_P_H

#include "abstract3dcontroller_p.h"
#include "datavisualizationglobal_p.h"

#include <QLinearGradient>

QT_DATAVISUALIZATION_BEGIN_NAMESPACE

class Surface3DRenderer;
class QSurface3DSeries;

struct Surface3DChangeBitField {
    bool gradientColorChanged     : 1;
    bool smoothStatusChanged      : 1;
    bool surfaceGridChanged       : 1;
    bool selectedPointChanged     : 1;

    Surface3DChangeBitField() :
        gradientColorChanged(true),
        smoothStatusChanged(true),
        surfaceGridChanged(true),
        selectedPointChanged(true)
    {
    }
};

class QT_DATAVISUALIZATION_EXPORT Surface3DController : public Abstract3DController
{
    Q_OBJECT

private:
    Surface3DChangeBitField m_changeTracker;
    Surface3DRenderer *m_renderer;
    bool m_isSmoothSurfaceEnabled;
    bool m_isSurfaceEnabled;
    bool m_isSurfaceGridEnabled;
    QLinearGradient m_userDefinedGradient;
    QPoint m_selectedPoint;
    QSurface3DSeries *m_selectedSeries; // Points to the series for which the point is selected in
                                        // single series selection cases.

public:
    explicit Surface3DController(QRect rect);
    ~Surface3DController();

    void initializeOpenGL();
    virtual void synchDataToRenderer();

    void setSmoothSurface(bool enable);
    bool smoothSurface();

    void setSurfaceGrid(bool enable);
    bool surfaceGrid();

    void setGradient(const QLinearGradient &gradient);
    QLinearGradient gradient() const;

    void setGradientColorAt(qreal pos, const QColor &color);

    void setSelectionMode(QDataVis::SelectionFlags mode);
    void setSelectedPoint(const QPoint &position, QSurface3DSeries *series);

    virtual void handleAxisAutoAdjustRangeChangedInOrientation(Q3DAbstractAxis::AxisOrientation orientation, bool autoAdjust);
    virtual void handleAxisRangeChangedBySender(QObject *sender);
    virtual void handleSeriesVisibilityChangedBySender(QObject *sender);

    static QPoint invalidSelectionPosition();

    virtual void addSeries(QAbstract3DSeries *series);
    virtual void removeSeries(QAbstract3DSeries *series);
    virtual QList<QSurface3DSeries *> surfaceSeriesList();

public slots:
    void handleArrayReset();

    // Renderer callback handlers
    void handlePointClicked(const QPoint &position, QSurface3DSeries *series);
    void handleRequestSmoothSurface(bool enable);

signals:
    void smoothSurfaceEnabledChanged(bool enable);
    void surfaceVisibleChanged(bool visible);
    void surfaceGridEnabledChanged(bool enable);

private:
    void adjustValueAxisRange();

    Q_DISABLE_COPY(Surface3DController)
};

QT_DATAVISUALIZATION_END_NAMESPACE

#endif // SURFACE3DCONTROLLER_P_H
