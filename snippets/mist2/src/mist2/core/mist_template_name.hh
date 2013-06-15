/*
 * Names of templates and parameters.
 */

#ifndef MIST_TEMPLATE_NAME_HH
#define MIST_TEMPLATE_NAME_HH

#include <vector>
#include <string>

/* Name of template or parameter */
struct MistTemplateName
{
    std::vector<std::string> components;
    bool isRelative;
    
    MistTemplateName(const std::vector<std::string>& components,
        bool isRelative = false)
        : components(components), isRelative(isRelative) {}
};

/* 
 * Name of the parameter.
 * 
 * Always absolute(as opposed to MistTemplateName).
 */
struct MistParamNameAbs
{
    /* Names components. */
    std::vector<std::string> components;
    
    MistParamNameAbs(const std::vector<std::string>& components)
        : components(components) {}
    /* Construct absolute name from base and relative name. */
    MistParamNameAbs(const MistParamNameAbs& base,
        const std::vector<std::string> relComponents)
        : components(base.components)
    {
        components.insert(components.end(),
            relComponents.begin(), relComponents.end());
    }
   
    /* For using name as key of mapping */
    bool operator<(const struct MistParamNameAbs& name) const;
};


#endif /* MIST_TEMPLATE_NAME_HH */