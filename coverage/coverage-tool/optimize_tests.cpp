#include "optimize_tests.hh"

#include "trace.hh"

#include "trace_modifier.hh"

#include <algorithm>
#include <functional>
#include <iterator>

#include <list>

#include <cassert>

using namespace std;


/* 
 * Information about test trace with which we will operate during
 * optimization procedure.
 */
struct TestTraceInfo
{
    TestTraceInfo(void): test(NULL) {}

    Trace trace;
    double weight;
    /* Correspondence with test */
    const TestCoverageDesc* test;
    /* Load information about test. */
    void loadTest(const TestCoverageDesc& test);
};

/*
 * Information about test trace set.
 */
struct TestTraceSetInfo
{
    Trace trace;
    double weight;

    TestTraceSetInfo();
    void addTestTrace(const TestTraceInfo& trace);
    
    /* Auxiliary parameters of trace set. */
    int linesTotalHit;
    
    void updateStat(void);
};

/* Add coverage of traces. Result will be stored in 'trace'.*/
static void traceAdd(Trace& trace, const Trace& traceAdded);

/* Common algorithm for optimization */
class TraceSetOptimizer
{
public:
    /* 
     * Load given traces list into optimizer and setup some
     * vector of traces with maximum coverage.
     */
    TraceSetOptimizer(list<TestTraceInfo>& traces);
    /* Return vector of traces. */
    const vector<list<TestTraceInfo>::const_iterator>& get(void) const;
    /* Optimize current vector of traces*/
    void optimize(void);
private:
    list<TestTraceInfo>& traces;
    int linesTotalHit;
    /* Setup worst traces set and compute corresponded values. */
    void setWorst();

    /* Current optimal values */
    vector<list<TestTraceInfo>::const_iterator> optTraces;
    double optWeight;
    
    /* Currently checked traces. */
    vector<list<TestTraceInfo>::const_iterator> currentTraces;
    double currentWeight;
    /* 
     * Stack of traces which are result of joining.
     * First element in that stack - empty trace.
     */
    list<Trace> setTraceStack;
    
    /* 
     * Compare current set(top in the stack) of traces with optimal values.
     * If need, update optimal values.
     * 
     * Return true if current trace set may be completed with futher
     * traces to be optimal set, false otherwise.
     */
    bool check(void);
    /* Modifications of current trace set. */
    /* Add trace to the current set. */
    void setAddTrace(const list<TestTraceInfo>::const_iterator& traceIter);
    /* Remove last trace from the current set. Return element removed. */
    list<TestTraceInfo>::const_iterator setRemoveTrace(void);
};

/* 
 * Remove information about branches and functons from coverage trace
 * for simplify its processing.
 * Also decrease all positive line counters to 1.
 */

static void traceClearExceptLines(Trace& trace);

/* 
 * Fill traces according to tests.
 * 
 * 'tracesWeightless' list will contain traces for tests with weight = 0,
 * 'traces' list will contain traces for all other tests.
 */
static void fillTraces(list<TestTraceInfo>& traces,
    list<TestTraceInfo>& tracesWeightless,
    const vector<TestCoverageDesc>& tests)
{
    vector<TestCoverageDesc>::const_iterator iter = tests.begin(),
        iterEnd = tests.end();
    for(; iter != iterEnd; ++iter)
    {
        assert(iter->weight >= 0);
        if(iter->weight != 0)
        {
            traces.push_back(TestTraceInfo());
            traces.back().loadTest(*iter);
        }
        else
        {
            tracesWeightless.push_back(TestTraceInfo());
            tracesWeightless.back().loadTest(*iter);
        }
    }
}

/* Clear all lines in trace which are also exist in mask. */
static void traceClearLinesMasked(Trace& trace, const Trace& mask);


/* Helpers for transform algorithms */
const TestCoverageDesc* transformTestTrace(const TestTraceInfo& testTrace)
    {return testTrace.test; }
const TestCoverageDesc* transformTestTraceIter(
    const list<TestTraceInfo>::const_iterator& testTraceIter)
    {return testTraceIter->test; }
const TestCoverageDesc& transformTest(const TestCoverageDesc* test)
    {return *test;}

/* Main procedure */
void optimizeTests(const std::vector<TestCoverageDesc>& tests,
    std::vector<TestCoverageDesc>& optTests)
{
    if(tests.empty()) return;/* Empty tests set is already optimized. */
    
    /* List of traces, corresponded to the tests. */
    list<TestTraceInfo> traces;
    /* Traces with weight = 0. */
    list<TestTraceInfo> tracesWeightless;
    
    fillTraces(traces, tracesWeightless, tests);
    
    if(!tracesWeightless.empty())
    {
        /* 
         * Clear all lines in 'traces' which are covered in 'tracesWeightless'.
         */
        
        /* Combination of all weightless traces*/
        Trace traceWeightless;
        
        list<TestTraceInfo>::const_iterator
            witer = tracesWeightless.begin(),
            witerEnd = tracesWeightless.end();
        
        for(;witer != witerEnd; ++witer)
        {
            traceAdd(traceWeightless, witer->trace);
        }
        
        list<TestTraceInfo>::iterator
            iter = traces.begin(),
            iterEnd = traces.end();

        for(;iter != iterEnd; ++iter)
        {
            traceClearLinesMasked(iter->trace, traceWeightless);
        }
    }
    
    /*TraceSetOptimizer optimizer(traces);
    
    optimizer.optimize();
    
    const vector<list<TestTraceInfo>::const_iterator>& optVector = optimizer.get();*/
    //debug
    vector<list<TestTraceInfo>::const_iterator> optVector;
    optVector.push_back(traces.begin());
    
    /* 
     * Optimal vector of tests, may be misordered.
     */
    vector<const TestCoverageDesc*> optTestsP;

    /*transform(tracesWeightless.begin(), tracesWeightless.end(),
        back_inserter(optTestsP),
        transformTestTrace);*/
    
    transform(optVector.begin(), optVector.end(),
        back_inserter(optTestsP),
        transformTestTraceIter);
    
    /* Sort it. */
    /*sort(optTestsP.begin(), optTestsP.end());*/
    /* Convert pointers to elements. */
    transform(optTestsP.begin(), optTestsP.end(),
        back_inserter(optTests),
        transformTest);
}

/****************** TestTraceInfo implementation **********************/
void TestTraceInfo::loadTest(const TestCoverageDesc& test)
{
    trace.read(test.traceFile.c_str());
    traceClearExceptLines(trace);
    weight = test.weight;
}

/******************** traceAdd() **************************************/
class TraceAddModifier: public Trace::Modifier
{
public:
    bool onFileGroupStart(
        const std::map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& group)
    {
        currentGroup = &group;
        return false;
    }
    void onFileGroupEndNew(Trace& traceModified)
    {
        pair<map<Trace::FileGroupID, Trace::FileGroupInfo*>::iterator, bool>
            iterNew = traceModified.fileGroups.insert(*currentGroup);
        iterNew.first->second = new Trace::FileGroupInfo(*iterNew.first->second);
    }
    
    Trace::FileInfo::Modifier* onSourceStart(
        const std::map<std::string, Trace::FileInfo>::value_type& source)
    {
        currentFile = &source;
        return &fileAddModifier;
    }
    void onSourceEndNew(Trace::FileGroupInfo& groupModified)
    {
        groupModified.files.insert(*currentFile);
    }
private:
    const std::map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type
        *currentGroup;
    const std::map<string, Trace::FileInfo>::value_type *currentFile;
    
    class FileAddModifier: public Trace::FileInfo::Modifier
    {
    public:
        bool onLine(const std::map<int, counter_t>::value_type& line)
        {
            if(line.second <= 0) return true;
            currentLine = &line;
            return false;
        }

        void modifyLineCounter(counter_t counter, counter_t& counterModified)
        {
            counterModified += counter;
        }

        void newLine(Trace::FileInfo& fileModified)
        {
            fileModified.lines.insert(*currentLine);
        }
    private:
        const std::map<int, counter_t>::value_type* currentLine;
    }fileAddModifier;
};

void traceAdd(Trace& trace, const Trace& traceAdded)
{
    TraceAddModifier modifier;
    modifyTrace(traceAdded, modifier, trace);
}

/*********************** Optimizer implementation *********************/

TraceSetOptimizer::TraceSetOptimizer(list<TestTraceInfo>& traces):
    traces(traces)
{
    assert(!traces.empty());
    setWorst();
}

const vector<list<TestTraceInfo>::const_iterator>&
TraceSetOptimizer::get(void) const
{
    return optTraces;
}

void TraceSetOptimizer::optimize(void)
{
    list<TestTraceInfo>::const_iterator endIter = traces.end();

    /* Initialize all data needed for search */
    setTraceStack.push_back(Trace());
    currentWeight = 0;
    currentTraces.clear();
    /* Setup first trace sequence for check. */
    setAddTrace(traces.begin());
    /* Iterate throw possible traces set. */
    while(1)
    {
        if(check())
        {
            /* Trace set may be optimal with additional traces. */
            list<TestTraceInfo>::const_iterator nextTraceIter
                = currentTraces.back();
            nextTraceIter++;
            
            if(nextTraceIter != endIter)
            {
                /* Append trace */
                setAddTrace(nextTraceIter);
                continue;
            }
        }
        else
        {
            /* 
             * Trace set cannot be be optimal with additional traces.
             * Or there is no additional traces.
             * 
             * Drop last element.
             */
            setRemoveTrace();
            
            if(currentTraces.empty()) break;
            
            /* 
             * Advance current last element.
             * 
             * After removing it is always possible.
             */
            setAddTrace(++setRemoveTrace());
        }
    }
}

void TraceSetOptimizer::setWorst()
{
    Trace traceCombined;
    optWeight = 0;
    optTraces.clear();
    
    list<TestTraceInfo>::const_iterator traceIter = traces.begin(),
        traceIterEnd = traces.end();
    
    for(; traceIter != traceIterEnd; ++traceIter)
    {
        optTraces.push_back(traceIter);
        optWeight = traceIter->weight;
        traceAdd(traceCombined, traceIter->trace);
    }
}

bool TraceSetOptimizer::check(void)
{
    if(currentWeight >= optWeight)
    {
        return false;
    }
    
    int currentLinesTotalHit = setTraceStack.back().linesTotalHit();
    if(currentLinesTotalHit >= linesTotalHit)
    {
        /* Update optimal set */
        optTraces = currentTraces;
        optWeight = currentWeight;
        /* Set already optimal - needn't to be extended. */
        return false;
    }
    
    return true;
}

void TraceSetOptimizer::setAddTrace(
    const list<TestTraceInfo>::const_iterator& traceIter)
{
    currentTraces.push_back(traceIter);
    currentWeight+= traceIter->weight;
    setTraceStack.push_back(setTraceStack.back());
    traceAdd(setTraceStack.back(), traceIter->trace);
}

list<TestTraceInfo>::const_iterator TraceSetOptimizer::setRemoveTrace()
{
    currentWeight-= currentTraces.back()->weight;
    setTraceStack.pop_back();
    
    list<TestTraceInfo>::const_iterator result = currentTraces.back();
    
    currentTraces.pop_back();
    return result;
}

/********************** Trace cleaner *********************************/
class TraceCleaner
{
public:
    void operator()(map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& group)
    {
        for_each(group.second->files.begin(), group.second->files.end(), *this);
    }
    
    void operator()(map<string, Trace::FileInfo>::value_type& file)
    {
        file.second.functions.clear();
        file.second.branches.clear();
        for_each(file.second.lines.begin(), file.second.lines.end(), *this);
    }

    void operator()(map<int, counter_t>::value_type& line)
    {
        if(line.second > 1) line.second = 1;
    }
};


void traceClearExceptLines(Trace& trace)
{
    for_each(trace.fileGroups.begin(), trace.fileGroups.end(), TraceCleaner());
}

/************************Clear masked lines from trace *****************/

class TraceClearLines
{
public:
    TraceClearLines(Trace& traceModified): traceModified(traceModified) {}

    void operator()(const std::map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& group)
    {
        std::map<Trace::FileGroupID, Trace::FileGroupInfo*>::iterator iter =
            traceModified.fileGroups.find(group.first);
        if(iter != traceModified.fileGroups.end())
        {
            GroupClearLines::clear(*iter->second, *group.second);
        }
    }
    
private:
    class FileClearLines
    {
    public:
        FileClearLines(Trace::FileInfo& fileModified):
            fileModified(fileModified) {}

        static void clear(Trace::FileInfo& fileModified, const Trace::FileInfo& mask)
        {
            for_each(mask.lines.begin(), mask.lines.end(), FileClearLines(fileModified));
        }

        void operator()(const std::map<int, counter_t>::value_type& line)
        {
            if(line.second > 0)
            {
                std::map<int, counter_t>::iterator iter =
                    fileModified.lines.find(line.first);
                if(iter != fileModified.lines.end())
                {
                    fileModified.lines.erase(iter);
                }
            }
        }
    private:
        Trace::FileInfo& fileModified;
    };

    class GroupClearLines
    {
    public:
        GroupClearLines(Trace::FileGroupInfo& groupModified):
            groupModified(groupModified) {}

        static void clear(Trace::FileGroupInfo& groupModified,
            const Trace::FileGroupInfo& mask)
        {
            for_each(mask.files.begin(), mask.files.end(), GroupClearLines(groupModified));
        }

        void operator()(const std::map<string, Trace::FileInfo>::value_type& file)
        {
            std::map<string, Trace::FileInfo>::iterator iter =
                groupModified.files.find(file.first);
            if(iter != groupModified.files.end())
            {
                FileClearLines::clear(iter->second, file.second);
            }
        }
    private:
        Trace::FileGroupInfo& groupModified;
    };
    Trace& traceModified;
};

void traceClearLinesMasked(Trace& trace, const Trace& mask)
{
    for_each(mask.fileGroups.begin(), mask.fileGroups.end(), TraceClearLines(trace));
}
