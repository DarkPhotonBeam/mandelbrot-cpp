//
// Created by dan on 2/9/25.
//

#ifndef MANDELBROTDOUBLE_H
#define MANDELBROTDOUBLE_H
#include <SDL3/SDL_render.h>
#include <complex>

struct Pixeld {
    int x{0};
    int y{0};
    Pixeld(const int x, const int y) : x(x), y(y) {}
};

typedef std::complex<long double> Complex;

class MandelbrotDouble {
private:
    SDL_Renderer *renderer{nullptr};
    SDL_Texture *texture{nullptr};
    bool initialized{false};
    int screen_width{1000};
    int screen_height{1000};
    int max_iterations{20};
    Complex vp_bl{2, 2};
    Complex vp_tr{-2, -2};

    Complex vp_diff{};

    Complex offset{2, 2};

    [[nodiscard]] uint32_t coloring1(double sm_iterations) const;

    [[nodiscard]] Complex toComplex(const Pixeld &pixel) const;
    [[nodiscard]] Pixeld toScreen(Complex z) const;
    void precompute();
    void compute();
    double smooth_iteration(Complex &z, const Complex &c) const;
public:
    void initialize(SDL_Renderer *, int, int);
    [[nodiscard]] SDL_Texture *get_texture() const;
    void shrink_offset();
    void set_viewport(const Pixeld &center);
    void print_info() const;
};

inline void MandelbrotDouble::precompute() {
    vp_diff = vp_tr - vp_bl;
}


#endif //MANDELBROTDOUBLE_H
