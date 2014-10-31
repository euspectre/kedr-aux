#ifndef DO_TRACE_OPERATION_HH
#define DO_TRACE_OPERATION_HH

#include "trace.hh"
#include "trace_operation_include/trace_operation.hh"

#include <vector>

void doTraceOperation(TraceOperation& op,
    const std::vector<Trace>& operands, Trace& result);

#endif /* DO_TRACE_OPERATION_HH */
