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

/* Name of template or parameter */
struct MistTemplateName
{
    std::vector<std::string> components;
    bool isRelative;
    
    MistTemplateName(const std::vector<std::string>& components,
        bool isRelative = false)
        : components(components), isRelative(isRelative) {}
};

/* 
 * Name of the parameter.
 * 
 * Always absolute(as opposed to MistTemplateName).
 */
struct MistParamNameAbs
{
    /* Names components. */
    std::vector<std::string> components;
    
    MistParamNameAbs(const std::vector<std::string>& components)
        : components(components) {}
    /* Construct absolute name from base and relative name. */
    MistParamNameAbs(const MistParamNameAbs& base,
        const std::vector<std::string> relComponents)
        : components(base.components)
    {
        components.insert(components.end(),
            relComponents.begin(), relComponents.end());
    }
   
    /* For using name as key of mapping */
    bool operator<(const struct MistParamNameAbs& name) const;
};

#include "mist_template_group.hh"

using namespace std;
using namespace Mist;

/* Abstract template implementation class */
class Template::Impl
{
public:
    virtual ~Impl(){}
    
    /* Instantiate template into group */
    virtual MistTemplateGroupBlock* createGroup(
        Mist::TemplateGroup::Builder& groupBuilder) const = 0;
};



/* Template contained sequence of zero or more subtemplates */
class MistTemplateSequence: public Template::Impl
{
public:
    vector<Template::Impl*> subtemplates;
    
    ~MistTemplateSequence();
    
    void addTemplate(Template::Impl* subtemplate)
        {subtemplates.push_back(subtemplate);}
    
    MistTemplateGroupBlock* createGroup(
        Mist::TemplateGroup::Builder& groupBuilder) const;
};

/* Empty template. */
class MistTemplateEmpty: public Template::Impl
{
public:
    MistTemplateGroupBlock* createGroup(
        Mist::TemplateGroup::Builder& groupBuilder) const;
};


/* Simple text string, without directives */
class MistTemplateText: public Template::Impl
{
public:
    string text;
    
    MistTemplateText(const string& text) : text(text) {}
    
    MistTemplateGroupBlock* createGroup(
        Mist::TemplateGroup::Builder& groupBuilder) const;
};

/* Reference(by name) to template or parameter */
class MistTemplateRef: public Template::Impl
{
public:
    MistTemplateName name;
    
    MistTemplateRef(const MistTemplateName& name):
        name(name) {}
    
    MistTemplateGroupBlock* createGroup(
        Mist::TemplateGroup::Builder& groupBuilder) const;
};

/* 
 * If statement.
 * 
 * In general, this is switch between two templates according to
 * emptiness propertie of the third one.
 */
class MistTemplateIf: public Template::Impl
{
public:
    auto_ptr<Template::Impl> conditionTemplate;
    auto_ptr<Template::Impl> ifTemplate;
    auto_ptr<Template::Impl> elseTemplate;
    
    MistTemplateIf(Template::Impl* conditionTemplate,
        Template::Impl* ifTemplate,
        Template::Impl* elseTemplate):
            conditionTemplate(conditionTemplate),
            ifTemplate(ifTemplate), elseTemplate(elseTemplate) {}
    
    MistTemplateGroupBlock* createGroup(
        Mist::TemplateGroup::Builder& groupBuilder) const;
};

/* 
 * Join statement.
 * 
 * In general, repeating some template and insert some text between
 * repetitions.
 */
class MistTemplateJoin: public Template::Impl
{
public:
    auto_ptr<Template::Impl> templateInternal;
    string textBetween;
    
    MistTemplateJoin(Template::Impl* templateInternal,
        const string& textBetween)
        : templateInternal(templateInternal), textBetween(textBetween) {}
    
    MistTemplateGroupBlock* createGroup(
        Mist::TemplateGroup::Builder& groupBuilder) const;
};

/* 
 * With statement.
 * 
 * Inner template in some context, which should be used with join
 * routin and as base name for relative parameter names.
 */
class MistTemplateWith: public Template::Impl
{
public:
    auto_ptr<Template::Impl> templateInternal;
    MistTemplateName context;
    
    MistTemplateWith(Template::Impl* templateInternal,
        const MistTemplateName& context):
        templateInternal(templateInternal), context(context) {}
    
    MistTemplateGroupBlock* createGroup(
        Mist::TemplateGroup::Builder& groupBuilder) const;
};

#endif /* MIST_TEMPLATE_HH */
