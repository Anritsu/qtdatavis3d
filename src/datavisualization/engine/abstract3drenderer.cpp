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

#include "abstract3drenderer_p.h"
#include "q3dvalueaxis.h"
#include "texturehelper_p.h"
#include "utils_p.h"
#include "q3dscene_p.h"
#include "q3dcamera_p.h"
#include "q3dlight_p.h"
#include "qabstract3dseries_p.h"
#include "q3dtheme_p.h"
#include "objecthelper_p.h"

Q_DECLARE_METATYPE(QtDataVisualization::QDataVis::ShadowQuality)

QT_DATAVISUALIZATION_BEGIN_NAMESPACE

Abstract3DRenderer::Abstract3DRenderer(Abstract3DController *controller)
    : QObject(0),
      m_hasNegativeValues(false),
      m_cachedTheme(new Q3DTheme()),
      m_drawer(new Drawer(m_cachedTheme)),
      m_cachedShadowQuality(QDataVis::ShadowQualityMedium),
      m_autoScaleAdjustment(1.0f),
      m_cachedSelectionMode(QDataVis::SelectionNone),
      m_textureHelper(0),
      m_cachedScene(new Q3DScene()),
      m_selectionDirty(true),
      m_selectionState(SelectNone)
    #ifdef DISPLAY_RENDER_SPEED
    , m_isFirstFrame(true),
      m_numFrames(0)
    #endif

{
    QObject::connect(m_drawer, &Drawer::drawerChanged, this, &Abstract3DRenderer::updateTextures);
    QObject::connect(this, &Abstract3DRenderer::needRender, controller,
                     &Abstract3DController::needRender, Qt::QueuedConnection);
    QObject::connect(this, &Abstract3DRenderer::requestShadowQuality, controller,
                     &Abstract3DController::handleRequestShadowQuality, Qt::QueuedConnection);
}

Abstract3DRenderer::~Abstract3DRenderer()
{
    for (int i = 0; i < m_visibleSeriesList.size(); i++)
        m_visibleSeriesList[i].cleanup(m_textureHelper);

    delete m_drawer;
    delete m_textureHelper;
    delete m_cachedScene;
    delete m_cachedTheme;
}

void Abstract3DRenderer::initializeOpenGL()
{
    // Set OpenGL features
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

#if !defined(QT_OPENGL_ES_2)
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
#endif

    m_textureHelper = new TextureHelper();
    m_drawer->initializeOpenGL();

    axisCacheForOrientation(Q3DAbstractAxis::AxisOrientationX).setDrawer(m_drawer);
    axisCacheForOrientation(Q3DAbstractAxis::AxisOrientationY).setDrawer(m_drawer);
    axisCacheForOrientation(Q3DAbstractAxis::AxisOrientationZ).setDrawer(m_drawer);
}

void Abstract3DRenderer::render(const GLuint defaultFboHandle)
{
#ifdef DISPLAY_RENDER_SPEED
    // For speed computation
    if (m_isFirstFrame) {
        m_lastFrameTime.start();
        m_isFirstFrame = false;
    }

    // Measure speed (as milliseconds per frame)
    m_numFrames++;
    if (m_lastFrameTime.elapsed() >= 1000) { // print only if last measurement was more than 1s ago
        qDebug() << float(m_lastFrameTime.elapsed()) / float(m_numFrames) << "ms/frame (=" << float(m_numFrames) << "fps)";
        m_numFrames = 0;
        m_lastFrameTime.restart();
    }
#endif

    if (defaultFboHandle) {
        glDepthMask(true);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glDisable(GL_BLEND); // For QtQuick2 blending is enabled by default, but we don't want it to be
    }

    // Clear the graph background to the theme color
    glViewport(m_viewport.x(),
               m_viewport.y(),
               m_viewport.width(),
               m_viewport.height());
    QVector3D clearColor = Utils::vectorFromColor(m_cachedTheme->windowColor());
    glClearColor(clearColor.x(), clearColor.y(), clearColor.z(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

QString Abstract3DRenderer::generateValueLabel(const QString &format, float value)
{
    QString valueLabelFormat = format;
    Utils::ParamType valueParamType = Utils::findFormatParamType(valueLabelFormat);
    QByteArray valueFormatArray = valueLabelFormat.toUtf8();
    return Utils::formatLabel(valueFormatArray, valueParamType, value);
}

void Abstract3DRenderer::updateSelectionState(SelectionState state)
{
    m_selectionState = state;
}

void Abstract3DRenderer::updateInputPosition(const QPoint &position)
{
    m_inputPosition = position;
}

void Abstract3DRenderer::initGradientShaders(const QString &vertexShader, const QString &fragmentShader)
{
    // Do nothing by default
    Q_UNUSED(vertexShader)
    Q_UNUSED(fragmentShader)
}

void Abstract3DRenderer::updateTheme(Q3DTheme *theme)
{
    // Synchronize the controller theme with renderer
    bool changed = theme->d_ptr->sync(*m_cachedTheme->d_ptr);

    if (changed) {
        // Update drawer if sync changed something
        m_drawer->setTheme(m_cachedTheme);
        // Re-initialize shaders
        reInitShaders();
    }
}

void Abstract3DRenderer::updateScene(Q3DScene *scene)
{
    float devicePixelRatio = scene->devicePixelRatio();
    QPoint logicalPixelPosition = scene->selectionQueryPosition();
    updateInputPosition(QPoint(logicalPixelPosition.x() * devicePixelRatio,
                               logicalPixelPosition.y() * devicePixelRatio));

    m_viewport = scene->d_ptr->glViewport();
    m_secondarySubViewport = scene->d_ptr->glSecondarySubViewport();

    if (m_primarySubViewport != scene->d_ptr->glPrimarySubViewport()) {
        // Resize of primary subviewport means resizing shadow and selection buffers
        m_primarySubViewport = scene->d_ptr->glPrimarySubViewport();
        handleResize();
    }

    scene->activeCamera()->d_ptr->updateViewMatrix(m_autoScaleAdjustment);
    // Set light position (rotate light with m_cachedScene->activeCamera(), a bit above it (as set in defaultLightPos))
    scene->setLightPositionRelativeToCamera(defaultLightPos);

    if (Q3DScene::invalidSelectionPoint() == logicalPixelPosition) {
        updateSelectionState(SelectNone);
    } else {
        // Selections are one-shot, reset selection active to false before processing
        scene->setSelectionQueryPosition(Q3DScene::invalidSelectionPoint());

        if (scene->isSlicingActive()) {
            if (scene->isPointInPrimarySubView(logicalPixelPosition))
                updateSelectionState(SelectOnOverview);
            else if (scene->isPointInSecondarySubView(logicalPixelPosition))
                updateSelectionState(SelectOnSlice);
            else
                updateSelectionState(SelectNone);
        } else {
            updateSelectionState(SelectOnScene);
        }
    }

    // Synchronize the controller scene with renderer
    scene->d_ptr->sync(*m_cachedScene->d_ptr);
}

void Abstract3DRenderer::reInitShaders()
{
#if !defined(QT_OPENGL_ES_2)
    if (m_cachedShadowQuality > QDataVis::ShadowQualityNone) {
        initGradientShaders(QStringLiteral(":/shaders/vertexShadow"),
                            QStringLiteral(":/shaders/fragmentShadowNoTexColorOnY"));
        initShaders(QStringLiteral(":/shaders/vertexShadow"),
                    QStringLiteral(":/shaders/fragmentShadowNoTex"));
        initBackgroundShaders(QStringLiteral(":/shaders/vertexShadow"),
                              QStringLiteral(":/shaders/fragmentShadowNoTex"));
    } else {
        initGradientShaders(QStringLiteral(":/shaders/vertex"),
                            QStringLiteral(":/shaders/fragmentColorOnY"));
        initShaders(QStringLiteral(":/shaders/vertex"),
                    QStringLiteral(":/shaders/fragment"));
        initBackgroundShaders(QStringLiteral(":/shaders/vertex"),
                              QStringLiteral(":/shaders/fragment"));
    }
#else
    initGradientShaders(QStringLiteral(":/shaders/vertex"),
                        QStringLiteral(":/shaders/fragmentColorOnYES2"));
    initShaders(QStringLiteral(":/shaders/vertex"),
                QStringLiteral(":/shaders/fragmentES2"));
    initBackgroundShaders(QStringLiteral(":/shaders/vertex"),
                          QStringLiteral(":/shaders/fragmentES2"));
#endif
}

void Abstract3DRenderer::handleShadowQualityChange()
{
    reInitShaders();

#if defined(QT_OPENGL_ES_2)
    if (m_cachedShadowQuality != QDataVis::ShadowQualityNone) {
        emit requestShadowQuality(QDataVis::ShadowQualityNone);
        qWarning("Shadows are not yet supported for OpenGL ES2");
        m_cachedShadowQuality = QDataVis::ShadowQualityNone;
    }
#endif
}

void Abstract3DRenderer::updateSelectionMode(QDataVis::SelectionFlags mode)
{
    m_cachedSelectionMode = mode;
    m_selectionDirty = true;
}

void Abstract3DRenderer::handleResize()
{
    if (m_primarySubViewport.width() == 0 || m_primarySubViewport.height() == 0)
        return;

    // Calculate zoom level based on aspect ratio
    GLfloat div;
    GLfloat zoomAdjustment;
    div = qMin(m_primarySubViewport.width(), m_primarySubViewport.height());
    zoomAdjustment = defaultRatio * ((m_primarySubViewport.width() / div)
                                     / (m_primarySubViewport.height() / div));
    m_autoScaleAdjustment = qMin(zoomAdjustment, 1.0f); // clamp to 1.0f

    // Re-init selection buffer
    initSelectionBuffer();

#if !defined(QT_OPENGL_ES_2)
    // Re-init depth buffer
    updateDepthBuffer();
#endif
}

void Abstract3DRenderer::updateAxisType(Q3DAbstractAxis::AxisOrientation orientation, Q3DAbstractAxis::AxisType type)
{
    axisCacheForOrientation(orientation).setType(type);
}

void Abstract3DRenderer::updateAxisTitle(Q3DAbstractAxis::AxisOrientation orientation, const QString &title)
{
    axisCacheForOrientation(orientation).setTitle(title);
}

void Abstract3DRenderer::updateAxisLabels(Q3DAbstractAxis::AxisOrientation orientation, const QStringList &labels)
{
    axisCacheForOrientation(orientation).setLabels(labels);
}

void Abstract3DRenderer::updateAxisRange(Q3DAbstractAxis::AxisOrientation orientation, float min, float max)
{
    AxisRenderCache &cache = axisCacheForOrientation(orientation);
    cache.setMin(min);
    cache.setMax(max);
}

void Abstract3DRenderer::updateAxisSegmentCount(Q3DAbstractAxis::AxisOrientation orientation, int count)
{
    axisCacheForOrientation(orientation).setSegmentCount(count);
}

void Abstract3DRenderer::updateAxisSubSegmentCount(Q3DAbstractAxis::AxisOrientation orientation, int count)
{
    axisCacheForOrientation(orientation).setSubSegmentCount(count);
}

void Abstract3DRenderer::updateAxisLabelFormat(Q3DAbstractAxis::AxisOrientation orientation, const QString &format)
{
    axisCacheForOrientation(orientation).setLabelFormat(format);
}

void Abstract3DRenderer::fixMeshFileName(QString &fileName, QAbstract3DSeries::Mesh mesh)
{
    // Default implementation does nothing.
    Q_UNUSED(fileName)
    Q_UNUSED(mesh)
}

void Abstract3DRenderer::updateSeries(const QList<QAbstract3DSeries *> &seriesList,
                                      bool updateVisibility)
{
    int visibleCount = 0;
    if (updateVisibility) {
        int oldSize = m_visibleSeriesList.size();
        foreach (QAbstract3DSeries *current, seriesList) {
            if (current->isVisible())
                visibleCount++;
        }

        // Clean up series caches that are about to be permanently deleted.
        // Can't just use cache destructor, as resize will call that to all items.
        if (visibleCount < oldSize) {
            for (int i = visibleCount; i < oldSize; i++)
                m_visibleSeriesList[i].cleanup(m_textureHelper);
        }

        if (visibleCount != oldSize)
            m_visibleSeriesList.resize(visibleCount);

        visibleCount = 0;
    }
    foreach (QAbstract3DSeries *current, seriesList) {
        if (current->isVisible())
            m_visibleSeriesList[visibleCount++].populate(current, this);
    }
}

AxisRenderCache &Abstract3DRenderer::axisCacheForOrientation(Q3DAbstractAxis::AxisOrientation orientation)
{
    switch (orientation) {
    case Q3DAbstractAxis::AxisOrientationX:
        return m_axisCacheX;
    case Q3DAbstractAxis::AxisOrientationY:
        return m_axisCacheY;
    case Q3DAbstractAxis::AxisOrientationZ:
        return m_axisCacheZ;
    default:
        qFatal("Abstract3DRenderer::axisCacheForOrientation");
        return m_axisCacheX;
    }
}

void Abstract3DRenderer::lowerShadowQuality()
{
    QDataVis::ShadowQuality newQuality = QDataVis::ShadowQualityNone;

    switch (m_cachedShadowQuality) {
    case QDataVis::ShadowQualityHigh:
        qWarning("Creating high quality shadows failed. Changing to medium quality.");
        newQuality = QDataVis::ShadowQualityMedium;
        break;
    case QDataVis::ShadowQualityMedium:
        qWarning("Creating medium quality shadows failed. Changing to low quality.");
        newQuality = QDataVis::ShadowQualityLow;
        break;
    case QDataVis::ShadowQualityLow:
        qWarning("Creating low quality shadows failed. Switching shadows off.");
        newQuality = QDataVis::ShadowQualityNone;
        break;
    case QDataVis::ShadowQualitySoftHigh:
        qWarning("Creating soft high quality shadows failed. Changing to soft medium quality.");
        newQuality = QDataVis::ShadowQualitySoftMedium;
        break;
    case QDataVis::ShadowQualitySoftMedium:
        qWarning("Creating soft medium quality shadows failed. Changing to soft low quality.");
        newQuality = QDataVis::ShadowQualitySoftLow;
        break;
    case QDataVis::ShadowQualitySoftLow:
        qWarning("Creating soft low quality shadows failed. Switching shadows off.");
        newQuality = QDataVis::ShadowQualityNone;
        break;
    default:
        // You'll never get here
        break;
    }

    emit requestShadowQuality(newQuality);
    updateShadowQuality(newQuality);
}

void Abstract3DRenderer::fixGradientAndGenerateTexture(QLinearGradient *gradient, GLuint *gradientTexture)
{
    // Readjust start/stop to match gradient texture size
    gradient->setStart(qreal(gradientTextureWidth), qreal(gradientTextureHeight));
    gradient->setFinalStop(0.0, 0.0);

    if (*gradientTexture) {
        m_textureHelper->deleteTexture(gradientTexture);
        *gradientTexture = 0;
    }

    *gradientTexture = m_textureHelper->createGradientTexture(*gradient);
}

QT_DATAVISUALIZATION_END_NAMESPACE
