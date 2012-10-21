/* Check that 'join' is correctly work for template contained 'if'. */

#include <mist2/mist.hh>

#include "mist_test_common.hh"

#include <iostream>
#include <sstream>
#include <cassert>
using namespace std;

class TwoTemplates: public Mist::TemplateCollection
{
public:
    Mist::Template* findTemplate(const string& name)
    {
        if(name == "main")
        {
            istringstream ss("<$subtemplate: join \" \"$>");
            return new Mist::Template(ss, "");
        }
        if(name == "subtemplate")
        {
            istringstream ss("<$if param$><$param$><$else$>unknown<$endif$>");
            return new Mist::Template(ss, "");
        }

        else return NULL;
    }
};

int main(void)
{
    Mist::ParamSet paramSet;
    
    paramSet.addParameter("param", "value");
    paramSet.addParameter("param", "");
    paramSet.addParameter("param", "value1");
    
    TwoTemplates templateCollection;
    
    Mist::TemplateGroup templateGroup(templateCollection, "main");
    
    string result = templateGroup.instantiate(paramSet);
    
    assert_instantiation(result, "value unknown value1");
    
    return 0;
}