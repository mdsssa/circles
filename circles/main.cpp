#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

class EpidemicChart{
public:
    int chartX, chartY, chartW, chartH;
    std::vector<int> infectedHistory;
    std::vector<int> recoveredHistory;
    std::vector<int> healthyHistory;
    int maxValue;
    int totalCount;

    EpidemicChart(int x, int y, int w, int h, int total)
        : chartX(x), chartY(y), chartW(w), chartH(h), maxValue(1), totalCount(total) {}

    void update(int infected, int recovered)
    {
        int healthy = totalCount - infected - recovered;
        infectedHistory.push_back(infected);
        recoveredHistory.push_back(recovered);
        healthyHistory.push_back(healthy);
        maxValue = std::max(maxValue, totalCount);
    }

    void draw(sf::RenderWindow& window) const
    {
        sf::RectangleShape bg(sf::Vector2f(chartW, chartH));
        bg.setPosition({(float)chartX, (float)chartY});
        bg.setFillColor(sf::Color(20, 20, 20, 200));
        bg.setOutlineColor(sf::Color::White);
        bg.setOutlineThickness(1);
        window.draw(bg);

        int n = infectedHistory.size();
        if (n < 2) return;

        auto drawLine = [&](const std::vector<int>& data, sf::Color color)
        {
            sf::Color fillColor(color.r, color.g, color.b, 60);
            sf::VertexArray fill(sf::PrimitiveType::TriangleStrip, n * 2);
            for (int i = 0; i < n; i++)
            {
                float px = chartX + (float)i / (n - 1) * chartW;
                float py = chartY + chartH - (float)data[i] / maxValue * chartH;
                fill[i * 2].position     = {px, py};
                fill[i * 2].color        = fillColor;
                fill[i * 2 + 1].position = {px, (float)(chartY + chartH)};
                fill[i * 2 + 1].color    = sf::Color(fillColor.r, fillColor.g, fillColor.b, 0);
            }
            window.draw(fill);

            sf::VertexArray line(sf::PrimitiveType::LineStrip, n);
            for (int i = 0; i < n; i++)
            {
                float px = chartX + (float)i / (n - 1) * chartW;
                float py = chartY + chartH - (float)data[i] / maxValue * chartH;
                line[i].position = {px, py};
                line[i].color = color;
            }
            window.draw(line);
        };
        drawLine(healthyHistory,   sf::Color::White);
        drawLine(infectedHistory,  sf::Color::Red);
        drawLine(recoveredHistory, sf::Color::Green);
    }
};

class Circle
{
public:
    float x, y;
    float xSpeed, ySpeed;
    float R;
    sf::Color color;
    bool flag;
    int veaknesstick;
    int vessel = -1;
    bool isTraveling = false;
    float travelProgress = 0.0f;

    Circle() : x(0), y(0), xSpeed(0), ySpeed(0), R(10), color(sf::Color::White), flag(true), veaknesstick(0) {}
    
    Circle(float _x, float _y, float _dx, float _dy, float _R, sf::Color _color, int _vessel = -1)
        : x(_x), y(_y), xSpeed(_dx), ySpeed(_dy), R(_R), color(_color), flag(true), veaknesstick(0), vessel(_vessel) {}
    
    void draw(sf::RenderWindow& window) const
    {
        if (!flag) return;
        sf::CircleShape shape(R);
        shape.setFillColor(color);
        shape.setPosition({x - R, y - R});
        window.draw(shape);
    }
};

struct Vessel {
    float x, y;
    float baseR = 92.0f;
    int ballCount = 0;
};

float generateSpeed(){
    float speed = ((static_cast<float>(std::rand()) / RAND_MAX) * 0.6f) * (((std::rand() % 2 == 0) ? -1.0 : 1.0)) - 0.05;
    return speed;
}

void handleCollision(Circle& a, Circle& b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float distance = sqrtf(dx * dx + dy * dy);
    float minDistance = a.R + b.R;

    if (distance < minDistance && distance > 0.001f)
    {
        if (a.color == sf::Color::Red && b.color != sf::Color::Green && b.color != sf::Color::Red)
        {
            b.color = sf::Color::Red;
            b.veaknesstick = 1;
        }
        else if (b.color == sf::Color::Red && a.color != sf::Color::Green && a.color != sf::Color::Red)
        {
            a.color = sf::Color::Red;
            a.veaknesstick = 1;
        }

        float overlap = minDistance - distance;
        float angle = atan2f(dy, dx);
        float pushX = overlap * cosf(angle) / 2.0f;
        float pushY = overlap * sinf(angle) / 2.0f;
        
        a.x -= pushX; a.y -= pushY;
        b.x += pushX; b.y += pushY;
        
        float tempX = a.xSpeed; float tempY = a.ySpeed;
        a.xSpeed = b.xSpeed; a.ySpeed = b.ySpeed;
        b.xSpeed = tempX; b.ySpeed = tempY;
    }
}

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    const unsigned int fps = 60;
    float speedK = 2.5f;
    const float standartSpeedK = 1.5f;
    const int radius = 3;
    const unsigned int recoveryTime = 20 * fps;
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    const int RENDER_WINDOW_WIDTH = 800;
    const int RENDER_WINDOW_HEIGHT = 750;
    const int COUNT = 100;
    int frameCounter = 0;
    
    float transferTimer = 0.0f;
    int travelingBallIndex = -1;
    float suctionProgress = 0.0f;

    sf::RenderWindow window(sf::VideoMode({RENDER_WINDOW_WIDTH, RENDER_WINDOW_HEIGHT}), "Model 1 infection");
    Circle pt[COUNT];
    window.setFramerateLimit(fps);
    EpidemicChart chart(25, WINDOW_HEIGHT + 10, WINDOW_WIDTH - 50, 130, COUNT);

    Vessel vessel1 = {200, 300, 92.0f, 0};
    Vessel vessel2 = {540, 300, 92.0f, 0};

    for (int i = 0; i < COUNT; i++)
    {
        int vesselId = (i % 2 == 0) ? 0 : 1;
        Vessel& v = (vesselId == 0) ? vessel1 : vessel2;
        
        float angle = static_cast<float>(i) * 6.28f / (COUNT / 2);
        float dist = (std::rand() % 70) / 100.0f * (v.baseR - radius);
        
        float startX = v.x + cosf(angle) * dist;
        float startY = v.y + sinf(angle) * dist;
        
        pt[i] = Circle(startX, startY, generateSpeed(), generateSpeed(), radius, sf::Color::White, vesselId);
        if (i == COUNT-1) pt[i].color = sf::Color::Red;
    }

    while (window.isOpen())
    {
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::Equal || keyPressed->code == sf::Keyboard::Key::Add)
                    if (speedK < 50) speedK += 1;
                if (keyPressed->code == sf::Keyboard::Key::Hyphen || keyPressed->code == sf::Keyboard::Key::Subtract)
                    if (speedK > 0) speedK -= 1;
                if (keyPressed->code == sf::Keyboard::Key::Space)
                    speedK = standartSpeedK;
            }
        }

        // Движение шаров + очень лёгкое притяжение
        for (int i = 0; i < COUNT; i++)
        {
            if (pt[i].isTraveling) continue;

            // Обычное движение
            pt[i].x += pt[i].xSpeed * speedK;
            pt[i].y += pt[i].ySpeed * speedK;

            // === ОЧЕНЬ СЛАБОЕ ПРИТЯЖЕНИЕ К ВОРОНКЕ ===
            if (pt[i].vessel == 1)
            {
                float targetX = vessel2.x - vessel2.baseR + 28;
                float targetY = vessel2.y;
                
                float dx = targetX - pt[i].x;
                float dy = targetY - pt[i].y;
                
                // Очень слабое притяжение
                pt[i].x += dx * 0.0022f;
                pt[i].y += dy * 0.0022f;
            }

            // Коллизия со стенками
            if (pt[i].vessel == 0)
            {
                float dx = pt[i].x - vessel1.x;
                float dy = pt[i].y - vessel1.y;
                float dist = sqrtf(dx*dx + dy*dy);
                float maxDist = vessel1.baseR - pt[i].R - 3;

                if (dist > maxDist && dist > 0)
                {
                    float angle = atan2f(dy, dx);
                    pt[i].x = vessel1.x + cosf(angle) * maxDist;
                    pt[i].y = vessel1.y + sinf(angle) * maxDist;
                    pt[i].xSpeed = -pt[i].xSpeed * 0.82f;
                    pt[i].ySpeed = -pt[i].ySpeed * 0.82f;
                }
            }
            else if (pt[i].vessel == 1)
            {
                float dx = pt[i].x - vessel2.x;
                float dy = pt[i].y - vessel2.y;
                float dist = sqrtf(dx*dx + dy*dy);
                float maxDist = vessel2.baseR - pt[i].R - 3;

                if (dist > maxDist && dist > 0)
                {
                    float angle = atan2f(dy, dx);
                    pt[i].x = vessel2.x + cosf(angle) * maxDist;
                    pt[i].y = vessel2.y + sinf(angle) * maxDist;
                    pt[i].xSpeed = -pt[i].xSpeed * 0.82f;
                    pt[i].ySpeed = -pt[i].ySpeed * 0.82f;
                }
            }

            // Не даём шарам полностью останавливаться
            float speed = sqrtf(pt[i].xSpeed*pt[i].xSpeed + pt[i].ySpeed*pt[i].ySpeed);
            if (speed < 0.25f && speed > 0.001f)
            {
                pt[i].xSpeed *= 1.12f;
                pt[i].ySpeed *= 1.12f;
            }
        }
        for (int i = 0; i < COUNT; i++)
            for (int j = i + 1; j < COUNT; j++)
                if (!pt[i].isTraveling && !pt[j].isTraveling)
                    handleCollision(pt[i], pt[j]);

        // Засасывание + анимация
        transferTimer += 0.22f;
        if (transferTimer > 0.22f && travelingBallIndex == -1)
        {
            transferTimer = 0.0f;
            
            for (int i = 0; i < COUNT; i++)
            {
                if (pt[i].vessel != 1 || pt[i].isTraveling) continue;

                float dx = pt[i].x - vessel2.x;
                float dy = pt[i].y - vessel2.y;

                if (dx < -32 && std::abs(dy) < 13)
                {
                    travelingBallIndex = i;
                    pt[i].isTraveling = true;
                    pt[i].travelProgress = 0.0f;
                    suctionProgress = 0.0f;
                    pt[i].vessel = -1;
                    break;
                }
            }
        }

        if (travelingBallIndex != -1)
        {
            int i = travelingBallIndex;
            
            if (suctionProgress < 1.0f)
            {
                suctionProgress += 0.3f;
                float targetX = vessel2.x - vessel2.baseR + 22;
                float targetY = vessel2.y;
                
                pt[i].x += (targetX - pt[i].x) * 0.45f;
                pt[i].y += (targetY - pt[i].y) * 0.45f;
            }
            else
            {
                pt[i].travelProgress += 0.035f;

                float startX = vessel2.x - vessel2.baseR + 28;
                float endX   = vessel1.x + vessel1.baseR - 20;
                float tubeY  = vessel1.y;

                pt[i].x = startX + (endX - startX) * pt[i].travelProgress;
                pt[i].y = tubeY;

                if (pt[i].travelProgress >= 1.0f)
                {
                    pt[i].vessel = 0;
                    pt[i].isTraveling = false;
                    pt[i].travelProgress = 0.0f;
                    travelingBallIndex = -1;
                    suctionProgress = 0.0f;
                }
            }
        }

        vessel1.ballCount = 0;
        vessel2.ballCount = 0;
        for (int i = 0; i < COUNT; i++) {
            if (pt[i].vessel == 0) vessel1.ballCount++;
            if (pt[i].vessel == 1) vessel2.ballCount++;
        }
        
        vessel1.baseR = 75 + vessel1.ballCount * 0.85f;
        vessel2.baseR = 75 + vessel2.ballCount * 0.85f;

        window.clear();

        sf::RectangleShape tube(sf::Vector2f(vessel2.x - vessel1.x - 238, 14));
        tube.setPosition({vessel1.x + vessel1.baseR, vessel1.y - 7});
        tube.setFillColor(sf::Color(133 , 133 , 255));
        window.draw(tube);

        sf::CircleShape v1(vessel1.baseR);
        v1.setPosition({vessel1.x - vessel1.baseR, vessel1.y - vessel1.baseR});
        v1.setFillColor(sf::Color(30, 30, 70, 200));
        v1.setOutlineColor(sf::Color::White);
        v1.setOutlineThickness(6);
        window.draw(v1);

        sf::CircleShape v2(vessel2.baseR);
        v2.setPosition({vessel2.x - vessel2.baseR, vessel2.y - vessel2.baseR});
        v2.setFillColor(sf::Color(30, 30, 70, 200));
        v2.setOutlineColor(sf::Color::White);
        v2.setOutlineThickness(6);
        window.draw(v2);

        for (int i = 0; i < COUNT; i++)
        {
            pt[i].draw(window);

            if (pt[i].color == sf::Color::Red && !pt[i].isTraveling)
            {
                pt[i].veaknesstick++;
                if (pt[i].veaknesstick > recoveryTime)
                    pt[i].color = sf::Color::Green;
            }
        }

        if (frameCounter % 5 == 0)
        {
            int infected = 0, recovered = 0;
            for (int i = 0; i < COUNT; i++)
            {
                if (pt[i].color == sf::Color::Red) infected++;
                if (pt[i].color == sf::Color::Green) recovered++;
            }
            chart.update(infected, recovered);
        }
        frameCounter++;

        chart.draw(window);
        window.display();
    }
    return 0;
}
