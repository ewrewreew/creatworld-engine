// Creatworld.hpp - 完整版游戏引擎 v1.0
// 单头文件，零依赖（仅需Windows SDK），开箱即用
// 功能：2D图形、音频、输入、粒子、物理、动画、摄像机、场景管理、资源管理、定时器
#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include <functional>
#include <cmath>
#include <cstdio>
#include <map>
#include <unordered_map>
#include <queue>
#include <random>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cstring>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdiplus.lib")

// 版本信息
#define CREATWORLD_VERSION "1.0.0"
#define CREATWORLD_NAME "Creatworld Engine"

namespace creatworld {

// ==================== 前向声明 ====================
class game_object;
class scene;
class engine;

// ==================== 全局引擎指针 ====================
static engine* g_engine = nullptr;

// ==================== GDI+ 初始化器 ====================
class gdiplus_init {
private:
    ULONG_PTR gdiplus_token;
public:
    gdiplus_init() {
        Gdiplus::GdiplusStartupInput input;
        Gdiplus::GdiplusStartup(&gdiplus_token, &input, NULL);
    }
    ~gdiplus_init() {
        Gdiplus::GdiplusShutdown(gdiplus_token);
    }
};

// ==================== 数学库 ====================
struct vec2 {
    float x, y;
    vec2(float x = 0, float y = 0) : x(x), y(y) {}
    
    vec2 operator+(const vec2& v) const { return vec2(x + v.x, y + v.y); }
    vec2 operator-(const vec2& v) const { return vec2(x - v.x, y - v.y); }
    vec2 operator*(float s) const { return vec2(x * s, y * s); }
    vec2 operator/(float s) const { return vec2(x / s, y / s); }
    vec2& operator+=(const vec2& v) { x += v.x; y += v.y; return *this; }
    vec2& operator-=(const vec2& v) { x -= v.x; y -= v.y; return *this; }
    bool operator==(const vec2& v) const { return x == v.x && y == v.y; }
    
    float length() const { return std::sqrt(x * x + y * y); }
    float length_sq() const { return x * x + y * y; }
    vec2 normalized() const { float len = length(); return len > 0 ? vec2(x/len, y/len) : vec2(0,0); }
    float dot(const vec2& v) const { return x * v.x + y * v.y; }
    float cross(const vec2& v) const { return x * v.y - y * v.x; }
    float distance(const vec2& v) const { return (*this - v).length(); }
    vec2 lerp(const vec2& v, float t) const { return *this + (v - *this) * t; }
    vec2 rotate(float angle) const {
        float c = cos(angle), s = sin(angle);
        return vec2(x * c - y * s, x * s + y * c);
    }
    
    static vec2 zero() { return vec2(0, 0); }
    static vec2 one() { return vec2(1, 1); }
    static vec2 up() { return vec2(0, -1); }
    static vec2 down() { return vec2(0, 1); }
    static vec2 left() { return vec2(-1, 0); }
    static vec2 right() { return vec2(1, 0); }
};

struct vec3 {
    float x, y, z;
    vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    
    vec3 operator+(const vec3& v) const { return vec3(x + v.x, y + v.y, z + v.z); }
    vec3 operator-(const vec3& v) const { return vec3(x - v.x, y - v.y, z - v.z); }
    vec3 operator*(float s) const { return vec3(x * s, y * s, z * s); }
    
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    vec3 normalized() const { float len = length(); return len > 0 ? vec3(x/len, y/len, z/len) : vec3(0,0,0); }
    float dot(const vec3& v) const { return x*v.x + y*v.y + z*v.z; }
    vec3 cross(const vec3& v) const { return vec3(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x); }
};

// ==================== 颜色系统 ====================
struct color {
    unsigned char r, g, b, a;
    color(unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255)
        : r(r), g(g), b(b), a(a) {}
    
    COLORREF to_rgb() const { return RGB(r, g, b); }
    DWORD to_argb() const { return (a << 24) | (r << 16) | (g << 8) | b; }
    
    color operator*(float intensity) const {
        return color((unsigned char)(r * intensity), (unsigned char)(g * intensity), 
                     (unsigned char)(b * intensity), a);
    }
    
    color lerp(const color& other, float t) const {
        return color(r + (other.r - r) * t, g + (other.g - g) * t,
                     b + (other.b - b) * t, a + (other.a - a) * t);
    }
    
    static color red() { return color(255, 0, 0); }
    static color green() { return color(0, 255, 0); }
    static color blue() { return color(0, 0, 255); }
    static color black() { return color(0, 0, 0); }
    static color white() { return color(255, 255, 255); }
    static color yellow() { return color(255, 255, 0); }
    static color cyan() { return color(0, 255, 255); }
    static color magenta() { return color(255, 0, 255); }
    static color orange() { return color(255, 165, 0); }
    static color purple() { return color(128, 0, 128); }
    static color gray() { return color(128, 128, 128); }
    static color pink() { return color(255, 192, 203); }
    static color transparent() { return color(0, 0, 0, 0); }
    
    static color from_hsv(float h, float s, float v) {
        float c = v * s;
        float x = c * (1 - std::abs(fmod(h / 60, 2) - 1));
        float m = v - c;
        float r, g, b;
        if (h < 60) { r = c; g = x; b = 0; }
        else if (h < 120) { r = x; g = c; b = 0; }
        else if (h < 180) { r = 0; g = c; b = x; }
        else if (h < 240) { r = 0; g = x; b = c; }
        else if (h < 300) { r = x; g = 0; b = c; }
        else { r = c; g = 0; b = x; }
        return color((r + m) * 255, (g + m) * 255, (b + m) * 255);
    }
};

// ==================== 矩形 ====================
struct rect {
    float x, y, w, h;
    rect(float x = 0, float y = 0, float w = 0, float h = 0) : x(x), y(y), w(w), h(h) {}
    
    bool contains(float px, float py) const {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }
    bool contains(const vec2& p) const { return contains(p.x, p.y); }
    
    bool intersects(const rect& other) const {
        return !(x + w < other.x || x > other.x + other.w ||
                 y + h < other.y || y > other.y + other.h);
    }
    
    rect get_intersection(const rect& other) const {
        float ix = std::max(x, other.x);
        float iy = std::max(y, other.y);
        float iw = std::min(x + w, other.x + other.w) - ix;
        float ih = std::min(y + h, other.y + other.h) - iy;
        if (iw > 0 && ih > 0) return rect(ix, iy, iw, ih);
        return rect(0, 0, 0, 0);
    }
    
    vec2 center() const { return vec2(x + w/2, y + h/2); }
    void expand(float amount) { x -= amount; y -= amount; w += amount*2; h += amount*2; }
};

// ==================== 音频系统 ====================
enum class audio_type { WAV, MP3, UNKNOWN };

class sound {
private:
    struct sound_data {
        char* data;
        DWORD size;
        audio_type type;
        DWORD sample_rate;
        WORD channels;
        DWORD bitrate;
    };
    sound_data* sd = nullptr;
    HWAVEOUT hWaveOut = nullptr;
    WAVEHDR waveHdr;
    bool is_playing_flag = false;
    
    bool parse_wav_header() {
        if (sd->size < 44) return false;
        
        DWORD offset = 12;
        while (offset < sd->size - 8) {
            char chunk_id[5] = {0};
            memcpy(chunk_id, sd->data + offset, 4);
            DWORD chunk_size = *(DWORD*)(sd->data + offset + 4);
            
            if (strcmp(chunk_id, "fmt ") == 0) {
                sd->channels = *(WORD*)(sd->data + offset + 10);
                sd->sample_rate = *(DWORD*)(sd->data + offset + 12);
                sd->bitrate = *(DWORD*)(sd->data + offset + 16) * 8;
            }
            else if (strcmp(chunk_id, "data") == 0) {
                DWORD data_offset = offset + 8;
                DWORD data_size = chunk_size;
                char* audio_data = new char[data_size];
                memcpy(audio_data, sd->data + data_offset, data_size);
                delete[] sd->data;
                sd->data = audio_data;
                sd->size = data_size;
                return true;
            }
            offset += 8 + chunk_size;
        }
        return true;
    }
    
    void play_wav(float volume, bool loop) {
        WAVEFORMATEX wfx;
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nChannels = sd->channels;
        wfx.nSamplesPerSec = sd->sample_rate;
        wfx.wBitsPerSample = 16;
        wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
        wfx.cbSize = 0;
        
        if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
            return;
        }
        
        waveOutSetVolume(hWaveOut, (WORD)(volume * 0xFFFF));
        
        ZeroMemory(&waveHdr, sizeof(WAVEHDR));
        waveHdr.lpData = sd->data;
        waveHdr.dwBufferLength = sd->size;
        
        waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
        waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR));
        is_playing_flag = true;
    }
    
    void play_mp3(float volume, bool loop) {
        char temp_path[MAX_PATH];
        char mci_command[512];
        
        GetTempPathA(MAX_PATH, temp_path);
        std::string temp_file = std::string(temp_path) + "temp_audio.mp3";
        
        FILE* f = fopen(temp_file.c_str(), "wb");
        if (f) {
            fwrite(sd->data, 1, sd->size, f);
            fclose(f);
        } else {
            return;
        }
        
        sprintf(mci_command, "open \"%s\" type mpegvideo alias temp_audio", temp_file.c_str());
        mciSendStringA(mci_command, NULL, 0, NULL);
        
        sprintf(mci_command, "setaudio temp_audio volume to %d", (int)(volume * 1000));
        mciSendStringA(mci_command, NULL, 0, NULL);
        
        sprintf(mci_command, "play temp_audio %s", loop ? "repeat" : "");
        mciSendStringA(mci_command, NULL, 0, NULL);
        is_playing_flag = true;
    }
    
public:
    sound() { 
        sd = new sound_data(); 
        sd->data = nullptr; 
        sd->size = 0;
        sd->type = audio_type::UNKNOWN;
        sd->sample_rate = 44100;
        sd->channels = 1;
        sd->bitrate = 128000;
    }
    
    ~sound() { 
        stop();
        if (hWaveOut) {
            waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
            waveOutClose(hWaveOut);
        }
        delete[] sd->data; 
        delete sd; 
    }
    
    bool load(const char* filename) {
        FILE* f = fopen(filename, "rb");
        if (!f) return false;
        
        std::string name = filename;
        std::string ext = name.substr(name.find_last_of('.'));
        
        if (ext == ".mp3" || ext == ".MP3") sd->type = audio_type::MP3;
        else if (ext == ".wav" || ext == ".WAV") sd->type = audio_type::WAV;
        else sd->type = audio_type::UNKNOWN;
        
        fseek(f, 0, SEEK_END);
        sd->size = ftell(f);
        fseek(f, 0, SEEK_SET);
        sd->data = new char[sd->size];
        fread(sd->data, 1, sd->size, f);
        fclose(f);
        
        if (sd->type == audio_type::WAV) {
            parse_wav_header();
        }
        
        return true;
    }
    
    void play(float volume = 1.0f, bool loop = false) {
        if (!sd->data || sd->size == 0) return;
        stop();
        
        if (sd->type == audio_type::WAV) {
            play_wav(volume, loop);
        } else if (sd->type == audio_type::MP3) {
            play_mp3(volume, loop);
        }
    }
    
    void stop() {
        if (hWaveOut) {
            waveOutReset(hWaveOut);
            waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
            waveOutClose(hWaveOut);
            hWaveOut = nullptr;
        }
        mciSendStringA("close temp_audio", NULL, 0, NULL);
        is_playing_flag = false;
    }
    
    void pause() {
        if (hWaveOut) waveOutPause(hWaveOut);
        mciSendStringA("pause temp_audio", NULL, 0, NULL);
    }
    
    void resume() {
        if (hWaveOut) waveOutRestart(hWaveOut);
        mciSendStringA("resume temp_audio", NULL, 0, NULL);
    }
    
    void set_volume(float volume) {
        if (volume < 0) volume = 0;
        if (volume > 1) volume = 1;
        WORD vol = (WORD)(volume * 0xFFFF);
        DWORD volume_param = (vol << 16) | vol;
        if (hWaveOut) waveOutSetVolume(hWaveOut, volume_param);
    }
    
    bool is_playing() const { return is_playing_flag; }
};

// ==================== 纹理系统 ====================
class texture {
private:
    COLORREF* pixels = nullptr;
    int width = 0, height = 0;
    std::string name;
    bool has_alpha = false;
    
    bool load_bmp(const char* filename) {
        FILE* f = fopen(filename, "rb");
        if (!f) return false;
        
        BITMAPFILEHEADER fh;
        BITMAPINFOHEADER ih;
        fread(&fh, sizeof(BITMAPFILEHEADER), 1, f);
        fread(&ih, sizeof(BITMAPINFOHEADER), 1, f);
        
        if (fh.bfType != 0x4D42) { fclose(f); return false; }
        
        width = ih.biWidth;
        height = abs(ih.biHeight);
        
        delete[] pixels;
        pixels = new COLORREF[width * height];
        
        fseek(f, fh.bfOffBits, SEEK_SET);
        
        if (ih.biBitCount == 24) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    BYTE rgb[3];
                    fread(rgb, 1, 3, f);
                    pixels[y * width + x] = RGB(rgb[2], rgb[1], rgb[0]);
                }
                int padding = (4 - (width * 3) % 4) % 4;
                fseek(f, padding, SEEK_CUR);
            }
        } else if (ih.biBitCount == 32) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    BYTE rgba[4];
                    fread(rgba, 1, 4, f);
                    pixels[y * width + x] = RGB(rgba[2], rgba[1], rgba[0]);
                    if (rgba[3] < 255) has_alpha = true;
                }
            }
        }
        
        fclose(f);
        return true;
    }
    
    bool load_with_gdiplus(const char* filename) {
        WCHAR wfilename[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0, filename, -1, wfilename, MAX_PATH);
        
        Gdiplus::Image* image = Gdiplus::Image::FromFile(wfilename);
        if (!image || image->GetLastStatus() != Gdiplus::Ok) {
            delete image;
            return false;
        }
        
        width = image->GetWidth();
        height = image->GetHeight();
        
        delete[] pixels;
        pixels = new COLORREF[width * height];
        
        Gdiplus::Bitmap* bitmap = (Gdiplus::Bitmap*)image;
        Gdiplus::BitmapData bitmapData;
        Gdiplus::Rect rect(0, 0, width, height);
        
        if (bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, 
                             PixelFormat32bppARGB, &bitmapData) == Gdiplus::Ok) {
            BYTE* pixelData = (BYTE*)bitmapData.Scan0;
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    BYTE* pixel = pixelData + y * bitmapData.Stride + x * 4;
                    pixels[y * width + x] = RGB(pixel[2], pixel[1], pixel[0]);
                    if (pixel[3] < 255) has_alpha = true;
                }
            }
            bitmap->UnlockBits(&bitmapData);
        }
        
        delete image;
        return true;
    }
    
public:
    texture() = default;
    texture(int w, int h) : width(w), height(h) {
        pixels = new COLORREF[width * height];
        memset(pixels, 0, width * height * sizeof(COLORREF));
    }
    ~texture() { delete[] pixels; }
    
    bool load(const char* filename) {
        std::string fn = filename;
        std::string ext = fn.substr(fn.find_last_of('.'));
        for (auto& c : ext) c = tolower(c);
        
        if (ext == ".bmp") return load_bmp(filename);
        else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") return load_with_gdiplus(filename);
        return false;
    }
    
    void create_solid(int w, int h, const color& c) {
        width = w; height = h;
        delete[] pixels;
        pixels = new COLORREF[width * height];
        COLORREF col = c.to_rgb();
        for (int i = 0; i < width * height; i++) pixels[i] = col;
    }
    
    void create_gradient(int w, int h, const color& c1, const color& c2, bool horizontal = true) {
        width = w; height = h;
        delete[] pixels;
        pixels = new COLORREF[width * height];
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float t = horizontal ? (float)x / width : (float)y / height;
                color col = c1.lerp(c2, t);
                pixels[y * width + x] = col.to_rgb();
            }
        }
    }
    
    void create_checker(int w, int h, int cell_size, const color& c1, const color& c2) {
        width = w; height = h;
        delete[] pixels;
        pixels = new COLORREF[width * height];
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                bool is_c1 = ((x / cell_size) + (y / cell_size)) % 2 == 0;
                pixels[y * width + x] = (is_c1 ? c1 : c2).to_rgb();
            }
        }
    }
    
    void draw(HDC hdc, int x, int y) const {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                COLORREF c = pixels[j * width + i];
                if (c != RGB(255, 0, 255)) SetPixel(hdc, x + i, y + j, c);
            }
        }
    }
    
    void draw_alpha(HDC hdc, int x, int y, float alpha = 1.0f) const {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                COLORREF c = pixels[j * width + i];
                if (c != RGB(255, 0, 255)) {
                    if (alpha < 1.0f) {
                        COLORREF bg = GetPixel(hdc, x + i, y + j);
                        BYTE r = GetRValue(bg) * (1 - alpha) + GetRValue(c) * alpha;
                        BYTE g = GetGValue(bg) * (1 - alpha) + GetGValue(c) * alpha;
                        BYTE b = GetBValue(bg) * (1 - alpha) + GetBValue(c) * alpha;
                        SetPixel(hdc, x + i, y + j, RGB(r, g, b));
                    } else {
                        SetPixel(hdc, x + i, y + j, c);
                    }
                }
            }
        }
    }
    
    void draw_scaled(HDC hdc, int x, int y, int w, int h) const {
        for (int i = 0; i < w; i++) {
            for (int j = 0; j < h; j++) {
                int sx = (i * width) / w;
                int sy = (j * height) / h;
                COLORREF c = pixels[sy * width + sx];
                if (c != RGB(255, 0, 255)) SetPixel(hdc, x + i, y + j, c);
            }
        }
    }
    
    void set_pixel(int x, int y, const color& c) {
        if (x >= 0 && x < width && y >= 0 && y < height)
            pixels[y * width + x] = c.to_rgb();
    }
    
    color get_pixel(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            COLORREF c = pixels[y * width + x];
            return color(GetRValue(c), GetGValue(c), GetBValue(c));
        }
        return color::black();
    }
    
    void fill(const color& c) {
        COLORREF col = c.to_rgb();
        for (int i = 0; i < width * height; i++) pixels[i] = col;
    }
    
    void flip_horizontal() {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width / 2; x++) {
                std::swap(pixels[y * width + x], pixels[y * width + (width - 1 - x)]);
            }
        }
    }
    
    void flip_vertical() {
        for (int y = 0; y < height / 2; y++) {
            for (int x = 0; x < width; x++) {
                std::swap(pixels[y * width + x], pixels[(height - 1 - y) * width + x]);
            }
        }
    }
    
    int get_width() const { return width; }
    int get_height() const { return height; }
    void set_name(const std::string& n) { name = n; }
    const std::string& get_name() const { return name; }
    COLORREF* get_pixels() { return pixels; }
};

// ==================== 动画系统 ====================
struct animation_frame {
    texture* tex;
    float duration;
    rect source_rect;
};

class animation {
private:
    std::vector<animation_frame> frames;
    float current_time = 0;
    int current_frame = 0;
    bool playing = true;
    bool looping = true;
    std::string name;
    
public:
    animation(const std::string& n = "") : name(n) {}
    
    void add_frame(texture* tex, float duration, const rect& src = rect(0,0,0,0)) {
        animation_frame f;
        f.tex = tex;
        f.duration = duration;
        f.source_rect = src;
        frames.push_back(f);
    }
    
    void update(float dt) {
        if (!playing || frames.empty()) return;
        current_time += dt;
        if (current_time >= frames[current_frame].duration) {
            current_time = 0;
            current_frame++;
            if (current_frame >= frames.size()) {
                if (looping) current_frame = 0;
                else { current_frame = frames.size() - 1; playing = false; }
            }
        }
    }
    
    void draw(HDC hdc, int x, int y) {
        if (!frames.empty() && frames[current_frame].tex) {
            frames[current_frame].tex->draw(hdc, x, y);
        }
    }
    
    void play() { playing = true; }
    void stop() { playing = false; current_frame = 0; current_time = 0; }
    void pause() { playing = false; }
    void restart() { current_frame = 0; current_time = 0; playing = true; }
    
    const std::string& get_name() const { return name; }
    int get_frame_count() const { return frames.size(); }
};

class animator {
private:
    std::unordered_map<std::string, animation> anims;
    animation* current = nullptr;
    
public:
    void add_animation(const animation& anim) {
        anims[anim.get_name()] = anim;
    }
    
    void play(const std::string& name) {
        auto it = anims.find(name);
        if (it != anims.end()) {
            current = &it->second;
            current->play();
        }
    }
    
    void update(float dt) {
        if (current) current->update(dt);
    }
    
    void draw(HDC hdc, int x, int y) {
        if (current) current->draw(hdc, x, y);
    }
    
    animation* get_current() { return current; }
};

// ==================== 粒子系统 ====================
struct particle {
    vec2 position;
    vec2 velocity;
    vec2 acceleration;
    color start_color;
    color end_color;
    float life;
    float max_life;
    float size;
    float start_size;
    float end_size;
    bool active;
    
    particle() : active(false) {}
    
    void init(const vec2& pos, const vec2& vel, const color& start_c, const color& end_c,
              float lifetime, float start_sz, float end_sz) {
        position = pos;
        velocity = vel;
        acceleration = vec2(0, 100);
        start_color = start_c;
        end_color = end_c;
        life = lifetime;
        max_life = lifetime;
        start_size = start_sz;
        end_size = end_sz;
        size = start_sz;
        active = true;
    }
    
    bool update(float dt) {
        if (!active) return false;
        life -= dt;
        if (life <= 0) { active = false; return false; }
        
        velocity += acceleration * dt;
        position += velocity * dt;
        return true;
    }
    
    void draw(HDC hdc) {
        if (!active) return;
        float t = 1.0f - (life / max_life);
        color c = start_color.lerp(end_color, t);
        float sz = start_size + (end_size - start_size) * t;
        
        RECT r = {(int)(position.x - sz/2), (int)(position.y - sz/2),
                  (int)(position.x + sz/2), (int)(position.y + sz/2)};
        HBRUSH brush = CreateSolidBrush(c.to_rgb());
        FillRect(hdc, &r, brush);
        DeleteObject(brush);
    }
};

class particle_system {
private:
    std::vector<particle> particles;
    std::mt19937 rng;
    
public:
    particle_system() : rng(std::random_device{}()) {}
    
    void emit(const vec2& pos, int count, const color& start_c, const color& end_c,
              float min_speed = 50, float max_speed = 300, float min_life = 0.5f, float max_life = 2.0f) {
        std::uniform_real_distribution<float> angle_dist(0, 2 * 3.14159f);
        std::uniform_real_distribution<float> speed_dist(min_speed, max_speed);
        std::uniform_real_distribution<float> life_dist(min_life, max_life);
        
        for (int i = 0; i < count; i++) {
            particle p;
            float angle = angle_dist(rng);
            float speed = speed_dist(rng);
            float lifetime = life_dist(rng);
            p.init(pos, vec2(cos(angle) * speed, sin(angle) * speed),
                   start_c, end_c, lifetime, 4, 0);
            particles.push_back(p);
        }
    }
    
    void update(float dt) {
        particles.erase(std::remove_if(particles.begin(), particles.end(),
            [dt](particle& p) { return !p.update(dt); }), particles.end());
    }
    
    void draw(HDC hdc) {
        for (auto& p : particles) p.draw(hdc);
    }
    
    void clear() { particles.clear(); }
    int count() const { return particles.size(); }
};

// ==================== 物理系统 ====================
struct physics_body {
    enum type { STATIC, DYNAMIC, KINEMATIC };
    type body_type = DYNAMIC;
    vec2 velocity;
    vec2 acceleration;
    float mass = 1.0f;
    float restitution = 0.5f;
    float friction = 0.3f;
    bool use_gravity = true;
    bool is_sleeping = false;
};


// ==================== 摄像机系统 ====================
class camera {
private:
    vec2 position;
    float zoom;
    rect viewport;
    game_object* target = nullptr;
    float smooth_speed = 0.1f;
    float shake_intensity = 0;
    float shake_duration = 0;
    
public:
    camera(float screen_width, float screen_height) : zoom(1.0f) {
        viewport = rect(0, 0, screen_width, screen_height);
    }
    
    void update(float dt);
    
    vec2 world_to_screen(const vec2& world_pos) const {
        vec2 result((world_pos.x - position.x) * zoom + viewport.w/2,
                    (world_pos.y - position.y) * zoom + viewport.h/2);
        if (shake_intensity > 0) {
            result.x += (rand() % (int)(shake_intensity * 2 + 1)) - shake_intensity;
            result.y += (rand() % (int)(shake_intensity * 2 + 1)) - shake_intensity;
        }
        return result;
    }
    
    vec2 screen_to_world(const vec2& screen_pos) const {
        return vec2((screen_pos.x - viewport.w/2) / zoom + position.x,
                    (screen_pos.y - viewport.h/2) / zoom + position.y);
    }
    
    void follow(game_object* obj, float speed = 0.1f) {
        target = obj;
        smooth_speed = speed;
    }
    
    void shake(float intensity, float duration) {
        shake_intensity = intensity;
        shake_duration = duration;
    }
    
    void set_position(const vec2& pos) { position = pos; }
    void set_zoom(float z) { zoom = std::max(0.1f, std::min(10.0f, z)); }
    void move(const vec2& delta) { position += delta; }
    
    vec2 get_position() const { return position; }
    float get_zoom() const { return zoom; }
    rect get_viewport() const { return viewport; }
};

// ==================== 定时器 ====================
class timer {
private:
    float duration;
    float current_time;
    bool repeat;
    bool active;
    std::function<void()> callback;
    
public:
    timer(float d, std::function<void()> cb, bool rep = false)
        : duration(d), current_time(0), repeat(rep), active(true), callback(cb) {}
    
    void update(float dt) {
        if (!active) return;
        current_time += dt;
        if (current_time >= duration) {
            callback();
            if (repeat) current_time = 0;
            else active = false;
        }
    }
    
    void stop() { active = false; }
    void start() { active = true; current_time = 0; }
    bool is_active() const { return active; }
};

// ==================== 资源管理器 ====================
class resource_manager {
private:
    std::unordered_map<std::string, texture*> textures;
    std::unordered_map<std::string, sound*> sounds;
    
public:
    ~resource_manager() { clear(); }
    
    texture* load_texture(const std::string& name, const char* filename) {
        texture* tex = new texture();
        if (tex->load(filename)) {
            tex->set_name(name);
            textures[name] = tex;
            return tex;
        }
        delete tex;
        return nullptr;
    }
    
    sound* load_sound(const std::string& name, const char* filename) {
        sound* snd = new sound();
        if (snd->load(filename)) {
            sounds[name] = snd;
            return snd;
        }
        delete snd;
        return nullptr;
    }
    
    texture* get_texture(const std::string& name) {
        auto it = textures.find(name);
        return it != textures.end() ? it->second : nullptr;
    }
    
    sound* get_sound(const std::string& name) {
        auto it = sounds.find(name);
        return it != sounds.end() ? it->second : nullptr;
    }
    
    void unload_texture(const std::string& name) {
        auto it = textures.find(name);
        if (it != textures.end()) { delete it->second; textures.erase(it); }
    }
    
    void unload_sound(const std::string& name) {
        auto it = sounds.find(name);
        if (it != sounds.end()) { delete it->second; sounds.erase(it); }
    }
    
    void clear() {
        for (auto& pair : textures) delete pair.second;
        for (auto& pair : sounds) delete pair.second;
        textures.clear(); sounds.clear();
    }
};





// ==================== 游戏对象基类 ====================
class game_object {
public:
    vec2 position;
    vec2 velocity;
    int width = 32, height = 32;
    bool active = true;
    color object_color = color::white();
    void* user_data = nullptr;
    scene* current_scene = nullptr;
    physics_body physics;
    animator animator;
    std::string tag = "default";
    
    virtual void start() {}
    virtual void update(float dt) {
        position += velocity * dt;
        animator.update(dt);
    }
    virtual void draw(HDC hdc) {
        RECT r = {(int)position.x, (int)position.y,
                  (int)position.x + width, (int)position.y + height};
        HBRUSH brush = CreateSolidBrush(object_color.to_rgb());
        FillRect(hdc, &r, brush);
        DeleteObject(brush);
        animator.draw(hdc, (int)position.x, (int)position.y);
    }
    virtual void on_collision(game_object* other) {}
    virtual ~game_object() {}
    
    template<typename T>
    T* get_component() {
        if (typeid(T) == typeid(physics_body)) return (T*)&physics;
        return nullptr;
    }
};

class physics_world {
private:
    vec2 gravity;
    std::vector<game_object*> bodies;
    
public:
    physics_world(const vec2& g = vec2(0, 980)) : gravity(g) {}
    
    void add_body(game_object* obj) { bodies.push_back(obj); }
    void remove_body(game_object* obj) {
        auto it = std::find(bodies.begin(), bodies.end(), obj);
        if (it != bodies.end()) bodies.erase(it);
    }
    
    void update(float dt) {
        for (auto* body : bodies) {
            if (!body->active) continue;
            if (body->physics.use_gravity) {
                body->physics.acceleration += gravity;
            }
            body->physics.velocity += body->physics.acceleration * dt;
            if (body->physics.friction > 0 && body->physics.velocity.length() > 0) {
                vec2 friction_force = body->physics.velocity.normalized() * (-body->physics.friction * body->physics.mass);
                body->physics.velocity += friction_force / body->physics.mass * dt;
            }
            body->position += body->physics.velocity * dt;
            body->physics.acceleration = vec2(0, 0);
        }
    }
    
    void apply_force(game_object* obj, const vec2& force) {
        if (obj->physics.body_type == physics_body::DYNAMIC) {
            obj->physics.acceleration += force / obj->physics.mass;
        }
    }
    
    void apply_impulse(game_object* obj, const vec2& impulse) {
        if (obj->physics.body_type == physics_body::DYNAMIC) {
            obj->physics.velocity += impulse / obj->physics.mass;
        }
    }
    
    void set_gravity(const vec2& g) { gravity = g; }
    vec2 get_gravity() const { return gravity; }
};









// ==================== 场景管理 ====================
class scene {
private:
    std::string name;
    std::vector<game_object*> objects;
    std::vector<game_object*> pending_add;
    std::vector<game_object*> pending_remove;
    
public:
    scene(const std::string& n) : name(n) {}
    virtual ~scene() { clear(); }
    
    virtual void on_enter() {}
    virtual void on_exit() {}
    virtual void on_update(float dt) {}
    virtual void on_draw(HDC hdc) {}
    
    void add_object(game_object* obj) {
        pending_add.push_back(obj);
        obj->current_scene = this;
    }
    
    void remove_object(game_object* obj) { pending_remove.push_back(obj); }
    
    void update(float dt) {
        for (auto* obj : pending_add) objects.push_back(obj);
        pending_add.clear();
        
        for (auto* obj : pending_remove) {
            auto it = std::find(objects.begin(), objects.end(), obj);
            if (it != objects.end()) { objects.erase(it); delete obj; }
        }
        pending_remove.clear();
        
        for (auto* obj : objects) if (obj->active) obj->update(dt);
        on_update(dt);
    }
    
    void draw(HDC hdc) {
        for (auto* obj : objects) if (obj->active) obj->draw(hdc);
        on_draw(hdc);
    }
    
    void clear() {
        for (auto* obj : objects) delete obj;
        objects.clear();
    }
    
    const std::string& get_name() const { return name; }
    std::vector<game_object*>& get_objects() { return objects; }
};








// ==================== 输入系统 ====================
class input_system {
private:
    bool keys[256] = {};
    bool prev_keys[256] = {};
    vec2 mouse_pos;
    bool mouse_buttons[5] = {};
    bool prev_mouse_buttons[5] = {};
    vec2 mouse_delta;
    vec2 last_mouse_pos;
    
public:
    void update() {
        memcpy(prev_keys, keys, sizeof(keys));
        memcpy(prev_mouse_buttons, mouse_buttons, sizeof(mouse_buttons));
        mouse_delta = mouse_pos - last_mouse_pos;
        last_mouse_pos = mouse_pos;
    }
    
    void set_key(int key, bool pressed) { if (key >= 0 && key < 256) keys[key] = pressed; }
    void set_mouse_pos(int x, int y) { mouse_pos = vec2((float)x, (float)y); }
    void set_mouse_button(int btn, bool pressed) { if (btn >= 0 && btn < 5) mouse_buttons[btn] = pressed; }
    
    bool get_key(int key) const { return key >= 0 && key < 256 && keys[key]; }
    bool get_key_down(int key) const { return key >= 0 && key < 256 && keys[key] && !prev_keys[key]; }
    bool get_key_up(int key) const { return key >= 0 && key < 256 && !keys[key] && prev_keys[key]; }
    
    bool get_mouse_button(int btn) const { return btn >= 0 && btn < 5 && mouse_buttons[btn]; }
    bool get_mouse_button_down(int btn) const { return btn >= 0 && btn < 5 && mouse_buttons[btn] && !prev_mouse_buttons[btn]; }
    
    vec2 get_mouse_position() const { return mouse_pos; }
    vec2 get_mouse_delta() const { return mouse_delta; }
};

// ==================== 引擎核心 ====================
class engine {
private:
    HWND hwnd = nullptr;
    HDC back_dc = nullptr;
    HBITMAP back_bitmap = nullptr;
    int screen_width = 800, screen_height = 600;
    bool running = false;
    std::string window_title;
    
    std::vector<scene*> scenes;
    scene* current_scene = nullptr;
    scene* next_scene = nullptr;
    
    std::vector<timer*> timers;
    particle_system particle_sys;
    physics_world physics_world_obj;
    camera* main_camera = nullptr;
    resource_manager resources;
    input_system input_sys;
    
    LARGE_INTEGER freq, last_time;
    float delta_time = 0;
    float fps = 0;
    int frame_count = 0;
    float fps_timer = 0;
    
    static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        engine* e = (engine*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (!e) return DefWindowProc(hwnd, msg, wp, lp);
        
        switch (msg) {
            case WM_DESTROY: e->running = false; PostQuitMessage(0); return 0;
            case WM_KEYDOWN: if (wp < 256) e->input_sys.set_key(wp, true); return 0;
            case WM_KEYUP: if (wp < 256) e->input_sys.set_key(wp, false); return 0;
            case WM_MOUSEMOVE: e->input_sys.set_mouse_pos(LOWORD(lp), HIWORD(lp)); return 0;
            case WM_LBUTTONDOWN: e->input_sys.set_mouse_button(0, true); return 0;
            case WM_LBUTTONUP: e->input_sys.set_mouse_button(0, false); return 0;
            case WM_RBUTTONDOWN: e->input_sys.set_mouse_button(1, true); return 0;
            case WM_RBUTTONUP: e->input_sys.set_mouse_button(1, false); return 0;
        }
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    
    bool create_window(int width, int height, const char* title) {
        screen_width = width; screen_height = height; window_title = title;
        
        WNDCLASS wc = {};
        wc.lpfnWndProc = window_proc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = "CreatworldWindow";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        RegisterClass(&wc);
        
        hwnd = CreateWindowEx(0, "CreatworldWindow", title,
                              WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                              CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                              NULL, NULL, wc.hInstance, NULL);
        
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
        
        HDC hdc = GetDC(hwnd);
        back_dc = CreateCompatibleDC(hdc);
        back_bitmap = CreateCompatibleBitmap(hdc, width, height);
        SelectObject(back_dc, back_bitmap);
        ReleaseDC(hwnd, hdc);
        
        main_camera = new camera((float)width, (float)height);
        g_engine = this;
        
        return hwnd != nullptr;
    }
    
    void process_events() {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    void update_scene() {
        if (next_scene && next_scene != current_scene) {
            if (current_scene) current_scene->on_exit();
            current_scene = next_scene;
            if (current_scene) current_scene->on_enter();
            next_scene = nullptr;
        }
        if (current_scene) current_scene->update(delta_time);
    }
    
    void render() {
        RECT clear = {0, 0, screen_width, screen_height};
        HBRUSH black = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(back_dc, &clear, black);
        DeleteObject(black);
        
        if (main_camera) {
            XFORM old_xform;
            GetWorldTransform(back_dc, &old_xform);
            
            vec2 cam_pos = main_camera->get_position();
            float cam_zoom = main_camera->get_zoom();
            XFORM xform;
            xform.eM11 = cam_zoom; xform.eM12 = 0;
            xform.eM21 = 0; xform.eM22 = cam_zoom;
            xform.eDx = -cam_pos.x * cam_zoom + screen_width/2;
            xform.eDy = -cam_pos.y * cam_zoom + screen_height/2;
            SetWorldTransform(back_dc, &xform);
            
            if (current_scene) current_scene->draw(back_dc);
            particle_sys.draw(back_dc);
            on_draw(back_dc);
            
            SetWorldTransform(back_dc, &old_xform);
        } else {
            if (current_scene) current_scene->draw(back_dc);
            particle_sys.draw(back_dc);
            on_draw(back_dc);
        }
        
        char fps_text[32];
        sprintf(fps_text, "FPS: %.0f", fps);
        SetBkMode(back_dc, TRANSPARENT);
        SetTextColor(back_dc, RGB(0, 255, 0));
        TextOutA(back_dc, 10, 10, fps_text, strlen(fps_text));
        
        HDC hdc = GetDC(hwnd);
        BitBlt(hdc, 0, 0, screen_width, screen_height, back_dc, 0, 0, SRCCOPY);
        ReleaseDC(hwnd, hdc);
    }
    
public:
    engine() : physics_world_obj(vec2(0, 980)) {
        QueryPerformanceFrequency(&freq);
    }
    
    virtual ~engine() {
        delete main_camera;
        for (auto* t : timers) delete t;
        for (auto* s : scenes) delete s;
        g_engine = nullptr;
    }
    
    virtual void on_start() {}
    virtual void on_update(float dt) {}
    virtual void on_draw(HDC hdc) {}
    
    bool start(int width, int height, const char* title) {
        if (!create_window(width, height, title)) return false;
        QueryPerformanceCounter(&last_time);
        running = true;
        on_start();
        return true;
    }
    
    void run() {
        while (running) {
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            delta_time = (float)(now.QuadPart - last_time.QuadPart) / freq.QuadPart;
            if (delta_time > 0.033f) delta_time = 0.033f;
            last_time = now;
            
            frame_count++;
            fps_timer += delta_time;
            if (fps_timer >= 1.0f) {
                fps = frame_count / fps_timer;
                frame_count = 0;
                fps_timer = 0;
                char title[256];
                sprintf(title, "%s - FPS: %.0f", window_title.c_str(), fps);
                SetWindowTextA(hwnd, title);
            }
            
            input_sys.update();
            process_events();
            
            if (main_camera) main_camera->update(delta_time);
            physics_world_obj.update(delta_time);
            update_scene();
            particle_sys.update(delta_time);
            
            for (auto* t : timers) t->update(delta_time);
            timers.erase(std::remove_if(timers.begin(), timers.end(),
                [](timer* t) { return !t->is_active(); }), timers.end());
            
            on_update(delta_time);
            render();
            Sleep(1);
        }
    }
    
    void stop() { running = false; }
    void shutdown() {
        DeleteDC(back_dc);
        DeleteObject(back_bitmap);
        DestroyWindow(hwnd);
    }
    
    // 场景管理
    void add_scene(scene* s) { scenes.push_back(s); }
    void switch_scene(const std::string& name) {
        for (auto* s : scenes) if (s->get_name() == name) { next_scene = s; break; }
    }
    scene* get_current_scene() { return current_scene; }
    
    // 对象管理
    void add_object(game_object* obj) { if (current_scene) current_scene->add_object(obj); }
    void remove_object(game_object* obj) { if (current_scene) current_scene->remove_object(obj); }
    std::vector<game_object*> get_objects_with_tag(const std::string& tag) {
        std::vector<game_object*> result;
        if (current_scene) {
            for (auto* obj : current_scene->get_objects())
                if (obj->tag == tag) result.push_back(obj);
        }
        return result;
    }
    
    template<typename T>
    std::vector<T*> get_objects() {
        std::vector<T*> result;
        if (current_scene) {
            for (auto* obj : current_scene->get_objects())
                if (T* casted = dynamic_cast<T*>(obj)) result.push_back(casted);
        }
        return result;
    }
    
    // 定时器
    void set_timer(float duration, std::function<void()> callback, bool repeat = false) {
        timers.push_back(new timer(duration, callback, repeat));
    }
    
    // 粒子
    void emit_particles(const vec2& pos, int count, const color& start_c, const color& end_c,
                        float min_speed = 50, float max_speed = 300) {
        particle_sys.emit(pos, count, start_c, end_c, min_speed, max_speed);
    }
    
    // 物理
    void add_physics_body(game_object* obj) { physics_world_obj.add_body(obj); }
    void apply_force(game_object* obj, const vec2& force) { physics_world_obj.apply_force(obj, force); }
    void apply_impulse(game_object* obj, const vec2& impulse) { physics_world_obj.apply_impulse(obj, impulse); }
    
    // 摄像机
    camera* get_camera() { return main_camera; }
    void set_camera_target(game_object* target) { if (main_camera) main_camera->follow(target); }
    void shake_camera(float intensity, float duration) { if (main_camera) main_camera->shake(intensity, duration); }
    
    // 资源
    resource_manager& get_resources() { return resources; }
    
    // 输入
    bool is_key_pressed(int key) const { return input_sys.get_key(key); }
    bool is_key_just_pressed(int key) const { return input_sys.get_key_down(key); }
    vec2 get_mouse_position() const { return input_sys.get_mouse_position(); }
    bool is_mouse_button_pressed(int btn) const { return input_sys.get_mouse_button(btn); }
    
    // 工具
    int get_screen_width() const { return screen_width; }
    int get_screen_height() const { return screen_height; }
    float get_delta_time() const { return delta_time; }
    float get_fps() const { return fps; }
    HWND get_window_handle() const { return hwnd; }
    HDC get_back_buffer() const { return back_dc; }
};

// camera 函数实现
void camera::update(float dt) {
    if (shake_duration > 0) {
        shake_duration -= dt;
        if (shake_duration <= 0) shake_intensity = 0;
    }
    
    if (target) {
        vec2 target_pos = target->position;
        position.x += (target_pos.x - position.x) * smooth_speed;
        position.y += (target_pos.y - position.y) * smooth_speed;
    }
}

} // namespace creatworld