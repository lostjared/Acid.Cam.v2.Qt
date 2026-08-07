/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * FFmpeg audio mux worker
 */

#ifndef _FFMPEG_MUX_THREAD_H_
#define _FFMPEG_MUX_THREAD_H_

#include "ffmpeg_write.h"
#include <QString>
#include <QThread>
#include <atomic>

class FFmpegMuxThread : public QThread {
    Q_OBJECT

private:
    QString temp_file;
    QString source_file;
    QString output_file;
    std::atomic<bool> muxing;

public:
    explicit FFmpegMuxThread(QObject *parent = nullptr);
    ~FFmpegMuxThread() override;

    bool startMux(const QString &tempFile, const QString &sourceFile,
                  const QString &outputFile);
    bool isMuxing() const { return muxing; }

protected:
    void run() override;

signals:
    void muxFinished(bool success, QString tempFile, QString outputFile);
};

#endif
