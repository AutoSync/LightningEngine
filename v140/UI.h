#pragma once

#include "GameNode.h"
#include "Shader.h"

#include <functional>
#include <memory>
#include <string>

namespace LightningEngine
{
    struct UIRect
    {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;

        bool Contains(V2 point) const
        {
            return point.x >= x && point.y >= y &&
                   point.x <= x + width && point.y <= y + height;
        }
    };

    struct UIStyle
    {
        C3 background = C3(42, 42, 48);
        C3 foreground = C3(235, 235, 235);
        C3 border = C3(90, 90, 100);
        float borderWidth = 0.0f;
    };

    class UIRenderer
    {
    public:
        UIRenderer() = default;
        ~UIRenderer();

        bool Initialize();
        void Shutdown();
        bool IsInitialized() const { return initialized; }
        void SetViewport(float width, float height);
        void DrawRect(const UIRect& rect, C3 color);

    private:
        std::unique_ptr<Shader> shader;
        unsigned int VAO = 0;
        unsigned int VBO = 0;
        unsigned int EBO = 0;
        float viewportWidth = 1.0f;
        float viewportHeight = 1.0f;
        bool initialized = false;
    };

    // Base node for UI layout. Its rect is local to its UI parent and its
    // world rect is calculated in canvas/screen coordinates.
    class UINode : public GameNode
    {
    public:
        explicit UINode(const char* nodeName = "UINode") : GameNode(nodeName) {}
        explicit UINode(const std::string& nodeName) : GameNode(nodeName) {}
        ~UINode() override = default;

        UIRect rect;
        bool visible = true;

        void SetRect(UIRect value) { rect = value; }
        void SetPosition(float x, float y) { rect.x = x; rect.y = y; }
        void SetSize(float width, float height)
        {
            rect.width = width;
            rect.height = height;
        }
        UIRect GetRect() const { return rect; }

        UIRect GetWorldRect() const
        {
            const auto* parentNode = dynamic_cast<const UINode*>(GetParent());
            if (parentNode)
            {
                const UIRect parentRect = parentNode->GetWorldRect();
                return { parentRect.x + rect.x, parentRect.y + rect.y,
                         rect.width, rect.height };
            }
            return rect;
        }

        bool IsVisibleInHierarchy() const
        {
            const auto* parentNode = dynamic_cast<const UINode*>(GetParent());
            return visible && IsActiveInHierarchy() &&
                   (!parentNode || parentNode->IsVisibleInHierarchy());
        }
    };

    class Widget;

    // Canvas is the UI root. Attach it below any GameNode; widgets can then be
    // attached below the canvas or below another widget.
    class Canvas : public UINode
    {
    public:
        explicit Canvas(float width = 840.0f, float height = 480.0f)
            : UINode("Canvas")
        {
            rect = { 0.0f, 0.0f, width, height };
        }

        ~Canvas() override = default;

        UIRenderer* GetRenderer() const { return renderer.get(); }
        V2 GetMousePosition() const { return mousePosition; }
        Widget* GetHoveredWidget() const { return hoveredWidget; }

        void SetViewport(float width, float height)
        {
            rect.width = width;
            rect.height = height;
            if (renderer)
                renderer->SetViewport(width, height);
        }

    protected:
        void OnStart() override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;
        void OnDestroy() override;

    private:
        Widget* HitTest(const GameNode* node, V2 point) const;
        void UpdatePointerState();

        std::unique_ptr<UIRenderer> renderer;
        V2 mousePosition;
        Widget* hoveredWidget = nullptr;
        Widget* pressedWidget = nullptr;
        bool previousLeftDown = false;
    };

    // Interactive UI node. Subclasses implement their visual appearance and
    // can use the callbacks for game-side behaviour.
    class Widget : public UINode
    {
    public:
        explicit Widget(const char* nodeName = "Widget") : UINode(nodeName) {}
        explicit Widget(const std::string& nodeName) : UINode(nodeName) {}
        ~Widget() override = default;

        UIStyle style;
        bool mouseEnabled = true;
        bool hovered = false;
        bool pressed = false;

        std::function<void(Widget&)> onMouseEnter;
        std::function<void(Widget&)> onMouseLeave;
        std::function<void(Widget&, V2)> onMouseMove;
        std::function<void(Widget&)> onMouseDown;
        std::function<void(Widget&)> onMouseUp;
        std::function<void(Widget&)> onClick;

        bool HitTest(V2 point) const
        {
            return mouseEnabled && IsVisibleInHierarchy() && GetWorldRect().Contains(point);
        }

        bool IsHovered() const { return hovered; }
        bool IsPressed() const { return pressed; }

    protected:
        friend class Canvas;

        virtual void Draw(UIRenderer* uiRenderer)
        {
            if (uiRenderer)
                uiRenderer->DrawRect(GetWorldRect(), style.background);
        }

        void OnRender() override;
        virtual void OnMouseEnter(V2 point);
        virtual void OnMouseLeave(V2 point);
        virtual void OnMouseMove(V2 point);
        virtual void OnMouseDown(V2 point);
        virtual void OnMouseUp(V2 point);
        virtual void OnClick();

        Canvas* GetCanvas() const;
    };

    class Panel : public Widget
    {
    public:
        explicit Panel(const char* nodeName = "Panel") : Widget(nodeName) {}
        explicit Panel(const std::string& nodeName) : Widget(nodeName) {}
    };

    class Button : public Widget
    {
    public:
        explicit Button(const char* nodeName = "Button") : Widget(nodeName) {}
        explicit Button(const std::string& nodeName) : Widget(nodeName) {}

        C3 normalColor = C3(62, 78, 110);
        C3 hoverColor = C3(82, 106, 150);
        C3 pressedColor = C3(45, 56, 82);
        std::string text;

        void SetText(const std::string& value) { text = value; }
        const std::string& GetText() const { return text; }

    protected:
        void Draw(UIRenderer* uiRenderer) override;
    };

    // Label stores text/layout. v0.14 has no font atlas yet, so glyphs are not
    // rasterized by this class.
    class Label : public Widget
    {
    public:
        explicit Label(const char* nodeName = "Label") : Widget(nodeName) {}
        explicit Label(const std::string& nodeName) : Widget(nodeName) {}

        std::string text;
        float fontSize = 16.0f;

        void SetText(const std::string& value) { text = value; }
        const std::string& GetText() const { return text; }

    protected:
        void Draw(UIRenderer* /*uiRenderer*/) override {}
    };

    using UIWidget = Widget;
}

