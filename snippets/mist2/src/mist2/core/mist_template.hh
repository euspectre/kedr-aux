/*
 * Internals of template object.
 * 
 * Used when compose templates into group.
 */

#ifndef MIST_TEMPLATE_HH
#define MIST_TEMPLATE_HH

#include <mist2/mist.hh>
#include <iostream>
#include <stdexcept>

#include <vector>
#include <memory> /* auto_ptr */

#include "mist_template_name.hh"
#include "mist_template_group.hh"

class MistTemplateGroupBlock;

/* Abstract template implementation class */
class Mist::Template::Impl
{
public:
    virtual ~Impl(){}
    
    // Context for instantiate template into template group.
    class Context;
    
    /* Instantiate template into group */
    virtual MistTemplateGroupBlock* createGroup(
        Context& templateContext) const = 0;
    /* 
     * Return minimum depth of join iteration which iterate parameters
     * used by template.
     * 
     * 'depth_upper' denote maximum depth to return. So, if none of
     * (depth_upper - 1) iterations use template parameters, depth_upper
     * should be return.
     * 
     * Default implementation uses getParamsMask() method of temporary
     * created group.
     */
     virtual int iterationDepth(Context& templateContext,
        int depth_upper) const;
};


/* 
 * Builder of template group from templates.
 */
class Mist::Template::Impl::Context
{
public:
    Context(TemplateCollection& templateCollection);
    ~Context();
    
    MistTemplateGroupBlock* build(const std::string& mainTemplate);

    /* Find template by name. Return NULL if not found. */
    Template* findTemplate(const std::string& name);
    
    /*
     * Build template group block for template or parameter with given
     * name.
     * 
     * Detect circular dependences between templates,
     * and throw exception in case of them.
     * 
     * Also cache blocks, created for templates and parameters.
     */
    MistTemplateGroupBlock* buildTemplateOrParamRef(const std::string& name);
    
    /* 
     * Build template group block for parameter reference. 
     * 
     * Name of the parameter may be relative or absolute.
     */
    MistTemplateGroupBlock* buildParameterRef(const MistTemplateName& name);

    /* 
     * Return parameter name which should be used as based for relative
     * names.
     * 
     * NOTE: This method should be used only from template's createGroup()
     * method and only when it is needed.
     * 
     * For template which doesn't use this method its group instantiation
     * will used with any groupParam, otherwise for every groupParam
     * different instantiation will be created.
     */
    const MistParamNameAbs& baseParamName(void);
    /* 
     * Set parameter name which should be used as based for relative
     * parameters.
     */
    void pushBaseParam(const MistTemplateName& paramName);
    /* 
     * Restore parameter name which should be used as based for relative
     * parameters statements.
     */
    void popBaseParam(void);
    
    /* Begin scope for joined template. */
    void beginJoinScope(void);
    /* End scope for joined template. */
    void endJoinScope(void);
    
    /*
     * Return minimum depth of iteration which uses given parameter.
     * 
     * 'depth_upper' bounds value returned.
     */
    int iterationDepth(const MistParamNameAbs& paramName, int depth_upper) const;
    /*
     * Return minimum depth of iteration which uses one of given parameters.
     * 
     * 'depth_upper' bounds value returned.
     */
    int iterationDepth(const MistParamMask& mask, int depth_upper) const;

    /*
     * Return maximum depth of current iterations.
     * 
     * Normally returned value is used as first 'guess' for call
     * iterationDepth().
     */
     int iterationDepthMax(void) const;

private:
    TemplateCollection& templateCollection;

    /* Cache for parameter references */
    std::map<MistParamNameAbs, MistTemplateGroupBlockRef> paramCache;
    
    /* Cache for name, corresponded to template or parameter. */
    struct NamedBlockCached
    {
        /* 
         * Context-independed block if exist. Name which correspond to
         * parameter always has such block.
         * 
         * Template has such block only if it doesn't use context
         * in which it processed.
         */
        MistTemplateGroupBlockRef refGlobal;
        /* Block for every 'with' context */
        std::map<MistParamNameAbs, MistTemplateGroupBlockRef> contexts;
        /* 
         * If given name corresponds to template, this is reference to it.
         * Otherwise NULL.
         */
        Template* t;
        /* 
         * True if template is currently being instantiated.
         * 
         * Attempt to instantiate new instance while this flag is true
         * means cyclic dependencies between templates.
         */
        bool inProgress;
        
        NamedBlockCached(void): t(NULL), inProgress(false) {}
    };

    /* Cached named blocks */
    std::map<std::string, NamedBlockCached> namedCache;
    
    /* Stack of the base parameter names (for 'with' context) */
    std::list<MistParamNameAbs> baseParamStack;
    /* Size of baseParamStack. */
    size_t baseParamStackSize;
    /* Stack of the bases for join */
    std::list<MistParamNameAbs> joinBaseStack;
    /* Size of joinBaseStack, used for iterationDepthMax(). */
    size_t joinBaseStackSize;

    /* Information about template which is currently instantiated */
    struct TemplateContextInfo
    {
        /* 
         * Whether currently instantiated template use outer context for
         * join/concat statements.
         * 
         * If at the end of instantiating template this parameter is true,
         * then resulted block will be stored in per-context map.
         * Otherwise it will be stored globally.
         */
        bool useContext;
        /* 
         * Index of context (in stack) with which template begins
         * instantiate.
         * 
         * The thing is that, template may itself setup context and use
         * it.
         * 
         * Such usage doesn't affect on global status of block created.
         */
        size_t contextIndex;

        TemplateContextInfo(size_t contextIndex): useContext(false),
            contextIndex(contextIndex) {}
    };
    
    /* Stack of the instantiated templates */
    std::vector<TemplateContextInfo> templateStack;
    /*
     * Return named cache for given name. Cache will be created if needed.
     */
    NamedBlockCached& getNamedCache(const std::string& name);
    
    MistTemplateGroupBlock* buildParameterRef(const MistParamNameAbs& name);
};


/* Template contained sequence of zero or more subtemplates */
class MistTemplateSequence: public Mist::Template::Impl
{
public:
    std::vector<Mist::Template::Impl*> subtemplates;
    
    ~MistTemplateSequence();
    
    void addTemplate(Mist::Template::Impl* subtemplate)
        {subtemplates.push_back(subtemplate);}
    
    MistTemplateGroupBlock* createGroup(
        Context& templateContext) const;
};

/* Empty template. */
class MistTemplateEmpty: public Mist::Template::Impl
{
public:
    MistTemplateGroupBlock* createGroup(
        Context& templateContext) const;
};


/* Simple text string, without directives */
class MistTemplateText: public Mist::Template::Impl
{
public:
    std::string text;
    
    MistTemplateText(const std::string& text) : text(text) {}
    
    MistTemplateGroupBlock* createGroup(
        Context& templateContext) const;
};

/* Reference(by name) to template or parameter */
class MistTemplateRef: public Mist::Template::Impl
{
public:
    MistTemplateName name;
    
    MistTemplateRef(const MistTemplateName& name):
        name(name) {}
    
    MistTemplateGroupBlock* createGroup(
        Context& templateContext) const;
};

/* 
 * If statement.
 * 
 * In general, this is switch between two templates according to
 * emptiness property of the third one.
 */
class MistTemplateIf: public Mist::Template::Impl
{
public:
    /* 
     * Normally, there is only one condition in if statement
     * (without 'elseif').
     * 
     * So do not disturb with condition part arrays - them can be
     * emulated with cascading.
     */

    std::auto_ptr<Mist::Template::Impl> conditionTemplate;
    std::auto_ptr<Mist::Template::Impl> positiveTemplate;
    /* May not be NULL */
    std::auto_ptr<Mist::Template::Impl> elseTemplate;
    
    MistTemplateIf(Mist::Template::Impl* conditionTemplate,
        Mist::Template::Impl* positiveTemplate,
        Mist::Template::Impl* elseTemplate):
            conditionTemplate(conditionTemplate),
            positiveTemplate(positiveTemplate),
            elseTemplate(elseTemplate) {}

    MistTemplateGroupBlock* createGroup(
        Context& templateContext) const;
};

/* 
 * Join statement.
 * 
 * In general, repeat some template and insert some text between
 * repetitions.
 */
class MistTemplateJoin: public Mist::Template::Impl
{
public:
    std::auto_ptr<Mist::Template::Impl> templateInternal;
    std::string textBetween;
    
    MistTemplateJoin(Mist::Template::Impl* templateInternal,
        const std::string& textBetween)
        : templateInternal(templateInternal), textBetween(textBetween) {}
    
    MistTemplateGroupBlock* createGroup(
        Context& templateContext) const;
};

/* 
 * With statement.
 * 
 * Inner template in some context, which should be used with join
 * routin and as base name for relative parameter names.
 */
class MistTemplateWith: public Mist::Template::Impl
{
public:
    std::auto_ptr<Mist::Template::Impl> templateInternal;
    MistTemplateName context;
    
    MistTemplateWith(Mist::Template::Impl* templateInternal,
        const MistTemplateName& context):
        templateInternal(templateInternal), context(context) {}
    
    MistTemplateGroupBlock* createGroup(
        Context& templateContext) const;
};


/* 
 * Reverse join statement.
 * 
 * Like join, repeat some template and insert some text between
 * repetitions. But repetitions is written in reverse order.
 */
class MistTemplateRJoin: public MistTemplateJoin
{
public:
    MistTemplateRJoin(Mist::Template::Impl* templateInternal,
        const std::string& textBetween)
        : MistTemplateJoin(templateInternal, textBetween) {}
    
    MistTemplateGroupBlock* createGroup(
        Context& templateContext) const;
};

/*
 * Index of join iteration.
 */

class MistTemplateIndex0: public Mist::Template::Impl
{
public:
    std::auto_ptr<Mist::Template::Impl> templateInternal;

    MistTemplateIndex0(Mist::Template::Impl* templateInternal):
        templateInternal(templateInternal) {}

    MistTemplateGroupBlock* createGroup(
        Context& templateContext) const;
};

class MistTemplateIndex1: public Mist::Template::Impl
{
public:
    std::auto_ptr<Mist::Template::Impl> templateInternal;

    MistTemplateIndex1(Mist::Template::Impl* templateInternal):
        templateInternal(templateInternal) {}

    MistTemplateGroupBlock* createGroup(
        Context& templateContext) const;
};

/*
 * Indent functionality.
 */
class MistTemplateIndent: public Mist::Template::Impl
{
public:
    std::auto_ptr<Mist::Template::Impl> templateInternal;
    std::string indent;
    
    MistTemplateIndent(Mist::Template::Impl* templateInternal,
        const std::string& indent)
        : templateInternal(templateInternal), indent(indent) {}
    
    MistTemplateGroupBlock* createGroup(
        Context& templateContext) const;
};

#endif /* MIST_TEMPLATE_HH */
