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

#include "custominputhandler.h"

#include <QtDataVisualization/Q3DCamera>
#include <QtCore/qmath.h>

#include <QDebug>

CustomInputHandler::CustomInputHandler(QAbstract3DGraph *graph, QObject *parent) :
    Q3DInputHandler(parent),
    m_highlight(0),
    m_mousePressed(false),
    m_state(StateNormal),
    m_axisX(0),
    m_axisZ(0),
    m_speedModifier(20.0f)
{
    // Connect to the item selection signal from graph
    connect(graph, &QAbstract3DGraph::selectedElementChanged, this,
            &CustomInputHandler::handleElementSelected);
}

void CustomInputHandler::mousePressEvent(QMouseEvent *event, const QPoint &mousePos)
{
    if (Qt::LeftButton == event->button()) {
        m_highlight->setVisible(false);
        m_mousePressed = true;
    }
    Q3DInputHandler::mousePressEvent(event, mousePos);
}

void CustomInputHandler::wheelEvent(QWheelEvent *event)
{
    float delta = float(event->delta());

    m_axisXMinValue += delta;
    m_axisXMaxValue -= delta;
    m_axisZMinValue += delta;
    m_axisZMaxValue -= delta;
    checkConstraints();

    float y = (m_axisXMaxValue - m_axisXMinValue) * m_aspectRatio;

    m_axisX->setRange(m_axisXMinValue, m_axisXMaxValue);
    m_axisY->setRange(100.0f, y);
    m_axisZ->setRange(m_axisZMinValue, m_axisZMaxValue);
}

void CustomInputHandler::mouseMoveEvent(QMouseEvent *event, const QPoint &mousePos)
{
    // Check if we're trying to drag axis label
    if (m_mousePressed && m_state != StateNormal) {
        setPreviousInputPos(inputPosition());
        setInputPosition(mousePos);
        handleAxisDragging();
    } else {
        Q3DInputHandler::mouseMoveEvent(event, mousePos);
    }
}

void CustomInputHandler::mouseReleaseEvent(QMouseEvent *event, const QPoint &mousePos)
{
    Q3DInputHandler::mouseReleaseEvent(event, mousePos);
    m_mousePressed = false;
    m_state = StateNormal;
}

void CustomInputHandler::handleElementSelected(QAbstract3DGraph::ElementType type)
{
    switch (type) {
    case QAbstract3DGraph::ElementAxisXLabel:
        m_state = StateDraggingX;
        break;
    case QAbstract3DGraph::ElementAxisZLabel:
        m_state = StateDraggingZ;
        break;
    default:
        m_state = StateNormal;
        break;
    }
}

void CustomInputHandler::handleAxisDragging()
{
    float distance = 0.0f;

    // Get scene orientation from active camera
    float xRotation = scene()->activeCamera()->xRotation();

    // Calculate directional drag multipliers based on rotation
    float xMulX = qCos(qDegreesToRadians(xRotation));
    float xMulY = qSin(qDegreesToRadians(xRotation));
    float zMulX = qSin(qDegreesToRadians(xRotation));
    float zMulY = qCos(qDegreesToRadians(xRotation));

    // Get the drag amount
    QPoint move = inputPosition() - previousInputPos();

    // Adjust axes
    switch (m_state) {
    case StateDraggingX:
        distance = (move.x() * xMulX - move.y() * xMulY) * m_speedModifier;
        m_axisXMinValue -= distance;
        m_axisXMaxValue -= distance;
        if (m_axisXMinValue < m_areaMinValue) {
            float dist = m_axisXMaxValue - m_axisXMinValue;
            m_axisXMinValue = m_areaMinValue;
            m_axisXMaxValue = m_axisXMinValue + dist;
        }
        if (m_axisXMaxValue > m_areaMaxValue) {
            float dist = m_axisXMaxValue - m_axisXMinValue;
            m_axisXMaxValue = m_areaMaxValue;
            m_axisXMinValue = m_axisXMaxValue - dist;
        }
        m_axisX->setRange(m_axisXMinValue, m_axisXMaxValue);
        break;
    case StateDraggingZ:
        distance = (move.x() * zMulX + move.y() * zMulY) * m_speedModifier;
        m_axisZMinValue += distance;
        m_axisZMaxValue += distance;
        if (m_axisZMinValue < m_areaMinValue) {
            float dist = m_axisZMaxValue - m_axisZMinValue;
            m_axisZMinValue = m_areaMinValue;
            m_axisZMaxValue = m_axisZMinValue + dist;
        }
        if (m_axisZMaxValue > m_areaMaxValue) {
            float dist = m_axisZMaxValue - m_axisZMinValue;
            m_axisZMaxValue = m_areaMaxValue;
            m_axisZMinValue = m_axisZMaxValue - dist;
        }
        m_axisZ->setRange(m_axisZMinValue, m_axisZMaxValue);
        break;
    default:
        break;
    }
}

void CustomInputHandler::checkConstraints()
{
    if (m_axisXMinValue < m_areaMinValue)
        m_axisXMinValue = m_areaMinValue;
    if (m_axisXMaxValue > m_areaMaxValue)
        m_axisXMaxValue = m_areaMaxValue;
    // Don't allow too much zoom in
    if ((m_axisXMaxValue - m_axisXMinValue) < m_axisXMinRange) {
        float adjust = (m_axisXMinRange - (m_axisXMaxValue - m_axisXMinValue)) / 2.0f;
        m_axisXMinValue -= adjust;
        m_axisXMaxValue += adjust;
    }

    if (m_axisZMinValue < m_areaMinValue)
        m_axisZMinValue = m_areaMinValue;
    if (m_axisZMaxValue > m_areaMaxValue)
        m_axisZMaxValue = m_areaMaxValue;
    // Don't allow too much zoom in
    if ((m_axisZMaxValue - m_axisZMinValue) < m_axisZMinRange) {
        float adjust = (m_axisZMinRange - (m_axisZMaxValue - m_axisZMinValue)) / 2.0f;
        m_axisZMinValue -= adjust;
        m_axisZMaxValue += adjust;
    }
}
