#ifndef RESULT_HANDLER_HEADER_FILE
#define RESULT_HANDLER_HEADER_FILE

#include <vector>

/**
 * @brief This class registers data and computes statistics about it.
 *
 * This class is used to record reward/performance data in order to create mean
 * and standard deviation statistics from it in an efficient manner.
 */
class ResultHandler {
    public:
        //                        mean,  cum mean,  std,  cum std
        using Result = std::tuple<double, double, double, double>;
        using Results = std::vector<Result>;

        /**
         * @brief Basic constructor.
         *
         * @param timesteps The number of timesteps to process.
         */
        ResultHandler(size_t timesteps);

        /**
         * @brief This function records a new datapoint for the specified timestep.
         *
         * This function stores the input in a way that allows to obtain both
         * mean and standard deviation from the data later.
         *
         * @param value The value to register.
         * @param timestep The timestep of the value.
         */
        void record(double value, size_t timestep);

        /**
         * @brief This function computes mean and standard deviation for all timesteps.
         *
         * @return The mean and standard deviation for the recorded data.
         */
        Results process() const;

    private:
        //                       Count,    sum,    sum squared
        using Point = std::tuple<unsigned, double, double>;

        std::vector<Point> data_;
};

inline ResultHandler::ResultHandler(size_t timesteps) :
        data_(timesteps) {}

inline void ResultHandler::record(const double v, const size_t t) {
    auto & [count, sum, square] = data_[t];

    ++count;
    sum += v;
    square += v * v;
}

inline ResultHandler::Results ResultHandler::process() const {
    Results retval;
    retval.reserve(data_.size());

    double cumMean = 0.0;
    double cumVariance = 0.0;
    for (const auto & d : data_) {
        const auto & [count, sum, square] = d;

        const double mean = sum / count;
        const double variance = square / count - mean * mean;
        const double std = std::sqrt(variance);
        cumMean += mean;
        cumVariance += variance;
        const double cumStd = std::sqrt(cumVariance);

        retval.emplace_back(mean, cumMean, std, cumStd);
    }
    return retval;
}

/**
 * @brief This function writes the contents of the ResultHandler to the stream.
 *
 * The output will contain a series of lines, each formed by: timestep, mean,
 * and standard deviation.
 *
 * The output is GnuPlot friendly!
 *
 * @param os The stream to write to.
 * @param rh The ResultHandler to get data from.
 *
 * @return The input stream.
 */
inline std::ostream& operator<<(std::ostream& os, const ResultHandler & rh) {
    const auto data = rh.process();

    for (unsigned t = 0; t < data.size(); ++t) {
        const auto & [mean, cumMean, std, cumStd] = data[t];
        os << t << ' ' << mean << ' ' << cumMean << ' ' << std << ' ' << cumStd << '\n';
    }

    return os;
}

#endif
