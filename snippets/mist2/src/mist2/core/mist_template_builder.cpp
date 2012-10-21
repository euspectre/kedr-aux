/* Build template from AST. */

#include <mist2/mist.hh>
#include <iostream>
#include <stdexcept>

#include "mist_ast.hh"
#include "mist_parser.hh"

#include "mist_template.hh"

using namespace Mist;
using namespace std;

class Mist::Template::Builder
{
public:
    Template::Impl* build(const MistAST& ast);
private:
    Template::Impl* buildSequence(const MistASTTemplateSequence& astSequence);
    Template::Impl* buildTemplate(const MistASTTemplate& astTemplate);

    class InternalTemplateBuilder: public MistASTTemplate::Visitor
    {
    public:
        InternalTemplateBuilder(Builder& builder): builder(builder) {}
        
        Template::Impl* build(const MistASTTemplate& astTemplate);
    
        void visitRef(const MistASTTemplateRef& astRef);
        void visitText(const MistASTText& astText);
        void visitIf(const MistASTIf& astIf);
        void visitFunc(const MistASTFunction& astFunction);
        void visitWith(const MistASTWith& astWith);
        void visitSequence(const MistASTTemplateSequence& astSequence);
    private:
        Builder& builder;
        Template::Impl* result;
    };
};

Template::Impl* Mist::Template::Builder::build(const MistAST& ast)
{
    return buildSequence(*ast.astSequence);
}

Template::Impl* Mist::Template::Builder::buildSequence(
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

Template::Impl* Mist::Template::Builder::buildTemplate(
    const MistASTTemplate& astTemplate)
{
    InternalTemplateBuilder templateBuilder(*this);
    
    return templateBuilder.build(astTemplate);
}

Template::Impl* Mist::Template::Builder::InternalTemplateBuilder::build(
    const MistASTTemplate& astTemplate)
{
    result = NULL;
    visit(astTemplate);
    return result;
}

void Mist::Template::Builder::InternalTemplateBuilder::visitIf(
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


void Mist::Template::Builder::InternalTemplateBuilder::visitRef(
    const MistASTTemplateRef& astRef)
{
    result = new MistTemplateRef(MistTemplateName(
        astRef.name->components, astRef.name->isRelative));
}

void Mist::Template::Builder::InternalTemplateBuilder::visitText(
    const MistASTText& astText)
{
    result = new MistTemplateText(*astText.text);
}

void Mist::Template::Builder::InternalTemplateBuilder::visitFunc(
    const MistASTFunction& astFunction)
{
    const string& name = *astFunction.name;
    
    if(name == "join")
    {
        string textBetween;
        if(astFunction.param.get())
            textBetween = *astFunction.param;

        result = new MistTemplateJoin(
            builder.buildTemplate(*astFunction.templateInternal), textBetween);
    }
    else if(name == "rjoin")
    {
        string textBetween;
        if(astFunction.param.get())
            textBetween = *astFunction.param;

        result = new MistTemplateRJoin(
            builder.buildTemplate(*astFunction.templateInternal), textBetween);
    }
    else if(name == "indent")
    {
        string indent;
        if(astFunction.param.get())
            indent = *astFunction.param;

        result = new MistTemplateIndent(
            builder.buildTemplate(*astFunction.templateInternal), indent);
    }

    else
    {
        cerr << "Unknown function: " << *astFunction.name << endl;
        throw logic_error("Unknown function");
    }
}

void Mist::Template::Builder::InternalTemplateBuilder::visitWith(
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
    
    Builder builder;
    
    impl = builder.build(ast);
}

Template::~Template()
{
    delete impl;
}