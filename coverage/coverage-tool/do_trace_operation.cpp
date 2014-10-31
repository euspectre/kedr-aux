#include "do_trace_operation.hh"
#include <cassert>

using namespace std;

/* 
 * Apply given function for each vector of iterators in given maps.
 * 'maps' may contain NULL's.
 * 
 * Function is called as
 * 
 * f(const Key& key, const vector<Iterator>& iters, const vector<bool>& existence)
 * 
 * where existence[i] = false means that maps[i] has no value for given key,
 * and iters[i] iterator shouldn't be used.
 */
template<class MapClass, class Function>
void for_each_vector(const vector<MapClass*>& maps, Function f)
{
    typedef typename MapClass::key_type key_type;
    typedef typename MapClass::const_iterator const_iterator;
    
    int n = (int)maps.size();
    
    assert(n > 0);
    
    vector<const_iterator> iters(n);
    vector<const_iterator> iters_end(n);
    
    vector<bool> valid(n); /* iter[i] != iter_end[i] */
    
    /* 
     * First valid iterator( -1 means not found).
     * 
     * Used only at start.
     */
    int first_valid = -1;
    
   
    /* Setup iterators and 'valid' indicators*/
    for(int i = 0; i < n; i++)
    {
        if(!maps[i]) continue;

        iters[i] = maps[i]->begin();
        iters_end[i] = maps[i]->end();
        
        if(iters[i] != iters_end[i])
        {
            valid[i] = true;
            if(first_valid == -1)
                first_valid = i;
        }
    }
    
    if(first_valid == -1) return; /* All maps are empty. Nothing to do. */
    
    vector<bool> existence(n);
    
    /* Look for minimal key and fill existence array. */
    key_type key = iters[first_valid]->first;
    existence[first_valid] = true;
    // Whether key is valid (for current iteration).
    bool key_valid = true;
    
    for(int i = 0; i < n; i++)
    {
        if(!valid[i] || key < iters[i]->first) continue;
        
        existence[i] = true;

        if(iters[i]->first < key)
        {
            /* New minimal key found. */
            key = iters[i]->first;
            for(int j = 0; j < i; j++)
                existence[j] = false;
        }
    }
    
    /* Main iteration */
    do
    {
        f(key, iters, existence);
        key_valid = false;
        
        /* Advance only iters with 'existence' equal to true. */
        for(int i = 0; i < n; i++)
        {
            if(existence[i])
            {
                valid[i] = ++iters[i] != iters_end[i];
                existence[i] = false;
            }
            
            if(!valid[i]) continue;

            if(!key_valid)
            {
                existence[i] = true;
                key = iters[i]->first;
                key_valid = true;
            }
            else if(key < iters[i]->first) continue;
            
            existence[i] = true;
            
            if(iters[i]->first < key)
            {
                /* New minimal key found. */
                key = iters[i]->first;
                for(int j = 0; j < i; j++)
                    existence[j] = false;
            }
        }
    }
    while(key_valid);
};

class DoTraceOperation
{
public:
    DoTraceOperation(TraceOperation& op,
        const std::vector<Trace>& traceOperands, Trace& result):
        op(op),

        result(result),
        
        n(traceOperands.size()),
        
        fileGroupsMaps(n),
        filesMaps(n),
        functionsMaps(n),
        linesMaps(n),
        branchesMaps(n),
        operands(n)
    {
        for(int i = 0; i < n; i++)
            fileGroupsMaps[i] = &traceOperands[i].fileGroups;
    }
    
    void perform(void)
    {
        for_each_vector(fileGroupsMaps, *this);
    }
    
    void operator()(const Trace::FileGroupID& fileGroupID,
        const vector<map<Trace::FileGroupID, Trace::FileGroupInfo*>::const_iterator>& iters,
        const vector<bool>& existence)
    {
        /* Currently new group is created in any case. */
        pair<map<Trace::FileGroupID, Trace::FileGroupInfo*>::iterator, bool> newIter =
            result.fileGroups.insert(make_pair(fileGroupID, (Trace::FileGroupInfo*)0));
        assert(newIter.second);
        
        currentFileGroup = new Trace::FileGroupInfo();
        newIter.first->second = currentFileGroup;
        
        for(int i = 0; i < n; i++)
            filesMaps[i] = existence[i] ? &iters[i]->second->files : NULL;
        
        for_each_vector(filesMaps, *this);
    }
    
    void operator()(const string& filename,
        const vector<map<string, Trace::FileInfo>::const_iterator>& iters,
        const vector<bool>& existence)
    {
        /* Currently new file is created in any case. */
        pair<map<string, Trace::FileInfo>::iterator, bool> newIter =
            currentFileGroup->files.insert(make_pair(filename, Trace::FileInfo()));
        assert(newIter.second);
        
        currentFile = &newIter.first->second;
        
        for(int i = 0; i < n; i++)
        {
            functionsMaps[i] = existence[i] ? &iters[i]->second.functions : NULL;
            linesMaps[i] = existence[i] ? &iters[i]->second.lines : NULL;
            branchesMaps[i] = existence[i] ? &iters[i]->second.branches : NULL;
        }
        
        for_each_vector(functionsMaps, *this);
        for_each_vector(linesMaps, *this);
        for_each_vector(branchesMaps, *this);
    }
    
    void operator()(const string& functionName,
        const vector<map<string, Trace::FuncInfo>::const_iterator>& iters,
        const vector<bool>& existence)
    {
        /* Extract function's line from any iterator's value*/
        int line = -2; /* -1 is reserved for unknown line. */
        for(int i = 0; i < n; i++)
        {
            if(existence[i])
            {
                operands[i] = iters[i]->second.counter;
                if(line == -2) line = iters[i]->second.lineStart;
            }
            else
            {
                operands[i] = -1;
            }
        }
        
        Trace::counter_t newCounter = op.functionOperation(operands);
        
        if(newCounter >= 0)
        {
            Trace::FuncInfo funcInfo(line);
            funcInfo.counter = newCounter;
            currentFile->functions.insert(make_pair(functionName, funcInfo));
        }
    }
    
    void operator()(int lineNo,
        const vector<map<int, Trace::counter_t>::const_iterator>& iters,
        const vector<bool>& existence)
    {
        for(int i = 0; i < n; i++)
        {
            operands[i] = existence[i] ? iters[i]->second : -1;
        }
        
        Trace::counter_t newCounter = op.lineOperation(operands);
        
        if(newCounter >= 0)
        {
            currentFile->lines.insert(make_pair(lineNo, newCounter));
        }
    }

    void operator()(Trace::BranchID branchID,
        const vector<map<Trace::BranchID, Trace::counter_t>::const_iterator>& iters,
        const vector<bool>& existence)
    {
        for(int i = 0; i < n; i++)
        {
            operands[i] = existence[i] ?
                (iters[i]->second != -1 ? iters[i]->second : -2) : -1;
        }
        
        Trace::counter_t newCounter = op.branchOperation(operands);
        
        if(newCounter != -1)
        {
            currentFile->branches.insert(make_pair(branchID, newCounter != -2 ? newCounter : -1));
        }
    }

private:
    TraceOperation& op;
    Trace& result;
    
    int n;
    /* 
     * Temporary vectors of predefined size.
     * 
     * For minimize allocations.
     */
    vector<const map<Trace::FileGroupID, Trace::FileGroupInfo*>*> fileGroupsMaps;
    vector<const map<string, Trace::FileInfo>*> filesMaps;
    vector<const map<string, Trace::FuncInfo>*> functionsMaps;
    vector<const map<int, Trace::counter_t>*> linesMaps;
    vector<const map<Trace::BranchID, Trace::counter_t>*> branchesMaps;

    vector<TraceOperation::counter_t> operands;
    
    Trace::FileGroupInfo* currentFileGroup;
    Trace::FileInfo* currentFile;
};

void doTraceOperation(TraceOperation& op,
    const std::vector<Trace>& operands, Trace& result)
{
    DoTraceOperation p(op, operands, result);
    
    p.perform();
}
