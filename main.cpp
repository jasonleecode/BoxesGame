#include <iostream>
#include <vector>
#include <optional> // SFML 3.0 事件循环需要
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>

// --- Configuration Constants ---
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float SCALE = 30.0f; 
const float PI = 3.1415926535f; // 手动定义 PI，防止 Box2D 版本差异

// --- Global Variables ---
b2WorldId world; 
b2BodyId groundBody;
std::vector<b2BodyId> dynamicBodies;

// --- Initialization ---
void initGame() {
    b2Vec2 gravity = {0.0f, 9.8f};
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = gravity;
    world = b2CreateWorld(&worldDef);

    // Ground
    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = {WINDOW_WIDTH / 2.0f / SCALE, (WINDOW_HEIGHT - 10.0f) / SCALE};
    groundBody = b2CreateBody(world, &groundBodyDef);

    float halfWidth = WINDOW_WIDTH / 2.0f / SCALE;
    float halfHeight = 10.0f / SCALE;
    b2Polygon groundBox = b2MakeBox(halfWidth, halfHeight);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 0.0f;
    
    b2ShapeId groundShapeId = b2CreatePolygonShape(groundBody, &shapeDef, &groundBox);
    b2Shape_SetFriction(groundShapeId, 0.5f);
}

// --- Create Box ---
void createBox(float mouseX, float mouseY) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {mouseX / SCALE, mouseY / SCALE};
    
    b2BodyId body = b2CreateBody(world, &bodyDef);
    dynamicBodies.push_back(body);

    float half_size = (30.0f / 2.0f) / SCALE;
    b2Polygon dynamicBox = b2MakeBox(half_size, half_size);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;

    b2ShapeId shapeId = b2CreatePolygonShape(body, &shapeDef, &dynamicBox);
    b2Shape_SetFriction(shapeId, 0.3f);
    
    std::cout << "Box created at " << mouseX << ", " << mouseY << std::endl;
}

// --- Main Loop ---
int main() {
    // SFML 3.0: VideoMode 构造函数需要 Vector2u，使用 {} 初始化
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Box2D v3 + SFML 3.0");
    window.setFramerateLimit(60);

    initGame();
    createBox(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 4.0f);

    while (window.isOpen()) {
        // --- SFML 3.0 事件处理 ---
        // pollEvent 现在返回 std::optional
        while (const std::optional event = window.pollEvent()) {
            // 使用 is<Type>() 检查事件类型
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            // 使用 getIf<Type>() 获取具体事件数据
            else if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                // 枚举作用域变更: sf::Mouse::Left -> sf::Mouse::Button::Left
                if (mouseBtn->button == sf::Mouse::Button::Left) {
                    // 坐标现在是 Vector2，直接访问 .x .y 可能会变，但在 MouseButtonEvent 中通常是 position.x
                    createBox(static_cast<float>(mouseBtn->position.x), 
                              static_cast<float>(mouseBtn->position.y));
                }
            }
        }

        // 2. Physics Step
        b2World_Step(world, 1.0f / 60.0f, 4);

        // 3. Rendering
        window.clear(sf::Color::Black);

        // Draw Ground (Static)
        sf::RectangleShape groundRect(sf::Vector2f(WINDOW_WIDTH, 20.0f));
        // SFML 3.0: setOrigin 需要 Vector2f
        groundRect.setOrigin({WINDOW_WIDTH / 2.0f, 10.0f});
        
        b2Vec2 gPos = b2Body_GetPosition(groundBody);
        // SFML 3.0: setPosition 需要 Vector2f
        groundRect.setPosition({gPos.x * SCALE, gPos.y * SCALE});
        groundRect.setFillColor(sf::Color::Green);
        window.draw(groundRect);

        // Draw Dynamic Bodies
        sf::RectangleShape boxRect(sf::Vector2f(30.0f, 30.0f));
        boxRect.setOrigin({15.0f, 15.0f}); // Center origin
        boxRect.setFillColor(sf::Color::Red);

        for (b2BodyId bodyId : dynamicBodies) {
            b2Vec2 pos = b2Body_GetPosition(bodyId);
            b2Rot rot = b2Body_GetRotation(bodyId);
            float angle = b2Rot_GetAngle(rot); // Radians

            boxRect.setPosition({pos.x * SCALE, pos.y * SCALE});
            boxRect.setRotation(sf::radians(angle)); // SFML 3.0 推荐使用 sf::radians/degrees
            // 或者如果你的 SFML 版本还不支持 sf::radians，使用旧式转换:
            // boxRect.setRotation(angle * 180.0f / PI); 
            
            window.draw(boxRect);
        }

        window.display();
    }

    b2DestroyWorld(world);
    return 0;
}