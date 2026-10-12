#ifdef BPS_UNITY

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

#include "mainwindow.h"
#include "filters.h"   // includes Image_Class.h -- keep AFTER the Windows headers

// ============================================================================
//  Constants / theme
// ============================================================================
enum { ID_OPEN = 1001, ID_SAVE, ID_UNDO, ID_RESET, ID_FILTER_BASE = 2000 };
static const int SIDE_W = 320;
static const int kFilterCount = 18;
static const char* kFilterNames[kFilterCount] = {
    "Grayscale", "Black & White", "Invert Colors", "Add Frame", "Flip Image",
    "Rotate Image", "Brightness", "Resize Image", "Merge Images", "Detect Edges",
    "Crop Image", "Strong Blur", "Wano Sunlight", "Old TV Effect", "Purple Tint",
    "Infrared Effect", "Skew Image", "Oil Painting"
};

static const COLORREF COL_BG       = RGB(13, 17, 23);
static const COLORREF COL_PANEL    = RGB(17, 22, 30);
static const COLORREF COL_SIDE     = RGB(24, 29, 38);
static const COLORREF COL_BTN      = RGB(38, 46, 60);
static const COLORREF COL_BTN_HOT  = RGB(52, 63, 82);
static const COLORREF COL_BTN_DN   = RGB(30, 37, 49);
static const COLORREF COL_BTN_OFF  = RGB(29, 35, 45);
static const COLORREF COL_BORDER   = RGB(48, 56, 71);
static const COLORREF COL_ACCENT   = RGB(56, 139, 253);
static const COLORREF COL_ACCENT_L = RGB(121, 184, 255);
static const COLORREF COL_TEXT     = RGB(230, 237, 243);
static const COLORREF COL_MUTED    = RGB(139, 148, 158);
static const COLORREF COL_DISABLED = RGB(88, 97, 110);

// ============================================================================
//  Globals
// ============================================================================
static HINSTANCE g_hInst = nullptr;
static HWND g_main = nullptr;
static int  g_dpi = 96;
static HFONT g_fontUI = nullptr, g_fontBold = nullptr, g_fontTitle = nullptr,
             g_fontSmall = nullptr, g_fontStatus = nullptr;
static HBRUSH g_brSide = nullptr, g_brEdit = nullptr;

static Image g_current, g_original;
static bool  g_hasImage = false;
static std::vector<Image> g_undo;
static std::string g_filePath;
static std::string g_status = "Open an image or drop one here to get started.";
static int   g_activeFilter = -1;

static HBITMAP g_dib = nullptr;       // full-size DIB of g_current
static HBITMAP g_scaled = nullptr;    // cached scaled bitmap for the canvas
static int  g_scW = 0, g_scH = 0;
static bool g_scFast = false;
static bool g_inSizeMove = false;
static WNDPROC g_origBtnProc = nullptr;

static int S(int v) { return MulDiv(v, g_dpi, 96); }

// ============================================================================
//  Small helpers
// ============================================================================
static void FillSolid(HDC dc, const RECT& r, COLORREF c)
{
    HBRUSH b = CreateSolidBrush(c);
    FillRect(dc, &r, b);
    DeleteObject(b);
}

static void RoundBox(HDC dc, const RECT& r, COLORREF fill, COLORREF border, int rad)
{
    HBRUSH b = CreateSolidBrush(fill);
    HPEN p = CreatePen(PS_SOLID, 1, border);
    HGDIOBJ ob = SelectObject(dc, b);
    HGDIOBJ op = SelectObject(dc, p);
    RoundRect(dc, r.left, r.top, r.right, r.bottom, rad, rad);
    SelectObject(dc, ob);
    SelectObject(dc, op);
    DeleteObject(b);
    DeleteObject(p);
}

static void DrawTxt(HDC dc, HFONT f, COLORREF c, const char* t, RECT r, UINT flags)
{
    SelectObject(dc, f);
    SetTextColor(dc, c);
    SetBkMode(dc, TRANSPARENT);
    DrawTextA(dc, t, -1, &r, flags | DT_NOPREFIX);
}

static bool ParseInt(const std::string& s, int& out)
{
    const char* b = s.c_str();
    char* e = nullptr;
    long v = strtol(b, &e, 10);
    if (e == b) return false;
    while (*e == ' ') ++e;
    if (*e) return false;
    out = (int)v;
    return true;
}

static bool ParseDouble(const std::string& s, double& out)
{
    const char* b = s.c_str();
    char* e = nullptr;
    double v = strtod(b, &e);
    if (e == b) return false;
    while (*e == ' ') ++e;
    if (*e) return false;
    out = v;
    return true;
}

static bool ParseRGB(const std::string& s, int& r, int& g, int& b)
{
    if (sscanf(s.c_str(), "%d%*[ ,]%d%*[ ,]%d", &r, &g, &b) != 3) return false;
    return r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255;
}

static std::string BaseName(const std::string& p)
{
    size_t k = p.find_last_of("\\/");
    return k == std::string::npos ? p : p.substr(k + 1);
}

static void MsgBox(HWND h, const char* text, UINT icon = MB_ICONINFORMATION)
{
    MessageBoxA(h, text, "Baby Photoshop Studio", MB_OK | icon);
}

// ============================================================================
//  Setup: DPI, fonts, dark title bar
// ============================================================================
typedef HRESULT(WINAPI* DwmSetAttrFn)(HWND, DWORD, LPCVOID, DWORD);
typedef BOOL(WINAPI* SetDpiAwareFn)(void);

static void ApplyDarkTitleBar(HWND h)
{
    HMODULE m = LoadLibraryA("dwmapi.dll");
    if (!m) return;
    DwmSetAttrFn fn = (DwmSetAttrFn)(void*)GetProcAddress(m, "DwmSetWindowAttribute");
    if (fn) {
        BOOL on = TRUE;
        fn(h, 20, &on, sizeof(on));          // DWMWA_USE_IMMERSIVE_DARK_MODE
        fn(h, 19, &on, sizeof(on));          // older Win10 builds
        COLORREF cap = COL_SIDE, tx = COL_TEXT;
        fn(h, 35, &cap, sizeof(cap));        // caption color (Win11)
        fn(h, 36, &tx, sizeof(tx));          // caption text color (Win11)
    }
    FreeLibrary(m);
}

static HFONT MakeFont(int px, int weight)
{
    return CreateFontA(-MulDiv(px, g_dpi, 96), 0, 0, 0, weight, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
}

static void InitResources()
{
    HMODULE u = GetModuleHandleA("user32.dll");
    if (u) {
        SetDpiAwareFn f = (SetDpiAwareFn)(void*)GetProcAddress(u, "SetProcessDPIAware");
        if (f) f();
    }
    HDC dc = GetDC(nullptr);
    g_dpi = GetDeviceCaps(dc, LOGPIXELSX);
    ReleaseDC(nullptr, dc);

    g_fontUI     = MakeFont(15, FW_NORMAL);
    g_fontBold   = MakeFont(15, FW_SEMIBOLD);
    g_fontTitle  = MakeFont(24, FW_BOLD);
    g_fontSmall  = MakeFont(12, FW_SEMIBOLD);
    g_fontStatus = MakeFont(13, FW_NORMAL);
    g_brSide = CreateSolidBrush(COL_SIDE);
    g_brEdit = CreateSolidBrush(COL_BTN);
}

// ============================================================================
//  Owner-drawn buttons (hover tracking via subclass)
// ============================================================================
static LRESULT CALLBACK BtnProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    switch (m) {
    case WM_MOUSEMOVE:
        if (!GetPropA(h, "bps_hot")) {
            SetPropA(h, "bps_hot", (HANDLE)1);
            TRACKMOUSEEVENT t;
            t.cbSize = sizeof(t); t.dwFlags = TME_LEAVE; t.hwndTrack = h; t.dwHoverTime = 0;
            TrackMouseEvent(&t);
            InvalidateRect(h, nullptr, FALSE);
        }
        break;
    case WM_MOUSELEAVE:
        RemovePropA(h, "bps_hot");
        InvalidateRect(h, nullptr, FALSE);
        break;
    case WM_SETCURSOR:
        if (IsWindowEnabled(h)) { SetCursor(LoadCursor(nullptr, IDC_HAND)); return TRUE; }
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_NCDESTROY:
        RemovePropA(h, "bps_hot");
        break;
    }
    return CallWindowProcA(g_origBtnProc, h, m, w, l);
}

static HWND MakeButton(HWND parent, int id, const char* text, int x, int y, int w, int h)
{
    HWND b = CreateWindowExA(0, "BUTTON", text,
                             WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                             x, y, w, h, parent, (HMENU)(INT_PTR)id, g_hInst, nullptr);
    SendMessageA(b, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
    WNDPROC o = (WNDPROC)SetWindowLongPtrA(b, GWLP_WNDPROC, (LONG_PTR)BtnProc);
    if (!g_origBtnProc) g_origBtnProc = o;
    return b;
}

static void DrawButton(const DRAWITEMSTRUCT* d)
{
    const int id = (int)d->CtlID;
    HDC dc = d->hDC;
    RECT r = d->rcItem;
    bool down = (d->itemState & ODS_SELECTED) != 0;
    bool off  = (d->itemState & ODS_DISABLED) != 0;
    bool hot  = GetPropA(d->hwndItem, "bps_hot") != nullptr;
    bool primary = (id == ID_OPEN || id == ID_SAVE || id == IDOK);
    bool active = (id >= ID_FILTER_BASE && id - ID_FILTER_BASE == g_activeFilter);

    COLORREF fill, border, text = COL_TEXT;
    if (off) {
        fill = border = COL_BTN_OFF; text = COL_DISABLED;
    } else if (primary) {
        fill = down ? RGB(31, 111, 235) : (hot ? RGB(88, 166, 255) : COL_ACCENT);
        border = fill; text = RGB(255, 255, 255);
    } else {
        fill = down ? COL_BTN_DN : (hot ? COL_BTN_HOT : COL_BTN);
        border = COL_BORDER;
        if (active) { fill = hot ? COL_BTN_HOT : RGB(30, 45, 72); border = COL_ACCENT; text = COL_ACCENT_L; }
    }

    FillSolid(dc, r, COL_SIDE);                 // clear corners
    RoundBox(dc, r, fill, border, S(10));

    char buf[64] = {0};
    GetWindowTextA(d->hwndItem, buf, sizeof(buf));
    if (down) OffsetRect(&r, 0, 1);
    DrawTxt(dc, g_fontBold, text, buf, r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

// ============================================================================
//  Modal prompt dialog (dark, built in code)
// ============================================================================
struct PromptState {
    std::vector<std::string> labels, values;
    std::vector<HWND> edits;
    bool ok = false, done = false;
};

static LRESULT CALLBACK PromptProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    PromptState* st = (PromptState*)GetWindowLongPtrA(h, GWLP_USERDATA);
    switch (m) {
    case WM_NCCREATE:
        SetWindowLongPtrA(h, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCTA*)l)->lpCreateParams);
        return DefWindowProcA(h, m, w, l);
    case WM_CREATE: {
        int y = S(16);
        for (size_t i = 0; i < st->labels.size(); ++i) {
            HWND lab = CreateWindowExA(0, "STATIC", st->labels[i].c_str(), WS_CHILD | WS_VISIBLE,
                                       S(20), y, S(340), S(20), h, nullptr, g_hInst, nullptr);
            SendMessageA(lab, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
            HWND ed = CreateWindowExA(0, "EDIT", st->values[i].c_str(),
                                      WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL,
                                      S(20), y + S(24), S(340), S(28), h,
                                      (HMENU)(INT_PTR)(100 + (int)i), g_hInst, nullptr);
            SendMessageA(ed, WM_SETFONT, (WPARAM)g_fontUI, TRUE);
            st->edits.push_back(ed);
            y += S(58);
        }
        y += S(8);
        MakeButton(h, IDOK, "Apply", S(20), y, S(165), S(38));
        MakeButton(h, IDCANCEL, "Cancel", S(195), y, S(165), S(38));
        return 0;
    }
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)w;
        SetTextColor(dc, COL_TEXT);
        SetBkColor(dc, COL_BTN);
        return (LRESULT)g_brEdit;
    }
    case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)w;
        SetTextColor(dc, COL_TEXT);
        SetBkColor(dc, COL_SIDE);
        return (LRESULT)g_brSide;
    }
    case WM_DRAWITEM:
        DrawButton((const DRAWITEMSTRUCT*)l);
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(w) == IDOK) {
            for (size_t i = 0; i < st->edits.size(); ++i) {
                char buf[256] = {0};
                GetWindowTextA(st->edits[i], buf, sizeof(buf));
                st->values[i] = buf;
            }
            st->ok = true; st->done = true;
        } else if (LOWORD(w) == IDCANCEL) {
            st->done = true;
        }
        return 0;
    case WM_CLOSE:
        if (st) st->done = true;
        return 0;
    }
    return DefWindowProcA(h, m, w, l);
}

static bool Prompt(HWND owner, const char* title,
                   const std::vector<std::string>& labels, std::vector<std::string>& values)
{
    static bool registered = false;
    if (!registered) {
        WNDCLASSEXA wc = {};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = PromptProc;
        wc.hInstance = g_hInst;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = g_brSide;
        wc.lpszClassName = "BPSPrompt";
        RegisterClassExA(&wc);
        registered = true;
    }

    PromptState st;
    st.labels = labels;
    st.values = values;

    const DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU;
    const DWORD exstyle = WS_EX_DLGMODALFRAME | WS_EX_CONTROLPARENT;
    RECT r = {0, 0, S(380), S(16) + (int)labels.size() * S(58) + S(8) + S(38) + S(20)};
    AdjustWindowRectEx(&r, style, FALSE, exstyle);
    int dw = r.right - r.left, dh = r.bottom - r.top;
    RECT orc;
    GetWindowRect(owner, &orc);
    int x = orc.left + ((orc.right - orc.left) - dw) / 2;
    int y = orc.top + ((orc.bottom - orc.top) - dh) / 2;

    HWND dlg = CreateWindowExA(exstyle, "BPSPrompt", title, style, x, y, dw, dh,
                               owner, nullptr, g_hInst, &st);
    if (!dlg) return false;
    ApplyDarkTitleBar(dlg);

    EnableWindow(owner, FALSE);
    ShowWindow(dlg, SW_SHOW);
    UpdateWindow(dlg);
    if (!st.edits.empty()) {
        SetFocus(st.edits[0]);
        SendMessageA(st.edits[0], EM_SETSEL, 0, -1);
    }

    MSG msg;
    while (!st.done && GetMessageA(&msg, nullptr, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN && (msg.hwnd == dlg || IsChild(dlg, msg.hwnd))) {
            if (msg.wParam == VK_RETURN) { SendMessageA(dlg, WM_COMMAND, IDOK, 0); continue; }
            if (msg.wParam == VK_ESCAPE) { SendMessageA(dlg, WM_COMMAND, IDCANCEL, 0); continue; }
        }
        if (IsDialogMessageA(dlg, &msg)) continue;
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    if (!st.done) PostQuitMessage(0);   // WM_QUIT arrived while the dialog was open

    EnableWindow(owner, TRUE);
    DestroyWindow(dlg);
    SetForegroundWindow(owner);

    if (st.ok) values = st.values;
    return st.ok;
}

// ============================================================================
//  Image <-> screen
// ============================================================================
static void InvalidateScaled()
{
    if (g_scaled) { DeleteObject(g_scaled); g_scaled = nullptr; }
}

static void RebuildDIB()
{
    if (g_dib) { DeleteObject(g_dib); g_dib = nullptr; }
    InvalidateScaled();
    if (!g_hasImage) return;

    const int w = g_current.width, h = g_current.height;
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;               // top-down
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HDC dc = GetDC(nullptr);
    g_dib = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    ReleaseDC(nullptr, dc);
    if (!g_dib || !bits) return;

    unsigned char* p = (unsigned char*)bits;
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            p[0] = g_current(x, y, 2);   // B
            p[1] = g_current(x, y, 1);   // G
            p[2] = g_current(x, y, 0);   // R
            p[3] = 0;
            p += 4;
        }
}

static void DrawImageFit(HDC mem, const RECT& area)
{
    if (!g_hasImage || !g_dib) return;
    const int aw = area.right - area.left, ah = area.bottom - area.top;
    if (aw < 4 || ah < 4) return;

    double sc = std::min((double)aw / g_current.width, (double)ah / g_current.height);
    sc = std::min(sc, 3.0);
    const int dw = std::max(1, (int)(g_current.width * sc));
    const int dh = std::max(1, (int)(g_current.height * sc));
    const bool fast = g_inSizeMove;

    if (!g_scaled || g_scW != dw || g_scH != dh || g_scFast != fast) {
        InvalidateScaled();
        HDC src = CreateCompatibleDC(mem);
        HDC dst = CreateCompatibleDC(mem);
        HGDIOBJ o1 = SelectObject(src, g_dib);
        g_scaled = CreateCompatibleBitmap(mem, dw, dh);
        HGDIOBJ o2 = SelectObject(dst, g_scaled);
        SetStretchBltMode(dst, fast ? COLORONCOLOR : HALFTONE);
        SetBrushOrgEx(dst, 0, 0, nullptr);
        StretchBlt(dst, 0, 0, dw, dh, src, 0, 0, g_current.width, g_current.height, SRCCOPY);
        SelectObject(dst, o2);
        SelectObject(src, o1);
        DeleteDC(dst);
        DeleteDC(src);
        g_scW = dw; g_scH = dh; g_scFast = fast;
    }

    const int x = area.left + (aw - dw) / 2;
    const int y = area.top + (ah - dh) / 2;
    HDC t = CreateCompatibleDC(mem);
    HGDIOBJ o = SelectObject(t, g_scaled);
    BitBlt(mem, x, y, dw, dh, t, 0, 0, SRCCOPY);
    SelectObject(t, o);
    DeleteDC(t);
}

static RECT CanvasRect(int W, int H)
{
    RECT r = {S(SIDE_W) + S(20), S(20), W - S(20), H - S(58)};
    return r;
}

static void PaintMain(HWND h)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(h, &ps);
    RECT rc;
    GetClientRect(h, &rc);
    const int W = rc.right, H = rc.bottom;
    if (W <= 0 || H <= 0) { EndPaint(h, &ps); return; }

    HDC mem = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, W, H);
    HGDIOBJ oldBmp = SelectObject(mem, bmp);

    FillSolid(mem, rc, COL_BG);

    // Sidebar
    RECT side = {0, 0, S(SIDE_W), H};
    FillSolid(mem, side, COL_SIDE);
    RECT edge = {S(SIDE_W) - 1, 0, S(SIDE_W), H};
    FillSolid(mem, edge, COL_BORDER);

    // Gradient accent strip (blue -> purple)
    const int gw = S(SIDE_W) - 1;
    for (int x = 0; x < gw; ++x) {
        double t = (double)x / (double)(gw > 1 ? gw - 1 : 1);
        COLORREF c = RGB((int)(56 + (163 - 56) * t), (int)(139 + (113 - 139) * t), (int)(253 + (247 - 253) * t));
        RECT gr = {x, 0, x + 1, S(3)};
        FillSolid(mem, gr, c);
    }

    // Logo badge + title
    RECT badge = {S(20), S(20), S(60), S(60)};
    RoundBox(mem, badge, COL_ACCENT, COL_ACCENT, S(12));
    DrawTxt(mem, g_fontTitle, RGB(255, 255, 255), "B", badge, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    RECT t1 = {S(72), S(16), S(SIDE_W) - S(12), S(46)};
    DrawTxt(mem, g_fontTitle, COL_TEXT, "Baby Photoshop", t1, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    RECT t2 = {S(72), S(44), S(SIDE_W) - S(12), S(64)};
    DrawTxt(mem, g_fontSmall, COL_ACCENT_L, "STUDIO  -  18 FILTERS", t2, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    RECT t3 = {S(20), S(178), S(SIDE_W) - S(16), S(202)};
    DrawTxt(mem, g_fontSmall, COL_MUTED, "FILTERS", t3, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    // Canvas
    RECT cv = CanvasRect(W, H);
    if (cv.right > cv.left + 20 && cv.bottom > cv.top + 20) {
        RoundBox(mem, cv, COL_PANEL, COL_BORDER, S(14));
        RECT inner = cv;
        InflateRect(&inner, -S(12), -S(12));

        if (g_hasImage) {
            DrawImageFit(mem, inner);
        } else {
            RECT a = inner; a.bottom = (inner.top + inner.bottom) / 2;
            RECT b = inner; b.top = a.bottom + S(4);
            DrawTxt(mem, g_fontTitle, COL_MUTED, "No image loaded", a, DT_CENTER | DT_BOTTOM | DT_SINGLELINE);
            DrawTxt(mem, g_fontUI, COL_DISABLED, "Click  Open Image  or drop a file here", b,
                    DT_CENTER | DT_TOP | DT_SINGLELINE);
        }

        // Status line
        RECT sl = {cv.left + S(4), cv.bottom + S(10), cv.right - S(4), H - S(8)};
        std::string leftText = g_hasImage
            ? BaseName(g_filePath) + "   " + std::to_string(g_current.width) + " x " +
              std::to_string(g_current.height) + " px"
            : std::string("");
        DrawTxt(mem, g_fontStatus, COL_MUTED, leftText.c_str(), sl, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
        RECT sr = sl; sr.left = sl.left + (sl.right - sl.left) / 2;
        DrawTxt(mem, g_fontStatus, COL_ACCENT_L, g_status.c_str(), sr, DT_RIGHT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
    }

    BitBlt(hdc, 0, 0, W, H, mem, 0, 0, SRCCOPY);
    SelectObject(mem, oldBmp);
    DeleteObject(bmp);
    DeleteDC(mem);
    EndPaint(h, &ps);
}

// ============================================================================
//  File dialogs, load, save
// ============================================================================
static bool PickFile(HWND owner, bool save, std::string& out)
{
    char buf[MAX_PATH * 2] = {0};
    if (save) strcpy(buf, "edited.png");

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFile = buf;
    ofn.nMaxFile = sizeof(buf);
    ofn.nFilterIndex = 1;
    if (save) {
        ofn.lpstrFilter = "PNG (*.png)\0*.png\0JPEG (*.jpg)\0*.jpg\0Bitmap (*.bmp)\0*.bmp\0";
        ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    } else {
        ofn.lpstrFilter = "Images\0*.png;*.jpg;*.jpeg;*.bmp;*.tga\0All files\0*.*\0";
        ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    }
    BOOL ok = save ? GetSaveFileNameA(&ofn) : GetOpenFileNameA(&ofn);
    if (!ok) return false;

    std::string p = buf;
    if (save) {
        size_t slash = p.find_last_of("\\/");
        size_t dot = p.find_last_of('.');
        if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
            p += (ofn.nFilterIndex == 2 ? ".jpg" : (ofn.nFilterIndex == 3 ? ".bmp" : ".png"));
    }
    out = p;
    return true;
}

// Loads and normalizes to exactly 3 channels (RGB) so every filter can rely on it.
static bool LoadImageFile(const std::string& path, Image& out)
{
    try {
        Image tmp;
        tmp.loadNewImage(path);
        if (tmp.width <= 0 || tmp.height <= 0) return false;
        Image n(tmp.width, tmp.height);
        const int ch = tmp.channels;
        for (int y = 0; y < tmp.height; ++y)
            for (int x = 0; x < tmp.width; ++x)
                for (int k = 0; k < 3; ++k)
                    n(x, y, k) = tmp(x, y, ch >= 3 ? k : 0);
        out = n;
        return true;
    } catch (...) {
        return false;
    }
}

static void UpdateControls()
{
    EnableWindow(GetDlgItem(g_main, ID_SAVE), g_hasImage);
    EnableWindow(GetDlgItem(g_main, ID_UNDO), g_hasImage && !g_undo.empty());
    EnableWindow(GetDlgItem(g_main, ID_RESET), g_hasImage && !g_undo.empty());
    for (int i = 0; i < kFilterCount; ++i)
        EnableWindow(GetDlgItem(g_main, ID_FILTER_BASE + i), g_hasImage);
    RedrawWindow(g_main, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

static void OpenPath(HWND h, const std::string& path)
{
    Image img;
    if (!LoadImageFile(path, img)) {
        MsgBox(h, "Could not load this image.", MB_ICONERROR);
        return;
    }
    g_original = img;
    g_current = img;
    g_undo.clear();
    g_hasImage = true;
    g_activeFilter = -1;
    g_filePath = path;
    g_status = "Image loaded.";
    RebuildDIB();
    UpdateControls();
}

static void DoSave(HWND h)
{
    if (!g_hasImage) return;
    std::string p;
    if (!PickFile(h, true, p)) return;
    bool ok = false;
    try {
        g_current.saveImage(p);
        ok = GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES;
    } catch (...) {
        ok = false;
    }
    if (ok) g_status = "Saved: " + BaseName(p);
    else    MsgBox(h, "Could not save the image. Use .png, .jpg or .bmp.", MB_ICONERROR);
    InvalidateRect(h, nullptr, FALSE);
}

static void PushUndo()
{
    g_undo.push_back(g_current);
    if (g_undo.size() > 15) g_undo.erase(g_undo.begin());
}

static void DoUndo(HWND h)
{
    if (g_undo.empty()) return;
    g_current = g_undo.back();
    g_undo.pop_back();
    g_activeFilter = -1;
    g_status = "Undo";
    RebuildDIB();
    UpdateControls();
    InvalidateRect(h, nullptr, FALSE);
}

static void DoReset(HWND h)
{
    if (!g_hasImage) return;
    PushUndo();
    g_current = g_original;
    g_activeFilter = -1;
    g_status = "Reset to original";
    RebuildDIB();
    UpdateControls();
    InvalidateRect(h, nullptr, FALSE);
}

// ============================================================================
//  Applying filters (calls the real functions from filters.cpp)
// ============================================================================
static void ApplyFilter(HWND h, int idx)
{
    if (!g_hasImage || idx < 0 || idx >= kFilterCount) return;
    Image result;
    const int W = g_current.width, H = g_current.height;

    try {
        // ---- 1) Collect parameters first (no heavy work yet) -------------
        int  iv[4] = {0, 0, 0, 0};
        int  rr = 0, gg = 0, bb = 0;
        bool horizontal = true;
        double angle = 0.0;
        Image second;

        switch (idx) {
        case 3: {   // Add Frame
            std::vector<std::string> lb;
            lb.push_back("Frame thickness (px)");
            lb.push_back("Frame color (R,G,B)");
            std::vector<std::string> v;
            v.push_back("20");
            v.push_back("255,255,255");
            if (!Prompt(h, "Add Frame", lb, v)) return;
            if (!ParseInt(v[0], iv[0]) || iv[0] < 1 || iv[0] > 2000 || !ParseRGB(v[1], rr, gg, bb)) {
                MsgBox(h, "Invalid values.", MB_ICONWARNING); return;
            }
            break;
        }
        case 4: {   // Flip
            std::vector<std::string> lb(1, "Direction  (H = horizontal, V = vertical)");
            std::vector<std::string> v(1, "H");
            if (!Prompt(h, "Flip Image", lb, v)) return;
            char c = v[0].empty() ? 0 : v[0][0];
            if (c == 'H' || c == 'h') horizontal = true;
            else if (c == 'V' || c == 'v') horizontal = false;
            else { MsgBox(h, "Type H or V.", MB_ICONWARNING); return; }
            break;
        }
        case 5: {   // Rotate
            std::vector<std::string> lb(1, "Angle clockwise  (90, 180 or 270)");
            std::vector<std::string> v(1, "90");
            if (!Prompt(h, "Rotate Image", lb, v)) return;
            if (!ParseInt(v[0], iv[0]) || (iv[0] != 90 && iv[0] != 180 && iv[0] != 270)) {
                MsgBox(h, "Angle must be 90, 180 or 270.", MB_ICONWARNING); return;
            }
            break;
        }
        case 6: {   // Brightness
            std::vector<std::string> lb(1, "Brightness change %  (-100 to 100)");
            std::vector<std::string> v(1, "30");
            if (!Prompt(h, "Brightness", lb, v)) return;
            if (!ParseInt(v[0], iv[0]) || iv[0] < -100 || iv[0] > 100) {
                MsgBox(h, "Enter a value between -100 and 100.", MB_ICONWARNING); return;
            }
            break;
        }
        case 7: {   // Resize
            std::vector<std::string> lb;
            lb.push_back("New width (px)");
            lb.push_back("New height (px)");
            std::vector<std::string> v;
            v.push_back(std::to_string(W));
            v.push_back(std::to_string(H));
            if (!Prompt(h, "Resize Image", lb, v)) return;
            if (!ParseInt(v[0], iv[0]) || !ParseInt(v[1], iv[1]) || iv[0] < 1 || iv[1] < 1 ||
                iv[0] > 10000 || iv[1] > 10000 || (long long)iv[0] * iv[1] > 60000000LL) {
                MsgBox(h, "Invalid size.", MB_ICONWARNING); return;
            }
            break;
        }
        case 8: {   // Merge
            std::string p2;
            if (!PickFile(h, false, p2)) return;
            if (!LoadImageFile(p2, second)) { MsgBox(h, "Could not load the second image.", MB_ICONERROR); return; }
            break;
        }
        case 10: {  // Crop
            std::vector<std::string> lb;
            lb.push_back("X (left)");
            lb.push_back("Y (top)");
            lb.push_back("Width");
            lb.push_back("Height");
            std::vector<std::string> v;
            v.push_back("0");
            v.push_back("0");
            v.push_back(std::to_string(W / 2));
            v.push_back(std::to_string(H / 2));
            if (!Prompt(h, "Crop Image", lb, v)) return;
            if (!ParseInt(v[0], iv[0]) || !ParseInt(v[1], iv[1]) || !ParseInt(v[2], iv[2]) || !ParseInt(v[3], iv[3]) ||
                iv[0] < 0 || iv[1] < 0 || iv[2] < 1 || iv[3] < 1 || iv[0] + iv[2] > W || iv[1] + iv[3] > H) {
                MsgBox(h, "The crop area must be inside the image.", MB_ICONWARNING); return;
            }
            break;
        }
        case 16: {  // Skew
            std::vector<std::string> lb(1, "Angle in degrees  (-60 to 60)");
            std::vector<std::string> v(1, "20");
            if (!Prompt(h, "Skew Image", lb, v)) return;
            if (!ParseDouble(v[0], angle) || angle < -60.0 || angle > 60.0) {
                MsgBox(h, "Enter an angle between -60 and 60.", MB_ICONWARNING); return;
            }
            break;
        }
        default: break;
        }

        // ---- 2) Heavy work ----------------------------------------------
        g_status = std::string("Processing ") + kFilterNames[idx] + "...";
        InvalidateRect(h, nullptr, FALSE);
        UpdateWindow(h);
        HCURSOR oldCur = SetCursor(LoadCursor(nullptr, IDC_WAIT));

        switch (idx) {
        case 0:  result = grayscale(g_current); break;
        case 1:  result = blackAndWhite(g_current); break;
        case 2:  result = invertColors(g_current); break;
        case 3:  result = addFrame(g_current, iv[0], rr, gg, bb); break;
        case 4:  result = flipImage(g_current, horizontal); break;
        case 5:  result = rotateImage(g_current, iv[0]); break;
        case 6:  result = adjustBrightness(g_current, iv[0]); break;
        case 7:  result = resizeImage(g_current, iv[0], iv[1]); break;
        case 8:  result = mergeImages(g_current, second); break;
        case 9:  result = detectEdges(g_current); break;
        case 10: result = cropImage(g_current, iv[0], iv[1], iv[2], iv[3]); break;
        case 11: result = strongBlur(g_current); break;
        case 12: result = wanoSunlight(g_current); break;
        case 13: result = oldTVEffect(g_current); break;
        case 14: result = purpleTint(g_current); break;
        case 15: result = infraredEffect(g_current); break;
        case 16: result = skewImage(g_current, angle); break;
        case 17: result = oilPainting(g_current); break;
        }
        SetCursor(oldCur);
    } catch (...) {
        g_status = "Filter failed.";
        MsgBox(h, "The filter failed (maybe not enough memory).", MB_ICONERROR);
        InvalidateRect(h, nullptr, FALSE);
        return;
    }

    if (result.width <= 0 || result.height <= 0) {
        g_status = "Filter returned an empty image.";
        InvalidateRect(h, nullptr, FALSE);
        return;
    }

    PushUndo();
    g_current = result;
    g_activeFilter = idx;
    g_status = std::string("Applied: ") + kFilterNames[idx];
    RebuildDIB();
    UpdateControls();
    InvalidateRect(h, nullptr, FALSE);
}

// ============================================================================
//  Main window
// ============================================================================
static void CreateControls(HWND h)
{
    const int pad = S(16), gap = S(8);
    const int bw = (S(SIDE_W) - 2 * pad - gap) / 2;

    int ya = S(92), yb = S(92) + S(40) + gap;
    MakeButton(h, ID_OPEN,  "Open Image", pad,            ya, bw, S(40));
    MakeButton(h, ID_SAVE,  "Save Image", pad + bw + gap, ya, bw, S(40));
    MakeButton(h, ID_UNDO,  "Undo",       pad,            yb, bw, S(40));
    MakeButton(h, ID_RESET, "Reset",      pad + bw + gap, yb, bw, S(40));

    const int y0 = S(206);
    for (int i = 0; i < kFilterCount; ++i) {
        int col = i % 2, row = i / 2;
        MakeButton(h, ID_FILTER_BASE + i, kFilterNames[i],
                   pad + col * (bw + gap), y0 + row * (S(36) + gap), bw, S(36));
    }
}

static void OnCommand(HWND h, int id)
{
    if (id == ID_OPEN) {
        std::string p;
        if (PickFile(h, false, p)) OpenPath(h, p);
    } else if (id == ID_SAVE) {
        DoSave(h);
    } else if (id == ID_UNDO) {
        DoUndo(h);
    } else if (id == ID_RESET) {
        DoReset(h);
    } else if (id >= ID_FILTER_BASE && id < ID_FILTER_BASE + kFilterCount) {
        ApplyFilter(h, id - ID_FILTER_BASE);
    }
}

LRESULT CALLBACK MainWndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    switch (m) {
    case WM_CREATE:
        g_main = h;
        ApplyDarkTitleBar(h);
        CreateControls(h);
        UpdateControls();
        DragAcceptFiles(h, TRUE);
        return 0;

    case WM_COMMAND:
        if (HIWORD(w) == BN_CLICKED) OnCommand(h, LOWORD(w));
        return 0;

    case WM_DRAWITEM:
        DrawButton((const DRAWITEMSTRUCT*)l);
        return TRUE;

    case WM_DROPFILES: {
        char path[MAX_PATH] = {0};
        if (DragQueryFileA((HDROP)w, 0, path, MAX_PATH)) OpenPath(h, path);
        DragFinish((HDROP)w);
        return 0;
    }

    case WM_GETMINMAXINFO: {
        RECT r = {0, 0, S(1000), S(650)};
        AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
        MINMAXINFO* mm = (MINMAXINFO*)l;
        mm->ptMinTrackSize.x = r.right - r.left;
        mm->ptMinTrackSize.y = r.bottom - r.top;
        return 0;
    }

    case WM_ENTERSIZEMOVE:
        g_inSizeMove = true;
        return 0;
    case WM_EXITSIZEMOVE:
        g_inSizeMove = false;
        InvalidateRect(h, nullptr, FALSE);
        return 0;

    case WM_SIZE:
        if (w != SIZE_MINIMIZED) InvalidateRect(h, nullptr, FALSE);
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
        PaintMain(h);
        return 0;

    case WM_DESTROY:
        if (g_dib) DeleteObject(g_dib);
        if (g_scaled) DeleteObject(g_scaled);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(h, m, w, l);
}

bool RegisterMainWindowClass(HINSTANCE hInst)
{
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = "BabyPhotoshopStudioWnd";
    return RegisterClassExA(&wc) != 0;
}

HWND CreateMainWindow(HINSTANCE hInst, int nCmdShow)
{
    const DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    RECT work;
    SystemParametersInfoA(SPI_GETWORKAREA, 0, &work, 0);

    RECT r = {0, 0, S(1180), S(700)};
    AdjustWindowRect(&r, style, FALSE);
    int ww = std::min<int>(r.right - r.left, work.right - work.left);
    int wh = std::min<int>(r.bottom - r.top, work.bottom - work.top);
    int x = work.left + ((work.right - work.left) - ww) / 2;
    int y = work.top + ((work.bottom - work.top) - wh) / 2;

    HWND h = CreateWindowExA(0, "BabyPhotoshopStudioWnd", "Baby Photoshop Studio", style,
                             x, y, ww, wh, nullptr, nullptr, hInst, nullptr);
    if (h) {
        ShowWindow(h, nCmdShow);
        UpdateWindow(h);
    }
    return h;
}

int RunMessageLoop()
{
    MSG msg;
    while (GetMessageA(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}

int RunApp(HINSTANCE hInst, int nCmdShow)
{
    g_hInst = hInst;
    InitResources();
    if (!RegisterMainWindowClass(hInst)) return 1;
    if (!CreateMainWindow(hInst, nCmdShow)) return 2;
    return RunMessageLoop();
}

#endif // BPS_UNITY