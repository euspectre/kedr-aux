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
            istringstream ss("<$if param: join$><$param: join\":\"$><$else$>empty<$endif$>");
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
    
    assert_instantiation(result, "empty");
    
    paramSet.addParameter("param", "value");
    
    result = templateGroup.instantiate(paramSet);
    
    assert_instantiation(result, "value");
    
    paramSet.addParameter("param");
    paramSet.addParameter("param", "value1");
    
    result = templateGroup.instantiate(paramSet);
    assert_instantiation(result, "value::value1");
    
    return 0;
}