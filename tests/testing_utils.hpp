#include "anyode/anyode.hpp"

int rhs_cb(double t, const double * const y, double * const f, void * user_data){
    AnyODE::ignore(t); AnyODE::ignore(user_data);
    f[0] = -y[0];
    return 0;
}

struct Decay : public AnyODE::OdeSysBase {
    double m_k;

    Decay(double k) : m_k(k) {}
    int get_ny() const override { return 1; }
    AnyODE::Status rhs(double t, const double * const __restrict__ y, double * const __restrict__ f) override {
        AnyODE::ignore(t);
        f[0] = -y[0];
        this->nfev++;
        return AnyODE::Status::success;
    }
};
