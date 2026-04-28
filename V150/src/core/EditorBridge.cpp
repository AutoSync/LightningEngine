#include "../include/EditorBridge.h"
#include "../include/Level.h"
#include "../include/Node.h"
#include <mutex>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <typeinfo>
#include <typeindex>
#include <cstring>
#include <cstdlib>

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

// ─────────────────────────────────────────────────────────────────────────────
// Scene contract
// ─────────────────────────────────────────────────────────────────────────────

namespace {
    LightningEngine::Level* gActiveLevel = nullptr;
    const void*             gSelectedNodePtr = nullptr;
    CommandHandlers         gHandlers;

    std::string ptrToId(const void* p)
    {
        if (!p) return std::string();
        std::ostringstream ss;
        ss << "0x" << std::hex << reinterpret_cast<std::uintptr_t>(p);
        return ss.str();
    }

    // Best-effort prettify of typeid().name() across MSVC/GCC.
    std::string prettifyTypeName(const char* raw)
    {
        if (!raw) return "Component";
        std::string s = raw;
        // MSVC: "class LightningEngine::SpriteRenderer" / "struct ..."
        const char* prefixes[] = { "class ", "struct " };
        for (auto* p : prefixes) {
            auto plen = std::strlen(p);
            if (s.compare(0, plen, p) == 0) { s.erase(0, plen); break; }
        }
        auto pos = s.rfind("::");
        if (pos != std::string::npos) s = s.substr(pos + 2);
        return s.empty() ? std::string("Component") : s;
    }

    void serializeNode(std::ostringstream& out,
                       const LightningEngine::Node* node,
                       const LightningEngine::Node* parent,
                       bool& first)
    {
        if (!node) return;
        if (!first) out << ',';
        first = false;

        const auto& t = node->transform;
        out << '{'
            << "\"id\":\"" << ptrToId(node) << "\","
            << "\"parentId\":" << (parent ? ("\"" + ptrToId(parent) + "\"") : std::string("null")) << ','
            << "\"name\":\"" << jsonEscape(node->name) << "\","
            << "\"tag\":\"" << jsonEscape(node->tag) << "\","
            << "\"active\":" << (node->active ? "true" : "false") << ','
            << "\"components\":[";
        bool firstComp = true;
        for (const auto& kv : node->GetComponentsMap()) {
            if (!firstComp) out << ',';
            firstComp = false;
            out << "\"" << jsonEscape(prettifyTypeName(kv.first.name())) << "\"";
        }
        out << "],"
            << "\"transform\":{"
            << "\"px\":" << t.Position.x << ",\"py\":" << t.Position.y << ",\"pz\":" << t.Position.z << ','
            << "\"rx\":" << t.Rotation.x << ",\"ry\":" << t.Rotation.y << ",\"rz\":" << t.Rotation.z << ','
            << "\"sx\":" << t.Scale.x    << ",\"sy\":" << t.Scale.y    << ",\"sz\":" << t.Scale.z
            << "}}";

        for (const auto& child : node->GetChildren()) {
            serializeNode(out, child.get(), node, first);
        }
    }
}

void SetActiveLevel(LightningEngine::Level* level)
{
    std::lock_guard<std::mutex> lock(gMutex);
    gActiveLevel = level;
    if (!level) gSelectedNodePtr = nullptr;
}

void SetSelectedNodePtr(const void* nodePtr)
{
    std::lock_guard<std::mutex> lock(gMutex);
    gSelectedNodePtr = nodePtr;
}

std::string GetSceneJson()
{
    std::lock_guard<std::mutex> lock(gMutex);
    std::ostringstream out;
    out << '{'
        << "\"scene\":\"" << jsonEscape(gStatus.scene) << "\","
        << "\"selectedId\":" << (gSelectedNodePtr ? ("\"" + ptrToId(gSelectedNodePtr) + "\"") : std::string("null")) << ','
        << "\"nodes\":[";
    if (gActiveLevel) {
        bool first = true;
        for (const auto& root : gActiveLevel->GetNodes()) {
            serializeNode(out, root.get(), nullptr, first);
        }
    }
    out << "]}";
    return out.str();
}

void SaveSceneSnapshot(const std::string& filePath)
{
    std::ofstream out(filePath, std::ios::trunc);
    if (!out.is_open()) return;
    out << GetSceneJson();
}

void RegisterCommandHandlers(CommandHandlers handlers)
{
    std::lock_guard<std::mutex> lock(gMutex);
    gHandlers = std::move(handlers);
}

void ClearCommandHandlers()
{
    std::lock_guard<std::mutex> lock(gMutex);
    gHandlers = CommandHandlers{};
}

namespace {
    // Tiny JSON probing — avoids pulling a JSON lib for the bridge.
    // Extracts a string field "key":"value" and returns true if found.
    bool jsonGetString(const std::string& src, const std::string& key, std::string& out)
    {
        std::string needle = "\"" + key + "\"";
        auto k = src.find(needle);
        if (k == std::string::npos) return false;
        auto colon = src.find(':', k + needle.size());
        if (colon == std::string::npos) return false;
        // Skip whitespace
        auto i = colon + 1;
        while (i < src.size() && (src[i] == ' ' || src[i] == '\t')) ++i;
        if (i >= src.size()) return false;
        if (src[i] == 'n') return false; // null
        if (src[i] != '"') return false;
        ++i;
        std::string acc;
        while (i < src.size() && src[i] != '"') {
            if (src[i] == '\\' && i + 1 < src.size()) {
                char e = src[i + 1];
                switch (e) {
                    case 'n': acc.push_back('\n'); break;
                    case 't': acc.push_back('\t'); break;
                    case '"': acc.push_back('"');  break;
                    case '\\': acc.push_back('\\'); break;
                    default: acc.push_back(e); break;
                }
                i += 2;
            } else {
                acc.push_back(src[i]);
                ++i;
            }
        }
        out = acc;
        return true;
    }

    bool jsonGetNumber(const std::string& src, const std::string& key, float& out)
    {
        std::string needle = "\"" + key + "\"";
        auto k = src.find(needle);
        if (k == std::string::npos) return false;
        auto colon = src.find(':', k + needle.size());
        if (colon == std::string::npos) return false;
        auto i = colon + 1;
        while (i < src.size() && (src[i] == ' ' || src[i] == '\t')) ++i;
        if (i >= src.size()) return false;
        // Read number
        std::string num;
        while (i < src.size() && (isdigit((unsigned char)src[i]) || src[i] == '-' ||
                                  src[i] == '+' || src[i] == '.' || src[i] == 'e' || src[i] == 'E')) {
            num.push_back(src[i]);
            ++i;
        }
        if (num.empty()) return false;
        try { out = std::stof(num); } catch (...) { return false; }
        return true;
    }

    std::string makeResult(bool ok, const std::string& errOrPayload = std::string(),
                           bool payloadIsId = false)
    {
        std::ostringstream out;
        out << '{' << "\"ok\":" << (ok ? "true" : "false");
        if (ok && payloadIsId && !errOrPayload.empty()) {
            out << ",\"id\":\"" << jsonEscape(errOrPayload) << "\"";
        } else if (!ok && !errOrPayload.empty()) {
            out << ",\"error\":\"" << jsonEscape(errOrPayload) << "\"";
        }
        out << '}';
        return out.str();
    }
}

std::string DispatchCommand(const std::string& commandJson)
{
    CommandHandlers h;
    {
        std::lock_guard<std::mutex> lock(gMutex);
        h = gHandlers;
    }

    std::string op;
    if (!jsonGetString(commandJson, "op", op)) {
        return makeResult(false, "missing op");
    }

    if (op == "select") {
        if (!h.selectById) return makeResult(false, "unsupported");
        std::string id;
        if (!jsonGetString(commandJson, "id", id)) return makeResult(false, "missing id");
        bool ok = h.selectById(id);
        return makeResult(ok, ok ? "" : "not found");
    }

    if (op == "deselect") {
        if (!h.deselect) return makeResult(false, "unsupported");
        return makeResult(h.deselect());
    }

    if (op == "setTransform") {
        if (!h.setTransform) return makeResult(false, "unsupported");
        std::string id;
        if (!jsonGetString(commandJson, "id", id)) return makeResult(false, "missing id");
        float x = 0.f, y = 0.f, z = 0.f;
        bool hx = jsonGetNumber(commandJson, "px", x);
        bool hy = jsonGetNumber(commandJson, "py", y);
        bool hz = jsonGetNumber(commandJson, "pz", z);
        bool ok = h.setTransform(id, hx, x, hy, y, hz, z);
        return makeResult(ok, ok ? "" : "not found");
    }

    if (op == "createNode") {
        if (!h.createNode) return makeResult(false, "unsupported");
        std::string parentId;
        jsonGetString(commandJson, "parentId", parentId); // optional
        std::string name = "Node";
        jsonGetString(commandJson, "name", name);
        std::string archetype = "empty";
        jsonGetString(commandJson, "archetype", archetype);
        std::string newId = h.createNode(parentId, name, archetype);
        if (newId.empty()) return makeResult(false, "create failed");
        return makeResult(true, newId, /*payloadIsId=*/true);
    }

    if (op == "deleteNode") {
        if (!h.deleteNode) return makeResult(false, "unsupported");
        std::string id;
        if (!jsonGetString(commandJson, "id", id)) return makeResult(false, "missing id");
        bool ok = h.deleteNode(id);
        return makeResult(ok, ok ? "" : "not found");
    }

    return makeResult(false, "unknown op");
}

int DrainCommandQueue(const std::string& filePath)
{
    // Read all, then truncate. The host writes append-only JSONL.
    // We accept lines with or without trailing whitespace.
    std::ifstream in(filePath);
    if (!in.is_open()) return 0;

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        // Trim trailing \r (Windows line-endings)
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t'))
            line.pop_back();
        if (!line.empty()) lines.push_back(line);
    }
    in.close();

    if (lines.empty()) return 0;

    // Truncate the queue immediately so concurrent writes from the host
    // are not lost (host always appends; we only ever fight the tail).
    {
        std::ofstream trunc(filePath, std::ios::trunc);
        // Closing empties the file.
    }

    int processed = 0;
    for (const auto& cmd : lines) {
        DispatchCommand(cmd);
        ++processed;
    }
    return processed;
}

} // namespace LightningEngine::EditorBridge
