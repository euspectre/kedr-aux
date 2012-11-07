// test_modifier.cpp - implementation of generic trace modification algorithm.

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


#include "trace_modifier.hh"
#include <algorithm>

using namespace std;

/*********************** Trace::Modifier members **********************/
/* Internal Trace::FileInfo::Modifier object. */
Trace::FileInfo::Modifier modifierInternal;

Trace::FileInfo::Modifier* Trace::Modifier::onSourceStart(
    const std::map<std::string, Trace::FileInfo>::value_type& /*source*/)
{
    return &modifierInternal;
}

class TraceFileModifierTraversal
{
public:
    TraceFileModifierTraversal(Trace::FileInfo::Modifier& modifier,
        Trace::FileInfo& traceFileModified)
        : modifier(modifier), traceFileModified(traceFileModified) {}
    
    void traverseFile(const Trace::FileInfo& traceFile)
    {
        for_each(traceFile.functions.begin(), traceFile.functions.end(), *this);
        for_each(traceFile.branches.begin(), traceFile.branches.end(), *this);
        for_each(traceFile.lines.begin(), traceFile.lines.end(), *this);
    }
    
    void operator()(const map<string, Trace::FuncInfo>::value_type& func)
    {
        if(modifier.onFunction(func)) return;
        
        map<string, Trace::FuncInfo>::iterator funcIterModified =
            traceFileModified.functions.find(func.first);
        
        if(funcIterModified != traceFileModified.functions.end())
        {
            if((func.second.lineStart > 0)
                && (funcIterModified->second.lineStart > 0)
                && (funcIterModified->second.lineStart != func.second.lineStart))
            {
                cerr << "Warning: Start lines of function " << func.first
                    << " in traces differ." << endl;
            }
            
            modifier.modifyFuncCounter(func.second.counter,
                funcIterModified->second.counter);
        }
        else
        {
            modifier.newFunc(traceFileModified);
        }
    }
    
    void operator()(const map<Trace::BranchID, counter_t>::value_type& branch)
    {
        if(modifier.onBranch(branch)) return;
        
        map<Trace::BranchID, counter_t>::iterator
            branchIterModified = traceFileModified.branches.find(branch.first);
        if(branchIterModified != traceFileModified.branches.end())
        {
            modifier.modifyBranchCounter(branch.second,
                branchIterModified->second);
        }
        else
        {
            modifier.newBranch(traceFileModified);
        }
    }

    void operator()(const map<int, counter_t>::value_type& line)
    {
        if(modifier.onLine(line)) return;
        
        map<int, counter_t>::iterator
            lineIterModified = traceFileModified.lines.find(line.first);
        if(lineIterModified != traceFileModified.lines.end())
        {
            modifier.modifyLineCounter(line.second,
                lineIterModified->second);
        }
        else
        {
            modifier.newLine(traceFileModified);
        }
    }


private:
    Trace::FileInfo::Modifier& modifier;
    Trace::FileInfo& traceFileModified;
};

void modifyTraceFile(const Trace::FileInfo& traceFile,
    Trace::FileInfo::Modifier& modifier,
    Trace::FileInfo& traceFileModified)
{
    TraceFileModifierTraversal traversal(modifier, traceFileModified);
    traversal.traverseFile(traceFile);
}


class TraceModifierTraversal
{
public:
    TraceModifierTraversal(Trace::Modifier& modifier,
        Trace& traceModified)
        : modifier(modifier), traceModified(traceModified) {}

    void traverseTrace(const Trace& trace)
    {
        for_each(trace.fileGroups.begin(), trace.fileGroups.end(), *this);
    }

    void operator()(const map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& group)
    {
        if(modifier.onFileGroupStart(group)) return;
        
        map<Trace::FileGroupID, Trace::FileGroupInfo*>::iterator
            groupIterModified = traceModified.fileGroups.find(group.first);
        if(groupIterModified != traceModified.fileGroups.end())
        {
            currentGroupModified = &*groupIterModified;
            for_each(group.second->files.begin(), group.second->files.end(), *this);
            modifier.onFileGroupEnd();
        }
        else
        {
            modifier.onFileGroupEndNew(traceModified);
        }
    }

    void operator()(const map<string, Trace::FileInfo>::value_type& file)
    {
        Trace::FileInfo::Modifier* fileModifier =
            modifier.onSourceStart(file);
        if(fileModifier == NULL) return;
        
        map<string, Trace::FileInfo>::iterator
            sourceIterModified = currentGroupModified->second->files.find(file.first);

        if(sourceIterModified != currentGroupModified->second->files.end())
        {
            modifyTraceFile(file.second, *fileModifier,
                sourceIterModified->second);
            
            modifier.onSourceEnd();
        }
        else
        {
            modifier.onSourceEndNew(*currentGroupModified->second);
        }
    }

private:
    Trace::Modifier& modifier;
    Trace& traceModified;
    /* Group currently modified */
    map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type* currentGroupModified;
};

void modifyTrace(const Trace& trace, Trace::Modifier& modifier,
    Trace& traceModified)
{
    TraceModifierTraversal traversal(modifier, traceModified);
    traversal.traverseTrace(trace);
}
