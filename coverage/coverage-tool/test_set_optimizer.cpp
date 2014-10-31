// test_set_optimizer.cpp - test set optimization algorithm.

//      
//      Copyright (C) 2012, Institute for System Programming
//                          of the Russian Academy of Sciences (ISPRAS)
//      Author:
//          Andrey Tsyvarev <tsyvarev@ispras.ru>
//      
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.

#include "test_set_optimizer.hh"

#include <algorithm>
#include <functional>
#include <iterator>

#include <list>

#include <cassert>

using namespace std;

typedef Trace::counter_t counter_t;

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
    
    /* Auxiliary information */
    
    /* Groups which are included into trace. */
    vector<bool> groups;
    /* 
     * Special weight - weight per number of groups included.
     * 
     * It will be used for sorting.
     */
    double sWeight;
     /*
      * Return number of groups covered by the trace.
      */
     int groupsCovered(void) const;
     
     /* 
      * 'swap' overload for removing non-last element from array
      * and for sorting algorithm.
      */
     friend void swap(TestTraceInfo& traceInfo, TestTraceInfo& traceInfo1);
     
     static bool sWeightLess(const TestTraceInfo& traceInfo,
        const TestTraceInfo& traceInfo1)
    { return traceInfo.sWeight < traceInfo1.sWeight; }
};


/******* Some useful operations for vector of trace informations ******/

/* 
 * Remove element with given index from array.
 * 
 * If element is not last, it will be swapped with last.
 */
void testTraceArrayRemoveAt(vector<TestTraceInfo>& traces, int index);
    
/* 
 * Remove element with given index from array as it will be included
 * into optimal set.
 * 
 * Beside element removing, substract coverage of trace in that
 * element from all other traces.
 */
void testTraceArrayRemoveAtIncluded(vector<TestTraceInfo>& traces, int index);

/*************** Object used for iterate trace sets********************/
class TraceSetIterator
{
public:
    TraceSetIterator(const vector<TestTraceInfo>& testTraces);

    const vector<TestTraceInfo>& traces;
    
    /* Setup first trace set, which contained only first trace. */
    void setFirst();
    /* 
     * Check, whether set can be continued with futher traces to be 
     * optimal one.
     * 
     * Return true if so, false otherwise.
     * 
     * If current set is better than current optimal one, update optimal.
     */
    bool setCheck();
    
    /* 
     * Add next trace to the trace set.
     * 
     * Return true on success and false is last trace in the set is also
     * last in traces array.
     */
    bool setAddTrace();
    
    /*
     * Drop last trace in the set and move previous one to the next.
     * 
     * Return true on success and false if dropped trace was last one.
     */
    bool setDropAndNext();
    
    /* Optimal set(indecies in traces array). */
    vector<int> optSet;
    /* Optimal weight. */
    double optWeight;

    /* Current set(indecies in traces array). */
    vector<int> currentSet;
    /* Current weight. */
    double currentWeight;

    /* Groups coverage state. */
    struct GroupsCoverage
    {
        vector<bool> covered;
        /* Number of currently uncovered groups. */
        int groupsUncovered;
    };
    
    vector<GroupsCoverage> coverageStack;
    
    void setAddTrace(int index);
    void setDropTrace(void);
};

/************************** Lines modificators for trace **************/
/* 
 * Remove information about branches and functons from coverage trace
 * for simplify its processing.
 */
static void traceClearExceptLines(Trace& trace);

/* Clear all lines in trace which are also covered in mask. */
static void traceClearLinesMasked(Trace& trace, const Trace& mask);
/* Clear all lines in trace which are not covered in mask. */
static void traceClearLinesNotMasked(Trace& trace, const Trace& mask);

/****************** TestTraceInfo implementation **********************/
void TestTraceInfo::loadTest(const TestCoverageDesc& test)
{
    trace.read(test.traceFile.c_str());
    trace.groupFiles();
    traceClearExceptLines(trace);
    weight = test.weight;
    this->test = &test;
}

int TestTraceInfo::groupsCovered(void) const
{
    int counter = 0;
    for(int i = 0; i < (int)groups.size(); i++)
    {
        if(groups[i]) counter++;
    }
    
    return counter;
}

void swap(TestTraceInfo& traceInfo, TestTraceInfo& traceInfo1)
{
    swap(traceInfo.test,             traceInfo1.test);
    swap(traceInfo.trace.fileGroups, traceInfo1.trace.fileGroups);
    swap(traceInfo.weight,           traceInfo1.weight);
    swap(traceInfo.sWeight,          traceInfo1.sWeight);
    swap(traceInfo.groups,           traceInfo1.groups);
}

/********** Implementation of functions works with trace array ********/

void testTraceArrayRemoveAt(vector<TestTraceInfo>& traces, int index)
{
    if(index != (int)traces.size() - 1)
    {
        swap(traces[index], traces.back());
    }
    
    traces.pop_back();
}
    
void testTraceArrayRemoveAtIncluded(vector<TestTraceInfo>& traces, int index)
{
    for(int i = 0; i < (int)traces.size(); i++)
    {
        if(i == index) continue;
        
        traceClearLinesMasked(traces[i].trace, traces[index].trace);
    }
    
    testTraceArrayRemoveAt(traces, index);
}


/*********************** Optimizer implementation *********************/
TestSetOptimizer::TestSetOptimizer(const vector<TestCoverageDesc>& tests)
    : tests(tests) {}

/* Line identificator in the trace. */
struct TraceLineID
{
    Trace::FileGroupID groupID;
    int line;
    /* filename is same as one in groupID. */
};

/* 
 * Search any line which is covered in the trace.
 * 
 * If found, set lineID to identificator of that line and return true.
 * If trace doesn't cover any line, return false;
 */
bool traceGetLineCovered(const Trace& trace, TraceLineID& lineID)
{
    map<Trace::FileGroupID, Trace::FileGroupInfo*>::const_iterator
        iterGroup = trace.fileGroups.begin(), iterGroupEnd = trace.fileGroups.end();
    for(; iterGroup != iterGroupEnd; ++iterGroup)
    {
        /* Assume only one file in group. */
        map<string, Trace::FileInfo>::const_iterator iterFile =
            iterGroup->second->files.begin();
        
        assert(iterFile != iterGroup->second->files.end());
        assert(iterFile->first == iterGroup->first.filename);
        
        map<int, counter_t>::const_iterator
            iterLine = iterFile->second.lines.begin(),
            iterLineEnd = iterFile->second.lines.end();

        for(; iterLine != iterLineEnd; ++iterLine)
        {
            if(iterLine->second > 0)
            {
                lineID.groupID = iterGroup->first;
                lineID.line = iterLine->first;
                return true;
            }
        }
    }
    return false;
}

bool traceHasLine(const Trace& trace, const TraceLineID& lineID)
{
    map<Trace::FileGroupID, Trace::FileGroupInfo*>::const_iterator
        iterGroup = trace.fileGroups.find(lineID.groupID);
    if(iterGroup == trace.fileGroups.end()) return false;
    
    /* Assume only one file in group. */
    map<string, Trace::FileInfo>::const_iterator iterFile =
        iterGroup->second->files.begin();
    
    assert(iterFile != iterGroup->second->files.end());
    assert(iterFile->first == iterGroup->first.filename);

    map<int, counter_t>::const_iterator
        iterLine = iterFile->second.lines.find(lineID.line);
    if(iterLine == iterFile->second.lines.end()) return false;
    
    return iterLine->second > 0;
}

/*************************** Optimize function ************************/
const vector<TestCoverageDesc>& TestSetOptimizer::optimize(bool verbose)
{
#define info(msg) if(verbose) cerr << msg << endl

    /* Setup traces info array. */
    vector<TestTraceInfo> testTraces;

    int nTests = tests.size();
    
    testTraces.resize(nTests);
    
    for(int i = 0; i < nTests; i++)
    {
        TestTraceInfo& traceInfo = testTraces[i];

        traceInfo.loadTest(tests[i]);
        
        info("Trace loaded from file " << traceInfo.test->traceFile
            << ", " << traceInfo.trace.linesTotalHit()
            << " lines covered.");
    }

    /* 
     * Tests which are included into result without tests set comparision
     * for some reasons.
     */
    vector<const TestCoverageDesc*> includedTests;
    
    /* Filter 0-weight traces, which are always included into result. */
    for(int i = 0; i < (int)testTraces.size(); i++)
    {
        TestTraceInfo& traceInfo = testTraces[i];
        if(traceInfo.weight == 0)
        {
            info("Trace " << traceInfo.test->traceFile
             << " has zero weight. It is unconditionally included into "
             << "optimal set.");
            
            includedTests.push_back(traceInfo.test);
            testTraceArrayRemoveAtIncluded(testTraces, i);
            /* New element is set for given index. */
            --i;
        }
    }
    
    /* 
     * Combine covered lines into groups.
     * For each trace whole group is either included or not.
     */

    int currentTrace = 0;
    while(currentTrace < (int)testTraces.size())
    {
        TestTraceInfo& traceInfo = testTraces[currentTrace];
        /* Pick any line, covered in the trace. */
        TraceLineID lineID;
        
        if(!traceGetLineCovered(traceInfo.trace, lineID))
        {
            /* Trace coverage is empty. */
            
            //debug
            //cerr << "Trace " << currentTrace << " become empty." << endl;

            if(traceInfo.groupsCovered() == 0)
            {
                /* 
                 * Trace cover no group. It cannot appear in optimal set,
                 * so forgot it.
                 */
                info("Trace " << traceInfo.test->traceFile
                    << " has zero coverage. Discard it.");
                
                testTraceArrayRemoveAt(testTraces, currentTrace);
                /* New element is set for given index. */
            }
            else
            {
                /* All groups in the trace are defined, and it may be
                 * included into optimal set.
                 * 
                 * Move to the next trace.
                 */
                currentTrace++;
            }
            continue;
        }
        //debug
        //cerr << "Find group for trace " << currentTrace << "." << endl;
        /* Coverage corresponded to the group */
        Trace groupTrace = traceInfo.trace;
        traceInfo.groups.push_back(true);
        /* Whether group is covered by all traces. */
        bool allCover = true;
        /* Whether group is covered only by the current trace. */
        bool oneCover = true;
        
        /* 
         * Restrict group coverage corresponding to other traces.
         * 
         * Also set group coverage for other traces.
         */
        
        /* Previous traces cannot contain given group. */
        for(int j = 0; j < currentTrace; j++)
        {
            TestTraceInfo& traceInfo1 = testTraces[j];
            traceInfo1.groups.push_back(false);
            allCover = false;
        }
        /* Further traces may contain given group. */
        for(int j = currentTrace + 1; j < (int)testTraces.size(); j++)
        {
            TestTraceInfo& traceInfo1 = testTraces[j];
            if(traceHasLine(traceInfo1.trace, lineID))
            {
                //cerr << "Line " << lineID.groupID << ": " << lineID.line << " found in trace " << j << "." << endl;
                traceInfo1.groups.push_back(true);
                oneCover = false;
                
                traceClearLinesNotMasked(groupTrace, traceInfo1.trace);
            }
            else
            {
                //cerr << "Line " << lineID.groupID << ": " << lineID.line << " is not found in trace " << j << "." << endl;
                traceInfo1.groups.push_back(false);
                allCover = false;
                
                traceClearLinesMasked(groupTrace, traceInfo1.trace);
            }
        }

        //cerr << "Group contains " << groupTrace.linesTotalHit() << " lines." << endl;

        /* 
         * Remove coverage, corresponded to group, from traces
         * (including current one) which include it.
         */
        for(int j = currentTrace; j < (int)testTraces.size(); j++)
        {
            TestTraceInfo& traceInfo1 = testTraces[j];
            if(traceInfo1.groups.back())
            {
                //cerr << "Remove group coverage from trace " << j << "." << endl;
                traceClearLinesMasked(traceInfo1.trace, groupTrace);
            }
        }

        if(oneCover)
        {
            info("Source line " << lineID.groupID << ": " << lineID.line
                << "\nis covered only by trace " << traceInfo.test->traceFile
                << ".\nIt should be included into optimal set.");
            /* 
             * Group is covered only by one trace. This trace should
             * be included into optimal set.
             */
             includedTests.push_back(traceInfo.test);
             testTraceArrayRemoveAtIncluded(testTraces, currentTrace);
             
            /* Clear indicators for given group from all traces. */
            for(int j = 0; j < (int)testTraces.size(); j++)
            {
                TestTraceInfo& traceInfo1 = testTraces[j];
                traceInfo1.groups.pop_back();
            }
            /* New element is set for given index. */
            continue;
        }


        if(allCover)
        {
            /* 
             * Group is covered by all traces.
             */

            //TODO
        }
    }
    
    if(testTraces.empty())
    {
        info("Non-zero weights do not affect on optimal tests set.");

        sort(includedTests.begin(), includedTests.end());
        
        vector<TestCoverageDesc> optTests;
        optTests.reserve(includedTests.size());
        
        for(int i = 0; i < (int)includedTests.size(); i++)
        {
            optTests.push_back(*includedTests[i]);
        }
        
        swap(tests, optTests);
        
        return tests;

    }
    
    /* 
     * Now all groups are determined and some optimizations take a place.
     * 
     * Sort traces by special weights.
     */
    
    info("Search optimal set from tests:");
    for(int i = 0; i < (int)testTraces.size(); i++)
    {
        TestTraceInfo& traceInfo = testTraces[i];
        traceInfo.sWeight = traceInfo.weight / traceInfo.groupsCovered();
        
        info(traceInfo.test->traceFile);
    }
    info("---------------------------------");
    sort(testTraces.begin(), testTraces.end(), TestTraceInfo::sWeightLess);
    
    /* Iterate through trace sets. */
    TraceSetIterator traceSetIterator(testTraces);
    
    traceSetIterator.setFirst();
    while(1)
    {
        if(traceSetIterator.setCheck())
        {
            if(traceSetIterator.setAddTrace())
            {
                continue;
            }
        }
        if(!traceSetIterator.setDropAndNext()) break;
    }
    /* Now traceSetIterator.optSet contains optimal trace. */

    vector<const TestCoverageDesc*> optTestsP = includedTests;
    
    optTestsP.reserve(optTestsP.size() + traceSetIterator.optSet.size());
    for(int i = 0; i < (int)traceSetIterator.optSet.size(); i++)
    {
        optTestsP.push_back(testTraces[traceSetIterator.optSet[i]].test);
    }
    
    sort(optTestsP.begin(), optTestsP.end());
    
    vector<TestCoverageDesc> optTests;
    optTests.reserve(optTestsP.size());
    
    for(int i = 0; i < (int)optTestsP.size(); i++)
    {
        optTests.push_back(*optTestsP[i]);
    }
    
    swap(tests, optTests);
    
    return tests;
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
    }
};

void traceClearExceptLines(Trace& trace)
{
    for_each(trace.fileGroups.begin(), trace.fileGroups.end(), TraceCleaner());
}

/************************Clear masked lines from trace *****************/

class TraceClearLinesMasked
{
public:
    TraceClearLinesMasked(Trace& traceModified) {modified.trace = &traceModified;}

    void operator()(const std::map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& group)
    {
        std::map<Trace::FileGroupID, Trace::FileGroupInfo*>::iterator iter =
            modified.trace->fileGroups.find(group.first);
        if(iter != modified.trace->fileGroups.end())
        {
            for_each(group.second->files.begin(),
                        group.second->files.end(),
                        TraceClearLinesMasked(*iter->second));
        }
    }
    
    void operator()(const std::map<string, Trace::FileInfo>::value_type& file)
    {
        std::map<string, Trace::FileInfo>::iterator iter =
            modified.group->files.find(file.first);
        if(iter != modified.group->files.end())
        {
            for_each(file.second.lines.begin(),
                        file.second.lines.end(),
                        TraceClearLinesMasked(iter->second));
        }
    }

    void operator()(const std::map<int, counter_t>::value_type& line)
    {
        if(line.second > 0)
        {
            std::map<int, counter_t>::iterator iter =
                modified.file->lines.find(line.first);
            if(iter != modified.file->lines.end())
            {
                modified.file->lines.erase(iter);
            }
        }
    }

    
private:
    union
    {
        Trace* trace;
        Trace::FileGroupInfo* group;
        Trace::FileInfo* file;
    }modified;
    
    TraceClearLinesMasked(Trace::FileGroupInfo& groupModified)
        { modified.group = &groupModified; }
    
    TraceClearLinesMasked(Trace::FileInfo& fileModified)
        { modified.file = &fileModified; }
};

void traceClearLinesMasked(Trace& trace, const Trace& mask)
{
    for_each(mask.fileGroups.begin(), mask.fileGroups.end(), TraceClearLinesMasked(trace));
}

/******************** Clear not masked lines from trace ***************/
class TraceClearLinesNotMasked
{
public:
    TraceClearLinesNotMasked(const Trace& traceMask)
        {mask.trace = &traceMask;}
    
    void clear(Trace& trace)
    {
        std::map<Trace::FileGroupID, Trace::FileGroupInfo*>::iterator
            iter = trace.fileGroups.begin(),
            iterEnd = trace.fileGroups.end(),
            iterNext;
        
        for(; (iter != iterEnd); iter = iterNext)
        {
            iterNext = iter;
            ++iterNext;
            
            std::map<Trace::FileGroupID, Trace::FileGroupInfo*>::const_iterator
                maskIter = mask.trace->fileGroups.find(iter->first);
            
            if(maskIter != mask.trace->fileGroups.end())
            {
                TraceClearLinesNotMasked(*maskIter->second).clear(*iter->second);
            }
            else
            {
                trace.fileGroups.erase(iter);
            }
        }
    }

private:
    TraceClearLinesNotMasked(const Trace::FileGroupInfo& groupMask)
        {mask.group = &groupMask;}

    void clear(Trace::FileGroupInfo& group)
    {
        std::map<string, Trace::FileInfo>::iterator
            iter = group.files.begin(),
            iterEnd = group.files.end(),
            iterNext;
        
        for(; (iter != iterEnd); iter = iterNext)
        {
            iterNext = iter;
            ++iterNext;
            
            std::map<string, Trace::FileInfo>::const_iterator
                maskIter = mask.group->files.find(iter->first);
            
            if(maskIter != mask.group->files.end())
            {
                TraceClearLinesNotMasked(maskIter->second).clear(iter->second);
            }
            else
            {
                group.files.erase(iter);
            }
        }
    }

    TraceClearLinesNotMasked(const Trace::FileInfo& fileMask)
        {mask.file = &fileMask;}

    void clear(Trace::FileInfo& file)
    {
        std::map<int, counter_t>::iterator
            iter = file.lines.begin(),
            iterEnd = file.lines.end(),
            iterNext;
        
        for(; (iter != iterEnd); iter = iterNext)
        {
            iterNext = iter;
            ++iterNext;

            std::map<int, counter_t>::const_iterator
                maskIter = mask.file->lines.find(iter->first);
            
            if((maskIter == mask.file->lines.end())
                || (maskIter->second == 0))
            {
                file.lines.erase(iter);
            }
        }
    }

    union
    {
        const Trace* trace;
        const Trace::FileGroupInfo* group;
        const Trace::FileInfo* file;
    }mask;
};


void traceClearLinesNotMasked(Trace& trace, const Trace& mask)
{
    TraceClearLinesNotMasked(mask).clear(trace);
}

/********************TraceSetIterator implementation*******************/
TraceSetIterator::TraceSetIterator(const vector<TestTraceInfo>& traces)
    : traces(traces), optWeight(0), currentWeight(0)
{
    assert(!traces.empty());
    
    optSet.reserve(traces.size());
    coverageStack.reserve(traces.size() + 1);
    
    for(int i = 0; i < (int)traces.size(); i++)
    {
        optSet.push_back(i);
        optWeight += traces[i].weight;
    }

    coverageStack.push_back(GroupsCoverage());
    
    const TestTraceInfo& firstTraceInfo = traces[0];
    GroupsCoverage& firstCoverage = coverageStack[0];

    firstCoverage.covered.resize(firstTraceInfo.groups.size());
    firstCoverage.groupsUncovered = firstTraceInfo.groups.size();
}

/* Setup first trace set, which contained only first trace. */
void TraceSetIterator::setFirst()
{
    setAddTrace(0);
}
/* 
 * Check, whether set can be continued with futher traces to be 
 * optimal one.
 * 
 * Return true if so, false otherwise.
 * 
 * If current set is better than current optimal one, update optimal.
 */
bool TraceSetIterator::setCheck()
{
    if(optWeight <=  currentWeight) return false;
    
    GroupsCoverage& coverage = coverageStack.back();
    if(coverage.groupsUncovered == 0)
    {
        optSet = currentSet;
        optWeight = currentWeight;
        return false;
    }
    
    int nextTraceIndex = currentSet.back() + 1;
    if(nextTraceIndex >= (int)traces.size()) return false;
    
    const TestTraceInfo& nextTraceInfo = traces[nextTraceIndex];
    if((optWeight - currentWeight) / coverage.groupsUncovered < nextTraceInfo.sWeight)
    {
        return false;
    }
    
    return true;
}

/* 
 * Add next trace to the trace set.
 * 
 * Return true on success and false is last trace in the set is also
 * last in traces array.
 */
bool TraceSetIterator::setAddTrace()
{
    int nextTraceIndex = currentSet.back() + 1;
    if(nextTraceIndex >= (int)traces.size()) return false;

    setAddTrace(nextTraceIndex);
    
    return true;
}

/*
 * Drop last trace in the set and move previous one to the next.
 * 
 * Return true on success and false if dropped trace was last one.
 */
bool TraceSetIterator::setDropAndNext()
{
    setDropTrace();
    
    if(currentSet.empty()) return false;
    
    int nextTraceIndex = currentSet.back() + 1;
    
    setDropTrace();
    setAddTrace(nextTraceIndex);
    
    return true;
}

void TraceSetIterator::setAddTrace(int index)
{
    currentSet.push_back(index);
    coverageStack.push_back(GroupsCoverage());

    const TestTraceInfo& traceInfo = traces[index];
    GroupsCoverage& coverage = coverageStack.back();

    currentWeight += traceInfo.weight;

    coverage = coverageStack[coverageStack.size() - 2];
    
    for(int i = 0; i < (int)coverage.covered.size(); i++)
    {
        if(!coverage.covered[i] && traceInfo.groups[i])
        {
            coverage.covered[i] = true;
            --coverage.groupsUncovered;
        }
    }
}

void TraceSetIterator::setDropTrace(void)
{
    const TestTraceInfo& traceInfo = traces[currentSet.back()];
    
    currentWeight -= traceInfo.weight;
    currentSet.pop_back();
    
    coverageStack.pop_back();
}
