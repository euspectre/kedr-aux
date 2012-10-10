#include "trace_modifier.hh"

using namespace std;

static void modifyFileInfo(Trace::FileInfo& fileInfo,
    const Trace::FileInfo& fileInfoAnother,
    TraceModifierEventNotifier& notifier)
{
    /* Modify functions counters */
    map<string, Trace::FuncInfo>::const_iterator
        funcIterAnother = fileInfoAnother.functions.begin(),
        funcIterAnotherEnd = fileInfoAnother.functions.end();
    for(; funcIterAnother != funcIterAnotherEnd; ++funcIterAnother)
    {
        if(notifier.onFunction(*funcIterAnother)) continue;
        
        map<string, Trace::FuncInfo>::iterator funcIter =
            fileInfo.functions.find(funcIterAnother->first);
        
        if(funcIter != fileInfo.functions.end())
        {
            if((funcIter->second.lineStart > 0)
                && (funcIterAnother->second.lineStart > 0)
                && (funcIter->second.lineStart != funcIterAnother->second.lineStart))
            {
                cerr << "Warning: Start lines of function " << funcIter->first
                    << " in traces differ." << endl;
            }
            
            notifier.modifyFuncCounter(funcIter->second.counter,
                funcIterAnother->second.counter);
        }
        else
        {
            notifier.newFunc();
        }
    }
    /* Modify branches */
    map<Trace::BranchID, counter_t>::const_iterator
        branchIterAnother = fileInfoAnother.branches.begin(),
        branchIterAnotherEnd = fileInfo.branches.end();
    for(; branchIterAnother != branchIterAnother; ++branchIterAnother)
    {
        if(notifier.onBranchCounter(*branchIterAnother)) continue;
        
        map<Trace::BranchID, counter_t>::iterator
            branchIter = fileInfo.branches.find(branchIterAnother->first);
        if(branchIter != fileInfo.branches.end())
        {
            notifier.modifyBranchCounter(branchIter->second,
                branchIterAnother->second);
        }
        else
        {
            notifier.newBranch();
        }
    }
    /* Diff for lines */
    map<int, counter_t>::const_iterator
        lineIterAnother = fileInfoAnother.lines.begin(),
        lineIterAnotherEnd = fileInfoAnother.lines.end();
    for(; lineIterAnother != lineIterAnotherEnd; ++lineIterAnother)
    {
        if(notifier.onLine(*lineIterAnother)) continue;
        
        map<int, counter_t>::iterator
            lineIter = fileInfo.lines.find(lineIterAnother->first);
        if(lineIter != fileInfo.lines.end())
        {
            notifier.modifyLineCounter(lineIter->second,
                lineIterAnother->second);
        }
        else
        {
            notifier.newLine();
        }
    }
}

void modifyTrace(Trace& trace, const Trace& traceAnother,
    TraceModifierEventNotifier& notifier)
{
    map<Trace::FileGroupID, Trace::FileGroupInfo*>::const_iterator
        groupIterAnother = traceAnother.fileGroups.begin(),
        groupIterAnotherEnd = traceAnother.fileGroups.end();
    for(;groupIterAnother != groupIterAnotherEnd; ++groupIterAnother)
    {
        if(notifier.onFileGroupStart(*groupIterAnother)) continue;
        
        map<Trace::FileGroupID, Trace::FileGroupInfo*>::iterator
            groupIter = trace.fileGroups.find(groupIterAnother->first);
        if(groupIter == trace.fileGroups.end())
        {
            notifier.onFileGroupEndNew();
            continue;
        }
        
        map<string, Trace::FileInfo>::const_iterator
            sourceIterAnother = groupIterAnother->second->files.begin(),
            sourceIterAnotherEnd = groupIterAnother->second->files.end();
        for(; sourceIterAnother != sourceIterAnotherEnd; ++sourceIterAnother)
        {
            if(notifier.onSourceStart(*sourceIterAnother)) continue;
            
            map<string, Trace::FileInfo>::iterator
                sourceIter = groupIter->second->files.find(sourceIterAnother->first);

            if(sourceIterAnother == groupIterAnother->second->files.end())
            {
                notifier.onSourceEndNew();
                continue;
            }
            
            modifyFileInfo(sourceIter->second, sourceIterAnother->second,
                notifier);
            
            notifier.onSourceEnd();
        }
        
        notifier.onFileGroupEnd();
    }
}