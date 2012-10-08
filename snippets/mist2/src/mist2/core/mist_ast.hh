/*
 * Abstract syntax tree when parsing template.
 */
#ifndef MIST_AST_H_INCLUDED
#define MIST_AST_H_INCLUDED

#include <vector>
#include <string>
#include <cstdlib> /* malloc/free */
#include <cassert> /* assert() */
#include <memory> /* auto_ptr */

/* Forward declaration of template subclasses */
class MistASTTemplateRef;
class MistASTText;
class MistASTIf;
class MistASTJoin;
class MistASTWith;
class MistASTTemplateSequence;

/* Base AST Template class. */
class MistASTTemplate
{
public:
    virtual ~MistASTTemplate(void) {}
    
    class Visitor
    {
    public:
        void visit(const MistASTTemplate& astTemplate)
            {astTemplate.visit(*this);}

        virtual void visitRef
            (const MistASTTemplateRef& astRef) = 0;
        virtual void visitText
            (const MistASTText& astText) = 0;
        virtual void visitIf
            (const MistASTIf& astIf) = 0;
        virtual void visitJoin
            (const MistASTJoin& astJoin) = 0;
        virtual void visitWith
            (const MistASTWith& astWith) = 0;

        virtual ~Visitor(){}
    };

    virtual void visit(Visitor& visitor) const = 0;

};

/* Template consisted from sequence of zero or more templates */
class MistASTTemplateSequence
{
public:
    /* Iterator through statements */
    typedef std::vector<MistASTTemplate*>::const_iterator iterator;
    /* Add statement to the back of the scope. */
    void addTemplate(std::auto_ptr<MistASTTemplate> astTemplate)
        {templates.push_back(astTemplate.get()); astTemplate.release();}
    
    iterator begin(void) const {return templates.begin();}
    iterator end(void) const {return templates.end();}

    ~MistASTTemplateSequence()
    {
        for(std::vector<MistASTTemplate*>::iterator iter = templates.begin();
            iter != templates.end(); ++iter)
            delete *iter;
    }
private:
    std::vector<MistASTTemplate*> templates;
};

/* Name of the template(parameter). May be relative. */
class MistASTTemplateName
{
public:
    std::vector<std::string> components;
    bool isRelative;
    MistASTTemplateName(std::auto_ptr<std::string> first_component,
        bool isRelative = false): isRelative(isRelative)
    {
        components.push_back(*first_component);
        first_component.reset();
    }
    
    void addComponent(std::auto_ptr<std::string> component)
    {
        components.push_back(*component);
        component.reset();
    }
};

/* Reference to the template or parameter. */
class MistASTTemplateRef: public MistASTTemplate
{
public:
    std::auto_ptr<MistASTTemplateName> name;
    
    MistASTTemplateRef(std::auto_ptr<MistASTTemplateName> name): name(name) {}

    void visit(Visitor& visitor) const
        {return visitor.visitRef(*this);}
};

/* Template contained only text. */
class MistASTText: public MistASTTemplate
{
public:
    std::auto_ptr<std::string> text;
    
    MistASTText(std::auto_ptr<std::string> text): text(text) {}

    void visit(Visitor& visitor) const
        {return visitor.visitText(*this);}
};

/* 'if' statement as template. */
class MistASTIf: public MistASTTemplate
{
public:
    /* Condition - template checked for emptyness*/
    std::auto_ptr<MistASTTemplate> condition;
    /* Template corresponded to non-empty condition */
    std::auto_ptr<MistASTTemplateSequence> positivePart;
    /* Template corresponded to empty condition, may be NULL. */
    std::auto_ptr<MistASTTemplateSequence> negativePart;
    
    MistASTIf(std::auto_ptr<MistASTTemplate> condition,
        std::auto_ptr<MistASTTemplateSequence> positivePart,
        std::auto_ptr<MistASTTemplateSequence> negativePart = std::auto_ptr<MistASTTemplateSequence>(NULL) ):
        condition(condition),
        positivePart(positivePart), negativePart(negativePart) {}

    void visit(Visitor& visitor) const
        {return visitor.visitIf(*this);}
};

/* Join statement as template */
class MistASTJoin: public MistASTTemplate
{
public:
    /* Template reference which is joined */
    std::auto_ptr<MistASTTemplateRef> joinedPart;
    /* Text for insert between templates. May be NULL. */
    std::auto_ptr<std::string> textBetween;

    MistASTJoin(std::auto_ptr<MistASTTemplateRef> joinedPart,
        std::auto_ptr<std::string> textBetween):
        joinedPart(joinedPart), textBetween(textBetween) {}

    void visit(Visitor& visitor) const
        {return visitor.visitJoin(*this);}
};

/* With statement as template */
class MistASTWith: public MistASTTemplate
{
public:
    /* Name of the parameter which determine internal context of 'with' */
    std::auto_ptr<MistASTTemplateName> paramName;
    /* template which should be interpreted with given parameter */
    std::auto_ptr<MistASTTemplateSequence> templateInternal;

    MistASTWith(std::auto_ptr<MistASTTemplateName> paramName,
        std::auto_ptr<MistASTTemplateSequence> templateInternal):
        paramName(paramName), templateInternal(templateInternal) {}

    void visit(Visitor& visitor) const
        {return visitor.visitWith(*this);}
};


/************************** AST itself ********************************/
class MistAST
{
public:
    MistASTTemplateSequence* astSequence;
    
    MistAST(void) : astSequence(NULL) {}
    ~MistAST(void) {delete astSequence;}
};

#endif /* MIST_AST_H_INCLUDED */