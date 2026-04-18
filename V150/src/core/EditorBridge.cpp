#include "../include/EditorBridge.h"
#include <mutex>
#include <sstream>
#include <iomanip>
#include <fstream>

namespace LightningEngine::EditorBridge {

namespace {
    std::mutex gMutex;
    Status gStatus;

    std::string jsonEscape(const std::string& input)
    {
        std::ostringstream out;
        for (char ch : input) {
            switch (ch) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (static_cast<unsigned char>(ch) < 0x20) {
                    out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)(unsigned char)ch;
                } else {
                    out << ch;
                }
                break;
            }
        }
        return out.str();
    }
}

void SetProject(const std::string& projectName)
{
    std::lock_guard<std::mutex> lock(gMutex);
    gStatus.project = projectName;
}

void SetScene(const std::string& scenePath)
{
    std::lock_guard<std::mutex> lock(gMutex);
    gStatus.scene = scenePath;
}

void SetRunning(bool running)
{
    std::lock_guard<std::mutex> lock(gMutex);
    gStatus.running = running;
}

void SetFps(float fps)
{
    std::lock_guard<std::mutex> lock(gMutex);
    gStatus.fps = fps;
}

void SetLastChange(const std::string& message)
{
    std::lock_guard<std::mutex> lock(gMutex);
    gStatus.lastChange = message;
}

Status GetStatus()
{
    std::lock_guard<std::mutex> lock(gMutex);
    return gStatus;
}

std::string GetStatusJson()
{
    std::lock_guard<std::mutex> lock(gMutex);
    std::ostringstream out;
    out << '{'
        << "\"running\":" << (gStatus.running ? "true" : "false") << ','
        << "\"fps\":" << std::fixed << std::setprecision(1) << gStatus.fps << ','
        << "\"project\":\"" << jsonEscape(gStatus.project) << "\"," 
        << "\"scene\":\"" << jsonEscape(gStatus.scene) << "\"," 
        << "\"lastChange\":\"" << jsonEscape(gStatus.lastChange) << "\""
        << '}';
    return out.str();
}

void SaveStatusSnapshot(const std::string& filePath)
{
    std::ofstream out(filePath, std::ios::trunc);
    if (!out.is_open()) {
        return;
    }

    out << GetStatusJson();
}

void Reset()
{
    std::lock_guard<std::mutex> lock(gMutex);
    gStatus = Status{};
}

} // namespace LightningEngine::EditorBridge
