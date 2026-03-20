
Solution
What we did

New file: constantblackscholesprocess.hpp / .cpp
We created ConstantBlackScholesProcess, a subclass of StochasticProcess1D that stores r, q and sigma as scalars fixed at construction time. The drift() and diffusion() methods return these values directly, with no calls to the term structures.

To avoid duplicating the parameter extraction logic in every engine, we placed it in a free function makeConstantProcess(process, exerciseTime, strikeForVol):
r and q are the continuous zero-rates at the exercise date
sigma is the Black implied volatility at (exerciseTime, strikeForVol)

Modified engines
We applied the same change to all three engines (mceuropeanengine.hpp, mc_discr_arith_av_strike.hpp, mclookbackengine.hpp):
Added a bool useConstantParams_ member to each engine class
Added withConstantParameters() to each Make class

The process substitution happens only in pathGenerator() — the pathPricer() continues to use the original rate curve for discounting
For the Asian average-strike, which has no fixed strike, we use the current spot (x0()) as an ATM reference for the volatility lookup.
Results
After compilation and execution (make test):
                               old engine                  non constant                      constant
           kind            NPV       time [s]            NPV       time [s]            NPV       time [s]
     ----------------------------------------------------------------------------------------------------
       European        4.17073        2.61828        4.17073        2.60521        4.17073        0.29443
          Asian       0.729431        2.38121       0.729431        2.37143       0.731168       0.290745
       Lookback         5.9998        2.60767         5.9998         2.6261        5.99705       0.303538

Observations
Non-constant vs old engine NPV: values must be identical (same seed, same process) — this is a reliable way to verify that the _2 engines introduce no regression.
Constant vs non-constant NPV: values differ slightly, which is expected. The gap comes from flattening the rate curve and the vol surface down to a single point.
Execution time: the speedup is sharp for European and Lookback (~9x), which have a regular time grid where term structure calls dominate. For the Asian the gain is smaller: the path pricer itself is heavier, so term structure calls represent a less dominant share of total runtime.
Difference across the three options: the Asian uses x0() as the vol reference in the absence of a fixed strike. If the vol surface is not flat, this introduces an extra approximation error, which is why the constant/non-constant NPV gap tends to be larger for this engine than for the other two.