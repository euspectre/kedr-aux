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
            istringstream ss("<$subtemplate$>");
            return new Mist::Template(ss, "main");
        }
        else if(name == "subtemplate")
        {
            istringstream ss("hello");
            return new Mist::Template(ss, "subtemplate");
        }
        else return NULL;
    }
};

int main(void)
{
    Mist::ParamSet paramSet;
    
    TwoTemplates templateCollection;
    
    Mist::TemplateGroup templateGroup(templateCollection, "main");
    
    string result = templateGroup.instantiate(paramSet);
    
    assert_instantiation(result, "hello");
    
    return 0;
}