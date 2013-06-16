#include "mist_template.hh"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <sstream> /*for rjoin */
#include <cassert>

#include "mist_template_group.hh"

using namespace Mist;
using namespace std;

/****************** Context for build template group ******************/
Template::Impl::Context::Context(TemplateCollection& collection)
    : templateCollection(collection) {}

Template::Impl::Context::~Context()
{
    map<string, NamedBlockCached>::iterator iter = namedCache.begin(),
        iter_end = namedCache.end();
    for(;iter != iter_end; ++iter)
    {
        delete iter->second.t;
    }
}

MistTemplateGroupBlock*
Template::Impl::Context::build(const string& mainTemplate)
{
    /* Default 'with' scope is a root parameter(empty name). */
    baseParamStack.push_back(MistParamNameAbs(vector<string>()));
    baseParamStackSize = 1;
    
    /* 
     * Virtual root join context.
     * 
     * It is never iterated, and all subtemplates which are not iterated
     * with inner joins get index 0.
     */
    joinBaseStack.push_back(MistParamNameAbs(vector<string>()));
    joinBaseStackSize = 1;


    /* Build main template */
    MistTemplateGroupBlock* block = buildTemplateOrParamRef(mainTemplate);
    /* Remove element from stacks which we pushed at the start. */
    baseParamStack.pop_back();
    assert(baseParamStack.empty());
    
    joinBaseStack.pop_back();
    assert(joinBaseStack.empty());

    
    return block;
}

MistTemplateGroupBlock*
Template::Impl::Context::buildParameterRef(const MistTemplateName& name)
{
    if(!name.isRelative)
        return buildParameterRef(MistParamNameAbs(name.components));
    else
        return buildParameterRef(MistParamNameAbs(baseParamName(),
            name.components));
}


MistTemplateGroupBlock*
Template::Impl::Context::buildTemplateOrParamRef(const string& name)
{
    NamedBlockCached& cache = getNamedCache(name);
    
    if(cache.refGlobal)
    {
        /* Block is globally cached. Return it.*/
        return cache.refGlobal->ref();
    }
    else if(cache.t == NULL)
    {
        /*
         * Name refer to the parameter.
         * 
         * Extract block for it from paramCache, cache here and return.
         */
        MistTemplateGroupBlock* block = buildParameterRef(
            MistParamNameAbs(vector<string>(1, name)));

        cache.refGlobal = block->ref();

        return block;
    }
    else
    {
        /* 
         * Name refers to the template.
         * 
         * Check whether per-context cached block exists.
         */
        map<MistParamNameAbs, MistTemplateGroupBlockRef>::iterator
            contextIter = cache.contexts.find(baseParamStack.back());
        if(contextIter != cache.contexts.end())
        {
            /* Cache for given context already exists. Use it. */
            return contextIter->second->ref();
        }
    }
    /* Need to build new block for template */
    if(cache.inProgress)
    {
        /* Circular dependency detected! */
        throw logic_error("Circular templates dependency detected!");
    }
    
    templateStack.push_back(baseParamStackSize);
    
    cache.inProgress = true;
    
    MistTemplateGroupBlock* block = cache.t->impl->createGroup(*this);
    /* Cache block created, cache place depends on 'useContext' flag */
    if(templateStack.back().useContext)
    {
        /* Per-context cache */
        cache.contexts.insert(make_pair(baseParamStack.back(),
            MistTemplateGroupBlockRef(block->ref())));
    }
    else
    {
        /* Global cache */
        cache.refGlobal = block->ref();
    }
    
    cache.inProgress = false;
    
    templateStack.pop_back();
    
    return block;
}

Template* Template::Impl::Context::findTemplate(const string& name)
{
    NamedBlockCached& cache = getNamedCache(name);
    return cache.t;
}

Template::Impl::Context::NamedBlockCached&
    Template::Impl::Context::getNamedCache(const string& name)
{
    map<string, NamedBlockCached>::iterator iter = namedCache.find(name);
    if(iter != namedCache.end())
    {
        return iter->second;
    }
    else
    {
        /* New name */
        pair<map<string, NamedBlockCached>::iterator, bool> iterNew =
            namedCache.insert(make_pair(name, NamedBlockCached()));
        
        NamedBlockCached& cache = iterNew.first->second;
        
        Template* t = templateCollection.findTemplate(name);
        if(t)
        {
            /* Template with given name exists */
            cache.t = t;
        }
        return cache;
    }
}

MistTemplateGroupBlock*
Template::Impl::Context::buildParameterRef(const MistParamNameAbs& name)
{
    map<MistParamNameAbs, MistTemplateGroupBlockRef>::iterator iter =
        paramCache.find(name);
    if(iter != paramCache.end()) return iter->second->ref();
    /* Need to create cache entry. */
    MistTemplateGroupBlock* paramRef = new MistTemplateParamRefGroup(name);
    paramCache.insert(make_pair(name, paramRef->ref()));
    
    return paramRef;
}

const MistParamNameAbs& Template::Impl::Context::baseParamName(void)
{
    TemplateContextInfo& templateInfo = templateStack.back();
    templateInfo.useContext = templateInfo.useContext
        || (templateInfo.contextIndex == baseParamStackSize);
    return baseParamStack.back();
}
void Template::Impl::Context::pushBaseParam(const MistTemplateName& paramName)
{
    if(!paramName.isRelative)
        baseParamStack.push_back(paramName.components);
    else
        baseParamStack.push_back(
            MistParamNameAbs(baseParamName(), paramName.components));
    baseParamStackSize++;
}
void Template::Impl::Context::popBaseParam(void)
{
    baseParamStackSize--;
    baseParamStack.pop_back();
}

void Template::Impl::Context::beginJoinScope(void)
{
    joinBaseStack.push_back(baseParamName());
    joinBaseStackSize++;
}

void Template::Impl::Context::endJoinScope(void)
{
    joinBaseStackSize--;
    joinBaseStack.pop_back();
}


/*
 * Whether join with 'joinParamName' as base iterates 'paramName' parameter.
 * 
 * Auxiliary function for the next method implementation.
 */
bool joinUses(const MistParamNameAbs& joinParamName,
    const MistParamNameAbs& paramName)
{
    return (joinParamName.components.size() < paramName.components.size())
        && equal(joinParamName.components.begin(), joinParamName.components.end(),
            paramName.components.begin());
}

int Template::Impl::Context::iterationDepth(
    const MistParamNameAbs& paramName, int depth_upper) const
{
    int depth = 0;

    assert(depth_upper < (int)joinBaseStackSize);

    list<MistParamNameAbs>::const_reverse_iterator iter = joinBaseStack.rbegin();
    for(; depth < depth_upper; depth++, iter++)
    {
        if(joinUses(*iter, paramName)) return depth;
    }

    return depth_upper;
}

/*
 * Whether join with 'joinParamName' as base iterates at least one
 * parameter from given mask.
 * 
 * Auxiliary function for the next method implementation.
 */
bool joinUses(const MistParamNameAbs& joinParamName, const MistParamMask& mask)
{
    const MistParamMask* currentMask = &mask;
    
    for(vector<string>::const_iterator iter = joinParamName.components.begin();
        iter != joinParamName.components.end();
        ++iter)
    {
        MistParamMask::iterator maskIter = currentMask->begin();
        for(;maskIter != currentMask->end(); maskIter++)
        {
            if(maskIter->first == *iter) break;
        }
        
        if(maskIter == currentMask->end()) return false;
        currentMask = &maskIter->second;
    }
    
    // Check that mask contain at least one parameter inside join scope
    
    return currentMask->begin() != currentMask->end();
}

int Template::Impl::Context::iterationDepth(
    const MistParamMask& mask, int depth_upper) const
{
    int depth = 0;
    
    assert(depth_upper < (int)joinBaseStackSize);

    list<MistParamNameAbs>::const_reverse_iterator iter = joinBaseStack.rbegin();
    for(; depth < depth_upper; depth++, iter++)
    {
        if(joinUses(*iter, mask)) return depth;
    }
    
    return depth_upper;
}

int Template::Impl::Context::iterationDepthMax(void) const
{
    return joinBaseStackSize - 1;
}

/******************* Base template implementation**********************/
int Mist::Template::Impl::iterationDepth(Context& templateContext,
    int depth_upper) const
{
    MistTemplateGroupBlock* blockTmp = createGroup(templateContext);
    MistParamMask mask = blockTmp->getParamMask();
    blockTmp->unref();
    
    return templateContext.iterationDepth(mask, depth_upper);
}


/******************** Empty template  *********************************/
MistTemplateGroupBlock* MistTemplateEmpty::createGroup(
    Mist::Template::Impl::Context&) const
{
    return new MistEmptyGroup();
}

/********************** Sequence **************************************/
MistTemplateGroupBlock* MistTemplateSequence::createGroup(
    Mist::Template::Impl::Context& templateContext) const
{
    int size = subtemplates.size();
    /* Try to minimize count of elements. */
    if(size == 0)
        return new MistEmptyGroup();
    else if(size == 1)
        return subtemplates[0]->createGroup(templateContext);
    
    /* Two or more subtemplates */
    MistTemplateSequenceGroup* templateSequenceGroup =
        new MistTemplateSequenceGroup();
    
    for(int i = 0; i < size; i++)
        templateSequenceGroup->addTemplate(
            subtemplates[i]->createGroup(templateContext));

    return templateSequenceGroup;
}


/******************* Text *********************************************/
MistTemplateGroupBlock* MistTemplateText::createGroup(
    Mist::Template::Impl::Context&) const
{
    return new MistTextGroup(text);
}


/****************** Reference to template or parameter*****************/
MistTemplateGroupBlock* MistTemplateRef::createGroup(
    Mist::Template::Impl::Context& templateContext) const
{
    if(!name.isRelative && (name.components.size() == 1))
        return templateContext.buildTemplateOrParamRef(name.components[0]);
    else
        return templateContext.buildParameterRef(name);
}

/******************** "If" sentence ***********************************/
MistTemplateGroupBlock* MistTemplateIf::createGroup(
    Mist::Template::Impl::Context& templateContext) const
{
    return new MistIfGroup(conditionTemplate->createGroup(templateContext),
        positiveTemplate->createGroup(templateContext),
        elseTemplate->createGroup(templateContext));
}

/********************* "Join" sentence ********************************/
MistTemplateGroupBlock* MistTemplateJoin::createGroup(
    Mist::Template::Impl::Context& templateContext) const
{
    templateContext.beginJoinScope();
    MistTemplateGroupBlock* blockInternal =
        templateInternal->createGroup(templateContext);
    templateContext.endJoinScope();
    return new MistJoinGroup(blockInternal,
        templateContext.baseParamName(),
        textBetween);
}

/*********************** "With" sentence ******************************/
MistTemplateGroupBlock* MistTemplateWith::createGroup(
    Mist::Template::Impl::Context& templateContext) const
{
    templateContext.pushBaseParam(context);
    MistTemplateGroupBlock* block = templateInternal->createGroup(templateContext);
    templateContext.popBaseParam();
    
    return block;
}

/*********************** "RJoin" sentence *****************************/
MistTemplateGroupBlock* MistTemplateRJoin::createGroup(
    Mist::Template::Impl::Context& templateContext) const
{
    templateContext.beginJoinScope();
    MistTemplateGroupBlock* blockInternal =
        templateInternal->createGroup(templateContext);
    templateContext.endJoinScope();
    return new MistRJoinGroup(blockInternal,
        templateContext.baseParamName(),
        textBetween);
}

/************************** Index of iteration ************************/
class MistIndexGroup0: public MistIndexGroup
{
public:
    MistIndexGroup0(int depth ): MistIndexGroup(depth) {}
    
    ostream& printFormatted(int index, ostream& os) const {return os << index;}
};

MistTemplateGroupBlock* MistTemplateIndex0::createGroup(
    Mist::Template::Impl::Context& templateContext) const
{
    int depth = templateInternal->iterationDepth(templateContext,
        templateContext.iterationDepthMax());
    
    return new MistIndexGroup0(depth);
}

class MistIndexGroup1: public MistIndexGroup
{
public:
    MistIndexGroup1(int depth ): MistIndexGroup(depth) {}
    
    ostream& printFormatted(int index, ostream& os) const {return os << (index + 1);}
};


MistTemplateGroupBlock* MistTemplateIndex1::createGroup(
    Mist::Template::Impl::Context& templateContext) const
{
    int depth = templateInternal->iterationDepth(templateContext,
        templateContext.iterationDepthMax());
    
    return new MistIndexGroup1(depth);
}


/********************* Indent functionality ***************************/
MistTemplateGroupBlock* MistTemplateIndent::createGroup(
    Mist::Template::Impl::Context& templateContext) const
{
    MistTemplateGroupBlock* block = templateInternal->createGroup(templateContext);
    if(!indent.empty())
        block = new MistIndentGroup(block, indent);
    
    return block;
}
