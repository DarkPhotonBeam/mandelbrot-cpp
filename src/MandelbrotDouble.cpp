//
// Created by dan on 2/9/25.
//

#include "MandelbrotDouble.h"

#include <iostream>
#include <omp.h>
uint32_t HSVtoARGBd(double h, double s, double v) {
    double c = v * s;
    double x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
    double m = v - c;

    double r, g, b;
    if (h < 60)      { r = c, g = x, b = 0; }
    else if (h < 120) { r = x, g = c, b = 0; }
    else if (h < 180) { r = 0, g = c, b = x; }
    else if (h < 240) { r = 0, g = x, b = c; }
    else if (h < 300) { r = x, g = 0, b = c; }
    else             { r = c, g = 0, b = x; }

    uint8_t R = static_cast<uint8_t>((r + m) * 255);
    uint8_t G = static_cast<uint8_t>((g + m) * 255);
    uint8_t B = static_cast<uint8_t>((b + m) * 255);

    return (255 << 24) | (R << 16) | (G << 8) | B;  // ARGB format
}

uint32_t MandelbrotDouble::coloring1(const double sm_iterations) const {
    const double power_factor = pow(sm_iterations / max_iterations, 0.7); // Adjust exponent (0.5 - 0.8)
    const double hue = 360.0 * power_factor;
    return HSVtoARGBd(hue, 1.0, 1.0);
}

Complex MandelbrotDouble::toComplex(const Pixeld &pixel) const {
    const long double ratio_x = static_cast<double>(pixel.x) / screen_width;
    const long double ratio_y = static_cast<double>(pixel.y) / screen_height;
    Complex out{ratio_x * vp_diff.real(), ratio_y * vp_diff.imag()};
    out += vp_bl;
    return out;
}

Pixeld MandelbrotDouble::toScreen(Complex z) const {
    z -= vp_bl;
    const long double x = z.real() / vp_diff.real();
    const long double y = z.imag() / vp_diff.imag();
    const Pixeld out{static_cast<int>(x * screen_width), static_cast<int>(y * screen_height)};
    return out;
}

void MandelbrotDouble::compute() {
    precompute();
    // Lock the texture to modify pixels
        void* pixels;
        int pitch;
        SDL_LockTexture(texture, nullptr, &pixels, &pitch);

        auto* pixelArray = static_cast<uint32_t*>(pixels);

        constexpr double twopi = M_PI * 2;

        const uint32_t plasma_palette[] = {
                0xFF0D0887, 0xFF350498, 0xFF5402A3, 0xFF7000A8, 0xFF8B0DA6, 0xFFA31F9B, 0xFFB9378B, 0xFFCA4D76,
                0xFFD9625D, 0xFFE57643, 0xFFEE8A27, 0xFFF59C0E, 0xFFFCB312, 0xFFFFC527, 0xFFFFFF40
            };
        const int plasma_palette_size = sizeof(plasma_palette) / sizeof(plasma_palette[0]);

#pragma omp parallel for schedule(dynamic)
        for (size_t y = 0; y < screen_height; y++) {
                Complex z{0, 0};
                for (size_t x = 0; x < screen_width; x++) {
                        const size_t index = y * (pitch / sizeof(uint32_t)) + x;
                        Pixeld p{static_cast<int>(x), static_cast<int>(y)};
                        Complex c = toComplex(p);
                        z.real(0);
                        z.imag(0);
                        const double sm_iterations = smooth_iteration(z, c);
                        pixelArray[index] = coloring1(sm_iterations);
                }
        }
        std::cout << "DONE" << std::endl;

        SDL_UnlockTexture(texture);
}

double MandelbrotDouble::smooth_iteration(Complex &z, const Complex &c) const {
    int i = 0;
    for (; i < max_iterations; ++i) {
        z *= z;
        z += c;
        if (std::abs(z) > 2.0) break;
    }
    return static_cast<double>(static_cast<long double>(i + 1) + std::log(std::log(std::abs(z))));
}

void MandelbrotDouble::initialize(SDL_Renderer *renderer, const int width, const int height) {
    this->renderer = renderer;
    screen_width = width;
    screen_height = height;
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING, width, height);
    initialized = true;
}

SDL_Texture * MandelbrotDouble::get_texture() const {
    return texture;
}

void MandelbrotDouble::shrink_offset() {
    constexpr double factor = 0.1;
    offset *= factor;
}

void MandelbrotDouble::set_viewport(const Pixeld &center) {
    const Complex center_z = toComplex(center);
    vp_bl = center_z - offset;
    vp_tr = center_z + offset;

    std::cout << "Center: " << center_z << std::endl;

    const Complex diff = vp_tr - vp_bl;
    const long double width = diff.real();
    const long double zoom = 3.0 / width;
    max_iterations = static_cast<int>(100 + 50 * std::log2(zoom));

    std::cout << "Zoom: " << zoom << std::endl;

    compute();
}

void MandelbrotDouble::print_info() const {
    std::cout << "Offset: " << offset << std::endl;
    std::cout << "Bottom Left: " << vp_bl << std::endl;
    std::cout << "Top Right: " << vp_tr << std::endl;
    std::cout << "Max. Iterations: " << max_iterations << std::endl;
}
