/* ------------------------------------------------------------
author: "Julius Smith"
license: "MIT Style STK-4.2"
name: "compressor"
version: "0.0"
Code generated with Faust 2.40.0 (https://faust.grame.fr)
Compilation options: -a faust2header.cpp -lang cpp -i -inpl -cn compressordsp -es 1 -mcd
16 -single -ftz 0
------------------------------------------------------------ */

#ifndef __compressordsp_H__
#define __compressordsp_H__

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

class dsp
{
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
     * Trigger the Meta* parameter with instance specific calls to 'declare' (key, value)
     * metadata.
     *
     * @param m - the Meta* meta user
     */
    virtual void metadata(Meta* m) = 0;

    /**
     * DSP instance computation, to be called with successive in/out audio buffers.
     *
     * @param count - the number of frames to compute
     * @param inputs - the input audio buffers as an array of non-interleaved FAUSTFLOAT
     * samples (eiher float, double or quad)
     * @param outputs - the output audio buffers as an array of non-interleaved FAUSTFLOAT
     * samples (eiher float, double or quad)
     *
     */
    virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) = 0;

    /**
     * DSP instance computation: alternative method to be used by subclasses.
     *
     * @param date_usec - the timestamp in microsec given by audio driver.
     * @param count - the number of frames to compute
     * @param inputs - the input audio buffers as an array of non-interleaved FAUSTFLOAT
     * samples (either float, double or quad)
     * @param outputs - the output audio buffers as an array of non-interleaved FAUSTFLOAT
     * samples (either float, double or quad)
     *
     */
    virtual void compute(double /*date_usec*/, int count, FAUSTFLOAT** inputs,
                         FAUSTFLOAT** outputs)
    {
        compute(count, inputs, outputs);
    }
};

/**
 * Generic DSP decorator.
 */

class decorator_dsp : public dsp
{
   protected:
    dsp* fDSP;

   public:
    decorator_dsp(dsp* dsp = nullptr) : fDSP(dsp) {}
    virtual ~decorator_dsp() { delete fDSP; }

    virtual int getNumInputs() { return fDSP->getNumInputs(); }
    virtual int getNumOutputs() { return fDSP->getNumOutputs(); }
    virtual void buildUserInterface(UI* ui_interface)
    {
        fDSP->buildUserInterface(ui_interface);
    }
    virtual int getSampleRate() { return fDSP->getSampleRate(); }
    virtual void init(int sample_rate) { fDSP->init(sample_rate); }
    virtual void instanceInit(int sample_rate) { fDSP->instanceInit(sample_rate); }
    virtual void instanceConstants(int sample_rate)
    {
        fDSP->instanceConstants(sample_rate);
    }
    virtual void instanceResetUserInterface() { fDSP->instanceResetUserInterface(); }
    virtual void instanceClear() { fDSP->instanceClear(); }
    virtual decorator_dsp* clone() { return new decorator_dsp(fDSP->clone()); }
    virtual void metadata(Meta* m) { fDSP->metadata(m); }
    // Beware: subclasses usually have to overload the two 'compute' methods
    virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs)
    {
        fDSP->compute(count, inputs, outputs);
    }
    virtual void compute(double date_usec, int count, FAUSTFLOAT** inputs,
                         FAUSTFLOAT** outputs)
    {
        fDSP->compute(date_usec, count, inputs, outputs);
    }
};

/**
 * DSP factory class, used with LLVM and Interpreter backends
 * to create DSP instances from a compiled DSP program.
 */

class dsp_factory
{
   protected:
    // So that to force sub-classes to use deleteDSPFactory(dsp_factory* factory);
    virtual ~dsp_factory() {}

   public:
    virtual std::string getName()                          = 0;
    virtual std::string getSHAKey()                        = 0;
    virtual std::string getDSPCode()                       = 0;
    virtual std::string getCompileOptions()                = 0;
    virtual std::vector<std::string> getLibraryList()      = 0;
    virtual std::vector<std::string> getIncludePathnames() = 0;

    virtual dsp* createDSPInstance() = 0;

    virtual void setMemoryManager(dsp_memory_manager* manager) = 0;
    virtual dsp_memory_manager* getMemoryManager()             = 0;
};

// Denormal handling

#if defined(__SSE__)
#include <xmmintrin.h>
#endif

class ScopedNoDenormals
{
   private:
    intptr_t fpsr;

    void setFpStatusRegister(intptr_t fpsr_aux) noexcept
    {
#if defined(__arm64__) || defined(__aarch64__)
        asm volatile("msr fpcr, %0" : : "ri"(fpsr_aux));
#elif defined(__SSE__)
        _mm_setcsr(static_cast<uint32_t>(fpsr_aux));
#endif
    }

    void getFpStatusRegister() noexcept
    {
#if defined(__arm64__) || defined(__aarch64__)
        asm volatile("mrs %0, fpcr" : "=r"(fpsr));
#elif defined(__SSE__)
        fpsr          = static_cast<intptr_t>(_mm_getcsr());
#endif
    }

   public:
    ScopedNoDenormals() noexcept
    {
#if defined(__arm64__) || defined(__aarch64__)
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

    ~ScopedNoDenormals() noexcept { setFpStatusRegister(fpsr); }
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

#include <cstdio>
#include <map>
#include <sstream>
#include <string>
#include <vector>

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
 The base class of Meta handler to be used in dsp::metadata(Meta* m) method to retrieve
 (key, value) metadata.
 */
struct Meta {
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

template<typename REAL>
struct UIReal {
    UIReal() {}
    virtual ~UIReal() {}

    // -- widget's layouts

    virtual void openTabBox(const char* label)        = 0;
    virtual void openHorizontalBox(const char* label) = 0;
    virtual void openVerticalBox(const char* label)   = 0;
    virtual void closeBox()                           = 0;

    // -- active widgets

    virtual void addButton(const char* label, REAL* zone)      = 0;
    virtual void addCheckButton(const char* label, REAL* zone) = 0;
    virtual void addVerticalSlider(const char* label, REAL* zone, REAL init, REAL min,
                                   REAL max, REAL step)        = 0;
    virtual void addHorizontalSlider(const char* label, REAL* zone, REAL init, REAL min,
                                     REAL max, REAL step)      = 0;
    virtual void addNumEntry(const char* label, REAL* zone, REAL init, REAL min, REAL max,
                             REAL step)                        = 0;

    // -- passive widgets

    virtual void addHorizontalBargraph(const char* label, REAL* zone, REAL min,
                                       REAL max) = 0;
    virtual void addVerticalBargraph(const char* label, REAL* zone, REAL min,
                                     REAL max)   = 0;

    // -- soundfiles

    virtual void addSoundfile(const char* label, const char* filename,
                              Soundfile** sf_zone) = 0;

    // -- metadata declarations

    virtual void declare(REAL* zone, const char* key, const char* val) {}

    // To be used by LLVM client
    virtual int sizeOfFAUSTFLOAT() { return sizeof(FAUSTFLOAT); }
};

struct UI : public UIReal<FAUSTFLOAT> {
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

#include <algorithm>
#include <string>
#include <vector>

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
        for (c(char & i : str)
            if (std::find(beg, end, i) != end) {
           i = ch2;
            }
      i
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
        std::vector<char> rep = {' ', '#', '*', ',', '/', '?',
                                 '[', ']', '{', '}', '(', ')'};
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
 Interpolator(lo,hi,v1,v2) : Maps a value x between lo and hi to a value y between v1 and
v2 Interpolator3pt(lo,mi,hi,v1,vm,v2) : Map values between lo mid hi to values between v1
vm v2

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

#include <algorithm>  // std::max
#include <cassert>
#include <cfloat>
#include <cmath>
#include <vector>

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
    struct Range {
        double fLo;
        double fHi;

        Range(double x, double y)
            : fLo(std::min<double>(x, y)), fHi(std::max<double>(x, y))
        {
        }
        double operator()(double x) { return (x < fLo) ? fLo : (x > fHi) ? fHi : x; }
    };

    Range fRange;
    double fCoef;
    double fOffset;

   public:
    Interpolator(double lo, double hi, double v1, double v2) : fRange(lo, hi)
    {
        if (hi != lo) {
            // regular case
            fCoef   = (v2 - v1) / (hi - lo);
            fOffset = v1 - lo * fCoef;
        } else {
            // degenerate case, avoids division by zero
            fCoef   = 0;
            fOffset = (v1 + v2) / 2;
        }
    }
    double operator()(double v)
    {
        double x = fRange(v);
        return fOffset + x * fCoef;
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
    Interpolator3pt(double lo, double mi, double hi, double v1, double vm, double v2)
        : fSegment1(lo, mi, v1, vm), fSegment2(mi, hi, vm, v2), fMid(mi)
    {
    }
    double operator()(double x) { return (x < fMid) ? fSegment1(x) : fSegment2(x); }

    void getMappingValues(double& amin, double& amid, double& amax)
    {
        fSegment1.getLowHigh(amin, amid);
        fSegment2.getLowHigh(amid, amax);
    }
};

//--------------------------------------------------------------------------------------
// Abstract ValueConverter class. Converts values between UI and Faust representations
//--------------------------------------------------------------------------------------
class ValueConverter  // Identity by default
{
   public:
    virtual ~ValueConverter() {}
    virtual double ui2faust(double x) { return x; };
    virtual double faust2ui(double x) { return x; };
};

//--------------------------------------------------------------------------------------
// A converter than can be updated
//--------------------------------------------------------------------------------------

class UpdatableValueConverter : public ValueConverter
{
   protected:
    bool fActive;

   public:
    UpdatableValueConverter() : fActive(true) {}
    virtual ~UpdatableValueConverter() {}

    virtual void setMappingValues(double amin, double amid, double amax, double min,
                                  double init, double max)                  = 0;
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
    LinearValueConverter(double umin, double umax, double fmin, double fmax)
        : fUI2F(umin, umax, fmin, fmax), fF2UI(fmin, fmax, umin, umax)
    {
    }

    LinearValueConverter() : fUI2F(0., 0., 0., 0.), fF2UI(0., 0., 0., 0.) {}
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
    LinearValueConverter2(double amin, double amid, double amax, double min, double init,
                          double max)
        : fUI2F(amin, amid, amax, min, init, max), fF2UI(min, init, max, amin, amid, amax)
    {
    }

    LinearValueConverter2() : fUI2F(0., 0., 0., 0., 0., 0.), fF2UI(0., 0., 0., 0., 0., 0.)
    {
    }

    virtual double ui2faust(double x) { return fUI2F(x); }
    virtual double faust2ui(double x) { return fF2UI(x); }

    virtual void setMappingValues(double amin, double amid, double amax, double min,
                                  double init, double max)
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
    LogValueConverter(double umin, double umax, double fmin, double fmax)
        : LinearValueConverter(umin, umax, std::log(std::max<double>(DBL_MIN, fmin)),
                               std::log(std::max<double>(DBL_MIN, fmax)))
    {
    }

    virtual double ui2faust(double x)
    {
        return std::exp(LinearValueConverter::ui2faust(x));
    }
    virtual double faust2ui(double x)
    {
        return LinearValueConverter::faust2ui(std::log(std::max<double>(x, DBL_MIN)));
    }
};

//--------------------------------------------------------------------------------------
// Exponential conversion between ui and Faust values
//--------------------------------------------------------------------------------------
class ExpValueConverter : public LinearValueConverter
{
   public:
    ExpValueConverter(double umin, double umax, double fmin, double fmax)
        : LinearValueConverter(umin, umax, std::min<double>(DBL_MAX, std::exp(fmin)),
                               std::min<double>(DBL_MAX, std::exp(fmax)))
    {
    }

    virtual double ui2faust(double x)
    {
        return std::log(LinearValueConverter::ui2faust(x));
    }
    virtual double faust2ui(double x)
    {
        return LinearValueConverter::faust2ui(std::min<double>(DBL_MAX, std::exp(x)));
    }
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
    AccUpConverter(double amin, double amid, double amax, double fmin, double fmid,
                   double fmax)
        : fA2F(amin, amid, amax, fmin, fmid, fmax)
        , fF2A(fmin, fmid, fmax, amin, amid, amax)
    {
    }

    virtual double ui2faust(double x) { return fA2F(x); }
    virtual double faust2ui(double x) { return fF2A(x); }

    virtual void setMappingValues(double amin, double amid, double amax, double fmin,
                                  double fmid, double fmax)
    {
        //__android_log_print(ANDROID_LOG_ERROR, "Faust", "AccUpConverter update %f %f %f
        //%f %f %f", amin,amid,amax,fmin,fmid,fmax);
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
    Interpolator3pt fA2F;
    Interpolator3pt fF2A;

   public:
    AccDownConverter(double amin, double amid, double amax, double fmin, double fmid,
                     double fmax)
        : fA2F(amin, amid, amax, fmax, fmid, fmin)
        , fF2A(fmin, fmid, fmax, amax, amid, amin)
    {
    }

    virtual double ui2faust(double x) { return fA2F(x); }
    virtual double faust2ui(double x) { return fF2A(x); }

    virtual void setMappingValues(double amin, double amid, double amax, double fmin,
                                  double fmid, double fmax)
    {
        //__android_log_print(ANDROID_LOG_ERROR, "Faust", "AccDownConverter update %f %f
        //%f %f %f %f", amin,amid,amax,fmin,fmid,fmax);
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
    Interpolator3pt fA2F;
    Interpolator fF2A;

   public:
    AccUpDownConverter(double amin, double amid, double amax, double fmin, double fmid,
                       double fmax)
        : fA2F(amin, amid, amax, fmin, fmax, fmin)
        , fF2A(fmin, fmax, amin,
               amax)  // Special, pseudo inverse of a non monotonic function
    {
    }

    virtual double ui2faust(double x) { return fA2F(x); }
    virtual double faust2ui(double x) { return fF2A(x); }

    virtual void setMappingValues(double amin, double amid, double amax, double fmin,
                                  double fmid, double fmax)
    {
        //__android_log_print(ANDROID_LOG_ERROR, "Faust", "AccUpDownConverter update %f %f
        //%f %f %f %f", amin,amid,amax,fmin,fmid,fmax);
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
    Interpolator3pt fA2F;
    Interpolator fF2A;

   public:
    AccDownUpConverter(double amin, double amid, double amax, double fmin, double fmid,
                       double fmax)
        : fA2F(amin, amid, amax, fmax, fmin, fmax)
        , fF2A(fmin, fmax, amin,
               amax)  // Special, pseudo inverse of a non monotonic function
    {
    }

    virtual double ui2faust(double x) { return fA2F(x); }
    virtual double faust2ui(double x) { return fF2A(x); }

    virtual void setMappingValues(double amin, double amid, double amax, double fmin,
                                  double fmid, double fmax)
    {
        //__android_log_print(ANDROID_LOG_ERROR, "Faust", "AccDownUpConverter update %f %f
        //%f %f %f %f", amin,amid,amax,fmin,fmid,fmax);
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
    FAUSTFLOAT* fZone;

   public:
    ZoneControl(FAUSTFLOAT* zone) : fZone(zone) {}
    virtual ~ZoneControl() {}

    virtual void update(double v) const {}

    virtual void setMappingValues(int curve, double amin, double amid, double amax,
                                  double min, double init, double max)
    {
    }
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
    ConverterZoneControl(FAUSTFLOAT* zone, ValueConverter* converter)
        : ZoneControl(zone), fValueConverter(converter)
    {
    }
    virtual ~ConverterZoneControl()
    {
        delete fValueConverter;
    }  // Assuming fValueConverter is not kept elsewhere...

    virtual void update(double v) const
    {
        *fZone = FAUSTFLOAT(fValueConverter->ui2faust(v));
    }

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
    CurveZoneControl(FAUSTFLOAT* zone, int curve, double amin, double amid, double amax,
                     double min, double init, double max)
        : ZoneControl(zone), fCurve(0)
    {
        assert(curve >= 0 && curve <= 3);
        fValueConverters.push_back(new AccUpConverter(amin, amid, amax, min, init, max));
        fValueConverters.push_back(
            new AccDownConverter(amin, amid, amax, min, init, max));
        fValueConverters.push_back(
            new AccUpDownConverter(amin, amid, amax, min, init, max));
        fValueConverters.push_back(
            new AccDownUpConverter(amin, amid, amax, min, init, max));
        fCurve = curve;
    }
    virtual ~CurveZoneControl()
    {
        for (const auto& it : fValueConverters) {
            delete it;
        }
    }
    void update(double v) const
    {
        if (fValueConverters[fCurve]->getActive())
            *fZone = FAUSTFLOAT(fValueConverters[fCurve]->ui2faust(v));
    }

    void setMappingValues(int curve, double amin, double amid, double amax, double min,
                          double init, double max)
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
        for (const auto& it : fValueConverters) {
            it->setActive(on_off);
        }
    }

    int getCurve() { return fCurve; }
};

class ZoneReader
{
   private:
    FAUSTFLOAT* fZone;
    Interpolator fInterpolator;

   public:
    ZoneReader(FAUSTFLOAT* zone, double lo, double hi)
        : fZone(zone), fInterpolator(lo, hi, 0, 255)
    {
    }

    virtual ~ZoneReader() {}

    int getValue() { return (fZone != nullptr) ? int(fInterpolator(*fZone)) : 127; }
};

#endif
/**************************  END  ValueConverter.h **************************/

typedef unsigned int uint;

class APIUI
    : public PathBuilder
    , public Meta
    , public UI
{
   public:
    enum ItemType {
        kButton = 0,
        kCheckButton,
        kVSlider,
        kHSlider,
        kNumEntry,
        kHBargraph,
        kVBargraph
    };
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
    bool fHasScreenControl;  // true if control screen color metadata
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
    virtual void addParameter(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init,
                              FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step,
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

        fItems.push_back({path, label, converter, zone, init, min, max, step, type});

        if (fCurrentAcc.size() > 0 && fCurrentGyr.size() > 0) {
            fprintf(
                stderr,
                "warning : 'acc' and 'gyr' metadata used for the same %s parameter !!\n",
                label);
        }

        // handle acc metadata "...[acc : <axe> <curve> <amin> <amid> <amax>]..."
        if (fCurrentAcc.size() > 0) {
            std::istringstream iss(fCurrentAcc);
            int axe, curve;
            double amin, amid, amax;
            iss >> axe >> curve >> amin >> amid >> amax;

            if ((0 <= axe) && (axe < 3) && (0 <= curve) && (curve < 4) && (amin < amax)
                && (amin <= amid) && (amid <= amax)) {
                fAcc[axe].push_back(
                    new CurveZoneControl(zone, curve, amin, amid, amax, min, init, max));
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

            if ((0 <= axe) && (axe < 3) && (0 <= curve) && (curve < 4) && (amin < amax)
                && (amin <= amid) && (amid <= amax)) {
                fGyr[axe].push_back(
                    new CurveZoneControl(zone, curve, amin, amid, amax, min, init, max));
            } else {
                fprintf(stderr, "incorrect gyr metadata : %s \n", fCurrentGyr.c_str());
            }
            fCurrentGyr = "";
        }

        // handle screencolor metadata "...[screencolor:red|green|blue|white]..."
        if (fCurrentColor.size() > 0) {
            if ((fCurrentColor == "red") && (fRedReader == nullptr)) {
                fRedReader        = new ZoneReader(zone, min, max);
                fHasScreenControl = true;
            } else if ((fCurrentColor == "green") && (fGreenReader == nullptr)) {
                fGreenReader      = new ZoneReader(zone, min, max);
                fHasScreenControl = true;
            } else if ((fCurrentColor == "blue") && (fBlueReader == nullptr)) {
                fBlueReader       = new ZoneReader(zone, min, max);
                fHasScreenControl = true;
            } else if ((fCurrentColor == "white") && (fRedReader == nullptr)
                       && (fGreenReader == nullptr) && (fBlueReader == nullptr)) {
                fRedReader        = new ZoneReader(zone, min, max);
                fGreenReader      = new ZoneReader(zone, min, max);
                fBlueReader       = new ZoneReader(zone, min, max);
                fHasScreenControl = true;
            } else {
                fprintf(stderr, "incorrect screencolor metadata : %s \n",
                        fCurrentColor.c_str());
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
            if (zone == table[val][i]->getZone())
                return int(i);
        }
        return -1;
    }

    void setConverter(std::vector<ZoneControl*>* table, int p, int val, int curve,
                      double amin, double amid, double amax)
    {
        int id1 = getZoneIndex(table, p, 0);
        int id2 = getZoneIndex(table, p, 1);
        int id3 = getZoneIndex(table, p, 2);

        // Deactivates everywhere..
        if (id1 != -1)
            table[0][uint(id1)]->setActive(false);
        if (id2 != -1)
            table[1][uint(id2)]->setActive(false);
        if (id3 != -1)
            table[2][uint(id3)]->setActive(false);

        if (val == -1) {  // Means: no more mapping...
            // So stay all deactivated...
        } else {
            int id4 = getZoneIndex(table, p, val);
            if (id4 != -1) {
                // Reactivate the one we edit...
                table[val][uint(id4)]->setMappingValues(
                    curve, amin, amid, amax, fItems[uint(p)].fMin, fItems[uint(p)].fInit,
                    fItems[uint(p)].fMax);
                table[val][uint(id4)]->setActive(true);
            } else {
                // Allocate a new CurveZoneControl which is 'active' by default
                FAUSTFLOAT* zone = fItems[uint(p)].fZone;
                table[val].push_back(new CurveZoneControl(
                    zone, curve, amin, amid, amax, fItems[uint(p)].fMin,
                    fItems[uint(p)].fInit, fItems[uint(p)].fMax));
            }
        }
    }

    void getConverter(std::vector<ZoneControl*>* table, int p, int& val, int& curve,
                      double& amin, double& amid, double& amax)
    {
        int id1 = getZoneIndex(table, p, 0);
        int id2 = getZoneIndex(table, p, 1);
        int id3 = getZoneIndex(table, p, 2);

        if (id1 != -1) {
            val   = 0;
            curve = table[val][uint(id1)]->getCurve();
            table[val][uint(id1)]->getMappingValues(amin, amid, amax);
        } else if (id2 != -1) {
            val   = 1;
            curve = table[val][uint(id2)]->getCurve();
            table[val][uint(id2)]->getMappingValues(amin, amid, amax);
        } else if (id3 != -1) {
            val   = 2;
            curve = table[val][uint(id3)]->getCurve();
            table[val][uint(id3)]->getMappingValues(amin, amid, amax);
        } else {
            val   = -1;  // No mapping
            curve = 0;
            amin  = -100.;
            amid  = 0.;
            amax  = 100.;
        }
    }

   public:
    APIUI()
        : fHasScreenControl(false)
        , fRedReader(nullptr)
        , fGreenReader(nullptr)
        , fBlueReader(nullptr)
        , fCurrentScale(kLin)
    {
    }

    virtual ~APIUI()
    {
        for (const auto& it : fItems)
            delete it.fConversion;
        for (int i = 0; i < 3; i++) {
            for (const auto& it : fAcc[i])
                delete it;
            for (const auto& it : fGyr[i])
                delete it;
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

    virtual void addVerticalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init,
                                   FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step)
    {
        addParameter(label, zone, init, min, max, step, kVSlider);
    }

    virtual void addHorizontalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init,
                                     FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step)
    {
        addParameter(label, zone, init, min, max, step, kHSlider);
    }

    virtual void addNumEntry(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init,
                             FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step)
    {
        addParameter(label, zone, init, min, max, step, kNumEntry);
    }

    // -- passive widgets

    virtual void addHorizontalBargraph(const char* label, FAUSTFLOAT* zone,
                                       FAUSTFLOAT min, FAUSTFLOAT max)
    {
        addParameter(label, zone, min, min, max, (max - min) / 1000.0f, kHBargraph);
    }

    virtual void addVerticalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min,
                                     FAUSTFLOAT max)
    {
        addParameter(label, zone, min, min, max, (max - min) / 1000.0f, kVBargraph);
    }

    // -- soundfiles

    virtual void addSoundfile(const char* label, const char* filename,
                              Soundfile** sf_zone)
    {
    }

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
            fCurrentColor = val;  // val = "red", "green", "blue" or "white"
        } else if (strcmp(key, "tooltip") == 0) {
            fCurrentTooltip = val;
        }
    }

    virtual void declare(const char* key, const char* val) {}

    //-------------------------------------------------------------------------------
    // Simple API part
    //-------------------------------------------------------------------------------
    int getParamsCount() { return int(fItems.size()); }

    int getParamIndex(const char* path)
    {
        auto it1 = find_if(fItems.begin(), fItems.end(), [=](const Item& it) {
            return it.fPath == std::string(path);
        });
        if (it1 != fItems.end()) {
            return int(it1 - fItems.begin());
        }

        auto it2 = find_if(fItems.begin(), fItems.end(), [=](const Item& it) {
            return it.fLabel == std::string(path);
        });
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
        return (fMetaData[uint(p)].find(key) != fMetaData[uint(p)].end())
                   ? fMetaData[uint(p)][key].c_str()
                   : "";
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
            fprintf(stderr, "setParamValue : '%s' not found\n",
                    (path == nullptr ? "NULL" : path));
        }
    }

    double getParamRatio(int p)
    {
        return fItems[uint(p)].fConversion->faust2ui(*fItems[uint(p)].fZone);
    }
    void setParamRatio(int p, double r)
    {
        *fItems[uint(p)].fZone = FAUSTFLOAT(fItems[uint(p)].fConversion->ui2faust(r));
    }

    double value2ratio(int p, double r)
    {
        return fItems[uint(p)].fConversion->faust2ui(r);
    }
    double ratio2value(int p, double r)
    {
        return fItems[uint(p)].fConversion->ui2faust(r);
    }

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
            if (getZoneIndex(fAcc, p, 0) != -1 || getZoneIndex(fAcc, p, 1) != -1
                || getZoneIndex(fAcc, p, 2) != -1) {
                return kAcc;
            } else if (getZoneIndex(fGyr, p, 0) != -1 || getZoneIndex(fGyr, p, 1) != -1
                       || getZoneIndex(fGyr, p, 2) != -1) {
                return kGyr;
            }
        }
        return kNoType;
    }

    /**
     * Return the Item type (kButton = 0, kCheckButton, kVSlider, kHSlider, kNumEntry,
     * kHBargraph, kVBargraph) for a given parameter.
     *
     * @param p - the UI parameter index
     *
     * @return the Item type
     */
    ItemType getParamItemType(int p) { return fItems[uint(p)].fItemType; }

    /**
     * Set a new value coming from an accelerometer, propagate it to all relevant
     * FAUSTFLOAT* zones.
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
     * Used to edit accelerometer curves and mapping. Set curve and related mapping for a
     * given UI parameter.
     *
     * @param p - the UI parameter index
     * @param acc - 0 for X accelerometer, 1 for Y accelerometer, 2 for Z accelerometer
     * (-1 means "no mapping")
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
     * Used to edit gyroscope curves and mapping. Set curve and related mapping for a
     * given UI parameter.
     *
     * @param p - the UI parameter index
     * @param acc - 0 for X gyroscope, 1 for Y gyroscope, 2 for Z gyroscope (-1 means "no
     * mapping")
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
     * Used to edit accelerometer curves and mapping. Get curve and related mapping for a
     * given UI parameter.
     *
     * @param p - the UI parameter index
     * @param acc - the acc value to be retrieved (-1 means "no mapping")
     * @param curve - the curve value to be retrieved
     * @param amin - the amin value to be retrieved
     * @param amid - the amid value to be retrieved
     * @param amax - the amax value to be retrieved
     *
     */
    void getAccConverter(int p, int& acc, int& curve, double& amin, double& amid,
                         double& amax)
    {
        getConverter(fAcc, p, acc, curve, amin, amid, amax);
    }

    /**
     * Used to edit gyroscope curves and mapping. Get curve and related mapping for a
     * given UI parameter.
     *
     * @param p - the UI parameter index
     * @param gyr - the gyr value to be retrieved (-1 means "no mapping")
     * @param curve - the curve value to be retrieved
     * @param amin - the amin value to be retrieved
     * @param amid - the amid value to be retrieved
     * @param amax - the amax value to be retrieved
     *
     */
    void getGyrConverter(int p, int& gyr, int& curve, double& amin, double& amid,
                         double& amax)
    {
        getConverter(fGyr, p, gyr, curve, amin, amid, amax);
    }

    /**
     * Set a new value coming from an gyroscope, propagate it to all relevant FAUSTFLOAT*
     * zones.
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
    int getAccCount(int acc) { return (acc >= 0 && acc < 3) ? int(fAcc[acc].size()) : 0; }

    /**
     * Get the number of FAUSTFLOAT* zones controlled with the gyroscope.
     *
     * @param gyr - 0 for X gyroscope, 1 for Y gyroscope, 2 for Z gyroscope
     * @param the number of zones
     *
     */
    int getGyrCount(int gyr) { return (gyr >= 0 && gyr < 3) ? int(fGyr[gyr].size()) : 0; }

    // getScreenColor() : -1 means no screen color control (no screencolor metadata found)
    // otherwise return 0x00RRGGBB a ready to use color
    int getScreenColor()
    {
        if (fHasScreenControl) {
            int r = (fRedReader) ? fRedReader->getValue() : 0;
            int g = (fGreenReader) ? fGreenReader->getValue() : 0;
            int b = (fBlueReader) ? fBlueReader->getValue() : 0;
            return (r << 16) | (g << 8) | b;
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
// clang-format off

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cmath>

#ifnde<cmath>LASS 
#define <cmath>SS compressordsp
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


class compressordsp : public dsp {
	
 private:
	
	FAUSTFLOAT fCheckbox0;
	FAUSTFLOAT fHslider0;
	FAUSTFLOAT fHslider1;
	int fSampleRate;
	float fConst0;
	FAUSTFLOAT fHslider2;
	FAUSTFLOAT fHslider3;
	float fRec5[2];
	float fRec4[2];
	FAUSTFLOAT fHslider4;
	float fRec3[2];
	float fRec2[2];
	float fRec1[2];
	float fRec0[2];
	FAUSTFLOAT fHbargraph0;
	
 public:
	
	void metadata(Meta* m) { 
		m->declare("analyzers.lib/name", "Faust Analyzer Library");
		m->declare("analyzers.lib/version", "0.1");
		m->declare("author", "Julius Smith");
		m->declare("basics.lib/bypass1:author", "Julius Smith");
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "0.5");
		m->declare("compile_options", "-a faust2header.cpp -lang cpp -i -inpl -cn compressordsp -es 1 -mcd 16 -single -ftz 0");
		m->declare("compressors.lib/compression_gain_mono:author", "Julius O. Smith III");
		m->declare("compressors.lib/compression_gain_mono:copyright", "Copyright (C) 2014-2020 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("compressors.lib/compression_gain_mono:license", "MIT-style STK-4.3 license");
		m->declare("compressors.lib/compressor_lad_mono:author", "Julius O. Smith III");
		m->declare("compressors.lib/compressor_lad_mono:copyright", "Copyright (C) 2014-2020 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("compressors.lib/compressor_lad_mono:license", "MIT-style STK-4.3 license");
		m->declare("compressors.lib/compressor_mono:author", "Julius O. Smith III");
		m->declare("compressors.lib/compressor_mono:copyright", "Copyright (C) 2014-2020 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("compressors.lib/compressor_mono:license", "MIT-style STK-4.3 license");
		m->declare("compressors.lib/name", "Faust Compressor Effect Library");
		m->declare("compressors.lib/version", "0.2");
		m->declare("description", "Compressor demo application, adapted from the Faust Library's dm.compressor_demo in demos.lib");
		m->declare("documentation", "https://faustlibraries.grame.fr/libs/compressors/#cocompressor_mono");
		m->declare("filename", "compressordsp.dsp");
		m->declare("license", "MIT Style STK-4.2");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.5");
		m->declare("name", "compressor");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "0.2");
		m->declare("signals.lib/name", "Faust Signal Routing Library");
		m->declare("signals.lib/version", "0.1");
		m->declare("version", "0.0");
	}

	virtual int getNumInputs() {
		return 1;
	}
	virtual int getNumOutputs() {
		return 1;
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = 1.0f / std::min<float>(192000.0f, std::max<float>(1.0f, float(fSampleRate)));
	}
	
	virtual void instanceResetUserInterface() {
		fCheckbox0 = FAUSTFLOAT(0.0f);
		fHslider0 = FAUSTFLOAT(2.0f);
		fHslider1 = FAUSTFLOAT(2.0f);
		fHslider2 = FAUSTFLOAT(15.0f);
		fHslider3 = FAUSTFLOAT(40.0f);
		fHslider4 = FAUSTFLOAT(-24.0f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			fRec5[l0] = 0.0f;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fRec4[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec3[l2] = 0.0f;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec2[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fRec1[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fRec0[l5] = 0.0f;
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
	
	virtual compressordsp* clone() {
		return new compressordsp();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->declare(0, "tooltip", "References:                 https://faustlibraries.grame.fr/libs/compressors/                 http://en.wikipedia.org/wiki/Dynamic_range_compression");
		ui_interface->openVerticalBox("COMPRESSOR");
		ui_interface->declare(0, "0", "");
		ui_interface->openHorizontalBox("0x00");
		ui_interface->declare(&fCheckbox0, "0", "");
		ui_interface->declare(&fCheckbox0, "tooltip", "When this is checked, the compressor                 has no effect");
		ui_interface->addCheckButton("Bypass", &fCheckbox0);
		ui_interface->declare(&fHbargraph0, "1", "");
		ui_interface->declare(&fHbargraph0, "tooltip", "Compressor gain in dB");
		ui_interface->declare(&fHbargraph0, "unit", "dB");
		ui_interface->addHorizontalBargraph("Compressor Gain", &fHbargraph0, FAUSTFLOAT(-50.0f), FAUSTFLOAT(10.0f));
		ui_interface->closeBox();
		ui_interface->declare(0, "1", "");
		ui_interface->openHorizontalBox("0x00");
		ui_interface->declare(0, "3", "");
		ui_interface->openHorizontalBox("Compression Control");
		ui_interface->declare(&fHslider1, "0", "");
		ui_interface->declare(&fHslider1, "style", "knob");
		ui_interface->declare(&fHslider1, "tooltip", "A compression Ratio of N means that for each N dB increase in input         signal level above Threshold, the output level goes up 1 dB");
		ui_interface->addHorizontalSlider("Ratio", &fHslider1, FAUSTFLOAT(2.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(20.0f), FAUSTFLOAT(0.100000001f));
		ui_interface->declare(&fHslider4, "1", "");
		ui_interface->declare(&fHslider4, "style", "knob");
		ui_interface->declare(&fHslider4, "tooltip", "When the signal level exceeds the Threshold (in dB), its level         is compressed according to the Ratio");
		ui_interface->declare(&fHslider4, "unit", "dB");
		ui_interface->addHorizontalSlider("Threshold", &fHslider4, FAUSTFLOAT(-24.0f), FAUSTFLOAT(-100.0f), FAUSTFLOAT(10.0f), FAUSTFLOAT(0.100000001f));
		ui_interface->closeBox();
		ui_interface->declare(0, "4", "");
		ui_interface->openHorizontalBox("Compression Response");
		ui_interface->declare(&fHslider2, "1", "");
		ui_interface->declare(&fHslider2, "scale", "log");
		ui_interface->declare(&fHslider2, "style", "knob");
		ui_interface->declare(&fHslider2, "tooltip", "Time constant in ms (1/e smoothing time) for the compression gain         to approach (exponentially) a new lower target level (the compression         `kicking in')");
		ui_interface->declare(&fHslider2, "unit", "ms");
		ui_interface->addHorizontalSlider("Attack", &fHslider2, FAUSTFLOAT(15.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(1000.0f), FAUSTFLOAT(0.100000001f));
		ui_interface->declare(&fHslider3, "2", "");
		ui_interface->declare(&fHslider3, "scale", "log");
		ui_interface->declare(&fHslider3, "style", "knob");
		ui_interface->declare(&fHslider3, "tooltip", "Time constant in ms (1/e smoothing time) for the compression gain         to approach (exponentially) a new higher target level (the compression         'releasing')");
		ui_interface->declare(&fHslider3, "unit", "ms");
		ui_interface->addHorizontalSlider("Release", &fHslider3, FAUSTFLOAT(40.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(1000.0f), FAUSTFLOAT(0.100000001f));
		ui_interface->closeBox();
		ui_interface->closeBox();
		ui_interface->declare(&fHslider0, "5", "");
		ui_interface->declare(&fHslider0, "tooltip", "The compressed-signal output level is increased by this amount         (in dB) to make up for the level lost due to compression");
		ui_interface->declare(&fHslider0, "unit", "dB");
		ui_interface->addHorizontalSlider("MakeUpGain", &fHslider0, FAUSTFLOAT(2.0f), FAUSTFLOAT(-96.0f), FAUSTFLOAT(96.0f), FAUSTFLOAT(0.100000001f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* output0 = outputs[0];
		int iSlow0 = int(float(fCheckbox0));
		float fSlow1 = std::pow(10.0f, 0.0500000007f * float(fHslider0));
		float fSlow2 = 1.0f / std::max<float>(1.1920929e-07f, float(fHslider1)) + -1.0f;
		float fSlow3 = std::max<float>(fConst0, 0.00100000005f * float(fHslider2));
		int iSlow4 = std::fabs(fSlow3) < 1.1920929e-07f;
		float fThen2 = std::exp(0.0f - fConst0 / ((iSlow4) ? 1.0f : fSlow3));
		float fSlow5 = ((iSlow4) ? 0.0f : fThen2);
		float fSlow6 = std::max<float>(fConst0, 0.00100000005f * float(fHslider3));
		int iSlow7 = std::fabs(fSlow6) < 1.1920929e-07f;
		float fThen4 = std::exp(0.0f - fConst0 / ((iSlow7) ? 1.0f : fSlow6));
		float fSlow8 = ((iSlow7) ? 0.0f : fThen4);
		float fSlow9 = float(fHslider4);
		float fSlow10 = 0.5f * fSlow3;
		int iSlow11 = std::fabs(fSlow10) < 1.1920929e-07f;
		float fThen7 = std::exp(0.0f - fConst0 / ((iSlow11) ? 1.0f : fSlow10));
		float fSlow12 = ((iSlow11) ? 0.0f : fThen7);
		float fSlow13 = 1.0f - fSlow12;
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			float fTemp0 = float(input0[i0]);
			float fTemp1 = ((iSlow0) ? 0.0f : fTemp0);
			float fTemp2 = std::fabs(fTemp1);
			float fTemp3 = ((fRec4[1] > fTemp2) ? fSlow8 : fSlow5);
			fRec5[0] = fTemp2 * (1.0f - fTemp3) + fTemp3 * fRec5[1];
			fRec4[0] = fRec5[0];
			fRec3[0] = fSlow2 * std::max<float>(20.0f * std::log10(std::max<float>(1.17549435e-38f, fRec4[0])) - fSlow9, 0.0f) * fSlow13 + fSlow12 * fRec3[1];
			float fTemp4 = fTemp1 * std::pow(10.0f, 0.0500000007f * fRec3[0]);
			float fTemp5 = std::fabs(fTemp4);
			float fTemp6 = ((fRec1[1] > fTemp5) ? fSlow8 : fSlow5);
			fRec2[0] = fTemp5 * (1.0f - fTemp6) + fTemp6 * fRec2[1];
			fRec1[0] = fRec2[0];
			fRec0[0] = fSlow2 * std::max<float>(20.0f * std::log10(std::max<float>(1.17549435e-38f, fRec1[0])) - fSlow9, 0.0f) * fSlow13 + fSlow12 * fRec0[1];
			fHbargraph0 = FAUSTFLOAT(20.0f * std::log10(std::max<float>(1.17549435e-38f, std::pow(10.0f, 0.0500000007f * fRec0[0]))));
			float fThen9 = fSlow1 * fTemp4;
			output0[i0] = FAUSTFLOAT(((iSlow0) ? fTemp0 : fThen9));
			fRec5[1] = fRec5[0];
			fRec4[1] = fRec4[0];
			fRec3[1] = fRec3[0];
			fRec2[1] = fRec2[0];
			fRec1[1] = fRec1[0];
			fRec0[1] = fRec0[0];
		}
	}

};

// clang-format on

#endif
