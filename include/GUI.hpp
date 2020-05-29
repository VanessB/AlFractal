#ifndef ALFRACTAL_GUI
#define ALFRACTAL_GUI

#include <cinttypes>
#include <memory>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include "Fractal.hpp"

namespace alfrac
{
    // Получение прямоугольника, в котором полностью лежит область видимости sf::View
    sf::FloatRect getViewBounds(const sf::View& view);

    ////////////////      Tile      ////////////////
    // Класс для отображения прямоугольной части фрактала.
    class Tile : public sf::Drawable, public sf::Transformable
    {
    public:
        Tile();
        Tile(std::future<Fractal::Data> future); // Конструктор, принимающий на вход future для получения результатов обсчёта региона фрактала.
        ~Tile();

        void check(); // Проверка окончания вычисления региона фрактала.
        void recolour(sf::Color gradient_start = sf::Color::Black, sf::Color gradient_end = sf::Color::Blue, sf::Color error = sf::Color::Red); // Построение градиента.

        // sf::Drawable
        virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    protected:
        std::future<Fractal::Data> _future; // Внутренний объект для ожидания результатов обсчёта региона фрактала.
        Fractal::Data data;                 // Данные о регионе фракткала.
        bool is_completed = false;          // Завершён ли обсчёт тайла.

        std::vector<sf::Uint8> pixels; // Коды пикселей в формате RGBA.
        sf::Texture texture;           // Текстура.
        sf::Sprite sprite;             // Спрайт.

    private:

    };

    ////////////////       GUI       ///////////////
    // Основной класс для графического интерфейса.
    class GUI
    {
    public:
        GUI(std::shared_ptr<Fractal> init_fractal);
        ~GUI();

        void loop(); // Цикл отрисовки.

    protected:
        std::shared_ptr<Fractal> assigned_fractal; // Прикреплённый вычислитель фрактала.
        sf::RenderWindow window; // Главное окно для отрисовки.
        sf::View view;           // Окно просмотра.

        // Тайлы.
        // Временная реализация хэширования вектора с целыми координатамами.
        struct _Vector2iHasher
        {
            std::size_t operator()(const sf::Vector2i& vector) const
            {
                return std::hash<int64_t>()(static_cast<int64_t>(vector.x) + (static_cast<int64_t>(vector.y) << 31 << 1));
            }
        };
        std::unordered_map<sf::Vector2i, std::shared_ptr<Tile>, _Vector2iHasher> tiles; // Сетка отрисованных тайлов.
        std::vector<std::shared_ptr<Tile>> onscreen_tiles; // Массив отображаемых тайлов.

        // Масштабирование графики.
        float scale_base = 2.0;  // Основание в степенном законе масштабирования.
        int64_t scale_power = 0; // Степень в степенном законе масштабирования.

        // Масштабирование фрактала.
        mpf_vector_2d fractal_scale_origin = mpf_vector_2d(0.0, 0.0); // Текущий центр, относительно которого производится масштабирование фрактала.
        int64_t fractal_scale_power = 0;      // Текущая степень в степенном законе масштабирования.
        mpf_class fractal_scale_factor = 1.0; // Текущий масштаб фрактала.

        // Точность вычисления фраткала.
        mp_bitcnt_t precision = 1024;
        int64_t iterations_limit = 64;
        mpf_class max_absolute = 4.0;

        void fetch_tiles(const sf::FloatRect& rectangle); // Обновление отображаемых тайлов, попавших в rectangle.
        void rescale_fractal(); // Изменение масштаба отрисовки фрактала.

    private:

    };
}

#endif
