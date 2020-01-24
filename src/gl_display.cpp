
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
    resize(1280, 720);
    glEnable(GL_TEXTURE_2D);
    glClearDepth(1.0f);
    glClearColor(0, 0, 0, 0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width(), height(), 0, -1.0, 1.0);
    glEnable (GL_DEPTH_TEST);
}

void glDisplayWindow::setNewFrame(QImage image) {
    frame = image;
    updated = true;
}

void glDisplayWindow::updateTexture(QImage image) {
    static int lazy = 0;
    QImage new_one = image.mirrored();
    if(lazy == 0) {
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, new_one.width(), new_one.height(), 0, GL_BGR, GL_UNSIGNED_BYTE, new_one.bits());
        lazy = 1;
    } else {
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, new_one.width(), new_one.height(), GL_BGR, GL_UNSIGNED_BYTE, new_one.bits());
    }
}

void glDisplayWindow::render() {
    
    if (!m_device)
        m_device = new QOpenGLPaintDevice;
    
    if(updated == true && frame.width()>100 && frame.height()>100) {
        updateTexture(frame);
        updated = false;
    }
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1280, 0, 720, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, tex);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2i(0, 0); glVertex2i(0, 0);
    glTexCoord2i(0, 1); glVertex2i(0, 720);
    glTexCoord2i(1, 1); glVertex2i(1280, 720);
    glTexCoord2i(1, 0); glVertex2i(1280, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    //m_device->setSize(size() * devicePixelRatio());
    //m_device->setDevicePixelRatio(devicePixelRatio());
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

