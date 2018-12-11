#include <iostream>
#include <fstream>
#include <vector>

#include <AIToolbox/Utils/Core.hpp>
#include <AIToolbox/Factored/Utils/Core.hpp>

#include <CommandLineParsing.hpp>
#include <MiningProblem.hpp>
#include <ResultHandler.hpp>

namespace f = AIToolbox::Factored;
namespace fb = f::Bandit;

int main(int argc, char** argv) {
    size_t nodes;
    unsigned experiments;
    unsigned timesteps;
    std::string filename;
    Options options;
    options.push_back(makeRequiredOption("nodes,n", &nodes, "set the experiment's nodes number"));
    options.push_back(makeDefaultedOption("experiments,e", &experiments, "set the number of experiments", 1000u));
    options.push_back(makeDefaultedOption("timesteps,t", &timesteps, "set the timesteps per experiment", 500u));
    options.push_back(makeRequiredOption("output,o", &filename, "set the final output file"));

    if (!parseCommandLine(argc, argv, options))
        return 1;
    if (nodes < 3 || !(nodes % 2)) {
        std::cout << "Nodes cannot be less than three nor even!\n";
        return 1;
    }

    f::Action A(nodes, 2);
    f::Rewards rew(nodes - 1);
    std::default_random_engine actionRand(nodes);
    std::bernoulli_distribution actionDist(0.5);

    const double factorsNum = A.size() - 1.0;

    auto getEvenReward = [factorsNum](size_t a1, size_t a2){
        static std::default_random_engine rand(0);
        if (!a1 && !a2) {
            std::bernoulli_distribution roll(0.75);
            return roll(rand) / factorsNum;
        } else if (!a1 && a2) {
            return 1.0 / factorsNum;
        } else if (a1 && !a2) {
            std::bernoulli_distribution roll(0.25);
            return roll(rand) / factorsNum;
        } else {
            std::bernoulli_distribution roll(0.9);
            return roll(rand) / factorsNum;
        }
    };

    auto getOddReward = [factorsNum](size_t a1, size_t a2){
        static std::default_random_engine rand(1);
        if (!a1 && !a2) {
            std::bernoulli_distribution roll(0.75);
            return roll(rand) / factorsNum;
        } else if (!a1 && a2) {
            std::bernoulli_distribution roll(0.25);
            return roll(rand) / factorsNum;
        } else if (a1 && !a2) {
            return 1.0 / factorsNum;
        } else {
            std::bernoulli_distribution roll(0.9);
            return roll(rand) / factorsNum;
        }
    };

    ResultHandler regrets(timesteps);

    for (unsigned e = 0; e < experiments; ++e) {
        std::cout << "Experiment " << e+1 << std::endl;
        f::Action action(A.size(), 0);

        for (unsigned t = 0; t < timesteps; ++t) {
            // printAction(action);
            for (unsigned i = 0; i < nodes-1; i+=2) {
                rew[i]   = getEvenReward(action[i], action[i+1]);
                rew[i+1] = getOddReward(action[i+1], action[i+2]);
            }
            // std::cout << " ==> " << rew[0];
            // for (size_t q = 1; q < nodes-1; ++q) std::cout << ", " << rew[q];
            // std::cout << "\n";
            const double regret = (1.0 - rew.sum());
            regrets.record(regret, t);

            for (size_t i = 0; i < action.size(); ++i)
                action[i] = actionDist(actionRand);
        }
    }

    std::ofstream file(filename);
    file << regrets;
}
