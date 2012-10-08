#include <mist2/mist.hh>

#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cassert>

#include "mist_template_group.hh"
#include "mist_param_set_slice.hh"

using namespace std;
using namespace Mist;


/**********************************************************************/
MistTemplateGroupBlockRef::MistTemplateGroupBlockRef(
    MistTemplateGroupBlock* block) : block(block) {}
MistTemplateGroupBlockRef::MistTemplateGroupBlockRef(
    const MistTemplateGroupBlockRef& ref) : block(ref.block)
{
    if(block) block->ref();
}
MistTemplateGroupBlockRef::~MistTemplateGroupBlockRef()
{
    if(block) block->unref();
}
    
MistTemplateGroupBlockRef& MistTemplateGroupBlockRef::operator=(
    const MistTemplateGroupBlockRef& ref)
{
    if(block != ref.block)
    {
        if(block) block->unref();
        block = ref.block;
        if(block) block->ref();
    }
    return *this;
}

/* Template group constructor */
Mist::TemplateGroup::TemplateGroup(TemplateCollection& templateCollection,
    const std::string& mainTemplateName): impl(NULL)
{
    Builder groupBuilder(templateCollection);
    
    Template* mainTemplate = groupBuilder.findTemplate(mainTemplateName);
    if(mainTemplate == NULL)
    {
        cerr << "Template collection has no main template with name "
            << mainTemplateName << "." << endl;
        throw std::logic_error("Main template is absent");
    }
    
    MistTemplateGroupBlock* block = groupBuilder.build(mainTemplateName);
    
    impl = new Impl(block);
}

Mist::TemplateGroup::~TemplateGroup()
{
    delete impl;
}

std::ostream& Mist::TemplateGroup::instantiate(std::ostream& os, const ParamSet& paramSet)
{
    ParamSetSlice mainSlice(paramSet, impl->paramMask);
    if(!mainSlice.isSetLast())
    {
        cerr << "Main template is multivalued." << endl;
        throw std::logic_error("Main template is multivalued");
    }
    
    return impl->templateGroupBlockRef->evaluate(mainSlice, os);
}

std::string Mist::TemplateGroup::instantiate(const ParamSet& paramSet)
{
    ostringstream os;
    
    instantiate(os, paramSet);
    return os.str();
}

/**********************************************************************/
TemplateGroup::Impl::Impl(MistTemplateGroupBlock* templateGroupBlock)
    : templateGroupBlockRef(templateGroupBlock),
    paramMask(templateGroupBlock->getParamMask())
{
}


/**********************************************************************/
class MistTemplateParamRefGroup: public MistTemplateGroupBlock
{
public:
    const MistParamNameAbs name;
                    
    MistTemplateParamRefGroup(const MistParamNameAbs& name): name(name) {}
    
    
    ostream& evaluate(const ParamSetSlice& slice, ostream& os) const
    {
        const ParamSetSlice& paramSlice = slice.getSubslice(name);
        
        os << paramSlice.getValue();
        return os;
    }

    bool isEmpty(const ParamSetSlice& slice) const
    {
        const ParamSetSlice& paramSlice = slice.getSubslice(name);
        
        return paramSlice.getValue().empty();
    }

    MistParamMask getParamMask() const {return MistParamMask(name);}
    MistParamMask getParamMaskAll() const {return MistParamMask(name);}
};


/****************** Builder of templates group ************************/
TemplateGroup::Builder::Builder(TemplateCollection& collection)
    : templateCollection(collection) {}

TemplateGroup::Builder::~Builder()
{
    map<string, NamedBlockCached>::iterator iter = namedCache.begin(),
        iter_end = namedCache.end();
    for(;iter != iter_end; ++iter)
    {
        delete iter->second.t;
    }
}

MistTemplateGroupBlock*
TemplateGroup::Builder::build(const string& mainTemplate)
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
TemplateGroup::Builder::buildParameterRef(const MistTemplateName& name)
{
    if(!name.isRelative)
        return buildParameterRef(MistParamNameAbs(name.components));
    else
        return buildParameterRef(MistParamNameAbs(getGroupParamName(),
            name.components));
}


MistTemplateGroupBlock*
TemplateGroup::Builder::buildTemplateOrParamRef(const string& name)
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

Template* TemplateGroup::Builder::findTemplate(const string& name)
{
    NamedBlockCached& cache = getNamedCache(name);
    return cache.t;
}

TemplateGroup::Builder::NamedBlockCached&
    TemplateGroup::Builder::getNamedCache(const string& name)
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
TemplateGroup::Builder::buildParameterRef(const MistParamNameAbs& name)
{
    map<MistParamNameAbs, MistTemplateGroupBlockRef>::iterator iter =
        paramCache.find(name);
    if(iter != paramCache.end()) return iter->second->ref();
    /* Need to create cache entry. */
    MistTemplateGroupBlock* paramRef = new MistTemplateParamRefGroup(name);
    paramCache.insert(make_pair(name, paramRef->ref()));
    
    return paramRef;
}

const MistParamNameAbs& TemplateGroup::Builder::getGroupParamName(void)
{
    TemplateContextInfo& templateInfo = templateStack.back();
    templateInfo.useContext = templateInfo.useContext
        || (templateInfo.contextIndex == contextStack.size());
    return contextStack.back();
}
void TemplateGroup::Builder::pushGroupParamName(const MistTemplateName& paramName)
{
    if(!paramName.isRelative)
        contextStack.push_back(paramName.components);
    else
        contextStack.push_back(
            MistParamNameAbs(getGroupParamName(), paramName.components));
}
void TemplateGroup::Builder::popGroupParamName(void)
{
    contextStack.pop_back();
}