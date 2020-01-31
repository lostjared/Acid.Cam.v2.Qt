
#include "gl_display.h"

#include <QtCore/QCoreApplication>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLPaintDevice>
#include <QtGui/QPainter>
#include <iostream>
#include<cmath>
#include<ctime>


void _gluPerspective( GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar )
{
    const GLdouble pi = 3.1415926535897932384626433832795;
    GLdouble fW, fH;
    fH = tan( fovY / 360 * pi ) * zNear;
    fW = fH * aspect;
    glFrustum( -fW, fW, -fH, fH, zNear, zFar );
}

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
    srand((unsigned int)std::time(0));
    glEnable(GL_TEXTURE_2D);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glClearColor(0.0, 0.0, 0.0, 0.0);}

void glDisplayWindow::setNewFrame(QImage img) {
    QRect src(QPoint(0, 0), size());
    QPixmap p = QPixmap::fromImage(img).scaled(size(),Qt::KeepAspectRatio, Qt::FastTransformation);
    QRect dst(QPoint(0,0),p.size());
    dst.moveCenter(src.center());
    frame = p.toImage().mirrored();
    updated = true;
}

void glDisplayWindow::updateTexture(QImage) {
}

void glDisplayWindow::render() {
    
    glViewport(0, 0, width(), height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (!m_device)
        m_device = new QOpenGLPaintDevice;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width(), height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    _gluPerspective(45.0, 16.0/9.0*float(width())/float(height()), 0.1, 100.0);
    glDisable(GL_DEPTH_TEST);
    glOrtho(0, 1, 1, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDrawPixels(frame.width(), frame.height(), GL_BGRA, GL_UNSIGNED_BYTE, frame.bits());
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

