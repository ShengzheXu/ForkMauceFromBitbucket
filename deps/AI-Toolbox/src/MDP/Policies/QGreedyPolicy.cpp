#include <AIToolbox/MDP/Policies/QGreedyPolicy.hpp>

#include <AIToolbox/Utils/Core.hpp>

namespace AIToolbox::MDP {
    QGreedyPolicy::QGreedyPolicy(const QFunction & q) :
            PolicyInterface::Base(q.rows(), q.cols()), QPolicyInterface(q), bestActions_(getA()) {}

    size_t QGreedyPolicy::sampleAction(const size_t & s) const {
        // Automatically sets initial best action as bestAction[0] = 0
        bestActions_[0] = 0;

        // This work is due to multiple max-valued actions
        double bestQValue = q_(s, 0); unsigned bestActionCount = 1;
        for ( size_t a = 1; a < A; ++a ) {
            // The checkEqualGeneral is before the greater since we want to
            // trap here things that may be equal (even if one is a tiny bit
            // higher than the other).
            if ( checkEqualGeneral(q_(s, a), bestQValue) ) {
                bestActions_[bestActionCount] = a;
                ++bestActionCount;
            }
            // In case the new element is really higher than the other, then we
            // reset the counts.
            else if ( q_(s, a) > bestQValue ) {
                bestActions_[0] = a;
                bestActionCount = 1;
                bestQValue = q_(s, a);
            }
        }

        auto pickDistribution = std::uniform_int_distribution<unsigned>(0, bestActionCount-1);
        const unsigned selection = pickDistribution(rand_);

        return bestActions_[selection];
    }

    double QGreedyPolicy::getActionProbability(const size_t & s, const size_t & a) const {
        const double max = q_(s, a); unsigned count = 0;
        for ( size_t aa = 0; aa < A; ++aa ) {
            // The checkEqualGeneral is before the greater since we want to
            // trap here things that may be equal (even if one is a tiny bit
            // higher than the other).
            if ( checkEqualGeneral(q_(s, aa), max) ) ++count;
            // In case the new element is really higher than the other, then we
            // reset the counts.
            else if ( q_(s, aa) > max ) {
                return 0.0;
            }
        }
        return 1.0 / count;
    }

    Matrix2D QGreedyPolicy::getPolicy() const {
        Matrix2D retval(S, A);

        for (size_t s = 0; s < S; ++s) {
            double max = q_(s, 0); unsigned count = 1;
            for ( size_t aa = 1; aa < A; ++aa ) {
                if ( checkEqualGeneral(q_(s, aa), max) ) ++count;
                else if ( q_(s, aa) > max ) {
                    max = q_(s, aa);
                    count = 1;
                }
            }
            for ( size_t aa = 0; aa < A; ++aa ) {
                if ( checkEqualGeneral(q_(s, aa), max) )
                    retval(s, aa) = 1.0 / count;
                else
                    retval(s, aa) = 0.0;
            }
        }

        return retval;
    }
}