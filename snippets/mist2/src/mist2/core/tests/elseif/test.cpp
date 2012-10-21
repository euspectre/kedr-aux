/* Check that 'elseif' construction is interpreted correctly. */

#include <mist2/mist.hh>

#include "mist_test_common.hh"

#include <iostream>
#include <sstream>
#include <cassert>
using namespace std;

class OneTemplate: public Mist::TemplateCollection
{
public:
    Mist::Template* findTemplate(const string& name)
    {
        if(name == "main")
        {
            istringstream ss("<$if param$><$param$><$elseif param1$><$param1$><$else$>unknown<$endif$>");
            return new Mist::Template(ss, "");
        }
        else return NULL;
    }
};

int main(void)
{
    Mist::ParamSet paramSet;
    OneTemplate templateCollection;
    Mist::TemplateGroup templateGroup(templateCollection, "main");
    
    string result = templateGroup.instantiate(paramSet);
    assert_instantiation(result, "unknown");
    
    paramSet.addParameter("param1", "value1");
    result = templateGroup.instantiate(paramSet);
    assert_instantiation(result, "value1");
    
    paramSet.addParameter("param", "value");
    result = templateGroup.instantiate(paramSet);
    assert_instantiation(result, "value");

    return 0;
}