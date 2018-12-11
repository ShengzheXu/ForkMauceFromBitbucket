#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

#include <AIToolbox/Utils/Core.hpp>
#include <AIToolbox/Factored/Utils/Core.hpp>

#include <CommandLineParsing.hpp>
#include <TurbinesProblem.hpp>
#include <ResultHandler.hpp>

#define PRINTD(x)
//#define PRINTD(x) std::cout << x

namespace f = AIToolbox::Factored;
namespace fb = f::Bandit;
// namespace fm = f::MDP;

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

    std::tie(A, getRew, deps, std::ignore) = makeTurbinesProblem();
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

    std::default_random_engine actionRand(seed);
    std::vector<std::uniform_int_distribution<size_t>> selectors;
    for (size_t i = 0; i < A.size(); ++i)
        selectors.emplace_back(0, A[i]-1);

    for (unsigned e = 0; e < experiments; ++e) {
        std::cout << "Experiment " << e+1 << std::endl;
        f::Action action(A.size(), 0);

        for (unsigned t = 0; t < timesteps; ++t) {
            std::cout << "[" << e+1 << "] Timestep " << t + 1 << std::endl;
            const auto tmp = getRew(action);

            const double regret = (1.0 - tmp.sum());
            regrets.record(regret, t);

            for (size_t i = 0; i < action.size(); ++i)
                action[i] = selectors[i](actionRand);
            PRINTD("STEP UPDATE ENDED, NOW STARTING NEXT TIMESTEP ######\n\n");
        }
        // x.setTimestep(0);
    }

    std::ofstream file(filename);
    file << regrets;
}
