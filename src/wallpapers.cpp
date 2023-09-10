#include "wallpapers.h"
#include "convert.h"
#include "displays.h"
#include "log.h"
#include "mem.h"
#include "paths.h"
#include "repo.h"

#include <map>
#include <string>

#include <imageinfo.hpp>
#include <png.h>
#include <stb_image_write.h>

#include <windows.h>

namespace wallflow {

std::map<std::string, std::string> current_wallpapers;

struct Dimensions {
    uint16_t width;
    uint16_t height;
};

Dimensions GetCanvasSize()
{
    uint16_t width = 0;
    uint16_t height = 0;

    for (Display display : displays) {
        WF_LOG_OBJ(display);
        uint16_t x = display.x + display.width;
        uint16_t y = display.y + display.height;

        if (x > width) {
            width = x;
        }
        if (y > height) {
            height = y;
        }
    }
    WF_LOG(LogLevel::LINFO, std::format("GetCanvasSize(width={},height={})", width, height));

    return { width, height };
}

void ApplyPNGToWallpaperBuffer(uint8_t* buffer, std::string image_path, Display display, Dimensions canvas_size)
{
    FILE* file = fopen(image_path.c_str(), "rb");
    if (!file) {
        throw std::runtime_error(std::format("could not open file ({})", image_path));
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        throw std::runtime_error("error creating PNG read structure");
        fclose(file);
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        throw std::runtime_error("error creating PNG info structure");
        png_destroy_read_struct(&png, NULL, NULL);
        fclose(file);
        return;
    }

    if (setjmp(png_jmpbuf(png))) {
        throw std::runtime_error("error during PNG read");
        png_destroy_read_struct(&png, &info, NULL);
        fclose(file);
        return;
    }

    png_init_io(png, file);
    png_read_info(png, info);
    uint32_t width, height;
    int bit_depth, color_type;
    int channels = png_get_channels(png, info);
    png_get_IHDR(png, info, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
    if (bit_depth < 8) {
        png_set_packing(png);
    }
    if (bit_depth == 16) {
        png_set_strip_16(png);
    }
    int passes = png_set_interlace_handling(png);
    png_read_update_info(png, info);

    for (int pass = 0; pass < passes; pass++) {
        for (int y = 0; y < height; y++) {
            int canvas_y = display.y + y;
            int canvas_offset = (canvas_size.width * canvas_y + display.x) * 3;
            size_t row_size = channels * width;

            uint8_t* row = new uint8_t[row_size];

            for (int i = 0; i < row_size; i += channels) {
                size_t pixel_offset = canvas_offset + ((i / channels) * 3);
                row[i + 0] = buffer[pixel_offset + 0];
                row[i + 1] = buffer[pixel_offset + 1];
                row[i + 2] = buffer[pixel_offset + 2];
                if (channels == 4) {
                    row[i + 3] = 0xFF;
                }
            }

            png_read_row(png, row, NULL);

            for (int i = 0; i < row_size; i += channels) {
                uint8_t r = row[i + 0];
                uint8_t g = row[i + 1];
                uint8_t b = row[i + 2];

                size_t pixel_offset = canvas_offset + ((i / channels) * 3);
                buffer[pixel_offset + 0] = r;
                buffer[pixel_offset + 1] = g;
                buffer[pixel_offset + 2] = b;
            }

            delete[] row;
        }
    }

    png_destroy_info_struct(png, &info);
    fclose(file);
}

void WritePNGFile(std::string path, uint8_t* data, int width, int height)
{
    FILE* fp = fopen(path.c_str(), "wb");
    if (!fp) {
        throw std::runtime_error("error opening PNG file for writing");
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fclose(fp);
        throw std::runtime_error("error initializing libpng for writing");
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        fclose(fp);
        png_destroy_write_struct(&png, NULL);
        throw std::runtime_error("error creating PNG info struct");
    }

    if (setjmp(png_jmpbuf(png))) {
        fclose(fp);
        png_destroy_write_struct(&png, &info);
        throw std::runtime_error("error during PNG file write");
    }

    png_init_io(png, fp);

    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    for (int y = 0; y < height; y++) {
        png_write_row(png, &data[y * width * 3]);
    }

    png_write_end(png, info);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
}

void ApplyJPEGToWallpaperBuffer(uint8_t* buffer, std::string image_path, Display display, Dimensions canvas_size)
{
}

void ApplyPlaceholderToWallpaperBuffer(uint8_t* buffer, Display display, Dimensions canvas_size)
{
    uint8_t r = 0x46;
    uint8_t g = 0x2e;
    uint8_t b = 0x65;

    for (int y = 0; y < display.height; y++) {
        for (int x = 0; x < display.width; x++) {
            int row_offset = (display.y + y) * canvas_size.width * 3;
            int col_offset = (display.x + x) * 3;
            int canvas_offset = row_offset + col_offset;

            buffer[canvas_offset + 0] = r;
            buffer[canvas_offset + 1] = g;
            buffer[canvas_offset + 2] = b;
        }
    }
}

void SetWallpaperStyleToSpan()
{
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        throw std::exception("Could not change wallpaper fit to span");
    }

    const wchar_t* wallpaperStyleValue = L"22";
    RegSetValueExW(hKey, L"WallpaperStyle", 0, REG_SZ, (const BYTE*)wallpaperStyleValue, sizeof(wchar_t) * (wcslen(wallpaperStyleValue) + 1));
    RegCloseKey(hKey);
}

void ApplyWallpaper(std::string path)
{
    if (!SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, const_cast<wchar_t*>(StringToWString(path).c_str()), SPIF_UPDATEINIFILE)) {
        throw std::exception("Could not apply wallpaper");
    }
}

void ApplyImageToWallpaperBuffer(uint8_t* buffer, std::string image_path, Display display, Dimensions canvas_size)
{
    do {
        if (image_path == "") {
            WF_LOG(LogLevel::LINFO, std::format("no image found for display {}", display.id));
            ApplyPlaceholderToWallpaperBuffer(buffer, display, canvas_size);
            break;
        }

        ImageInfo info = getImageInfo<IIFilePathReader>(image_path);

        if (info.getFormat() == II_FORMAT_PNG) {
            WF_LOG(LogLevel::LINFO, std::format("applying image ({}) to display {}", image_path, display.id));
            ApplyPNGToWallpaperBuffer(buffer, image_path, display, canvas_size);
            break;
        }
    } while (false);
}

std::mutex wallpaper_cycle_mtx;

void CycleAllDisplays()
{
    std::lock_guard<std::mutex> lock(wallpaper_cycle_mtx);
    WF_START_TIMER("CycleAllDisplays()");
    Dimensions canvas_size = GetCanvasSize();
    WF_LOG(LogLevel::LINFO, std::format("creating buffer for canvas size width={},height={}", canvas_size.width, canvas_size.height));
    size_t buffer_size = canvas_size.width * canvas_size.height * 3;
    uint16_t buffer_key = CreateFileMemoryBuffer(buffer_size);
    FileMemoryBuffer fmb = GetFileMemoryBuffer(buffer_key);

    std::vector<std::thread> threads;

    for (Display display : displays) {
        std::string current_wallpaper = GetNextImage(display.width, display.height);
        current_wallpapers[display.id] = current_wallpaper;
        ApplyImageToWallpaperBuffer(fmb.ptr, current_wallpaper, display, canvas_size);
    }

    for (std::thread& thread : threads) {
        thread.join();
    }

    std::string wallpaper_path = GetAppDataPath("wallpaper.bmp");

    // WritePNGFile(wallpaper_path, fmb.ptr, canvas_size.width, canvas_size.height);
    stbi_write_bmp(wallpaper_path.c_str(), canvas_size.width, canvas_size.height, 3, fmb.ptr);
    DeleteFileMemoryBuffer(buffer_key);
    SetWallpaperStyleToSpan();
    ApplyWallpaper(wallpaper_path);

    WF_END_TIMER("CycleAllDisplays()");
}

void CycleDisplay(Display selected_display)
{
    std::lock_guard<std::mutex> lock(wallpaper_cycle_mtx);
    WF_START_TIMER(std::format("CycleDisplay({})", selected_display.alias));
    Dimensions canvas_size = GetCanvasSize();
    size_t buffer_size = canvas_size.width * canvas_size.height * 3;
    uint16_t buffer_key = CreateFileMemoryBuffer(buffer_size);
    FileMemoryBuffer fmb = GetFileMemoryBuffer(buffer_key);

    std::vector<std::thread> threads;

    for (Display display : displays) {
        if (display.id == selected_display.id) {
            current_wallpapers[display.id] = GetNextImage(display.width, display.height);
        }
        ApplyImageToWallpaperBuffer(fmb.ptr, current_wallpapers[display.id], display, canvas_size);
    }

    for (std::thread& thread : threads) {
        thread.join();
    }

    std::string wallpaper_path = GetAppDataPath("wallpaper.bmp");
    stbi_write_bmp(wallpaper_path.c_str(), canvas_size.width, canvas_size.height, 3, fmb.ptr);

    DeleteFileMemoryBuffer(buffer_key);
    SetWallpaperStyleToSpan();
    ApplyWallpaper(wallpaper_path);

    WF_END_TIMER(std::format("CycleDisplay({})", selected_display.alias));
}

void RedrawCurrent()
{
}

}