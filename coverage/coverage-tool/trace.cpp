// trace.cpp - implementation of 'Trace' class methods.

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


#include "trace.hh"
#include "trace_parser.hh"

#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>
#include <cassert>

#include <algorithm>

using namespace std;
typedef Trace::counter_t counter_t;

/* 
 * Report about error in trace and throw exception.
 * 
 * 'msg' is used only for insert into stream.
 * (That is, it may contain '<<' operators).
 */
#define trace_error(msg) \
cerr << traceLine << ": " << msg << endl; \
throw runtime_error("Incorrect trace file");

/********************* BranchID methods ****************************/
bool Trace::BranchID::operator<(const BranchID& branchID) const
{
    if(line < branchID.line) return true;
    else if(line > branchID.line) return false;

    if(blockNumber < branchID.blockNumber) return true;
    else if(blockNumber > branchID.blockNumber) return false;

    if(branchNumber < branchID.branchNumber) return true;
    return false;
}

std::ostream& operator<<(std::ostream& os, const Trace::BranchID& branchID)
{
    return os << "{" << branchID.line << ", " << branchID.blockNumber << ", "
        << branchID.branchNumber << "}";
}

/********************* FileGroupID methods ****************************/
bool Trace::FileGroupID::operator<(const FileGroupID& groupID) const
{
    /* 
     * Test name is rarely used when create traces, so
     * 'testName' is usually empty string.
     * 
     * Because of this compare 'testName' after 'filename'
     */
    if(filename < groupID.filename) return true;
    else if(filename > groupID.filename) return false;
    
    return testName < groupID.testName;
}


ostream& operator<<(ostream& os, const Trace::FileGroupID& groupId)
{
    os << "{";
    /*
     * We doesn't want to print empty test name.
     */
    if(!groupId.testName.empty())
        os << groupId.testName << ",";
    return os << groupId.filename << "}";
}

/* Whether filename correspond to source file(ends with ".c"). */
static bool isSource(const std::string& filename)
{
    size_t len = filename.length();
    return (len > 2) && (filename[len - 2] == '.') && (filename[len - 1] == 'c');
}


/*********************** Trace class methods **************************/

Trace::Trace() {}

Trace::~Trace()
{
    map<FileGroupID, FileGroupInfo*>::iterator iter = fileGroups.begin(),
        iterEnd = fileGroups.end();
    for(;iter != iterEnd; ++iter)
    {
        delete iter->second;
    }
}

Trace::Trace(const Trace& trace)
{
    *this = trace;
}

Trace& Trace::operator=(const Trace& trace)
{
    fileGroups = trace.fileGroups;
    
    map<FileGroupID, FileGroupInfo*>::iterator iter = fileGroups.begin(),
        iterEnd = fileGroups.end();
    for(;iter != iterEnd; ++iter)
    {
        iter->second = new FileGroupInfo(*iter->second);
    }
    
    return *this;
}

/* Calculate statistic */
class LinesTotalCollector
{
public:
    LinesTotalCollector(void): linesTotal(0) {}
    
    int linesTotal;
    
    void operator()(const map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& group)
    {
        *this = for_each(group.second->files.begin(), group.second->files.end(), *this);
    }
    
    void operator()(const map<string, Trace::FileInfo>::value_type& file)
    {
        *this = for_each(file.second.lines.begin(), file.second.lines.end(), *this);
    }

    void operator()(const map<int, counter_t>::value_type& /*line*/)
    {
        linesTotal++;
    }
    
    void addTrace(const Trace& trace)
    {
        *this = for_each(trace.fileGroups.begin(), trace.fileGroups.end(), *this);
    }
};

int Trace::linesTotal(void) const
{
    LinesTotalCollector collector;
    collector.addTrace(*this);
    return collector.linesTotal;
}

class LinesTotalHitCollector
{
public:
    LinesTotalHitCollector(): linesTotalHit(0) {}

    int linesTotalHit;
    
    void operator()(const map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& group)
    {
        *this = for_each(group.second->files.begin(), group.second->files.end(), *this);
    }
    
    void operator()(const map<string, Trace::FileInfo>::value_type& file)
    {
        *this = for_each(file.second.lines.begin(), file.second.lines.end(), *this);
    }

    void operator()(const map<int, counter_t>::value_type& line)
    {
        if(line.second > 0) linesTotalHit++;
    }
    
    void addTrace(const Trace& trace)
    {
        *this = for_each(trace.fileGroups.begin(), trace.fileGroups.end(), *this);
    }
};

int Trace::linesTotalHit(void) const
{
    LinesTotalHitCollector collector;
    collector.addTrace(*this);
    return collector.linesTotalHit;
}

class BranchesTotalCollector
{
public:
    BranchesTotalCollector(): branchesTotal(0) {}

    int branchesTotal;
    
    void operator()(const map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& group)
    {
        *this = for_each(group.second->files.begin(), group.second->files.end(), *this);
    }
    
    void operator()(const map<string, Trace::FileInfo>::value_type& file)
    {
        *this = for_each(file.second.branches.begin(), file.second.branches.end(), *this);
    }

    void operator()(const map<Trace::BranchID, counter_t>::value_type& /*branch*/)
    {
        branchesTotal++;
    }
    
    void addTrace(const Trace& trace)
    {
        *this = for_each(trace.fileGroups.begin(), trace.fileGroups.end(), *this);
    }
};

int Trace::branchesTotal(void) const
{
    BranchesTotalCollector collector;
    collector.addTrace(*this);
    return collector.branchesTotal;
}

class BranchesTotalHitCollector
{
public:
    BranchesTotalHitCollector(): branchesTotalHit(0) {}

    int branchesTotalHit;
    
    void operator()(const map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& group)
    {
        *this = for_each(group.second->files.begin(), group.second->files.end(), *this);
    }
    
    void operator()(const map<string, Trace::FileInfo>::value_type& file)
    {
        *this = for_each(file.second.branches.begin(), file.second.branches.end(), *this);
    }

    void operator()(const map<Trace::BranchID, counter_t>::value_type& branch)
    {
        if(branch.second > 0) branchesTotalHit++;
    }
    
    void addTrace(const Trace& trace)
    {
        *this = for_each(trace.fileGroups.begin(), trace.fileGroups.end(), *this);
    }
};

int Trace::branchesTotalHit(void) const
{
    BranchesTotalHitCollector collector;
    collector.addTrace(*this);
    return collector.branchesTotalHit;
}


class FunctionsTotalCollector
{
public:
    FunctionsTotalCollector(): functionsTotal(0) {}

    int functionsTotal;
    
    void operator()(const map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& group)
    {
        *this = for_each(group.second->files.begin(), group.second->files.end(), *this);
    }
    
    void operator()(const map<string, Trace::FileInfo>::value_type& file)
    {
        *this = for_each(file.second.functions.begin(), file.second.functions.end(), *this);
    }

    void operator()(const map<string, Trace::FuncInfo>::value_type& /*function*/)
    {
        functionsTotal++;
    }
    
    void addTrace(const Trace& trace)
    {
        *this = for_each(trace.fileGroups.begin(), trace.fileGroups.end(), *this);
    }
};

int Trace::functionsTotal(void) const
{
    FunctionsTotalCollector collector;
    collector.addTrace(*this);
    return collector.functionsTotal;
}

class FunctionsTotalHitCollector
{
public:
    FunctionsTotalHitCollector(): functionsTotalHit(0) {}

    int functionsTotalHit;
    
    void operator()(const map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& group)
    {
        *this = for_each(group.second->files.begin(), group.second->files.end(), *this);
    }
    
    void operator()(const map<string, Trace::FileInfo>::value_type& file)
    {
        *this = for_each(file.second.functions.begin(), file.second.functions.end(), *this);
    }

    void operator()(const map<string, Trace::FuncInfo>::value_type& function)
    {
        if(function.second.counter > 0) functionsTotalHit++;
    }
    
    void addTrace(const Trace& trace)
    {
        *this = for_each(trace.fileGroups.begin(), trace.fileGroups.end(), *this);
    }
};

int Trace::functionsTotalHit(void) const
{
    FunctionsTotalHitCollector collector;
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
    void group(Trace& trace)
    {
        this->trace = &trace;

        for_each(trace.fileGroups.begin(), trace.fileGroups.end(), *this);
    }

    void operator()(map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& group)
    {
        currentGroup = &group;
        
        map<string, Trace::FileInfo>& files = group.second->files;
        /* 
         * First, move content of all non-source files into
         * another groups.
         */
        for_each(group.second->files.begin(), group.second->files.end(), *this);
        /*
         * Second, remove all non-source files.
         */
        map<string, Trace::FileInfo>::iterator
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

    void operator()(map<string, Trace::FileInfo>::value_type& file)
    {
        if(file.first == currentGroup->first.filename) return;/* Already grouped */
        
        /* It is needed to move file into another group. */
        Trace::FileGroupID newGroupID;
        newGroupID.testName = currentGroup->first.testName;
        newGroupID.filename = file.first;
        
        groupIter iter = trace->fileGroups.find(newGroupID);
        if(iter != trace->fileGroups.end())
        {
            /* Merge file with same file in the existed group. */
            map<string, Trace::FileInfo>::iterator fileIter =
                iter->second->files.find(file.first);
            /* Group always have file with name which coincide with group's one. */
            assert(fileIter != iter->second->files.end());
            
            CombineFile combineFile(fileIter->second);
            combineFile.addFile(file.second);
        }
        else
        {
            /* Need to create group to which file will be moved. */
            Trace::FileGroupInfo* newGroup = new Trace::FileGroupInfo();
            
            trace->fileGroups.insert(make_pair(newGroupID, newGroup));
            pair<map<string, Trace::FileInfo>::iterator, bool> newFileIter =
                newGroup->files.insert(make_pair(newGroupID.filename, Trace::FileInfo()));
            swapFiles(newFileIter.first->second, file.second);
        }
    }
    
private:
    Trace* trace;
    
    typedef map<Trace::FileGroupID, Trace::FileGroupInfo*>::iterator groupIter;
    groupIter::value_type* currentGroup;
    
    /* 
     * Swap content of files. Helper for move file from one group into another.
     */
    static void swapFiles(Trace::FileInfo& file1, Trace::FileInfo& file2)
    {
        swap(file1.lines, file2.lines);
        swap(file1.functions, file2.functions);
        swap(file1.branches, file2.branches);
    }

    /* Combine counters in files. */
    class CombineFile
    {
    public:
        CombineFile(Trace::FileInfo& destFile) : destFile(destFile) {}
        void addFile(const Trace::FileInfo& file)
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

        void operator()(const map<Trace::BranchID, counter_t>::value_type& branch)
        {
            map<Trace::BranchID, counter_t>::iterator iter =
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
        void operator()(const map<string, Trace::FuncInfo>::value_type& func)
        {
            map<string, Trace::FuncInfo>::iterator iter =
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
        Trace::FileInfo& destFile;
    };

};


void Trace::groupFiles(void)
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
    
    void search(const Trace& trace)
    {
        for_each(trace.fileGroups.begin(), trace.fileGroups.end(), *this);
    }
    
    void operator()(const map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& group)
    {
        for_each(group.second->files.begin(), group.second->files.end(), *this);
    }
    
    void operator()(const map<std::string, Trace::FileInfo>::value_type& source)
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

string Trace::commonSourcePrefix(void) const
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
    void filter(Trace& trace)
    {
        for_each(trace.fileGroups.begin(), trace.fileGroups.end(), *this);
    }
    
    void operator()(map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& group)
    {
        map<string, Trace::FileInfo>& files = group.second->files;
        
        /* 
         * All strings lexicographically lower than 'prefix' have not
         * prefixed with it, so should be removed.
         */
        map<std::string, Trace::FileInfo>::iterator
            iter = files.lower_bound(prefix),

        files.erase(files.begin(), iter);
        
        for(; iter != files.end(); ++iter)
        {
            if(iter->first.compare(0, prefix.size(), prefix)) break;
        }
        /* 'iter' points to source file, which has not given prefix and
         * lexicographically bigger than it.
         * This file and all after it should be removed.
         */
        files.erase(iter, files.end());
    }

    string prefix;
};

void Trace::filterSources(const string& prefix)
{
    PrefixFilter pf(prefix);
    pf.filter(*this);
}

/********************** Builder of the trace **************************/
class Trace::TraceBuilder: private TraceEventProcessor
{
public:
    TraceBuilder(): currentGroupInfo(NULL) {}
    ~TraceBuilder() {delete currentGroupInfo;}

    void build(Trace* trace, istream& is, TraceParser& parser,
        const char* filename)
    {
        /* Recover from possible previous errors */
        delete currentGroupInfo;
        currentGroupInfo = NULL;
        
        this->trace = trace;
        
        parser.parse(is, *this, filename);
    }


    /* TN: <testName> */
    void onTestStart(const std::string& testName, int /*traceLine*/)
    {
        currentGroupInfo = new FileGroupInfo();
        currentGroupID.testName = testName;
        currentGroupID.filename.clear();
    }
    void onTestEnd(int traceLine)
    {
        assert(currentGroupInfo);
        if(currentGroupID.filename.empty())
        {
            cerr << "WARNING: Ignore test '" << currentGroupID.testName
                << "'without files." << endl;
            delete currentGroupInfo;
            currentGroupInfo = NULL;
            return;
        }
        
        pair<map<FileGroupID, FileGroupInfo*>::iterator, bool> iterNew =
            trace->fileGroups.insert(make_pair(currentGroupID, currentGroupInfo));
        if(!iterNew.second)
        {
            trace_error("Information for files group " << currentGroupID
                << " is written twice.");
        }
        
        currentGroupInfo = NULL;
    }
    


    /* SF: <sourcePath> */
    void onSourceStart(const std::string& sourcePath, int traceLine)
    {
        pair<map<string, FileInfo>::iterator, bool> newIter = 
            currentGroupInfo->files.insert(make_pair(sourcePath, FileInfo()));
        if(!newIter.second)
        {
            trace_error("Attempt to add information about file '"
                << sourcePath << "' to group"
                << currentGroupID << " which already has it.");
        }
        
        currentFile = &newIter.first->second;
        
        if(currentGroupID.filename.empty())
        {
            /* First file in the group - its name may be name of group */
            currentGroupID.filename = sourcePath;
        }
        else if(isSource(sourcePath))
        {
            /* Non-first file in the group, but corresponds to source file. */
            if(isSource(currentGroupID.filename))
            {
                trace_error("Two source files in group " << currentGroupID
                    << ".");
            }
            currentGroupID.filename = sourcePath;
        }
    }
    
    /* FN: <funcLine>, <funcName> */
    void onFunction(const std::string& funcName, int funcLine, int traceLine)
    {
        pair<map<string, FuncInfo>::iterator, bool> newIter = 
            currentFile->functions.insert(make_pair(funcName, funcLine));
        if(!newIter.second)
        {
            trace_error("Attempt to add function '" + funcName
                + "' for file which already has it.");
        }
    }
    
    /* FNDA: <counter>, <funcName> */
    void onFunctionCounter(const std::string& funcName,
        counter_t counter, int /*traceLine*/)
    {
        map<string, FuncInfo>::iterator iter = currentFile->functions.find(funcName);
        if(iter == currentFile->functions.end())
        {
            /* 
             * Strange, but possible - see definition of 'lineStart'
             * in 'FuncInfo' structure.
             */
            
            pair<map<string, FuncInfo>::iterator, bool> newIter = 
                currentFile->functions.insert(make_pair(funcName, -1));
            iter = newIter.first;
        }
        
        iter->second.counter = counter;
    }
    
    /* 
     * BRDA: <branchLine>, <blockNumber>, <branchNumber>, <counter>
     */
    void onBranchCoverage(int branchLine,
        int blockNumber, int branchNumber, counter_t counter, int traceLine)
    {
        BranchID branchID(branchLine, blockNumber, branchNumber);
        
        if(branchLine > 1000000)
            cerr << traceLine << ": Branch with line number " << branchLine << " is found." << endl;
        
        pair<map<BranchID, counter_t>::iterator, bool> newIter = 
            currentFile->branches.insert(make_pair(branchID, counter));
        if(!newIter.second)
        {
            trace_error("Attempt to add counter for branch "
                "to file which already has it.");
        }
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
        if(!newIter.second)
        {
            trace_error("Attempt to add '-' taken for branch "
                "to file which already has it.");
        }
    }
    
    /* DA: <line>, <counter> */
    void onLineCounter(int line, counter_t counter, int traceLine)
    {
        pair<map<int, counter_t>::iterator, bool> newIter = 
            currentFile->lines.insert(make_pair(line, counter));
        if(!newIter.second)
        {
            trace_error("Attempt to add counter for line " << line <<
                " to file which already has it.");
        }
    }

private:
    /* Set when need to build trace. */
    Trace* trace;

    /* 
     * Pointer to the allocated file group information.
     * 
     * When store this information to the trace pointer is nullified.
     */
    FileGroupInfo* currentGroupInfo;
    /*
     * Current information about group identificator.
     */
    FileGroupID currentGroupID;
    
    /* Pointer to file under construction.
     * 
     * It points to the information which already added to the
     * corresponded map.
     */
    FileInfo* currentFile;
};

void Trace::read(std::istream& is, TraceParser& parser, const char* filename)
{
    TraceBuilder builder;
    
    builder.build(this, is, parser, filename);
}

void Trace::read(std::istream& is, const char* filename)
{
    TraceParser parser;
    read(is, parser, filename);
}

void Trace::read(const char* filename, TraceParser& parser)
{
    ifstream is(filename);
    if(!is)
    {
        cerr << "Failed to open file '" << filename << "' for read trace." << endl;
        throw runtime_error("Cannot open file");
    }
    
    read(is, parser, filename);
}

void Trace::read(const char* filename)
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
static void writeFileInfoToStream(const Trace::FileInfo& fileInfo, ostream& os)
{
    int functionsTotal = 0, functionsTotalHit = 0;

    map<string, Trace::FuncInfo>::const_iterator funcIter =
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
    
    map<Trace::BranchID, counter_t>::const_iterator branchIter =
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

/*
 * Write file group information into stream.
 * 
 * Always start from current line.
 * Never append newline at the end.
 * Only for group with files, in that case non-empty.
 */
static void writeFileGroupInfoToStream(
    const Trace::FileGroupInfo& groupInfo, ostream& os)
{
    map<string, Trace::FileInfo>::const_iterator
        iter = groupInfo.files.begin(),
        iterEnd = groupInfo.files.end();
    
    assert(iter != iterEnd);/* should be non-empty */
    
    os << "SF:" << iter->first << endl;
    writeFileInfoToStream(iter->second, os);
    os << "end_of_record";
    
    for(++iter; iter != iterEnd; ++iter)
    {
        os << endl; /* newline after previous record */

        os << "SF:" << iter->first << endl;
        writeFileInfoToStream(iter->second, os);
        os << "end_of_record";
    }
}

void Trace::write(std::ostream& os) const
{
    map<FileGroupID, FileGroupInfo*>::const_iterator iter
        = fileGroups.begin(), iterEnd = fileGroups.end();
    /* Found first non-empty group */
    for(; iter != iterEnd; iter++)
    {
        if(!iter->second->files.empty()) break;
    }
    
    if(iter == iterEnd)
    {
        /* Empty trace. TODO: is it possible? how to write it?*/
        os << "TN:";
        return;
    }
    /* Write first group. */
    os << "TN:" << iter->first.testName << endl;
    writeFileGroupInfoToStream(*iter->second, os);
    /* Write other non-empty groups. */
    for(++iter; iter != iterEnd; iter++)
    {
        if(iter->second->files.empty()) continue;
        
        os << endl;
        os << "TN:" << iter->first.testName << endl;
        writeFileGroupInfoToStream(*iter->second, os);
    }
}
