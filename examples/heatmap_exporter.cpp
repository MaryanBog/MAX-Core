// examples/heatmap_exporter.cpp
// MAX-Core: WorldBank Stability Surface (collapse_year)
// Usage: ./heatmap_exporter <input.csv> <output_prefix>
// Output: <output_prefix>.png  (Windows WIC, no external headers/files)

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <windows.h>
#include <wincodec.h>

// -----------------------------
// Tiny bitmap font (5x7), scalable
// -----------------------------
static const uint8_t FONT5X7[96][7] = {
    {0,0,0,0,0,0,0},{0x04,0x04,0x04,0x04,0x04,0x00,0x04},{0x0A,0x0A,0x00,0x00,0x00,0x00,0x00},{0x0A,0x1F,0x0A,0x0A,0x1F,0x0A,0x00},
    {0x04,0x0F,0x14,0x0E,0x05,0x1E,0x04},{0x19,0x19,0x02,0x04,0x08,0x13,0x13},{0x0C,0x12,0x14,0x08,0x15,0x12,0x0D},{0x06,0x04,0x08,0x00,0x00,0x00,0x00},
    {0x02,0x04,0x08,0x08,0x08,0x04,0x02},{0x08,0x04,0x02,0x02,0x02,0x04,0x08},{0x00,0x0A,0x04,0x1F,0x04,0x0A,0x00},{0x00,0x04,0x04,0x1F,0x04,0x04,0x00},
    {0x00,0x00,0x00,0x00,0x06,0x04,0x08},{0x00,0x00,0x00,0x1F,0x00,0x00,0x00},{0x00,0x00,0x00,0x00,0x00,0x06,0x06},{0x01,0x02,0x04,0x08,0x10,0x00,0x00},
    {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E},{0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},{0x0E,0x11,0x01,0x06,0x08,0x10,0x1F},{0x1F,0x01,0x02,0x06,0x01,0x11,0x0E},
    {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02},{0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},{0x06,0x08,0x10,0x1E,0x11,0x11,0x0E},{0x1F,0x01,0x02,0x04,0x08,0x08,0x08},
    {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},{0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C},{0x00,0x06,0x06,0x00,0x06,0x06,0x00},{0x00,0x06,0x06,0x00,0x06,0x04,0x08},
    {0x02,0x04,0x08,0x10,0x08,0x04,0x02},{0x00,0x00,0x1F,0x00,0x1F,0x00,0x00},{0x08,0x04,0x02,0x01,0x02,0x04,0x08},{0x0E,0x11,0x01,0x02,0x04,0x00,0x04},
    {0x0E,0x11,0x01,0x0D,0x15,0x15,0x0E},{0x0E,0x11,0x11,0x1F,0x11,0x11,0x11},{0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E},{0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},
    {0x1C,0x12,0x11,0x11,0x11,0x12,0x1C},{0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},{0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},{0x0E,0x11,0x10,0x17,0x11,0x11,0x0E},
    {0x11,0x11,0x11,0x1F,0x11,0x11,0x11},{0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},{0x07,0x02,0x02,0x02,0x02,0x12,0x0C},{0x11,0x12,0x14,0x18,0x14,0x12,0x11},
    {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},{0x11,0x1B,0x15,0x11,0x11,0x11,0x11},{0x11,0x19,0x15,0x13,0x11,0x11,0x11},{0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},{0x0E,0x11,0x11,0x11,0x15,0x12,0x0D},{0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},{0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E},
    {0x1F,0x04,0x04,0x04,0x04,0x04,0x04},{0x11,0x11,0x11,0x11,0x11,0x11,0x0E},{0x11,0x11,0x11,0x11,0x11,0x0A,0x04},{0x11,0x11,0x11,0x11,0x15,0x1B,0x11},
    {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},{0x11,0x11,0x0A,0x04,0x04,0x04,0x04},{0x1F,0x01,0x02,0x04,0x08,0x10,0x1F},{0x0E,0x08,0x08,0x08,0x08,0x08,0x0E},
    {0x10,0x08,0x04,0x02,0x01,0x00,0x00},{0x0E,0x02,0x02,0x02,0x02,0x02,0x0E},{0x04,0x0A,0x11,0x00,0x00,0x00,0x00},{0x00,0x00,0x00,0x00,0x00,0x00,0x1F},
    {0x08,0x04,0x02,0x00,0x00,0x00,0x00},{0x00,0x0E,0x01,0x0F,0x11,0x11,0x0F},{0x10,0x10,0x1E,0x11,0x11,0x11,0x1E},{0x00,0x0E,0x11,0x10,0x10,0x11,0x0E},{0x01,0x01,0x0F,0x11,0x11,0x11,0x0F},
    {0x00,0x0E,0x11,0x1F,0x10,0x11,0x0E},{0x06,0x09,0x08,0x1C,0x08,0x08,0x08},{0x00,0x0F,0x11,0x11,0x0F,0x01,0x0E},{0x10,0x10,0x1E,0x11,0x11,0x11,0x11},
    {0x04,0x00,0x0C,0x04,0x04,0x04,0x0E},{0x02,0x00,0x06,0x02,0x02,0x12,0x0C},{0x10,0x10,0x11,0x12,0x1C,0x12,0x11},{0x0C,0x04,0x04,0x04,0x04,0x04,0x0E},{0x00,0x1A,0x15,0x15,0x15,0x15,0x15},
    {0x00,0x1E,0x11,0x11,0x11,0x11,0x11},{0x00,0x0E,0x11,0x11,0x11,0x11,0x0E},{0x00,0x1E,0x11,0x11,0x1E,0x10,0x10},{0x00,0x0F,0x11,0x11,0x0F,0x01,0x01},{0x00,0x16,0x19,0x10,0x10,0x10,0x10},{0x00,0x0F,0x10,0x0E,0x01,0x01,0x1E},
    {0x08,0x08,0x1C,0x08,0x08,0x09,0x06},{0x00,0x11,0x11,0x11,0x11,0x11,0x0F},{0x00,0x11,0x11,0x11,0x11,0x0A,0x04},{0x00,0x11,0x11,0x11,0x15,0x1B,0x11},{0x00,0x11,0x0A,0x04,0x0A,0x11,0x11},{0x00,0x11,0x11,0x11,0x0F,0x01,0x0E},{0x00,0x1F,0x02,0x04,0x08,0x10,0x1F},
    {0x03,0x04,0x04,0x18,0x04,0x04,0x03},{0x04,0x04,0x04,0x04,0x04,0x04,0x04},{0x18,0x04,0x04,0x03,0x04,0x04,0x18},{0x00,0x00,0x0D,0x12,0x00,0x00,0x00},{0,0,0,0,0,0,0}
};

struct Image {
    int w = 0, h = 0;
    std::vector<uint8_t> rgba; // R,G,B,A
};

static uint32_t pack_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return (uint32_t(r) << 24) | (uint32_t(g) << 16) | (uint32_t(b) << 8) | uint32_t(a);
}

static void put_px(Image& img, int x, int y, uint32_t c) {
    if (x < 0 || y < 0 || x >= img.w || y >= img.h) return;
    const size_t i = (size_t(y) * size_t(img.w) + size_t(x)) * 4;
    img.rgba[i + 0] = uint8_t((c >> 24) & 255);
    img.rgba[i + 1] = uint8_t((c >> 16) & 255);
    img.rgba[i + 2] = uint8_t((c >> 8) & 255);
    img.rgba[i + 3] = uint8_t((c) & 255);
}

static void fill_rect(Image& img, int x, int y, int w, int h, uint32_t c) {
    for (int yy = 0; yy < h; ++yy)
        for (int xx = 0; xx < w; ++xx)
            put_px(img, x + xx, y + yy, c);
}

static void rect(Image& img, int x, int y, int w, int h, uint32_t c, int t = 1) {
    for (int i = 0; i < t; ++i) {
        for (int xx = 0; xx < w; ++xx) { put_px(img, x + xx, y + i, c); put_px(img, x + xx, y + h - 1 - i, c); }
        for (int yy = 0; yy < h; ++yy) { put_px(img, x + i, y + yy, c); put_px(img, x + w - 1 - i, y + yy, c); }
    }
}

static void draw_char(Image& img, int x, int y, char ch, int scale, uint32_t fg) {
    const unsigned uc = (unsigned)(unsigned char)ch;
    if (uc < 32 || uc > 127) ch = '?';
    const int idx = int((unsigned char)ch) - 32;
    const uint8_t* rows = FONT5X7[idx];
    for (int ry = 0; ry < 7; ++ry) {
        const uint8_t bits = rows[ry];
        for (int rx = 0; rx < 5; ++rx) {
            const bool on = ((bits >> (4 - rx)) & 1) != 0;
            if (!on) continue;
            fill_rect(img, x + rx * scale, y + ry * scale, scale, scale, fg);
        }
    }
}

static void draw_text(Image& img, int x, int y, const std::string& s, int scale, uint32_t fg) {
    int cx = x;
    for (char ch : s) {
        if (ch == '\n') { y += 8 * scale; cx = x; continue; }
        draw_char(img, cx, y, ch, scale, fg);
        cx += 6 * scale;
    }
}

static uint8_t clamp_u8(int v) { return uint8_t(std::max(0, std::min(255, v))); }

static uint32_t lerp(uint32_t a, uint32_t b, double t) {
    t = std::max(0.0, std::min(1.0, t));
    const int ar = int((a >> 24) & 255), ag = int((a >> 16) & 255), ab = int((a >> 8) & 255), aa = int(a & 255);
    const int br = int((b >> 24) & 255), bg = int((b >> 16) & 255), bb = int((b >> 8) & 255), ba = int(b & 255);
    const int rr  = int(ar + (br - ar) * t);
    const int gg  = int(ag + (bg - ag) * t);
    const int bb2 = int(ab + (bb - ab) * t);
    const int aa2 = int(aa + (ba - aa) * t);
    return pack_rgba(clamp_u8(rr), clamp_u8(gg), clamp_u8(bb2), clamp_u8(aa2));
}

static uint32_t year_color(int year, int year_min, int year_max) {
    if (year_max <= year_min) return pack_rgba(255, 210, 80, 255);
    double t = double(year - year_min) / double(year_max - year_min);
    t = std::max(0.0, std::min(1.0, t));
    const uint32_t red    = pack_rgba(235,  70,  70, 255);
    const uint32_t orange = pack_rgba(245, 155,  60, 255);
    const uint32_t yellow = pack_rgba(250, 220,  90, 255);
    if (t < 0.55) return lerp(red, orange, t / 0.55);
    return lerp(orange, yellow, (t - 0.55) / 0.45);
}

static uint32_t none_color() { return pack_rgba(60, 200, 110, 255); }

static std::string trim(std::string s) {
    auto issp = [](unsigned char c) { return std::isspace(c) != 0; };
    while (!s.empty() && issp((unsigned char)s.front())) s.erase(s.begin());
    while (!s.empty() && issp((unsigned char)s.back())) s.pop_back();
    return s;
}

struct Row {
    std::string country;
    double rho = 0.0;
    double lambda_phi = 0.0;
    double lambda_m = 0.0;
    bool has_year = false;
    int year = 0;
};

static bool parse_csv(const std::string& path, std::vector<Row>& out) {
    std::ifstream f(path);
    if (!f) return false;

    std::string line;
    if (!std::getline(f, line)) return false; // header

    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string tok;
        std::vector<std::string> cols;
        while (std::getline(ss, tok, ',')) cols.push_back(trim(tok));
        if (cols.size() < 5) continue;

        Row r;
        r.country = cols[0];
        r.rho = std::stod(cols[1]);
        r.lambda_phi = std::stod(cols[2]);
        r.lambda_m = std::stod(cols[3]);

        if (cols[4] == "NONE" || cols[4] == "None" || cols[4] == "none") {
            r.has_year = false;
            r.year = 0;
        } else {
            r.has_year = true;
            r.year = std::stoi(cols[4]);
        }
        out.push_back(r);
    }
    return true;
}

struct Grid {
    std::string country;
    std::vector<double> rhos;
    std::vector<double> lphis;
    std::vector<int> v; // year or -1
};

static Grid build_grid(const std::vector<Row>& rows, const std::string& country) {
    Grid g;
    g.country = country;

    std::vector<double> rhos, lphis;
    for (const auto& r : rows) {
        if (r.country != country) continue;
        rhos.push_back(r.rho);
        lphis.push_back(r.lambda_phi);
    }
    std::sort(rhos.begin(), rhos.end());
    rhos.erase(std::unique(rhos.begin(), rhos.end()), rhos.end());
    std::sort(lphis.begin(), lphis.end());
    lphis.erase(std::unique(lphis.begin(), lphis.end()), lphis.end());

    g.rhos = rhos;
    g.lphis = lphis;
    g.v.assign(rhos.size() * lphis.size(), -1);

    auto idx_r = [&](double x) {
        auto it = std::lower_bound(g.rhos.begin(), g.rhos.end(), x);
        return int(it - g.rhos.begin());
    };
    auto idx_c = [&](double x) {
        auto it = std::lower_bound(g.lphis.begin(), g.lphis.end(), x);
        return int(it - g.lphis.begin());
    };

    for (const auto& r : rows) {
        if (r.country != country) continue;
        const int rr = idx_r(r.rho);
        const int cc = idx_c(r.lambda_phi);
        const int val = r.has_year ? r.year : -1;
        g.v[size_t(rr) * g.lphis.size() + size_t(cc)] = val;
    }
    return g;
}

static void draw_panel(
    Image& img,
    int x0, int y0, int panel_w, int panel_h,
    const Grid& g,
    int year_min, int year_max
) {
    const uint32_t bg = pack_rgba(10, 12, 16, 255);
    const uint32_t frame = pack_rgba(60, 70, 85, 255);
    const uint32_t text = pack_rgba(230, 235, 245, 255);
    const uint32_t text_dim = pack_rgba(160, 170, 185, 255);

    fill_rect(img, x0, y0, panel_w, panel_h, bg);
    rect(img, x0, y0, panel_w, panel_h, frame, 2);

    draw_text(img, x0 + 18, y0 + 14, g.country, 4, text);

    const int m = int(g.rhos.size());
    const int n = int(g.lphis.size());

    const int grid_x = x0 + 50;
    const int grid_y = y0 + 70;
    const int grid_w = panel_w - 80;
    const int grid_h = panel_h - 130;

    const int cell_w = (n > 0) ? (grid_w / n) : 1;
    const int cell_h = (m > 0) ? (grid_h / m) : 1;

    draw_text(img, x0 + 10, y0 + 55, "rho", 2, text_dim);
    draw_text(img, x0 + 10, y0 + panel_h - 45, "lambda_phi", 2, text_dim);

    for (int r = 0; r < m; ++r) {
        const int rr = (m - 1 - r);
        for (int c = 0; c < n; ++c) {
            const int val = g.v[size_t(rr) * size_t(n) + size_t(c)];
            const uint32_t col = (val < 0) ? none_color() : year_color(val, year_min, year_max);

            const int cx = grid_x + c * cell_w;
            const int cy = grid_y + r * cell_h;

            fill_rect(img, cx, cy, cell_w - 2, cell_h - 2, col);
            rect(img, cx, cy, cell_w - 2, cell_h - 2, pack_rgba(20, 24, 32, 255), 2);

            const std::string s = (val < 0) ? "NONE" : std::to_string(val);
            const uint32_t fg = pack_rgba(5, 8, 12, 255);
            const int scale = 3;

            const int tw = int(s.size()) * 6 * scale;
            const int th = 7 * scale;
            const int tx = cx + (cell_w - 2 - tw) / 2;
            const int ty = cy + (cell_h - 2 - th) / 2;
            draw_text(img, tx, ty, s, scale, fg);
        }
    }

    for (int r = 0; r < m; ++r) {
        const double rho = g.rhos[m - 1 - r];
        std::ostringstream os;
        os << std::fixed << std::setprecision(2) << rho;
        draw_text(img, x0 + 10, grid_y + r * cell_h + (cell_h - 14) / 2, os.str(), 2, text_dim);
    }

    for (int c = 0; c < n; ++c) {
        const double lp = g.lphis[c];
        std::ostringstream os;
        os << std::fixed << std::setprecision(2) << lp;
        const std::string s = os.str();
        const int tx = grid_x + c * cell_w + (cell_w - int(s.size()) * 12) / 2;
        draw_text(img, tx, grid_y + m * cell_h + 10, s, 2, text_dim);
    }
}

static void draw_legend(Image& img, int x0, int y0, int w, int h, int year_min, int year_max) {
    const uint32_t bg = pack_rgba(10, 12, 16, 255);
    const uint32_t frame = pack_rgba(60, 70, 85, 255);
    const uint32_t text = pack_rgba(230, 235, 245, 255);
    const uint32_t text_dim = pack_rgba(160, 170, 185, 255);

    fill_rect(img, x0, y0, w, h, bg);
    rect(img, x0, y0, w, h, frame, 2);

    draw_text(img, x0 + 16, y0 + 12, "Legend: collapse year", 3, text);

    const int bar_x = x0 + 20;
    const int bar_y = y0 + 55;
    const int bar_w = w - 40;
    const int bar_h = 22;

    for (int i = 0; i < bar_w; ++i) {
        const double t = (bar_w <= 1) ? 0.0 : double(i) / double(bar_w - 1);
        const int y = year_min + int((year_max - year_min) * t + 0.5);
        const uint32_t c = year_color(y, year_min, year_max);
        fill_rect(img, bar_x + i, bar_y, 1, bar_h, c);
    }
    rect(img, bar_x, bar_y, bar_w, bar_h, pack_rgba(20, 24, 32, 255), 2);

    draw_text(img, bar_x, bar_y + 30, std::to_string(year_min) + " (earlier)", 2, text_dim);
    {
        const std::string s = std::to_string(year_max) + " (later)";
        draw_text(img, bar_x + bar_w - int(s.size()) * 12, bar_y + 30, s, 2, text_dim);
    }

    const int chip_x = x0 + 20;
    const int chip_y = y0 + 95;
    fill_rect(img, chip_x, chip_y, 30, 18, none_color());
    rect(img, chip_x, chip_y, 30, 18, pack_rgba(20, 24, 32, 255), 2);
    draw_text(img, chip_x + 40, chip_y - 2, "NONE = no collapse", 2, text_dim);
}

static void draw_header(Image& img, const std::string& title) {
    const uint32_t bg = pack_rgba(8, 10, 14, 255);
    const uint32_t text = pack_rgba(230, 235, 245, 255);
    const uint32_t text_dim = pack_rgba(160, 170, 185, 255);

    fill_rect(img, 0, 0, img.w, 90, bg);
    draw_text(img, 30, 18, title, 4, text);
    draw_text(img, 30, 58, "GREEN = NONE (no collapse).  WARM = earlier->later collapse.", 2, text_dim);
}

static std::wstring widen_ascii(const std::string& s) {
    // ASCII-only is fine for your report/out names.
    std::wstring w;
    w.reserve(s.size());
    for (unsigned char c : s) w.push_back((wchar_t)c);
    return w;
}

static bool write_png_wic(const std::string& path, const Image& img) {
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    const bool did_coinit = SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE;
    if (!(SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE)) return false;

    IWICImagingFactory* factory = nullptr;
    hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
                          IID_IWICImagingFactory, (void**)&factory);
    if (FAILED(hr) || !factory) { if (did_coinit && hr != RPC_E_CHANGED_MODE) CoUninitialize(); return false; }

    IWICStream* stream = nullptr;
    hr = factory->CreateStream(&stream);
    if (FAILED(hr) || !stream) { factory->Release(); if (did_coinit && hr != RPC_E_CHANGED_MODE) CoUninitialize(); return false; }

    const std::wstring wpath = widen_ascii(path);
    hr = stream->InitializeFromFilename(wpath.c_str(), GENERIC_WRITE);
    if (FAILED(hr)) { stream->Release(); factory->Release(); if (did_coinit && hr != RPC_E_CHANGED_MODE) CoUninitialize(); return false; }

    IWICBitmapEncoder* encoder = nullptr;
    hr = factory->CreateEncoder(GUID_ContainerFormatPng, NULL, &encoder);
    if (FAILED(hr) || !encoder) { stream->Release(); factory->Release(); if (did_coinit && hr != RPC_E_CHANGED_MODE) CoUninitialize(); return false; }

    hr = encoder->Initialize(stream, WICBitmapEncoderNoCache);
    if (FAILED(hr)) { encoder->Release(); stream->Release(); factory->Release(); if (did_coinit && hr != RPC_E_CHANGED_MODE) CoUninitialize(); return false; }

    IWICBitmapFrameEncode* frame = nullptr;
    IPropertyBag2* props = nullptr;
    hr = encoder->CreateNewFrame(&frame, &props);
    if (props) props->Release();
    if (FAILED(hr) || !frame) { encoder->Release(); stream->Release(); factory->Release(); if (did_coinit && hr != RPC_E_CHANGED_MODE) CoUninitialize(); return false; }

    hr = frame->Initialize(NULL);
    if (FAILED(hr)) { frame->Release(); encoder->Release(); stream->Release(); factory->Release(); if (did_coinit && hr != RPC_E_CHANGED_MODE) CoUninitialize(); return false; }

    hr = frame->SetSize((UINT)img.w, (UINT)img.h);
    if (FAILED(hr)) { frame->Release(); encoder->Release(); stream->Release(); factory->Release(); if (did_coinit && hr != RPC_E_CHANGED_MODE) CoUninitialize(); return false; }

    WICPixelFormatGUID fmt = GUID_WICPixelFormat32bppBGRA;
    hr = frame->SetPixelFormat(&fmt);
    if (FAILED(hr)) { frame->Release(); encoder->Release(); stream->Release(); factory->Release(); if (did_coinit && hr != RPC_E_CHANGED_MODE) CoUninitialize(); return false; }

    // Convert RGBA -> BGRA for WIC
    std::vector<uint8_t> bgra;
    bgra.resize((size_t)img.w * (size_t)img.h * 4);
    for (size_t i = 0; i < (size_t)img.w * (size_t)img.h; ++i) {
        const uint8_t r = img.rgba[i*4 + 0];
        const uint8_t g = img.rgba[i*4 + 1];
        const uint8_t b = img.rgba[i*4 + 2];
        const uint8_t a = img.rgba[i*4 + 3];
        bgra[i*4 + 0] = b;
        bgra[i*4 + 1] = g;
        bgra[i*4 + 2] = r;
        bgra[i*4 + 3] = a;
    }

    hr = frame->WritePixels((UINT)img.h, (UINT)(img.w * 4), (UINT)bgra.size(), bgra.data());
    if (FAILED(hr)) { frame->Release(); encoder->Release(); stream->Release(); factory->Release(); if (did_coinit && hr != RPC_E_CHANGED_MODE) CoUninitialize(); return false; }

    hr = frame->Commit();
    if (FAILED(hr)) { frame->Release(); encoder->Release(); stream->Release(); factory->Release(); if (did_coinit && hr != RPC_E_CHANGED_MODE) CoUninitialize(); return false; }

    hr = encoder->Commit();

    frame->Release();
    encoder->Release();
    stream->Release();
    factory->Release();
    if (did_coinit && hr != RPC_E_CHANGED_MODE) CoUninitialize();

    return SUCCEEDED(hr);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: heatmap_exporter <input.csv> <output_prefix>\n";
        return 2;
    }
    const std::string csv_path = argv[1];
    const std::string out_prefix = argv[2];

    std::vector<Row> rows;
    if (!parse_csv(csv_path, rows)) {
        std::cerr << "ERROR: cannot open input csv: " << csv_path << "\n";
        return 3;
    }

    Grid usa = build_grid(rows, "USA");
    Grid euu = build_grid(rows, "EUU");

    int year_min = 999999, year_max = -999999;
    for (const auto& r : rows) {
        if (!r.has_year) continue;
        year_min = (std::min)(year_min, r.year);
        year_max = (std::max)(year_max, r.year);
    }
    if (year_max < year_min) { year_min = 2000; year_max = 2024; }

    Image img;
    img.w = 1600;
    img.h = 900;
    img.rgba.assign((size_t)img.w * (size_t)img.h * 4, 0);

    fill_rect(img, 0, 0, img.w, img.h, pack_rgba(6, 8, 12, 255));
    draw_header(img, "MAX-Core | WorldBank Stability Surface (collapse_year)");

    const int panel_w = 720;
    const int panel_h = 430;
    const int left_x = 60;
    const int top_y = 120;
    const int right_x = left_x + panel_w + 40;

    draw_panel(img, left_x, top_y, panel_w, panel_h, usa, year_min, year_max);
    draw_panel(img, right_x, top_y, panel_w, panel_h, euu, year_min, year_max);

    draw_legend(img, left_x, top_y + panel_h + 30, panel_w + 40 + panel_w, 220, year_min, year_max);

    const std::string out_png = out_prefix + ".png";
    if (!write_png_wic(out_png, img)) {
        std::cerr << "ERROR: failed to write png: " << out_png << "\n";
        return 4;
    }
    std::cout << "Wrote: " << out_png << "\n";
    return 0;
}
