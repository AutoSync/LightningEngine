#include "../../UI.h"

#include <GLFW/glfw3.h>

namespace
{
    constexpr const char* UI_VERTEX_SHADER = R"glsl(
#version 330 core
layout (location = 0) in vec2 Position;
layout (location = 1) in vec3 VertexColor;
uniform vec2 viewport;
out vec3 Color;
void main()
{
    vec2 ndc = vec2((Position.x / viewport.x) * 2.0 - 1.0,
                    1.0 - (Position.y / viewport.y) * 2.0);
    gl_Position = vec4(ndc, 0.0, 1.0);
    Color = VertexColor;
}
)glsl";

    constexpr const char* UI_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec3 Color;
out vec4 FragColor;
void main()
{
    FragColor = vec4(Color, 1.0);
}
)glsl";
}

namespace LightningEngine
{
    UIRenderer::~UIRenderer()
    {
        Shutdown();
    }

    bool UIRenderer::Initialize()
    {
        if (initialized)
            return true;
        if (glfwGetCurrentContext() == nullptr)
            return false;

        shader = std::make_unique<Shader>(ShaderSource(UI_VERTEX_SHADER, UI_FRAGMENT_SHADER));
        shader->Init();
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        initialized = true;
        return true;
    }

    void UIRenderer::Shutdown()
    {
        if (glfwGetCurrentContext() != nullptr)
        {
            if (shader && shader->id != 0)
                glDeleteProgram(shader->id);
            if (VAO != 0)
                glDeleteVertexArrays(1, &VAO);
            if (VBO != 0)
                glDeleteBuffers(1, &VBO);
            if (EBO != 0)
                glDeleteBuffers(1, &EBO);
        }
        shader.reset();
        VAO = VBO = EBO = 0;
        initialized = false;
    }

    void UIRenderer::SetViewport(float width, float height)
    {
        viewportWidth = width > 0.0f ? width : 1.0f;
        viewportHeight = height > 0.0f ? height : 1.0f;
    }

    void UIRenderer::DrawRect(const UIRect& rect, C3 color)
    {
        if (!initialized || !shader)
            return;

        const float vertices[] = {
            rect.x,              rect.y,               color.r, color.g, color.b,
            rect.x + rect.width, rect.y,               color.r, color.g, color.b,
            rect.x + rect.width, rect.y + rect.height, color.r, color.g, color.b,
            rect.x,              rect.y + rect.height, color.r, color.g, color.b
        };
        const unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

        const GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
        const GLboolean blendingEnabled = glIsEnabled(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);

        shader->Init();
        shader->SetV2("viewport", V2(viewportWidth, viewportHeight));
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        if (depthTestEnabled)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
        if (blendingEnabled)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);
    }

    void Canvas::OnStart()
    {
        renderer = std::make_unique<UIRenderer>();
        renderer->SetViewport(rect.width, rect.height);
        renderer->Initialize();
        previousLeftDown = false;
        hoveredWidget = nullptr;
        pressedWidget = nullptr;
    }

    void Canvas::OnUpdate(float /*deltaTime*/)
    {
        UpdatePointerState();
    }

    void Canvas::OnRender()
    {
        // UIRenderer preserves the caller's OpenGL state around each widget.
    }

    void Canvas::OnDestroy()
    {
        hoveredWidget = nullptr;
        pressedWidget = nullptr;
        renderer.reset();
    }

    Widget* Canvas::HitTest(const GameNode* node, V2 point) const
    {
        const auto& nodes = node->GetChildren();
        for (auto iterator = nodes.rbegin(); iterator != nodes.rend(); ++iterator)
        {
            const GameNode* child = iterator->get();
            const auto* uiNode = dynamic_cast<const UINode*>(child);
            if (!uiNode || !uiNode->IsVisibleInHierarchy())
                continue;

            if (Widget* nested = HitTest(child, point))
                return nested;

            if (auto* widget = dynamic_cast<Widget*>(const_cast<GameNode*>(child)))
            {
                if (widget->HitTest(point))
                    return widget;
            }
        }
        return nullptr;
    }

    void Canvas::UpdatePointerState()
    {
        GLFWwindow* window = glfwGetCurrentContext();
        if (window == nullptr)
            return;

        double mouseX = 0.0;
        double mouseY = 0.0;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        mousePosition = V2(static_cast<float>(mouseX), static_cast<float>(mouseY));

        Widget* nextHovered = HitTest(this, mousePosition);
        if (nextHovered != hoveredWidget)
        {
            if (hoveredWidget)
            {
                hoveredWidget->hovered = false;
                hoveredWidget->OnMouseLeave(mousePosition);
            }
            hoveredWidget = nextHovered;
            if (hoveredWidget)
            {
                hoveredWidget->hovered = true;
                hoveredWidget->OnMouseEnter(mousePosition);
            }
        }

        if (hoveredWidget)
            hoveredWidget->OnMouseMove(mousePosition);

        const bool leftDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        if (leftDown && !previousLeftDown)
        {
            pressedWidget = hoveredWidget;
            if (pressedWidget)
            {
                pressedWidget->pressed = true;
                pressedWidget->OnMouseDown(mousePosition);
            }
        }
        else if (!leftDown && previousLeftDown)
        {
            if (pressedWidget)
            {
                Widget* released = pressedWidget;
                released->pressed = false;
                released->OnMouseUp(mousePosition);
                if (released == hoveredWidget)
                    released->OnClick();
            }
            pressedWidget = nullptr;
        }
        previousLeftDown = leftDown;
    }

    void Widget::OnRender()
    {
        if (visible && IsActiveInHierarchy())
            Draw(GetCanvas() ? GetCanvas()->GetRenderer() : nullptr);
    }

    void Widget::OnMouseEnter(V2 point)
    {
        if (onMouseEnter)
            onMouseEnter(*this);
        (void)point;
    }

    void Widget::OnMouseLeave(V2 point)
    {
        if (onMouseLeave)
            onMouseLeave(*this);
        (void)point;
    }

    void Widget::OnMouseMove(V2 point)
    {
        if (onMouseMove)
            onMouseMove(*this, point);
    }

    void Widget::OnMouseDown(V2 point)
    {
        if (onMouseDown)
            onMouseDown(*this);
        (void)point;
    }

    void Widget::OnMouseUp(V2 point)
    {
        if (onMouseUp)
            onMouseUp(*this);
        (void)point;
    }

    void Widget::OnClick()
    {
        if (onClick)
            onClick(*this);
    }

    Canvas* Widget::GetCanvas() const
    {
        const GameNode* node = GetParent();
        while (node)
        {
            if (auto* canvas = dynamic_cast<Canvas*>(const_cast<GameNode*>(node)))
                return canvas;
            node = node->GetParent();
        }
        return nullptr;
    }

    void Button::Draw(UIRenderer* uiRenderer)
    {
        if (!uiRenderer)
            return;
        C3 color = pressed ? pressedColor : (hovered ? hoverColor : normalColor);
        uiRenderer->DrawRect(GetWorldRect(), color);
    }
}
