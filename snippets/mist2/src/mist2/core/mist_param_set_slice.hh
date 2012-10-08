/*
 * Definition of parameters set slice - parameter set, in which each
 * parameter has only on value.
 * It is possible to move this slice to the next set.
 * 
 * Parameter set slice is used when insantiate template group with given
 * parameters.
 */

#ifndef MIST_PARAM_SET_SLICE_HH
#define MIST_PARAM_SET_SLICE_HH

#include <mist2/mist.hh>
class MistTemplateName;
class MistParamNameAbs;

#include <vector>

using namespace std;
using namespace Mist;

/* 
 * 'Mask' of parameters.
 * 
 * It contain names of parameters and subparameters, which are interested
 * for some purpose.
 */
class MistParamMask
{
public:
    /* Create empty parameters mask(e.g., for text chunk) */
    MistParamMask(void);
    /* Create parameters mask for one parameter name */
    MistParamMask(const MistParamNameAbs& paramName);
    /* Copy mask */
    MistParamMask(const MistParamMask& mask);
    MistParamMask& operator=(const MistParamMask& mask);
    
    ~MistParamMask();

    /* 
     * Append mask to given one.
     * 
     * NOTE: This is the only function which may change current mask.
     */
    void append(const MistParamMask& mask);

    /* 
     * Delete subtree, corresponded to given parameter, from mask.
     * 
     * NB: Parameter itself is not cut.
     * 
     * NOTE: It is an error to call this function for mask, which
     * has not given name.
     */
    void cut(const MistParamNameAbs& name);
    /* 
     * Return subtree of mask.
     * 
     * NOTE: It is an error to call this function for mask, which
     * has not given name.
     */
    const MistParamMask& getSubmask(const MistParamNameAbs& name) const;
    
    /* Iterator through submasks. */
    typedef vector<pair<string, MistParamMask> >::const_iterator iterator;

    iterator begin(void) const;
    iterator end(void) const;

    //For debug
    void print(const string& tab = "") const;

    /* Auxiliary class for modify mask */
    class Modifier
    {
    public:
        /* 
         * Parameter may be only top-level mask, because only it may
         * be non-const in outer world.
         */
        Modifier(MistParamMask& topMask);
        /* Return mask currently positioned in modifier */
        const MistParamMask& current(void) const;
        /* Move into subelement of modifier */
        void moveChild(iterator& iter);
        /* Reverse operation - move to upper layer */
        void moveParent(void);
        /* 
         * Add submask for given position.
         * 
         * NOTE: All iterators for current mask become invalid.
         */
         void addSubmask(const string& name, const MistParamMask& submask);
        /* 
         * Remove all submasks for given position.
         * 
         * NOTE: All iterators for current mask become invalid.
         */
        void removeSubmasks(void);
        
    private:
        Modifier(const Modifier& modifier);/* not implemented */
        /* Masks stack up to the current one. Not empty(!). */
        vector<MistParamMask*> maskStack;
        /* 
         * Stack of indicies of submasks.
         * 
         * NB: indiciesStack.size() === maskStack.size() - 1
         */
        vector<int> indiciesStack;
        /* Prepare current mask for modify*/
        void prepareForModify(void);
    };

private:
    class Impl;
    Impl* impl;
    
    MistParamMask(Impl* impl);
};

/* 
 * Slice of the parameters set.
 * 
 * Slice is created from parameter set using concrete parameters mask.
 * 
 * Last is needed, because slice advancing(moving to the next value set)
 * depends on which parameters it covers.
 */
class ParamSetSlice
{
public:
    /* First slice in the parameter set. */
    ParamSetSlice(const ParamSet& paramSet, const MistParamMask& mask);
    /* Deep copy of slice. */
    ParamSetSlice(const ParamSetSlice& slice);
    ~ParamSetSlice();

    /* Return string value of the slice */
    const string& getValue(void) const;
    /* 
     * Get sub-slice with given name.
     * 
     * NOTE: It is an error to call this function with name which is
     * absent in slice.
     */
    ParamSetSlice& getSubslice(const string& name);
    ParamSetSlice& getSubslice(const MistParamNameAbs& name);
    const ParamSetSlice& getSubslice(const string& name) const;
    const ParamSetSlice& getSubslice(const MistParamNameAbs& name) const;
    /* Whether parameter set of this slice is the last one. */
    bool isSetLast(void) const;
    
    /* Move to the next parameter set. */
    void nextSet(void);
    /* 
     * Change mask in slice. Useful for join/concat implementation.
     * 
     * NB: For reset submask in slice, apply this function on result of
     * getSubslice().
     */
    void resetMask(const MistParamMask& mask);
private:
    const ParamSet* value;
    struct Subslice
    {
        string name;
        const vector<ParamSet*>* values;/* Array non empty(!)*/
        int index;
        ParamSetSlice* slice;
        /* Create empty subslice */
        Subslice(const string& name, const vector<ParamSet*>* values);
    };
    vector<Subslice> subslices;
    bool isSetLastFlag;
    
    /* Set mask. May be called only when current mask is empty */
    void setMask(const MistParamMask& mask);
    /* Change value and adjust all subslices to the first values */
    void reset(const ParamSet* value);
    /* Update value of isSetLastFlag, taken into account given subslice*/
    void updateSetLast(const Subslice& subslice);
    /* Slice with empty mask */
    ParamSetSlice(const ParamSet& paramSet);
};

#endif /* MIST_PARAM_SET_SLICE_HH */