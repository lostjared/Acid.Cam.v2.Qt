
#include "gl_display.h"

#include <QtCore/QCoreApplication>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLPaintDevice>
#include <QtGui/QPainter>
#include <iostream>

glDisplayWindow::glDisplayWindow(QWindow *parent)
    : QWindow(parent)
    , m_animating(false)
    , m_context(0)
, m_device(0), updated(false)
{
    setSurfaceType(QWindow::OpenGLSurface);
}

glDisplayWindow::~glDisplayWindow() {
    delete m_device;
}

void glDisplayWindow::render(QPainter *painter) {
    Q_UNUSED(painter);
}

void glDisplayWindow::initialize() {
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    resize(1280, 720);
    srand((unsigned int)time(0));
    glEnable(GL_TEXTURE_2D);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glClearColor(0.0, 0.0, 0.0, 0.0);}

void glDisplayWindow::setNewFrame(QImage image) {
    frame = image.mirrored();
    updated = true;
}

void glDisplayWindow::updateTexture(QImage) {
}

void glDisplayWindow::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (!m_device)
        m_device = new QOpenGLPaintDevice;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glOrtho(0, 1, 1, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDrawPixels(frame.width(), frame.height(), GL_RGB, GL_UNSIGNED_BYTE, frame.bits());
    m_device->setSize(size() * devicePixelRatio());
    m_device->setDevicePixelRatio(devicePixelRatio());
    QPainter painter(m_device);
    render(&painter);
}

void glDisplayWindow::renderLater() {
    requestUpdate();
}

bool glDisplayWindow::event(QEvent *event) {
    switch (event->type()) {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    default:
        return QWindow::event(event);
    }
}

void glDisplayWindow::exposeEvent(QExposeEvent *event) {
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}

void glDisplayWindow::renderNow()
{
    if (!isExposed())
        return;

    bool needsInitialize = false;

    if (!m_context) {
        m_context = new QOpenGLContext(this);
        m_context->setFormat(requestedFormat());
        m_context->create();

        needsInitialize = true;
    }

    m_context->makeCurrent(this);

    if (needsInitialize) {
        initializeOpenGLFunctions();
        initialize();
    }

    render();

    m_context->swapBuffers(this);

    if (m_animating)
        renderLater();
}

void glDisplayWindow::setAnimating(bool animating)
{
    m_animating = animating;

    if (animating)
        renderLater();
}

