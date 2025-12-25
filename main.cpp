#include<SFML/Graphics.hpp>
#include<iostream>
#include<cmath>
#include<string>
#include<vector>
#include<thread>
#include<atomic>

std::atomic<char> sharedType = ' ';
std::atomic<float>sharedA = 0, sharedB = 0, sharedC = 0;

void inputThreadFunc()
{
    while(true)
    {
        char t;
        std::cout<<"\n[Input Thread]Enter type('s': sin, 'c': cos, 't': tan, 'q': quadratic, 'r': reset, 'x': exit): ";
        std::cin>>t;

        if(t == 's' || t == 'c' || t =='t')
        {
            float amp, freq;
            std::cout<<"Enter Amplitude (A) and Frequency (B) :";
            std::cin>>amp>>freq;
            sharedA = amp;
            sharedB = freq;
        }
        else if(t == 'q')
        {
            float ta, tb, tc;
            std::cout<<"Enter a, b, c: (for ax^2 +bx + c): ";
            std::cin>>ta>>tb>>tc;
            sharedA = ta; sharedB = tb; sharedC = tc;
        }

        sharedType = t;

        if(t == 'x') break;
    }
}

class GraphingCalculator
{
    private:
        float width, height;
        float originX, originY;
        float scale;
        sf::Font font;

    public:
        GraphingCalculator(float w, float h) : width(w), height(h)
            {
                resetView();
                if(!font.openFromFile("arial.ttf"))
                {
                    std::cout<<"Error loading font!"<<std::endl;
                }
            }

        void resetView()
        {
            originX = width / 2.0f;
            originY = height / 2.0f;
            scale = 40.0f;
        }

        void zoom(float delta)
        {
            if(delta > 0) scale *= 1.1f;
            else scale /= 1.1f;

            if(scale < 1.0f) scale = 1.0f;
            if(scale > 5000.0f) scale = 5000.0f;
        }

        void move(float dx, float dy)
        {
            originX += dx;
            originY += dy;
        }

        void drawTick(sf::RenderWindow& window, float val, bool isXAxis)
        {
            sf::Vector2f mathPos = isXAxis ? sf::Vector2f(val, 0) : sf::Vector2f(0, val);
            sf::Vector2f screenPos = mapToScreen(mathPos.x, mathPos.y);

            sf::RectangleShape tick(isXAxis ? sf::Vector2f(1, 10) : sf::Vector2f(10, 1));
            tick.setOrigin(isXAxis ? sf::Vector2f(0.5f, 5.0f) : sf::Vector2f(5.0f, 0.5f));
            tick.setPosition(screenPos);
            tick.setFillColor(sf::Color::White);
            window.draw(tick);

            if(scale > 15.0f)
            {
                sf::Text text(font, std::to_string((int)val));
                text.setCharacterSize(10);
                text.setFillColor(sf::Color::White);
                text.setPosition({screenPos.x + 3, screenPos.y + 3});
                window.draw(text);
            }
        }

        void drawAxes(sf::RenderWindow& window)
            {
                sf::VertexArray axes(sf::PrimitiveType::Lines, 4);

                axes[0].position = {0, originY};
                axes[1].position = {width, originY};
                axes[2].position = {originX, 0};
                axes[3].position = {originX, height};

                for(int i = 0; i < 4; i++) axes[i].color = sf::Color::White;
                window.draw(axes);


                for(float x = 0;mapToScreen(x, 0).x <= width; x += 1.0f) drawTick(window, x, true);
                for(float x = -1.0f;mapToScreen(x, 0).x >= 0; x -= 1.0f) drawTick(window,  x, true);
                for(float y = 1.0f;mapToScreen(0, y).y >= 0; y += 1.0f) drawTick(window, y, false);
                for(float y = -1.0f;mapToScreen(0, y).y <= height; y -= 1.0f) drawTick(window, y, false);

            }

            sf::Vector2f mapToScreen(float x, float y)
            {
                return sf::Vector2f(originX + (x * scale), originY - (y * scale));
             }

        void drawSine(sf::RenderWindow& window, float A, float B)
        {
            sf::VertexArray graph(sf::PrimitiveType::LineStrip);
            for(float pixelX = 0; pixelX <= width; pixelX += 1.0f)
            {
                float x = (pixelX - originX) / scale;
                float y = A * std::sin(B * x);
                graph.append(sf::Vertex(mapToScreen(x, y),  sf::Color::Yellow));
            }
            window.draw(graph);
        }

        void drawCosine(sf::RenderWindow& window, float A, float B)
        {
            sf::VertexArray graph(sf::PrimitiveType::LineStrip);
            for(float pixelX = 0;pixelX <= width;pixelX += 1.0f)
            {
                float x = (pixelX - originX) / scale;
                float y = A * std::cos(B * x);
                graph.append(sf::Vertex(mapToScreen(x, y), sf::Color::Green));
            }
            window.draw(graph);
        }

        void drawTangent(sf::RenderWindow& window, float A, float B)
        {
            sf::VertexArray graph(sf::PrimitiveType::LineStrip);
            for(float pixelX = 0; pixelX <= width; pixelX += 1.0f)
            {
                float x = (pixelX - originX) / scale;
                float y = A* std::tan(B * x);

                if(std::abs(y) >15.0f)
                {
                    window.draw(graph);
                    graph.clear();
                }
                else
                {
                    graph.append(sf::Vertex(mapToScreen(x, y), sf::Color::Red));
                }
                window.draw(graph);
            }
        }

        void drawQuadratic(sf::RenderWindow& window, float a, float b, float c, sf::Color color)
        {
            sf::VertexArray graph(sf::PrimitiveType::LineStrip);
            for(float pixelX = 0; pixelX <= width; pixelX += 1.0f)
            {
                float x = (pixelX - originX) / scale;
                float y = a * std::pow(x, 2) + b * x +c;
                sf::Vector2f screenPos = mapToScreen(x, y);
                graph.append(sf::Vertex(screenPos, color));
            }
            window.draw(graph);
        }
};

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Graphing Calculator");
    GraphingCalculator calc(800, 600);

    std::thread t(inputThreadFunc);
    t.detach();

    sf::Vector2i lastMousePos = sf::Mouse::getPosition(window);
    bool isDragging = false;

    while(window.isOpen())
    {
        while(const std::optional event = window.pollEvent())
        {
            if(event->is<sf::Event::Closed>())
                window.close();

            if(const auto* mouseWheel = event->getIf<sf::Event::MouseWheelScrolled>())
            {
              calc.zoom(mouseWheel->delta);
            }

            if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if(keyPressed->code == sf::Keyboard::Key::R)
                {
                    calc.resetView();
                }
            }

            if(const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if(mouseButton->button == sf::Mouse::Button::Left)
                {
                    isDragging = true;
                    lastMousePos = sf::Mouse::getPosition(window);
                }
            }

            if(const auto* mouseButton = event->getIf<sf::Event::MouseButtonReleased>())
            {
                    if(mouseButton->button == sf::Mouse::Button::Left)
                    {
                         isDragging = false;
                    }
            }
        }

        sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);

        if(isDragging)
        {
            sf::Vector2f delta = sf::Vector2f(currentMousePos - lastMousePos);

            if(delta.x != 0 || delta.y != 0)
            {
                calc.move(delta.x, delta.y);
            }
        }

        lastMousePos = currentMousePos;

        char currentType = sharedType;
        float a = sharedA, b = sharedB, c = sharedC;

        if(currentType == 'x')window.close();
        if(currentType == 'r')
        {
            sharedType = ' ';
            currentType =' ';
            system("cls");
        }

        window.clear(sf::Color::Black);
        calc.drawAxes(window);

        if(currentType == 's')
        {
            calc.drawSine(window, a,b);
        }
        else if(currentType == 'c')
        {
            calc.drawCosine(window, a, b);
        }
        else if(currentType == 't')
        {
            calc.drawTangent(window, a, b);
        }
        else if(currentType == 'q')
        {
            calc.drawQuadratic(window, a, b, c, sf::Color::Yellow);
        }

        window.display();
        sf::sleep(sf::milliseconds(1));

    }
    return 0;
}
