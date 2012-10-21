/* Check that relative parameter names are correctly interpreted. */

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
            istringstream ss("<$with param$><$if .subparam$><$.subparam$><$else$>unknown<$endif$><$endwith$>");
            return new Mist::Template(ss, "");
        }
        if(name == "subparam")
        {
            /* Relative name cannot be name of template! */
            throw std::logic_error("'subparam' template has been requested, "
                "but shouldn't");
        }

        else return NULL;
    }
};

int main(void)
{
    Mist::ParamSet paramSet;
    
    paramSet.addSubset("param")->addParameter("subparam", "value");
    paramSet.addSubset("param")->addParameter("subparam");
    paramSet.addSubset("param")->addParameter("subparam", "value1");
    
    TwoTemplates templateCollection;
    
    Mist::TemplateGroup templateGroup(templateCollection, "main");
    
    string result = templateGroup.instantiate(paramSet);
    
    assert_instantiation(result, "value unknown value1");
    
    return 0;
}