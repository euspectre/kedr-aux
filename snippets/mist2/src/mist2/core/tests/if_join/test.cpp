/* Check that 'join' for 'if' statement is correctly parsed. */

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
            istringstream ss("<$if param$><$param$><$else$>unknown<$endif: join \" \"$>");
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