# MXWrite

MXWrite is a small C++20 static library for encoding tightly packed RGBA frames with FFmpeg. It provides a single `mx::Writer` class, performs encoding on a background thread, and lets FFmpeg select the output container from the filename extension. The public API is contained in the `mx` namespace.

The library supports:

- H.264 output for resolutions up to 4K and HEVC for larger frames
- automatic NVIDIA NVENC selection with software fallback
- configurable preset, tune, CRF, low-latency, and queue behavior
- host RGBA8 and optional CUDA device-frame input
- HEVC Main10 HDR output with BT.2020 and PQ/HLG signaling
- remuxing audio from another media file into an encoded video

## Requirements

- CMake 3.10 or newer
- A C++20 compiler
- `pkg-config`
- FFmpeg development libraries:
  - `libavcodec` 58 or newer
  - `libavformat` 58 or newer
  - `libavutil` 56 or newer
  - `libswscale` 5 or newer
- A threads implementation supported by CMake
- Optional: the CUDA Toolkit and an FFmpeg build with NVENC support

On Ubuntu or Debian:

```sh
sudo apt update
sudo apt install build-essential cmake pkg-config \
  libavcodec-dev libavformat-dev libavutil-dev libswscale-dev
```

On macOS with Homebrew:

```sh
brew install cmake pkg-config ffmpeg
```

## Build and install

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix /path/to/prefix
```

This builds the `mxwrite` static library. If CMake finds the CUDA Toolkit, device-frame ingestion is enabled automatically and the required compile definition and CUDA runtime dependency are exported with the CMake target.

The public header is installed as `include/MXWrite/mxwrite.hpp` beneath the selected prefix.

To consume an installed copy from another CMake project:

```cmake
find_package(MXWrite REQUIRED)
target_link_libraries(my_app PRIVATE MXWrite::mxwrite)
```

If MXWrite was installed to a nonstandard prefix, configure the consuming project with `-DCMAKE_PREFIX_PATH=/path/to/prefix` or set `MXWrite_DIR` to the directory containing `MXWriteConfig.cmake`.

## Quick start

```cpp
#include <MXWrite/mxwrite.hpp>

#include <cstdint>
#include <vector>

int main() {
    constexpr int width = 1280;
    constexpr int height = 720;
    constexpr float fps = 30.0F;

    mx::EncodeOptions options;
    options.codec = "auto";
    options.preset = "medium";
    options.crf = 23;
    options.block_when_full = true;

    mx::Writer writer;
    if (!writer.open("output.mp4", width, height, fps, options)) {
        return 1;
    }

    std::vector<std::uint8_t> rgba(width * height * 4, 255);
    for (int frame = 0; frame < 90; ++frame) {
        // Fill rgba with the next frame. Pixels are R, G, B, A byte order.
        writer.write(rgba.data());
    }

    // Drains the queue, flushes the encoder, and finalizes the container.
    writer.close();
}
```

Input passed to `write()` must be a tightly packed, top-down RGBA8 buffer containing `width * height * 4` bytes. MXWrite copies the frame before `write()` returns, so the caller may immediately reuse the input buffer. Use even frame dimensions for the YUV 4:2:0 output formats.

The legacy overload is also available:

```cpp
writer.open("output.mp4", 1280, 720, 30.0F, "23");
```

It uses the requested CRF with the library's legacy ultrafast, zero-latency settings. Prefer `mx::EncodeOptions` in new code.

## Encoder options

`mx::EncodeOptions` has the following defaults:

| Field | Default | Description |
| --- | --- | --- |
| `preset` | `"medium"` | x264/x265-style preset. For NVENC, common x264 preset names are mapped to `p1` through `p7`. |
| `tune` | `""` | Optional software encoder tune such as `film`, `animation`, or `zerolatency`. |
| `crf` | `18` | Quality value from 0 (best/lossless where supported) to 51 (lowest quality). Passed to NVENC as `cq`. |
| `codec` | `"auto"` | Encoder selection policy; see below. |
| `realtime` | `false` | Enables low-latency encoder settings and overrides `tune`. |
| `block_when_full` | `false` | Blocks producers when the queue is full instead of dropping frames. |

Accepted `codec` values are:

- `"auto"`: use NVENC when its encoder and CUDA context are available; otherwise use software encoding
- `"software"` (also `"x264"` or `"cpu"`): force a software encoder
- `"nvenc"`: request resolution-selected NVENC with software fallback
- `"h264_nvenc"`: request H.264 NVENC with H.264 software fallback
- `"hevc_nvenc"` or `"h265_nvenc"`: request HEVC NVENC with HEVC software fallback

In automatic and generic NVENC modes, MXWrite selects H.264 through 3840x2160 and HEVC when either dimension exceeds that limit. `is_hardware_encode()` reports which backend was actually opened.

Encoding uses a bounded queue of 120 frames. By default a full queue drops new frames to keep live producers responsive. Set `block_when_full` in `mx::EncodeOptions`, or call `set_block_when_full(true)`, for batch jobs where every submitted frame must be retained. `close()` waits for accepted frames to finish encoding.

## Timestamp-named API

MXWrite exposes `open_ts()` and `write_ts()` for compatibility with existing callers:

```cpp
if (writer.open_ts("capture.mp4", width, height, fps, options)) {
    writer.write_ts(rgba.data());
    writer.close();
}
```

The current implementation assigns sequential frame timestamps using the configured frame rate; `write_ts()` delegates to `write()`. It does not currently preserve wall-clock capture intervals. Opening through `open_ts()` does select low-delay threading and codec flags.

## CUDA device frames

When MXWrite was built with the CUDA Toolkit and the active encoder is NVENC, CUDA RGBA8 device memory can be submitted without first copying it to a caller-owned host buffer:

```cpp
if (writer.is_hardware_encode()) {
    const bool handled = writer.write_cuda_rgba(device_rgba, pitch_bytes, false);
}
```

`src_stride` is the row pitch in bytes. Set `bottom_up` to `true` to vertically flip a bottom-up source during upload. The function returns `false` for invalid input, a non-NVENC writer, disabled CUDA-copy support, or an upload failure. A frame intentionally dropped because the queue is full is considered handled and returns `true`.

## HDR output

Set `options.hdr.enabled` to use the dedicated software `libx265` HEVC Main10 path. This path produces 10-bit YUV 4:2:0 and defaults to BT.2020 primaries, SMPTE ST 2084 (PQ), BT.2020 non-constant-luminance matrix coefficients, and limited range. It requires an FFmpeg build that includes `libx265` and does not use NVENC.

Two input paths are available:

- `write(rgba8)` treats RGBA8 as sRGB/BT.709 SDR and maps it to a 100-nit reference level inside the HDR signal.
- `write_hdr_rgba16(rgba16)` accepts tightly packed little-endian RGBA16 that is already in BT.2020 primaries and already PQ- or HLG-encoded.

The nested `mx::EncodeOptions::HdrInfo` fields can override the FFmpeg color properties and carry raw mastering-display or content-light side-data payloads. Setting `color_trc` to FFmpeg's `AVCOL_TRC_ARIB_STD_B67` selects HLG signaling; otherwise the default is PQ.

## Audio remuxing

MXWrite writes video only. After closing the writer, `mx::transfer_audio()` can replace the destination file with a remuxed copy containing its video stream and the first audio stream from another media file:

```cpp
writer.close();
if (!mx::transfer_audio("source-with-audio.mp4", "output.mp4")) {
    std::cerr << "Audio transfer failed\n";
}
```

The audio is stream-copied rather than re-encoded and is trimmed to the video duration. The function reports errors to standard error and returns `false` when the transfer fails. Keep a backup if replacing the destination file would be risky.

## State and metrics

- `is_open()` reports whether the writer is open.
- `is_hardware_encode()` reports whether the active encoder uses NVENC.
- `get_frame_count()` returns the number of frames accepted into the encode queue.
- `get_duration()` returns the best available encoded duration in seconds, including a cached duration after `close()`.
- Destroying an open `mx::Writer` closes and finalizes it automatically; an explicit `close()` is still recommended so errors and lifecycle are clear.

## OpenCV example

[`opencv_example/opencv_ex.cpp`](opencv_example/opencv_ex.cpp) records a webcam after converting OpenCV's BGR frames to RGBA. Once MXWrite is installed, build it with:

```sh
cmake -S opencv_example -B opencv_example/build \
  -DCMAKE_PREFIX_PATH=/path/to/prefix
cmake --build opencv_example/build
./opencv_example/build/opencv_ex 0 0
```

The arguments are the camera index and mode (`0` for `open`/`write`, `1` for `open_ts`/`write_ts`). Press Escape to stop recording and finalize `output.mp4`.
