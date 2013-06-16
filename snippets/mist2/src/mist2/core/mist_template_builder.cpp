/* Build template from AST. */

#include <mist2/mist.hh>
#include <iostream>
#include <stdexcept>

#include "mist_ast.hh"
#include "mist_parser.hh"

#include "mist_template.hh"

using namespace Mist;
using namespace std;

class MistTemplateBuilder
{
public:
    Template::Impl* build(const MistAST& ast);
private:
    Template::Impl* buildSequence(const MistASTTemplateSequence& astSequence);
    Template::Impl* buildTemplate(const MistASTTemplate& astTemplate);

    class InternalTemplateBuilder: public MistASTTemplate::Visitor
    {
    public:
        InternalTemplateBuilder(MistTemplateBuilder& builder): builder(builder) {}
        
        Template::Impl* build(const MistASTTemplate& astTemplate);
    
        void visitRef(const MistASTTemplateRef& astRef);
        void visitText(const MistASTText& astText);
        void visitIf(const MistASTIf& astIf);
        void visitFunc(const MistASTFunction& astFunction);
        void visitWith(const MistASTWith& astWith);
        void visitSequence(const MistASTTemplateSequence& astSequence);
    private:
        MistTemplateBuilder& builder;
        Template::Impl* result;
    };
};

Template::Impl* MistTemplateBuilder::build(const MistAST& ast)
{
    return buildSequence(*ast.astSequence);
}

Template::Impl* MistTemplateBuilder::buildSequence(
    const MistASTTemplateSequence& astSequence)
{
    MistTemplateSequence* templateSequence = new MistTemplateSequence();
    
    MistASTTemplateSequence::iterator iter = astSequence.begin(),
        iter_end = astSequence.end();
    for(;iter != iter_end; ++iter)
    {
        templateSequence->addTemplate(buildTemplate(**iter));
    }
    
    return templateSequence;
}

Template::Impl* MistTemplateBuilder::buildTemplate(
    const MistASTTemplate& astTemplate)
{
    InternalTemplateBuilder templateBuilder(*this);
    
    return templateBuilder.build(astTemplate);
}

Template::Impl* MistTemplateBuilder::InternalTemplateBuilder::build(
    const MistASTTemplate& astTemplate)
{
    result = NULL;
    visit(astTemplate);
    return result;
}

void MistTemplateBuilder::InternalTemplateBuilder::visitIf(
    const MistASTIf& astIf)
{
    assert(!astIf.conditionParts.empty());

    MistASTIf::ConditionPart* lastPart = astIf.conditionParts.back();
    
    Template::Impl* elseTemplate;
    
    if(astIf.elsePart.get())
    {
        elseTemplate = builder.buildSequence(*astIf.elsePart);
    }
    else
    {
        elseTemplate = new MistTemplateEmpty();
    }
    
    result = new MistTemplateIf(
        builder.buildTemplate(*lastPart->condition),
        builder.buildSequence(*lastPart->positivePart),
        elseTemplate);
    
    for(int i = (int)astIf.conditionParts.size() - 2; i>= 0; i--)
    {
        MistASTIf::ConditionPart* part = astIf.conditionParts[i];
        
        result = new MistTemplateIf(
            builder.buildTemplate(*part->condition),
            builder.buildSequence(*part->positivePart),
            result);
    }
}


void MistTemplateBuilder::InternalTemplateBuilder::visitRef(
    const MistASTTemplateRef& astRef)
{
    result = new MistTemplateRef(MistTemplateName(
        astRef.name->components, astRef.name->isRelative));
}

void MistTemplateBuilder::InternalTemplateBuilder::visitText(
    const MistASTText& astText)
{
    result = new MistTemplateText(*astText.text);
}

/* 
 * Generic function implementation. Implemented as static class.
 * (one instance for every function).
 */
class MistFunction
{
public:
    MistFunction(const string& name);
    
    virtual Template::Impl* createTemplate(Template::Impl* templateInternal,
        const string& text) const = 0;
};

static map<string, MistFunction*> functions;

MistFunction::MistFunction(const string& name)
{
    pair<map<string, MistFunction*>::iterator, bool> iter =
        functions.insert(make_pair(name, this));
    assert(iter.second);
}

// Concrete functions
class MistFunctionJoin: public MistFunction
{
public:
    MistFunctionJoin(): MistFunction("join") {}
    
    Template::Impl* createTemplate(Template::Impl* templateInternal,
        const string& text) const
    {
        return new MistTemplateJoin(templateInternal, text);
    }
} functionJoin;

class MistFunctionRJoin: public MistFunction
{
public:
    MistFunctionRJoin(): MistFunction("rjoin") {}
    
    Template::Impl* createTemplate(Template::Impl* templateInternal,
        const string& text) const
    {
        return new MistTemplateRJoin(templateInternal, text);
    }
} functionRJoin;

class MistFunctionIndent: public MistFunction
{
public:
    MistFunctionIndent(): MistFunction("indent") {}
    
    Template::Impl* createTemplate(Template::Impl* templateInternal,
        const string& text) const
    {
        return new MistTemplateIndent(templateInternal, text);
    }
} functionIndent;

class MistFunctionIndex: public MistFunction
{
public:
    MistFunctionIndex(): MistFunction("i") {}
    
    Template::Impl* createTemplate(Template::Impl* templateInternal,
        const string&) const
    {
        return new MistTemplateIndex1(templateInternal);
    }
} functionIndex;

class MistFunctionIndex0: public MistFunction
{
public:
    MistFunctionIndex0(): MistFunction("i0") {}
    
    Template::Impl* createTemplate(Template::Impl* templateInternal,
        const string&) const
    {
        return new MistTemplateIndex0(templateInternal);
    }
} functionIndex0;


void MistTemplateBuilder::InternalTemplateBuilder::visitFunc(
    const MistASTFunction& astFunction)
{
    const string& name = *astFunction.name;
    map<string, MistFunction*>::const_iterator iter = functions.find(name);

    if(iter != functions.end())
    {
        string text;
        if(astFunction.param.get())
            text = *astFunction.param;

        Template::Impl* templateInternal = 
            builder.buildTemplate(*astFunction.templateInternal);

        result = iter->second->createTemplate(templateInternal, text);
    }
    else
    {
        cerr << "Unknown function: " << name << endl;
        throw logic_error("Unknown function");
    }
}

void MistTemplateBuilder::InternalTemplateBuilder::visitWith(
    const MistASTWith& astWith)
{
    result = new MistTemplateWith(
        builder.buildSequence(*astWith.templateInternal),
        MistTemplateName(astWith.paramName->components,
            astWith.paramName->isRelative));
}

Template::Template(istream& s, const string& filename)
{
    MistAST ast;
    
    MistParser parser;
    
    parser.parse(s, ast, filename);
    
    MistTemplateBuilder builder;
    
    impl = builder.build(ast);
}

Template::~Template()
{
    delete impl;
}