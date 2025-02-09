//
// Created by dan on 2/8/25.
//

#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <SDL3/SDL.h>
#include "MPC.h"
#include <vector>

struct Pixel {
        int x{0};
        int y{0};
        Pixel(const int x, const int y) : x(x), y(y) {}
};

struct Orbit {
        MPC full_precision;  // High-precision reference orbit
        std::vector<std::complex<double>> deltas;  // Perturbations
};


#define POINT_RE 0
#define POINT_IM 0
#define DELTA 2

class Mandelbrot {
private:
        int screen_width{100};
        int screen_height{100};
        SDL_Renderer *renderer{nullptr};
        SDL_Texture *texture{nullptr};
        MPC vp_bl{POINT_RE+DELTA, POINT_IM+DELTA};
        MPC vp_bl_real;
        MPC vp_bl_imag;
        MPC vp_tr{POINT_RE-DELTA, POINT_IM-DELTA};
        MPC vp_diff;
        MPC vp_diff_real;
        MPC vp_diff_imag;
        MPC vp_corr_real;
        MPC vp_corr_imag;
        MPC vp_fact_real;
        MPC vp_fact_imag;
        MPC vp_width;
        MPC vp_height;
        MPC thres;
        MPC offset{DELTA, DELTA};
        // Transform point in complex plane to point on screen
        Pixel toScreen(const MPC&);
        MPC toComplex(Pixel&);

        int iteration(MPC &z, MPC &c);

        double smooth_iteration(MPC &z, MPC &c);

        int max_iterations{10};

        void init();

        void precompute_viewport();

        void compute_texture();

        uint32_t coloring1(double sm_iterations) const;

        void compute_texture2();

        void compute() {
                precompute_viewport();
                compute_texture();
        }

public:
        void initialize(SDL_Renderer *, int, int);

        [[nodiscard]] SDL_Texture *get_texture() const;

        void set_viewport(std::complex<double> bl, std::complex<double> tr) {
                vp_bl.set(bl.real(), bl.imag());
                vp_tr.set(tr.real(), tr.imag());
                compute();
        }

        void set_viewport(Pixel bl, Pixel tr) {
                vp_bl = toComplex(bl);
                vp_tr = toComplex(tr);
                compute();
        }

        void shrink_offset() {
                constexpr double fac_d = 0.25;
                MPC fac{fac_d};
                //max_iterations = static_cast<int>(max_iterations * std::sqrt(1.0/fac_d));
                offset *= fac;
        }

        void set_viewport(Pixel center) {
                MPC c1 = toComplex(center);
                MPC c2 = toComplex(center);
                std::cout << "Center: " << c1 << std::endl;
                c1 -= offset;
                c2 += offset;
                vp_bl = c1;
                vp_tr = c2;

                // compute max iterations
                MPC diff = vp_tr - vp_bl;
                MPC width = diff.real();
                MPC zoom{3.0};
                zoom /= width;
                double zoom_d = zoom.real_double();
                std::cout << "Zoom: " << zoom_d << std::endl;
                max_iterations = static_cast<int>(100.0+50.0*log2(zoom_d));

                compute();
        }

        void print_info() {
                std::cout << "Offset: " << offset << std::endl;
                std::cout << "Bottom Left: " << vp_bl << std::endl;
                std::cout << "Top Right: " << vp_tr << std::endl;
                std::cout << "Max. Iterations: " << max_iterations << std::endl;
        }

        ~Mandelbrot();
};



#endif //MANDELBROT_H
