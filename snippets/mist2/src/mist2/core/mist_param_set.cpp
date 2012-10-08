/*
 * Implementation of parameters set and its slice.
 */

#include <mist2/mist.hh>
#include "mist_param_set_slice.hh"
#include "mist_template.hh"

#include <stdexcept>
#include <vector>
#include <algorithm>
#include <map>
#include <cassert>

using namespace Mist;
using namespace std;

/***************** Parameter set implementation ***********************/
/* Shared set for absent values */
static ParamSet emptySet;
/* Shared array of parameter values, corresponded to absent names. */
static const vector<ParamSet*> emptyValues(1, &emptySet);

Mist::ParamSet::ParamSet(): value("") {}

Mist::ParamSet::~ParamSet()
{
    map<string, vector<ParamSet*> >::iterator iter = subsets.begin(),
        iter_end = subsets.end();
    for(;iter != iter_end; ++iter)
    {
        vector<ParamSet*>& sets = iter->second;
        int setsSize = (int)sets.size();
        
        for(int i = 0; i < setsSize; i++) delete sets[i];
    }
}

const std::vector<ParamSet*>& Mist::ParamSet::getSubsets(
    const std::string& name) const
{
    map<string, vector<ParamSet*> >::const_iterator iter = subsets.find(name);
    if(iter != subsets.end())
        return iter->second;
    else
        return emptyValues;
}


/* Empty array of parameter values. Only for intermediate use. */
static const vector<ParamSet*> zeroValues;

ParamSet* Mist::ParamSet::addSubset(const std::string& name)
{
    ParamSet* subset = new ParamSet();
    map<string, vector<ParamSet*> >::iterator iter = subsets.find(name);
    if(iter != subsets.end())
    {
        iter->second.push_back(subset);
    }
    else
    {
        pair<map<string, vector<ParamSet*> >::iterator, bool> iterNew =
            subsets.insert(make_pair(name, zeroValues));
        iterNew.first->second.push_back(subset);
    }
    return subset;
}

void Mist::ParamSet::setValue(const string& value)
{
    this->value = value;
}

const std::string& Mist::ParamSet::getValue(void) const
{
    return value;
}

Mist::ParamSet::ParamSet(const ParamSet& paramSet)
    : value(paramSet.value), subsets(paramSet.subsets)
{
    /* Copy subsets. */
    map<string, vector<ParamSet*> >::iterator iter = subsets.begin(),
        iter_end = subsets.end();
    for(;iter != iter_end; ++iter)
    {
        vector<ParamSet*>& sets = iter->second;
        int setsSize = (int)sets.size();
        
        for(int i = 0; i < setsSize; i++) sets[i] = new ParamSet(*sets[i]);
    }
}


Mist::ParamSet& Mist::ParamSet::operator=(
    const ParamSet& paramSet)
{
    /* Delete current subsets. */
    map<string, vector<ParamSet*> >::iterator iter = subsets.begin(),
        iter_end = subsets.end();
    for(;iter != iter_end; ++iter)
    {
        vector<ParamSet*>& sets = iter->second;
        int setsSize = (int)sets.size();
        
        for(int i = 0; i < setsSize; i++) delete sets[i];
    }
    /* Copy value and map*/
    value = paramSet.value;
    subsets = paramSet.subsets;
    
    /* And copy subsets. */
    map<string, vector<ParamSet*> >::iterator iter1 = subsets.begin(),
        iter1_end = subsets.end();
    for(;iter1 != iter1_end; ++iter1)
    {
        vector<ParamSet*>& sets = iter1->second;
        int setsSize = (int)sets.size();
        
        for(int i = 0; i < setsSize; i++) delete sets[i];
    }
    
    return *this;
}

/******************* Parameters mask implementation *******************/
class MistParamMask::Impl
{
public:
    Impl(): refs(1) {}

    void ref(void) {++refs;}
    void unref(void) {if(--refs == 0) delete this;}
    
    bool isShared(void) {return refs != 1;}
    
    /* 
     * Make copy of implementation, which has reference count 1 and
     * may be used independently from others.
     * 
     * Return implementation created and unref current one, so it
     * shouldn't be used after this.
     * 
     * NOTE: Only object itself is copied, subobjects will be simply
     * refferenced.
     * 
     * This function is useful when need to change object, buts its
     * refcount is more than 1(that is, it is used by someone else).
     */
    Impl* makePrivate(void);

    vector<pair<string, MistParamMask> > submasks;
private:
    int refs;
    ~Impl() {}
};

MistParamMask::Impl* MistParamMask::Impl::makePrivate(void)
{
    Impl* impl = new Impl();
    /* Copy array of submasks. */
    impl->submasks = this->submasks;
    this->unref();
    return impl;
}

MistParamMask::MistParamMask(void): impl(new Impl()) {}

MistParamMask::MistParamMask(Impl* impl): impl(impl) {}

MistParamMask::MistParamMask(const MistParamMask& mask): impl(mask.impl)
{
    /* Copy-on-write semantic */
    impl->ref();
}

MistParamMask& MistParamMask::operator=(const MistParamMask& mask)
{
    if(impl != mask.impl)
    {
        impl->unref();
        impl = mask.impl;
        impl->ref();
    }
    return *this;
}


MistParamMask::~MistParamMask()
{
    impl->unref();
}

MistParamMask::MistParamMask(const MistParamNameAbs& name)
    : impl(new Impl())
{
    int nameSize = (int)name.components.size();
    Impl* currentImpl = impl;
    for(int i = 0; i < nameSize; i++)
    {
        Impl* subimpl = new Impl();
        /* Implementation is private, so we can safetly modify it. */
        currentImpl->submasks.push_back(make_pair(name.components[i],
            MistParamMask(subimpl)));
        currentImpl = subimpl;
    }
}


MistParamMask::Modifier::Modifier(MistParamMask& topMask)
    : maskStack(1, &topMask) {}

const MistParamMask& MistParamMask::Modifier::current(void) const
{
    return *maskStack.back();
}

void MistParamMask::Modifier::moveChild(iterator& iter)
{
    vector<pair<string, MistParamMask> >& submasks =
        maskStack.back()->impl->submasks;
    
    int index = iter - submasks.begin();
    assert((index >= 0) && (index - submasks.size()));
    
    maskStack.push_back(&submasks[index].second);
    indiciesStack.push_back(index);
}

void MistParamMask::Modifier::moveParent(void)
{
    indiciesStack.pop_back();
    maskStack.pop_back();
}

void MistParamMask::Modifier::prepareForModify(void)
{
    assert(maskStack.size() > 0);
    int maskStackSize = maskStack.size();
    int i;

    /* Find first shared implementation in the stack. */
    for(i = 0; i < maskStackSize; i++)
    {
        if(maskStack[i]->impl->isShared()) break;
    }
    
    if(i < maskStackSize)
    {
        /* Make this implementation and all after it private */
        maskStack[i]->impl = maskStack[i]->impl->makePrivate();
        /* 
         * Make all implementations after this private.
         * 
         * NB: If mask is copied, all its child masks has refcounts
         * incremented, so them cannot be private in any case.
         */
        for(++i; i < maskStackSize; i++)
        {
            /* Recalculate mask pointer, because upper one has changed */
            int index = indiciesStack[i - 1];
            maskStack[i] = &maskStack[i - 1]->impl->submasks[index].second;
            
            maskStack[i]->impl = maskStack[i]->impl->makePrivate();
        }
    }
    /* Now current mask is private with garantee and may be changed. */
}


void MistParamMask::Modifier::addSubmask(const string& name,
    const MistParamMask& submask)
{
    prepareForModify();
    maskStack.back()->impl->submasks.push_back(make_pair(name, submask));
}
void MistParamMask::Modifier::removeSubmasks(void)
{
    prepareForModify();
    maskStack.back()->impl->submasks.clear();
}



void MistParamMask::cut(const MistParamNameAbs& name)
{
    Modifier modifier(*this);
    
    int name_size = (int)name.components.size();
    /* Adjust modifier to given parameter... */
    for(int i = 0; i < name_size; i++)
    {
        const string& subname = name.components[i];
        iterator iter = modifier.current().begin(),
            iter_end = modifier.current().end();
        for(;iter != iter_end; ++iter)
        {
            if(iter->first == subname) break;
        }
        assert(iter != iter_end);
        
        modifier.moveChild(iter);
    }
    /* ... and remove all subparameters */
    modifier.removeSubmasks();
}

void MistParamMask::append(const MistParamMask& mask)
{
    Modifier modifier(*this);
    /* 
     * Prepare for linear(non-recursive) tree-traversal. 
     * 
     * Stack of pairs of <current iter, eof iter> per layer.
     */
    vector<pair<iterator, iterator> > iteratorStack(1,
        make_pair(mask.begin(), mask.end()));
    
    while(true)
    {
        iterator currentIter = iteratorStack.back().first;
        
        if(currentIter == iteratorStack.back().second)
        {
            /* EOF child, move to parent*/
            iteratorStack.pop_back();

            if(iteratorStack.empty()) break;/* Traversal finished. */
            
            /* Next (parent) */
            ++iteratorStack.back().first;
            modifier.moveParent();
        
            continue;
        }
        
        iterator iter = modifier.current().begin(),
            iter_end = modifier.current().end();
        for(;iter != iter_end; ++iter)
        {
            if(iter->first == currentIter->first)
            {
                break;
            }
        }
        if(iter != iter_end)
        {
            /* Found submask with given name. Descent into it(its children). */
            iteratorStack.push_back(make_pair(currentIter->second.begin(),
                currentIter->second.end()));
            modifier.moveChild(iter);
        }
        else
        {
            /* Did not find submask with given name. Need to add it. */
            modifier.addSubmask(currentIter->first, currentIter->second);
            /* Next sibling */
            ++iteratorStack.back().first;
        }
    }
}


MistParamMask::iterator MistParamMask::begin(void) const
{
    return ((const Impl*)impl)->submasks.begin();
}
MistParamMask::iterator MistParamMask::end(void) const
{
    return ((const Impl*)impl)->submasks.end();
}

void MistParamMask::print(const string& tab) const
{
    iterator iter = begin(), iter_end = end();
    for(; iter != iter_end; ++iter)
    {
        cout << tab << iter->first << ":" << endl;
        iter->second.print(tab + "  ");
    }
}

const MistParamMask& MistParamMask::getSubmask(
    const MistParamNameAbs& name) const
{
    int nameSize = name.components.size();
    const MistParamMask* current = this;
    
    for(int i = 0; i < nameSize; i++)
    {
        const string& nameCurrent = name.components[i];
        iterator iter = current->begin(), iter_end = current->end();
        for(; iter != iter_end; ++iter)
        {
            if(iter->first == nameCurrent) break;
        }
        
        assert(iter != iter_end);
        
        current = &iter->second;
    }
    
    return *current;
}

/****************** Parameter set slice implementation ****************/
ParamSetSlice::ParamSetSlice(const ParamSet& paramSet,
    const MistParamMask& mask): value(&paramSet), isSetLastFlag(true)
{
    setMask(mask);
}

ParamSetSlice::ParamSetSlice(const ParamSetSlice& slice):
    value(slice.value), subslices(slice.subslices),
    isSetLastFlag(slice.isSetLastFlag)
{
    int subslicesSize = subslices.size();
    for(int i = 0; i < subslicesSize; i++)
    {
        subslices[i].slice = new ParamSetSlice(*subslices[i].slice);
    }
}

ParamSetSlice::~ParamSetSlice()
{
    int subslicesSize = subslices.size();
    for(int i = 0; i < subslicesSize; i++)
    {
        delete subslices[i].slice;
    }
}

const string& ParamSetSlice::getValue(void) const
{
    return value->getValue();
}

ParamSetSlice& ParamSetSlice::getSubslice(const string& name)
{
    vector<Subslice>::iterator iter = subslices.begin(),
        iter_end = subslices.end();
    for(; iter != iter_end; ++iter)
    {
        if(iter->name == name) break;
    }
    
    assert(iter != iter_end);
    return *iter->slice;
}

const ParamSetSlice& ParamSetSlice::getSubslice(const string& name) const
{
    vector<Subslice>::const_iterator iter = subslices.begin(),
        iter_end = subslices.end();
    for(; iter != iter_end; ++iter)
    {
        if(iter->name == name) break;
    }
    
    assert(iter != iter_end);
    return *iter->slice;
}


ParamSetSlice& ParamSetSlice::getSubslice(const MistParamNameAbs& name)
{
    ParamSetSlice* current = this;
    int nameSize = name.components.size();
    for(int i = 0; i < nameSize; i++)
    {
        current = &current->getSubslice(name.components[i]);
    }
    
    return *current;
}

const ParamSetSlice& ParamSetSlice::getSubslice(const MistParamNameAbs& name) const
{
    const ParamSetSlice* current = this;
    int nameSize = name.components.size();
    for(int i = 0; i < nameSize; i++)
    {
        current = &current->getSubslice(name.components[i]);
    }
    
    return *current;
}


bool ParamSetSlice::isSetLast(void) const
{
    return isSetLastFlag;
}

void ParamSetSlice::nextSet(void)
{
    isSetLastFlag = true;

    vector<Subslice>::iterator iter = subslices.begin(),
        iter_end = subslices.end();
    for(; iter != iter_end; ++iter)
    {
        if(iter->slice->isSetLast())
        {
            /* Set of subslice is the last one.*/
            if((iter->index + 1) < iter->values->size())
            {
                /* But subslice is not last in array. Advance it. */
                ++iter->index;
                iter->slice->reset((*iter->values)[iter->index]);
            }
            else
            {
                /* Nothing to do */
                continue;
            }
        }
        else
        {
            /* Set of subslice may be advance. Do that. */
            iter->slice->nextSet();
        }
        
        updateSetLast(*iter);
    }
}

void ParamSetSlice::resetMask(const MistParamMask& mask)
{
    int subslicesSize = subslices.size();
    for(int i = 0; i < subslicesSize; i++)
    {
        delete subslices[i].slice;
    }

    subslices.clear();
    
    isSetLastFlag = true;
    
    setMask(mask);
}

ParamSetSlice::Subslice::Subslice(const string& name,
    const vector<ParamSet*>* values): name(name),
        values(values), index(0),
        slice(new ParamSetSlice(*(*values)[0]))
{
}

void ParamSetSlice::setMask(const MistParamMask& mask)
{
    MistParamMask::iterator iter = mask.begin(),
        iter_end = mask.end();
    for(; iter != iter_end; ++iter)
    {
        /* Add empty subslice */
        subslices.push_back(Subslice(iter->first,
            &value->getSubsets(iter->first)));
        Subslice& subslice = subslices.back();
        /* Fill it */
        subslice.slice->setMask(iter->second);

        updateSetLast(subslice);
    }
}

void ParamSetSlice::reset(const ParamSet* value)
{
    this->value = value;
    
    isSetLastFlag = true;
    
    vector<Subslice>::iterator iter = subslices.begin(),
        iter_end = subslices.end();
    for(; iter != iter_end; ++iter)
    {
        iter->index = 0;
        iter->values = &value->getSubsets(iter->name);
        iter->slice->reset((*iter->values)[iter->index]);
        
        updateSetLast(*iter);
    }
}

void ParamSetSlice::updateSetLast(const Subslice& subslice)
{
    isSetLastFlag = isSetLastFlag
        && ((subslice.index + 1) == subslice.values->size())
        && subslice.slice->isSetLast();
}

ParamSetSlice::ParamSetSlice(const ParamSet& paramSet): value(&paramSet),
    isSetLastFlag(true)
{
}
