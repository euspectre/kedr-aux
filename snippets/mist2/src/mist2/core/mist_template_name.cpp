#include "mist_template_name.hh"

using namespace std;

bool MistParamNameAbs::operator<(const MistParamNameAbs& name) const
{
    int size = components.size();
    int name_size = name.components.size();
    
    int size_min = min(size, name_size);
    
    for(int i = 0; i < size_min; i++)
    {
        int comp = components[i].compare(name.components[i]);
        if(comp < 0) return true;
        else if(comp > 0) return false;
    }
    
    return size < name_size;
}
