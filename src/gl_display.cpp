
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
    , m_device(0)
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
    glClearColor(0, 0, 0, 0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width(), height(), 0, -1.0, 1.0);
    glEnable (GL_DEPTH_TEST);
}

void glDisplayWindow::setNewFrame(const QImage &new_one) {
    frame_copy = new_one;
}

unsigned int texID;

void glDisplayWindow::render() {
    if (!m_device)
        m_device = new QOpenGLPaintDevice;
    if(frame_copy.width()>100 && isVisible()) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect=(float)width()/(float)height();
        
        if(width() <= height())
            glOrtho ( -5.0, 5.0, -5.0/aspect, 5.0/aspect, -5.0, 5.0);
        else
            glOrtho (-5.0*aspect, 5.0*aspect, -5.0, 5.0, -5.0, 5.0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if(frame_copy.width() > 25 && frame_copy.height() > 25) {
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glDrawPixels(frame_copy.width(), frame_copy.height(), GL_RGB,GL_UNSIGNED_BYTE,(unsigned char*)frame_copy.bits());
            glFlush();
        }
    }
    m_device->setPaintFlipped(true);
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

