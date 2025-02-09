//
// Created by dan on 2/8/25.
//

#include "Mandelbrot.h"
#include <iostream>
#include <cmath>
#include <omp.h>
#define ARGB(r, g, b, a) ((0xff << 24) | (r << 16) | (g << 8) | b)

uint32_t HSVtoARGB(double h, double s, double v) {
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


// PRIVATE METHODS START
//
void Mandelbrot::init() {
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
        //compute();
}

void Mandelbrot::precompute_viewport() {
        thres = MPC{4, 0};
        vp_bl_real = vp_bl.real();
        vp_bl_imag = vp_bl.imag();
        vp_width = MPC{static_cast<double>(screen_width)};
        vp_height = MPC{static_cast<double>(screen_height)};
        vp_diff = vp_tr - vp_bl;
        vp_diff_real = vp_diff.real();
        vp_diff_imag = vp_diff.imag();
        const MPC tr_real = vp_tr.real();
        const MPC tr_imag = vp_tr.imag();
        const MPC tmp_real = tr_real / vp_diff_real;
        const MPC tmp_imag = tr_imag / vp_diff_imag;
        vp_corr_real = tmp_real * vp_width;
        vp_corr_imag = tmp_imag * vp_height;
        vp_fact_real = vp_width / vp_diff_real;
        vp_fact_imag = vp_height / vp_diff_imag;
}

void Mandelbrot::compute_texture() {
        // Lock the texture to modify pixels
        void* pixels;
        int pitch;
        SDL_LockTexture(texture, nullptr, &pixels, &pitch);
        //std::cout << "|\n";

        auto* pixelArray = static_cast<uint32_t*>(pixels);
        //size_t pixelCount = (pitch / sizeof(uint32_t)) * WINDOW_HEIGHT;
        constexpr double twopi = M_PI * 2;

        const uint32_t plasma_palette[] = {
                0xFF0D0887, 0xFF350498, 0xFF5402A3, 0xFF7000A8, 0xFF8B0DA6, 0xFFA31F9B, 0xFFB9378B, 0xFFCA4D76,
                0xFFD9625D, 0xFFE57643, 0xFFEE8A27, 0xFFF59C0E, 0xFFFCB312, 0xFFFFC527, 0xFFFFFF40
            };
        const int plasma_palette_size = sizeof(plasma_palette) / sizeof(plasma_palette[0]);





        // Fill pixels with a gradient effect
#pragma omp parallel for schedule(dynamic)
        for (size_t y = 0; y < screen_height; y++) {
                MPC z{0, 0};
                for (size_t x = 0; x < screen_width; x++) {
                        size_t index = y * (pitch / sizeof(uint32_t)) + x;
                        Pixel p{static_cast<int>(x), static_cast<int>(y)};
                        MPC c = toComplex(p);
                        z.set(0, 0);
                        //std::cout << z;
                        const double sm_iterations = smooth_iteration(z, c);
                        //double scaled_iter = log(sm_iterations) / log(max_iterations) * 360.0;
                        //double brightness = pow(sm_iterations / max_iterations, 0.5);  // Adjust gamma
                        //double hue = 360.0 * brightness;
                        //pixelArray[index] = HSVtoARGB(scaled_iter, 1.0, 1.0);
                        //pixelArray[index] = HSVtoARGB(hue, 1.0, brightness);

                        pixelArray[index] = coloring1(sm_iterations);

                        //int palette_index = (sm_iterations / max_iterations) * (plasma_palette_size - 1);
                        //palette_index = std::min(palette_index, plasma_palette_size - 1);
                        //pixelArray[index] = plasma_palette[palette_index];

                }
                //if (y % 10 == 0) std::cout << ".";
        }
        std::cout << "------------------------------------------------\nDONE!\n";

        SDL_UnlockTexture(texture);
}

uint32_t Mandelbrot::coloring1(const double sm_iterations) const {
        const double power_factor = pow(sm_iterations / max_iterations, 0.7); // Adjust exponent (0.5 - 0.8)
        const double hue = 360.0 * power_factor;
        return HSVtoARGB(hue, 1.0, 1.0);
}

void Mandelbrot::compute_texture2() {
        void* pixels;
        int pitch;
        SDL_LockTexture(texture, nullptr, &pixels, &pitch);
        auto* pixelArray = static_cast<uint32_t*>(pixels);

        Pixel referencePixel = {screen_width / 2, screen_height / 2};  // Use center as reference
        MPC ref_c = toComplex(referencePixel);

        std::vector<std::complex<double>> ref_orbit;
        MPC ref_z{0, 0};

        for (unsigned i = 0; i < max_iterations; ++i) {
                ref_z = ref_z * ref_z + ref_c;
                if (ref_z.abs_larger_than(thres)) break;
                ref_orbit.push_back({ref_z.real_double(), ref_z.imag_double()});
        }

#pragma omp parallel for schedule(dynamic)
        for (int y = 0; y < screen_height; y++) {
                MPC z{0, 0};
                for (int x = 0; x < screen_width; x++) {
                        size_t index = y * (pitch / sizeof(uint32_t)) + x;
                        Pixel p{x, y};
                        MPC c = toComplex(p);

                        std::complex<double> delta_c = {c.real_double() - ref_c.real_double(),
                                                        c.imag_double() - ref_c.imag_double()};
                        std::complex<double> delta_z = {0.0, 0.0};
                        unsigned i = 0;

                        for (; i < ref_orbit.size(); ++i) {
                                delta_z = 2.0 * ref_orbit[i] * delta_z + delta_c;
                                if (std::norm(ref_orbit[i] + delta_z) > 4.0) break;
                        }

                        double sm_iter{};

                        // If pixel orbit diverges from reference, recompute with full precision
                        if (i == ref_orbit.size()) {
                                z.set(0, 0);
                                sm_iter = smooth_iteration(z, c);
                                i = static_cast<int>(sm_iter);
                        }

                        //double hue = 360.0 * i / max_iterations;
                        //pixelArray[index] = HSVtoARGB(hue, 1.0, 1.0);
                        ////double brightness = pow(sm_iter / max_iterations, 0.5);  // Adjust gamma
                        ////double hue = 360.0 * brightness;
                        //pixelArray[index] = HSVtoARGB(scaled_iter, 1.0, 1.0);
                        ////pixelArray[index] = HSVtoARGB(hue, 1.0, brightness);
                        pixelArray[index] = coloring1(sm_iter);
                }
        }

        SDL_UnlockTexture(texture);
}


Pixel Mandelbrot::toScreen(const MPC &z) {

        MPC tmp1 = z - vp_bl;
        MPC re = tmp1.real();
        MPC im = tmp1.imag();
        MPC re_tmp = re / vp_diff_real;
        MPC im_tmp = im / vp_diff_imag;
        MPC out_re = re_tmp * vp_width;
        MPC out_im = im_tmp * vp_height;
        double out_x = out_re.real_double();
        double out_y = out_im.real_double();

        return Pixel{static_cast<int>(out_x), static_cast<int>(out_y)};
}

MPC Mandelbrot::toComplex(Pixel &pixel) {
        MPC re_ratio{static_cast<double>(pixel.x)/static_cast<double>(screen_width)};
        MPC im_ratio{static_cast<double>(pixel.y)/static_cast<double>(screen_height)};
        re_ratio *= vp_diff_real;
        im_ratio *= vp_diff_imag;
        re_ratio += vp_bl_real;
        im_ratio += vp_bl_imag;
        MPC tmp{re_ratio, im_ratio};
        return tmp;
}

int Mandelbrot::iteration(MPC &z, MPC &c) {
        int i = 0;
        for (; i < max_iterations; ++i) {
                z *= z;
                z += c;
                if (z.abs_larger_than(thres)) return i;
        }
        return i;
}

double Mandelbrot::smooth_iteration(MPC &z, MPC &c) {
        unsigned i = 0;
        for (; i < max_iterations; ++i) {
                z *= z;
                z += c;
                if (z.abs_larger_than(thres)) break;
        }
        z.norm_inplace();
        z.log_inplace();
        z.log_inplace();
        MPC i_c{static_cast<double>(i+1)};
        i_c -= z;
        return i_c.real_double();
}
//
// PRIVATE METHODS END



// CONSTRUCTORS START
//
//
// CONSTRUCTORS END



// PUBLIC METHODS START
//
void Mandelbrot::initialize(SDL_Renderer *renderer, int width, int height) {
        screen_width = width;
        screen_height = height;
        this->renderer = renderer;
        init();
}

SDL_Texture *Mandelbrot::get_texture() const {
        return texture;
}
//
// PUBLIC METHODS END



// DECONSTRUCTOR START
//
Mandelbrot::~Mandelbrot() {
        SDL_DestroyTexture(texture);
}

//
// DECONSTRUCTOR END