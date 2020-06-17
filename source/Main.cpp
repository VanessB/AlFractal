#include <iostream>
#include <vector>
#include <cinttypes>
#include <gmpxx.h>
#include "GUI.hpp"

int main(int argc, char* argv[])
{
    std::shared_ptr<alfrac::Fractal> fractal = std::make_shared<alfrac::Fractal>();
    std::thread thread_fractal1(&alfrac::Fractal::loop, fractal.get());
    std::thread thread_fractal2(&alfrac::Fractal::loop, fractal.get());
    std::thread thread_fractal3(&alfrac::Fractal::loop, fractal.get());
    std::thread thread_fractal4(&alfrac::Fractal::loop, fractal.get());

    alfrac::GUI gui(fractal);
    gui.loop();

    fractal->terminate_loops();
    thread_fractal1.join();
    thread_fractal2.join();
    thread_fractal3.join();
    thread_fractal4.join();
    return 0;
}
