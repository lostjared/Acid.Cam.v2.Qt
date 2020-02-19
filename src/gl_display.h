

#ifndef __GL_DISPLAY___H
#define __GL_DISPLAY___H


#include <QtGui/QWindow>
#include <QtGui/QOpenGLFunctions>
#include"ac.h"
QT_BEGIN_NAMESPACE
class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;
QT_END_NAMESPACE


class glDisplayWindow : public QWindow, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit glDisplayWindow(QWindow *parent = 0);
    ~glDisplayWindow();

    virtual void render(QPainter *painter);
    virtual void render();
    virtual void initialize();

    void setAnimating(bool animating);
    void setNewFrame(QImage image);
    void updateTexture(QImage image);
public slots:
    void renderLater();
    void renderNow();

protected:
    bool event(QEvent *event) override;

    void exposeEvent(QExposeEvent *event) override;

private:
    bool m_animating;
    QOpenGLContext *m_context;
    QOpenGLPaintDevice *m_device;
    GLuint tex;
    QImage frame;
    cv::Mat copy_mat;
    bool updated;
};

#endif

