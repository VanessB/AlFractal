#ifndef ALFRACTAL_FRACTAL
#define ALFRACTAL_FRACTAL

#include <cinttypes>
#include <queue>
#include <memory>
#include <shared_mutex>
#include <future>

#include <gmpxx.h>
#include "Algebra.hpp"

namespace alfrac
{
    ////////////////      CONST      ///////////////
    const mp_bitcnt_t precision_bits = 128;



    ////////////////     STRUCTS     ///////////////
    // Вектор с координатами типа mpf_class.
    struct mpf_vector_2d
    {
        mpf_class x;
        mpf_class y;

        mpf_vector_2d();
        explicit mpf_vector_2d(mpf_class init_x, mpf_class init_y);

        // Точность.
        mp_bitcnt_t get_min_prec(); // Получение наименьшего количества бит точности среди координат.
        mp_bitcnt_t get_max_prec(); // Получение наибольшего количества бит точности среди координат.
        mp_bitcnt_t get_prec();     // Эквивалентно get_min_prec().
        void set_prec(mp_bitcnt_t prec); // Установка нового числа битов точности для всех координат.
    };

    // Прямоугольник с координатами углов типа mpf_class.
    struct mpf_rectangle
    {
        // Углы прямоугольника.
        mpf_vector_2d bottom_left;
        mpf_vector_2d top_right;

        mpf_rectangle();
        explicit mpf_rectangle(mpf_vector_2d init_bl, mpf_vector_2d init_tr);
        explicit mpf_rectangle(mpf_class init_bl_x, mpf_class init_bl_y, mpf_class init_tr_x, mpf_class init_tr_y);

        // Точность.
        mp_bitcnt_t get_min_prec(); // Получение наименьшего количества бит точности среди координат.
        mp_bitcnt_t get_max_prec(); // Получение наибольшего количества бит точности среди координат.
        mp_bitcnt_t get_prec();     // Эквивалентно get_min_prec().
        void set_prec(mp_bitcnt_t prec); // Установка нового числа битов точности для всех координат.
    };



    ////////////////     Algebra     ///////////////
    // Таблица произведений для алгебры.
    const mpf_class product_tensor_mpf[2][2][2]
    {
        {
            // x_1
            // 1                            2
            { mpf_class(1, precision_bits), mpf_class(0,  precision_bits) }, // 1
            { mpf_class(0, precision_bits), mpf_class(-1, precision_bits) }  // 2
        },
        {
            // x_2
            // 1                            2
            { mpf_class(0, precision_bits), mpf_class(1, precision_bits) }, // 1
            { mpf_class(1, precision_bits), mpf_class(0, precision_bits) }  // 2
        }
    };

    // Тип элемента алгебры.
    using alg_mpf = algebra::Algebra<mpf_class, 2, product_tensor_mpf>;



    ////////////////     Fractal     ///////////////
    // Класс для проведения расчётов, связанных с вычислением структуры фрактала.
    class Fractal
    {
    public:
        // Структура для хранения и передачи данных о запросе на обсчёт региона алгебраической плоскости.
        struct Request
        {
            // Прямоугольник, выделяющий регион.
            mpf_rectangle rectangle;

            // Параметры сетки.
            size_t grid_x;
            size_t grid_y;

            mp_bitcnt_t precision;    // Точность арифметики чисел с плавающей точкой.
            int64_t iterations_limit; // Максимальное число итераций на одну точку сетки.
            mpf_class max_absolute;   // Максимальное значение модуля числа.
        };

        // Структура для хранения и передачи данных о результатах обсчёта региона.
        struct Data
        {
            // Параметры сетки.
            size_t grid_x;
            size_t grid_y;

            std::vector<int64_t> iterations; // Таблица числа итераций для каждой точки.
            int64_t iterations_limit;        // Максимальное число итераций на одну точку сетки.

            Data();
            explicit Data(const Fractal::Request& request); // Автоматическая настройка метаданных по данным о запросе.
        };

        Fractal();
        Fractal(const Fractal& fractal) = delete; // Запрет конструктора-копирования.
        ~Fractal();

        // Обсчёт области алгебраической плоскости.
        Fractal::Data calculate(const Fractal::Request& request);                 // Расчёт в текущем потоке.
        std::future<Fractal::Data> request_calc(const Fractal::Request& request); // Запрос на проведение расчётов в отдельном потоке.

        void loop();            // Цикл для рассчётов.
        void terminate_loops(); // Завершить все циклы рассчётов.

        Fractal& operator=(const Fractal& right) = delete; // Запрет присвоения-копирования.

    protected:
        // Запросы.
        std::queue< std::pair< Fractal::Request, std::promise< Fractal::Data> > > requests_queue; // Очередь запросов на обсчёт.
        std::shared_mutex mutex_requests_queue;   // shared_mutex для контроля доступа к requests_queue.

        // Механизмы синфронизации.
        std::atomic<bool> in_loop = true;           // Переменная для контроля циклов расчётов.
        std::condition_variable condition_requests; // Условная переменная для реализации функции ожидания.
        std::mutex mutex_condition_requests;        // mutex для реализации функции ожидания.

        Fractal::Data _calculate(const Fractal::Request& request); // Внутренняя версия расчёта.

    private:

    };
}

#endif
