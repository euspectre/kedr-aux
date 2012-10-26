/*
 * Optimize tests set, line coverage remain unchanged.
 */

#ifndef OPTIMIZE_TESTS_HH
#define OPTIMIZE_TESTS_HH

#include <vector>
#include <string>

#include "trace.hh"

/* Coverage-related descriptor of the test. */
struct TestCoverageDesc
{
    /* Trace file with coverage */
    std::string traceFile;
    /* 
     * Weight of the test.
     * 
     * It is interpreted as:
     * With other conditions remains, one tests set is more preferrable
     * then other set if its weight(sum of weights of tests it contain)
     * is less then weight of other set.
     * 
     * Weight should be non-negative.
     */
    double weight;
    
    TestCoverageDesc(const std::string& traceFile, double weight):
        traceFile(traceFile), weight(weight) {}
};


class TestSetOptimizer
{
public:
    /* 
     * Load given test set into optimizer.
     */
    TestSetOptimizer(const std::vector<TestCoverageDesc>& tests);
    /* 
     * Return test set which has same line coverage as one, passed to
     * constructor, but minimal weight.
     * 
     * Order is be preserved.
     * 
     * Resulted array exists while object itself exists.
     */
    const std::vector<TestCoverageDesc>& optimize(bool verbose);
private:
    /* Input and output(optimal) tests set. */
    std::vector<TestCoverageDesc> tests;
};



/* 
 * Accept set of tests descriptors.
 * 
 * Optimize this set, so its total line coverage remain unchanged,
 * but weight has minimal value.
 * 
 * Result is stored in 'optTests'. Order of tests is preserved.
 * 
 * All tests with zero weight are included into resulted set
 * unconditionally.
 */
void optimizeTests(const std::vector<TestCoverageDesc>& tests,
    std::vector<TestCoverageDesc>& optTests);


#endif /* OPTIMIZE_TESTS_HH */
