
#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#include <QtQuick/QQuickItem>
#include <QtQuick/qquickwindow.h>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>
#include <QOpenGLTexture>
#include <QImage>
#include <QWindow>
#include <QByteArray>
#include <QEvent>
#include <QLinkedList>

#include "qdebug.h"
#include "core.h"
#include "audio.h"
#include "sdljoystick.h"
#include "logging.h"

class VideoItem : public QQuickItem {
    Q_OBJECT

    Q_PROPERTY(QString libcore READ libcore WRITE setCore NOTIFY libcoreChanged)
    Q_PROPERTY(QString game READ game WRITE setGame NOTIFY gameChanged)
    Q_PROPERTY(bool run READ run WRITE setRun NOTIFY runChanged)
    Q_PROPERTY(QWindow::Visibility windowVisibility READ windowVisibility WRITE setWindowVisibility NOTIFY windowVisibilityChanged)
    Q_PROPERTY(QString systemDirectory READ systemDirectory WRITE setSystemDirectory NOTIFY systemDirectoryChanged)
    Q_PROPERTY(bool gamepadScan READ gamepadScan WRITE setGamePadScan NOTIFY gamepadScanChanged)
    Q_PROPERTY(int fps READ fps NOTIFY fpsChanged)


public:
    VideoItem();
    ~VideoItem();

    void initShader();
    void initGL();
    void setCore(QString libcore);
    void setGame(QString game);
    void setRun(bool run );
    void setWindowVisibility(QWindow::Visibility windowVisibility);
    void setSystemDirectory(QString systemDirectory);
    void setTexture(QOpenGLTexture::Filter min_scale,
                    QOpenGLTexture::Filter max_scale);
    void setGamePadScan(bool gamepadScan);


    QString libcore() const {
        return m_libcore;
    }

    QString game() const {
        return m_game;
    }

    bool run() const {
        return m_run;
    }

    QWindow::Visibility windowVisibility() const {
        return m_win_visibility;
    }

    QString systemDirectory() const {
        return m_system_directory;
    }

    bool gamepadScan() const {
        return m_gamepad_scan;
    }

    int fps() const {
        return m_fps;
    }


protected:
    void keyEvent(QKeyEvent *event);
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE {
        keyEvent(event);
    };
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE {
        keyEvent(event);
    };
    void geometryChanged(const QRectF &newGeom, const QRectF &oldGeom) Q_DECL_OVERRIDE {
        Q_UNUSED(newGeom);
        Q_UNUSED(oldGeom);
        QQuickItem::geometryChanged(newGeom, oldGeom);
        refreshItemGeometry();
    };

signals:
    void libcoreChanged(QString);
    void gameChanged(QString);
    void runChanged(bool);
    void windowVisibilityChanged(QWindow::Visibility);
    void systemDirectoryChanged();
    void gamepadScanChanged(bool);
    void fpsChanged(int);

public slots:
    void paint();
    void cleanup();

private slots:
    void handleWindowChanged(QQuickWindow *win);
    void handleGeometryChanged(int unused) {
        Q_UNUSED(unused);
        refreshItemGeometry();
    }
    void handleSceneGraphInitialized();
    void updateFps() {
        m_fps = fps_count * (1000.0 / fps_timer.interval());
        fps_count = 0;
        emit fpsChanged(m_fps);
    }


private:
    // Video
    // [1]
    QOpenGLShaderProgram *m_program;
    QOpenGLTexture *m_texture;
    Core *core;
    int item_w;
    int item_h;
    qreal item_aspect; // item aspect ratio
    QPoint viewportXY;
    int fps_count;
    QTimer fps_timer;
    QElapsedTimer frame_timer;
    qint64 fps_deviation;
    // [1]

    // Qml defined variables
    // [2]
    QString m_system_directory;
    QString m_libcore;
    QString m_game;
    QWindow::Visibility m_win_visibility;
    bool m_run;
    bool m_gamepad_scan;
    int m_fps;
    //[2]

    // Audio
    //[3]
    Audio *audio;
    void updateAudioFormat();
    //[3]

    // Input
    // [4]
    unsigned id;
    unsigned device;
    unsigned port;
    bool is_pressed;
    uint32_t index;
    //[4]

    void refreshItemGeometry(); // called every time the item's with/height/x/y change

    bool limitFps(); // return true if it's too soon to ask for another frame

    static inline QImage::Format retroToQImageFormat(enum retro_pixel_format fmt) {
        static QImage::Format format_table[3] = {
            QImage::Format_RGB16,   // RETRO_PIXEL_FORMAT_0RGB1555
            QImage::Format_RGB32,   // RETRO_PIXEL_FORMAT_XRGB8888
            QImage::Format_RGB16    // RETRO_PIXEL_FORMAT_RGB565
        };

        if(fmt >= 0 && fmt < (sizeof(format_table) / sizeof(QImage::Format))) {
            return format_table[fmt];
        }
        return QImage::Format_Invalid;
    }

};

#endif // VIDEOITEM_H
