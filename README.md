![screenshot](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/screens/screenshot.jpg "Acid Cam v2 Qt")

## What is Acid Cam?

Acid Cam is a **real-time glitch art application** for processing live webcam feeds and video files through a massive library of visual distortion filters. It is built for artists, VJs, musicians, and anyone interested in creating unique, one-of-a-kind visual effects commonly referred to as **Acid Glitches**. The application takes video input — either from a webcam or a video file — and runs each frame through one or more pixel-manipulation filters in real time, producing output that ranges from subtle color shifts to extreme psychedelic distortions.

With over **2,200 filters** powered by the [libacidcam](https://github.com/lostjared/libacidcam) filter engine, Acid Cam can generate an essentially limitless variety of visual effects. Filters can be applied individually or chained together in a custom sequence. Many filters support **sub-filters**, allowing you to compose complex multi-layered effects. The results can be recorded directly to video files using the integrated FFmpeg encoder or exported as individual PNG frames.

Acid Cam is ideal for:
- Generating foundational glitch artwork for further manipulation in other software
- Creating real-time visuals for live music performances and VJ sets
- Producing source material for music videos and experimental film
- Exploration and experimentation with pixel-level video processing

![screenshot](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/screens/acqt-ss.png?raw=true "Acid Cam Interface")

## Recent Updates

### New Interface (v1.80)

The application has been redesigned with a new interface layout featuring organized sections for filter selection, custom filter chain management, and color adjustments — all accessible from the main window. The new interface includes:

- **Category-based filter browsing** — Filters are organized into categories (Blend, Distort, Pattern, Gradient, Mirror, Strobe, Blur, Image/Video, Square, SubFilter, Special, and more) for easier navigation
- **Custom Filter Chain builder** — Add, remove, reorder, save, and load filter chains with full sub-filter support
- **Integrated color controls** — RGB sliders, brightness, gamma, saturation, 13 color maps, channel order selection, and negate toggle all in one panel
- **Filter search** (Ctrl+S) — Tokenized keyword search across all filter names
- **User-defined filter names** — Rename and organize your favorite filters
- **Image/Video Manager** — Manage multiple blend images and secondary video sources with cycling modes (Random, In Order, Shuffle)

### libacidcam Updates — Error Handling & FFmpeg Video Writing

The core filter library [libacidcam](https://github.com/lostjared/libacidcam) has received significant updates:

- **Improved error handling** — Functions that interact with external resources (file I/O, filter invocation, video capture) now return boolean success/failure indicators. Bounds checking is available for pixel access operations. Invalid filter names and out-of-range array positions are caught and reported via diagnostic messages rather than silently failing or crashing.
- **FFmpeg integration for video writing** — The application now uses FFmpeg for video encoding instead of relying solely on OpenCV's built-in video writer. This provides dramatically better compression, smaller output file sizes, and support for modern codecs. The **recommended codec is `libx264`** (H.264) because it is the most compatible codec across all platforms and uses the CPU, meaning it will work on any system without requiring specific GPU hardware. Quality is controlled via the CRF (Constant Rate Factor) setting — lower values produce higher quality at larger file sizes.
- **Additional codec support** — Beyond libx264, the encoder also supports `libx265` (HEVC/CPU), `h264_nvenc` and `hevc_nvenc` (NVIDIA GPU hardware encoding), and `h264_vaapi` and `hevc_vaapi` (Intel/AMD VAAPI hardware encoding). Hardware encoders are auto-detected at startup.
- **Threaded encoding** — Video encoding runs on a dedicated background thread with a frame queue, so encoding does not block the real-time filter preview. The encoder thread processes frames asynchronously and gracefully handles shutdown.
- **Audio muxing** — When capturing from a video file, the original audio track can be automatically copied into the output file via a secondary FFmpeg pass.

## Important: VRAM and Memory Usage

**This application can allocate a large number of video frames in memory.** Many of the temporal filters (trails, blending, slit-scan, intertwine, in-order effects) work by storing collections of previous frames. Depending on the resolution and the number of stored frames, memory usage can grow very quickly. A single 1920x1080 BGR frame is approximately **6 MB**, so storing 300 frames at that resolution consumes roughly **1.8 GB of RAM**. Filters requiring slit-scan or intertwine effects may need over 1,080 stored frames, potentially using **6+ GB of RAM**.

### Monitoring Memory Usage

Always keep an eye on the **status bar** at the bottom of the main window. It displays real-time memory information:

```
(CurrentFrame/TotalFrames/Seconds) - Memory Allocated: XX MB - Initialized: XX - Allocated: XXX
```

- **Memory Allocated** — Total RAM consumed by stored video frames, shown in megabytes
- **Initialized** — Number of active filter objects that have been created
- **Allocated** — Current count of stored frames in the frame buffer

When recording, the status bar additionally shows the output file size and completion percentage.

### Setting the Maximum Frame Allocation

Open **Controls → Options** and set the **Max Frames** value to control how many frames the application is allowed to store. The default is **300 frames**. If you are running low on system memory, reduce this value. If you need to use Intertwine, SlitScan, or InOrder filters, you must increase Max Frames to at least **1,080**. Be aware that increasing this value at high resolutions will require significantly more RAM.

**Recommended minimum system RAM: 8 GB.** For heavy use at 1080p with large frame buffers, 16 GB or more is strongly recommended.

## System Requirements

- **OS:** Linux, macOS, or Windows
- **RAM:** 8 GB minimum (16 GB recommended for 1080p workflows)
- **FFmpeg:** Must be installed and available on PATH for video encoding
- **GPU (optional):** OpenCL-capable GPU for accelerated filter processing; NVIDIA or VAAPI-capable GPU for hardware video encoding

## Dependencies

| Dependency | Purpose |
|---|---|
| [libacidcam](https://github.com/lostjared/libacidcam) | Core filter engine (2,200+ filters) |
| OpenCV 3.0+ | Video capture, image processing, matrix operations |
| Qt6 (or Qt5 5.15+) | GUI framework (Widgets, OpenGL, Network) |
| SDL2 | Audio/display support |
| FFmpeg | Video encoding and audio muxing |
| pkg-config | Build dependency resolution |
| CMake 3.16+ | Build system |
| C++17 compiler | GCC, Clang, or MSVC with C++17 support |

## Building from Source

### Linux (Debian/Ubuntu)

```bash
# Install dependencies
sudo apt-get install build-essential cmake pkg-config \
    qt6-base-dev libopencv-dev libsdl2-dev ffmpeg \
    git autoconf automake libtool

# Build and install libacidcam first
git clone https://github.com/lostjared/libacidcam.git
cd libacidcam
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
sudo ldconfig
cd ../..

# Build Acid Cam Qt
cd Acid.Cam.v2.Qt/src
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### macOS

```bash
brew install cmake opencv qt@6 sdl2 ffmpeg pkg-config
# Then follow the same cmake build steps as Linux
```

### Windows (MinGW Cross-Compilation)

A MinGW toolchain file and build scripts are provided. See the `scripts/` directory for `build-mingw64.sh` and `deploy-mingw64.sh`.

## Application Features

### Capture Modes

- **Webcam Capture** (Ctrl+N) — Capture from a connected camera at 640x480, 1280x720, or 1920x1080
- **Video File Capture** (Ctrl+V) — Process an existing video file frame-by-frame

### Filter System

- **2,200+ built-in filters** organized into categories: Blend, Distort, Pattern, Gradient, Mirror, Strobe, Blur, Image/Video, Square, SubFilter, Special, and more
- **Single Filter mode** — Apply one filter at a time, switch on the fly
- **Custom Filter mode** — Build ordered chains of multiple filters applied sequentially to each frame
- **Sub-filters** — Many filters accept a secondary "sub-filter" for layered composition
- **Random filter** — Press Space to jump to a random filter
- **Filter cycling** — Enable Cycle Custom to automatically rotate through your filter chain with a configurable delay

### Color Controls

- **RGB sliders** (0–255) for per-channel adjustment
- **Brightness, Gamma, Saturation** sliders
- **13 Color Maps** — Autumn, Bone, Jet, Winter, Rainbow, Ocean, Summer, Cool, HSV, Pink, Hot, Parula
- **Channel Order** — RGB, BGR, BRG, GRB
- **Negate Colors** toggle

### Recording & Export

- **FFmpeg encoding** with codec selection (libx264 recommended), CRF quality control, and optional audio muxing from the source video
- **PNG frame export** — Save every processed frame as an individual PNG
- **Snapshots** (Ctrl+A) — Save the current frame as a PNG at any time
- **Legacy OpenCV encoding** — MPEG-4, AVC, XviD (produces larger files)

### Additional Features

- **Chroma Key** — Color-based keying with range or tolerance modes; replace keyed areas with a filter effect or an image
- **Slit-Scan** — Configurable slit-scan effect with adjustable width, height, slit height, repeat, delay, and wait parameters
- **Image/Video Manager** — Load multiple blend images, set cycling modes (Random, In Order, Shuffle), open secondary video files for dual-source effects
- **Plugin System** — Load custom filter plugins as shared libraries (.so/.dylib/.dll) from the plugins directory
- **Display Window** — Resizable preview with fullscreen support (press Esc to exit fullscreen)
- **Frame Navigation** — Pause and seek to any frame by number or timestamp; step through frames one at a time (Ctrl+I)
- **Filter Save/Load** — Save complete filter configurations (chain, colors, movement, settings) as `.filter` files for later recall
- **Movement Modes** — Control alpha blending animation direction: Move In/Out/Increase, Move In/Out, Move Out/Reset, with 7 speed levels
- **Cross Fade** — Smooth alpha-blend transition when switching between filters

### Options (Controls → Options)

| Setting | Default | Description |
|---|---|---|
| Thread Count | 4 | Number of parallel processing threads |
| Difference | 55 | Pixel collection intensity |
| Max Frames | 300 | Maximum stored frames in memory — increase for slit-scan/intertwine filters |
| Delay | 60 | Custom filter cycle delay |
| Wait | 5 | Variable wait value for filter timing |
| Level | 125 | Color level threshold |

### Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| Ctrl+N | Capture from Webcam |
| Ctrl+V | Capture from Video |
| Ctrl+S | Search Filters |
| Ctrl+P | Pause/Resume |
| Ctrl+I | Step one frame |
| Ctrl+A | Take snapshot |
| Ctrl+K | Next filter |
| Ctrl+L | Previous filter |
| Ctrl+2 | Show Slit-Scan settings |
| Space | Random filter |
| Esc | Exit fullscreen display |

## Developing Custom Plugins

Acid Cam supports dynamically loaded filter plugins. Place shared libraries in the `plugins/` directory. Each plugin must export two C functions:

```c
// Called for every pixel in the frame (BGR byte order)
void pixel(int x, int y, unsigned char *buf);

// Called once after all pixels have been processed
void complete();
```

See the `src/plugins/` directory for example plugin source code.

## Tips

- **Use libx264 as your encoding codec** — It is the most compatible H.264 encoder, works on every platform via CPU, and produces excellent quality at reasonable file sizes. Set the CRF value between 18 (high quality, larger files) and 28 (lower quality, smaller files).
- **Watch your memory** — Check the status bar regularly. If Memory Allocated is climbing quickly, consider reducing Max Frames in Options or working at a lower resolution.
- **Save your filter chains** — When you find a combination you like, save it as a `.filter` file so you can reload it later.
- **Filters marked 720 or 1080** in their name are designed for those resolutions and may require significant RAM. Insufficient memory will cause the program to exit.
- **Hardware encoding** — If you have an NVIDIA GPU, the application will auto-detect NVENC support and offer GPU-accelerated encoding at much faster speeds than CPU encoding.

## Screenshots

![Linux](https://github.com/lostjared/Acid.Cam.v2.Qt/blob/master/screens/acidcam.2.lin.jpg?raw=true "Acid Cam on Linux")

## License

This project is released under the **BSD 2-Clause License**. See [LICENSE](LICENSE) for details.

## Links

- [libacidcam — Core Filter Library](https://github.com/lostjared/libacidcam)


