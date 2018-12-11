#include <iostream>
#include <fstream>
#include <vector>
#include <array>

#include <AIToolbox/Utils/Core.hpp>
#include <AIToolbox/Factored/MDP/Algorithms/SparseCooperativeQLearning.hpp>
#include <AIToolbox/Factored/Utils/Core.hpp>
#include <AIToolbox/Factored/MDP/Policies/SingleActionPolicy.hpp>
#include <AIToolbox/Factored/MDP/Policies/EpsilonPolicy.hpp>

#include <CommandLineParsing.hpp>
#include <TurbinesProblem.hpp>
#include <ResultHandler.hpp>

namespace f = AIToolbox::Factored;
namespace fb = f::Bandit;
namespace fm = f::MDP;

int main(int argc, char** argv) {
    unsigned experiments;
    unsigned timesteps;
    std::string filename;
    Options options;
    options.push_back(makeDefaultedOption("experiments,e", &experiments, "set the number of experiments", 1u));
    options.push_back(makeDefaultedOption("timesteps,t", &timesteps, "set the timesteps per experiment", 40000u));
    options.push_back(makeRequiredOption("output,o", &filename, "set the final output file"));

    if (!parseCommandLine(argc, argv, options))
        return 1;

    auto [A, getRew, deps, rules] = makeTurbinesProblem();
    // size_t factorsNum = deps.size();

    f::Rewards rews(A.size());

    fm::SingleActionPolicy p({}, A);
    fm::EpsilonPolicy ep(p);

    double minRew = 1.0, maxRew = 0.0;
    double minAllRew = 1.0, maxAllRew = 0.0;
    std::array<unsigned, 20> buckets{};

    ResultHandler regrets(timesteps);

    for (unsigned e = 0; e < experiments; ++e) {
        std::cout << "Experiment " << e+1 << std::endl;
        f::Action action(A.size());

        fm::SparseCooperativeQLearning solver({}, A, 0.9, 0.3);

        for (const auto & rule : rules)
            solver.insertRule(fm::QFunctionRule{{}, rule.action, rule.value});

        constexpr double init = 0.90;
        for (unsigned t = 0; t < timesteps; ++t) {
            ep.setEpsilon(init + std::min(1.0 - init, 0.000001666 * t));
            std::cout << "[" << e+1 << "] Timestep " << t+1 << std::endl;//" " << ep.getEpsilon() << "\n";

            auto rews = getRew(action);
            // const auto min = rews.minCoeff();
            // const auto max = rews.maxCoeff();
            // const auto sum = rews.sum();
            // if (min < minRew)
            //     minRew = min;
            // if (max > maxRew)
            //     maxRew = max;
            // if (sum < minAllRew)
            //     minAllRew = sum;
            // if (sum > maxAllRew)
            //     maxAllRew = sum;
            // buckets[sum*20]++;
            // if (t < 100) {
            //     printAction(action);
            //     std::cout << " ==> " << rews.transpose() << '\n';
            // }

            const double regret = (1.0 - rews.sum());
            regrets.record(regret, t);

            p.updateAction(solver.stepUpdateQ({}, action, {}, rews));
            action = ep.sampleAction({});
        }
        // solver.getDiscount();
    }
    std::cout << "MIN REW OBTAINED: " << minRew << "; MAX REW OBTAINED = " << maxRew << '\n';
    std::cout << "OVERALL MIN REW OBTAINED: " << minAllRew << "; MAX REW OBTAINED = " << maxAllRew << '\n';
    std::cout << "BUCKETS: [";
    for (const auto v : buckets) std::cout << v << ", ";
    std::cout << "]\n";

    std::ofstream file(filename);
    file << regrets;
}
