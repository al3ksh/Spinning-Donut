#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>
#include <windows.h>
#include <conio.h>
#include <cstdio>
#include <string>

void hide_cursor() {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(console_handle, &info);
}

void set_cursor_position(int x, int y) {
    static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    std::cout.flush();
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hOut, coord);
}

void set_color(int color) {
    static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hOut, color);
}

void write_to_console_buffer(HANDLE hConsole, const std::vector<char>& screen,
    const std::vector<int>& colors, int width, int height, bool rainbow,
    SHORT topOffset)
{
    static std::vector<CHAR_INFO> buffer;
    if ((int)buffer.size() != width * height) buffer.resize(width * height);

    for (int i = 0; i < width * height; i++) {
        buffer[i].Char.AsciiChar = screen[i];
        buffer[i].Attributes = rainbow ? colors[i] : 7;
    }

    COORD bufferSize = { (SHORT)width, (SHORT)height };
    COORD bufferCoord = { 0, 0 };
    SMALL_RECT writeRegion = { 0, topOffset, (SHORT)(width - 1), (SHORT)(topOffset + height - 1) };

    WriteConsoleOutput(hConsole, buffer.data(), bufferSize, bufferCoord, &writeRegion);
}

int main(int argc, char** argv) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    hide_cursor();

    float A = 0, B = 0;
    float time = 0;
    float verticalAspect = 2.2f;
    float base_speed = 1.0f;
    float zoom = 0.45f;
    bool rainbow_mode = true;

    for (int k = 1; k < argc; k++) {
        std::string a = argv[k];
        if (a.rfind("--zoom=", 0) == 0) zoom = std::stof(a.substr(7));
        else if (a.rfind("--speed=", 0) == 0) base_speed = std::stof(a.substr(8));
        else if (a.rfind("--aspect=", 0) == 0) verticalAspect = std::stof(a.substr(9));
        else if (a == "--no-rainbow") rainbow_mode = false;
    }
    if (zoom < 0.2f) zoom = 0.2f;
    if (zoom > 2.0f) zoom = 2.0f;
    if (base_speed < 0.1f) base_speed = 0.1f;
    if (base_speed > 10.0f) base_speed = 10.0f;
    if (verticalAspect < 1.4f) verticalAspect = 1.4f;
    if (verticalAspect > 3.0f) verticalAspect = 3.0f;

    set_color(11);
    std::cout << "=== SPINNING DONUT 3D (OPTIMIZED) ===\n";
    set_color(7);
    std::cout << "Controls: Arrow UP/DOWN=Speed | LEFT/RIGHT=Zoom | PgUp/PgDn=Aspect | R=Rainbow | ESC=Exit\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    int frame = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    auto last_fps_update = start_time;

    int width = 0, height = 0;
    std::vector<char> screen;
    std::vector<float> zbuffer;
    std::vector<int> colors;

    const SHORT topOffset = 2;
    const SHORT bottomOffset = 3;
    const int color_map[6] = { 9, 11, 10, 14, 13, 12 };

    const char* shades = ".,-~:;=!*#$@";

    while (true) {
        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        int winW = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int winH = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        int newWidth = (winW < 20 ? 20 : winW);
        int tmpH = winH - topOffset - bottomOffset;
        if (tmpH < 10) tmpH = 10;
        int newHeight = tmpH;

        if (newWidth != width || newHeight != height) {
            width = newWidth;
            height = newHeight;
            screen.assign(width * height, ' ');
            zbuffer.assign(width * height, 0);
            colors.assign(width * height, 7);
        }

        if (_kbhit()) {
            int key = _getch();
            if (key == 224) {
                key = _getch();
                if (key == 72) base_speed += 0.2f;
                if (key == 80) { base_speed -= 0.2f; if (base_speed < 0.1f) base_speed = 0.1f; }
                if (key == 77) zoom += 0.05f;
                if (key == 75) { zoom -= 0.05f; if (zoom < 0.2f) zoom = 0.2f; }
                if (key == 73) { verticalAspect -= 0.1f; if (verticalAspect < 1.4f) verticalAspect = 1.4f; }
                if (key == 81) { verticalAspect += 0.1f; if (verticalAspect > 3.0f) verticalAspect = 3.0f; }
            }
            else if (key == 'r' || key == 'R') {
                rainbow_mode = !rainbow_mode;
            }
            else if (key == 27) {
                break;
            }
        }

        std::fill(screen.begin(), screen.end(), ' ');
        std::fill(zbuffer.begin(), zbuffer.end(), 0);
        std::fill(colors.begin(), colors.end(), 7);

        const float pulse = 1.0f;
        const float R1 = 0.35f * pulse * zoom;
        const float R2 = 0.70f * pulse * zoom;
        const float K2 = 9.0f;
        const float K1 = width * K2 * 3.0f / (8.0f * (R1 + R2));

        float cosA = cos(A), sinA = sin(A);
        float cosB = cos(B), sinB = sin(B);

        for (float j = 0; j < 6.28f; j += 0.05f) {
            float costheta = cos(j), sintheta = sin(j);
            for (float i = 0; i < 6.28f; i += 0.012f) {
                float cosphi = cos(i), sinphi = sin(i);

                float circlex = R2 + R1 * costheta;
                float circley = R1 * sintheta;

                float x = circlex * (cosB * cosphi + sinA * sinB * sinphi) - circley * cosA * sinB;
                float y = circlex * (sinB * cosphi - sinA * cosB * sinphi) + circley * cosA * cosB;
                float z = K2 + cosA * circlex * sinphi + circley * sinA;
                float ooz = 1.0f / z;

                int xp = (int)(width / 2 + K1 * ooz * x);
                int yp = (int)(height / 2 - (K1 * ooz * y) / verticalAspect);

                float L = cosphi * costheta * sinB - cosA * costheta * sinphi - sinA * sintheta
                    + cosB * (cosA * sintheta - costheta * sinA * sinphi);

                if (xp >= 0 && xp < width && yp >= 0 && yp < height) {
                    int buffer_idx = xp + width * yp;
                    if (L > 0 && ooz > zbuffer[buffer_idx]) {
                        zbuffer[buffer_idx] = ooz;
                        int luminance_index = (int)(L * 8);
                        if (luminance_index < 0) luminance_index = 0;
                        if (luminance_index > 11) luminance_index = 11;
                        screen[buffer_idx] = shades[luminance_index];
                        if (rainbow_mode) {
                            int color_idx = ((int)(((j + i + time) / 3.0f) * 3) % 6);
                            colors[buffer_idx] = color_map[color_idx];
                        }
                        else {
                            colors[buffer_idx] = 15;
                        }
                    }
                }
            }
        }

        write_to_console_buffer(hConsole, screen, colors, width, height, rainbow_mode, topOffset);

        frame++;
        auto current_time = std::chrono::high_resolution_clock::now();
        static float fpsShown = 0.0f;
        float elapsed_since_fps = std::chrono::duration<float>(current_time - last_fps_update).count();
        if (elapsed_since_fps > 0.2f) {
            float elapsed_total = std::chrono::duration<float>(current_time - start_time).count();
            fpsShown = frame / elapsed_total;
            last_fps_update = current_time;
        }
        set_cursor_position(0, topOffset + height + 1);
        set_color(14);
        std::printf("FPS:%3d | Speed:%.1f | Zoom:%.2f | Aspect:%.2f | Size:%dx%d | Rainbow:%s    ",
            (int)fpsShown, base_speed, zoom, verticalAspect, width, height,
            rainbow_mode ? "ON" : "OFF");
        set_color(7);

        A += 0.15f * base_speed;
        B += 0.08f * base_speed;
        time += 0.02f * base_speed;

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    set_color(7);
    std::cout << "\n\nGoodbye!\n";
    return 0;
}
