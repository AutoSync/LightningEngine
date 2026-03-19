// NativeDialog.h — Native OS file/folder picker.
//
// Usage:
//   std::string path = NativeDialog::PickFolderSDL(sdlWindow, "Select Project Folder");
//   if (!path.empty()) { /* user picked path */ }
//
// Platform:
//   Windows  — IFileOpenDialog (Vista+, COM)
//   Linux    — zenity --file-selection --directory
//   Other    — returns empty string (fallback)
#pragma once

// Windows headers must come BEFORE C++ STL headers to avoid
// std::byte vs rpcndr.h byte conflict (MSVC C2872).
#ifdef _WIN32
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   include <windows.h>
#   include <shobjidl.h>
#   pragma comment(lib, "ole32.lib")
#   pragma comment(lib, "uuid.lib")
#elif defined(__linux__)
#   include <cstdio>
#   include <cstring>
#endif

#include <atomic>
#include <string>
#include <SDL3/SDL.h>

namespace NativeDialog {

#ifdef _WIN32
inline void PrepareDialogParentWindow(SDL_Window* win, HWND hwnd)
{
    if (win) {
        SDL_RaiseWindow(win);
    }

    if (!hwnd) {
        return;
    }

    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }

    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);

    if (!SetForegroundWindow(hwnd)) {
        DWORD fgThread = GetWindowThreadProcessId(GetForegroundWindow(), nullptr);
        DWORD curThread = GetCurrentThreadId();
        if (fgThread != 0 && fgThread != curThread) {
            if (AttachThreadInput(curThread, fgThread, TRUE)) {
                BringWindowToTop(hwnd);
                SetForegroundWindow(hwnd);
                AttachThreadInput(curThread, fgThread, FALSE);
            }
        } else {
            BringWindowToTop(hwnd);
            SetForegroundWindow(hwnd);
        }
    }

    SetActiveWindow(hwnd);
    SetFocus(hwnd);
}
#endif

// Opens a native OS folder-picker dialog.
// win  — SDL_Window* used to get the parent HWND (Win32) or ignored (Linux).
// title — dialog title string.
// Returns the absolute path the user selected, or "" on cancel/error.
inline std::string PickFolder(SDL_Window* win, const char* title = "Select Folder")
{
#ifdef _WIN32
    // ── Windows: IFileOpenDialog (Vista+) ────────────────────────────────
    HWND hwnd = NULL;
    if (win) {
        hwnd = (HWND)SDL_GetPointerProperty(
            SDL_GetWindowProperties(win),
            SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    }

    PrepareDialogParentWindow(win, hwnd);

    std::string result;

    if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
        return result;

    IFileOpenDialog* pfd = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL,
                                  CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr)) {
        FILEOPENDIALOGOPTIONS opts;
        pfd->GetOptions(&opts);
        pfd->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);

        if (title) {
            int wlen = MultiByteToWideChar(CP_UTF8, 0, title, -1, nullptr, 0);
            std::wstring wtitle(wlen, 0);
            MultiByteToWideChar(CP_UTF8, 0, title, -1, &wtitle[0], wlen);
            pfd->SetTitle(wtitle.c_str());
        }

        hr = pfd->Show(hwnd);
        if (SUCCEEDED(hr)) {
            IShellItem* psi = nullptr;
            hr = pfd->GetResult(&psi);
            if (SUCCEEDED(hr)) {
                PWSTR pszPath = nullptr;
                hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                if (SUCCEEDED(hr) && pszPath) {
                    int len = WideCharToMultiByte(CP_UTF8, 0, pszPath, -1,
                                                  nullptr, 0, nullptr, nullptr);
                    result.resize(len - 1);
                    WideCharToMultiByte(CP_UTF8, 0, pszPath, -1,
                                        &result[0], len, nullptr, nullptr);
                    CoTaskMemFree(pszPath);
                }
                psi->Release();
            }
        }
        pfd->Release();
    }

    CoUninitialize();
    return result;

#elif defined(__linux__)
    // ── Linux: zenity ────────────────────────────────────────────────────
    std::string cmd = "zenity --file-selection --directory";
    if (title && title[0]) {
        cmd += " --title=\"";
        cmd += title;
        cmd += "\"";
    }
    cmd += " 2>/dev/null";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    char buf[4096] = {};
    if (fgets(buf, sizeof(buf), pipe)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
    }
    pclose(pipe);
    return std::string(buf);

#else
    // ── Unsupported platform ──────────────────────────────────────────────
    (void)win; (void)title;
    return "";
#endif
}

// Opens a native OS file-open dialog (for files, not folders).
// filter — e.g. "Scene Files\0*.lescene\0All Files\0*.*\0\0" (Win32 style, ignored on Linux).
// Returns selected file path or "".
inline std::string PickFile(SDL_Window* win, const char* title = "Open File",
                            const char* /*filter*/ = nullptr)
{
#ifdef _WIN32
    HWND hwnd = NULL;
    if (win) {
        hwnd = (HWND)SDL_GetPointerProperty(
            SDL_GetWindowProperties(win),
            SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    }

    PrepareDialogParentWindow(win, hwnd);

    std::string result;

    if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
        return result;

    IFileOpenDialog* pfd = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL,
                                  CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr)) {
        FILEOPENDIALOGOPTIONS opts;
        pfd->GetOptions(&opts);
        pfd->SetOptions(opts | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST);

        if (title) {
            int wlen = MultiByteToWideChar(CP_UTF8, 0, title, -1, nullptr, 0);
            std::wstring wtitle(wlen, 0);
            MultiByteToWideChar(CP_UTF8, 0, title, -1, &wtitle[0], wlen);
            pfd->SetTitle(wtitle.c_str());
        }

        hr = pfd->Show(hwnd);
        if (SUCCEEDED(hr)) {
            IShellItem* psi = nullptr;
            hr = pfd->GetResult(&psi);
            if (SUCCEEDED(hr)) {
                PWSTR pszPath = nullptr;
                hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                if (SUCCEEDED(hr) && pszPath) {
                    int len = WideCharToMultiByte(CP_UTF8, 0, pszPath, -1,
                                                  nullptr, 0, nullptr, nullptr);
                    result.resize(len - 1);
                    WideCharToMultiByte(CP_UTF8, 0, pszPath, -1,
                                        &result[0], len, nullptr, nullptr);
                    CoTaskMemFree(pszPath);
                }
                psi->Release();
            }
        }
        pfd->Release();
    }

    CoUninitialize();
    return result;

#elif defined(__linux__)
    std::string cmd = "zenity --file-selection";
    if (title && title[0]) {
        cmd += " --title=\"";
        cmd += title;
        cmd += "\"";
    }
    cmd += " 2>/dev/null";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    char buf[4096] = {};
    if (fgets(buf, sizeof(buf), pipe)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
    }
    pclose(pipe);
    return std::string(buf);
#else
    (void)win; (void)title;
    return "";
#endif
}

// ── SDL3 blocking helpers ─────────────────────────────────────────────────────
// Cross-platform: usa SDL_ShowFileDialogWithProperties (SDL 3.2+).
// Envolve a API assíncrona do SDL3 em chamadas síncronas fazendo pump de eventos.
// Deve ser chamado a partir da thread principal SDL.
//
//   NativeDialog::PickFolderSDL(win, "Select Folder");
//   NativeDialog::PickFileSDL  (win, "Open File", nullptr, filters, nfilters);

namespace {
    struct _NativeSDLDialogResult {
        std::string       path;
        std::atomic<bool> done { false };
    };
    inline void _nativeSDLDialogCb(void* ud, const char* const* fl, int /*filter*/)
    {
        auto* r = static_cast<_NativeSDLDialogResult*>(ud);
        if (fl && fl[0]) r->path = fl[0];
        r->done.store(true, std::memory_order_release);
    }
} // anonymous namespace

// Seletor de pasta (folder) nativo via SDL3.
// Bloqueia a thread chamante até o usuário confirmar ou cancelar.
inline std::string PickFolderSDL(SDL_Window* win,
                                 const char* title       = "Select Folder",
                                 const char* defaultPath = nullptr)
{
    if (win) SDL_RaiseWindow(win);

    _NativeSDLDialogResult res;

    SDL_PropertiesID props = SDL_CreateProperties();
    if (props) {
        SDL_SetPointerProperty(props, SDL_PROP_FILE_DIALOG_WINDOW_POINTER, win);
        SDL_SetStringProperty (props, SDL_PROP_FILE_DIALOG_TITLE_STRING,   title ? title : "Select Folder");
        if (defaultPath)
            SDL_SetStringProperty(props, SDL_PROP_FILE_DIALOG_LOCATION_STRING, defaultPath);
        SDL_ShowFileDialogWithProperties(SDL_FILEDIALOG_OPENFOLDER, _nativeSDLDialogCb, &res, props);
        SDL_DestroyProperties(props);
    } else {
        SDL_ShowOpenFolderDialog(_nativeSDLDialogCb, &res, win, defaultPath, false);
    }

    while (!res.done.load(std::memory_order_acquire)) {
        SDL_PumpEvents();
        SDL_Delay(10);
    }
    return res.path;
}

// Seletor de arquivo nativo via SDL3.
// filters/nfilters opcionais (nullptr/0 = sem filtro).
// Bloqueia a thread chamante até o usuário confirmar ou cancelar.
inline std::string PickFileSDL(SDL_Window*                 win,
                                const char*                 title       = "Open File",
                                const char*                 defaultPath = nullptr,
                                const SDL_DialogFileFilter* filters     = nullptr,
                                int                         nfilters    = 0)
{
    if (win) SDL_RaiseWindow(win);

    _NativeSDLDialogResult res;

    SDL_PropertiesID props = SDL_CreateProperties();
    if (props) {
        SDL_SetPointerProperty(props, SDL_PROP_FILE_DIALOG_WINDOW_POINTER, win);
        SDL_SetStringProperty (props, SDL_PROP_FILE_DIALOG_TITLE_STRING,   title ? title : "Open File");
        if (defaultPath)
            SDL_SetStringProperty(props, SDL_PROP_FILE_DIALOG_LOCATION_STRING, defaultPath);
        if (filters && nfilters > 0) {
            SDL_SetPointerProperty(props, SDL_PROP_FILE_DIALOG_FILTERS_POINTER,
                                   const_cast<SDL_DialogFileFilter*>(filters));
            SDL_SetNumberProperty (props, SDL_PROP_FILE_DIALOG_NFILTERS_NUMBER, nfilters);
        }
        SDL_ShowFileDialogWithProperties(SDL_FILEDIALOG_OPENFILE, _nativeSDLDialogCb, &res, props);
        SDL_DestroyProperties(props);
    } else {
        SDL_ShowOpenFileDialog(_nativeSDLDialogCb, &res, win, filters, nfilters, defaultPath, false);
    }

    while (!res.done.load(std::memory_order_acquire)) {
        SDL_PumpEvents();
        SDL_Delay(10);
    }
    return res.path;
}

} // namespace NativeDialog
