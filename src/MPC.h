//
// Created by Daniel Leuthold on 2/8/25.
//

#ifndef MPC_H
#define MPC_H

#include <mpc.h>
#include <string>
#include <iostream>
#include <complex>

#define DEFAULT_VALUE 0
#define DEFAULT_RE_PRECISION 256
#define DEFAULT_IM_PRECISION 256
#define DEFAULT_ROUND_MODE MPC_RNDNN

/*
 * THIS IS NOT A PRODUCTION READY CLASS, USE IT AT YOUR OWN RISK
 */

class MPC {
private:
        mpc_t _mpc{NULL};
        void init() {
                //std::cout << "init() called" << std::endl;
                mpc_init3(_mpc, DEFAULT_RE_PRECISION, DEFAULT_IM_PRECISION);
        }
public:
        MPC() {
                init();
                mpc_set_d(_mpc, DEFAULT_VALUE, DEFAULT_ROUND_MODE);
        }

        explicit MPC(const double real) {
                init();
                mpc_set_d(_mpc, real, DEFAULT_ROUND_MODE);
        }

        explicit MPC(const mpfr_t real) {
                init();
                mpc_set_fr(_mpc, real, DEFAULT_ROUND_MODE);
        }

        MPC(const double real, const double imag) {
                init();
                mpc_set_d_d(_mpc, real, imag, DEFAULT_ROUND_MODE);
        }

        MPC(MPC &real, MPC &imag) {
                init();
                mpfr_t real_val;
                mpfr_init2(real_val, DEFAULT_RE_PRECISION);
                mpc_real(real_val, real.get_mpc(), MPFR_RNDN);
                mpfr_t imag_val;
                mpfr_init2(imag_val, DEFAULT_IM_PRECISION);
                mpc_real(imag_val, imag.get_mpc(), MPFR_RNDN);
                mpc_set_fr_fr(_mpc, real_val, imag_val, DEFAULT_ROUND_MODE);
                mpfr_clear(real_val);
                mpfr_clear(imag_val);
        }

        explicit MPC(const std::complex<double> z) {
                init();
                mpc_set_d_d(_mpc, z.real(), z.imag(), DEFAULT_ROUND_MODE);
        }

        mpc_t &get_mpc() {
                return _mpc;
        }

        MPC operator+(MPC& rhs) const {
                MPC result;
                mpc_add(result.get_mpc(), _mpc, rhs.get_mpc(), DEFAULT_ROUND_MODE);
                return result;
        }

        MPC operator-(MPC& rhs) const {
                MPC result;
                mpc_sub(result.get_mpc(), _mpc, rhs.get_mpc(), DEFAULT_ROUND_MODE);
                return result;
        }

        MPC operator-() const {
                MPC result;
                mpc_neg(result.get_mpc(), _mpc, DEFAULT_ROUND_MODE);
                return result;
        }

        MPC operator*(MPC& rhs) const {
                MPC result;
                mpc_mul(result.get_mpc(), _mpc, rhs.get_mpc(), DEFAULT_ROUND_MODE);
                return result;
        }

        MPC operator/(MPC& rhs) const {
                MPC result;
                mpc_div(result.get_mpc(), _mpc, rhs.get_mpc(), DEFAULT_ROUND_MODE);
                return result;
        }

        void operator/=(MPC& rhs) {
                mpc_div(_mpc, _mpc, rhs.get_mpc(), DEFAULT_ROUND_MODE);
        }

        void operator+=(MPC& rhs) {
                mpc_add(_mpc, _mpc, rhs.get_mpc(), DEFAULT_ROUND_MODE);
        }

        void operator-=(MPC& rhs) {
                mpc_sub(_mpc, _mpc, rhs.get_mpc(), DEFAULT_ROUND_MODE);
        }

        void operator*=(MPC& rhs) {
                mpc_mul(_mpc, _mpc, rhs.get_mpc(), DEFAULT_ROUND_MODE);
        }

        friend std::ostream& operator<<(std::ostream &lhs, MPC &obj) {
                char *str = mpc_get_str(10, 20, obj.get_mpc(), DEFAULT_ROUND_MODE);
                const std::string s{str};
                mpc_free_str(str);
                return lhs << s;
        }

        [[nodiscard]] std::string to_string(const unsigned n) const {
                char *str = mpc_get_str(10, n, _mpc, DEFAULT_ROUND_MODE);
                const std::string s{str};
                mpc_free_str(str);
                return s;
        }

        [[nodiscard]] double get_arg() const {
                mpfr_t arg;
                mpfr_init2(arg, DEFAULT_RE_PRECISION);
                mpc_arg(arg, _mpc, MPFR_RNDN);
                double res = mpfr_get_d(arg, MPFR_RNDN);
                mpfr_clear(arg);
                return res;
        }

        bool abs_larger_than(MPC &z) const {
                return mpc_cmp_abs(_mpc, z.get_mpc()) > 0;
        }

        [[nodiscard]] MPC real() const {
                mpfr_t real_val;
                mpfr_init2(real_val, DEFAULT_RE_PRECISION);
                mpc_real(real_val, _mpc, MPFR_RNDN);
                MPC res{real_val};
                mpfr_clear(real_val);
                return res;
        }

        [[nodiscard]] MPC norm() const {
                mpfr_t norm;
                mpfr_init2(norm, DEFAULT_RE_PRECISION);
                mpc_norm(norm, _mpc, MPFR_RNDN);
                MPC res{norm};
                mpfr_clear(norm);
                return res;
        }

        void norm_inplace() {
                __mpfr_struct *re = _mpc->re;
                __mpfr_struct *im = _mpc->im;
                mpc_norm(re, _mpc, MPFR_RNDN);
                mpfr_set_d(im, 0.0, MPFR_RNDN);
        }

        [[nodiscard]] double real_double() const {
                mpfr_t real_val;
                mpfr_init2(real_val, DEFAULT_RE_PRECISION);
                mpc_real(real_val, _mpc, MPFR_RNDN);
                double res = mpfr_get_d(real_val, MPFR_RNDN);
                mpfr_clear(real_val);
                return res;
        }

        [[nodiscard]] MPC imag() const {
                mpfr_t imag_val;
                mpfr_init2(imag_val, DEFAULT_IM_PRECISION);
                mpc_imag(imag_val, _mpc, MPFR_RNDN);
                MPC res{imag_val};
                mpfr_clear(imag_val);
                return res;
        }

        [[nodiscard]] MPC log() const {
                MPC res{};
                mpc_log(res.get_mpc(), _mpc, DEFAULT_ROUND_MODE);
                return res;
        }

        void log_inplace() {
                mpc_log(_mpc, _mpc, DEFAULT_ROUND_MODE);
        }

        [[nodiscard]] double imag_double() const {
                mpfr_t imag_val;
                mpfr_init2(imag_val, DEFAULT_IM_PRECISION);
                mpc_imag(imag_val, _mpc, MPFR_RNDN);
                double res = mpfr_get_d(imag_val, MPFR_RNDN);
                mpfr_clear(imag_val);
                return res;
        }

        void set(double re, double im) {
                mpc_set_d_d(_mpc, re, im, DEFAULT_ROUND_MODE);
        }

        ~MPC() {
                // Free allocated memory
                //mpc_clear(_mpc);
        }
};



#endif //MPC_H
