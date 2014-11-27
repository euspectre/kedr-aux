#ifndef TRACE_OPERATION_HH_INCLUDE
#define TRACE_OPERATION_HH_INCLUDE

/*
 * Abstract operation with trace(s), which produce another trace.
 * 
 * Operation is assumed to be aggregate operation,
 * which operates on array of counters for trace's objects with same id,
 * and produce one counter.
 */

#include <string>
#include <vector>
#include <map>

class TraceOperation
{
public:
    virtual ~TraceOperation() {}

    /*
     * Counters are encoded with signed integer type.
     * 
     * - non-negative value represent real value of the counter;
     * - -1 means that corresponded counter is absent in the trace;
     * - -2 is used for '-' in branch records(BRDA directive).
     */
    typedef long counter_t;

    /*
     * Operates with counters for the same objects in different traces.
     * 
     * Methods are called only for counters, which exists at least in
     * one trace.
     * 
     * Methods may return -1 which means that counter shouldn't be present
     * in the resulted trace.
     */
    virtual counter_t lineOperation(const std::vector<counter_t>& operands) = 0;
    virtual counter_t functionOperation(const std::vector<counter_t>& operands) = 0;
    virtual counter_t branchOperation(const std::vector<counter_t>& operands) = 0;
};

/* Operation which affect on all types of counters in same fashion. */
class TraceOperationUnified: public TraceOperation
{
public:
    virtual counter_t lineOperation(const std::vector<counter_t>& operands)
    {
        return counterOperation(operands);
    }
    virtual counter_t functionOperation(const std::vector<counter_t>& operands)
    {
        return counterOperation(operands);
    }
    virtual counter_t branchOperation(const std::vector<counter_t>& operands)
    {
        return counterOperation(operands);
    }


    virtual counter_t counterOperation(const std::vector<counter_t>& operands) = 0;
};

#endif /* TRACE_OPERATION_HH_INCLUDE */
