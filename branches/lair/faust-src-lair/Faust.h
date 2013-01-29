#ifndef __FAUST_H__
#define __FAUST_H__
#include <iostream>
#include <unistd.h>
// for use with generated Faust classes

// dummy definition of unused Faust class
class Meta 
{ 
public:
    Meta();
    void declare(const std::string& name, const std::string& nameStr);
    virtual ~Meta();
};

// dummy definition of unused Faust class
class UI 
{ 
public:
    UI();
    void openVerticalBox(const std::string& nameStr);
    void closeBox();
    virtual ~UI();
};

// definition of base Faust class
class dsp
{
public:
    dsp();
    virtual ~dsp();
    float fSamplingFreq;
    virtual int getNumInputs();
    virtual int getNumOutputs();
    virtual void init(int samplingFreq);
};

#endif // __FAUST_H__


