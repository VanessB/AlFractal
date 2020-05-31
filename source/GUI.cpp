#include <iostream>
#include <cmath>
#include "GUI.hpp"

//#define DEBUG_OUTPUT_FUTURE_REQUEST

namespace alfrac
{
    // Параметры графики.
    const unsigned int FPS = 60;
    const unsigned int window_width  = 1280;
    const unsigned int window_height = 720;

    // Ширина и высота тайлов.
    const size_t tile_width  = 128;
    const size_t tile_height = 128;

    sf::FloatRect getViewBounds(const sf::View& view)
    {
        // TODO: учесть вращение.
        sf::FloatRect view_rect;
        view_rect.left = view.getCenter().x - 0.5f * view.getSize().x;
        view_rect.top  = view.getCenter().y - 0.5f * view.getSize().y;
        view_rect.width = view.getSize().x;
        view_rect.height = view.getSize().y;
        return view_rect;
    }

    ////////////////      Tile      ////////////////
    // Класс для отображения прямоугольной части фрактала.
    // PUBLIC:
    Tile::Tile()
    {
        sprite.rotate(-90.0f);
    }
    Tile::Tile(std::future<Fractal::Data> future) : Tile()
    {
        _future = std::move(future);
    }
    Tile::~Tile()
    {
        // ...
    }

    void Tile::check()
    {
        if (!is_completed)
        {
            if (_future.valid() && (_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready))
            {
                #ifdef DEBUG_OUTPUT_FUTURE_REQUEST
                std::cout << "Результат запроса готов к отрисовке." << std::endl;
                #endif

                data = std::move(_future.get());
                recolour();
                is_completed = true;
            }
        }
    }
    void Tile::recolour(sf::Color gradient_start, sf::Color gradient_end, sf::Color error)
    {
        size_t size = data.grid_x * data.grid_y;
        pixels = std::vector<sf::Uint8>(4 * size, 0);

        // Получение цветов через линейный градиент.
        for (size_t i = 0; i < size; ++i)
        {
            double relative_iteration = (double)(data.iterations[i] % data.iterations_limit) / (double)data.iterations_limit;
            //std::cout << data.iterations[i] << std::endl;
            pixels[i * 4]     = (sf::Uint8)((double)gradient_start.r * (1.0 - relative_iteration) + (double)gradient_end.r * relative_iteration);
            pixels[i * 4 + 1] = (sf::Uint8)((double)gradient_start.g * (1.0 - relative_iteration) + (double)gradient_end.g * relative_iteration);
            pixels[i * 4 + 2] = (sf::Uint8)((double)gradient_start.b * (1.0 - relative_iteration) + (double)gradient_end.b * relative_iteration);
            pixels[i * 4 + 3] = (sf::Uint8)((double)gradient_start.a * (1.0 - relative_iteration) + (double)gradient_end.a * relative_iteration);
        }
        pixels[0] = 255;
        pixels[1] = 255;
        pixels[2] = 255;

        // Создание и применение текстуры.
        texture.create(data.grid_x, data.grid_y);
        texture.update(&pixels[0]);
        sprite.setTexture(texture, true);
    }

    void Tile::draw(sf::RenderTarget &target, sf::RenderStates states) const
    {
        states.transform *= getTransform(); // Применение текущей трансформации.
        target.draw(sprite, states);        // Отрисовка привязанного спрайта.
    }

    // PROTECTED:


    // PRIVATE:




    ////////////////      GUI       ////////////////
    // Основной класс для графического интерфейса.
    // PUBLIC:
    GUI::GUI(std::shared_ptr<Fractal> init_fractal)
    {
        assigned_fractal = init_fractal;

        // Контекст OpentGl.
        sf::ContextSettings context;

        window.create(sf::VideoMode(window_width, window_height), "AlFractal", sf::Style::Default, context); // Создание окна.
        window.setFramerateLimit(FPS); // Ограничение на частоту обновления экрана.
        sf::Vector2f window_size = static_cast<sf::Vector2f>(window.getSize());

        view.setCenter(0.0f, 0.0f);
        view.setSize(window_size);
        view.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));

        ui_view.setCenter(window_size / 2.0f);
        ui_view.setSize(window_size);
        ui_view.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));

        window.setView(view);
    }
    GUI::~GUI()
    {
        // ...
    }

    // Цикл отрисовки.
    void GUI::loop()
    {
        // Масштаб фрактала.
        settings.fractal_scale_power = -10;
        rescale_fractal();

        // Нахождение первично отображаемых тайлов.
        fetch_tiles(getViewBounds(view));

        // Шрифт.
        sf::Font font;
        if (!font.loadFromFile("SourceCodePro-Light.otf"))
        {
            std::cerr << "Error!" << std::endl;
            throw;
        }

        // Текст для масштаба.
        sf::Text scale_text;
        scale_text.setFont(font);
        scale_text.setCharacterSize(16);
        scale_text.setFillColor(sf::Color::White);
        scale_text.setString("x"  + std::to_string(static_cast<int64_t>(pow(settings.scale_base, static_cast<double>(-settings.scale_power - settings.fractal_scale_power)))) +
                             "(x" + std::to_string(static_cast<int64_t>(pow(settings.scale_base, static_cast<double>(-settings.fractal_scale_power)))) + ")");

        // Текст для числа итераций.
        sf::Text iterations_text;
        iterations_text.setFont(font);
        iterations_text.setCharacterSize(16);
        iterations_text.setFillColor(sf::Color::White);
        iterations_text.setPosition(0.0f, 16.0f);
        iterations_text.setString(std::to_string(settings.iterations_limit) + " iterations");

        // Текст для числа бит.
        sf::Text bits_text;
        bits_text.setFont(font);
        bits_text.setCharacterSize(16);
        bits_text.setFillColor(sf::Color::White);
        bits_text.setPosition(0.0f, 32.0f);
        bits_text.setString(std::to_string(settings.precision) + " bits");

        sf::Vector2f mouse_position;
        sf::Event window_event;
        while (window.isOpen())
        {
            while (window.pollEvent(window_event))
            {
                switch (window_event.type)
                {
                    case sf::Event::Closed:
                    {
                        //assigned_fractal->in_loop.store(false);
                        return;
                        break;
                    }
                    case sf::Event::Resized:
                    {
                        sf::Vector2f window_size = static_cast<sf::Vector2f>(window.getSize());
                        view.setSize(window_size * static_cast<float>(pow(settings.scale_base, settings.scale_power)));
                        view.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));

                        ui_view.setSize(window_size);
                        ui_view.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));
                        ui_view.setCenter(window_size / 2.0f);

                        window.setView(view);

                        fetch_tiles(getViewBounds(view));
                        break;
                    }
                    case sf::Event::MouseButtonPressed:
                    {
                        switch (window_event.mouseButton.button)
                        {
                            case sf::Mouse::Left:
                            {
                                mouse_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                                break;
                            }
                            default: { break; }
                        }
                        break;
                    }
                    case sf::Event::MouseButtonReleased:
                    {
                        switch (window_event.mouseButton.button)
                        {
                            case sf::Mouse::Left:
                            {
                                fetch_tiles(getViewBounds(view));
                                break;
                            }
                            default: { break; }
                        }
                        break;
                    }
                    case sf::Event::MouseWheelMoved:
                    {
                        if (window_event.mouseWheel.delta > 0)
                        { --settings.scale_power; }
                        else
                        { ++settings.scale_power; }

                        view.setSize(static_cast<sf::Vector2f>(window.getSize()) * static_cast<float>(pow(settings.scale_base, settings.scale_power)));
                        window.setView(view);
                        mouse_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                        fetch_tiles(getViewBounds(view));

                        scale_text.setString("x"  + std::to_string(static_cast<int64_t>(pow(settings.scale_base, static_cast<double>(-settings.scale_power - settings.fractal_scale_power)))) +
                                             "(x" + std::to_string(static_cast<int64_t>(pow(settings.scale_base, static_cast<double>(-settings.fractal_scale_power)))) + ")");
                        break;
                    }
                    case sf::Event::KeyPressed:
                    {
                        switch (window_event.key.code)
                        {
                            case sf::Keyboard::R:
                            {
                                rescale_fractal();
                                scale_text.setString("x"  + std::to_string(static_cast<int64_t>(pow(settings.scale_base, static_cast<double>(-settings.scale_power - settings.fractal_scale_power)))) +
                                                     "(x" + std::to_string(static_cast<int64_t>(pow(settings.scale_base, static_cast<double>(-settings.fractal_scale_power)))) + ")");
                                break;
                            }
                            case sf::Keyboard::U:
                            {
                                settings.draw_ui = !settings.draw_ui;
                                break;
                            }
                            case sf::Keyboard::Dash:
                            {
                                if (sf::Keyboard::isKeyPressed(sf::Keyboard::I))
                                {
                                    settings.iterations_limit >>= 1;
                                    iterations_text.setString(std::to_string(settings.iterations_limit) + " iterations");
                                }
                                if (sf::Keyboard::isKeyPressed(sf::Keyboard::B))
                                {
                                    settings.precision >>= 1;
                                    bits_text.setString(std::to_string(settings.precision) + " bits");
                                }

                                break;
                            }
                            case sf::Keyboard::Equal:
                            {
                                if (sf::Keyboard::isKeyPressed(sf::Keyboard::I))
                                {
                                    settings.iterations_limit <<= 1;
                                    iterations_text.setString(std::to_string(settings.iterations_limit) + " iterations");
                                }
                                if (sf::Keyboard::isKeyPressed(sf::Keyboard::B))
                                {
                                    settings.precision <<= 1;
                                    bits_text.setString(std::to_string(settings.precision) + " bits");
                                }

                                break;
                            }

                            default: { break; }
                        }
                    }
                    default: { break; }
                }
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && window.hasFocus())
            {
                sf::Vector2f new_mouse_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                view.move(mouse_position - new_mouse_position);
                window.setView(view);
                //mouse_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                //mouse_position = new_mouse_position;
            }

            window.clear(sf::Color::Black);

            // Рисование тайлов.
            window.setView(view);
            for (size_t i = 0; i < onscreen_tiles.size(); ++i)
            {
                onscreen_tiles[i]->check();
                window.draw(*onscreen_tiles[i]);
            }

            // Рисование элементов интерфейса.
            if (settings.draw_ui)
            {
                window.setView(ui_view);
                window.draw(scale_text);
                window.draw(iterations_text);
                window.draw(bits_text);
            }

            window.setView(view); // Требуется для корректной обработки движения камеры мышкой.
            window.display();
        }
    }

    // PROTECTED:

    void GUI::fetch_tiles(const sf::FloatRect& rectangle)
    {
        // Отображение точек прямоугольника в тайлы сетки (учитывается максимально возможное покрытие).
        int X1 = static_cast<int>(floor(floor(rectangle.left) / static_cast<float>(tile_width)));
        int Y1 = static_cast<int>(floor(floor(rectangle.top)  / static_cast<float>(tile_height)));
        int X2 = static_cast<int>(ceil(ceil(rectangle.left + rectangle.width) / static_cast<float>(tile_width)));
        int Y2 = static_cast<int>(ceil(ceil(rectangle.top + rectangle.height) / static_cast<float>(tile_height)));

        size_t tiles_number = static_cast<size_t>((X2 - X1 + 1) * (Y2 - Y1 + 1));
        if (tiles_number > settings.max_tiles_number)
        { return; }

        onscreen_tiles.clear();
        onscreen_tiles.reserve(tiles_number);
        for (int y = Y1; y <= Y2; ++y)
        {
            for (int x = X1; x <= X2; ++x)
            {
                //std::cout << x << " : " << y << std::endl;
                auto iterator = tiles.find(sf::Vector2i(x, y));

                // Проверка, существует ли указанный тайл.
                if (iterator == tiles.end())
                {
                    // Возможно, запрос новый тайлов запрещён при текущем масштабе.
                    if (settings.request_on_downscale || settings.scale_power <= 0)
                    {
                        // Если тайл не существует, создаётся новый и добавляется в таблицу тайлов и в массив отображаемых тайлов.

                        // Составление запроса.
                        Fractal::Request request;
                        request.rectangle.bottom_left.x = static_cast<mpf_class>(  x      * static_cast<int>(tile_width))  * settings.fractal_scale_factor + settings.fractal_scale_origin.x;
                        request.rectangle.bottom_left.y = static_cast<mpf_class>( -y      * static_cast<int>(tile_height)) * settings.fractal_scale_factor + settings.fractal_scale_origin.y;
                        request.rectangle.top_right.x   = static_cast<mpf_class>(( x + 1) * static_cast<int>(tile_width))  * settings.fractal_scale_factor + settings.fractal_scale_origin.x;
                        request.rectangle.top_right.y   = static_cast<mpf_class>((-y + 1) * static_cast<int>(tile_height)) * settings.fractal_scale_factor + settings.fractal_scale_origin.y;

                        request.grid_x = tile_width;
                        request.grid_y = tile_height;

                        request.precision = settings.precision;
                        request.iterations_limit = settings.iterations_limit;
                        request.max_absolute = settings.max_absolute;
                        request.max_absolute.set_prec(settings.precision);

                        // Создание тайла, соответствующего запросу, и добавление его в таблицу и массив.
                        std::shared_ptr<Tile> tile = std::make_shared<Tile>(assigned_fractal->request_calc(request));
                        tiles.insert(std::pair<sf::Vector2i, std::shared_ptr<Tile>>(sf::Vector2i(x, y), tile));
                        onscreen_tiles.push_back(tile);
                        tile->setPosition(static_cast<float>(x * static_cast<int>(tile_width)), static_cast<float>(y * static_cast<int>(tile_height)));
                    }
                }
                else
                {
                    // Если тайл существует, он добавляется в массив отображаемых тайлов.
                    onscreen_tiles.push_back(iterator->second);
                }
            }
        }
    }

    void GUI::rescale_fractal()
    {
        // TODO: сделвть нормальное возведение в степень (через средства mpf).
        settings.fractal_scale_power += settings.scale_power;
        settings.scale_power = 0;
        mpf_class new_factor = mpf_class(pow(settings.scale_base, static_cast<double>(settings.fractal_scale_power)));
        new_factor.set_prec(settings.precision);

        mpf_vector_2d new_origin;
        new_origin.x =  static_cast<mpf_class>(view.getCenter().x) * settings.fractal_scale_factor + settings.fractal_scale_origin.x;
        new_origin.y = -static_cast<mpf_class>(view.getCenter().y) * settings.fractal_scale_factor + settings.fractal_scale_origin.y;
        new_origin.set_prec(settings.precision);

        settings.fractal_scale_origin = new_origin;
        settings.fractal_scale_factor = new_factor;

        view.setCenter(0.0f, 0.0f);
        view.setSize(static_cast<sf::Vector2f>(window.getSize()));
        view.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));
        window.setView(view);

        onscreen_tiles.clear();
        tiles.clear();
        fetch_tiles(getViewBounds(view));
    }

    // PRIVATE:

}
