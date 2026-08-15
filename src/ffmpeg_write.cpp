/*
 * Acid Cam v2 - Qt/OpenCV Edition
 * written by Jared Bruni ( http://lostsidedead.com )
 * (C) 2017 GPL
 */

#include "ffmpeg_write.h"
#include "MXWrite/mxwrite.hpp"
#include <filesystem>

bool ffmpeg_mux_audio(const std::string &temp_video, const std::string &source,
                      const std::string &output,
                      const FFmpegLogCallback &logCallback) {
    std::error_code error;
    std::filesystem::copy_file(temp_video, output,
                               std::filesystem::copy_options::overwrite_existing,
                               error);
    if(error) {
        if(logCallback)
            logCallback("Could not prepare output for audio transfer: " +
                        error.message() + "\n");
        return false;
    }
    const bool success = mx::transfer_audio(source, output);
    if(!success) {
        std::error_code cleanupError;
        std::filesystem::remove(output + ".tmp", cleanupError);
        if(logCallback) {
            logCallback("MXWrite could not transfer the source audio stream. "
                        "The final video was kept without audio.\n");
            if(cleanupError)
                logCallback("Could not remove the partial mux file: " +
                            cleanupError.message() + "\n");
        }
    }
    return success;
}
