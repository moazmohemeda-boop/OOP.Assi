#ifdef BPS_UNITY

#include "filters.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>

static const double kPi = 3.14159265358979323846;

static inline unsigned char clamp8(double v)
{
    if (v < 0.0) return 0;
    if (v > 255.0) return 255;
    return (unsigned char)(v + 0.5);
}

static inline int clampi(int v, int lo, int hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

static double bilinear(Image& s, double fx, double fy, int c)
{
    const int w = s.width, h = s.height;
    int xa = (int)std::floor(fx), ya = (int)std::floor(fy);
    double ax = fx - xa, ay = fy - ya;
    int xb = clampi(xa + 1, 0, w - 1), yb = clampi(ya + 1, 0, h - 1);
    xa = clampi(xa, 0, w - 1);
    ya = clampi(ya, 0, h - 1);
    double p00 = s(xa, ya, c), p10 = s(xb, ya, c);
    double p01 = s(xa, yb, c), p11 = s(xb, yb, c);
    return (p00 * (1 - ax) + p10 * ax) * (1 - ay) + (p01 * (1 - ax) + p11 * ax) * ay;
}

// ---------------------------------------------------------------- 1
Image grayscale(Image img)
{
    for (int y = 0; y < img.height; ++y)
        for (int x = 0; x < img.width; ++x) {
            unsigned char g = clamp8(0.299 * img(x, y, 0) + 0.587 * img(x, y, 1) + 0.114 * img(x, y, 2));
            for (int c = 0; c < 3; ++c) img(x, y, c) = g;
        }
    return img;
}

// ---------------------------------------------------------------- 2
Image blackAndWhite(Image img)
{
    for (int y = 0; y < img.height; ++y)
        for (int x = 0; x < img.width; ++x) {
            double g = 0.299 * img(x, y, 0) + 0.587 * img(x, y, 1) + 0.114 * img(x, y, 2);
            unsigned char v = g >= 128.0 ? 255 : 0;
            for (int c = 0; c < 3; ++c) img(x, y, c) = v;
        }
    return img;
}

// ---------------------------------------------------------------- 3
Image invertColors(Image img)
{
    for (int y = 0; y < img.height; ++y)
        for (int x = 0; x < img.width; ++x)
            for (int c = 0; c < 3; ++c) img(x, y, c) = (unsigned char)(255 - img(x, y, c));
    return img;
}

// ---------------------------------------------------------------- 4
Image addFrame(Image img, int thickness, int r, int g, int b)
{
    const int w = img.width, h = img.height;
    const int t = std::max(1, thickness);
    Image out(w + 2 * t, h + 2 * t);
    for (int y = 0; y < out.height; ++y)
        for (int x = 0; x < out.width; ++x) {
            out(x, y, 0) = (unsigned char)clampi(r, 0, 255);
            out(x, y, 1) = (unsigned char)clampi(g, 0, 255);
            out(x, y, 2) = (unsigned char)clampi(b, 0, 255);
        }
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            for (int c = 0; c < 3; ++c) out(x + t, y + t, c) = img(x, y, c);
    return out;
}

// ---------------------------------------------------------------- 5
Image flipImage(Image img, bool horizontal)
{
    const int w = img.width, h = img.height;
    Image out(w, h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            int sx = horizontal ? (w - 1 - x) : x;
            int sy = horizontal ? y : (h - 1 - y);
            for (int c = 0; c < 3; ++c) out(x, y, c) = img(sx, sy, c);
        }
    return out;
}

// ---------------------------------------------------------------- 6
Image rotateImage(Image img, int degrees)
{
    const int w = img.width, h = img.height;
    int d = ((degrees % 360) + 360) % 360;
    if (d == 90) {
        Image out(h, w);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                for (int c = 0; c < 3; ++c) out(h - 1 - y, x, c) = img(x, y, c);
        return out;
    }
    if (d == 180) {
        Image out(w, h);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                for (int c = 0; c < 3; ++c) out(w - 1 - x, h - 1 - y, c) = img(x, y, c);
        return out;
    }
    if (d == 270) {
        Image out(h, w);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                for (int c = 0; c < 3; ++c) out(y, w - 1 - x, c) = img(x, y, c);
        return out;
    }
    return img;
}

// ---------------------------------------------------------------- 7
Image adjustBrightness(Image img, int percent)
{
    const double f = 1.0 + percent / 100.0;
    for (int y = 0; y < img.height; ++y)
        for (int x = 0; x < img.width; ++x)
            for (int c = 0; c < 3; ++c) img(x, y, c) = clamp8(img(x, y, c) * f);
    return img;
}

// ---------------------------------------------------------------- 8
Image resizeImage(Image img, int newW, int newH)
{
    newW = std::max(1, newW);
    newH = std::max(1, newH);
    Image out(newW, newH);
    const double rx = (double)img.width / newW, ry = (double)img.height / newH;
    for (int y = 0; y < newH; ++y)
        for (int x = 0; x < newW; ++x) {
            double fx = (x + 0.5) * rx - 0.5, fy = (y + 0.5) * ry - 0.5;
            for (int c = 0; c < 3; ++c) out(x, y, c) = clamp8(bilinear(img, fx, fy, c));
        }
    return out;
}

// ---------------------------------------------------------------- 9
Image mergeImages(Image a, Image b)
{
    Image bb = resizeImage(b, a.width, a.height);   // second image is fitted to the first
    Image out(a.width, a.height);
    for (int y = 0; y < a.height; ++y)
        for (int x = 0; x < a.width; ++x)
            for (int c = 0; c < 3; ++c)
                out(x, y, c) = (unsigned char)(((int)a(x, y, c) + (int)bb(x, y, c)) / 2);
    return out;
}

// ---------------------------------------------------------------- 10
Image detectEdges(Image img)
{
    const int w = img.width, h = img.height;
    std::vector<int> g((size_t)w * h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            g[(size_t)y * w + x] = (int)(0.299 * img(x, y, 0) + 0.587 * img(x, y, 1) + 0.114 * img(x, y, 2));

    Image out(w, h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            int x0 = clampi(x - 1, 0, w - 1), x1 = clampi(x + 1, 0, w - 1);
            int y0 = clampi(y - 1, 0, h - 1), y1 = clampi(y + 1, 0, h - 1);
            int gx = -g[(size_t)y0 * w + x0] + g[(size_t)y0 * w + x1]
                     - 2 * g[(size_t)y * w + x0] + 2 * g[(size_t)y * w + x1]
                     - g[(size_t)y1 * w + x0] + g[(size_t)y1 * w + x1];
            int gy = -g[(size_t)y0 * w + x0] - 2 * g[(size_t)y0 * w + x] - g[(size_t)y0 * w + x1]
                     + g[(size_t)y1 * w + x0] + 2 * g[(size_t)y1 * w + x] + g[(size_t)y1 * w + x1];
            double mag = std::sqrt((double)gx * gx + (double)gy * gy);
            unsigned char v = mag > 100.0 ? 0 : 255;       // black edges on white
            for (int c = 0; c < 3; ++c) out(x, y, c) = v;
        }
    return out;
}

// ---------------------------------------------------------------- 11
Image cropImage(Image img, int x, int y, int w, int h)
{
    x = clampi(x, 0, img.width - 1);
    y = clampi(y, 0, img.height - 1);
    w = clampi(w, 1, img.width - x);
    h = clampi(h, 1, img.height - y);
    Image out(w, h);
    for (int j = 0; j < h; ++j)
        for (int i = 0; i < w; ++i)
            for (int c = 0; c < 3; ++c) out(i, j, c) = img(x + i, y + j, c);
    return out;
}

// ---------------------------------------------------------------- 12
static Image boxBlurPass(Image& s, int r)
{
    const int w = s.width, h = s.height;
    const size_t W1 = (size_t)w + 1;
    Image out(w, h);
    std::vector<long long> I(W1 * ((size_t)h + 1));
    for (int c = 0; c < 3; ++c) {
        std::fill(I.begin(), I.end(), 0LL);
        for (int y = 0; y < h; ++y) {
            long long row = 0;
            for (int x = 0; x < w; ++x) {
                row += s(x, y, c);
                I[(size_t)(y + 1) * W1 + (x + 1)] = I[(size_t)y * W1 + (x + 1)] + row;
            }
        }
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x) {
                int xa = std::max(0, x - r), xb = std::min(w - 1, x + r);
                int ya = std::max(0, y - r), yb = std::min(h - 1, y + r);
                long long area = (long long)(xb - xa + 1) * (yb - ya + 1);
                long long sum = I[(size_t)(yb + 1) * W1 + (xb + 1)] - I[(size_t)ya * W1 + (xb + 1)]
                                - I[(size_t)(yb + 1) * W1 + xa] + I[(size_t)ya * W1 + xa];
                out(x, y, c) = (unsigned char)(sum / area);
            }
    }
    return out;
}

Image strongBlur(Image img)
{
    int r = std::max(6, std::min(img.width, img.height) / 40);
    Image t = boxBlurPass(img, r);
    return boxBlurPass(t, r);
}

// ---------------------------------------------------------------- 13
Image wanoSunlight(Image img)
{
    for (int y = 0; y < img.height; ++y)
        for (int x = 0; x < img.width; ++x) {
            double r = img(x, y, 0), g = img(x, y, 1), b = img(x, y, 2);
            img(x, y, 0) = clamp8(r * 1.12 + 28);
            img(x, y, 1) = clamp8(g * 1.05 + 14);
            img(x, y, 2) = clamp8(b * 0.82);
        }
    return img;
}

// ---------------------------------------------------------------- 14
Image oldTVEffect(Image img)
{
    const int w = img.width, h = img.height;
    std::mt19937 rng(1234);
    std::uniform_int_distribution<int> nd(-25, 25);
    const double cx = w / 2.0, cy = h / 2.0;
    const double maxd = std::sqrt(cx * cx + cy * cy);
    const int band = std::max(2, h / 200);

    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            double r = img(x, y, 0), g = img(x, y, 1), b = img(x, y, 2);
            double gray = 0.299 * r + 0.587 * g + 0.114 * b;
            r = r * 0.7 + gray * 0.3;
            g = g * 0.7 + gray * 0.3;
            b = b * 0.7 + gray * 0.3;
            double scan = ((y / band) % 2 == 0) ? 1.0 : 0.78;
            double d = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy)) / maxd;
            double vig = 1.0 - 0.45 * d * d;
            double n = (double)nd(rng);
            img(x, y, 0) = clamp8((r + n) * scan * vig);
            img(x, y, 1) = clamp8((g + n) * scan * vig * 1.04);
            img(x, y, 2) = clamp8((b + n) * scan * vig);
        }
    return img;
}

// ---------------------------------------------------------------- 15
Image purpleTint(Image img)
{
    for (int y = 0; y < img.height; ++y)
        for (int x = 0; x < img.width; ++x) {
            double r = img(x, y, 0), g = img(x, y, 1), b = img(x, y, 2);
            img(x, y, 0) = clamp8(r * 0.9 + 30);
            img(x, y, 1) = clamp8(g * 0.7);
            img(x, y, 2) = clamp8(b * 1.1 + 45);
        }
    return img;
}

// ---------------------------------------------------------------- 16
Image infraredEffect(Image img)
{
    for (int y = 0; y < img.height; ++y)
        for (int x = 0; x < img.width; ++x) {
            double r = img(x, y, 0), g = img(x, y, 1), b = img(x, y, 2);
            img(x, y, 0) = clamp8(g * 1.5 + r * 0.2);
            img(x, y, 1) = clamp8(r * 0.35 + g * 0.25 + b * 0.15);
            img(x, y, 2) = clamp8(b * 0.55 + (255.0 - g) * 0.3);
        }
    return img;
}

// ---------------------------------------------------------------- 17
Image skewImage(Image img, double angleDegrees)
{
    const int w = img.width, h = img.height;
    double a = std::max(-60.0, std::min(60.0, angleDegrees));
    double t = std::tan(a * kPi / 180.0);
    int extra = (int)std::ceil(std::fabs(t) * h);
    const int nw = w + extra;
    Image out(nw, h);
    for (int y = 0; y < h; ++y) {
        double off = (t >= 0.0) ? t * y : -t * (h - 1 - y);
        for (int x = 0; x < nw; ++x) {
            double sx = x - off;
            if (sx < 0.0 || sx > w - 1) {
                out(x, y, 0) = 0; out(x, y, 1) = 0; out(x, y, 2) = 0;
            } else {
                int xa = (int)std::floor(sx);
                int xb = std::min(xa + 1, w - 1);
                double f = sx - xa;
                for (int c = 0; c < 3; ++c)
                    out(x, y, c) = clamp8(img(xa, y, c) * (1.0 - f) + img(xb, y, c) * f);
            }
        }
    }
    return out;
}

// ---------------------------------------------------------------- 18
Image oilPainting(Image img)
{
    const int w = img.width, h = img.height;
    const int R = clampi(std::min(w, h) / 150, 2, 5);
    const int L = 20;

    std::vector<unsigned char> buf((size_t)w * h * 3);
    std::vector<int> lvl((size_t)w * h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            size_t i = (size_t)y * w + x;
            int r = img(x, y, 0), g = img(x, y, 1), b = img(x, y, 2);
            buf[i * 3] = (unsigned char)r;
            buf[i * 3 + 1] = (unsigned char)g;
            buf[i * 3 + 2] = (unsigned char)b;
            lvl[i] = ((r + g + b) / 3) * (L - 1) / 255;
        }

    Image out(w, h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            int cnt[L], sr[L], sg[L], sb[L];
            for (int k = 0; k < L; ++k) { cnt[k] = 0; sr[k] = 0; sg[k] = 0; sb[k] = 0; }
            for (int dy = -R; dy <= R; ++dy) {
                int yy = clampi(y + dy, 0, h - 1);
                for (int dx = -R; dx <= R; ++dx) {
                    int xx = clampi(x + dx, 0, w - 1);
                    size_t i = (size_t)yy * w + xx;
                    int k = lvl[i];
                    cnt[k]++;
                    sr[k] += buf[i * 3];
                    sg[k] += buf[i * 3 + 1];
                    sb[k] += buf[i * 3 + 2];
                }
            }
            int best = 0;
            for (int k = 1; k < L; ++k)
                if (cnt[k] > cnt[best]) best = k;
            out(x, y, 0) = (unsigned char)(sr[best] / cnt[best]);
            out(x, y, 1) = (unsigned char)(sg[best] / cnt[best]);
            out(x, y, 2) = (unsigned char)(sb[best] / cnt[best]);
        }
    return out;
}

#endif // BPS_UNITY