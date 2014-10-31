/*
 * Parser for trace, contained coverage information.
 * 
 * See http://ltp.sourceforge.net/coverage/lcov/geninfo.1.php
 * 
 * for desctiption of trace file format.
 */
#ifndef TRACE_PARSER_H
#define TRACE_PARSER_H

#include <string>
#include <trace.hh>

class TraceEventProcessor
{
public:
    typedef Trace::counter_t counter_t;
    /* TN: <testName> */
    virtual void onTestStart(const std::string& /*testName*/, int /*traceLine*/) {}
    /* No directive, just there is no other directives for current test */
    virtual void onTestEnd(int /*traceLine*/) {}
    
    /* SF: <sourcePath> */
    virtual void onSourceStart(const std::string& /*sourcePath*/, int /*traceLine*/) {}
    /* end_of_record */
    virtual void onSourceEnd(int /*traceLine*/) {}
    
    /* FN: <funcLine>, <funcName> */
    virtual void onFunction(const std::string& /*funcName*/,
        int /*funcLine*/, int /*traceLine*/) {}
    
    /* FNDA: <counter>, <funcName> */
    virtual void onFunctionCounter(const std::string& /*funcName*/,
        counter_t /*counter*/, int /*traceLine*/) {}
    
    /* FNF: <functionsTotal> */
    virtual void onFunctionsTotal(int /*functionsTotal*/, int /*traceLine*/) {}
    /* FNH: <functionsTotalHit> */
    virtual void onFunctionsTotalHit(int /*functionsTotalHit*/, int /*traceLine*/) {}
    
    /* 
     * BRDA: <branchLine>, <blockNumber>, <branchNumber>, <counter>
     */
    virtual void onBranchCoverage(int /*branchLine*/,
        int /*blockNumber*/, int /*branchNumber*/, counter_t /*counter*/, int /*traceLine*/) {}
    /*
     * BRDA: <branchLine>, <blockNumber>, <branchNumber>, -
     */
    virtual void onBranchNotCovered(int /*branchLine*/,
        int /*blockNumber*/, int /*branchNumber*/, int /*traceLine*/) {}
    
    /* BRF: <branchesTotal> */
    virtual void onBranchesTotal(int /*branchesTotal*/, int /*traceLine*/) {}
    /* BRH: <branchesTotalHit> */
    virtual void onBranchesTotalHit(int /*branchesTotalHit*/, int /*traceLine*/) {}

    /* DA: <line>, <counter> */
    virtual void onLineCounter(int /*line*/, counter_t /*counter*/, int /*traceLine*/) {}

    /* LF: <linesTotal> */
    virtual void onLinesTotal(int /*linesTotal*/, int /*traceLine*/) {}
    /* LH: <linesTotalHit> */
    virtual void onLinesTotalHit(int /*linesTotal*/, int /*traceLine*/) {}
};

class TraceParser
{
public:
    TraceParser();
    ~TraceParser();

    /* 
     * Parse stream, calling corresponded callback functions of
     * eventProcessor when needed.
     * 
     * If not empty, 'filename' is used in error reporting.
     */
    void parse(std::istream& is, TraceEventProcessor& eventProcessor,
        const char* filename = "");

    class Impl;
private:
    Impl *impl;
};

#endif /* TRACE_PARSER_H */
