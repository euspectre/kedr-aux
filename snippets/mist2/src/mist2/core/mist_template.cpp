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
    contextStack.push_back(MistParamNameAbs(vector<string>()));
    
    /* Build main template */
    MistTemplateGroupBlock* block = buildTemplateOrParamRef(mainTemplate);
    
    /* Remove element from context stack which we pushed before. */
    assert(contextStack.size() == 1);
    contextStack.clear();
    
    return block;
}

MistTemplateGroupBlock*
Template::Impl::Context::buildParameterRef(const MistTemplateName& name)
{
    if(!name.isRelative)
        return buildParameterRef(MistParamNameAbs(name.components));
    else
        return buildParameterRef(MistParamNameAbs(getGroupParamName(),
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
            contextIter = cache.contexts.find(contextStack.back());
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
    
    templateStack.push_back(contextStack.size());
    
    cache.inProgress = true;
    
    MistTemplateGroupBlock* block = cache.t->impl->createGroup(*this);
    /* Cache block created, cache place depends on 'useContext' flag */
    if(templateStack.back().useContext)
    {
        /* Per-context cache */
        cache.contexts.insert(make_pair(contextStack.back(),
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

const MistParamNameAbs& Template::Impl::Context::getGroupParamName(void)
{
    TemplateContextInfo& templateInfo = templateStack.back();
    templateInfo.useContext = templateInfo.useContext
        || (templateInfo.contextIndex == contextStack.size());
    return contextStack.back();
}
void Template::Impl::Context::pushGroupParamName(const MistTemplateName& paramName)
{
    if(!paramName.isRelative)
        contextStack.push_back(paramName.components);
    else
        contextStack.push_back(
            MistParamNameAbs(getGroupParamName(), paramName.components));
}
void Template::Impl::Context::popGroupParamName(void)
{
    contextStack.pop_back();
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
    MistTemplateGroupBlock* blockInternal =
        templateInternal->createGroup(templateContext);
    return new MistJoinGroup(blockInternal,
        templateContext.getGroupParamName(),
        textBetween);
}

/*********************** "With" sentence ******************************/
MistTemplateGroupBlock* MistTemplateWith::createGroup(
    Mist::Template::Impl::Context& templateContext) const
{
    templateContext.pushGroupParamName(context);
    MistTemplateGroupBlock* block = templateInternal->createGroup(templateContext);
    templateContext.popGroupParamName();
    
    return block;
}

/*********************** "RJoin" sentence *****************************/
MistTemplateGroupBlock* MistTemplateRJoin::createGroup(
    Mist::Template::Impl::Context& templateContext) const
{
    MistTemplateGroupBlock* blockInternal =
        templateInternal->createGroup(templateContext);
    return new MistRJoinGroup(blockInternal,
        templateContext.getGroupParamName(),
        textBetween);
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
