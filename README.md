# Spinning Donut 3D (Because Apparently We Needed More Donuts)
Behold: a mathematically overachieving pastry pirouetting in your Windows console. It spins. It shimmers. It eats your CPU for breakfast and asks for seconds.

## Features
- Real-time 3D donut that spins like it�s late for a meeting.
- Dynamic shading with a full suite of punctuation marks.
- Rainbow mode, because science.
- Adjustable speed, zoom, and aspect ratio for the truly discerning donut connoisseur.
- Highly educational use of `WriteConsoleOutput` and raw buffers, which you will absolutely never need again.

## Requirements
- Windows (console APIs are used; bring your own Linux porting kit and a sense of adventure).
- C++14 compiler.
- A GPU is not required. Your CPU will bravely volunteer.

## Build
### Visual Studio
1. Open the solution/project (or create a new Win32 Console App and add `Spinning-Donut.cpp`).
2. Set Language Standard to C++14 (or later).
3. Build. Pray. Run.

### MinGW-w64 (g++)
```
g++ -std=c++14 -O2 -static -o SpinningDonut.exe Spinning-Donut.cpp
```
Note: Windows-only includes (`windows.h`, `conio.h`) are used.

## Run
Just launch the executable. Then pretend you�re in the 80s but with better shaders.

### Controls
- Arrow Up/Down: Speed
- Arrow Left/Right: Zoom
- Page Up/Page Down: Vertical aspect
- R: Toggle rainbow mode
- ESC: Escape this reality
