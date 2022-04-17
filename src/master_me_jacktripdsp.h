/* ------------------------------------------------------------
author: "Klaus Scheuermann"
copyright: "(C) 2021 Klaus Scheuermann"
name: "master_me_gui"
version: "2.0"
Code generated with Faust 2.40.1 (https://faust.grame.fr)
Compilation options: -a faust2header.cpp -lang cpp -i -inpl -cn master_me_jacktripdsp -es 1 -mcd 16 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __master_me_jacktripdsp_H__
#define  __master_me_jacktripdsp_H__

// NOTE: ANY INCLUDE-GUARD HERE MUST BE DERIVED FROM THE CLASS NAME
//
// faust2header.cpp - FAUST Architecture File
// This is a simple variation of matlabplot.cpp in the Faust distribution
// aimed at creating a simple C++ header file (.h) containing a Faust DSP.
// See the Makefile for how to use it.

/************************** BEGIN dsp.h ********************************
 FAUST Architecture File
 Copyright (C) 2003-2022 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 
 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 ************************************************************************/

#ifndef __dsp__
#define __dsp__

#include <string>
#include <vector>

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

struct UI;
struct Meta;

/**
 * DSP memory manager.
 */

struct dsp_memory_manager {
    
    virtual ~dsp_memory_manager() {}
    
    /**
     * Inform the Memory Manager with the number of expected memory zones.
     * @param count - the number of expected memory zones
     */
    virtual void begin(size_t count) {}
    
    /**
     * Give the Memory Manager information on a given memory zone.
     * @param size - the size in bytes of the memory zone
     * @param reads - the number of Read access to the zone used to compute one frame
     * @param writes - the number of Write access to the zone used to compute one frame
     */
    virtual void info(size_t size, size_t reads, size_t writes) {}
    
    /**
     * Inform the Memory Manager that all memory zones have been described,
     * to possibly start a 'compute the best allocation strategy' step.
     */
    virtual void end() {}
    
    /**
     * Allocate a memory zone.
     * @param size - the memory zone size in bytes
     */
    virtual void* allocate(size_t size) = 0;
    
    /**
     * Destroy a memory zone.
     * @param ptr - the memory zone pointer to be deallocated
     */
    virtual void destroy(void* ptr) = 0;
    
};

/**
* Signal processor definition.
*/

class dsp {

    public:

        dsp() {}
        virtual ~dsp() {}

        /* Return instance number of audio inputs */
        virtual int getNumInputs() = 0;
    
        /* Return instance number of audio outputs */
        virtual int getNumOutputs() = 0;
    
        /**
         * Trigger the ui_interface parameter with instance specific calls
         * to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the UI.
         *
         * @param ui_interface - the user interface builder
         */
        virtual void buildUserInterface(UI* ui_interface) = 0;
    
        /* Return the sample rate currently used by the instance */
        virtual int getSampleRate() = 0;
    
        /**
         * Global init, calls the following methods:
         * - static class 'classInit': static tables initialization
         * - 'instanceInit': constants and instance state initialization
         *
         * @param sample_rate - the sampling rate in Hz
         */
        virtual void init(int sample_rate) = 0;

        /**
         * Init instance state
         *
         * @param sample_rate - the sampling rate in Hz
         */
        virtual void instanceInit(int sample_rate) = 0;
    
        /**
         * Init instance constant state
         *
         * @param sample_rate - the sampling rate in Hz
         */
        virtual void instanceConstants(int sample_rate) = 0;
    
        /* Init default control parameters values */
        virtual void instanceResetUserInterface() = 0;
    
        /* Init instance state (like delay lines...) but keep the control parameter values */
        virtual void instanceClear() = 0;
 
        /**
         * Return a clone of the instance.
         *
         * @return a copy of the instance on success, otherwise a null pointer.
         */
        virtual dsp* clone() = 0;
    
        /**
         * Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.
         *
         * @param m - the Meta* meta user
         */
        virtual void metadata(Meta* m) = 0;
    
        /**
         * DSP instance computation, to be called with successive in/out audio buffers.
         *
         * @param count - the number of frames to compute
         * @param inputs - the input audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad)
         * @param outputs - the output audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad)
         *
         */
        virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) = 0;
    
        /**
         * DSP instance computation: alternative method to be used by subclasses.
         *
         * @param date_usec - the timestamp in microsec given by audio driver.
         * @param count - the number of frames to compute
         * @param inputs - the input audio buffers as an array of non-interleaved FAUSTFLOAT samples (either float, double or quad)
         * @param outputs - the output audio buffers as an array of non-interleaved FAUSTFLOAT samples (either float, double or quad)
         *
         */
        virtual void compute(double /*date_usec*/, int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) { compute(count, inputs, outputs); }
       
};

/**
 * Generic DSP decorator.
 */

class decorator_dsp : public dsp {

    protected:

        dsp* fDSP;

    public:

        decorator_dsp(dsp* dsp = nullptr):fDSP(dsp) {}
        virtual ~decorator_dsp() { delete fDSP; }

        virtual int getNumInputs() { return fDSP->getNumInputs(); }
        virtual int getNumOutputs() { return fDSP->getNumOutputs(); }
        virtual void buildUserInterface(UI* ui_interface) { fDSP->buildUserInterface(ui_interface); }
        virtual int getSampleRate() { return fDSP->getSampleRate(); }
        virtual void init(int sample_rate) { fDSP->init(sample_rate); }
        virtual void instanceInit(int sample_rate) { fDSP->instanceInit(sample_rate); }
        virtual void instanceConstants(int sample_rate) { fDSP->instanceConstants(sample_rate); }
        virtual void instanceResetUserInterface() { fDSP->instanceResetUserInterface(); }
        virtual void instanceClear() { fDSP->instanceClear(); }
        virtual decorator_dsp* clone() { return new decorator_dsp(fDSP->clone()); }
        virtual void metadata(Meta* m) { fDSP->metadata(m); }
        // Beware: subclasses usually have to overload the two 'compute' methods
        virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) { fDSP->compute(count, inputs, outputs); }
        virtual void compute(double date_usec, int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) { fDSP->compute(date_usec, count, inputs, outputs); }
    
};

/**
 * DSP factory class, used with LLVM and Interpreter backends
 * to create DSP instances from a compiled DSP program.
 */

class dsp_factory {
    
    protected:
    
        // So that to force sub-classes to use deleteDSPFactory(dsp_factory* factory);
        virtual ~dsp_factory() {}
    
    public:
    
        virtual std::string getName() = 0;
        virtual std::string getSHAKey() = 0;
        virtual std::string getDSPCode() = 0;
        virtual std::string getCompileOptions() = 0;
        virtual std::vector<std::string> getLibraryList() = 0;
        virtual std::vector<std::string> getIncludePathnames() = 0;
    
        virtual dsp* createDSPInstance() = 0;
    
        virtual void setMemoryManager(dsp_memory_manager* manager) = 0;
        virtual dsp_memory_manager* getMemoryManager() = 0;
    
};

// Denormal handling

#if defined (__SSE__)
#include <xmmintrin.h>
#endif

class ScopedNoDenormals
{
    private:
    
        intptr_t fpsr;
        
        void setFpStatusRegister(intptr_t fpsr_aux) noexcept
        {
        #if defined (__arm64__) || defined (__aarch64__)
           asm volatile("msr fpcr, %0" : : "ri" (fpsr_aux));
        #elif defined (__SSE__)
            _mm_setcsr(static_cast<uint32_t>(fpsr_aux));
        #endif
        }
        
        void getFpStatusRegister() noexcept
        {
        #if defined (__arm64__) || defined (__aarch64__)
            asm volatile("mrs %0, fpcr" : "=r" (fpsr));
        #elif defined ( __SSE__)
            fpsr = static_cast<intptr_t>(_mm_getcsr());
        #endif
        }
    
    public:
    
        ScopedNoDenormals() noexcept
        {
        #if defined (__arm64__) || defined (__aarch64__)
            intptr_t mask = (1 << 24 /* FZ */);
        #else
            #if defined(__SSE__)
            #if defined(__SSE2__)
                intptr_t mask = 0x8040;
            #else
                intptr_t mask = 0x8000;
            #endif
            #else
                intptr_t mask = 0x0000;
            #endif
        #endif
            getFpStatusRegister();
            setFpStatusRegister(fpsr | mask);
        }
        
        ~ScopedNoDenormals() noexcept
        {
            setFpStatusRegister(fpsr);
        }

};

#define AVOIDDENORMALS ScopedNoDenormals();

#endif

/************************** END dsp.h **************************/
/************************** BEGIN APIUI.h *****************************
FAUST Architecture File
Copyright (C) 2003-2022 GRAME, Centre National de Creation Musicale
---------------------------------------------------------------------
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

EXCEPTION : As a special exception, you may create a larger work
that contains this FAUST architecture section and distribute
that work under terms of your choice, so long as this FAUST
architecture section is not modified.
************************************************************************/

#ifndef API_UI_H
#define API_UI_H

#include <sstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <map>

/************************** BEGIN meta.h *******************************
 FAUST Architecture File
 Copyright (C) 2003-2022 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 
 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 ************************************************************************/

#ifndef __meta__
#define __meta__

/**
 The base class of Meta handler to be used in dsp::metadata(Meta* m) method to retrieve (key, value) metadata.
 */
struct Meta
{
    virtual ~Meta() {}
    virtual void declare(const char* key, const char* value) = 0;
};

#endif
/**************************  END  meta.h **************************/
/************************** BEGIN UI.h *****************************
 FAUST Architecture File
 Copyright (C) 2003-2022 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 
 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 ********************************************************************/

#ifndef __UI_H__
#define __UI_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

/*******************************************************************************
 * UI : Faust DSP User Interface
 * User Interface as expected by the buildUserInterface() method of a DSP.
 * This abstract class contains only the method that the Faust compiler can
 * generate to describe a DSP user interface.
 ******************************************************************************/

struct Soundfile;

template <typename REAL>
struct UIReal
{
    UIReal() {}
    virtual ~UIReal() {}
    
    // -- widget's layouts
    
    virtual void openTabBox(const char* label) = 0;
    virtual void openHorizontalBox(const char* label) = 0;
    virtual void openVerticalBox(const char* label) = 0;
    virtual void closeBox() = 0;
    
    // -- active widgets
    
    virtual void addButton(const char* label, REAL* zone) = 0;
    virtual void addCheckButton(const char* label, REAL* zone) = 0;
    virtual void addVerticalSlider(const char* label, REAL* zone, REAL init, REAL min, REAL max, REAL step) = 0;
    virtual void addHorizontalSlider(const char* label, REAL* zone, REAL init, REAL min, REAL max, REAL step) = 0;
    virtual void addNumEntry(const char* label, REAL* zone, REAL init, REAL min, REAL max, REAL step) = 0;
    
    // -- passive widgets
    
    virtual void addHorizontalBargraph(const char* label, REAL* zone, REAL min, REAL max) = 0;
    virtual void addVerticalBargraph(const char* label, REAL* zone, REAL min, REAL max) = 0;
    
    // -- soundfiles
    
    virtual void addSoundfile(const char* label, const char* filename, Soundfile** sf_zone) = 0;
    
    // -- metadata declarations
    
    virtual void declare(REAL* zone, const char* key, const char* val) {}
    
    // To be used by LLVM client
    virtual int sizeOfFAUSTFLOAT() { return sizeof(FAUSTFLOAT); }
};

struct UI : public UIReal<FAUSTFLOAT>
{
    UI() {}
    virtual ~UI() {}
};

#endif
/**************************  END  UI.h **************************/
/************************** BEGIN PathBuilder.h **************************
 FAUST Architecture File
 Copyright (C) 2003-2022 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 
 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 ************************************************************************/

#ifndef FAUST_PATHBUILDER_H
#define FAUST_PATHBUILDER_H

#include <vector>
#include <string>
#include <algorithm>

/*******************************************************************************
 * PathBuilder : Faust User Interface
 * Helper class to build complete hierarchical path for UI items.
 ******************************************************************************/

class PathBuilder
{

    protected:
    
        std::vector<std::string> fControlsLevel;
       
    public:
    
        PathBuilder() {}
        virtual ~PathBuilder() {}
    
        std::string replaceCharList(std::string str, const std::vector<char>& ch1, char ch2)
        {
            std::vector<char>::const_iterator beg = ch1.begin();
            std::vector<char>::const_iterator end = ch1.end();
            for (size_t i = 0; i < str.length(); ++i) {
                if (std::find(beg, end, str[i]) != end) {
                    str[i] = ch2;
                }
            }
            return str;
        }
    
        std::string buildPath(const std::string& label) 
        {
            std::string res = "/";
            for (size_t i = 0; i < fControlsLevel.size(); i++) {
                res += fControlsLevel[i];
                res += "/";
            }
            res += label;
            std::vector<char> rep = {' ', '#', '*', ',', '/', '?', '[', ']', '{', '}', '(', ')'};
            replaceCharList(res, rep, '_');
            return res;
        }
    
        void pushLabel(const std::string& label) { fControlsLevel.push_back(label); }
        void popLabel() { fControlsLevel.pop_back(); }
    
};

#endif  // FAUST_PATHBUILDER_H
/**************************  END  PathBuilder.h **************************/
/************************** BEGIN ValueConverter.h ********************
 FAUST Architecture File
 Copyright (C) 2003-2022 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 
 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 ********************************************************************/

#ifndef __ValueConverter__
#define __ValueConverter__

/***************************************************************************************
 ValueConverter.h
 (GRAME, Copyright 2015-2019)
 
 Set of conversion objects used to map user interface values (for example a gui slider
 delivering values between 0 and 1) to faust values (for example a vslider between
 20 and 20000) using a log scale.
 
 -- Utilities
 
 Range(lo,hi) : clip a value x between lo and hi
 Interpolator(lo,hi,v1,v2) : Maps a value x between lo and hi to a value y between v1 and v2
 Interpolator3pt(lo,mi,hi,v1,vm,v2) : Map values between lo mid hi to values between v1 vm v2
 
 -- Value Converters
 
 ValueConverter::ui2faust(x)
 ValueConverter::faust2ui(x)
 
 -- ValueConverters used for sliders depending of the scale
 
 LinearValueConverter(umin, umax, fmin, fmax)
 LinearValueConverter2(lo, mi, hi, v1, vm, v2) using 2 segments
 LogValueConverter(umin, umax, fmin, fmax)
 ExpValueConverter(umin, umax, fmin, fmax)
 
 -- ValueConverters used for accelerometers based on 3 points
 
 AccUpConverter(amin, amid, amax, fmin, fmid, fmax)        -- curve 0
 AccDownConverter(amin, amid, amax, fmin, fmid, fmax)      -- curve 1
 AccUpDownConverter(amin, amid, amax, fmin, fmid, fmax)    -- curve 2
 AccDownUpConverter(amin, amid, amax, fmin, fmid, fmax)    -- curve 3
 
 -- lists of ZoneControl are used to implement accelerometers metadata for each axes
 
 ZoneControl(zone, valueConverter) : a zone with an accelerometer data converter
 
 -- ZoneReader are used to implement screencolor metadata
 
 ZoneReader(zone, valueConverter) : a zone with a data converter

****************************************************************************************/

#include <float.h>
#include <algorithm>    // std::max
#include <cmath>
#include <vector>
#include <assert.h>

//--------------------------------------------------------------------------------------
// Interpolator(lo,hi,v1,v2)
// Maps a value x between lo and hi to a value y between v1 and v2
// y = v1 + (x-lo)/(hi-lo)*(v2-v1)
// y = v1 + (x-lo) * coef           with coef = (v2-v1)/(hi-lo)
// y = v1 + x*coef - lo*coef
// y = v1 - lo*coef + x*coef
// y = offset + x*coef              with offset = v1 - lo*coef
//--------------------------------------------------------------------------------------
class Interpolator
{
    private:

        //--------------------------------------------------------------------------------------
        // Range(lo,hi) clip a value between lo and hi
        //--------------------------------------------------------------------------------------
        struct Range
        {
            double fLo;
            double fHi;

            Range(double x, double y) : fLo(std::min<double>(x,y)), fHi(std::max<double>(x,y)) {}
            double operator()(double x) { return (x<fLo) ? fLo : (x>fHi) ? fHi : x; }
        };


        Range fRange;
        double fCoef;
        double fOffset;

    public:

        Interpolator(double lo, double hi, double v1, double v2) : fRange(lo,hi)
        {
            if (hi != lo) {
                // regular case
                fCoef = (v2-v1)/(hi-lo);
                fOffset = v1 - lo*fCoef;
            } else {
                // degenerate case, avoids division by zero
                fCoef = 0;
                fOffset = (v1+v2)/2;
            }
        }
        double operator()(double v)
        {
            double x = fRange(v);
            return  fOffset + x*fCoef;
        }

        void getLowHigh(double& amin, double& amax)
        {
            amin = fRange.fLo;
            amax = fRange.fHi;
        }
};

//--------------------------------------------------------------------------------------
// Interpolator3pt(lo,mi,hi,v1,vm,v2)
// Map values between lo mid hi to values between v1 vm v2
//--------------------------------------------------------------------------------------
class Interpolator3pt
{

    private:

        Interpolator fSegment1;
        Interpolator fSegment2;
        double fMid;

    public:

        Interpolator3pt(double lo, double mi, double hi, double v1, double vm, double v2) :
            fSegment1(lo, mi, v1, vm),
            fSegment2(mi, hi, vm, v2),
            fMid(mi) {}
        double operator()(double x) { return  (x < fMid) ? fSegment1(x) : fSegment2(x); }

        void getMappingValues(double& amin, double& amid, double& amax)
        {
            fSegment1.getLowHigh(amin, amid);
            fSegment2.getLowHigh(amid, amax);
        }
};

//--------------------------------------------------------------------------------------
// Abstract ValueConverter class. Converts values between UI and Faust representations
//--------------------------------------------------------------------------------------
class ValueConverter // Identity by default
{

    public:

        virtual ~ValueConverter() {}
        virtual double ui2faust(double x) { return x; };
        virtual double faust2ui(double x) { return x; };
};

//--------------------------------------------------------------------------------------
// A converter than can be updated
//--------------------------------------------------------------------------------------

class UpdatableValueConverter : public ValueConverter {
    
    protected:
        
        bool fActive;
        
    public:
        
        UpdatableValueConverter():fActive(true)
        {}
        virtual ~UpdatableValueConverter()
        {}
        
        virtual void setMappingValues(double amin, double amid, double amax, double min, double init, double max) = 0;
        virtual void getMappingValues(double& amin, double& amid, double& amax) = 0;
        
        void setActive(bool on_off) { fActive = on_off; }
        bool getActive() { return fActive; }
    
};

//--------------------------------------------------------------------------------------
// Linear conversion between ui and Faust values
//--------------------------------------------------------------------------------------
class LinearValueConverter : public ValueConverter
{
    
    private:
        
        Interpolator fUI2F;
        Interpolator fF2UI;
        
    public:
        
        LinearValueConverter(double umin, double umax, double fmin, double fmax) :
            fUI2F(umin,umax,fmin,fmax), fF2UI(fmin,fmax,umin,umax)
        {}
        
        LinearValueConverter() : fUI2F(0.,0.,0.,0.), fF2UI(0.,0.,0.,0.)
        {}
        virtual double ui2faust(double x) { return fUI2F(x); }
        virtual double faust2ui(double x) { return fF2UI(x); }
    
};

//--------------------------------------------------------------------------------------
// Two segments linear conversion between ui and Faust values
//--------------------------------------------------------------------------------------
class LinearValueConverter2 : public UpdatableValueConverter
{
    
    private:
    
        Interpolator3pt fUI2F;
        Interpolator3pt fF2UI;
        
    public:
    
        LinearValueConverter2(double amin, double amid, double amax, double min, double init, double max) :
            fUI2F(amin, amid, amax, min, init, max), fF2UI(min, init, max, amin, amid, amax)
        {}
        
        LinearValueConverter2() : fUI2F(0.,0.,0.,0.,0.,0.), fF2UI(0.,0.,0.,0.,0.,0.)
        {}
    
        virtual double ui2faust(double x) { return fUI2F(x); }
        virtual double faust2ui(double x) { return fF2UI(x); }
    
        virtual void setMappingValues(double amin, double amid, double amax, double min, double init, double max)
        {
            fUI2F = Interpolator3pt(amin, amid, amax, min, init, max);
            fF2UI = Interpolator3pt(min, init, max, amin, amid, amax);
        }

        virtual void getMappingValues(double& amin, double& amid, double& amax)
        {
            fUI2F.getMappingValues(amin, amid, amax);
        }
    
};

//--------------------------------------------------------------------------------------
// Logarithmic conversion between ui and Faust values
//--------------------------------------------------------------------------------------
class LogValueConverter : public LinearValueConverter
{

    public:

        LogValueConverter(double umin, double umax, double fmin, double fmax) :
            LinearValueConverter(umin, umax, std::log(std::max<double>(DBL_MIN, fmin)), std::log(std::max<double>(DBL_MIN, fmax)))
        {}

        virtual double ui2faust(double x) { return std::exp(LinearValueConverter::ui2faust(x)); }
        virtual double faust2ui(double x) { return LinearValueConverter::faust2ui(std::log(std::max<double>(x, DBL_MIN))); }

};

//--------------------------------------------------------------------------------------
// Exponential conversion between ui and Faust values
//--------------------------------------------------------------------------------------
class ExpValueConverter : public LinearValueConverter
{

    public:

        ExpValueConverter(double umin, double umax, double fmin, double fmax) :
            LinearValueConverter(umin, umax, std::min<double>(DBL_MAX, std::exp(fmin)), std::min<double>(DBL_MAX, std::exp(fmax)))
        {}

        virtual double ui2faust(double x) { return std::log(LinearValueConverter::ui2faust(x)); }
        virtual double faust2ui(double x) { return LinearValueConverter::faust2ui(std::min<double>(DBL_MAX, std::exp(x))); }

};

//--------------------------------------------------------------------------------------
// Convert accelerometer or gyroscope values to Faust values
// Using an Up curve (curve 0)
//--------------------------------------------------------------------------------------
class AccUpConverter : public UpdatableValueConverter
{

    private:

        Interpolator3pt fA2F;
        Interpolator3pt fF2A;

    public:

        AccUpConverter(double amin, double amid, double amax, double fmin, double fmid, double fmax) :
            fA2F(amin,amid,amax,fmin,fmid,fmax),
            fF2A(fmin,fmid,fmax,amin,amid,amax)
        {}

        virtual double ui2faust(double x) { return fA2F(x); }
        virtual double faust2ui(double x) { return fF2A(x); }

        virtual void setMappingValues(double amin, double amid, double amax, double fmin, double fmid, double fmax)
        {
            //__android_log_print(ANDROID_LOG_ERROR, "Faust", "AccUpConverter update %f %f %f %f %f %f", amin,amid,amax,fmin,fmid,fmax);
            fA2F = Interpolator3pt(amin, amid, amax, fmin, fmid, fmax);
            fF2A = Interpolator3pt(fmin, fmid, fmax, amin, amid, amax);
        }

        virtual void getMappingValues(double& amin, double& amid, double& amax)
        {
            fA2F.getMappingValues(amin, amid, amax);
        }

};

//--------------------------------------------------------------------------------------
// Convert accelerometer or gyroscope values to Faust values
// Using a Down curve (curve 1)
//--------------------------------------------------------------------------------------
class AccDownConverter : public UpdatableValueConverter
{

    private:

        Interpolator3pt	fA2F;
        Interpolator3pt	fF2A;

    public:

        AccDownConverter(double amin, double amid, double amax, double fmin, double fmid, double fmax) :
            fA2F(amin,amid,amax,fmax,fmid,fmin),
            fF2A(fmin,fmid,fmax,amax,amid,amin)
        {}

        virtual double ui2faust(double x) { return fA2F(x); }
        virtual double faust2ui(double x) { return fF2A(x); }

        virtual void setMappingValues(double amin, double amid, double amax, double fmin, double fmid, double fmax)
        {
             //__android_log_print(ANDROID_LOG_ERROR, "Faust", "AccDownConverter update %f %f %f %f %f %f", amin,amid,amax,fmin,fmid,fmax);
            fA2F = Interpolator3pt(amin, amid, amax, fmax, fmid, fmin);
            fF2A = Interpolator3pt(fmin, fmid, fmax, amax, amid, amin);
        }

        virtual void getMappingValues(double& amin, double& amid, double& amax)
        {
            fA2F.getMappingValues(amin, amid, amax);
        }
};

//--------------------------------------------------------------------------------------
// Convert accelerometer or gyroscope values to Faust values
// Using an Up-Down curve (curve 2)
//--------------------------------------------------------------------------------------
class AccUpDownConverter : public UpdatableValueConverter
{

    private:

        Interpolator3pt	fA2F;
        Interpolator fF2A;

    public:

        AccUpDownConverter(double amin, double amid, double amax, double fmin, double fmid, double fmax) :
            fA2F(amin,amid,amax,fmin,fmax,fmin),
            fF2A(fmin,fmax,amin,amax)				// Special, pseudo inverse of a non monotonic function
        {}

        virtual double ui2faust(double x) { return fA2F(x); }
        virtual double faust2ui(double x) { return fF2A(x); }

        virtual void setMappingValues(double amin, double amid, double amax, double fmin, double fmid, double fmax)
        {
            //__android_log_print(ANDROID_LOG_ERROR, "Faust", "AccUpDownConverter update %f %f %f %f %f %f", amin,amid,amax,fmin,fmid,fmax);
            fA2F = Interpolator3pt(amin, amid, amax, fmin, fmax, fmin);
            fF2A = Interpolator(fmin, fmax, amin, amax);
        }

        virtual void getMappingValues(double& amin, double& amid, double& amax)
        {
            fA2F.getMappingValues(amin, amid, amax);
        }
};

//--------------------------------------------------------------------------------------
// Convert accelerometer or gyroscope values to Faust values
// Using a Down-Up curve (curve 3)
//--------------------------------------------------------------------------------------
class AccDownUpConverter : public UpdatableValueConverter
{

    private:

        Interpolator3pt	fA2F;
        Interpolator fF2A;

    public:

        AccDownUpConverter(double amin, double amid, double amax, double fmin, double fmid, double fmax) :
            fA2F(amin,amid,amax,fmax,fmin,fmax),
            fF2A(fmin,fmax,amin,amax)				// Special, pseudo inverse of a non monotonic function
        {}

        virtual double ui2faust(double x) { return fA2F(x); }
        virtual double faust2ui(double x) { return fF2A(x); }

        virtual void setMappingValues(double amin, double amid, double amax, double fmin, double fmid, double fmax)
        {
            //__android_log_print(ANDROID_LOG_ERROR, "Faust", "AccDownUpConverter update %f %f %f %f %f %f", amin,amid,amax,fmin,fmid,fmax);
            fA2F = Interpolator3pt(amin, amid, amax, fmax, fmin, fmax);
            fF2A = Interpolator(fmin, fmax, amin, amax);
        }

        virtual void getMappingValues(double& amin, double& amid, double& amax)
        {
            fA2F.getMappingValues(amin, amid, amax);
        }
};

//--------------------------------------------------------------------------------------
// Base class for ZoneControl
//--------------------------------------------------------------------------------------
class ZoneControl
{

    protected:

        FAUSTFLOAT*	fZone;

    public:

        ZoneControl(FAUSTFLOAT* zone) : fZone(zone) {}
        virtual ~ZoneControl() {}

        virtual void update(double v) const {}

        virtual void setMappingValues(int curve, double amin, double amid, double amax, double min, double init, double max) {}
        virtual void getMappingValues(double& amin, double& amid, double& amax) {}

        FAUSTFLOAT* getZone() { return fZone; }

        virtual void setActive(bool on_off) {}
        virtual bool getActive() { return false; }

        virtual int getCurve() { return -1; }

};

//--------------------------------------------------------------------------------------
//  Useful to implement accelerometers metadata as a list of ZoneControl for each axes
//--------------------------------------------------------------------------------------
class ConverterZoneControl : public ZoneControl
{

    protected:

        ValueConverter* fValueConverter;

    public:

        ConverterZoneControl(FAUSTFLOAT* zone, ValueConverter* converter) : ZoneControl(zone), fValueConverter(converter) {}
        virtual ~ConverterZoneControl() { delete fValueConverter; } // Assuming fValueConverter is not kept elsewhere...

        virtual void update(double v) const { *fZone = FAUSTFLOAT(fValueConverter->ui2faust(v)); }

        ValueConverter* getConverter() { return fValueConverter; }

};

//--------------------------------------------------------------------------------------
// Association of a zone and a four value converter, each one for each possible curve.
// Useful to implement accelerometers metadata as a list of ZoneControl for each axes
//--------------------------------------------------------------------------------------
class CurveZoneControl : public ZoneControl
{

    private:

        std::vector<UpdatableValueConverter*> fValueConverters;
        int fCurve;

    public:

        CurveZoneControl(FAUSTFLOAT* zone, int curve, double amin, double amid, double amax, double min, double init, double max) : ZoneControl(zone), fCurve(0)
        {
            assert(curve >= 0 && curve <= 3);
            fValueConverters.push_back(new AccUpConverter(amin, amid, amax, min, init, max));
            fValueConverters.push_back(new AccDownConverter(amin, amid, amax, min, init, max));
            fValueConverters.push_back(new AccUpDownConverter(amin, amid, amax, min, init, max));
            fValueConverters.push_back(new AccDownUpConverter(amin, amid, amax, min, init, max));
            fCurve = curve;
        }
        virtual ~CurveZoneControl()
        {
            for (const auto& it : fValueConverters) { delete it; }
        }
        void update(double v) const { if (fValueConverters[fCurve]->getActive()) *fZone = FAUSTFLOAT(fValueConverters[fCurve]->ui2faust(v)); }

        void setMappingValues(int curve, double amin, double amid, double amax, double min, double init, double max)
        {
            fValueConverters[curve]->setMappingValues(amin, amid, amax, min, init, max);
            fCurve = curve;
        }

        void getMappingValues(double& amin, double& amid, double& amax)
        {
            fValueConverters[fCurve]->getMappingValues(amin, amid, amax);
        }

        void setActive(bool on_off)
        {
            for (const auto& it : fValueConverters) { it->setActive(on_off); }
        }

        int getCurve() { return fCurve; }
};

class ZoneReader
{

    private:

        FAUSTFLOAT* fZone;
        Interpolator fInterpolator;

    public:

        ZoneReader(FAUSTFLOAT* zone, double lo, double hi) : fZone(zone), fInterpolator(lo, hi, 0, 255) {}

        virtual ~ZoneReader() {}

        int getValue()
        {
            return (fZone != nullptr) ? int(fInterpolator(*fZone)) : 127;
        }

};

#endif
/**************************  END  ValueConverter.h **************************/

typedef unsigned int uint;

class APIUI : public PathBuilder, public Meta, public UI
{
    public:
        enum ItemType { kButton = 0, kCheckButton, kVSlider, kHSlider, kNumEntry, kHBargraph, kVBargraph };
        enum Type { kAcc = 0, kGyr = 1, kNoType };

    protected:

        enum Mapping { kLin = 0, kLog = 1, kExp = 2 };

        struct Item {
            std::string fPath;
            std::string fLabel;
            ValueConverter* fConversion;
            FAUSTFLOAT* fZone;
            FAUSTFLOAT fInit;
            FAUSTFLOAT fMin;
            FAUSTFLOAT fMax;
            FAUSTFLOAT fStep;
            ItemType fItemType;
        };
        std::vector<Item> fItems;

        std::vector<std::map<std::string, std::string> > fMetaData;
        std::vector<ZoneControl*> fAcc[3];
        std::vector<ZoneControl*> fGyr[3];

        // Screen color control
        // "...[screencolor:red]..." etc.
        bool fHasScreenControl;      // true if control screen color metadata
        ZoneReader* fRedReader;
        ZoneReader* fGreenReader;
        ZoneReader* fBlueReader;

        // Current values controlled by metadata
        std::string fCurrentUnit;
        int fCurrentScale;
        std::string fCurrentAcc;
        std::string fCurrentGyr;
        std::string fCurrentColor;
        std::string fCurrentTooltip;
        std::map<std::string, std::string> fCurrentMetadata;

        // Add a generic parameter
        virtual void addParameter(const char* label,
                                  FAUSTFLOAT* zone,
                                  FAUSTFLOAT init,
                                  FAUSTFLOAT min,
                                  FAUSTFLOAT max,
                                  FAUSTFLOAT step,
                                  ItemType type)
        {
            std::string path = buildPath(label);

            // handle scale metadata
            ValueConverter* converter = nullptr;
            switch (fCurrentScale) {
                case kLin:
                    converter = new LinearValueConverter(0, 1, min, max);
                    break;
                case kLog:
                    converter = new LogValueConverter(0, 1, min, max);
                    break;
                case kExp:
                    converter = new ExpValueConverter(0, 1, min, max);
                    break;
            }
            fCurrentScale = kLin;

            fItems.push_back({path, label, converter, zone, init, min, max, step, type });
       
            if (fCurrentAcc.size() > 0 && fCurrentGyr.size() > 0) {
                fprintf(stderr, "warning : 'acc' and 'gyr' metadata used for the same %s parameter !!\n", label);
            }

            // handle acc metadata "...[acc : <axe> <curve> <amin> <amid> <amax>]..."
            if (fCurrentAcc.size() > 0) {
                std::istringstream iss(fCurrentAcc);
                int axe, curve;
                double amin, amid, amax;
                iss >> axe >> curve >> amin >> amid >> amax;

                if ((0 <= axe) && (axe < 3) &&
                    (0 <= curve) && (curve < 4) &&
                    (amin < amax) && (amin <= amid) && (amid <= amax))
                {
                    fAcc[axe].push_back(new CurveZoneControl(zone, curve, amin, amid, amax, min, init, max));
                } else {
                    fprintf(stderr, "incorrect acc metadata : %s \n", fCurrentAcc.c_str());
                }
                fCurrentAcc = "";
            }

            // handle gyr metadata "...[gyr : <axe> <curve> <amin> <amid> <amax>]..."
            if (fCurrentGyr.size() > 0) {
                std::istringstream iss(fCurrentGyr);
                int axe, curve;
                double amin, amid, amax;
                iss >> axe >> curve >> amin >> amid >> amax;

                if ((0 <= axe) && (axe < 3) &&
                    (0 <= curve) && (curve < 4) &&
                    (amin < amax) && (amin <= amid) && (amid <= amax))
                {
                    fGyr[axe].push_back(new CurveZoneControl(zone, curve, amin, amid, amax, min, init, max));
                } else {
                    fprintf(stderr, "incorrect gyr metadata : %s \n", fCurrentGyr.c_str());
                }
                fCurrentGyr = "";
            }

            // handle screencolor metadata "...[screencolor:red|green|blue|white]..."
            if (fCurrentColor.size() > 0) {
                if ((fCurrentColor == "red") && (fRedReader == nullptr)) {
                    fRedReader = new ZoneReader(zone, min, max);
                    fHasScreenControl = true;
                } else if ((fCurrentColor == "green") && (fGreenReader == nullptr)) {
                    fGreenReader = new ZoneReader(zone, min, max);
                    fHasScreenControl = true;
                } else if ((fCurrentColor == "blue") && (fBlueReader == nullptr)) {
                    fBlueReader = new ZoneReader(zone, min, max);
                    fHasScreenControl = true;
                } else if ((fCurrentColor == "white") && (fRedReader == nullptr) && (fGreenReader == nullptr) && (fBlueReader == nullptr)) {
                    fRedReader = new ZoneReader(zone, min, max);
                    fGreenReader = new ZoneReader(zone, min, max);
                    fBlueReader = new ZoneReader(zone, min, max);
                    fHasScreenControl = true;
                } else {
                    fprintf(stderr, "incorrect screencolor metadata : %s \n", fCurrentColor.c_str());
                }
            }
            fCurrentColor = "";

            fMetaData.push_back(fCurrentMetadata);
            fCurrentMetadata.clear();
        }

        int getZoneIndex(std::vector<ZoneControl*>* table, int p, int val)
        {
            FAUSTFLOAT* zone = fItems[uint(p)].fZone;
            for (size_t i = 0; i < table[val].size(); i++) {
                if (zone == table[val][i]->getZone()) return int(i);
            }
            return -1;
        }

        void setConverter(std::vector<ZoneControl*>* table, int p, int val, int curve, double amin, double amid, double amax)
        {
            int id1 = getZoneIndex(table, p, 0);
            int id2 = getZoneIndex(table, p, 1);
            int id3 = getZoneIndex(table, p, 2);

            // Deactivates everywhere..
            if (id1 != -1) table[0][uint(id1)]->setActive(false);
            if (id2 != -1) table[1][uint(id2)]->setActive(false);
            if (id3 != -1) table[2][uint(id3)]->setActive(false);

            if (val == -1) { // Means: no more mapping...
                // So stay all deactivated...
            } else {
                int id4 = getZoneIndex(table, p, val);
                if (id4 != -1) {
                    // Reactivate the one we edit...
                  table[val][uint(id4)]->setMappingValues(curve, amin, amid, amax, fItems[uint(p)].fMin, fItems[uint(p)].fInit, fItems[uint(p)].fMax);
                  table[val][uint(id4)]->setActive(true);
                } else {
                    // Allocate a new CurveZoneControl which is 'active' by default
                    FAUSTFLOAT* zone = fItems[uint(p)].fZone;
                    table[val].push_back(new CurveZoneControl(zone, curve, amin, amid, amax, fItems[uint(p)].fMin, fItems[uint(p)].fInit, fItems[uint(p)].fMax));
                }
            }
        }

        void getConverter(std::vector<ZoneControl*>* table, int p, int& val, int& curve, double& amin, double& amid, double& amax)
        {
            int id1 = getZoneIndex(table, p, 0);
            int id2 = getZoneIndex(table, p, 1);
            int id3 = getZoneIndex(table, p, 2);

            if (id1 != -1) {
                val = 0;
                curve = table[val][uint(id1)]->getCurve();
                table[val][uint(id1)]->getMappingValues(amin, amid, amax);
            } else if (id2 != -1) {
                val = 1;
                curve = table[val][uint(id2)]->getCurve();
                table[val][uint(id2)]->getMappingValues(amin, amid, amax);
            } else if (id3 != -1) {
                val = 2;
                curve = table[val][uint(id3)]->getCurve();
                table[val][uint(id3)]->getMappingValues(amin, amid, amax);
            } else {
                val = -1; // No mapping
                curve = 0;
                amin = -100.;
                amid = 0.;
                amax = 100.;
            }
        }

    public:

        APIUI() : fHasScreenControl(false), fRedReader(nullptr), fGreenReader(nullptr), fBlueReader(nullptr), fCurrentScale(kLin)
        {}

        virtual ~APIUI()
        {
            for (const auto& it : fItems) delete it.fConversion;
            for (int i = 0; i < 3; i++) {
                for (const auto& it : fAcc[i]) delete it;
                for (const auto& it : fGyr[i]) delete it;
            }
            delete fRedReader;
            delete fGreenReader;
            delete fBlueReader;
        }

        // -- widget's layouts

        virtual void openTabBox(const char* label) { pushLabel(label); }
        virtual void openHorizontalBox(const char* label) { pushLabel(label); }
        virtual void openVerticalBox(const char* label) { pushLabel(label); }
        virtual void closeBox() { popLabel(); }

        // -- active widgets

        virtual void addButton(const char* label, FAUSTFLOAT* zone)
        {
            addParameter(label, zone, 0, 0, 1, 1, kButton);
        }

        virtual void addCheckButton(const char* label, FAUSTFLOAT* zone)
        {
            addParameter(label, zone, 0, 0, 1, 1, kCheckButton);
        }

        virtual void addVerticalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step)
        {
            addParameter(label, zone, init, min, max, step, kVSlider);
        }

        virtual void addHorizontalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step)
        {
            addParameter(label, zone, init, min, max, step, kHSlider);
        }

        virtual void addNumEntry(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step)
        {
            addParameter(label, zone, init, min, max, step, kNumEntry);
        }

        // -- passive widgets

        virtual void addHorizontalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max)
        {
            addParameter(label, zone, min, min, max, (max-min)/1000.0f, kHBargraph);
        }

        virtual void addVerticalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max)
        {
            addParameter(label, zone, min, min, max, (max-min)/1000.0f, kVBargraph);
        }

        // -- soundfiles

        virtual void addSoundfile(const char* label, const char* filename, Soundfile** sf_zone) {}

        // -- metadata declarations

        virtual void declare(FAUSTFLOAT* zone, const char* key, const char* val)
        {
            // Keep metadata
            fCurrentMetadata[key] = val;

            if (strcmp(key, "scale") == 0) {
                if (strcmp(val, "log") == 0) {
                    fCurrentScale = kLog;
                } else if (strcmp(val, "exp") == 0) {
                    fCurrentScale = kExp;
                } else {
                    fCurrentScale = kLin;
                }
            } else if (strcmp(key, "unit") == 0) {
                fCurrentUnit = val;
            } else if (strcmp(key, "acc") == 0) {
                fCurrentAcc = val;
            } else if (strcmp(key, "gyr") == 0) {
                fCurrentGyr = val;
            } else if (strcmp(key, "screencolor") == 0) {
                fCurrentColor = val; // val = "red", "green", "blue" or "white"
            } else if (strcmp(key, "tooltip") == 0) {
                fCurrentTooltip = val;
            }
        }

        virtual void declare(const char* key, const char* val)
        {}

        //-------------------------------------------------------------------------------
        // Simple API part
        //-------------------------------------------------------------------------------
        int getParamsCount() { return int(fItems.size()); }

        int getParamIndex(const char* path)
        {
            auto it1 = find_if(fItems.begin(), fItems.end(), [=](const Item& it) { return it.fPath == std::string(path); });
            if (it1 != fItems.end()) {
                return int(it1 - fItems.begin());
            }

            auto it2 = find_if(fItems.begin(), fItems.end(), [=](const Item& it) { return it.fLabel == std::string(path); });
            if (it2 != fItems.end()) {
                return int(it2 - fItems.begin());
            }

            return -1;
        }
        const char* getParamAddress(int p) { return fItems[uint(p)].fPath.c_str(); }
        const char* getParamLabel(int p) { return fItems[uint(p)].fLabel.c_str(); }
        std::map<const char*, const char*> getMetadata(int p)
        {
            std::map<const char*, const char*> res;
            std::map<std::string, std::string> metadata = fMetaData[uint(p)];
            for (const auto& it : metadata) {
                res[it.first.c_str()] = it.second.c_str();
            }
            return res;
        }

        const char* getMetadata(int p, const char* key)
        {
            return (fMetaData[uint(p)].find(key) != fMetaData[uint(p)].end()) ? fMetaData[uint(p)][key].c_str() : "";
        }
        FAUSTFLOAT getParamMin(int p) { return fItems[uint(p)].fMin; }
        FAUSTFLOAT getParamMax(int p) { return fItems[uint(p)].fMax; }
        FAUSTFLOAT getParamStep(int p) { return fItems[uint(p)].fStep; }
        FAUSTFLOAT getParamInit(int p) { return fItems[uint(p)].fInit; }

        FAUSTFLOAT* getParamZone(int p) { return fItems[uint(p)].fZone; }

        FAUSTFLOAT getParamValue(int p) { return *fItems[uint(p)].fZone; }
        FAUSTFLOAT getParamValue(const char* path)
        {
            int index = getParamIndex(path);
            return (index >= 0) ? getParamValue(index) : FAUSTFLOAT(0);
        }

        void setParamValue(int p, FAUSTFLOAT v) { *fItems[uint(p)].fZone = v; }
        void setParamValue(const char* path, FAUSTFLOAT v)
        {
            int index = getParamIndex(path);
            if (index >= 0) {
                setParamValue(index, v);
            } else {
                fprintf(stderr, "setParamValue : '%s' not found\n", (path == nullptr ? "NULL" : path));
            }
        }

        double getParamRatio(int p) { return fItems[uint(p)].fConversion->faust2ui(*fItems[uint(p)].fZone); }
        void setParamRatio(int p, double r) { *fItems[uint(p)].fZone = FAUSTFLOAT(fItems[uint(p)].fConversion->ui2faust(r)); }

        double value2ratio(int p, double r)    { return fItems[uint(p)].fConversion->faust2ui(r); }
        double ratio2value(int p, double r)    { return fItems[uint(p)].fConversion->ui2faust(r); }

        /**
         * Return the control type (kAcc, kGyr, or -1) for a given parameter.
         *
         * @param p - the UI parameter index
         *
         * @return the type
         */
        Type getParamType(int p)
        {
            if (p >= 0) {
                if (getZoneIndex(fAcc, p, 0) != -1
                    || getZoneIndex(fAcc, p, 1) != -1
                    || getZoneIndex(fAcc, p, 2) != -1) {
                    return kAcc;
                } else if (getZoneIndex(fGyr, p, 0) != -1
                           || getZoneIndex(fGyr, p, 1) != -1
                           || getZoneIndex(fGyr, p, 2) != -1) {
                    return kGyr;
                }
            }
            return kNoType;
        }

        /**
         * Return the Item type (kButton = 0, kCheckButton, kVSlider, kHSlider, kNumEntry, kHBargraph, kVBargraph) for a given parameter.
         *
         * @param p - the UI parameter index
         *
         * @return the Item type
         */
        ItemType getParamItemType(int p)
        {
            return fItems[uint(p)].fItemType;
        }

        /**
         * Set a new value coming from an accelerometer, propagate it to all relevant FAUSTFLOAT* zones.
         *
         * @param acc - 0 for X accelerometer, 1 for Y accelerometer, 2 for Z accelerometer
         * @param value - the new value
         *
         */
        void propagateAcc(int acc, double value)
        {
            for (size_t i = 0; i < fAcc[acc].size(); i++) {
                fAcc[acc][i]->update(value);
            }
        }

        /**
         * Used to edit accelerometer curves and mapping. Set curve and related mapping for a given UI parameter.
         *
         * @param p - the UI parameter index
         * @param acc - 0 for X accelerometer, 1 for Y accelerometer, 2 for Z accelerometer (-1 means "no mapping")
         * @param curve - between 0 and 3
         * @param amin - mapping 'min' point
         * @param amid - mapping 'middle' point
         * @param amax - mapping 'max' point
         *
         */
        void setAccConverter(int p, int acc, int curve, double amin, double amid, double amax)
        {
            setConverter(fAcc, p, acc, curve, amin, amid, amax);
        }

        /**
         * Used to edit gyroscope curves and mapping. Set curve and related mapping for a given UI parameter.
         *
         * @param p - the UI parameter index
         * @param acc - 0 for X gyroscope, 1 for Y gyroscope, 2 for Z gyroscope (-1 means "no mapping")
         * @param curve - between 0 and 3
         * @param amin - mapping 'min' point
         * @param amid - mapping 'middle' point
         * @param amax - mapping 'max' point
         *
         */
        void setGyrConverter(int p, int gyr, int curve, double amin, double amid, double amax)
        {
            setConverter(fGyr, p, gyr, curve, amin, amid, amax);
        }

        /**
         * Used to edit accelerometer curves and mapping. Get curve and related mapping for a given UI parameter.
         *
         * @param p - the UI parameter index
         * @param acc - the acc value to be retrieved (-1 means "no mapping")
         * @param curve - the curve value to be retrieved
         * @param amin - the amin value to be retrieved
         * @param amid - the amid value to be retrieved
         * @param amax - the amax value to be retrieved
         *
         */
        void getAccConverter(int p, int& acc, int& curve, double& amin, double& amid, double& amax)
        {
            getConverter(fAcc, p, acc, curve, amin, amid, amax);
        }

        /**
         * Used to edit gyroscope curves and mapping. Get curve and related mapping for a given UI parameter.
         *
         * @param p - the UI parameter index
         * @param gyr - the gyr value to be retrieved (-1 means "no mapping")
         * @param curve - the curve value to be retrieved
         * @param amin - the amin value to be retrieved
         * @param amid - the amid value to be retrieved
         * @param amax - the amax value to be retrieved
         *
         */
        void getGyrConverter(int p, int& gyr, int& curve, double& amin, double& amid, double& amax)
        {
            getConverter(fGyr, p, gyr, curve, amin, amid, amax);
        }

        /**
         * Set a new value coming from an gyroscope, propagate it to all relevant FAUSTFLOAT* zones.
         *
         * @param gyr - 0 for X gyroscope, 1 for Y gyroscope, 2 for Z gyroscope
         * @param value - the new value
         *
         */
        void propagateGyr(int gyr, double value)
        {
            for (size_t i = 0; i < fGyr[gyr].size(); i++) {
                fGyr[gyr][i]->update(value);
            }
        }

        /**
         * Get the number of FAUSTFLOAT* zones controlled with the accelerometer.
         *
         * @param acc - 0 for X accelerometer, 1 for Y accelerometer, 2 for Z accelerometer
         * @return the number of zones
         *
         */
        int getAccCount(int acc)
        {
            return (acc >= 0 && acc < 3) ? int(fAcc[acc].size()) : 0;
        }

        /**
         * Get the number of FAUSTFLOAT* zones controlled with the gyroscope.
         *
         * @param gyr - 0 for X gyroscope, 1 for Y gyroscope, 2 for Z gyroscope
         * @param the number of zones
         *
         */
        int getGyrCount(int gyr)
        {
            return (gyr >= 0 && gyr < 3) ? int(fGyr[gyr].size()) : 0;
        }

        // getScreenColor() : -1 means no screen color control (no screencolor metadata found)
        // otherwise return 0x00RRGGBB a ready to use color
        int getScreenColor()
        {
            if (fHasScreenControl) {
                int r = (fRedReader) ? fRedReader->getValue() : 0;
                int g = (fGreenReader) ? fGreenReader->getValue() : 0;
                int b = (fBlueReader) ? fBlueReader->getValue() : 0;
                return (r<<16) | (g<<8) | b;
            } else {
                return -1;
            }
        }

};

#endif
/**************************  END  APIUI.h **************************/

// NOTE: "faust -scn name" changes the last line above to
// #include <faust/name/name.h>

//----------------------------------------------------------------------------
//  FAUST Generated Code
//----------------------------------------------------------------------------


#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS master_me_jacktripdsp
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

#if defined(_WIN32)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

static float master_me_jacktripdsp_faustpower2_f(float value) {
	return value * value;
}

class master_me_jacktripdsp : public dsp {
	
 private:
	
	float fVec0[2];
	float fVec1[2];
	int fSampleRate;
	float fConst1;
	float fConst2;
	float fConst3;
	int iRec5[2];
	FAUSTFLOAT fVslider0;
	float fConst4;
	float fConst5;
	float fConst6;
	int iRec11[2];
	FAUSTFLOAT fVslider1;
	float fConst7;
	float fConst8;
	float fRec19[2];
	float fRec18[2];
	FAUSTFLOAT fVslider2;
	float fRec21[2];
	float fRec20[2];
	float fRec23[2];
	float fRec22[2];
	float fRec25[2];
	float fRec24[2];
	float fRec27[2];
	float fRec26[2];
	float fRec29[2];
	float fRec28[2];
	FAUSTFLOAT fVbargraph0;
	float fConst11;
	float fConst13;
	float fConst15;
	float fRec33[2];
	float fRec34[2];
	float fRec38[2];
	FAUSTFLOAT fVslider3;
	int iVec2[2];
	int iConst16;
	int iRec39[2];
	float fConst17;
	float fRec36[2];
	float fRec35[2];
	FAUSTFLOAT fVbargraph1;
	float fVec3[2];
	FAUSTFLOAT fVslider4;
	FAUSTFLOAT fVslider5;
	float fConst18;
	FAUSTFLOAT fVslider6;
	float fConst19;
	float fConst20;
	float fRec44[2];
	FAUSTFLOAT fVslider7;
	int iRec45[2];
	float fRec42[2];
	FAUSTFLOAT fVbargraph2;
	FAUSTFLOAT fVslider8;
	float fConst21;
	float fConst25;
	float fConst27;
	float fConst29;
	float fConst30;
	float fConst32;
	float fConst33;
	float fRec49[2];
	float fConst34;
	float fConst36;
	float fConst37;
	float fRec48[3];
	float fConst38;
	float fRec51[2];
	float fRec50[3];
	float fConst39;
	float fConst40;
	float fConst41;
	float fConst42;
	float fRec47[3];
	float fConst43;
	float fRec46[2];
	FAUSTFLOAT fVbargraph3;
	float fRec40[2];
	FAUSTFLOAT fVbargraph4;
	float fVec4[2];
	float fConst44;
	float fConst45;
	float fConst46;
	float fRec32[2];
	float fConst47;
	float fConst48;
	float fRec31[3];
	float fConst49;
	float fConst53;
	float fConst54;
	float fConst55;
	float fConst57;
	float fConst58;
	float fRec30[3];
	float fRec12[2];
	float fConst59;
	FAUSTFLOAT fVbargraph5;
	float fConst60;
	float fConst61;
	float fRec55[2];
	float fRec54[3];
	float fVec5[2];
	float fConst62;
	float fConst63;
	float fRec53[2];
	float fConst64;
	float fRec52[3];
	float fConst65;
	float fRec13[2];
	FAUSTFLOAT fVbargraph6;
	float fConst66;
	float fRec57[2];
	float fRec56[3];
	float fRec14[2];
	FAUSTFLOAT fVbargraph7;
	float fVec6[2];
	float fRec60[2];
	float fRec59[3];
	float fRec58[3];
	float fRec15[2];
	FAUSTFLOAT fVbargraph8;
	float fRec64[2];
	float fRec63[3];
	float fVec7[2];
	float fRec62[2];
	float fRec61[3];
	float fRec16[2];
	FAUSTFLOAT fVbargraph9;
	float fRec66[2];
	float fRec65[3];
	float fRec17[2];
	int IOTA0;
	float fVec8[2048];
	float fVec9[2048];
	float fRec10[2];
	int iRec8[2];
	float fRec7[2];
	float fConst67;
	float fConst68;
	float fRec6[2];
	FAUSTFLOAT fVbargraph10;
	int iConst69;
	float fVec10[2048];
	float fVec11[2048];
	float fRec4[2];
	int iRec2[2];
	float fRec1[2];
	float fConst70;
	float fConst71;
	float fRec0[2];
	FAUSTFLOAT fVbargraph11;
	
 public:
	
	void metadata(Meta* m) { 
		m->declare("analyzers.lib/name", "Faust Analyzer Library");
		m->declare("analyzers.lib/version", "0.1");
		m->declare("author", "Klaus Scheuermann");
		m->declare("basics.lib/bypass1:author", "Julius Smith");
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/peakhold:author", "Jonatan Liljedahl, revised by Romain Michon");
		m->declare("basics.lib/peakholder:author", "Jonatan Liljedahl");
		m->declare("basics.lib/sweep:author", "Jonatan Liljedahl");
		m->declare("basics.lib/version", "0.5");
		m->declare("compile_options", "-a faust2header.cpp -lang cpp -i -inpl -cn master_me_jacktripdsp -es 1 -mcd 16 -single -ftz 0");
		m->declare("compressors.lib/name", "Faust Compressor Effect Library");
		m->declare("compressors.lib/version", "0.2");
		m->declare("copyright", "(C) 2021 Klaus Scheuermann");
		m->declare("filename", "master_me_jacktripdsp.dsp");
		m->declare("filters.lib/dcblocker:author", "Julius O. Smith III");
		m->declare("filters.lib/dcblocker:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/dcblocker:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/filterbank:author", "Julius O. Smith III");
		m->declare("filters.lib/filterbank:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/filterbank:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/fir:author", "Julius O. Smith III");
		m->declare("filters.lib/fir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/fir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/highpass:author", "Julius O. Smith III");
		m->declare("filters.lib/highpass:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/highpass_plus_lowpass:author", "Julius O. Smith III");
		m->declare("filters.lib/highpass_plus_lowpass:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/highpass_plus_lowpass:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/highshelf:author", "Julius O. Smith III");
		m->declare("filters.lib/highshelf:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/highshelf:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/iir:author", "Julius O. Smith III");
		m->declare("filters.lib/iir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/iir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass0_highpass1:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/pole:author", "Julius O. Smith III");
		m->declare("filters.lib/pole:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/pole:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf1:author", "Julius O. Smith III");
		m->declare("filters.lib/tf1:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf1:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf1s:author", "Julius O. Smith III");
		m->declare("filters.lib/tf1s:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf1s:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf2:author", "Julius O. Smith III");
		m->declare("filters.lib/tf2:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf2:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf2s:author", "Julius O. Smith III");
		m->declare("filters.lib/tf2s:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf2s:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/version", "0.3");
		m->declare("filters.lib/zero:author", "Julius O. Smith III");
		m->declare("filters.lib/zero:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/zero:license", "MIT-style STK-4.3 license");
		m->declare("interpolators.lib/name", "Faust Interpolator Library");
		m->declare("interpolators.lib/version", "0.3");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.5");
		m->declare("name", "master_me_gui");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "0.2");
		m->declare("routes.lib/name", "Faust Signal Routing Library");
		m->declare("routes.lib/version", "0.2");
		m->declare("signals.lib/name", "Faust Signal Routing Library");
		m->declare("signals.lib/version", "0.1");
		m->declare("version", "2.0");
	}

	virtual int getNumInputs() {
		return 2;
	}
	virtual int getNumOutputs() {
		return 2;
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		float fConst0 = std::min<float>(192000.0f, std::max<float>(1.0f, float(fSampleRate)));
		fConst1 = std::exp(0.0f - 628.318542f / fConst0);
		fConst2 = 1.0f - fConst1;
		fConst3 = 0.100000001f * fConst0;
		fConst4 = std::exp(0.0f - 100.0f / fConst0);
		fConst5 = 1.0f - fConst4;
		fConst6 = 0.0500000007f * fConst0;
		fConst7 = std::exp(0.0f - 50.0f / fConst0);
		fConst8 = std::exp(0.0f - 2.0f / fConst0);
		float fConst9 = std::tan(7853.98145f / fConst0);
		float fConst10 = 1.0f / fConst9;
		fConst11 = 1.0f / ((fConst10 + 1.0f) / fConst9 + 1.0f);
		float fConst12 = master_me_jacktripdsp_faustpower2_f(fConst9);
		fConst13 = 1.0f / fConst12;
		float fConst14 = fConst10 + 1.0f;
		fConst15 = 1.0f / (fConst9 * fConst14);
		iConst16 = int(fConst0);
		fConst17 = std::exp(0.0f - 0.5f / fConst0);
		fConst18 = 1.0f / fConst0;
		fConst19 = std::exp(0.0f - 10.0f / fConst0);
		fConst20 = 1.0f - fConst19;
		fConst21 = std::exp(0.0f - 2.09439516f / fConst0);
		float fConst22 = std::tan(125.663704f / fConst0);
		float fConst23 = 1.0f / fConst22;
		float fConst24 = (fConst23 + 1.41421354f) / fConst22 + 1.0f;
		fConst25 = (1.0f - fConst21) / master_me_jacktripdsp_faustpower2_f(fConst24);
		float fConst26 = master_me_jacktripdsp_faustpower2_f(fConst22);
		fConst27 = 1.0f / fConst26;
		float fConst28 = std::tan(4492.47754f / fConst0);
		fConst29 = 1.0f / fConst28;
		fConst30 = 1.0f / ((fConst29 + 1.0f) / fConst28 + 1.0f);
		float fConst31 = fConst29 + 1.0f;
		fConst32 = 1.0f / fConst31;
		fConst33 = 1.0f - fConst29;
		fConst34 = (fConst29 + -1.0f) / fConst28 + 1.0f;
		float fConst35 = master_me_jacktripdsp_faustpower2_f(fConst28);
		fConst36 = 1.0f / fConst35;
		fConst37 = 2.0f * (1.0f - fConst36);
		fConst38 = 0.0f - 1.0f / (fConst28 * fConst31);
		fConst39 = 0.0f - 2.0f / fConst35;
		fConst40 = 1.0f / fConst24;
		fConst41 = (fConst23 + -1.41421354f) / fConst22 + 1.0f;
		fConst42 = 2.0f * (1.0f - fConst27);
		fConst43 = 0.0f - 2.0f / fConst26;
		fConst44 = 0.0f - fConst15;
		fConst45 = 1.0f - fConst10;
		fConst46 = fConst45 / fConst14;
		fConst47 = (fConst10 + -1.0f) / fConst9 + 1.0f;
		fConst48 = 2.0f * (1.0f - fConst13);
		fConst49 = 0.0f - 2.0f / fConst12;
		float fConst50 = std::tan(785.398193f / fConst0);
		float fConst51 = 1.0f / fConst50;
		float fConst52 = fConst51 + 1.0f;
		fConst53 = 1.0f / (fConst52 / fConst50 + 1.0f);
		fConst54 = 1.0f - fConst51;
		fConst55 = 1.0f - fConst54 / fConst50;
		float fConst56 = master_me_jacktripdsp_faustpower2_f(fConst50);
		fConst57 = 1.0f / fConst56;
		fConst58 = 2.0f * (1.0f - fConst57);
		fConst59 = 1.0f / ((fConst51 + 1.0f) / fConst50 + 1.0f);
		fConst60 = 1.0f / (fConst50 * fConst52);
		fConst61 = 1.0f / fConst14;
		fConst62 = 0.0f - fConst60;
		fConst63 = fConst54 / fConst52;
		fConst64 = (fConst51 + -1.0f) / fConst50 + 1.0f;
		fConst65 = 0.0f - 2.0f / fConst56;
		fConst66 = 1.0f / fConst52;
		fConst67 = std::exp(0.0f - 5.0f / fConst0);
		fConst68 = 1.0f - fConst67;
		iConst69 = int(0.00999999978f * fConst0);
		fConst70 = std::exp(0.0f - 6.28318548f / fConst0);
		fConst71 = 1.0f - fConst70;
	}
	
	virtual void instanceResetUserInterface() {
		fVslider0 = FAUSTFLOAT(15.0f);
		fVslider1 = FAUSTFLOAT(0.0f);
		fVslider2 = FAUSTFLOAT(-10.0f);
		fVslider3 = FAUSTFLOAT(-70.0f);
		fVslider4 = FAUSTFLOAT(6.0f);
		fVslider5 = FAUSTFLOAT(6.0f);
		fVslider6 = FAUSTFLOAT(0.02f);
		fVslider7 = FAUSTFLOAT(-45.0f);
		fVslider8 = FAUSTFLOAT(-16.0f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			fVec0[l0] = 0.0f;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fVec1[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			iRec5[l2] = 0;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			iRec11[l3] = 0;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fRec19[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fRec18[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fRec21[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			fRec20[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 2; l8 = l8 + 1) {
			fRec23[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
			fRec22[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			fRec25[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			fRec24[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			fRec27[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 2; l13 = l13 + 1) {
			fRec26[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 2; l14 = l14 + 1) {
			fRec29[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			fRec28[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 2; l16 = l16 + 1) {
			fRec33[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 2; l17 = l17 + 1) {
			fRec34[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
			fRec38[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 2; l19 = l19 + 1) {
			iVec2[l19] = 0;
		}
		for (int l20 = 0; l20 < 2; l20 = l20 + 1) {
			iRec39[l20] = 0;
		}
		for (int l21 = 0; l21 < 2; l21 = l21 + 1) {
			fRec36[l21] = 0.0f;
		}
		for (int l22 = 0; l22 < 2; l22 = l22 + 1) {
			fRec35[l22] = 0.0f;
		}
		for (int l23 = 0; l23 < 2; l23 = l23 + 1) {
			fVec3[l23] = 0.0f;
		}
		for (int l24 = 0; l24 < 2; l24 = l24 + 1) {
			fRec44[l24] = 0.0f;
		}
		for (int l25 = 0; l25 < 2; l25 = l25 + 1) {
			iRec45[l25] = 0;
		}
		for (int l26 = 0; l26 < 2; l26 = l26 + 1) {
			fRec42[l26] = 0.0f;
		}
		for (int l27 = 0; l27 < 2; l27 = l27 + 1) {
			fRec49[l27] = 0.0f;
		}
		for (int l28 = 0; l28 < 3; l28 = l28 + 1) {
			fRec48[l28] = 0.0f;
		}
		for (int l29 = 0; l29 < 2; l29 = l29 + 1) {
			fRec51[l29] = 0.0f;
		}
		for (int l30 = 0; l30 < 3; l30 = l30 + 1) {
			fRec50[l30] = 0.0f;
		}
		for (int l31 = 0; l31 < 3; l31 = l31 + 1) {
			fRec47[l31] = 0.0f;
		}
		for (int l32 = 0; l32 < 2; l32 = l32 + 1) {
			fRec46[l32] = 0.0f;
		}
		for (int l33 = 0; l33 < 2; l33 = l33 + 1) {
			fRec40[l33] = 0.0f;
		}
		for (int l34 = 0; l34 < 2; l34 = l34 + 1) {
			fVec4[l34] = 0.0f;
		}
		for (int l35 = 0; l35 < 2; l35 = l35 + 1) {
			fRec32[l35] = 0.0f;
		}
		for (int l36 = 0; l36 < 3; l36 = l36 + 1) {
			fRec31[l36] = 0.0f;
		}
		for (int l37 = 0; l37 < 3; l37 = l37 + 1) {
			fRec30[l37] = 0.0f;
		}
		for (int l38 = 0; l38 < 2; l38 = l38 + 1) {
			fRec12[l38] = 0.0f;
		}
		for (int l39 = 0; l39 < 2; l39 = l39 + 1) {
			fRec55[l39] = 0.0f;
		}
		for (int l40 = 0; l40 < 3; l40 = l40 + 1) {
			fRec54[l40] = 0.0f;
		}
		for (int l41 = 0; l41 < 2; l41 = l41 + 1) {
			fVec5[l41] = 0.0f;
		}
		for (int l42 = 0; l42 < 2; l42 = l42 + 1) {
			fRec53[l42] = 0.0f;
		}
		for (int l43 = 0; l43 < 3; l43 = l43 + 1) {
			fRec52[l43] = 0.0f;
		}
		for (int l44 = 0; l44 < 2; l44 = l44 + 1) {
			fRec13[l44] = 0.0f;
		}
		for (int l45 = 0; l45 < 2; l45 = l45 + 1) {
			fRec57[l45] = 0.0f;
		}
		for (int l46 = 0; l46 < 3; l46 = l46 + 1) {
			fRec56[l46] = 0.0f;
		}
		for (int l47 = 0; l47 < 2; l47 = l47 + 1) {
			fRec14[l47] = 0.0f;
		}
		for (int l48 = 0; l48 < 2; l48 = l48 + 1) {
			fVec6[l48] = 0.0f;
		}
		for (int l49 = 0; l49 < 2; l49 = l49 + 1) {
			fRec60[l49] = 0.0f;
		}
		for (int l50 = 0; l50 < 3; l50 = l50 + 1) {
			fRec59[l50] = 0.0f;
		}
		for (int l51 = 0; l51 < 3; l51 = l51 + 1) {
			fRec58[l51] = 0.0f;
		}
		for (int l52 = 0; l52 < 2; l52 = l52 + 1) {
			fRec15[l52] = 0.0f;
		}
		for (int l53 = 0; l53 < 2; l53 = l53 + 1) {
			fRec64[l53] = 0.0f;
		}
		for (int l54 = 0; l54 < 3; l54 = l54 + 1) {
			fRec63[l54] = 0.0f;
		}
		for (int l55 = 0; l55 < 2; l55 = l55 + 1) {
			fVec7[l55] = 0.0f;
		}
		for (int l56 = 0; l56 < 2; l56 = l56 + 1) {
			fRec62[l56] = 0.0f;
		}
		for (int l57 = 0; l57 < 3; l57 = l57 + 1) {
			fRec61[l57] = 0.0f;
		}
		for (int l58 = 0; l58 < 2; l58 = l58 + 1) {
			fRec16[l58] = 0.0f;
		}
		for (int l59 = 0; l59 < 2; l59 = l59 + 1) {
			fRec66[l59] = 0.0f;
		}
		for (int l60 = 0; l60 < 3; l60 = l60 + 1) {
			fRec65[l60] = 0.0f;
		}
		for (int l61 = 0; l61 < 2; l61 = l61 + 1) {
			fRec17[l61] = 0.0f;
		}
		IOTA0 = 0;
		for (int l62 = 0; l62 < 2048; l62 = l62 + 1) {
			fVec8[l62] = 0.0f;
		}
		for (int l63 = 0; l63 < 2048; l63 = l63 + 1) {
			fVec9[l63] = 0.0f;
		}
		for (int l64 = 0; l64 < 2; l64 = l64 + 1) {
			fRec10[l64] = 0.0f;
		}
		for (int l65 = 0; l65 < 2; l65 = l65 + 1) {
			iRec8[l65] = 0;
		}
		for (int l66 = 0; l66 < 2; l66 = l66 + 1) {
			fRec7[l66] = 0.0f;
		}
		for (int l67 = 0; l67 < 2; l67 = l67 + 1) {
			fRec6[l67] = 0.0f;
		}
		for (int l68 = 0; l68 < 2048; l68 = l68 + 1) {
			fVec10[l68] = 0.0f;
		}
		for (int l69 = 0; l69 < 2048; l69 = l69 + 1) {
			fVec11[l69] = 0.0f;
		}
		for (int l70 = 0; l70 < 2; l70 = l70 + 1) {
			fRec4[l70] = 0.0f;
		}
		for (int l71 = 0; l71 < 2; l71 = l71 + 1) {
			iRec2[l71] = 0;
		}
		for (int l72 = 0; l72 < 2; l72 = l72 + 1) {
			fRec1[l72] = 0.0f;
		}
		for (int l73 = 0; l73 < 2; l73 = l73 + 1) {
			fRec0[l73] = 0.0f;
		}
	}
	
	virtual void init(int sample_rate) {
		classInit(sample_rate);
		instanceInit(sample_rate);
	}
	virtual void instanceInit(int sample_rate) {
		instanceConstants(sample_rate);
		instanceResetUserInterface();
		instanceClear();
	}
	
	virtual master_me_jacktripdsp* clone() {
		return new master_me_jacktripdsp();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openHorizontalBox("MASTER_ME");
		ui_interface->declare(0, "1.5", "");
		ui_interface->openHorizontalBox("NOISEGATE");
		ui_interface->declare(&fVslider3, "0", "");
		ui_interface->addVerticalSlider("threshold", &fVslider3, FAUSTFLOAT(-70.0f), FAUSTFLOAT(-95.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f));
		ui_interface->declare(&fVbargraph1, "2", "");
		ui_interface->addVerticalBargraph("gate level", &fVbargraph1, FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f));
		ui_interface->closeBox();
		ui_interface->declare(0, "2", "");
		ui_interface->openHorizontalBox("LEVELER");
		ui_interface->declare(&fVbargraph3, "1", "");
		ui_interface->addVerticalBargraph("in LUFS S", &fVbargraph3, FAUSTFLOAT(-40.0f), FAUSTFLOAT(0.0f));
		ui_interface->declare(&fVbargraph4, "2", "");
		ui_interface->addVerticalBargraph("gain", &fVbargraph4, FAUSTFLOAT(-50.0f), FAUSTFLOAT(50.0f));
		ui_interface->declare(&fVslider8, "3", "");
		ui_interface->declare(&fVslider8, "unit", "dB");
		ui_interface->addVerticalSlider("target LUFS", &fVslider8, FAUSTFLOAT(-16.0f), FAUSTFLOAT(-50.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f));
		ui_interface->declare(&fVslider6, "4", "");
		ui_interface->addVerticalSlider("speed", &fVslider6, FAUSTFLOAT(0.0199999996f), FAUSTFLOAT(0.00499999989f), FAUSTFLOAT(0.100000001f), FAUSTFLOAT(0.00499999989f));
		ui_interface->declare(&fVslider5, "5", "");
		ui_interface->addVerticalSlider("max boost", &fVslider5, FAUSTFLOAT(6.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(50.0f), FAUSTFLOAT(1.0f));
		ui_interface->declare(&fVslider4, "6", "");
		ui_interface->addVerticalSlider("max cut", &fVslider4, FAUSTFLOAT(6.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(50.0f), FAUSTFLOAT(1.0f));
		ui_interface->declare(&fVbargraph2, "7", "");
		ui_interface->addVerticalBargraph("leveler gate", &fVbargraph2, FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f));
		ui_interface->declare(&fVslider7, "8", "");
		ui_interface->declare(&fVslider7, "unit", "dB");
		ui_interface->addVerticalSlider("lev gate thresh", &fVslider7, FAUSTFLOAT(-45.0f), FAUSTFLOAT(-90.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f));
		ui_interface->closeBox();
		ui_interface->declare(0, "3", "");
		ui_interface->openHorizontalBox("MULTIBAND MID-SIDE COMPRESSOR");
		ui_interface->declare(&fVslider2, "1", "");
		ui_interface->addVerticalSlider("threshold", &fVslider2, FAUSTFLOAT(-10.0f), FAUSTFLOAT(-60.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f));
		ui_interface->declare(&fVbargraph0, "2", "");
		ui_interface->declare(&fVbargraph0, "unit", "db");
		ui_interface->addVerticalBargraph("0x1ea8f90", &fVbargraph0, FAUSTFLOAT(0.0f), FAUSTFLOAT(6.0f));
		ui_interface->declare(&fVbargraph5, "2", "");
		ui_interface->declare(&fVbargraph5, "unit", "db");
		ui_interface->addVerticalBargraph("0x1f045d0", &fVbargraph5, FAUSTFLOAT(0.0f), FAUSTFLOAT(6.0f));
		ui_interface->declare(&fVbargraph6, "2", "");
		ui_interface->declare(&fVbargraph6, "unit", "db");
		ui_interface->addVerticalBargraph("0x1f15040", &fVbargraph6, FAUSTFLOAT(0.0f), FAUSTFLOAT(6.0f));
		ui_interface->declare(&fVbargraph7, "2", "");
		ui_interface->declare(&fVbargraph7, "unit", "db");
		ui_interface->addVerticalBargraph("0x1f1dd30", &fVbargraph7, FAUSTFLOAT(0.0f), FAUSTFLOAT(6.0f));
		ui_interface->declare(&fVbargraph8, "2", "");
		ui_interface->declare(&fVbargraph8, "unit", "db");
		ui_interface->addVerticalBargraph("0x1f2b020", &fVbargraph8, FAUSTFLOAT(0.0f), FAUSTFLOAT(6.0f));
		ui_interface->declare(&fVbargraph9, "2", "");
		ui_interface->declare(&fVbargraph9, "unit", "db");
		ui_interface->addVerticalBargraph("0x1f398c0", &fVbargraph9, FAUSTFLOAT(0.0f), FAUSTFLOAT(6.0f));
		ui_interface->declare(&fVslider1, "9", "");
		ui_interface->declare(&fVslider1, "unit", "dB");
		ui_interface->addVerticalSlider("makeup", &fVslider1, FAUSTFLOAT(0.0f), FAUSTFLOAT(-10.0f), FAUSTFLOAT(10.0f), FAUSTFLOAT(0.5f));
		ui_interface->closeBox();
		ui_interface->declare(0, "7", "");
		ui_interface->openHorizontalBox("LIMITER");
		ui_interface->declare(&fVbargraph10, "8", "");
		ui_interface->declare(&fVbargraph10, "unit", "dB");
		ui_interface->addVerticalBargraph("GR", &fVbargraph10, FAUSTFLOAT(0.0f), FAUSTFLOAT(12.0f));
		ui_interface->declare(&fVslider0, "9", "");
		ui_interface->declare(&fVslider0, "unit", "dB");
		ui_interface->addVerticalSlider("post gain", &fVslider0, FAUSTFLOAT(15.0f), FAUSTFLOAT(-10.0f), FAUSTFLOAT(15.0f), FAUSTFLOAT(0.5f));
		ui_interface->closeBox();
		ui_interface->declare(0, "8", "");
		ui_interface->openHorizontalBox("BRICKWALL");
		ui_interface->declare(&fVbargraph11, "8", "");
		ui_interface->declare(&fVbargraph11, "unit", "dB");
		ui_interface->addVerticalBargraph("GR", &fVbargraph11, FAUSTFLOAT(0.0f), FAUSTFLOAT(12.0f));
		ui_interface->closeBox();
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* input1 = inputs[1];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = std::pow(10.0f, 0.0500000007f * float(fVslider0));
		float fSlow1 = std::pow(10.0f, 0.0500000007f * float(fVslider1));
		float fSlow2 = float(fVslider2);
		float fSlow3 = fSlow2 + -3.0f;
		float fSlow4 = fSlow2 + 3.0f;
		float fSlow5 = std::pow(10.0f, 0.0500000007f * float(fVslider3));
		float fSlow6 = -1.0f * float(fVslider4);
		float fSlow7 = float(fVslider5);
		float fSlow8 = 6.28318548f * float(fVslider6);
		float fSlow9 = std::pow(10.0f, 0.0500000007f * float(fVslider7));
		float fSlow10 = float(fVslider8);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			float fTemp0 = float(input0[i0]);
			fVec0[0] = fTemp0;
			float fTemp1 = float(input1[i0]);
			fVec1[0] = fTemp1;
			iRec5[0] = (iRec5[1] + 1) % int(std::max<float>(1.0f, fConst3 * float(iRec2[1])));
			iRec11[0] = (iRec11[1] + 1) % int(std::max<float>(1.0f, fConst6 * float(iRec8[1])));
			float fTemp2 = std::fabs(fRec12[1]);
			float fTemp3 = ((fRec18[1] > fTemp2) ? fConst8 : fConst7);
			fRec19[0] = fTemp2 * (1.0f - fTemp3) + fTemp3 * fRec19[1];
			fRec18[0] = fRec19[0];
			float fTemp4 = 20.0f * std::log10(std::max<float>(1.17549435e-38f, fRec18[0]));
			int iTemp5 = (fTemp4 > fSlow3) + (fTemp4 > fSlow4);
			float fThen1 = fTemp4 - fSlow2;
			float fElse1 = 0.0833333358f * master_me_jacktripdsp_faustpower2_f((fTemp4 + 3.0f) - fSlow2);
			float fThen2 = ((iTemp5 == 1) ? fElse1 : fThen1);
			float fTemp6 = std::pow(10.0f, 0.0500000007f * (0.0f - 0.600000024f * std::max<float>(0.0f, ((iTemp5 == 0) ? 0.0f : fThen2))));
			float fTemp7 = std::fabs(fRec13[1]);
			float fTemp8 = ((fRec20[1] > fTemp7) ? fConst8 : fConst7);
			fRec21[0] = fTemp7 * (1.0f - fTemp8) + fTemp8 * fRec21[1];
			fRec20[0] = fRec21[0];
			float fTemp9 = 20.0f * std::log10(std::max<float>(1.17549435e-38f, fRec20[0]));
			int iTemp10 = (fTemp9 > fSlow3) + (fTemp9 > fSlow4);
			float fThen4 = fTemp9 - fSlow2;
			float fElse4 = 0.0833333358f * master_me_jacktripdsp_faustpower2_f((fTemp9 + 3.0f) - fSlow2);
			float fThen5 = ((iTemp10 == 1) ? fElse4 : fThen4);
			float fTemp11 = std::pow(10.0f, 0.0500000007f * (0.0f - 0.600000024f * std::max<float>(0.0f, ((iTemp10 == 0) ? 0.0f : fThen5))));
			float fTemp12 = std::fabs(fRec14[1]);
			float fTemp13 = ((fRec22[1] > fTemp12) ? fConst8 : fConst7);
			fRec23[0] = fTemp12 * (1.0f - fTemp13) + fTemp13 * fRec23[1];
			fRec22[0] = fRec23[0];
			float fTemp14 = 20.0f * std::log10(std::max<float>(1.17549435e-38f, fRec22[0]));
			int iTemp15 = (fTemp14 > fSlow3) + (fTemp14 > fSlow4);
			float fThen7 = fTemp14 - fSlow2;
			float fElse7 = 0.0833333358f * master_me_jacktripdsp_faustpower2_f((fTemp14 + 3.0f) - fSlow2);
			float fThen8 = ((iTemp15 == 1) ? fElse7 : fThen7);
			float fTemp16 = std::pow(10.0f, 0.0500000007f * (0.0f - 0.600000024f * std::max<float>(0.0f, ((iTemp15 == 0) ? 0.0f : fThen8))));
			float fTemp17 = std::fabs(fRec15[1]);
			float fTemp18 = ((fRec24[1] > fTemp17) ? fConst8 : fConst7);
			fRec25[0] = fTemp17 * (1.0f - fTemp18) + fTemp18 * fRec25[1];
			fRec24[0] = fRec25[0];
			float fTemp19 = 20.0f * std::log10(std::max<float>(1.17549435e-38f, fRec24[0]));
			int iTemp20 = (fTemp19 > fSlow3) + (fTemp19 > fSlow4);
			float fThen10 = fTemp19 - fSlow2;
			float fElse10 = 0.0833333358f * master_me_jacktripdsp_faustpower2_f((fTemp19 + 3.0f) - fSlow2);
			float fThen11 = ((iTemp20 == 1) ? fElse10 : fThen10);
			float fTemp21 = std::pow(10.0f, 0.0500000007f * (0.0f - 0.600000024f * std::max<float>(0.0f, ((iTemp20 == 0) ? 0.0f : fThen11))));
			float fTemp22 = std::fabs(fRec16[1]);
			float fTemp23 = ((fRec26[1] > fTemp22) ? fConst8 : fConst7);
			fRec27[0] = fTemp22 * (1.0f - fTemp23) + fTemp23 * fRec27[1];
			fRec26[0] = fRec27[0];
			float fTemp24 = 20.0f * std::log10(std::max<float>(1.17549435e-38f, fRec26[0]));
			int iTemp25 = (fTemp24 > fSlow3) + (fTemp24 > fSlow4);
			float fThen13 = fTemp24 - fSlow2;
			float fElse13 = 0.0833333358f * master_me_jacktripdsp_faustpower2_f((fTemp24 + 3.0f) - fSlow2);
			float fThen14 = ((iTemp25 == 1) ? fElse13 : fThen13);
			float fTemp26 = std::pow(10.0f, 0.0500000007f * (0.0f - 0.600000024f * std::max<float>(0.0f, ((iTemp25 == 0) ? 0.0f : fThen14))));
			float fTemp27 = std::fabs(fRec17[1]);
			float fTemp28 = ((fRec28[1] > fTemp27) ? fConst8 : fConst7);
			fRec29[0] = fTemp27 * (1.0f - fTemp28) + fTemp28 * fRec29[1];
			fRec28[0] = fRec29[0];
			float fTemp29 = 20.0f * std::log10(std::max<float>(1.17549435e-38f, fRec28[0]));
			int iTemp30 = (fTemp29 > fSlow3) + (fTemp29 > fSlow4);
			float fThen16 = fTemp29 - fSlow2;
			float fElse16 = 0.0833333358f * master_me_jacktripdsp_faustpower2_f((fTemp29 + 3.0f) - fSlow2);
			float fThen17 = ((iTemp30 == 1) ? fElse16 : fThen16);
			float fTemp31 = std::pow(10.0f, 0.0500000007f * (0.0f - 0.600000024f * std::max<float>(0.0f, ((iTemp30 == 0) ? 0.0f : fThen17))));
			float fTemp32 = std::min<float>(std::min<float>(std::min<float>(std::min<float>(std::min<float>(fTemp6, fTemp11), fTemp16), fTemp21), fTemp26), fTemp31);
			float fTemp33 = fTemp6 + 0.300000012f * (fTemp32 - fTemp6);
			fVbargraph0 = FAUSTFLOAT(std::fabs(20.0f * std::log10(std::max<float>(1.17549435e-38f, fTemp33))));
			fRec33[0] = (fTemp0 + 0.995000005f * fRec33[1]) - fVec0[1];
			fRec34[0] = (fTemp1 + 0.995000005f * fRec34[1]) - fVec1[1];
			float fTemp34 = fRec33[0] + fRec34[0];
			fRec38[0] = fConst5 * std::fabs(fTemp34) + fConst4 * fRec38[1];
			float fRec37 = fRec38[0];
			int iTemp35 = fRec37 > fSlow5;
			iVec2[0] = iTemp35;
			iRec39[0] = std::max<int>(int(iConst16 * (iTemp35 < iVec2[1])), int(iRec39[1] + -1));
			float fTemp36 = std::max<float>(float(iTemp35), float(iRec39[0] > 0));
			float fTemp37 = ((fRec35[1] > fTemp36) ? fConst17 : fConst4);
			fRec36[0] = fTemp36 * (1.0f - fTemp37) + fTemp37 * fRec36[1];
			fRec35[0] = fRec36[0];
			fVbargraph1 = FAUSTFLOAT(1.0f - fRec35[0]);
			float fTemp38 = fRec35[0];
			float fTemp39 = fTemp34 * fTemp38;
			fVec3[0] = fTemp39;
			fRec44[0] = fConst20 * std::fabs(fTemp39) + fConst19 * fRec44[1];
			float fRec43 = fRec44[0];
			iRec45[0] = 0;
			fRec42[0] = fConst20 * std::max<float>(float(fRec43 > fSlow9), float(iRec45[0] > 0)) + fConst19 * fRec42[1];
			float fRec41 = fRec42[0];
			fVbargraph2 = FAUSTFLOAT(1.0f - fRec41);
			float fTemp40 = std::exp(fConst18 * (0.0f - fSlow8 * fRec41));
			fRec49[0] = 0.0f - fConst32 * (fConst33 * fRec49[1] - (fTemp39 + fVec3[1]));
			fRec48[0] = fRec49[0] - fConst30 * (fConst34 * fRec48[2] + fConst37 * fRec48[1]);
			fRec51[0] = fConst38 * fVec3[1] - fConst32 * (fConst33 * fRec51[1] - fConst29 * fTemp39);
			fRec50[0] = fRec51[0] - fConst30 * (fConst34 * fRec50[2] + fConst37 * fRec50[1]);
			fRec47[0] = fConst30 * (fRec48[2] + fRec48[0] + 2.0f * fRec48[1] + 1.58489323f * (fConst36 * fRec50[0] + fConst39 * fRec50[1] + fConst36 * fRec50[2])) - fConst40 * (fConst41 * fRec47[2] + fConst42 * fRec47[1]);
			fRec46[0] = fConst21 * fRec46[1] + fConst25 * master_me_jacktripdsp_faustpower2_f(fConst27 * fRec47[0] + fConst43 * fRec47[1] + fConst27 * fRec47[2]);
			fVbargraph3 = FAUSTFLOAT(10.0f * std::log10(std::max<float>(1.1920929e-07f, fRec46[0])) + -0.690999985f);
			fRec40[0] = fRec40[1] * fTemp40 + (fSlow10 - fVbargraph3) * (1.0f - fTemp40);
			fVbargraph4 = FAUSTFLOAT(std::max<float>(fSlow6, std::min<float>(fSlow7, fRec40[0])));
			float fTemp41 = std::pow(10.0f, 0.0500000007f * fVbargraph4);
			float fTemp42 = fTemp39 * fTemp41;
			fVec4[0] = fTemp42;
			fRec32[0] = 0.5f * (fConst15 * fTemp42 + fConst44 * fVec4[1]) - fConst46 * fRec32[1];
			fRec31[0] = fRec32[0] - fConst11 * (fConst47 * fRec31[2] + fConst48 * fRec31[1]);
			float fTemp43 = fConst58 * fRec30[1];
			fRec30[0] = fConst11 * (fConst13 * fRec31[0] + fConst49 * fRec31[1] + fConst13 * fRec31[2]) - fConst53 * (fConst55 * fRec30[2] + fTemp43);
			fRec12[0] = fTemp33 * (fRec30[2] + fConst53 * (fTemp43 + fConst55 * fRec30[0]));
			float fTemp44 = fTemp11 + 0.300000012f * (fTemp32 - fTemp11);
			fVbargraph5 = FAUSTFLOAT(std::fabs(20.0f * std::log10(std::max<float>(1.17549435e-38f, fTemp44))));
			fRec55[0] = fConst61 * (0.5f * (fTemp42 + fVec4[1]) - fConst45 * fRec55[1]);
			fRec54[0] = fRec55[0] - fConst11 * (fConst47 * fRec54[2] + fConst48 * fRec54[1]);
			float fTemp45 = fRec54[2] + fRec54[0] + 2.0f * fRec54[1];
			fVec5[0] = fTemp45;
			fRec53[0] = fConst11 * (fConst60 * fTemp45 + fConst62 * fVec5[1]) - fConst63 * fRec53[1];
			fRec52[0] = fRec53[0] - fConst59 * (fConst64 * fRec52[2] + fConst58 * fRec52[1]);
			fRec13[0] = fConst59 * fTemp44 * (fConst57 * fRec52[0] + fConst65 * fRec52[1] + fConst57 * fRec52[2]);
			float fTemp46 = fTemp16 + 0.300000012f * (fTemp32 - fTemp16);
			fVbargraph6 = FAUSTFLOAT(std::fabs(20.0f * std::log10(std::max<float>(1.17549435e-38f, fTemp46))));
			fRec57[0] = 0.0f - fConst66 * (fConst54 * fRec57[1] - fConst11 * (fTemp45 + fVec5[1]));
			fRec56[0] = fRec57[0] - fConst59 * (fConst64 * fRec56[2] + fConst58 * fRec56[1]);
			fRec14[0] = fConst59 * fTemp46 * (fRec56[2] + fRec56[0] + 2.0f * fRec56[1]);
			float fTemp47 = fTemp21 + 0.300000012f * (fTemp32 - fTemp21);
			fVbargraph7 = FAUSTFLOAT(std::fabs(20.0f * std::log10(std::max<float>(1.17549435e-38f, fTemp47))));
			float fTemp48 = fTemp38 * fTemp41 * (fRec33[0] - fRec34[0]);
			fVec6[0] = fTemp48;
			fRec60[0] = 0.5f * (fConst15 * fTemp48 + fConst44 * fVec6[1]) - fConst46 * fRec60[1];
			fRec59[0] = fRec60[0] - fConst11 * (fConst47 * fRec59[2] + fConst48 * fRec59[1]);
			float fTemp49 = fConst58 * fRec58[1];
			fRec58[0] = fConst11 * (fConst13 * fRec59[0] + fConst49 * fRec59[1] + fConst13 * fRec59[2]) - fConst53 * (fConst55 * fRec58[2] + fTemp49);
			fRec15[0] = fTemp47 * (fRec58[2] + fConst53 * (fTemp49 + fConst55 * fRec58[0]));
			float fTemp50 = fTemp26 + 0.300000012f * (fTemp32 - fTemp26);
			fVbargraph8 = FAUSTFLOAT(std::fabs(20.0f * std::log10(std::max<float>(1.17549435e-38f, fTemp50))));
			fRec64[0] = fConst61 * (0.5f * (fTemp48 + fVec6[1]) - fConst45 * fRec64[1]);
			fRec63[0] = fRec64[0] - fConst11 * (fConst47 * fRec63[2] + fConst48 * fRec63[1]);
			float fTemp51 = fRec63[2] + fRec63[0] + 2.0f * fRec63[1];
			fVec7[0] = fTemp51;
			fRec62[0] = fConst11 * (fConst60 * fTemp51 + fConst62 * fVec7[1]) - fConst63 * fRec62[1];
			fRec61[0] = fRec62[0] - fConst59 * (fConst64 * fRec61[2] + fConst58 * fRec61[1]);
			fRec16[0] = fConst59 * fTemp50 * (fConst57 * fRec61[0] + fConst65 * fRec61[1] + fConst57 * fRec61[2]);
			float fTemp52 = fTemp31 + 0.300000012f * (fTemp32 - fTemp31);
			fVbargraph9 = FAUSTFLOAT(std::fabs(20.0f * std::log10(std::max<float>(1.17549435e-38f, fTemp52))));
			fRec66[0] = 0.0f - fConst66 * (fConst54 * fRec66[1] - fConst11 * (fTemp51 + fVec7[1]));
			fRec65[0] = fRec66[0] - fConst59 * (fConst64 * fRec65[2] + fConst58 * fRec65[1]);
			fRec17[0] = fConst59 * fTemp52 * (fRec65[2] + fRec65[0] + 2.0f * fRec65[1]);
			float fTemp53 = fRec12[0] + fRec13[0] + fRec14[0];
			float fTemp54 = fSlow1 * (fRec17[0] + fRec16[0] + fTemp53 + fRec15[0]);
			fVec8[IOTA0 & 2047] = fTemp54;
			float fTemp55 = fSlow1 * (fTemp53 - (fRec15[0] + fRec16[0] + fRec17[0]));
			fVec9[IOTA0 & 2047] = fTemp55;
			float fTemp56 = std::fabs(std::max<float>(std::fabs(fTemp54), std::fabs(fTemp55)));
			fRec10[0] = std::max<float>(float(iRec11[0] > 0) * fRec10[1], fTemp56);
			iRec8[0] = fRec10[0] >= fTemp56;
			float fRec9 = fRec10[0];
			fRec7[0] = fConst5 * fRec9 + fConst4 * fRec7[1];
			float fTemp57 = std::fabs(fRec7[0]);
			fRec6[0] = std::max<float>(fTemp57, fConst67 * fRec6[1] + fConst68 * fTemp57);
			float fTemp58 = std::min<float>(1.0f, 0.794328213f / std::max<float>(fRec6[0], 1.1920929e-07f));
			fVbargraph10 = FAUSTFLOAT(std::fabs(20.0f * std::log10(std::max<float>(1.17549435e-38f, fTemp58))));
			float fTemp59 = fTemp58;
			float fTemp60 = fSlow0 * fTemp59 * fVec8[(IOTA0 - iConst69) & 2047];
			fVec10[IOTA0 & 2047] = fTemp60;
			float fTemp61 = fSlow0 * fTemp59 * fVec9[(IOTA0 - iConst69) & 2047];
			fVec11[IOTA0 & 2047] = fTemp61;
			float fTemp62 = std::fabs(std::max<float>(std::fabs(fTemp60), std::fabs(fTemp61)));
			fRec4[0] = std::max<float>(float(iRec5[0] > 0) * fRec4[1], fTemp62);
			iRec2[0] = fRec4[0] >= fTemp62;
			float fRec3 = fRec4[0];
			fRec1[0] = fConst2 * fRec3 + fConst1 * fRec1[1];
			float fTemp63 = std::fabs(fRec1[0]);
			fRec0[0] = std::max<float>(fTemp63, fConst70 * fRec0[1] + fConst71 * fTemp63);
			float fTemp64 = std::min<float>(1.0f, 0.891250908f / std::max<float>(fRec0[0], 1.1920929e-07f));
			fVbargraph11 = FAUSTFLOAT(std::fabs(20.0f * std::log10(std::max<float>(1.17549435e-38f, fTemp64))));
			float fTemp65 = fTemp64;
			output0[i0] = FAUSTFLOAT(fTemp65 * fVec10[(IOTA0 - iConst69) & 2047]);
			output1[i0] = FAUSTFLOAT(fTemp65 * fVec11[(IOTA0 - iConst69) & 2047]);
			fVec0[1] = fVec0[0];
			fVec1[1] = fVec1[0];
			iRec5[1] = iRec5[0];
			iRec11[1] = iRec11[0];
			fRec19[1] = fRec19[0];
			fRec18[1] = fRec18[0];
			fRec21[1] = fRec21[0];
			fRec20[1] = fRec20[0];
			fRec23[1] = fRec23[0];
			fRec22[1] = fRec22[0];
			fRec25[1] = fRec25[0];
			fRec24[1] = fRec24[0];
			fRec27[1] = fRec27[0];
			fRec26[1] = fRec26[0];
			fRec29[1] = fRec29[0];
			fRec28[1] = fRec28[0];
			fRec33[1] = fRec33[0];
			fRec34[1] = fRec34[0];
			fRec38[1] = fRec38[0];
			iVec2[1] = iVec2[0];
			iRec39[1] = iRec39[0];
			fRec36[1] = fRec36[0];
			fRec35[1] = fRec35[0];
			fVec3[1] = fVec3[0];
			fRec44[1] = fRec44[0];
			iRec45[1] = iRec45[0];
			fRec42[1] = fRec42[0];
			fRec49[1] = fRec49[0];
			fRec48[2] = fRec48[1];
			fRec48[1] = fRec48[0];
			fRec51[1] = fRec51[0];
			fRec50[2] = fRec50[1];
			fRec50[1] = fRec50[0];
			fRec47[2] = fRec47[1];
			fRec47[1] = fRec47[0];
			fRec46[1] = fRec46[0];
			fRec40[1] = fRec40[0];
			fVec4[1] = fVec4[0];
			fRec32[1] = fRec32[0];
			fRec31[2] = fRec31[1];
			fRec31[1] = fRec31[0];
			fRec30[2] = fRec30[1];
			fRec30[1] = fRec30[0];
			fRec12[1] = fRec12[0];
			fRec55[1] = fRec55[0];
			fRec54[2] = fRec54[1];
			fRec54[1] = fRec54[0];
			fVec5[1] = fVec5[0];
			fRec53[1] = fRec53[0];
			fRec52[2] = fRec52[1];
			fRec52[1] = fRec52[0];
			fRec13[1] = fRec13[0];
			fRec57[1] = fRec57[0];
			fRec56[2] = fRec56[1];
			fRec56[1] = fRec56[0];
			fRec14[1] = fRec14[0];
			fVec6[1] = fVec6[0];
			fRec60[1] = fRec60[0];
			fRec59[2] = fRec59[1];
			fRec59[1] = fRec59[0];
			fRec58[2] = fRec58[1];
			fRec58[1] = fRec58[0];
			fRec15[1] = fRec15[0];
			fRec64[1] = fRec64[0];
			fRec63[2] = fRec63[1];
			fRec63[1] = fRec63[0];
			fVec7[1] = fVec7[0];
			fRec62[1] = fRec62[0];
			fRec61[2] = fRec61[1];
			fRec61[1] = fRec61[0];
			fRec16[1] = fRec16[0];
			fRec66[1] = fRec66[0];
			fRec65[2] = fRec65[1];
			fRec65[1] = fRec65[0];
			fRec17[1] = fRec17[0];
			IOTA0 = IOTA0 + 1;
			fRec10[1] = fRec10[0];
			iRec8[1] = iRec8[0];
			fRec7[1] = fRec7[0];
			fRec6[1] = fRec6[0];
			fRec4[1] = fRec4[0];
			iRec2[1] = iRec2[0];
			fRec1[1] = fRec1[0];
			fRec0[1] = fRec0[0];
		}
	}

};


#endif
