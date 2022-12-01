// trace_simple.cpp - implementation of 'TraceSimple' class methods.

//
//      Copyright (C) 2022, Institute for System Programming
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


#include "trace_simple.hh"
#include "trace_parser.hh"

#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>
#include <cassert>

#include <algorithm>

using namespace std;
typedef TraceSimple::counter_t counter_t;

/*
 * Report about error in trace and throw exception.
 *
 * 'msg' is used only for insert into stream.
 * (That is, it may contain '<<' operators).
 */
#define trace_error(msg) \
cerr << traceLine << ": " << msg << endl; \
throw runtime_error("Incorrect trace file");

/*
 * Update counter with the new value.
 * Counters are added only if both are non-negative.
 */
static inline void updateCounter(counter_t& currentValue, counter_t newValue)
{
    if (newValue >= 0)
    {
        if (currentValue < 0)
        {
            currentValue = newValue;
        }
        else
        {
            currentValue += newValue;
        }
    }
}

/********************* BranchID methods ****************************/
bool TraceSimple::BranchID::operator<(const BranchID& branchID) const
{
    if(line < branchID.line) return true;
    else if(line > branchID.line) return false;

    if(blockNumber < branchID.blockNumber) return true;
    else if(blockNumber > branchID.blockNumber) return false;

    if(branchNumber < branchID.branchNumber) return true;
    return false;
}

std::ostream& operator<<(std::ostream& os, const TraceSimple::BranchID& branchID)
{
    return os << "{" << branchID.line << ", " << branchID.blockNumber << ", "
        << branchID.branchNumber << "}";
}

/*********************** TraceSimple class methods **************************/

/* Calculate statistic */
class LinesTotalCollectorSimple
{
public:
    LinesTotalCollectorSimple(void): linesTotal(0) {}

    int linesTotal;

    void operator()(const map<string, TraceSimple::FileInfo>::value_type& file)
    {
        *this = for_each(file.second.lines.begin(), file.second.lines.end(), *this);
    }

    void operator()(const map<int, counter_t>::value_type& /*line*/)
    {
        linesTotal++;
    }

    void addTrace(const TraceSimple& trace)
    {
        *this = for_each(trace.files.begin(), trace.files.end(), *this);
    }
};

int TraceSimple::linesTotal(void) const
{
    LinesTotalCollectorSimple collector;
    collector.addTrace(*this);
    return collector.linesTotal;
}

class LinesTotalHitCollectorSimple
{
public:
    LinesTotalHitCollectorSimple(): linesTotalHit(0) {}

    int linesTotalHit;

    void operator()(const map<string, TraceSimple::FileInfo>::value_type& file)
    {
        *this = for_each(file.second.lines.begin(), file.second.lines.end(), *this);
    }

    void operator()(const map<int, counter_t>::value_type& line)
    {
        if(line.second > 0) linesTotalHit++;
    }

    void addTrace(const TraceSimple& trace)
    {
        *this = for_each(trace.files.begin(), trace.files.end(), *this);
    }
};

int TraceSimple::linesTotalHit(void) const
{
    LinesTotalHitCollectorSimple collector;
    collector.addTrace(*this);
    return collector.linesTotalHit;
}

class BranchesTotalCollectorSimple
{
public:
    BranchesTotalCollectorSimple(): branchesTotal(0) {}

    int branchesTotal;

    void operator()(const map<string, TraceSimple::FileInfo>::value_type& file)
    {
        *this = for_each(file.second.branches.begin(), file.second.branches.end(), *this);
    }

    void operator()(const map<TraceSimple::BranchID, counter_t>::value_type& /*branch*/)
    {
        branchesTotal++;
    }

    void addTrace(const TraceSimple& trace)
    {
        *this = for_each(trace.files.begin(), trace.files.end(), *this);
    }
};

int TraceSimple::branchesTotal(void) const
{
    BranchesTotalCollectorSimple collector;
    collector.addTrace(*this);
    return collector.branchesTotal;
}

class BranchesTotalHitCollectorSimple
{
public:
    BranchesTotalHitCollectorSimple(): branchesTotalHit(0) {}

    int branchesTotalHit;

    void operator()(const map<string, TraceSimple::FileInfo>::value_type& file)
    {
        *this = for_each(file.second.branches.begin(), file.second.branches.end(), *this);
    }

    void operator()(const map<TraceSimple::BranchID, counter_t>::value_type& branch)
    {
        if(branch.second > 0) branchesTotalHit++;
    }

    void addTrace(const TraceSimple& trace)
    {
        *this = for_each(trace.files.begin(), trace.files.end(), *this);
    }
};

int TraceSimple::branchesTotalHit(void) const
{
    BranchesTotalHitCollectorSimple collector;
    collector.addTrace(*this);
    return collector.branchesTotalHit;
}


class FunctionsTotalCollectorSimple
{
public:
    FunctionsTotalCollectorSimple(): functionsTotal(0) {}

    int functionsTotal;

    void operator()(const map<string, TraceSimple::FileInfo>::value_type& file)
    {
        *this = for_each(file.second.functions.begin(), file.second.functions.end(), *this);
    }

    void operator()(const map<string, TraceSimple::FuncInfo>::value_type& /*function*/)
    {
        functionsTotal++;
    }

    void addTrace(const TraceSimple& trace)
    {
        *this = for_each(trace.files.begin(), trace.files.end(), *this);
    }
};

int TraceSimple::functionsTotal(void) const
{
    FunctionsTotalCollectorSimple collector;
    collector.addTrace(*this);
    return collector.functionsTotal;
}

class FunctionsTotalHitCollectorSimple
{
public:
    FunctionsTotalHitCollectorSimple(): functionsTotalHit(0) {}

    int functionsTotalHit;

    void operator()(const map<string, TraceSimple::FileInfo>::value_type& file)
    {
        *this = for_each(file.second.functions.begin(), file.second.functions.end(), *this);
    }

    void operator()(const map<string, TraceSimple::FuncInfo>::value_type& function)
    {
        if(function.second.counter > 0) functionsTotalHit++;
    }

    void addTrace(const TraceSimple& trace)
    {
        *this = for_each(trace.files.begin(), trace.files.end(), *this);
    }
};

int TraceSimple::functionsTotalHit(void) const
{
    FunctionsTotalHitCollectorSimple collector;
    collector.addTrace(*this);
    return collector.functionsTotalHit;
}

/*
 * Group files.
 *
 * NOTE: Files are grouped only within test (usually there is only one
 * test with empty name).
 */
class FilesGrouper
{
public:
    void group(TraceSimple& trace)
    {
        this->trace = &trace;

        for_each(trace.fileGroups.begin(), trace.fileGroups.end(), *this);
    }

    void operator()(map<TraceSimple::FileGroupID, TraceSimple::FileGroupInfo*>::value_type& group)
    {
        currentGroup = &group;

        map<string, TraceSimple::FileInfo>& files = group.second->files;
        /*
         * First, move content of all non-source files into
         * another groups.
         */
        for_each(group.second->files.begin(), group.second->files.end(), *this);
        /*
         * Second, remove all non-source files.
         */
        map<string, TraceSimple::FileInfo>::iterator
            fileIter = files.begin(),
            fileIterEnd = files.end();
        while(fileIter != fileIterEnd)
        {
            if(fileIter->first != group.first.filename)
            {
                files.erase(fileIter++);
            }
            else
            {
                ++fileIter;
            }
        }
    }

    void operator()(map<string, TraceSimple::FileInfo>::value_type& file)
    {
        if(file.first == currentGroup->first.filename) return;/* Already grouped */

        /* It is needed to move file into another group. */
        TraceSimple::FileGroupID newGroupID;
        newGroupID.testName = currentGroup->first.testName;
        newGroupID.filename = file.first;

        groupIter iter = trace->fileGroups.find(newGroupID);
        if(iter != trace->fileGroups.end())
        {
            /* Merge file with same file in the existed group. */
            map<string, TraceSimple::FileInfo>::iterator fileIter =
                iter->second->files.find(file.first);
            /* Group always have file with name which coincide with group's one. */
            assert(fileIter != iter->second->files.end());

            CombineFile combineFile(fileIter->second);
            combineFile.addFile(file.second);
        }
        else
        {
            /* Need to create group to which file will be moved. */
            TraceSimple::FileGroupInfo* newGroup = new TraceSimple::FileGroupInfo();

            trace->fileGroups.insert(make_pair(newGroupID, newGroup));
            pair<map<string, TraceSimple::FileInfo>::iterator, bool> newFileIter =
                newGroup->files.insert(make_pair(newGroupID.filename, TraceSimple::FileInfo()));
            swapFiles(newFileIter.first->second, file.second);
        }
    }

private:
    TraceSimple* trace;

    typedef map<TraceSimple::FileGroupID, TraceSimple::FileGroupInfo*>::iterator groupIter;
    groupIter::value_type* currentGroup;

    /*
     * Swap content of files. Helper for move file from one group into another.
     */
    static void swapFiles(TraceSimple::FileInfo& file1, TraceSimple::FileInfo& file2)
    {
        swap(file1.lines, file2.lines);
        swap(file1.functions, file2.functions);
        swap(file1.branches, file2.branches);
    }

    /* Combine counters in files. */
    class CombineFile
    {
    public:
        CombineFile(TraceSimple::FileInfo& destFile) : destFile(destFile) {}
        void addFile(const TraceSimple::FileInfo& file)
        {
            for_each(file.lines.begin(), file.lines.end(), *this);
            for_each(file.branches.begin(), file.branches.end(), *this);
            for_each(file.functions.begin(), file.functions.end(), *this);
        }

        void operator()(const map<int, counter_t>::value_type& line)
        {
            map<int, counter_t>::iterator iter = destFile.lines.find(line.first);
            if(iter == destFile.lines.end())
            {
                destFile.lines.insert(line);
            }
            else
            {
                iter->second += line.second;
            }
        }

        /*
         * Combine branch counters.
         * Take into account that '-1' corresponds to '-' in trace file
         * and is "arranged before" '0'.
         */
        static void combineBranchCounter(counter_t& counter,
            counter_t counterAnother)
        {
            if(counter == -1)
            {
                if(counterAnother != -1) counter = counterAnother;
            }
            if(counterAnother > 0)
            {
                counter += counterAnother;
            }
        }

        void operator()(const map<TraceSimple::BranchID, counter_t>::value_type& branch)
        {
            map<TraceSimple::BranchID, counter_t>::iterator iter =
                destFile.branches.find(branch.first);

            if(iter == destFile.branches.end())
            {
                destFile.branches.insert(branch);
            }
            else
            {
                combineBranchCounter(iter->second, branch.second);
            }
        }

        /*
         * Update(if needed) function start line 'line' using
         * 'lineAnother'.
         */
        //debug - return bool
        static bool updateFuncLine(int &line, int lineAnother)
        {
            if(line == -1)
            {
                if(lineAnother >= 0) line = lineAnother;
            }
            else if(lineAnother != -1)
            {
                if(line != lineAnother)
                {
                    static bool isFirst = true;
                    if(isFirst)
                    {
                        cerr << "Function has different lines in different "
                            "coverage groups. This warning is reported once." << endl;
                        isFirst = false;
                    }
                    return false;
                }
            }
            return true;
        }
        void operator()(const map<string, TraceSimple::FuncInfo>::value_type& func)
        {
            map<string, TraceSimple::FuncInfo>::iterator iter =
                destFile.functions.find(func.first);
            if(iter == destFile.functions.end())
            {
                destFile.functions.insert(func);
            }
            else
            {
                updateFuncLine(iter->second.lineStart, func.second.lineStart);

                iter->second.counter += func.second.counter;
            }
        }


    public:
        TraceSimple::FileInfo& destFile;
    };

};


void TraceSimple::groupFiles(void)
{
    FilesGrouper grouper;
    grouper.group(*this);
}

struct CommonPrefixFinder
{
    CommonPrefixFinder(string &prefix, bool &isPrefixSet):
        prefix(prefix), isPrefixSet(isPrefixSet)
    {
    }

    void search(const TraceSimple& trace)
    {
        for_each(trace.files.begin(), trace.files.end(), *this);
    }

    void operator()(const map<std::string, TraceSimple::FileInfo>::value_type& source)
    {
        if(isPrefixSet)
        {
            int n = (int)std::min(prefix.size(), source.first.size());
            for(int i = 0; i < n; i++)
            {
                if(prefix[i] != source.first[i])
                {
                    prefix = prefix.substr(0, i);
                    break;
                }
            }
        }
        else
        {
            prefix = source.first;
            isPrefixSet = true;
        }
    }

    string& prefix;
    /* Prefix is set when first source file found. */
    bool& isPrefixSet;
};

string TraceSimple::commonSourcePrefix(void) const
{
    string prefix;
    bool isPrefixSet = false;

    CommonPrefixFinder cpf(prefix, isPrefixSet);
    cpf.search(*this);

    return prefix;
}

struct PrefixFilter
{
    PrefixFilter(const string& prefix): prefix(prefix) {}
    void filter(TraceSimple& trace)
    {
        /*
         * All strings lexicographically lower than 'prefix' have not
         * prefixed with it, so should be removed.
         */
        map<std::string, TraceSimple::FileInfo>::iterator
            iter = trace.files.lower_bound(prefix);

        trace.files.erase(trace.files.begin(), iter);

        for(; iter != trace.files.end(); ++iter)
        {
            if(iter->first.compare(0, prefix.size(), prefix)) break;
        }
        /* 'iter' points to source file, which has not given prefix and
         * lexicographically bigger than it.
         * This file and all after it should be removed.
         */
        trace.files.erase(iter, trace.files.end());
    }

    string prefix;
};

void TraceSimple::filterSources(const string& prefix)
{
    PrefixFilter pf(prefix);
    pf.filter(*this);
}

/********************** Builder of the trace **************************/
class TraceSimple::TraceBuilder: private TraceEventProcessor
{
public:
    void build(TraceSimple* trace, istream& is, TraceParser& parser,
        const char* filename)
    {
        this->trace = trace;

        parser.parse(is, *this, filename);
    }

    /* SF: <sourcePath> */
    void onSourceStart(const std::string& sourcePath, int traceLine)
    {
        pair<map<string, FileInfo>::iterator, bool> newIter =
            trace->files.insert(make_pair(sourcePath, FileInfo()));

        currentFile = &newIter.first->second;
    }

    /* FN: <funcLine>, <funcName> */
    void onFunction(const std::string& funcName, int funcLine, int traceLine)
    {
        pair<map<string, FuncInfo>::iterator, bool> newIter =
            currentFile->functions.insert(make_pair(funcName, funcLine));
        if(!newIter.second)
        {
            FuncInfo& existedFunc = newIter.first->second;
            if(funcLine >= 0)
            {
                if (existedFunc.lineStart < 0)
                {
                    existedFunc.lineStart = funcLine;
                }
            }
        }
    }

    /* FNDA: <counter>, <funcName> */
    void onFunctionCounter(const std::string& funcName,
        counter_t counter, int /*traceLine*/)
    {
        /* Create function record, if it wasn't created yet. */
        pair<map<string, FuncInfo>::iterator, bool> newIter =
            currentFile->functions.insert(make_pair(funcName, -1));

        updateCounter(newIter.first->second.counter, counter);
    }

    /*
     * BRDA: <branchLine>, <blockNumber>, <branchNumber>, <counter>
     */
    void onBranchCoverage(int branchLine,
        int blockNumber, int branchNumber, counter_t counter, int traceLine)
    {
        BranchID branchID(branchLine, blockNumber, branchNumber);

        pair<map<BranchID, counter_t>::iterator, bool> newIter =
            currentFile->branches.insert(make_pair(branchID, -1));

        updateCounter(newIter.first->second, counter);
    }
    /*
     * BRDA: <branchLine>, <blockNumber>, <branchNumber>, -
     */
    void onBranchNotCovered(int branchLine,
        int blockNumber, int branchNumber, int traceLine)
    {
        BranchID branchID(branchLine, blockNumber, branchNumber);

        pair<map<BranchID, counter_t>::iterator, bool> newIter =
            currentFile->branches.insert(make_pair(branchID, -1));
    }

    /* DA: <line>, <counter> */
    void onLineCounter(int line, counter_t counter, int traceLine)
    {
        pair<map<int, counter_t>::iterator, bool> newIter =
            currentFile->lines.insert(make_pair(line, counter));

        updateCounter(newIter.first->second, counter);
    }

private:
    /* Set when need to build trace. */
    TraceSimple* trace;

    /* Pointer to file under construction.
     *
     * It points to the information which already added to the
     * corresponded map.
     */
    FileInfo* currentFile;
};

void TraceSimple::read(std::istream& is, TraceParser& parser, const char* filename)
{
    TraceBuilder builder;

    builder.build(this, is, parser, filename);
}

void TraceSimple::read(std::istream& is, const char* filename)
{
    TraceParser parser;
    read(is, parser, filename);
}

void TraceSimple::read(const char* filename, TraceParser& parser)
{
    ifstream is(filename);
    if(!is)
    {
        cerr << "Failed to open file '" << filename << "' for read trace." << endl;
        throw runtime_error("Cannot open file");
    }

    read(is, parser, filename);
}

void TraceSimple::read(const char* filename)
{
    TraceParser parser;
    read(filename, parser);
}


/*
 * Write file information into stream.
 *
 * Always start from current line.
 * Always append newline after last line.
 * Always non-empty.
 */
static void writeFileInfoToStream(const TraceSimple::FileInfo& fileInfo, ostream& os)
{
    int functionsTotal = 0, functionsTotalHit = 0;

    map<string, TraceSimple::FuncInfo>::const_iterator funcIter =
        fileInfo.functions.begin(), funcIterEnd = fileInfo.functions.end();

    /* Print functions lines */
    for(; funcIter != funcIterEnd; ++funcIter)
    {
        /*
         * See description of 'lineStart' field in 'FuncInfo' structure
         * for special processing of its -1 value.
         */
        if(funcIter->second.lineStart != -1)
        {
            os << "FN:" << funcIter->second.lineStart
                << ',' << funcIter->first << endl;
        }
        ++functionsTotal;
    }
    /* Print functions counters */
    funcIter = fileInfo.functions.begin();
    for(; funcIter != funcIterEnd; ++funcIter)
    {
        os << "FNDA:" << funcIter->second.counter << ',' << funcIter->first << endl;
        if(funcIter->second.counter > 0) ++functionsTotalHit;
    }
    /* Print functions statistic */
    os << "FNF:" << functionsTotal << endl;
    os << "FNH:" << functionsTotalHit << endl;

    /************/
    int branchesTotal = 0, branchesTotalHit = 0;

    map<TraceSimple::BranchID, counter_t>::const_iterator branchIter =
        fileInfo.branches.begin(), branchIterEnd = fileInfo.branches.end();

    /* Print branches counters */

    for(; branchIter != branchIterEnd; ++branchIter)
    {
        os << "BRDA:"
            << branchIter->first.line << ','
            << branchIter->first.blockNumber << ','
            << branchIter->first.branchNumber << ',';

        counter_t counter = branchIter->second;

        if(counter != -1)
            os << counter;
        else
            os << '-';
        os << endl;

        ++branchesTotal;
        // TODO: compare with 0 or with -1?
        if(counter > 0) ++branchesTotalHit;
    }
    /* Print branches statistic */
    os << "BRF:" << branchesTotal << endl;
    os << "BRH:" << branchesTotalHit << endl;

    /********************/
    int linesTotal = 0, linesTotalHit = 0;


    map<int, counter_t>::const_iterator lineIter = fileInfo.lines.begin(),
        lineIterEnd = fileInfo.lines.end();
    /* Print lines counters */
    for(; lineIter != lineIterEnd; ++lineIter)
    {
        os << "DA:" << lineIter->first << ',' << lineIter->second << endl;

        linesTotal++;
        if(lineIter->second > 0) ++linesTotalHit;
    }
    /* Print lines statistic */
    os << "LF:" << linesTotal << endl;
    os << "LH:" << linesTotalHit << endl;
}

void TraceSimple::write(std::ostream& os) const
{
    map<std::string, FileInfo>::const_iterator iter
        = files.begin(), iterEnd = files.end();

    if(iter == iterEnd)
    {
        /* Empty trace. TODO: is it possible? how to write it?*/
        os << "TN:";
        return;
    }
    for (;iter != iterEnd; iter++)
    {
        os << "TN:" << endl;
        writeFileInfoToStream(iter->second, os);
    }
}
