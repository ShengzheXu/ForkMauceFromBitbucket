#include <iostream>
#include <fstream>
#include <vector>

#include <AIToolbox/Utils/Core.hpp>
#include <AIToolbox/Factored/Bandit/Algorithms/MAUCE.hpp>
#include <AIToolbox/Factored/Utils/Core.hpp>

#include <CommandLineParsing.hpp>
#include <MiningProblem.hpp>
#include <ResultHandler.hpp>

namespace f = AIToolbox::Factored;
namespace fb = f::Bandit;

int main(int argc, char** argv) {
    int seed;
    unsigned experiments;
    unsigned timesteps;
    std::string filename;
    Options options;
    options.push_back(makeRequiredOption("seed,s", &seed, "set the experiment's seed"));
    options.push_back(makeDefaultedOption("experiments,e", &experiments, "set the number of experiments", 1u));
    options.push_back(makeDefaultedOption("timesteps,t", &timesteps, "set the timesteps per experiment", 40000u));
    options.push_back(makeRequiredOption("output,o", &filename, "set the final output file"));

    if (!parseCommandLine(argc, argv, options))
        return 1;

    f::Action A;
    std::vector<std::pair<double, std::vector<size_t>>> deps;
    std::function<f::Rewards(const f::Action &)> getRew;

    std::tie(A, getRew, deps, std::ignore) = makeMiningProblem(seed);
    size_t factorsNum = deps.size();
    for (size_t i = 0; i < deps.size(); ++i) {
        std::cout << "Dep: ";
        for (size_t j = 0; j < deps[i].second.size(); ++j) {
            std::cout << deps[i].second[j] << ", ";
        }
        std::cout << "\n";
    }

    ResultHandler regrets(timesteps);
    f::Rewards rews(factorsNum);

    for (unsigned e = 0; e < experiments; ++e) {
        std::cout << "Experiment " << e+1 << '\n';
        f::Action action(A.size());

        fb::MAUCE x(A, deps);

        for (unsigned t = 0; t < timesteps; ++t) {
            std::cout << "Timestep " << t + 1 << '\n';
            rews = getRew(action);

            // if (t > 28000) {
            //    printAction(action);
            //    std::cout << " ==> " << rews.transpose() << '\n';
            // }

            const double regret = (1.0 - rews.sum());
            regrets.record(regret, t);

            action = x.stepUpdateQ(action, rews);
        }
        // x.setTimestep(0);
    }

    std::ofstream file(filename);
    file << regrets;
}
