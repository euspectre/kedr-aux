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
#include <do_trace_operation.hh>

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
        os << "SF:" << iter->first << endl;
        writeFileInfoToStream(iter->second, os);
        os << "end_of_record" << endl;
    }
}

template<>
void doTraceOperation<TraceSimple>(TraceOperation& op,
    const std::vector<TraceSimple>& operands, TraceSimple& result)
{
    int n = (int)operands.size();
    std::vector<const std::map<std::string, TraceSimple::FileInfo>*> fileInfoMaps(n);
    for (int i = 0; i < n; i++)
    {
        fileInfoMaps[i] = &operands[i].files;
    }

    typedef MapVectorIterator<std::map<std::string, TraceSimple::FileInfo>> FileInfoMapsIterType;
    FileInfoMapsIterType fileInfoMapsIterEnd = FileInfoMapsIterType::end(fileInfoMaps);
    for (FileInfoMapsIterType fileInfoMapsIter = FileInfoMapsIterType::begin(fileInfoMaps); fileInfoMapsIter != fileInfoMapsIterEnd; ++fileInfoMapsIter)
    {
        /* Currently new file is created in any case. */
        pair<map<string, TraceSimple::FileInfo>::iterator, bool> newIter =
            result.files.insert(make_pair(fileInfoMapsIter->first, TraceSimple::FileInfo()));
        assert(newIter.second);

        TraceSimple::FileInfo &resultFileInfo = newIter.first->second;

        std::vector<const std::map<std::string, TraceSimple::FuncInfo>*> funcInfoMaps(n);
        for (int i = 0; i < n; i++)
        {
            if (fileInfoMapsIter->second[i])
            {
                funcInfoMaps[i] = &fileInfoMapsIter->second[i]->functions;
            }
        }
        typedef MapVectorIterator<std::map<std::string, TraceSimple::FuncInfo>> FuncInfoMapsIterType;
        FuncInfoMapsIterType funcInfoMapsIterEnd = FuncInfoMapsIterType::end(funcInfoMaps);
        for (FuncInfoMapsIterType funcInfoMapsIter = FuncInfoMapsIterType::begin(funcInfoMaps); funcInfoMapsIter != funcInfoMapsIterEnd; ++funcInfoMapsIter)
        {
            int line = -1;
            std::vector<counter_t> funcCounters(n);
            for(int i = 0; i < n; i++)
            {
                if (funcInfoMapsIter->second[i])
                {
                    funcCounters[i] = funcInfoMapsIter->second[i]->counter;
                    if (line == -1)
                    {
                        line = funcInfoMapsIter->second[i]->lineStart;
                    }
                }
                else
                {
                    funcCounters[i] = -1;
                }
            }

            Trace::counter_t newCounter = op.functionOperation(funcCounters);

            TraceSimple::FuncInfo funcInfo(line);
            funcInfo.counter = newCounter;
            resultFileInfo.functions.insert(make_pair(funcInfoMapsIter->first, funcInfo));
        }

        std::vector<const std::map<TraceSimple::BranchID, counter_t>*> branchInfoMaps(n);
        for (int i = 0; i < n; i++)
        {
            if (fileInfoMapsIter->second[i])
            {
                branchInfoMaps[i] = &fileInfoMapsIter->second[i]->branches;
            }
        }
        typedef MapVectorIterator<std::map<TraceSimple::BranchID, counter_t>> BranchInfoMapsIterType;
        BranchInfoMapsIterType branchInfoMapsIterEnd = BranchInfoMapsIterType::end(branchInfoMaps);
        for (BranchInfoMapsIterType branchInfoMapsIter = BranchInfoMapsIterType::begin(branchInfoMaps); branchInfoMapsIter != branchInfoMapsIterEnd; ++branchInfoMapsIter)
        {
            std::vector<counter_t> branchCounters(n);
            for(int i = 0; i < n; i++)
            {
                if (branchInfoMapsIter->second[i])
                {
                    branchCounters[i] = *branchInfoMapsIter->second[i];
                }
                else
                {
                    branchCounters[i] = -1;
                }
            }

            Trace::counter_t newCounter = op.branchOperation(branchCounters);

            resultFileInfo.branches.insert(make_pair(branchInfoMapsIter->first, newCounter));
        }

        std::vector<const std::map<int, counter_t>*> lineInfoMaps(n);
        for (int i = 0; i < n; i++)
        {
            if (fileInfoMapsIter->second[i])
            {
                lineInfoMaps[i] = &fileInfoMapsIter->second[i]->lines;
            }
        }

        typedef MapVectorIterator<std::map<int, counter_t>> LineInfoMapsIterType;
        LineInfoMapsIterType lineInfoMapsIterEnd = LineInfoMapsIterType::end(lineInfoMaps);
        for (LineInfoMapsIterType lineInfoMapsIter = LineInfoMapsIterType::begin(lineInfoMaps); lineInfoMapsIter != lineInfoMapsIterEnd; ++lineInfoMapsIter)
        {
            std::vector<counter_t> lineCounters(n);
            for(int i = 0; i < n; i++)
            {
                if (lineInfoMapsIter->second[i])
                {
                    lineCounters[i] = *lineInfoMapsIter->second[i];
                }
                else
                {
                    lineCounters[i] = -1;
                }
            }

            Trace::counter_t newCounter = op.lineOperation(lineCounters);

            resultFileInfo.lines.insert(make_pair(lineInfoMapsIter->first, newCounter));
        }
    }
}
