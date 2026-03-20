#pragma once

#include <ql/processes/blackscholesprocess.hpp>
#include <ql/stochasticprocess.hpp>
#include <cmath>

namespace QuantLib {

    // Black-Scholes process with constant parameters (r, q, sigma fixed at construction).
    // drift() returns the log-price drift r - q - 0.5*sigma^2, no term-structure calls.
    class ConstantBlackScholesProcess : public StochasticProcess1D {
      public:
        ConstantBlackScholesProcess(Real x0, Rate r, Rate q, Volatility sigma)
        : x0_(x0), r_(r), q_(q), sigma_(sigma) {}

        Real x0()                          const override { return x0_; }
        Real drift(Time, Real)             const override { return r_ - q_ - 0.5*sigma_*sigma_; }
        Real diffusion(Time, Real)         const override { return sigma_; }
        Real apply(Real x0, Real dx)       const override { return x0 * std::exp(dx); }
        Real evolve(Time, Real x0, Time dt, Real dw) const override {
            // standard lognormal step: S(t+dt) = S(t) * exp((r-q-0.5s^2)*dt + s*sqrt(dt)*dw)
            return apply(x0, drift(0.0, x0)*dt + diffusion(0.0, x0)*std::sqrt(dt)*dw);
        }

      private:
        Real x0_;
        Rate r_, q_;
        Volatility sigma_;
    };

    // Extract constant parameters from a GeneralizedBlackScholesProcess at exerciseTime.
    // strikeForVol: reference point on the vol surface (use spot for path-dependent options
    // that have no fixed strike, like average-strike Asian).
    // r and q are continuously-compounded zero rates; sigma is the Black implied vol.
    // This helper is shared by all three engines to avoid duplication.
    inline ext::shared_ptr<ConstantBlackScholesProcess>
    makeConstantProcess(const ext::shared_ptr<GeneralizedBlackScholesProcess>& p,
                        Time exerciseTime,
                        Real strikeForVol) {
        Real       x0  = p->x0();
        Rate       r   = p->riskFreeRate()
                          ->zeroRate(exerciseTime, Continuous, NoFrequency, true).rate();
        Rate       q   = p->dividendYield()
                          ->zeroRate(exerciseTime, Continuous, NoFrequency, true).rate();
        Volatility vol = p->blackVolatility()->blackVol(exerciseTime, strikeForVol, true);
        return ext::make_shared<ConstantBlackScholesProcess>(x0, r, q, vol);
    }

}
