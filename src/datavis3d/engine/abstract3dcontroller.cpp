/****************************************************************************
**
** Copyright (C) 2013 Digia Plc
** All rights reserved.
** For any questions to Digia, please use contact form at http://qt.digia.com
**
** This file is part of the QtDataVis3D module.
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

#include "abstract3dcontroller_p.h"
#include "camerahelper_p.h"
#include "qabstractaxis_p.h"
#include "qvalueaxis.h"
#include "qcategoryaxis.h"
#include "abstract3drenderer_p.h"
#include "qabstractdataproxy_p.h"

#if defined(Q_OS_ANDROID)
#include "qtouch3dinputhandler.h"
#else
#include "q3dinputhandler.h"
#endif


QT_DATAVIS3D_BEGIN_NAMESPACE

Abstract3DController::Abstract3DController(QRect boundRect, QObject *parent) :
    QObject(parent),
    m_boundingRect(boundRect.x(), boundRect.y(), boundRect.width(), boundRect.height()),
    m_horizontalRotation(-45.0f),
    m_verticalRotation(15.0f),
    m_theme(),
    m_font(QFont(QStringLiteral("Arial"))),
    m_selectionMode(QDataVis::ModeItem),
    m_shadowQuality(QDataVis::ShadowMedium),
    m_labelTransparency(QDataVis::TransparencyNoBackground),
    m_isBackgroundEnabled(true),
    m_isGridEnabled(true),
    m_cameraHelper(new CameraHelper()),
    m_zoomLevel(100),
    m_axisX(0),
    m_axisY(0),
    m_axisZ(0),
    m_renderer(0),
    m_isDataDirty(true),
    m_data(0)
{
    m_theme.useColorTheme(QDataVis::ThemeSystem);
#if defined(Q_OS_ANDROID)
    m_inputHandler = new QTouch3DInputHandler();
#else
    m_inputHandler = new Q3DInputHandler();
#endif
    m_inputHandler->setCamera(m_cameraHelper);
}

Abstract3DController::~Abstract3DController()
{
    delete m_cameraHelper;

    // Attached axes are children, so no need to explicitly delete them
}

void Abstract3DController::setRenderer(Abstract3DRenderer *renderer)
{
    m_renderer = renderer;
}

void Abstract3DController::synchDataToRenderer()
{
    // If we don't have a renderer, don't do anything
    if (!m_renderer)
        return;

    if (m_changeTracker.positionChanged) {
        m_renderer->updatePosition(m_boundingRect);
        m_changeTracker.positionChanged = false;
    }

    if (m_changeTracker.zoomLevelChanged) {
        m_renderer->updateZoomLevel(m_zoomLevel);
        m_changeTracker.zoomLevelChanged = false;
    }

    if (m_changeTracker.themeChanged) {
        m_renderer->updateTheme(m_theme);
        m_changeTracker.themeChanged = false;
    }

    if (m_changeTracker.fontChanged) {
        m_renderer->updateFont(m_font);
        m_changeTracker.fontChanged = false;
    }

    if (m_changeTracker.labelTransparencyChanged) {
        m_renderer->updateLabelTransparency(m_labelTransparency);
        m_changeTracker.labelTransparencyChanged = false;
    }

    if (m_changeTracker.boundingRectChanged || m_changeTracker.sizeChanged) {
        m_renderer->updateBoundingRect(m_boundingRect);
        m_changeTracker.boundingRectChanged = false;
        m_changeTracker.sizeChanged = false;
    }

    if (m_changeTracker.shadowQualityChanged) {
        m_renderer->updateShadowQuality(m_shadowQuality);
        m_changeTracker.shadowQualityChanged = false;
    }

    if (m_changeTracker.selectionModeChanged) {
        m_renderer->updateSelectionMode(m_selectionMode);
        m_changeTracker.selectionModeChanged = false;
    }

    if (m_changeTracker.objFileChanged) {
        m_renderer->updateMeshFileName(m_objFile);
        m_changeTracker.objFileChanged = false;
    }

    if (m_changeTracker.gridEnabledChanged) {
        m_renderer->updateGridEnabled(m_isGridEnabled);
        m_changeTracker.gridEnabledChanged = false;
    }

    if (m_changeTracker.backgroundEnabledChanged) {
        m_renderer->updateBackgroundEnabled(m_isBackgroundEnabled);
        m_changeTracker.backgroundEnabledChanged = false;
    }

    if (m_changeTracker.axisXTypeChanged) {
        m_renderer->updateAxisType(QAbstractAxis::AxisOrientationX, m_axisX->type());
        m_changeTracker.axisXTypeChanged = false;
    }

    if (m_changeTracker.axisYTypeChanged) {
        m_renderer->updateAxisType(QAbstractAxis::AxisOrientationY, m_axisY->type());
        m_changeTracker.axisYTypeChanged = false;
    }

    if (m_changeTracker.axisZTypeChanged) {
        m_renderer->updateAxisType(QAbstractAxis::AxisOrientationZ, m_axisZ->type());
        m_changeTracker.axisZTypeChanged = false;
    }

    if (m_changeTracker.axisXTitleChanged) {
        m_renderer->updateAxisTitle(QAbstractAxis::AxisOrientationX, m_axisX->title());
        m_changeTracker.axisXTitleChanged = false;
    }

    if (m_changeTracker.axisYTitleChanged) {
        m_renderer->updateAxisTitle(QAbstractAxis::AxisOrientationY, m_axisY->title());
        m_changeTracker.axisYTitleChanged = false;
    }

    if (m_changeTracker.axisZTitleChanged) {
        m_renderer->updateAxisTitle(QAbstractAxis::AxisOrientationZ, m_axisZ->title());
        m_changeTracker.axisZTitleChanged = false;
    }

    if (m_changeTracker.axisXLabelsChanged) {
        m_renderer->updateAxisLabels(QAbstractAxis::AxisOrientationX, m_axisX->labels());
        m_changeTracker.axisXLabelsChanged = false;
    }

    if (m_changeTracker.axisYLabelsChanged) {
        m_renderer->updateAxisLabels(QAbstractAxis::AxisOrientationY, m_axisY->labels());
        m_changeTracker.axisYLabelsChanged = false;
    }
    if (m_changeTracker.axisZLabelsChanged) {
        m_renderer->updateAxisLabels(QAbstractAxis::AxisOrientationZ, m_axisZ->labels());
        m_changeTracker.axisZLabelsChanged = false;
    }

    if (m_changeTracker.axisXRangeChanged) {
        m_changeTracker.axisXRangeChanged = false;
        if (m_axisX->type() & QAbstractAxis::AxisTypeValue) {
            QValueAxis *valueAxisX = static_cast<QValueAxis *>(m_axisX);
            m_renderer->updateAxisRange(QAbstractAxis::AxisOrientationX,
                                        valueAxisX->min(), valueAxisX->max());
        }
    }

    if (m_changeTracker.axisYRangeChanged) {
        m_changeTracker.axisYRangeChanged = false;
        if (m_axisY->type() & QAbstractAxis::AxisTypeValue) {
            QValueAxis *valueAxisY = static_cast<QValueAxis *>(m_axisY);
            m_renderer->updateAxisRange(QAbstractAxis::AxisOrientationY,
                                        valueAxisY->min(), valueAxisY->max());
        }
    }

    if (m_changeTracker.axisZRangeChanged) {
        m_changeTracker.axisZRangeChanged = false;
        if (m_axisZ->type() & QAbstractAxis::AxisTypeValue) {
            QValueAxis *valueAxisZ = static_cast<QValueAxis *>(m_axisZ);
            m_renderer->updateAxisRange(QAbstractAxis::AxisOrientationZ,
                                        valueAxisZ->min(), valueAxisZ->max());
        }
    }

    if (m_changeTracker.axisXSegmentCountChanged) {
        m_changeTracker.axisXSegmentCountChanged = false;
        if (m_axisX->type() & QAbstractAxis::AxisTypeValue) {
            QValueAxis *valueAxisX = static_cast<QValueAxis *>(m_axisX);
            m_renderer->updateAxisSegmentCount(QAbstractAxis::AxisOrientationX,
                                               valueAxisX->segmentCount());
        }
    }

    if (m_changeTracker.axisYSegmentCountChanged) {
        m_changeTracker.axisYSegmentCountChanged = false;
        if (m_axisY->type() & QAbstractAxis::AxisTypeValue) {
            QValueAxis *valueAxisY = static_cast<QValueAxis *>(m_axisY);
            m_renderer->updateAxisSegmentCount(QAbstractAxis::AxisOrientationY,
                                               valueAxisY->segmentCount());
        }
    }

    if (m_changeTracker.axisZSegmentCountChanged) {
        m_changeTracker.axisZSegmentCountChanged = false;
        if (m_axisZ->type() & QAbstractAxis::AxisTypeValue) {
            QValueAxis *valueAxisZ = static_cast<QValueAxis *>(m_axisZ);
            m_renderer->updateAxisSegmentCount(QAbstractAxis::AxisOrientationZ,
                                               valueAxisZ->segmentCount());
        }
    }

    if (m_changeTracker.axisXSubSegmentCountChanged) {
        m_changeTracker.axisXSubSegmentCountChanged = false;
        if (m_axisX->type() & QAbstractAxis::AxisTypeValue) {
            QValueAxis *valueAxisX = static_cast<QValueAxis *>(m_axisX);
            m_renderer->updateAxisSubSegmentCount(QAbstractAxis::AxisOrientationX,
                                                  valueAxisX->subSegmentCount());
        }
    }

    if (m_changeTracker.axisYSubSegmentCountChanged) {
        m_changeTracker.axisYSubSegmentCountChanged = false;
        if (m_axisY->type() & QAbstractAxis::AxisTypeValue) {
            QValueAxis *valueAxisY = static_cast<QValueAxis *>(m_axisY);
            m_renderer->updateAxisSubSegmentCount(QAbstractAxis::AxisOrientationY,
                                                  valueAxisY->subSegmentCount());
        }
    }

    if (m_changeTracker.axisZSubSegmentCountChanged) {
        m_changeTracker.axisZSubSegmentCountChanged = false;
        if (m_axisZ->type() & QAbstractAxis::AxisTypeValue) {
            QValueAxis *valueAxisZ = static_cast<QValueAxis *>(m_axisZ);
            m_renderer->updateAxisSubSegmentCount(QAbstractAxis::AxisOrientationZ,
                                                  valueAxisZ->subSegmentCount());
        }
    }

    if (m_changeTracker.axisXLabelFormatChanged) {
        m_changeTracker.axisXLabelFormatChanged = false;
        if (m_axisX->type() & QAbstractAxis::AxisTypeValue) {
            QValueAxis *valueAxisX = static_cast<QValueAxis *>(m_axisX);
            m_renderer->updateAxisLabelFormat(QAbstractAxis::AxisOrientationX,
                                              valueAxisX->labelFormat());
        }
    }

    if (m_changeTracker.axisYLabelFormatChanged) {
        m_changeTracker.axisYLabelFormatChanged = false;
        if (m_axisY->type() & QAbstractAxis::AxisTypeValue) {
            QValueAxis *valueAxisY = static_cast<QValueAxis *>(m_axisY);
            m_renderer->updateAxisLabelFormat(QAbstractAxis::AxisOrientationY,
                                              valueAxisY->labelFormat());
        }
    }

    if (m_changeTracker.axisZLabelFormatChanged) {
        m_changeTracker.axisZLabelFormatChanged = false;
        if (m_axisZ->type() & QAbstractAxis::AxisTypeValue) {
            QValueAxis *valueAxisZ = static_cast<QValueAxis *>(m_axisZ);
            m_renderer->updateAxisLabelFormat(QAbstractAxis::AxisOrientationZ,
                                              valueAxisZ->labelFormat());
        }
    }
}

void Abstract3DController::render(const GLuint defaultFboHandle)
{
    // If not initialized, do nothing.
    if (!m_renderer)
        return;

    m_renderer->render(m_cameraHelper, defaultFboHandle);
}

void Abstract3DController::setSize(const int width, const int height)
{
    m_boundingRect.setWidth(width);
    m_boundingRect.setHeight(height);

    m_changeTracker.boundingRectChanged = true;
    emit boundingRectChanged(m_boundingRect);
}

const QSize Abstract3DController::size()
{
    return m_boundingRect.size();
}

const QRect Abstract3DController::boundingRect()
{
    return m_boundingRect;
}

void Abstract3DController::setBoundingRect(const QRect boundingRect)
{
    m_boundingRect = boundingRect;

    m_changeTracker.boundingRectChanged = true;
    emit boundingRectChanged(m_boundingRect);
}

void Abstract3DController::setWidth(const int width)
{
    m_boundingRect.setWidth(width);

    m_changeTracker.sizeChanged = true;
    emit sizeChanged(m_boundingRect);
}

int Abstract3DController::width()
{
    return m_boundingRect.width();
}

void Abstract3DController::setHeight(const int height)
{
    m_boundingRect.setHeight(height);

    m_changeTracker.sizeChanged = true;
    emit sizeChanged(m_boundingRect);
}

int Abstract3DController::height()
{
    return m_boundingRect.height();
}

void Abstract3DController::setX(const int x)
{
    m_boundingRect.setX(x);

    m_changeTracker.positionChanged = true;
    emit positionChanged(m_boundingRect);
}

int Abstract3DController::x()
{
    return m_boundingRect.x();
}

void Abstract3DController::setY(const int y)
{
    m_boundingRect.setY(y);

    m_changeTracker.positionChanged = true;
    emit positionChanged(m_boundingRect);
}

int Abstract3DController::y()
{
    return m_boundingRect.y();
}

void Abstract3DController::setAxisX(QAbstractAxis *axis)
{
    setAxisHelper(QAbstractAxis::AxisOrientationX, axis, &m_axisX);
}

QAbstractAxis *Abstract3DController::axisX()
{
    return m_axisX;
}

void Abstract3DController::setAxisY(QAbstractAxis *axis)
{
    setAxisHelper(QAbstractAxis::AxisOrientationY, axis, &m_axisY);
}

QAbstractAxis *Abstract3DController::axisY()
{
    return m_axisY;
}

void Abstract3DController::setAxisZ(QAbstractAxis *axis)
{
    setAxisHelper(QAbstractAxis::AxisOrientationZ, axis, &m_axisZ);
}

QAbstractAxis *Abstract3DController::axisZ()
{
    return m_axisZ;
}

void Abstract3DController::addAxis(QAbstractAxis *axis)
{
    Q_ASSERT(axis);
    Abstract3DController *owner = qobject_cast<Abstract3DController *>(axis->parent());
    if (owner != this) {
        Q_ASSERT_X(!owner, "addAxis", "Axis already attached to a graph.");
        axis->setParent(this);
    }
    if (!m_axes.contains(axis))
        m_axes.append(axis);
}

void Abstract3DController::releaseAxis(QAbstractAxis *axis)
{
    if (axis && m_axes.contains(axis)) {
        // Clear the default status from released default axes
        if (axis->d_ptr->isDefaultAxis())
            axis->d_ptr->setDefaultAxis(false);

        // If the axis is in use, replace it with a temporary one
        switch (axis->orientation()) {
        case QAbstractAxis::AxisOrientationX:
            setAxisX(0);
            break;
        case QAbstractAxis::AxisOrientationY:
            setAxisY(0);
            break;
        case QAbstractAxis::AxisOrientationZ:
            setAxisZ(0);
            break;
        default:
            break;
        }

        m_axes.removeAll(axis);
        axis->setParent(0);
    }
}

QList<QAbstractAxis *> Abstract3DController::axes() const
{
    return m_axes;
}

QAbstractDataProxy *Abstract3DController::activeDataProxy() const
{
    return m_data;
}

void Abstract3DController::addDataProxy(QAbstractDataProxy *proxy)
{
    Q_ASSERT(proxy);
    Abstract3DController *owner = qobject_cast<Abstract3DController *>(proxy->parent());
    if (owner != this) {
        Q_ASSERT_X(!owner, "addDataProxy", "Proxy already attached to a graph.");
        proxy->setParent(this);
    }
    if (!m_dataProxies.contains(proxy))
        m_dataProxies.append(proxy);
}

void Abstract3DController::releaseDataProxy(QAbstractDataProxy *proxy)
{
    if (proxy && m_dataProxies.contains(proxy)) {
        // Clear the default status from released default proxies
        if (proxy->d_ptr->isDefaultProxy())
            proxy->d_ptr->setDefaultProxy(false);

        // If the proxy is in use, replace it with a temporary one
        if (m_data == proxy)
            setActiveDataProxy(0);

        m_dataProxies.removeAll(proxy);
        proxy->setParent(0);
    }
}

QList<QAbstractDataProxy *> Abstract3DController::dataProxies() const
{
    return m_dataProxies;
}

void Abstract3DController::setActiveDataProxy(QAbstractDataProxy *proxy)
{
    // If existing proxy is the default proxy, delete it
    if (m_data) {
        if (m_data->d_ptr->isDefaultProxy()) {
            m_dataProxies.removeAll(m_data);
            delete m_data;
        } else {
            // Disconnect the old proxy from use
            QObject::disconnect(m_data, 0, this, 0);
        }
    }

    // Assume ownership and activate
    addDataProxy(proxy);
    m_data = proxy;
}

int Abstract3DController::zoomLevel()
{
    return m_zoomLevel;
}

void Abstract3DController::setZoomLevel(int zoomLevel)
{
    m_zoomLevel = zoomLevel;

    m_changeTracker.zoomLevelChanged = true;
    emit zoomLevelChanged(zoomLevel);
}

void Abstract3DController::setCameraPreset(QDataVis::CameraPreset preset)
{
    m_cameraHelper->setCameraPreset(preset);
}

void Abstract3DController::setCameraPosition(GLfloat horizontal, GLfloat vertical, GLint distance)
{
    m_horizontalRotation = qBound(-180.0f, horizontal, 180.0f);
    m_verticalRotation = qBound(0.0f, vertical, 90.0f);
    m_zoomLevel = qBound(10, distance, 500);
    m_cameraHelper->setCameraRotation(QPointF(m_horizontalRotation,
                                              m_verticalRotation));
    //qDebug() << "camera rotation set to" << m_horizontalRotation << m_verticalRotation;
}

void Abstract3DController::setObjectColor(const QColor &baseColor, const QColor &heightColor,
                                          const QColor &depthColor, bool uniform)
{
    m_theme.m_baseColor = baseColor;
    m_theme.m_heightColor = heightColor;
    m_theme.m_depthColor = depthColor;
    m_theme.m_uniformColor = uniform;

    m_changeTracker.themeChanged = true;
    emit themeChanged(m_theme);
}

void Abstract3DController::setColorTheme(QDataVis::ColorTheme colorTheme)
{
    m_theme.useColorTheme(colorTheme);

    m_changeTracker.themeChanged = true;
    emit themeChanged(m_theme);
}

Theme Abstract3DController::theme()
{
    return m_theme;
}

void Abstract3DController::setFont(const QFont &font)
{
    m_font = font;

    m_changeTracker.fontChanged = true;
    emit fontChanged(m_font);
}

QFont Abstract3DController::font()
{
    return m_font;
}

void Abstract3DController::setSelectionMode(QDataVis::SelectionMode mode)
{
    m_selectionMode = mode;
    m_changeTracker.selectionModeChanged = true;
    emit selectionModeChanged(m_selectionMode);
}

QDataVis::SelectionMode Abstract3DController::selectionMode()
{
    return m_selectionMode;
}

void Abstract3DController::setShadowQuality(QDataVis::ShadowQuality quality)
{
    m_shadowQuality = quality;

    m_changeTracker.shadowQualityChanged = true;
    emit shadowQualityChanged(m_shadowQuality);
}

QDataVis::ShadowQuality Abstract3DController::shadowQuality()
{
    return m_shadowQuality;
}

void Abstract3DController::setLabelTransparency(QDataVis::LabelTransparency transparency)
{
    m_labelTransparency = transparency;

    m_changeTracker.labelTransparencyChanged = true;
    emit labelTransparencyChanged(m_labelTransparency);
}

QDataVis::LabelTransparency Abstract3DController::labelTransparency()
{
    return m_labelTransparency;
}

void Abstract3DController::setBackgroundEnabled(bool enable)
{
    m_isBackgroundEnabled = enable;
    m_changeTracker.backgroundEnabledChanged = true;
    emit backgroundEnabledChanged(m_isBackgroundEnabled);
}

bool Abstract3DController::backgroundEnabled()
{
    return m_isBackgroundEnabled;
}

void Abstract3DController::setGridEnabled(bool enable)
{
    m_isGridEnabled = enable;
    m_changeTracker.gridEnabledChanged = true;
    emit gridEnabledChanged(m_isGridEnabled);
}

bool Abstract3DController::gridEnabled()
{
    return m_isGridEnabled;
}

void Abstract3DController::setMeshFileName(const QString &fileName)
{
    m_objFile = fileName;
    m_changeTracker.objFileChanged = true;
    emit meshFileNameChanged(m_objFile);
}

QString Abstract3DController::meshFileName()
{
    return m_objFile;
}

void Abstract3DController::handleAxisTitleChanged(const QString &title)
{
    Q_UNUSED(title)
    handleAxisTitleChangedBySender(sender());
}

void Abstract3DController::handleAxisTitleChangedBySender(QObject *sender)
{
    if (sender == m_axisX)
        m_changeTracker.axisXTitleChanged = true;
    else if (sender == m_axisY)
        m_changeTracker.axisYTitleChanged = true;
    else if (sender == m_axisZ)
        m_changeTracker.axisZTitleChanged = true;
    else
        qWarning() << __FUNCTION__ << "invoked for invalid axis";
}

void Abstract3DController::handleAxisLabelsChanged()
{
    handleAxisLabelsChangedBySender(sender());
}

void Abstract3DController::handleAxisLabelsChangedBySender(QObject *sender)
{
    if (sender == m_axisX)
        m_changeTracker.axisXLabelsChanged = true;
    else if (sender == m_axisY)
        m_changeTracker.axisYLabelsChanged = true;
    else if (sender == m_axisZ)
        m_changeTracker.axisZLabelsChanged = true;
    else
        qWarning() << __FUNCTION__ << "invoked for invalid axis";
}

void Abstract3DController::handleAxisRangeChanged(qreal min, qreal max)
{
    Q_UNUSED(min)
    Q_UNUSED(max)
    handleAxisRangeChangedBySender(sender());
}

void Abstract3DController::handleAxisRangeChangedBySender(QObject *sender)
{
    if (sender == m_axisX) {
        m_isDataDirty = true;
        m_changeTracker.axisXRangeChanged = true;
    } else if (sender == m_axisY) {
        m_isDataDirty = true;
        m_changeTracker.axisYRangeChanged = true;
    } else if (sender == m_axisZ) {
        m_isDataDirty = true;
        m_changeTracker.axisZRangeChanged = true;
    } else {
        qWarning() << __FUNCTION__ << "invoked for invalid axis";
    }
}

void Abstract3DController::handleAxisSegmentCountChanged(int count)
{
    Q_UNUSED(count)
    handleAxisSegmentCountChangedBySender(sender());
}

void Abstract3DController::handleAxisSegmentCountChangedBySender(QObject *sender)
{
    if (sender == m_axisX)
        m_changeTracker.axisXSegmentCountChanged = true;
    else if (sender == m_axisY)
        m_changeTracker.axisYSegmentCountChanged = true;
    else if (sender == m_axisZ)
        m_changeTracker.axisZSegmentCountChanged = true;
    else
        qWarning() << __FUNCTION__ << "invoked for invalid axis";
}

void Abstract3DController::handleAxisSubSegmentCountChanged(int count)
{
    Q_UNUSED(count)
    handleAxisSubSegmentCountChangedBySender(sender());
}

void Abstract3DController::handleAxisSubSegmentCountChangedBySender(QObject *sender)
{
    if (sender == m_axisX)
        m_changeTracker.axisXSubSegmentCountChanged = true;
    else if (sender == m_axisY)
        m_changeTracker.axisYSubSegmentCountChanged = true;
    else if (sender == m_axisZ)
        m_changeTracker.axisZSubSegmentCountChanged = true;
    else
        qWarning() << __FUNCTION__ << "invoked for invalid axis";
}

void Abstract3DController::handleAxisAutoAdjustRangeChanged(bool autoAdjust)
{
    QObject *sender = QObject::sender();
    if (sender != m_axisX && sender != m_axisY && sender != m_axisZ)
        return;

    QAbstractAxis *axis = static_cast<QAbstractAxis*>(sender);
    handleAxisAutoAdjustRangeChangedInOrientation(axis->orientation(), autoAdjust);
}

void Abstract3DController::handleAxisLabelFormatChanged(const QString &format)
{
    Q_UNUSED(format)
    handleAxisLabelFormatChangedBySender(sender());
}

void Abstract3DController::handleAxisLabelFormatChangedBySender(QObject *sender)
{
    // Label format changing needs to dirty the data so that labels are reset.
    if (sender == m_axisX) {
        m_isDataDirty = true;
        m_changeTracker.axisXLabelFormatChanged = true;
    } else if (sender == m_axisY) {
        m_isDataDirty = true;
        m_changeTracker.axisYLabelFormatChanged = true;
    } else if (sender == m_axisZ) {
        m_isDataDirty = true;
        m_changeTracker.axisZLabelFormatChanged = true;
    } else {
        qWarning() << __FUNCTION__ << "invoked for invalid axis";
    }
}

void Abstract3DController::setAxisHelper(QAbstractAxis::AxisOrientation orientation,
                                         QAbstractAxis *axis, QAbstractAxis **axisPtr)
{
    // Setting null axis indicates using default axis
    if (!axis)
        axis = createDefaultAxis(orientation);

    // If old axis is default axis, delete it
    QAbstractAxis *oldAxis = *axisPtr;
    if (oldAxis) {
        if (oldAxis->d_ptr->isDefaultAxis()) {
            m_axes.removeAll(oldAxis);
            delete oldAxis;
            oldAxis = 0;
        } else {
            // Disconnect the old axis from use
            QObject::disconnect(oldAxis, 0, this, 0);
            oldAxis->d_ptr->setOrientation(QAbstractAxis::AxisOrientationNone);
        }
    }

    // Assume ownership
    addAxis(axis);

    // Connect the new axis
    *axisPtr = axis;

    axis->d_ptr->setOrientation(orientation);

    QObject::connect(axis, &QAbstractAxis::titleChanged,
                     this, &Abstract3DController::handleAxisTitleChanged);
    QObject::connect(axis, &QAbstractAxis::labelsChanged,
                     this, &Abstract3DController::handleAxisLabelsChanged);

    if (orientation == QAbstractAxis::AxisOrientationX)
        m_changeTracker.axisXTypeChanged = true;
    else if (orientation == QAbstractAxis::AxisOrientationY)
        m_changeTracker.axisYTypeChanged = true;
    else if (orientation == QAbstractAxis::AxisOrientationZ)
        m_changeTracker.axisZTypeChanged = true;

    handleAxisTitleChangedBySender(axis);
    handleAxisLabelsChangedBySender(axis);

    if (axis->type() & QAbstractAxis::AxisTypeValue) {
        QValueAxis *valueAxis = static_cast<QValueAxis *>(axis);
        QObject::connect(valueAxis, &QValueAxis::rangeChanged,
                         this, &Abstract3DController::handleAxisRangeChanged);
        QObject::connect(valueAxis, &QValueAxis::segmentCountChanged,
                         this, &Abstract3DController::handleAxisSegmentCountChanged);
        QObject::connect(valueAxis, &QValueAxis::subSegmentCountChanged,
                         this, &Abstract3DController::handleAxisSubSegmentCountChanged);
        QObject::connect(valueAxis, &QValueAxis::autoAdjustRangeChanged,
                         this, &Abstract3DController::handleAxisAutoAdjustRangeChanged);
        QObject::connect(valueAxis, &QValueAxis::labelFormatChanged,
                         this, &Abstract3DController::handleAxisLabelFormatChanged);

        handleAxisRangeChangedBySender(valueAxis);
        handleAxisSegmentCountChangedBySender(valueAxis);
        handleAxisSubSegmentCountChangedBySender(valueAxis);
        handleAxisAutoAdjustRangeChangedInOrientation(valueAxis->orientation(),
                                                      valueAxis->isAutoAdjustRange());
        handleAxisLabelFormatChangedBySender(valueAxis);
    }
}

QAbstractAxis *Abstract3DController::createDefaultAxis(QAbstractAxis::AxisOrientation orientation)
{
    Q_UNUSED(orientation)

    // The default default axis is a value axis. If the chart type has a different default axis
    // for some orientation, this function needs to be overridden.
    QAbstractAxis *defaultAxis = createDefaultValueAxis();
    return defaultAxis;
}

QValueAxis *Abstract3DController::createDefaultValueAxis()
{
    // Default value axis has single segment, empty label format, and auto scaling
    // TODO: Grid should be also hidden, but that is not currently controlled by axis
    QValueAxis *defaultAxis = new QValueAxis;
    defaultAxis->setSegmentCount(1);
    defaultAxis->setSubSegmentCount(1);
    defaultAxis->setAutoAdjustRange(true);
    defaultAxis->setLabelFormat(QString());
    defaultAxis->d_ptr->setDefaultAxis(true);

    return defaultAxis;
}

QCategoryAxis *Abstract3DController::createDefaultCategoryAxis()
{
    // Default category axis has no labels
    // TODO: Grid should be also hidden, but that is not currently controlled by axis.
    QCategoryAxis *defaultAxis = new QCategoryAxis;
    defaultAxis->d_ptr->setDefaultAxis(true);
    return defaultAxis;
}

QT_DATAVIS3D_END_NAMESPACE
