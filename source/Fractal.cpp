#include "Fractal.hpp"
#include <thread>
#include <chrono>
#include <iostream>

//#define DEBUG_OUTPUT_REQUESTS
//#define DEBUG_OUTPUT_LOOP
//#define DEBUG_OUTPUT_CALCULATE

using namespace std::chrono_literals;

namespace alfrac
{
    ////////////////     STRUCTS     ///////////////
    // mpf_vector_2d
    mpf_vector_2d::mpf_vector_2d() { }
    mpf_vector_2d::mpf_vector_2d(mpf_class init_x, mpf_class init_y)
    {
        x = init_x;
        y = init_y;
    }
    // Получение наименьшего количества бит точности среди координат.
    mp_bitcnt_t mpf_vector_2d::get_min_prec()
    {
        return std::min(x.get_prec(), y.get_prec());
    }
    // Получение наибольшего количества бит точности среди координат.
    mp_bitcnt_t mpf_vector_2d::get_max_prec()
    {
        return std::max(x.get_prec(), y.get_prec());
    }
    // Эквивалентно get_min_prec().
    mp_bitcnt_t mpf_vector_2d::get_prec()
    {
        return get_min_prec();
    }
    // Установка нового числа битов точности для всех координат.
    void mpf_vector_2d::set_prec(mp_bitcnt_t prec)
    {
        x.set_prec(prec);
        y.set_prec(prec);
    }

    // Прямоугольник с координатами углов типа mpf_class.
    // mpf_rectangle
    mpf_rectangle::mpf_rectangle() { }
    mpf_rectangle::mpf_rectangle(mpf_vector_2d init_bl, mpf_vector_2d init_tr) : bottom_left(init_bl), top_right(init_tr) { }
    mpf_rectangle::mpf_rectangle(mpf_class init_bl_x, mpf_class init_bl_y, mpf_class init_tr_x, mpf_class init_tr_y)
        : bottom_left(init_bl_x, init_bl_y), top_right(init_tr_x, init_tr_y) { }
    mp_bitcnt_t mpf_rectangle::get_min_prec()
    {
        return std::min(bottom_left.get_min_prec(), top_right.get_min_prec());
    }
    // Получение наибольшего количества бит точности среди координат.
    mp_bitcnt_t mpf_rectangle::get_max_prec()
    {
        return std::max(bottom_left.get_max_prec(), top_right.get_max_prec());
    }
    // Эквивалентно get_min_prec().
    mp_bitcnt_t mpf_rectangle::get_prec()
    {
        return get_min_prec();
    }
    // Установка нового числа битов точности для всех координат.
    void mpf_rectangle::set_prec(mp_bitcnt_t prec)
    {
        bottom_left.set_prec(prec);
        top_right.set_prec(prec);
    }



    ////////////////     Fractal     ///////////////
    // Класс для проведения рассчётов, связанных с вычислением структуры фрактала.
    // PUBLIC:
    Fractal::Data::Data() { }
    Fractal::Data::Data(const Fractal::Request& request) :
        grid_x(request.grid_x), grid_y(request.grid_y), iterations(request.grid_x * request.grid_y, 0), iterations_limit(request.iterations_limit) { }

    Fractal::Fractal() : in_loop(true) { }
    Fractal::~Fractal() { }

    std::future<Fractal::Data> Fractal::request_calc(const Fractal::Request& request)
    {
        #ifdef DEBUG_OUTPUT_REQUESTS
        std::cout << "Новый запрос." << std::endl;
        #endif

        // Блокировка очереди запросов (запись).
        std::unique_lock<std::shared_mutex> lock_requests_queue(mutex_requests_queue);

        std::promise<Fractal::Data> promise;
        std::future<Fractal::Data> future = promise.get_future();
        requests_queue.push(std::pair< Fractal::Request, std::promise<Fractal::Data> >(request, std::move(promise)));

        #ifdef DEBUG_OUTPUT_REQUESTS
        std::cout << "Запрос успешно добавлен в очередь." << std::endl;
        #endif

        // Оповещение о добавлении в очередь.
        condition_requests.notify_one();

        return future;
    }

    Fractal::Data Fractal::calculate(const Fractal::Request& request)
    { return _calculate(std::move(request)); }

    void Fractal::loop()
    {
        while(in_loop.load())
        {
            // Проверка очереди запросов.
            #ifdef DEBUG_OUTPUT_LOOP
            std::cout << "Проверка очереди запросов." << std::endl;
            #endif

            std::pair< Fractal::Request, std::promise<Fractal::Data> > request__promise;
            bool is_empty = true;

            // Проверка на пустоту.
            {
                std::unique_lock<std::shared_mutex> lock_requests_queue(mutex_requests_queue);
                is_empty = requests_queue.empty();
                if (!is_empty)
                {
                    #ifdef DEBUG_OUTPUT_LOOP
                    std::cout << "Очередь не пуста." << std::endl;
                    #endif

                    // Если очередь не пустая, извлекаем последний запрос.
                    request__promise = std::move(requests_queue.front());
                    requests_queue.pop();

                    #ifdef DEBUG_OUTPUT_LOOP
                    std::cout << "Из очереди извлечен очередной запрос." << std::endl;
                    #endif
                }
            }

            // Если очередь была не пуста, производится вычисление данных.
            if (!is_empty)
            {
                #ifdef DEBUG_OUTPUT_LOOP
                std::cout << "Начата обработка запроса." << std::endl;
                #endif

                request__promise.second.set_value(std::move(_calculate(request__promise.first)));

                #ifdef DEBUG_OUTPUT_LOOP
                std::cout << "Запрос обработан успешно." << std::endl;
                #endif
            }
            // Простой, когда очередь пуста.
            else
            {
                // Если очередь пустая, а обход есть, производится переход в режим ожидания.
                std::unique_lock<std::mutex> wait_lock(mutex_condition_requests);
                condition_requests.wait(wait_lock);
            }
            //std::this_thread::sleep_for(100ms);
        }

        #ifdef DEBUG_OUTPUT_LOOP
        std::cout << "Завершение цикла обработки." << std::endl;
        #endif
        return;
    }

    void Fractal::terminate_loops()
    {
        in_loop.store(false);
        condition_requests.notify_all();
        return;
    }

    // PROTECTED:
    Fractal::Data Fractal::_calculate(const Fractal::Request& request)
    {
        Fractal::Data result(request);

        mpf_rectangle _rectangle = request.rectangle;
        _rectangle.set_prec(request.precision);

        alg_mpf Constant({ mpf_class("-2.6", request.precision), mpf_class("-2.9", request.precision) });
        alg_mpf Var({ mpf_class(0.0, request.precision), mpf_class(0.0, request.precision) });

        mpf_class SqrMaxAbsolute = request.max_absolute * request.max_absolute;

        for (size_t x = 0; x < request.grid_x; ++x)
        {
            for (size_t y = 0; y < request.grid_y; ++y)
            {
                Var.components[0] = mpf_class(0.0, request.precision);
                Var.components[1] = mpf_class(0.0, request.precision);
                Constant.components[0] = request.rectangle.bottom_left.x + (request.rectangle.top_right.x - request.rectangle.bottom_left.x)
                * mpf_class((double)x / (double)request.grid_x, request.precision);
                Constant.components[1] = request.rectangle.bottom_left.y + (request.rectangle.top_right.y - request.rectangle.bottom_left.y)
                * mpf_class((double)y / (double)request.grid_y, request.precision);

                //std::cout << Var.components[0] << " : " << Var.components[1] << std::endl;

                int64_t Step = 0;
                for (; Step < request.iterations_limit; ++Step)
                {
                    Var = Var * Var + Constant;

                    mpf_class SqrAbsolute = Var.components[0] * Var.components[0] + Var.components[1] * Var.components[1];
                    //mpf_class SqrAbsolute = Var.components[0] * Var.components[1];
                    //std::cout << SqrMaxAbsolute.get_prec() << std::endl;

                    #ifdef DEBUG_STEPS_OUTPUT
                    std::cout << Step << ": " << X.components[0] << " " << X.components[1] << "   " << SqrAbsolute << '\n';
                    #endif

                    if (cmp(SqrAbsolute, SqrMaxAbsolute) > 0) { break; }
                }
                result.iterations[x * request.grid_x + y] = Step;
                //mpf_class SqrAbsolute = Var.components[0] * Var.components[0] - Var.components[1] * Var.components[1];
                //mpf_class SqrAbsolute = Var.components[0] * Var.components[1];
                //if (SqrAbsolute > SqrMaxAbsolute) { result.iterations[x * request.grid_x + y] = 0; }
                //else { result.iterations[x * request.grid_x + y] = request.iterations_limit; }
            }
        }

        return result;
    }

    // PRIVATE:
}
