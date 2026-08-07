/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * FFmpeg audio mux worker implementation
 */

#include "ffmpeg_mux_thread.h"

FFmpegMuxThread::FFmpegMuxThread(QObject *parent)
    : QThread(parent), muxing(false) {}

FFmpegMuxThread::~FFmpegMuxThread() {
    wait();
}

bool FFmpegMuxThread::startMux(const QString &tempFile,
                               const QString &sourceFile,
                               const QString &outputFile) {
    if(muxing || isRunning() || tempFile.isEmpty() || sourceFile.isEmpty() ||
       outputFile.isEmpty()) {
        return false;
    }

    temp_file = tempFile;
    source_file = sourceFile;
    output_file = outputFile;
    muxing = true;
    start();
    return true;
}

void FFmpegMuxThread::run() {
    const QString tempFile = temp_file;
    const QString sourceFile = source_file;
    const QString outputFile = output_file;
    const bool success =
        ffmpeg_mux_audio(tempFile.toStdString(), sourceFile.toStdString(),
                         outputFile.toStdString());
    muxing = false;
    emit muxFinished(success, tempFile, outputFile);
}
