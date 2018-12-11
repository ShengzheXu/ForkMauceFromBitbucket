#include <random>
#include <iostream>

#include <AIToolbox/Utils/Core.hpp>
#include <AIToolbox/Factored/Bandit/Algorithms/MAUCE.hpp>
#include <AIToolbox/Factored/Bandit/Algorithms/Utils/VariableElimination.hpp>
#include <AIToolbox/Factored/Utils/Core.hpp>

namespace f = AIToolbox::Factored;
namespace fb = f::Bandit;

inline void printPartialAction(const f::PartialAction & pa) {
    for (size_t i = 0; i < pa.first.size(); ++i) {
        std::cout << "(" << pa.first[i] << ", " << pa.second[i] << ")";
    }
}

inline void printAction(const f::Action & y){
    std::cout << "[";
    for (auto yy : y) std::cout << yy << ", ";
    std::cout << "]";
};

double rewFun(double p, size_t workers) {
    if (!workers)
        return 0.0;
    return p * std::pow(1.03, workers);
};

inline auto makeMiningProblem(unsigned seed) {
    std::cout << "Using seed " << seed << '\n';

    std::default_random_engine rand(seed);
    std::uniform_int_distribution<size_t> villages(5, 15);
    std::uniform_int_distribution<size_t> workersPerVillage(1, 5);
    std::uniform_int_distribution<size_t> minesPerVillage(2, 4);
    std::uniform_real_distribution<double> mineP(0, 0.5);

    // Generate villages and attached mines
    auto villagesNum = villages(rand);
    f::Action A(villagesNum);
    f::Action workers(villagesNum);
    for (size_t n = 0; n < villagesNum; ++n) {
        A[n] = minesPerVillage(rand);
        // Last village has 4 mines
        if (n == villagesNum - 1)
            A[n] = 4;

        workers[n] = workersPerVillage(rand);

        std::cout << "Village " << n << " has " << workers[n]
                  << " workers and " << A[n] << " mines.\n";
    }
    // Find out which villages are attached to each mine
    const auto minesNum = villagesNum + 3;
    auto mines = std::vector<std::vector<size_t>>(minesNum);
    for (size_t n = 0; n < villagesNum; ++n)
        for (size_t i = n; i < n + A[n]; ++i)
            mines[i].push_back(n);
    for (size_t i = 0; i < mines.size(); ++i) {
        std::cout << "Mine " << i << " is connected to villages: ";
        for (size_t j = 0; j < mines[i].size(); ++j)
            std::cout << mines[i][j] << ", ";
        std::cout << "\n";
    }

    // Some mines can be duplicates (they are connected to the same exact
    // villages). We need a way to get which mine belongs to what factor
    // for XXXAlgorithm.
    auto mineToFactor = std::vector<size_t>(minesNum);
    size_t factorsNum = 0;
    for (size_t m = 0; m < minesNum; ++m) {
        if (mines[m].size() != mines[factorsNum].size() ||
            AIToolbox::veccmp(mines[m], mines[factorsNum]) != 0)
        {
            ++factorsNum;
        }
        mineToFactor[m] = factorsNum;
        std::cout << "Mine " << m << " in factor " << factorsNum << "\n";
    }
    ++factorsNum;

    // Compute probabilities for each mine
    auto minePs = std::vector<double>(minesNum);
    for (size_t m = 0; m < minesNum; ++m) {
        minePs[m] = mineP(rand);
        std::cout << "Mine " << m << " has p: " << minePs[m] << '\n';
    }

    // Here we build the rules for the problem as now specified.
    std::vector<fb::QFunctionRule> rules;
    for (size_t m = 0; m < minesNum; ++m) {
        f::PartialFactorsEnumerator enumerator(A, mines[m]);
        while (enumerator.isValid()) {
            const auto & pAction = *enumerator;
            unsigned totalMiners = 0;
            for (size_t i = 0; i < pAction.first.size(); ++i) {
                if (pAction.first[i] + pAction.second[i] == m)
                    totalMiners += workers[pAction.first[i]];
            }
            const double v = rewFun(minePs[m], totalMiners);
            rules.emplace_back(fb::QFunctionRule{pAction, v});

            enumerator.advance();
        }
    }
    // And we solve it in order to find out which action is the best.
    fb::VariableElimination ve(A);
    auto result = ve(rules);

    std::cout << "Best action: "; printAction(std::get<0>(result));
    std::cout << " ==> " << std::get<1>(result) << '\n';
    auto norm = std::get<1>(result);

    // BUILDING STRUCTURES TO RUN THE EXPERIMENTS:

    // Here we build the function we will use to sample reward.
    // The reward obtained is normalized so that the average regret will be 1.
    auto getRew = [rand, workers, minePs, mineToFactor, norm](const f::Action & a) mutable {
        f::Rewards rews(mineToFactor.size());
        rews.fill(0.0);
        for (size_t m = 0; m < minePs.size(); ++m) {
            unsigned totalMiners = 0;
            for (size_t i = 0; i < a.size(); ++i) {
                if (i + a[i] == m)
                    totalMiners += workers[i];
            }
            const double p = rewFun(minePs[m], totalMiners);
            std::bernoulli_distribution roll(p / norm);
            rews[mineToFactor[m]] += static_cast<double>(roll(rand));
        }
        return rews;
    };

    // We build the factor dependencies for XXXAlgorithm
    std::vector<std::pair<double, std::vector<size_t>>> deps;
    double range = 0.0;
    size_t lastMine = 0;
    for (size_t m = 0; m <= minesNum; ++m) {
        if (m < minesNum && mineToFactor[m] == lastMine) {
            range += 1.0;
            continue;
        }
        deps.emplace_back(range, mines[lastMine]);
        range = 1.0;
        std::cout << "Adding vector: " << std::get<0>(deps.back()) << " -- ";
        for (auto v : mines[lastMine])
            std::cout << v << ", ";
        std::cout << "\n";

        lastMine = m;
    }

    // Building rules for SparseCooperativeQLearning
    std::vector<fb::QFunctionRule> rules2;
    lastMine = 1; // Any number different from 0 is fine here.
    size_t counter = 0;
    for (size_t m = 0; m < minesNum; ++m) {
        if (mineToFactor[m] == lastMine)
            continue;
        f::PartialFactorsEnumerator enumerator(A, mines[m]);
        while (enumerator.isValid()) {
            const auto & pAction = *enumerator;
            rules2.emplace_back(fb::QFunctionRule{pAction, 10.0});
            std::cout << "Rule number " << counter++ << ": ";
            printPartialAction(pAction);
            std::cout << " ==> 10.0\n";

            enumerator.advance();
        }
        std::cout << "------\n";
        lastMine = m;
    }

    return std::make_tuple(A, getRew, deps, rules2);
}
