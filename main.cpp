#include <iostream>
#include <vector>
#include <optional>
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>

// --- 配置常量 ---
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float SCALE = 30.0f; // 像素/米
const float BOX_PIXEL_SIZE = 30.0f; // 箱子在屏幕上的显示大小（像素）

// --- 全局变量 ---
b2WorldId world; 
b2BodyId groundBody;
std::vector<b2BodyId> dynamicBodies;

// --- 初始化物理世界 ---
void initGame() {
    b2Vec2 gravity = {0.0f, 9.8f};
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = gravity;
    world = b2CreateWorld(&worldDef);

    // 地面
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

// --- 创建箱子 (物理部分) ---
void createBox(float mouseX, float mouseY) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {mouseX / SCALE, mouseY / SCALE};
    
    b2BodyId body = b2CreateBody(world, &bodyDef);
    dynamicBodies.push_back(body);

    // 物理尺寸是显示尺寸的一半（半宽/半高），并转换为米
    float half_size_meters = (BOX_PIXEL_SIZE / 2.0f) / SCALE;
    b2Polygon dynamicBox = b2MakeBox(half_size_meters, half_size_meters);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    // 注意：删除了 shapeDef.restitution = ...

    b2ShapeId shapeId = b2CreatePolygonShape(body, &shapeDef, &dynamicBox);
    
    // --- 修复：使用函数设置物理属性 ---
    b2Shape_SetFriction(shapeId, 0.3f);     // 设置摩擦力
    b2Shape_SetRestitution(shapeId, 0.3f);  // 设置弹性 (0.0 = 无弹性, 1.0 = 完全弹性)
    
    std::cout << "Box created at " << mouseX << ", " << mouseY << std::endl;
}

// --- 主循环 ---
int main() {
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Box2D v3 + SFML 3.0 Texture");
    window.setFramerateLimit(60);

    // --- 1. 加载纹理 ---
    sf::Texture boxTexture;
    bool textureLoaded = false;
    
    // 确保你有一个名为 box.png 的文件在当前目录下
    // 如果没有图片，控制台会报错，但程序会运行（显示红色方块）
    if (!boxTexture.loadFromFile("res/box.png")) {
        std::cerr << "Warning: Could not load box.png! Using default red box." << std::endl;
    } else {
        textureLoaded = true;
        boxTexture.setSmooth(true);
    }

    // --- 2. 设置 Sprite (精灵) ---
    sf::Sprite boxSprite(boxTexture);

    if (textureLoaded) {
        sf::Vector2u texSize = boxTexture.getSize();
        // 设置原点到图片中心
        boxSprite.setOrigin({float(texSize.x) / 2.0f, float(texSize.y) / 2.0f});
        // 计算缩放比例
        float scaleX = BOX_PIXEL_SIZE / float(texSize.x);
        float scaleY = BOX_PIXEL_SIZE / float(texSize.y);
        boxSprite.setScale({scaleX, scaleY});
    } else {
        // 备用方案：如果没有图片，设置一个红色的矩形区域
        boxSprite.setTextureRect(sf::IntRect({0, 0}, {30, 30}));
        boxSprite.setOrigin({15.0f, 15.0f});
        boxSprite.setColor(sf::Color::Red); 
    }

    initGame();
    createBox(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 4.0f);

    while (window.isOpen()) {
        // 事件处理
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseBtn->button == sf::Mouse::Button::Left) {
                    createBox(static_cast<float>(mouseBtn->position.x), 
                              static_cast<float>(mouseBtn->position.y));
                }
            }
        }

        // 物理步进
        b2World_Step(world, 1.0f / 60.0f, 4);

        // 渲染
        window.clear(sf::Color::Black);

        // 绘制地面
        sf::RectangleShape groundRect(sf::Vector2f(WINDOW_WIDTH, 20.0f));
        groundRect.setOrigin({WINDOW_WIDTH / 2.0f, 10.0f});
        b2Vec2 gPos = b2Body_GetPosition(groundBody);
        groundRect.setPosition({gPos.x * SCALE, gPos.y * SCALE});
        groundRect.setFillColor(sf::Color::Green);
        window.draw(groundRect);

        // 绘制所有箱子
        for (b2BodyId bodyId : dynamicBodies) {
            b2Vec2 pos = b2Body_GetPosition(bodyId);
            b2Rot rot = b2Body_GetRotation(bodyId);
            float angle = b2Rot_GetAngle(rot); // 弧度

            // 更新 Sprite 的位置和旋转
            boxSprite.setPosition({pos.x * SCALE, pos.y * SCALE});
            boxSprite.setRotation(sf::radians(angle));

            window.draw(boxSprite);
        }

        window.display();
    }

    b2DestroyWorld(world);
    return 0;
}
