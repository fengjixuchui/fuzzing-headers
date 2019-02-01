#ifndef FUZZING_TESTERS_SERIALIZE_DIFFERENTIAL_HPP
#define FUZZING_TESTERS_SERIALIZE_DIFFERENTIAL_HPP

#include <fuzzing/datasource/datasource.hpp>
#include <memory>
#include <vector>
#include <optional>
#include <functional>

namespace fuzzing {
namespace testers {
namespace differential {

template <typename UniversalInput, typename UniversalOutput>
class DifferentialTarget {
    private:
        std::function<std::optional<UniversalOutput>(const UniversalInput&)> fn;
    public:
        DifferentialTarget(std::function<std::optional<UniversalOutput>(const UniversalInput&)> fn) : fn(fn) { }
        std::optional<UniversalOutput> Run(const UniversalInput& input) const {
            return fn(input);
        }
};


template <typename UniversalInput, typename UniversalOutput>
class DifferentialTester {
    protected:
        virtual UniversalInput DSToUniversalInput(datasource::Datasource& ds) const = 0;
    private:
        std::vector<DifferentialTarget<UniversalInput, UniversalOutput>> targets;

        bool compare(const std::vector<std::optional<UniversalOutput>> results) {
            std::vector<size_t> success;

            for (size_t i = 0; i < results.size(); i++) {
                if ( results[i] != std::nullopt ) {
                    success.push_back(i);
                }
            }

            if ( success.size() < 2 ) {
                /* Nothing to compare */
                return true;
            }

            for (size_t i = 0; i < success.size() - 1; i++) {
                const size_t curIndex = success[i];
                const size_t nextIndex = success[i+1];
                if ( *(results[curIndex]) != *(results[nextIndex]) ) {
                    return false;
                }
            }

            return true;
        }

    public:
        DifferentialTester(std::initializer_list<DifferentialTarget<UniversalInput, UniversalOutput>> targets) : targets{std::move(targets)} {}
        bool Run(datasource::Datasource& ds) {
            std::vector<std::optional<UniversalOutput>> results(targets.size());

            const auto input = DSToUniversalInput(ds);

            size_t numFailed = 0;
            for (size_t i = 0; i < targets.size(); i++) {
                auto& curTarget = targets[i];
                results[i] = curTarget.Run(input);
                
                numFailed += results[i] == std::nullopt ? 1 : 0;
            }

            if ( numFailed == targets.size() ) {
                /* All failed */

                return false;
            }

            if ( compare(results) == false ) {
                printf("KRESH\n");
                /* TODO call crash callback */
                return false;
            }

            return true;
        }
};

} /* namespace differential */
} /* namespace testers */
} /* namespace fuzzing */

#endif
