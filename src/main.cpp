#include <functional>

#include <platform/win32.h>
#include <engine/gl_renderer.h>
#include <ecs/ecs.h>

struct Transform
{
    vec2 position;
    vec2 size;
};

struct RectTransform
{
    vec2 position;
    vec2 size;
    vec2 anchor = vec2(0.0f); // 0.5 for center
    vec2 pivot = vec2(0.0f);  // 0.5 for center
};

vec2 GetUIPosition(const RectTransform& rt) {
    return vec2(
        (float)input.screen.x * rt.anchor.x - rt.size.x * rt.pivot.x + rt.position.x,
        (float)input.screen.y * rt.anchor.y - rt.size.y * rt.pivot.y + rt.position.y
    );
}

struct Sprite
{
    str path;
    vec3 color;
};

struct Text
{
    str content;
    float scale;
    vec3 color;
    TextAlignment horizontal = TextAlignment::Left;
    VerticalAlignment vertical = VerticalAlignment::Middle;

    vec2 padding = vec2(10.0f, 5.0f); // Default: 10px sides, 5px top/bottom
};

struct Image
{
    Sprite sprite;
};

struct Canvas
{
    Image image;
};

struct Button
{
    bool hovered;
    bool pressed;
    bool lastPressed;              // To detect the exact "Click" moment
    std::function<void()> onClick; // The callback function
};

struct Active
{
    bool IsActive = true;
};

struct Entity
{
    ecs::EntityID id;
    ecs::World *world;
    bool *enabled;

    // A helper to add components easily
    template <typename T>
    Entity &Add(T component)
    {
        world->AddComponent<T>(id, component);
        return *this; // This allows "Chaining" (linking calls together)
    }

    // A helper to get components
    template <typename T>
    T &Get()
    {
        return world->GetComponent<T>(id);
    }

    void SetActive(bool state)
    {
        if (world->HasComponent<Active>(id))
        {
            world->GetComponent<Active>(id).IsActive = state;
        }
    }
};

static ecs::World world;

void RenderSystem()
{
    // --- 1. WORLD SPACE ---
    auto *transforms = world.GetPool<Transform>();
    auto *sprites = world.GetPool<Sprite>();
    for (size_t i = 0; i < sprites->data.size(); ++i)
    {
        ecs::EntityID entity = sprites->denseToEntity[i];
        Transform &t = transforms->Get(entity);
        Sprite &s = sprites->data[i];
        DrawRectangle(t.position, t.size, s.color, Texture{.path = s.path});
    }

    // --- 2. UI SPACE: IMAGES (BACKGROUNDS) ---
    auto *rects = world.GetPool<RectTransform>();
    auto *images = world.GetPool<Image>();
    auto *buttons = world.GetPool<Button>(); // To check hover/press states

    auto *actives = world.GetPool<Active>(); // Get the active pool
    for (size_t i = 0; i < images->data.size(); ++i)
    {
        ecs::EntityID entity = images->denseToEntity[i];
        RectTransform &rt = rects->Get(entity);
        Image &img = images->data[i];

        if (world.HasComponent<Active>(entity) && !world.GetComponent<Active>(entity).IsActive)
            continue;

        vec3 finalColor = img.sprite.color;

        // Visual Feedback: If this entity is also a button, tint it
        if (world.HasComponent<Button>(entity))
        {
            Button &btn = world.GetComponent<Button>(entity);
            if (btn.pressed)
                finalColor *= 0.5f;
            else if (btn.hovered)
                finalColor *= 0.8f;
        }

        DrawRectangle(GetUIPosition(rt), rt.size, finalColor, Texture{.path = img.sprite.path});
    }

    // --- 3. UI SPACE: TEXT (LABELS) ---
    auto *texts = world.GetPool<Text>();

    for (size_t i = 0; i < texts->data.size(); ++i)
    {
        ecs::EntityID entity = texts->denseToEntity[i];
        Text &txt = texts->data[i];
        RectTransform &rt = rects->Get(entity);

        if (world.HasComponent<Active>(entity) && !world.GetComponent<Active>(entity).IsActive)
            continue;
        
        vec2 pos = GetUIPosition(rt);
        vec2 safePos = pos + txt.padding;
        vec2 safeSize = rt.size - (txt.padding * 2.0f);

        // Calculate anchor based on alignment
        vec2 renderAnchor = safePos;

        if (txt.horizontal == TextAlignment::Center)
        {
            renderAnchor.x = safePos.x + (safeSize.x * 0.5f);
        }
        else if (txt.horizontal == TextAlignment::Right)
        {
            renderAnchor.x = safePos.x + safeSize.x;
        }

        if (txt.vertical == VerticalAlignment::Middle)
        {
            renderAnchor.y = safePos.y + (safeSize.y * 0.5f);
        }
        else if (txt.vertical == VerticalAlignment::Top)
        {
            renderAnchor.y = safePos.y;
        }

        DrawTextUI(TextData{
                       .content = txt.content,
                       .scale = txt.scale,
                       .color = txt.color,
                       .maxWidth = safeSize.x},
                   renderAnchor, txt.vertical, txt.horizontal);
    }
}

void UpdateSystem()
{
    auto *buttons = world.GetPool<Button>();
    auto *rects = world.GetPool<RectTransform>();

    vec2 mousePos = vec2(input.mouseX, input.screen.y - input.mouseY);
    bool mouseIsDown = input.mouseDown[0];

    for (size_t i = 0; i < buttons->data.size(); ++i)
    {
        ecs::EntityID entity = buttons->denseToEntity[i];
        Button &btn = buttons->data[i];
        RectTransform &rt = rects->Get(entity);

        if (world.HasComponent<Active>(entity) && !world.GetComponent<Active>(entity).IsActive)
        {
            buttons->data[i].hovered = false;
            buttons->data[i].pressed = false;
            continue;
        }

        vec2 pos = GetUIPosition(rt);
        bool inside = (mousePos.x >= pos.x &&
                       mousePos.x <= pos.x + rt.size.x &&
                       mousePos.y >= pos.y &&
                       mousePos.y <= pos.y + rt.size.y);

        btn.hovered = inside;

        // Logic: Trigger when mouse is RELEASED over the button
        if (inside && btn.pressed && !mouseIsDown)
        {
            if (btn.onClick)
                btn.onClick();
        }

        btn.pressed = inside && mouseIsDown;
    }
}

void Update(float deltaTime);
void Start();
int main()
{
    if (!InitPlatform())
        return -1;
    if (!CreateWindowPlatform("Botum Engine", 956, 540))
        return -1;
    if (!InitGLRender())
        return -1;

    Start();
    while (!ShouldClose())
    {
        Event event;
        PollEvent(&event);

        Update(event.deltaTime);
        RenderSystem();

        glRender();
        SwapBuffersWindow();
    }
    DestroyGLContext();
    DestroyPlatform();
    return 0;
}

Entity button;
Entity mainmenu;

void Start()
{
    // Initialize entities
    mainmenu = {world.CreateEntity(), &world};
    button = {world.CreateEntity(), &world};

    // Setup Main Menu (the thing we want to hide/show)
    mainmenu.Add(Active{true})
        .Add(RectTransform{
            .position = vec2(0), 
            .size = vec2(400, 300),
            .anchor = vec2(0.5f), 
            .pivot = vec2(0.5f)})
        .Add(Image{.sprite = Sprite{.path = "assets/textures/profile.png", .color = vec3(0.2f)}});

    // Setup Toggle Button
    button.Add(Active{true})
        .Add(RectTransform{
            .position = vec2(0, -150), // 150 pixels below center
            .size = vec2(200, 60), 
            .anchor = vec2(0.5f), 
            .pivot = vec2(0.5f)})
        .Add(Image{.sprite = Sprite{.path = "assets/textures/sprite.png", .color = vec3(1.0f)}})
        .Add(Text{
            .content = "Toggle Menu",
            .scale = 0.5f,
            .color = vec3(0.0f),
            .horizontal = TextAlignment::Center,
            .vertical = VerticalAlignment::Middle})
        .Add(Button{.onClick = []()
                    {
                        // Access the 'mainmenu' global variable
                        bool currentState = world.GetComponent<Active>(mainmenu.id).IsActive;
                        mainmenu.SetActive(!currentState);

                        printf("Menu is now %s\n", !currentState ? "VISIBLE" : "HIDDEN");
                    }});
}

void Update(float deltaTime)
{
    UpdateSystem();
}
