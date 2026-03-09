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
};

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

struct Entity
{
    ecs::EntityID id;
    ecs::World *world;

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

    for (size_t i = 0; i < images->data.size(); ++i)
    {
        ecs::EntityID entity = images->denseToEntity[i];
        RectTransform &t = rects->Get(entity);
        Image &img = images->data[i];

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

        DrawRectangle(t.position, t.size, finalColor, Texture{.path = img.sprite.path});
    }

    // --- 3. UI SPACE: TEXT (LABELS) ---
    auto *texts = world.GetPool<Text>();
    for (size_t i = 0; i < texts->data.size(); ++i)
    {
        ecs::EntityID entity = texts->denseToEntity[i];
        Text &txt = texts->data[i];
        RectTransform &rect = rects->Get(entity);

        vec2 safePos = rect.position + txt.padding;
        vec2 safeSize = rect.size - (txt.padding * 2.0f);

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
        RectTransform &rect = rects->Get(entity);

        bool inside = (mousePos.x >= rect.position.x &&
                       mousePos.x <= rect.position.x + rect.size.x &&
                       mousePos.y >= rect.position.y &&
                       mousePos.y <= rect.position.y + rect.size.y);

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

void Start()
{
    Entity player = {.id = world.CreateEntity(), .world = &world};
    Entity text = {.id = world.CreateEntity(), .world = &world};

    player.Add(
              Transform{
                  .position = vec2(100),
                  .size = vec2(50)})
        .Add(Sprite{"assets/textures/profile.jpg", vec3(1.0f)});
    text.Add(
            RectTransform{
                .position = vec2(10, 10),
                .size = vec2(300, 100),
            })
        .Add(
            Image{
                .sprite = Sprite{
                    .path = "assets/textures/profile.jpg",
                    .color = vec3(1.0f, 0.8f, 0.5f)}});

    Entity myBtn = {world.CreateEntity(), &world};

    myBtn.Add(RectTransform{vec2(400, 200), vec2(200, 60)})
        .Add(Image{Sprite{"white_quad", vec3(1.0f)}})
        .Add(Text{
            .content = "Click Me!",
            .scale = 0.8f,
            .color = vec3(0.0f), // Black text on white button
            .horizontal = TextAlignment::Center,
            .vertical = VerticalAlignment::Middle,
            .padding = vec2(20.0f, 10.0f) // Extra breathing room
        })
        .Add(Button{
            .onClick = []()
            {
                printf("Button Clicked! Spawning an entity...\n");
                // You can even interact with the world here!
            }});

    Entity khmerText = {world.CreateEntity(), &world};
    khmerText.Add(RectTransform{vec2(100, 400), vec2(400, 100)})
        .Add(Text{
            .content = "សួស្តីពិភពលោក (Hello World)",
            .scale = 1.0f,
            .color = vec3(1.0f),
            .horizontal = TextAlignment::Left,
            .vertical = VerticalAlignment::Middle
        });
    }

void Update(float deltaTime)
{
    UpdateSystem();
}
