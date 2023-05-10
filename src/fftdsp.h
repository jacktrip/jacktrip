/* ------------------------------------------------------------
author: "Dominick Hing"
license: "MIT Style STK-4.2"
name: "fft"
version: "1.0"
Code generated with Faust 2.54.9 (https://faust.grame.fr)
Compilation options: -a faust2header.cpp -lang cpp -i -inpl -cn fftdsp -es 1 -mcd 16 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __fftdsp_H__
#define  __fftdsp_H__

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

/************************************************************************
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
 ***************************************************************************/

#ifndef __export__
#define __export__

#define FAUSTVERSION "2.54.9"

// Use FAUST_API for code that is part of the external API but is also compiled in faust and libfaust
// Use LIBFAUST_API for code that is compiled in faust and libfaust

#ifdef _WIN32
    #pragma warning (disable: 4251)
    #ifdef FAUST_EXE
        #define FAUST_API
        #define LIBFAUST_API
    #elif FAUST_LIB
        #define FAUST_API __declspec(dllexport)
        #define LIBFAUST_API __declspec(dllexport)
    #else
        #define FAUST_API
        #define LIBFAUST_API 
    #endif
#else
    #ifdef FAUST_EXE
        #define FAUST_API
        #define LIBFAUST_API
    #else
        #define FAUST_API __attribute__((visibility("default")))
        #define LIBFAUST_API __attribute__((visibility("default")))
    #endif
#endif

#endif

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

struct FAUST_API UI;
struct FAUST_API Meta;

/**
 * DSP memory manager.
 */

struct FAUST_API dsp_memory_manager {
    
    virtual ~dsp_memory_manager() {}
    
    /**
     * Inform the Memory Manager with the number of expected memory zones.
     * @param count - the number of expected memory zones
     */
    virtual void begin(size_t /*count*/) {}
    
    /**
     * Give the Memory Manager information on a given memory zone.
     * @param size - the size in bytes of the memory zone
     * @param reads - the number of Read access to the zone used to compute one frame
     * @param writes - the number of Write access to the zone used to compute one frame
     */
    virtual void info(size_t /*size*/, size_t /*reads*/, size_t /*writes*/) {}

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

class FAUST_API dsp {

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

class FAUST_API decorator_dsp : public dsp {

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

class FAUST_API dsp_factory {
    
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
        virtual std::vector<std::string> getWarningMessages() = 0;
    
        virtual dsp* createDSPInstance() = 0;
    
        virtual void setMemoryManager(dsp_memory_manager* manager) = 0;
        virtual dsp_memory_manager* getMemoryManager() = 0;
    
};

// Denormal handling

#if defined (__SSE__)
#include <xmmintrin.h>
#endif

class FAUST_API ScopedNoDenormals {
    
    private:
    
        intptr_t fpsr = 0;
        
        void setFpStatusRegister(intptr_t fpsr_aux) noexcept
        {
        #if defined (__arm64__) || defined (__aarch64__)
            asm volatile("msr fpcr, %0" : : "ri" (fpsr_aux));
        #elif defined (__SSE__)
            // The volatile keyword here is needed to workaround a bug in AppleClang 13.0
            // which aggressively optimises away the variable otherwise
            volatile uint32_t fpsr_w = static_cast<uint32_t>(fpsr_aux);
            _mm_setcsr(fpsr_w);
        #endif
        }
        
        void getFpStatusRegister() noexcept
        {
        #if defined (__arm64__) || defined (__aarch64__)
            asm volatile("mrs %0, fpcr" : "=r" (fpsr));
        #elif defined (__SSE__)
            fpsr = static_cast<intptr_t>(_mm_getcsr());
        #endif
        }
    
    public:
    
        ScopedNoDenormals() noexcept
        {
        #if defined (__arm64__) || defined (__aarch64__)
            intptr_t mask = (1 << 24 /* FZ */);
        #elif defined (__SSE__)
        #if defined (__SSE2__)
            intptr_t mask = 0x8040;
        #else
            intptr_t mask = 0x8000;
        #endif
        #else
            intptr_t mask = 0x0000;
        #endif
            getFpStatusRegister();
            setFpStatusRegister(fpsr | mask);
        }
        
        ~ScopedNoDenormals() noexcept
        {
            setFpStatusRegister(fpsr);
        }

};

#define AVOIDDENORMALS ScopedNoDenormals ftz_scope;

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
struct FAUST_API Meta {
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
struct FAUST_API UIReal {
    
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
    
    virtual void declare(REAL* /*zone*/, const char* /*key*/, const char* /*val*/) {}

    // To be used by LLVM client
    virtual int sizeOfFAUSTFLOAT() { return sizeof(FAUSTFLOAT); }
};

struct FAUST_API UI : public UIReal<FAUSTFLOAT> {
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

#ifndef __PathBuilder__
#define __PathBuilder__

#include <vector>
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <regex>


/*******************************************************************************
 * PathBuilder : Faust User Interface
 * Helper class to build complete hierarchical path for UI items.
 ******************************************************************************/

class FAUST_API PathBuilder {

    protected:
    
        std::vector<std::string> fControlsLevel;
        std::vector<std::string> fFullPaths;
        std::map<std::string, std::string> fFull2Short;  // filled by computeShortNames()
    
        /**
         * @brief check if a character is acceptable for an ID
         *
         * @param c
         * @return true is the character is acceptable for an ID
         */
        bool isIDChar(char c) const
        {
            return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9'));
        }
    
        /**
         * @brief remove all "/0x00" parts
         *
         * @param src
         * @return modified string
         */
        std::string remove0x00(const std::string& src) const
        {
            return std::regex_replace(src, std::regex("/0x00"), "");
        }
    
        /**
         * @brief replace all non ID char with '_' (one '_' may replace several non ID char)
         *
         * @param src
         * @return modified string
         */
        std::string str2ID(const std::string& src) const
        {
            std::string dst;
            bool need_underscore = false;
            for (char c : src) {
                if (isIDChar(c) || (c == '/')) {
                    if (need_underscore) {
                        dst.push_back('_');
                        need_underscore = false;
                    }
                    dst.push_back(c);
                } else {
                    need_underscore = true;
                }
            }
            return dst;
        }
    
        /**
         * @brief Keep only the last n slash-parts
         *
         * @param src
         * @param n : 1 indicates the last slash-part
         * @return modified string
         */
        std::string cut(const std::string& src, int n) const
        {
            std::string rdst;
            for (int i = int(src.length())-1; i >= 0; i--) {
                char c = src[i];
                if (c != '/') {
                    rdst.push_back(c);
                } else if (n == 1) {
                    std::string dst;
                    for (int j = int(rdst.length())-1; j >= 0; j--) {
                        dst.push_back(rdst[j]);
                    }
                    return dst;
                } else {
                    n--;
                    rdst.push_back(c);
                }
            }
            return src;
        }
    
        void addFullPath(const std::string& label) { fFullPaths.push_back(buildPath(label)); }
    
        /**
         * @brief Compute the mapping between full path and short names
         */
        void computeShortNames()
        {
            std::vector<std::string>           uniquePaths;  // all full paths transformed but made unique with a prefix
            std::map<std::string, std::string> unique2full;  // all full paths transformed but made unique with a prefix
            char num_buffer[16];
            int pnum = 0;
        
            for (const auto& s : fFullPaths) {
                sprintf(num_buffer, "%d", pnum++);
                std::string u = "/P" + std::string(num_buffer) + str2ID(remove0x00(s));
                uniquePaths.push_back(u);
                unique2full[u] = s;  // remember the full path associated to a unique path
            }
        
            std::map<std::string, int> uniquePath2level;                // map path to level
            for (const auto& s : uniquePaths) uniquePath2level[s] = 1;   // we init all levels to 1
            bool have_collisions = true;
        
            while (have_collisions) {
                // compute collision list
                std::set<std::string>              collisionSet;
                std::map<std::string, std::string> short2full;
                have_collisions = false;
                for (const auto& it : uniquePath2level) {
                    std::string u = it.first;
                    int n = it.second;
                    std::string shortName = cut(u, n);
                    auto p = short2full.find(shortName);
                    if (p == short2full.end()) {
                        // no collision
                        short2full[shortName] = u;
                    } else {
                        // we have a collision, add the two paths to the collision set
                        have_collisions = true;
                        collisionSet.insert(u);
                        collisionSet.insert(p->second);
                    }
                }
                for (const auto& s : collisionSet) uniquePath2level[s]++;  // increase level of colliding path
            }
        
            for (const auto& it : uniquePath2level) {
                std::string u = it.first;
                int n = it.second;
                std::string shortName = replaceCharList(cut(u, n), {'/'}, '_');
                fFull2Short[unique2full[u]] = shortName;
            }
        }
    
        std::string replaceCharList(const std::string& str, const std::vector<char>& ch1, char ch2)
        {
            auto beg = ch1.begin();
            auto end = ch1.end();
            std::string res = str;
            for (size_t i = 0; i < str.length(); ++i) {
                if (std::find(beg, end, str[i]) != end) res[i] = ch2;
            }
            return res;
        }
     
    public:
    
        PathBuilder() {}
        virtual ~PathBuilder() {}
    
        // Return true for the first level of groups
        bool pushLabel(const std::string& label) { fControlsLevel.push_back(label); return fControlsLevel.size() == 1;}
    
        // Return true for the last level of groups
        bool popLabel() { fControlsLevel.pop_back(); return fControlsLevel.size() == 0; }
    
        std::string buildPath(const std::string& label)
        {
            std::string res = "/";
            for (size_t i = 0; i < fControlsLevel.size(); i++) {
                res = res + fControlsLevel[i] + "/";
            }
            res += label;
            return replaceCharList(res, {' ', '#', '*', ',', '?', '[', ']', '{', '}', '(', ')'}, '_');
        }
    
};

#endif  // __PathBuilder__
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
class FAUST_API Interpolator {
    
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
class FAUST_API Interpolator3pt {

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
class FAUST_API ValueConverter {

    public:

        virtual ~ValueConverter() {}
        virtual double ui2faust(double x) { return x; };
        virtual double faust2ui(double x) { return x; };
};

//--------------------------------------------------------------------------------------
// A converter than can be updated
//--------------------------------------------------------------------------------------

class FAUST_API UpdatableValueConverter : public ValueConverter {
    
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
class FAUST_API LinearValueConverter : public ValueConverter {
    
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
class FAUST_API LinearValueConverter2 : public UpdatableValueConverter {
    
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
class FAUST_API LogValueConverter : public LinearValueConverter {

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
class FAUST_API ExpValueConverter : public LinearValueConverter {

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
class FAUST_API AccUpConverter : public UpdatableValueConverter {

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
class FAUST_API AccDownConverter : public UpdatableValueConverter {

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
class FAUST_API AccUpDownConverter : public UpdatableValueConverter {

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
class FAUST_API AccDownUpConverter : public UpdatableValueConverter {

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
class FAUST_API ZoneControl {

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
class FAUST_API ConverterZoneControl : public ZoneControl {

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
class FAUST_API CurveZoneControl : public ZoneControl {

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

class FAUST_API ZoneReader {

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
            std::string fLabel;
            std::string fShortname;
            std::string fPath;
            ValueConverter* fConversion;
            FAUSTFLOAT* fZone;
            FAUSTFLOAT fInit;
            FAUSTFLOAT fMin;
            FAUSTFLOAT fMax;
            FAUSTFLOAT fStep;
            ItemType fItemType;
            
            Item(const std::string& label,
                 const std::string& short_name,
                 const std::string& path,
                 ValueConverter* conversion,
                 FAUSTFLOAT* zone,
                 FAUSTFLOAT init,
                 FAUSTFLOAT min,
                 FAUSTFLOAT max,
                 FAUSTFLOAT step,
                 ItemType item_type)
            :fLabel(label), fShortname(short_name), fPath(path), fConversion(conversion),
            fZone(zone), fInit(init), fMin(min), fMax(max), fStep(step), fItemType(item_type)
            {}
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
            fFullPaths.push_back(path);

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

            fItems.push_back(Item(label, "", path, converter, zone, init, min, max, step, type));
       
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
        virtual void closeBox()
        {
            if (popLabel()) {
                // Shortnames can be computed when all fullnames are known
                computeShortNames();
                // Fill 'shortname' field for each item
                for (const auto& it : fFull2Short) {
                    int index = getParamIndex(it.first.c_str());
                    fItems[index].fShortname = it.second;
                }
            }
        }

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
    
        /**
         * Return the number of parameters in the UI.
         *
         * @return the number of parameters
         */
        int getParamsCount() { return int(fItems.size()); }

        /**
         * Return the param index.
         *
         * @param str - the UI parameter label/shortname/path
         *
         * @return the param index
         */
        int getParamIndex(const char* str)
        {
            std::string path = std::string(str);
            auto it = find_if(fItems.begin(), fItems.end(),
                              [=](const Item& it) { return (it.fLabel == path) || (it.fShortname == path) || (it.fPath == path); });
            return (it != fItems.end()) ? int(it - fItems.begin()) : -1;
        }
    
        /**
         * Return the param label.
         *
         * @param p - the UI parameter index
         *
         * @return the param label
         */
        const char* getParamLabel(int p) { return fItems[uint(p)].fLabel.c_str(); }
    
        /**
         * Return the param shortname.
         *
         * @param p - the UI parameter index
         *
         * @return the param shortname
         */
        const char* getParamShortname(int p) { return fItems[uint(p)].fShortname.c_str(); }
    
        /**
         * Return the param path.
         *
         * @param p - the UI parameter index
         *
         * @return the param path
         */
        const char* getParamAddress(int p) { return fItems[uint(p)].fPath.c_str(); }
    
        /**
         * Return the param metadata.
         *
         * @param p - the UI parameter index
         *
         * @return the param metadata as a map<key, value>
         */
        std::map<const char*, const char*> getMetadata(int p)
        {
            std::map<const char*, const char*> res;
            std::map<std::string, std::string> metadata = fMetaData[uint(p)];
            for (const auto& it : metadata) {
                res[it.first.c_str()] = it.second.c_str();
            }
            return res;
        }

        /**
         * Return the param metadata value.
         *
         * @param p - the UI parameter index
         * @param key - the UI parameter index
         *
         * @return the param metadata value associate to the key
         */
        const char* getMetadata(int p, const char* key)
        {
            return (fMetaData[uint(p)].find(key) != fMetaData[uint(p)].end()) ? fMetaData[uint(p)][key].c_str() : "";
        }
    
        /**
         * Return the param minimum value.
         *
         * @param p - the UI parameter index
         *
         * @return the param minimum value
         */
        FAUSTFLOAT getParamMin(int p) { return fItems[uint(p)].fMin; }
    
        /**
         * Return the param maximum value.
         *
         * @param p - the UI parameter index
         *
         * @return the param maximum value
         */
        FAUSTFLOAT getParamMax(int p) { return fItems[uint(p)].fMax; }
    
        /**
         * Return the param step value.
         *
         * @param p - the UI parameter index
         *
         * @return the param step value
         */
        FAUSTFLOAT getParamStep(int p) { return fItems[uint(p)].fStep; }
    
        /**
         * Return the param init value.
         *
         * @param p - the UI parameter index
         *
         * @return the param init value
         */
        FAUSTFLOAT getParamInit(int p) { return fItems[uint(p)].fInit; }

        /**
         * Return the param memory zone.
         *
         * @param p - the UI parameter index
         *
         * @return the param memory zone.
         */
        FAUSTFLOAT* getParamZone(int p) { return fItems[uint(p)].fZone; }

        /**
         * Return the param value.
         *
         * @param p - the UI parameter index
         *
         * @return the param value.
         */
        FAUSTFLOAT getParamValue(int p) { return *fItems[uint(p)].fZone; }
    
        /**
         * Return the param value.
         *
         * @param str - the UI parameter label/shortname/path
         *
         * @return the param value.
         */
        FAUSTFLOAT getParamValue(const char* str)
        {
            int index = getParamIndex(str);
            if (index >= 0) {
                return getParamValue(index);
            } else {
                fprintf(stderr, "getParamValue : '%s' not found\n", (str == nullptr ? "NULL" : str));
                return FAUSTFLOAT(0);
            }
        }

        /**
         * Set the param value.
         *
         * @param p - the UI parameter index
         * @param v - the UI parameter value
         *
         */
        void setParamValue(int p, FAUSTFLOAT v) { *fItems[uint(p)].fZone = v; }
        
        /**
         * Set the param value.
         *
         * @param p - the UI parameter label/shortname/path
         * @param v - the UI parameter value
         *
         */
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
         * @param curve - between 0 and 3 (0: up, 1: down, 2: up and down, 2: down and up)
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
         * @param gyr - 0 for X gyroscope, 1 for Y gyroscope, 2 for Z gyroscope (-1 means "no mapping")
         * @param curve - between 0 and 3 (0: up, 1: down, 2: up and down, 2: down and up)
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
         * @param curve - the curve value to be retrieved (between 0 and 3)
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
         * @param curve - the curve value to be retrieved (between 0 and 3)
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

        /**
         * Get the requested screen color.
         *
         * -1 means no screen color control (no screencolor metadata found)
         * otherwise return 0x00RRGGBB a ready to use color
         *
         */
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

#ifndef FAUSTCLASS 
#define FAUSTCLASS fftdsp
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

static float fftdsp_faustpower2_f(float value) {
	return value * value;
}

class fftdsp : public dsp {
	
 private:
	
	int IOTA0;
	float fVec0[128];
	int fSampleRate;
	
 public:
	
	void metadata(Meta* m) { 
		m->declare("analyzers.lib/name", "Faust Analyzer Library");
		m->declare("analyzers.lib/version", "0.2");
		m->declare("author", "Dominick Hing");
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "0.9");
		m->declare("compile_options", "-a faust2header.cpp -lang cpp -i -inpl -cn fftdsp -es 1 -mcd 16 -single -ftz 0");
		m->declare("description", "FFT Faust Plugin for JackTrip, using FFT window of 512 and including postive and negative frequencies");
		m->declare("filename", "fftdsp.dsp");
		m->declare("license", "MIT Style STK-4.2");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.5");
		m->declare("name", "fft");
		m->declare("signals.lib/name", "Faust Signal Routing Library");
		m->declare("signals.lib/version", "0.3");
		m->declare("version", "1.0");
	}

	virtual int getNumInputs() {
		return 1;
	}
	virtual int getNumOutputs() {
		return 65;
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
	}
	
	virtual void instanceResetUserInterface() {
	}
	
	virtual void instanceClear() {
		IOTA0 = 0;
		for (int l0 = 0; l0 < 128; l0 = l0 + 1) {
			fVec0[l0] = 0.0f;
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
	
	virtual fftdsp* clone() {
		return new fftdsp();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("fft");
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		FAUSTFLOAT* output2 = outputs[2];
		FAUSTFLOAT* output3 = outputs[3];
		FAUSTFLOAT* output4 = outputs[4];
		FAUSTFLOAT* output5 = outputs[5];
		FAUSTFLOAT* output6 = outputs[6];
		FAUSTFLOAT* output7 = outputs[7];
		FAUSTFLOAT* output8 = outputs[8];
		FAUSTFLOAT* output9 = outputs[9];
		FAUSTFLOAT* output10 = outputs[10];
		FAUSTFLOAT* output11 = outputs[11];
		FAUSTFLOAT* output12 = outputs[12];
		FAUSTFLOAT* output13 = outputs[13];
		FAUSTFLOAT* output14 = outputs[14];
		FAUSTFLOAT* output15 = outputs[15];
		FAUSTFLOAT* output16 = outputs[16];
		FAUSTFLOAT* output17 = outputs[17];
		FAUSTFLOAT* output18 = outputs[18];
		FAUSTFLOAT* output19 = outputs[19];
		FAUSTFLOAT* output20 = outputs[20];
		FAUSTFLOAT* output21 = outputs[21];
		FAUSTFLOAT* output22 = outputs[22];
		FAUSTFLOAT* output23 = outputs[23];
		FAUSTFLOAT* output24 = outputs[24];
		FAUSTFLOAT* output25 = outputs[25];
		FAUSTFLOAT* output26 = outputs[26];
		FAUSTFLOAT* output27 = outputs[27];
		FAUSTFLOAT* output28 = outputs[28];
		FAUSTFLOAT* output29 = outputs[29];
		FAUSTFLOAT* output30 = outputs[30];
		FAUSTFLOAT* output31 = outputs[31];
		FAUSTFLOAT* output32 = outputs[32];
		FAUSTFLOAT* output33 = outputs[33];
		FAUSTFLOAT* output34 = outputs[34];
		FAUSTFLOAT* output35 = outputs[35];
		FAUSTFLOAT* output36 = outputs[36];
		FAUSTFLOAT* output37 = outputs[37];
		FAUSTFLOAT* output38 = outputs[38];
		FAUSTFLOAT* output39 = outputs[39];
		FAUSTFLOAT* output40 = outputs[40];
		FAUSTFLOAT* output41 = outputs[41];
		FAUSTFLOAT* output42 = outputs[42];
		FAUSTFLOAT* output43 = outputs[43];
		FAUSTFLOAT* output44 = outputs[44];
		FAUSTFLOAT* output45 = outputs[45];
		FAUSTFLOAT* output46 = outputs[46];
		FAUSTFLOAT* output47 = outputs[47];
		FAUSTFLOAT* output48 = outputs[48];
		FAUSTFLOAT* output49 = outputs[49];
		FAUSTFLOAT* output50 = outputs[50];
		FAUSTFLOAT* output51 = outputs[51];
		FAUSTFLOAT* output52 = outputs[52];
		FAUSTFLOAT* output53 = outputs[53];
		FAUSTFLOAT* output54 = outputs[54];
		FAUSTFLOAT* output55 = outputs[55];
		FAUSTFLOAT* output56 = outputs[56];
		FAUSTFLOAT* output57 = outputs[57];
		FAUSTFLOAT* output58 = outputs[58];
		FAUSTFLOAT* output59 = outputs[59];
		FAUSTFLOAT* output60 = outputs[60];
		FAUSTFLOAT* output61 = outputs[61];
		FAUSTFLOAT* output62 = outputs[62];
		FAUSTFLOAT* output63 = outputs[63];
		FAUSTFLOAT* output64 = outputs[64];
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			float fTemp0 = float(input0[i0]);
			fVec0[IOTA0 & 127] = fTemp0;
			float fTemp1 = fVec0[(IOTA0 - 61) & 127];
			float fTemp2 = fVec0[(IOTA0 - 60) & 127];
			float fTemp3 = fVec0[(IOTA0 - 59) & 127];
			float fTemp4 = fVec0[(IOTA0 - 58) & 127];
			float fTemp5 = fVec0[(IOTA0 - 57) & 127];
			float fTemp6 = fVec0[(IOTA0 - 56) & 127];
			float fTemp7 = fVec0[(IOTA0 - 55) & 127];
			float fTemp8 = fVec0[(IOTA0 - 54) & 127];
			float fTemp9 = fVec0[(IOTA0 - 53) & 127];
			float fTemp10 = fVec0[(IOTA0 - 52) & 127];
			float fTemp11 = fVec0[(IOTA0 - 51) & 127];
			float fTemp12 = fVec0[(IOTA0 - 50) & 127];
			float fTemp13 = fVec0[(IOTA0 - 49) & 127];
			float fTemp14 = fVec0[(IOTA0 - 48) & 127];
			float fTemp15 = fVec0[(IOTA0 - 47) & 127];
			float fTemp16 = fVec0[(IOTA0 - 46) & 127];
			float fTemp17 = fVec0[(IOTA0 - 45) & 127];
			float fTemp18 = fVec0[(IOTA0 - 44) & 127];
			float fTemp19 = fVec0[(IOTA0 - 43) & 127];
			float fTemp20 = fVec0[(IOTA0 - 42) & 127];
			float fTemp21 = fVec0[(IOTA0 - 41) & 127];
			float fTemp22 = fVec0[(IOTA0 - 40) & 127];
			float fTemp23 = fVec0[(IOTA0 - 39) & 127];
			float fTemp24 = fVec0[(IOTA0 - 84) & 127];
			float fTemp25 = fVec0[(IOTA0 - 83) & 127];
			float fTemp26 = fVec0[(IOTA0 - 82) & 127];
			float fTemp27 = fVec0[(IOTA0 - 81) & 127];
			float fTemp28 = fVec0[(IOTA0 - 80) & 127];
			float fTemp29 = fVec0[(IOTA0 - 79) & 127];
			float fTemp30 = fVec0[(IOTA0 - 78) & 127];
			float fTemp31 = fVec0[(IOTA0 - 77) & 127];
			float fTemp32 = fVec0[(IOTA0 - 76) & 127];
			float fTemp33 = fVec0[(IOTA0 - 75) & 127];
			float fTemp34 = fVec0[(IOTA0 - 74) & 127];
			float fTemp35 = fVec0[(IOTA0 - 73) & 127];
			float fTemp36 = fVec0[(IOTA0 - 72) & 127];
			float fTemp37 = fVec0[(IOTA0 - 71) & 127];
			float fTemp38 = fVec0[(IOTA0 - 70) & 127];
			float fTemp39 = fVec0[(IOTA0 - 69) & 127];
			float fTemp40 = fVec0[(IOTA0 - 68) & 127];
			float fTemp41 = fVec0[(IOTA0 - 67) & 127];
			float fTemp42 = fVec0[(IOTA0 - 66) & 127];
			float fTemp43 = fVec0[(IOTA0 - 65) & 127];
			float fTemp44 = fVec0[(IOTA0 - 64) & 127];
			float fTemp45 = fVec0[(IOTA0 - 63) & 127];
			float fTemp46 = fVec0[(IOTA0 - 62) & 127];
			float fTemp47 = fVec0[(IOTA0 - 16) & 127];
			float fTemp48 = fVec0[(IOTA0 - 15) & 127];
			float fTemp49 = fVec0[(IOTA0 - 14) & 127];
			float fTemp50 = fVec0[(IOTA0 - 13) & 127];
			float fTemp51 = fVec0[(IOTA0 - 12) & 127];
			float fTemp52 = fVec0[(IOTA0 - 11) & 127];
			float fTemp53 = fVec0[(IOTA0 - 10) & 127];
			float fTemp54 = fVec0[(IOTA0 - 9) & 127];
			float fTemp55 = fVec0[(IOTA0 - 8) & 127];
			float fTemp56 = fVec0[(IOTA0 - 7) & 127];
			float fTemp57 = fVec0[(IOTA0 - 6) & 127];
			float fTemp58 = fVec0[(IOTA0 - 5) & 127];
			float fTemp59 = fVec0[(IOTA0 - 4) & 127];
			float fTemp60 = fVec0[(IOTA0 - 3) & 127];
			float fTemp61 = fVec0[(IOTA0 - 2) & 127];
			float fTemp62 = fVec0[(IOTA0 - 1) & 127];
			float fTemp63 = fVec0[(IOTA0 - 38) & 127];
			float fTemp64 = fVec0[(IOTA0 - 37) & 127];
			float fTemp65 = fVec0[(IOTA0 - 36) & 127];
			float fTemp66 = fVec0[(IOTA0 - 35) & 127];
			float fTemp67 = fVec0[(IOTA0 - 34) & 127];
			float fTemp68 = fVec0[(IOTA0 - 33) & 127];
			float fTemp69 = fVec0[(IOTA0 - 32) & 127];
			float fTemp70 = fVec0[(IOTA0 - 31) & 127];
			float fTemp71 = fVec0[(IOTA0 - 30) & 127];
			float fTemp72 = fVec0[(IOTA0 - 29) & 127];
			float fTemp73 = fVec0[(IOTA0 - 28) & 127];
			float fTemp74 = fVec0[(IOTA0 - 27) & 127];
			float fTemp75 = fVec0[(IOTA0 - 26) & 127];
			float fTemp76 = fVec0[(IOTA0 - 25) & 127];
			float fTemp77 = fVec0[(IOTA0 - 24) & 127];
			float fTemp78 = fVec0[(IOTA0 - 23) & 127];
			float fTemp79 = fVec0[(IOTA0 - 22) & 127];
			float fTemp80 = fVec0[(IOTA0 - 21) & 127];
			float fTemp81 = fVec0[(IOTA0 - 20) & 127];
			float fTemp82 = fVec0[(IOTA0 - 19) & 127];
			float fTemp83 = fVec0[(IOTA0 - 18) & 127];
			float fTemp84 = fVec0[(IOTA0 - 17) & 127];
			float fTemp85 = fVec0[(IOTA0 - 106) & 127];
			float fTemp86 = fVec0[(IOTA0 - 105) & 127];
			float fTemp87 = fVec0[(IOTA0 - 104) & 127];
			float fTemp88 = fVec0[(IOTA0 - 103) & 127];
			float fTemp89 = fVec0[(IOTA0 - 102) & 127];
			float fTemp90 = fVec0[(IOTA0 - 101) & 127];
			float fTemp91 = fVec0[(IOTA0 - 100) & 127];
			float fTemp92 = fVec0[(IOTA0 - 99) & 127];
			float fTemp93 = fVec0[(IOTA0 - 98) & 127];
			float fTemp94 = fVec0[(IOTA0 - 97) & 127];
			float fTemp95 = fVec0[(IOTA0 - 96) & 127];
			float fTemp96 = fVec0[(IOTA0 - 95) & 127];
			float fTemp97 = fVec0[(IOTA0 - 94) & 127];
			float fTemp98 = fVec0[(IOTA0 - 93) & 127];
			float fTemp99 = fVec0[(IOTA0 - 92) & 127];
			float fTemp100 = fVec0[(IOTA0 - 91) & 127];
			float fTemp101 = fVec0[(IOTA0 - 90) & 127];
			float fTemp102 = fVec0[(IOTA0 - 89) & 127];
			float fTemp103 = fVec0[(IOTA0 - 88) & 127];
			float fTemp104 = fVec0[(IOTA0 - 87) & 127];
			float fTemp105 = fVec0[(IOTA0 - 86) & 127];
			float fTemp106 = fVec0[(IOTA0 - 85) & 127];
			float fTemp107 = fVec0[(IOTA0 - 127) & 127];
			float fTemp108 = fVec0[(IOTA0 - 126) & 127];
			float fTemp109 = fVec0[(IOTA0 - 125) & 127];
			float fTemp110 = fVec0[(IOTA0 - 124) & 127];
			float fTemp111 = fVec0[(IOTA0 - 123) & 127];
			float fTemp112 = fVec0[(IOTA0 - 122) & 127];
			float fTemp113 = fVec0[(IOTA0 - 121) & 127];
			float fTemp114 = fVec0[(IOTA0 - 120) & 127];
			float fTemp115 = fVec0[(IOTA0 - 119) & 127];
			float fTemp116 = fVec0[(IOTA0 - 118) & 127];
			float fTemp117 = fVec0[(IOTA0 - 117) & 127];
			float fTemp118 = fVec0[(IOTA0 - 116) & 127];
			float fTemp119 = fVec0[(IOTA0 - 115) & 127];
			float fTemp120 = fVec0[(IOTA0 - 114) & 127];
			float fTemp121 = fVec0[(IOTA0 - 113) & 127];
			float fTemp122 = fVec0[(IOTA0 - 112) & 127];
			float fTemp123 = fVec0[(IOTA0 - 111) & 127];
			float fTemp124 = fVec0[(IOTA0 - 110) & 127];
			float fTemp125 = fVec0[(IOTA0 - 109) & 127];
			float fTemp126 = fVec0[(IOTA0 - 107) & 127];
			float fTemp127 = fVec0[(IOTA0 - 108) & 127];
			output0[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp1 + fTemp2 + fTemp3 + fTemp4 + fTemp5 + fTemp6 + fTemp7 + fTemp8 + fTemp9 + fTemp10 + fTemp11 + fTemp12 + fTemp13 + fTemp14 + fTemp15 + fTemp16 + fTemp17 + fTemp18 + fTemp19 + fTemp20 + fTemp21 + fTemp22 + fTemp23 + fTemp24 + fTemp25 + fTemp26 + fTemp27 + fTemp28 + fTemp29 + fTemp30 + fTemp31 + fTemp32 + fTemp33 + fTemp34 + fTemp35 + fTemp36 + fTemp37 + fTemp38 + fTemp39 + fTemp40 + fTemp41 + fTemp42 + fTemp43 + fTemp44 + fTemp45 + fTemp46 + fTemp47 + fTemp48 + fTemp49 + fTemp50 + fTemp51 + fTemp52 + fTemp53 + fTemp54 + fTemp55 + fTemp56 + fTemp57 + fTemp58 + fTemp59 + fTemp60 + fTemp61 + fTemp62 + fTemp0 + fTemp63 + fTemp64 + fTemp65 + fTemp66 + fTemp67 + fTemp68 + fTemp69 + fTemp70 + fTemp71 + fTemp72 + fTemp73 + fTemp74 + fTemp75 + fTemp76 + fTemp77 + fTemp78 + fTemp79 + fTemp80 + fTemp81 + fTemp82 + fTemp83 + fTemp84 + fTemp85 + fTemp86 + fTemp87 + fTemp88 + fTemp89 + fTemp90 + fTemp91 + fTemp92 + fTemp93 + fTemp94 + fTemp95 + fTemp96 + fTemp97 + fTemp98 + fTemp99 + fTemp100 + fTemp101 + fTemp102 + fTemp103 + fTemp104 + fTemp105 + fTemp106 + fTemp107 + fTemp108 + fTemp109 + fTemp110 + fTemp111 + fTemp112 + fTemp113 + fTemp114 + fTemp115 + fTemp116 + fTemp117 + fTemp118 + fTemp119 + fTemp120 + fTemp121 + fTemp122 + fTemp123 + fTemp124 + fTemp125 + fTemp126 + fTemp127)));
			float fTemp128 = fTemp13 - fTemp121;
			float fTemp129 = 0.0f - fTemp128;
			float fTemp130 = 0.70710677f * fTemp129;
			float fTemp131 = fTemp5 - fTemp113;
			float fTemp132 = 0.0f - fTemp131;
			float fTemp133 = fTemp76 - fTemp102;
			float fTemp134 = 0.70710677f * fTemp133;
			float fTemp135 = fTemp86 + 0.70710677f * fTemp132 - (fTemp21 + fTemp134);
			float fTemp136 = 0.9238795f * fTemp135;
			float fTemp137 = fTemp94 + fTemp136;
			float fTemp138 = fTemp9 - fTemp117;
			float fTemp139 = 0.0f - fTemp138;
			float fTemp140 = fTemp90 + 0.70710677f * fTemp139;
			float fTemp141 = fTemp1 - fTemp109;
			float fTemp142 = 0.0f - fTemp141;
			float fTemp143 = fTemp72 - fTemp98;
			float fTemp144 = 0.70710677f * fTemp143;
			float fTemp145 = fTemp125 + 0.70710677f * fTemp142 - (fTemp17 + fTemp144);
			float fTemp146 = fTemp80 - fTemp106;
			float fTemp147 = 0.70710677f * fTemp146;
			float fTemp148 = fTemp64 + fTemp147;
			float fTemp149 = 0.70710677f * fTemp142;
			float fTemp150 = 0.70710677f * fTemp143;
			float fTemp151 = fTemp149 + fTemp50 + fTemp150 - fTemp31;
			float fTemp152 = fTemp140 + 0.9238795f * fTemp145 - (fTemp148 + 0.38268343f * fTemp151);
			float fTemp153 = 0.98078525f * fTemp152;
			float fTemp154 = fTemp130 + fTemp137 + fTemp153;
			float fTemp155 = fTemp11 - fTemp119;
			float fTemp156 = 0.0f - fTemp155;
			float fTemp157 = 0.70710677f * fTemp156;
			float fTemp158 = fTemp92 + fTemp157;
			float fTemp159 = fTemp3 - fTemp111;
			float fTemp160 = 0.0f - fTemp159;
			float fTemp161 = fTemp74 - fTemp100;
			float fTemp162 = 0.70710677f * fTemp161;
			float fTemp163 = fTemp126 + 0.70710677f * fTemp160 - (fTemp19 + fTemp162);
			float fTemp164 = fTemp158 + 0.9238795f * fTemp163;
			float fTemp165 = fTemp7 - fTemp115;
			float fTemp166 = 0.0f - fTemp165;
			float fTemp167 = fTemp88 + 0.70710677f * fTemp166;
			float fTemp168 = fTemp45 - fTemp107;
			float fTemp169 = 0.0f - fTemp168;
			float fTemp170 = fTemp70 - fTemp96;
			float fTemp171 = 0.70710677f * fTemp170;
			float fTemp172 = fTemp123 + 0.70710677f * fTemp169 - (fTemp15 + fTemp171);
			float fTemp173 = fTemp78 - fTemp104;
			float fTemp174 = 0.70710677f * fTemp173;
			float fTemp175 = fTemp23 + fTemp174;
			float fTemp176 = 0.70710677f * fTemp169;
			float fTemp177 = 0.70710677f * fTemp170;
			float fTemp178 = fTemp176 + fTemp48 + fTemp177 - fTemp29;
			float fTemp179 = fTemp167 + 0.9238795f * fTemp172 - (fTemp175 + 0.38268343f * fTemp178);
			float fTemp180 = 0.98078525f * fTemp179;
			float fTemp181 = fTemp82 - fTemp25;
			float fTemp182 = 0.70710677f * fTemp181;
			float fTemp183 = fTemp66 + fTemp182;
			float fTemp184 = 0.70710677f * fTemp160;
			float fTemp185 = 0.70710677f * fTemp161;
			float fTemp186 = fTemp184 + fTemp52 + fTemp185 - fTemp33;
			float fTemp187 = fTemp183 + 0.38268343f * fTemp186;
			float fTemp188 = 0.70710677f * fTemp166;
			float fTemp189 = 0.70710677f * fTemp173;
			float fTemp190 = fTemp188 + fTemp56 + fTemp189;
			float fTemp191 = 0.38268343f * fTemp172 + fTemp190 + 0.9238795f * fTemp178 - fTemp37;
			float fTemp192 = fTemp164 + fTemp180 - (fTemp187 + 0.19509032f * fTemp191);
			float fTemp193 = 0.9951847f * fTemp192;
			float fTemp194 = 0.70710677f * fTemp132;
			float fTemp195 = 0.70710677f * fTemp133;
			float fTemp196 = fTemp194 + fTemp54 + fTemp195 - fTemp35;
			float fTemp197 = 0.38268343f * fTemp196;
			float fTemp198 = fTemp84 - fTemp27;
			float fTemp199 = 0.70710677f * fTemp198;
			float fTemp200 = 0.70710677f * fTemp139;
			float fTemp201 = 0.70710677f * fTemp146;
			float fTemp202 = fTemp200 + fTemp58 + fTemp201;
			float fTemp203 = 0.38268343f * fTemp145 + fTemp202 + 0.9238795f * fTemp151 - fTemp39;
			float fTemp204 = fTemp197 + fTemp199 + fTemp68 + 0.19509032f * fTemp203;
			float fTemp205 = 0.70710677f * fTemp156;
			float fTemp206 = 0.70710677f * fTemp181;
			float fTemp207 = 0.38268343f * fTemp163 + fTemp205 + fTemp60 + fTemp206 + 0.9238795f * fTemp186;
			float fTemp208 = 0.98078525f * fTemp191;
			float fTemp209 = 0.19509032f * fTemp179 + fTemp207 + fTemp208 - fTemp41;
			float fTemp210 = fTemp154 + fTemp193 - (fTemp204 + 0.09801714f * fTemp209);
			float fTemp211 = fTemp12 - fTemp120;
			float fTemp212 = 0.0f - fTemp211;
			float fTemp213 = 0.70710677f * fTemp212;
			float fTemp214 = fTemp93 + fTemp213;
			float fTemp215 = fTemp4 - fTemp112;
			float fTemp216 = 0.0f - fTemp215;
			float fTemp217 = fTemp75 - fTemp101;
			float fTemp218 = 0.70710677f * fTemp217;
			float fTemp219 = fTemp85 + 0.70710677f * fTemp216 - (fTemp20 + fTemp218);
			float fTemp220 = fTemp214 + 0.9238795f * fTemp219;
			float fTemp221 = fTemp8 - fTemp116;
			float fTemp222 = 0.0f - fTemp221;
			float fTemp223 = fTemp89 + 0.70710677f * fTemp222;
			float fTemp224 = fTemp46 - fTemp108;
			float fTemp225 = 0.0f - fTemp224;
			float fTemp226 = fTemp71 - fTemp97;
			float fTemp227 = 0.70710677f * fTemp226;
			float fTemp228 = fTemp124 + 0.70710677f * fTemp225 - (fTemp16 + fTemp227);
			float fTemp229 = fTemp79 - fTemp105;
			float fTemp230 = 0.70710677f * fTemp229;
			float fTemp231 = fTemp63 + fTemp230;
			float fTemp232 = 0.70710677f * fTemp225;
			float fTemp233 = 0.70710677f * fTemp226;
			float fTemp234 = fTemp232 + fTemp49 + fTemp233 - fTemp30;
			float fTemp235 = fTemp223 + 0.9238795f * fTemp228 - (fTemp231 + 0.38268343f * fTemp234);
			float fTemp236 = 0.98078525f * fTemp235;
			float fTemp237 = fTemp83 - fTemp26;
			float fTemp238 = 0.70710677f * fTemp237;
			float fTemp239 = fTemp67 + fTemp238;
			float fTemp240 = 0.70710677f * fTemp216;
			float fTemp241 = 0.70710677f * fTemp217;
			float fTemp242 = fTemp240 + fTemp53 + fTemp241 - fTemp34;
			float fTemp243 = fTemp239 + 0.38268343f * fTemp242;
			float fTemp244 = 0.70710677f * fTemp222;
			float fTemp245 = 0.70710677f * fTemp229;
			float fTemp246 = fTemp244 + fTemp57 + fTemp245;
			float fTemp247 = 0.38268343f * fTemp228 + fTemp246 + 0.9238795f * fTemp234 - fTemp38;
			float fTemp248 = fTemp220 + fTemp236 - (fTemp243 + 0.19509032f * fTemp247);
			float fTemp249 = fTemp6 - fTemp114;
			float fTemp250 = 0.0f - fTemp249;
			float fTemp251 = fTemp77 - fTemp103;
			float fTemp252 = 0.70710677f * fTemp251;
			float fTemp253 = fTemp87 + 0.70710677f * fTemp250 - (fTemp22 + fTemp252);
			float fTemp254 = 0.38268343f * fTemp253;
			float fTemp255 = 0.70710677f * fTemp250;
			float fTemp256 = 0.70710677f * fTemp251;
			float fTemp257 = fTemp255 + fTemp55 + fTemp256 - fTemp36;
			float fTemp258 = 0.9238795f * fTemp257;
			float fTemp259 = fTemp14 - fTemp122;
			float fTemp260 = 0.0f - fTemp259;
			float fTemp261 = 0.70710677f * fTemp260;
			float fTemp262 = fTemp47 - fTemp28;
			float fTemp263 = 0.70710677f * fTemp262;
			float fTemp264 = fTemp10 - fTemp118;
			float fTemp265 = 0.0f - fTemp264;
			float fTemp266 = fTemp91 + 0.70710677f * fTemp265;
			float fTemp267 = fTemp2 - fTemp110;
			float fTemp268 = 0.0f - fTemp267;
			float fTemp269 = fTemp73 - fTemp99;
			float fTemp270 = 0.70710677f * fTemp269;
			float fTemp271 = fTemp127 + 0.70710677f * fTemp268 - (fTemp18 + fTemp270);
			float fTemp272 = fTemp81 - fTemp24;
			float fTemp273 = 0.70710677f * fTemp272;
			float fTemp274 = fTemp65 + fTemp273;
			float fTemp275 = 0.70710677f * fTemp268;
			float fTemp276 = 0.70710677f * fTemp269;
			float fTemp277 = fTemp275 + fTemp51 + fTemp276 - fTemp32;
			float fTemp278 = fTemp266 + 0.9238795f * fTemp271 - (fTemp274 + 0.38268343f * fTemp277);
			float fTemp279 = 0.19509032f * fTemp278;
			float fTemp280 = 0.70710677f * fTemp265;
			float fTemp281 = 0.70710677f * fTemp272;
			float fTemp282 = fTemp280 + fTemp59 + fTemp281;
			float fTemp283 = 0.38268343f * fTemp271 + fTemp282 + 0.9238795f * fTemp277 - fTemp40;
			float fTemp284 = 0.98078525f * fTemp283;
			float fTemp285 = 0.70710677f * fTemp212;
			float fTemp286 = 0.70710677f * fTemp237;
			float fTemp287 = 0.38268343f * fTemp219 + fTemp285 + fTemp61 + fTemp286 + 0.9238795f * fTemp242;
			float fTemp288 = 0.98078525f * fTemp247;
			float fTemp289 = 0.19509032f * fTemp235 + fTemp287 + fTemp288 - fTemp42;
			float fTemp290 = 0.9951847f * fTemp289;
			float fTemp291 = 0.9238795f * fTemp196;
			float fTemp292 = 0.70710677f * fTemp129;
			float fTemp293 = 0.70710677f * fTemp198;
			float fTemp294 = 0.38268343f * fTemp135;
			float fTemp295 = 0.98078525f * fTemp203;
			float fTemp296 = fTemp291 + fTemp292 + fTemp293 + 0.19509032f * fTemp152 + fTemp62 + fTemp294 + fTemp295;
			float fTemp297 = 0.9951847f * fTemp209;
			float fTemp298 = 0.09801714f * fTemp192 + fTemp296 + fTemp297 - fTemp43;
			float fTemp299 = 0.9238795f * fTemp253;
			float fTemp300 = 0.70710677f * fTemp260;
			float fTemp301 = 0.98078525f * fTemp278;
			float fTemp302 = fTemp299 + fTemp300 + fTemp95 + fTemp301;
			float fTemp303 = 0.9951847f * fTemp248;
			float fTemp304 = 0.38268343f * fTemp257;
			float fTemp305 = 0.70710677f * fTemp262;
			float fTemp306 = 0.19509032f * fTemp283;
			output1[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(0.049067676f * fTemp210 + 0.09801714f * fTemp248 + fTemp254 + fTemp258 + fTemp261 + fTemp263 + fTemp279 + fTemp0 + fTemp284 + fTemp290 + 0.99879545f * fTemp298 - fTemp44)) + fftdsp_faustpower2_f(std::fabs(fTemp302 + fTemp303 + 0.99879545f * fTemp210 - (fTemp304 + fTemp305 + fTemp69 + fTemp306 + 0.09801714f * fTemp289 + 0.049067676f * fTemp298))));
			float fTemp307 = fTemp12 + fTemp120;
			float fTemp308 = fTemp75 + fTemp101 - (fTemp4 + fTemp112);
			float fTemp309 = 0.0f - fTemp308;
			float fTemp310 = fTemp307 + 0.70710677f * fTemp309;
			float fTemp311 = fTemp8 + fTemp116;
			float fTemp312 = fTemp71 + fTemp97 - (fTemp46 + fTemp108);
			float fTemp313 = 0.0f - fTemp312;
			float fTemp314 = fTemp79 + fTemp105;
			float fTemp315 = fTemp49 + fTemp30 - (fTemp16 + fTemp124);
			float fTemp316 = 0.70710677f * fTemp315;
			float fTemp317 = fTemp311 + 0.70710677f * fTemp313 - (fTemp314 + fTemp316);
			float fTemp318 = fTemp83 + fTemp26;
			float fTemp319 = fTemp53 + fTemp34 - (fTemp20 + fTemp85);
			float fTemp320 = 0.70710677f * fTemp319;
			float fTemp321 = fTemp318 + fTemp320;
			float fTemp322 = 0.70710677f * fTemp313;
			float fTemp323 = fTemp57 + fTemp38;
			float fTemp324 = 0.70710677f * fTemp315;
			float fTemp325 = fTemp63 + fTemp89;
			float fTemp326 = fTemp322 + fTemp323 + fTemp324 - fTemp325;
			float fTemp327 = fTemp310 + 0.9238795f * fTemp317 - (fTemp321 + 0.38268343f * fTemp326);
			float fTemp328 = 0.70710677f * fTemp309;
			float fTemp329 = fTemp61 + fTemp42;
			float fTemp330 = 0.70710677f * fTemp319;
			float fTemp331 = fTemp328 + fTemp329 + fTemp330;
			float fTemp332 = fTemp67 + fTemp93;
			float fTemp333 = 0.38268343f * fTemp317 + fTemp331 + 0.9238795f * fTemp326 - fTemp332;
			float fTemp334 = 0.98078525f * fTemp333;
			float fTemp335 = fTemp9 + fTemp117;
			float fTemp336 = fTemp72 + fTemp98 - (fTemp1 + fTemp109);
			float fTemp337 = 0.0f - fTemp336;
			float fTemp338 = fTemp80 + fTemp106;
			float fTemp339 = fTemp50 + fTemp31 - (fTemp17 + fTemp125);
			float fTemp340 = 0.70710677f * fTemp339;
			float fTemp341 = fTemp335 + 0.70710677f * fTemp337 - (fTemp338 + fTemp340);
			float fTemp342 = 0.9238795f * fTemp341;
			float fTemp343 = fTemp76 + fTemp102 - (fTemp5 + fTemp113);
			float fTemp344 = 0.0f - fTemp343;
			float fTemp345 = 0.70710677f * fTemp344;
			float fTemp346 = fTemp13 + fTemp121;
			float fTemp347 = fTemp74 + fTemp100 - (fTemp3 + fTemp111);
			float fTemp348 = 0.0f - fTemp347;
			float fTemp349 = 0.70710677f * fTemp348;
			float fTemp350 = fTemp11 + fTemp119;
			float fTemp351 = fTemp7 + fTemp115;
			float fTemp352 = fTemp70 + fTemp96 - (fTemp45 + fTemp107);
			float fTemp353 = 0.0f - fTemp352;
			float fTemp354 = fTemp78 + fTemp104;
			float fTemp355 = fTemp48 + fTemp29 - (fTemp15 + fTemp123);
			float fTemp356 = 0.70710677f * fTemp355;
			float fTemp357 = fTemp351 + 0.70710677f * fTemp353 - (fTemp354 + fTemp356);
			float fTemp358 = fTemp52 + fTemp33 - (fTemp19 + fTemp126);
			float fTemp359 = 0.70710677f * fTemp358;
			float fTemp360 = fTemp82 + fTemp25;
			float fTemp361 = 0.70710677f * fTemp355;
			float fTemp362 = fTemp56 + fTemp37;
			float fTemp363 = 0.70710677f * fTemp353;
			float fTemp364 = fTemp23 + fTemp88;
			float fTemp365 = fTemp361 + fTemp362 + fTemp363 - fTemp364;
			float fTemp366 = fTemp349 + fTemp350 + 0.9238795f * fTemp357 - (fTemp359 + fTemp360 + 0.38268343f * fTemp365);
			float fTemp367 = 0.98078525f * fTemp366;
			float fTemp368 = 0.70710677f * fTemp337;
			float fTemp369 = fTemp58 + fTemp39;
			float fTemp370 = 0.70710677f * fTemp339;
			float fTemp371 = fTemp64 + fTemp90;
			float fTemp372 = fTemp368 + fTemp369 + fTemp370 - fTemp371;
			float fTemp373 = 0.38268343f * fTemp372;
			float fTemp374 = fTemp54 + fTemp35 - (fTemp21 + fTemp86);
			float fTemp375 = 0.70710677f * fTemp374;
			float fTemp376 = fTemp84 + fTemp27;
			float fTemp377 = 0.70710677f * fTemp348;
			float fTemp378 = 0.70710677f * fTemp358;
			float fTemp379 = fTemp60 + fTemp41;
			float fTemp380 = fTemp66 + fTemp92;
			float fTemp381 = fTemp377 + fTemp378 + 0.38268343f * fTemp357 + fTemp379 + 0.9238795f * fTemp365 - fTemp380;
			float fTemp382 = fTemp342 + fTemp345 + fTemp346 + fTemp367 - (fTemp373 + fTemp375 + fTemp376 + 0.19509032f * fTemp381);
			float fTemp383 = fTemp10 + fTemp118;
			float fTemp384 = fTemp73 + fTemp99 - (fTemp2 + fTemp110);
			float fTemp385 = 0.0f - fTemp384;
			float fTemp386 = fTemp81 + fTemp24;
			float fTemp387 = fTemp51 + fTemp32 - (fTemp18 + fTemp127);
			float fTemp388 = 0.70710677f * fTemp387;
			float fTemp389 = fTemp383 + 0.70710677f * fTemp385 - (fTemp386 + fTemp388);
			float fTemp390 = 0.38268343f * fTemp389;
			float fTemp391 = fTemp77 + fTemp103 - (fTemp6 + fTemp114);
			float fTemp392 = 0.0f - fTemp391;
			float fTemp393 = 0.70710677f * fTemp392;
			float fTemp394 = fTemp0 + fTemp44;
			float fTemp395 = fTemp55 + fTemp36 - (fTemp22 + fTemp87);
			float fTemp396 = 0.70710677f * fTemp395;
			float fTemp397 = 0.70710677f * fTemp385;
			float fTemp398 = fTemp59 + fTemp40;
			float fTemp399 = 0.70710677f * fTemp387;
			float fTemp400 = fTemp65 + fTemp91;
			float fTemp401 = fTemp397 + fTemp398 + fTemp399 - fTemp400;
			float fTemp402 = 0.9238795f * fTemp401;
			float fTemp403 = 0.38268343f * fTemp341;
			float fTemp404 = 0.9238795f * fTemp372;
			float fTemp405 = 0.70710677f * fTemp344;
			float fTemp406 = 0.70710677f * fTemp374;
			float fTemp407 = fTemp62 + fTemp43;
			float fTemp408 = 0.98078525f * fTemp381;
			float fTemp409 = fTemp68 + fTemp94;
			float fTemp410 = fTemp403 + fTemp404 + fTemp405 + fTemp406 + 0.19509032f * fTemp366 + fTemp407 + fTemp408 - fTemp409;
			float fTemp411 = fTemp69 + fTemp95;
			float fTemp412 = 0.98078525f * fTemp327;
			float fTemp413 = fTemp14 + fTemp122;
			float fTemp414 = 0.70710677f * fTemp392;
			float fTemp415 = fTemp413 + fTemp414 + 0.9238795f * fTemp389;
			float fTemp416 = fTemp47 + fTemp28;
			float fTemp417 = 0.70710677f * fTemp395;
			float fTemp418 = fTemp416 + fTemp417;
			float fTemp419 = 0.38268343f * fTemp401;
			output2[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(0.19509032f * fTemp327 + fTemp334 + 0.09801714f * fTemp382 + fTemp390 + fTemp393 + fTemp394 + fTemp396 + fTemp402 + 0.9951847f * fTemp410 - fTemp411)) + fftdsp_faustpower2_f(std::fabs(fTemp412 + fTemp415 + 0.9951847f * fTemp382 - (0.19509032f * fTemp333 + fTemp418 + fTemp419 + 0.09801714f * fTemp410))));
			float fTemp420 = fTemp21 - (0.70710677f * fTemp131 + fTemp86 + fTemp195);
			float fTemp421 = 0.38268343f * fTemp420;
			float fTemp422 = fTemp68 + fTemp421;
			float fTemp423 = fTemp17 - (0.70710677f * fTemp141 + fTemp125 + fTemp150);
			float fTemp424 = 0.70710677f * fTemp138 + fTemp90 + fTemp201;
			float fTemp425 = fTemp50 + 0.70710677f * fTemp141 - (fTemp31 + fTemp144);
			float fTemp426 = fTemp64 + 0.38268343f * fTemp423 - (fTemp424 + 0.9238795f * fTemp425);
			float fTemp427 = fTemp422 + 0.8314696f * fTemp426;
			float fTemp428 = fTemp19 - (0.70710677f * fTemp159 + fTemp126 + fTemp185);
			float fTemp429 = fTemp66 + 0.38268343f * fTemp428;
			float fTemp430 = fTemp15 - (0.70710677f * fTemp168 + fTemp123 + fTemp177);
			float fTemp431 = 0.70710677f * fTemp165 + fTemp88 + fTemp189;
			float fTemp432 = fTemp48 + 0.70710677f * fTemp168 - (fTemp29 + fTemp171);
			float fTemp433 = fTemp23 + 0.38268343f * fTemp430 - (fTemp431 + 0.9238795f * fTemp432);
			float fTemp434 = 0.70710677f * fTemp155;
			float fTemp435 = fTemp434 + fTemp92 + fTemp206;
			float fTemp436 = fTemp52 + 0.70710677f * fTemp159 - (fTemp33 + fTemp162);
			float fTemp437 = fTemp435 + 0.9238795f * fTemp436;
			float fTemp438 = fTemp56 + 0.70710677f * fTemp165;
			float fTemp439 = fTemp37 + fTemp174;
			float fTemp440 = 0.9238795f * fTemp430 + fTemp438 + 0.38268343f * fTemp432 - fTemp439;
			float fTemp441 = fTemp429 + 0.8314696f * fTemp433 - (fTemp437 + 0.55557024f * fTemp440);
			float fTemp442 = 0.70710677f * fTemp128;
			float fTemp443 = fTemp442 + fTemp94 + fTemp293;
			float fTemp444 = fTemp54 + 0.70710677f * fTemp131 - (fTemp35 + fTemp134);
			float fTemp445 = 0.9238795f * fTemp444;
			float fTemp446 = fTemp443 + fTemp445;
			float fTemp447 = fTemp58 + 0.70710677f * fTemp138;
			float fTemp448 = fTemp39 + fTemp147;
			float fTemp449 = 0.9238795f * fTemp423 + fTemp447 + 0.38268343f * fTemp425 - fTemp448;
			float fTemp450 = fTemp446 + 0.55557024f * fTemp449;
			float fTemp451 = 0.70710677f * fTemp155;
			float fTemp452 = 0.9238795f * fTemp428 + fTemp60 + fTemp451 + 0.38268343f * fTemp436;
			float fTemp453 = fTemp41 + fTemp182;
			float fTemp454 = 0.55557024f * fTemp433 + fTemp452 + 0.8314696f * fTemp440 - fTemp453;
			float fTemp455 = fTemp427 + 0.95694035f * fTemp441 - (fTemp450 + 0.29028466f * fTemp454);
			float fTemp456 = fTemp16 - (0.70710677f * fTemp224 + fTemp124 + fTemp233);
			float fTemp457 = 0.70710677f * fTemp221 + fTemp89 + fTemp245;
			float fTemp458 = fTemp49 + 0.70710677f * fTemp224 - (fTemp30 + fTemp227);
			float fTemp459 = fTemp63 + 0.38268343f * fTemp456 - (fTemp457 + 0.9238795f * fTemp458);
			float fTemp460 = fTemp20 - (0.70710677f * fTemp215 + fTemp85 + fTemp241);
			float fTemp461 = 0.70710677f * fTemp211;
			float fTemp462 = fTemp53 + 0.70710677f * fTemp215 - (fTemp34 + fTemp218);
			float fTemp463 = 0.9238795f * fTemp460 + fTemp61 + fTemp461 + 0.38268343f * fTemp462;
			float fTemp464 = fTemp57 + 0.70710677f * fTemp221;
			float fTemp465 = fTemp38 + fTemp230;
			float fTemp466 = 0.9238795f * fTemp456 + fTemp464 + 0.38268343f * fTemp458 - fTemp465;
			float fTemp467 = fTemp42 + fTemp238;
			float fTemp468 = 0.55557024f * fTemp459 + fTemp463 + 0.8314696f * fTemp466 - fTemp467;
			float fTemp469 = fTemp18 - (0.70710677f * fTemp267 + fTemp127 + fTemp276);
			float fTemp470 = 0.70710677f * fTemp264 + fTemp91 + fTemp281;
			float fTemp471 = fTemp51 + 0.70710677f * fTemp267 - (fTemp32 + fTemp270);
			float fTemp472 = fTemp65 + 0.38268343f * fTemp469 - (fTemp470 + 0.9238795f * fTemp471);
			float fTemp473 = 0.55557024f * fTemp472;
			float fTemp474 = fTemp59 + 0.70710677f * fTemp264;
			float fTemp475 = fTemp40 + fTemp273;
			float fTemp476 = 0.9238795f * fTemp469 + fTemp474 + 0.38268343f * fTemp471 - fTemp475;
			float fTemp477 = 0.8314696f * fTemp476;
			float fTemp478 = fTemp22 - (0.70710677f * fTemp249 + fTemp87 + fTemp256);
			float fTemp479 = 0.9238795f * fTemp478;
			float fTemp480 = 0.70710677f * fTemp259;
			float fTemp481 = fTemp55 + 0.70710677f * fTemp249 - (fTemp36 + fTemp252);
			float fTemp482 = 0.38268343f * fTemp481;
			float fTemp483 = fTemp67 + 0.38268343f * fTemp460;
			float fTemp484 = 0.70710677f * fTemp211;
			float fTemp485 = fTemp484 + fTemp93 + fTemp286;
			float fTemp486 = fTemp485 + 0.9238795f * fTemp462;
			float fTemp487 = fTemp483 + 0.8314696f * fTemp459 - (fTemp486 + 0.55557024f * fTemp466);
			float fTemp488 = 0.9238795f * fTemp420;
			float fTemp489 = 0.70710677f * fTemp128;
			float fTemp490 = 0.38268343f * fTemp444;
			float fTemp491 = 0.55557024f * fTemp426 + fTemp488 + fTemp62 + fTemp489 + fTemp490 + 0.8314696f * fTemp449;
			float fTemp492 = fTemp43 + fTemp199;
			float fTemp493 = 0.29028466f * fTemp441 + fTemp491 + 0.95694035f * fTemp454 - fTemp492;
			float fTemp494 = 0.8314696f * fTemp472;
			float fTemp495 = 0.38268343f * fTemp478;
			float fTemp496 = fTemp69 + fTemp495;
			float fTemp497 = 0.70710677f * fTemp259;
			float fTemp498 = fTemp497 + fTemp95 + fTemp263;
			float fTemp499 = 0.9238795f * fTemp481;
			float fTemp500 = fTemp498 + fTemp499;
			float fTemp501 = 0.55557024f * fTemp476;
			output3[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(0.14673047f * fTemp455 + 0.95694035f * fTemp468 + fTemp473 + fTemp477 + fTemp479 + fTemp0 + fTemp480 + fTemp482 + 0.29028466f * fTemp487 + 0.9891765f * fTemp493 - (fTemp44 + fTemp305))) + fftdsp_faustpower2_f(std::fabs(fTemp494 + fTemp496 + 0.95694035f * fTemp487 + 0.9891765f * fTemp455 - (fTemp500 + fTemp501 + 0.29028466f * fTemp468 + 0.14673047f * fTemp493))));
			float fTemp502 = fTemp102 + fTemp113;
			float fTemp503 = fTemp5 + fTemp76 + fTemp502;
			float fTemp504 = fTemp17 + fTemp31 + fTemp50 + fTemp125 - (fTemp1 + fTemp72 + fTemp98 + fTemp109);
			float fTemp505 = 0.0f - fTemp504;
			float fTemp506 = fTemp503 + 0.70710677f * fTemp505;
			float fTemp507 = fTemp3 + fTemp74 + fTemp100 + fTemp111;
			float fTemp508 = fTemp15 + fTemp29 + fTemp48 + fTemp123 - (fTemp45 + fTemp70 + fTemp96 + fTemp107);
			float fTemp509 = 0.0f - fTemp508;
			float fTemp510 = fTemp19 + fTemp33 + fTemp52 + fTemp126;
			float fTemp511 = fTemp104 + fTemp115;
			float fTemp512 = fTemp23 + fTemp37 + fTemp56 + fTemp88 - (fTemp7 + fTemp78 + fTemp511);
			float fTemp513 = 0.70710677f * fTemp512;
			float fTemp514 = fTemp507 + 0.70710677f * fTemp509 - (fTemp510 + fTemp513);
			float fTemp515 = fTemp21 + fTemp35 + fTemp54 + fTemp86;
			float fTemp516 = fTemp106 + fTemp117;
			float fTemp517 = fTemp39 + fTemp58 + fTemp371 - (fTemp9 + fTemp80 + fTemp516);
			float fTemp518 = 0.70710677f * fTemp517;
			float fTemp519 = fTemp515 + fTemp518;
			float fTemp520 = 0.70710677f * fTemp509;
			float fTemp521 = fTemp41 + fTemp60 + fTemp380;
			float fTemp522 = 0.70710677f * fTemp512;
			float fTemp523 = fTemp11 + fTemp25 + fTemp82 + fTemp119;
			float fTemp524 = fTemp520 + fTemp521 + fTemp522 - fTemp523;
			float fTemp525 = fTemp506 + 0.9238795f * fTemp514 - (fTemp519 + 0.38268343f * fTemp524);
			float fTemp526 = fTemp101 + fTemp112;
			float fTemp527 = fTemp4 + fTemp75 + fTemp526;
			float fTemp528 = fTemp16 + fTemp30 + fTemp49 + fTemp124 - (fTemp46 + fTemp71 + fTemp97 + fTemp108);
			float fTemp529 = 0.0f - fTemp528;
			float fTemp530 = fTemp20 + fTemp34 + fTemp53 + fTemp85;
			float fTemp531 = fTemp105 + fTemp116;
			float fTemp532 = fTemp38 + fTemp57 + fTemp325 - (fTemp8 + fTemp79 + fTemp531);
			float fTemp533 = 0.70710677f * fTemp532;
			float fTemp534 = fTemp527 + 0.70710677f * fTemp529 - (fTemp530 + fTemp533);
			float fTemp535 = fTemp18 + fTemp32 + fTemp51 + fTemp127 - (fTemp2 + fTemp73 + fTemp99 + fTemp110);
			float fTemp536 = 0.0f - fTemp535;
			float fTemp537 = 0.70710677f * fTemp536;
			float fTemp538 = fTemp44 + fTemp0 + fTemp411;
			float fTemp539 = fTemp40 + fTemp59 + fTemp400 - (fTemp10 + fTemp24 + fTemp81 + fTemp118);
			float fTemp540 = 0.70710677f * fTemp539;
			float fTemp541 = 0.70710677f * fTemp529;
			float fTemp542 = fTemp42 + fTemp61 + fTemp332;
			float fTemp543 = 0.70710677f * fTemp532;
			float fTemp544 = fTemp12 + fTemp26 + fTemp83 + fTemp120;
			float fTemp545 = fTemp541 + fTemp542 + fTemp543 - fTemp544;
			float fTemp546 = 0.70710677f * fTemp505;
			float fTemp547 = fTemp43 + fTemp62 + fTemp409;
			float fTemp548 = 0.70710677f * fTemp517;
			float fTemp549 = fTemp546 + fTemp547 + fTemp548;
			float fTemp550 = fTemp13 + fTemp27 + fTemp84 + fTemp121;
			float fTemp551 = 0.38268343f * fTemp514 + fTemp549 + 0.9238795f * fTemp524 - fTemp550;
			float fTemp552 = fTemp14 + fTemp28 + fTemp47 + fTemp122;
			float fTemp553 = fTemp103 + fTemp114;
			float fTemp554 = fTemp6 + fTemp77 + fTemp553;
			float fTemp555 = fTemp554 + 0.70710677f * fTemp536;
			float fTemp556 = fTemp22 + fTemp36 + fTemp55 + fTemp87;
			float fTemp557 = 0.70710677f * fTemp539;
			output4[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(0.19509032f * fTemp525 + 0.38268343f * fTemp534 + fTemp537 + fTemp538 + fTemp540 + 0.9238795f * fTemp545 + 0.98078525f * fTemp551 - fTemp552)) + fftdsp_faustpower2_f(std::fabs(fTemp555 + 0.9238795f * fTemp534 + 0.98078525f * fTemp525 - (fTemp556 + fTemp557 + 0.38268343f * fTemp545 + 0.19509032f * fTemp551))));
			float fTemp558 = 0.70710677f * fTemp139;
			float fTemp559 = fTemp125 + fTemp144 - (fTemp17 + 0.70710677f * fTemp142);
			float fTemp560 = 0.70710677f * fTemp143;
			float fTemp561 = fTemp50 - (fTemp149 + fTemp31 + fTemp560);
			float fTemp562 = fTemp90 + fTemp147 - (fTemp558 + 0.38268343f * fTemp559 + fTemp64 + 0.9238795f * fTemp561);
			float fTemp563 = fTemp94 + 0.55557024f * fTemp562;
			float fTemp564 = 0.70710677f * fTemp166;
			float fTemp565 = fTemp123 + fTemp171 - (fTemp15 + 0.70710677f * fTemp169);
			float fTemp566 = 0.70710677f * fTemp170;
			float fTemp567 = fTemp48 - (fTemp176 + fTemp29 + fTemp566);
			float fTemp568 = fTemp88 + fTemp174 - (fTemp564 + 0.38268343f * fTemp565 + fTemp23 + 0.9238795f * fTemp567);
			float fTemp569 = 0.70710677f * fTemp156;
			float fTemp570 = fTemp126 + fTemp162 - (fTemp19 + 0.70710677f * fTemp160);
			float fTemp571 = 0.70710677f * fTemp161;
			float fTemp572 = fTemp52 - (fTemp184 + fTemp33 + fTemp571);
			float fTemp573 = 0.38268343f * fTemp570 + fTemp66 + 0.9238795f * fTemp572;
			float fTemp574 = 0.70710677f * fTemp173;
			float fTemp575 = fTemp56 + 0.9238795f * fTemp565 - (fTemp188 + fTemp574 + fTemp37 + 0.38268343f * fTemp567);
			float fTemp576 = 0.8314696f * fTemp575;
			float fTemp577 = fTemp182 + fTemp92 + 0.55557024f * fTemp568 - (fTemp569 + fTemp573 + fTemp576);
			float fTemp578 = 0.8819213f * fTemp577;
			float fTemp579 = 0.70710677f * fTemp129;
			float fTemp580 = fTemp86 + fTemp134 - (fTemp21 + 0.70710677f * fTemp132);
			float fTemp581 = 0.38268343f * fTemp580;
			float fTemp582 = 0.70710677f * fTemp133;
			float fTemp583 = fTemp54 - (fTemp194 + fTemp35 + fTemp582);
			float fTemp584 = 0.9238795f * fTemp583;
			float fTemp585 = fTemp581 + fTemp68 + fTemp584;
			float fTemp586 = 0.70710677f * fTemp146;
			float fTemp587 = fTemp58 + 0.9238795f * fTemp559 - (fTemp200 + fTemp586 + fTemp39 + 0.38268343f * fTemp561);
			float fTemp588 = 0.8314696f * fTemp587;
			float fTemp589 = fTemp585 + fTemp588;
			float fTemp590 = 0.8314696f * fTemp568;
			float fTemp591 = fTemp60 + 0.9238795f * fTemp570;
			float fTemp592 = 0.70710677f * fTemp181;
			float fTemp593 = fTemp41 + 0.38268343f * fTemp572;
			float fTemp594 = fTemp590 + fTemp591 + 0.55557024f * fTemp575 - (fTemp205 + fTemp592 + fTemp593);
			float fTemp595 = 0.47139674f * fTemp594;
			float fTemp596 = fTemp199 + fTemp563 + fTemp578 - (fTemp579 + fTemp589 + fTemp595);
			float fTemp597 = 0.70710677f * fTemp265;
			float fTemp598 = fTemp127 + fTemp270 - (fTemp18 + 0.70710677f * fTemp268);
			float fTemp599 = 0.70710677f * fTemp269;
			float fTemp600 = fTemp51 - (fTemp275 + fTemp32 + fTemp599);
			float fTemp601 = fTemp91 + fTemp273 - (fTemp597 + 0.38268343f * fTemp598 + fTemp65 + 0.9238795f * fTemp600);
			float fTemp602 = 0.8314696f * fTemp601;
			float fTemp603 = 0.70710677f * fTemp272;
			float fTemp604 = fTemp59 + 0.9238795f * fTemp598 - (fTemp280 + fTemp603 + fTemp40 + 0.38268343f * fTemp600);
			float fTemp605 = 0.55557024f * fTemp604;
			float fTemp606 = fTemp87 + fTemp252 - (fTemp22 + 0.70710677f * fTemp250);
			float fTemp607 = 0.9238795f * fTemp606;
			float fTemp608 = 0.70710677f * fTemp222;
			float fTemp609 = fTemp124 + fTemp227 - (fTemp16 + 0.70710677f * fTemp225);
			float fTemp610 = 0.70710677f * fTemp226;
			float fTemp611 = fTemp49 - (fTemp232 + fTemp30 + fTemp610);
			float fTemp612 = fTemp89 + fTemp230 - (fTemp608 + 0.38268343f * fTemp609 + fTemp63 + 0.9238795f * fTemp611);
			float fTemp613 = 0.70710677f * fTemp212;
			float fTemp614 = fTemp85 + fTemp218 - (fTemp20 + 0.70710677f * fTemp216);
			float fTemp615 = 0.38268343f * fTemp614;
			float fTemp616 = 0.70710677f * fTemp217;
			float fTemp617 = fTemp53 - (fTemp240 + fTemp34 + fTemp616);
			float fTemp618 = 0.9238795f * fTemp617;
			float fTemp619 = 0.70710677f * fTemp229;
			float fTemp620 = fTemp57 + 0.9238795f * fTemp609 - (fTemp244 + fTemp619 + fTemp38 + 0.38268343f * fTemp611);
			float fTemp621 = 0.8314696f * fTemp620;
			float fTemp622 = fTemp238 + fTemp93 + 0.55557024f * fTemp612 - (fTemp613 + fTemp615 + fTemp618 + fTemp67 + fTemp621);
			float fTemp623 = 0.47139674f * fTemp622;
			float fTemp624 = 0.9238795f * fTemp614;
			float fTemp625 = 0.8314696f * fTemp612;
			float fTemp626 = 0.70710677f * fTemp237;
			float fTemp627 = 0.38268343f * fTemp617;
			float fTemp628 = fTemp624 + fTemp625 + fTemp61 + 0.55557024f * fTemp620 - (fTemp285 + fTemp626 + fTemp42 + fTemp627);
			float fTemp629 = 0.8819213f * fTemp628;
			float fTemp630 = 0.47139674f * fTemp577;
			float fTemp631 = 0.8314696f * fTemp562;
			float fTemp632 = 0.9238795f * fTemp580;
			float fTemp633 = fTemp631 + fTemp62 + fTemp632 + 0.55557024f * fTemp587;
			float fTemp634 = 0.8819213f * fTemp594;
			float fTemp635 = 0.70710677f * fTemp198;
			float fTemp636 = 0.38268343f * fTemp583;
			float fTemp637 = fTemp43 + fTemp636;
			float fTemp638 = fTemp630 + fTemp633 + fTemp634 - (fTemp292 + fTemp635 + fTemp637);
			float fTemp639 = 0.70710677f * fTemp262;
			float fTemp640 = 0.70710677f * fTemp251;
			float fTemp641 = fTemp55 - (fTemp255 + fTemp36 + fTemp640);
			float fTemp642 = 0.38268343f * fTemp641;
			float fTemp643 = 0.55557024f * fTemp601;
			float fTemp644 = 0.8819213f * fTemp622;
			float fTemp645 = 0.70710677f * fTemp260;
			float fTemp646 = 0.8314696f * fTemp604;
			float fTemp647 = 0.38268343f * fTemp606;
			float fTemp648 = 0.9238795f * fTemp641;
			float fTemp649 = 0.47139674f * fTemp628;
			output5[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(0.24298018f * fTemp596 + fTemp602 + fTemp605 + fTemp607 + fTemp623 + fTemp0 + fTemp629 + 0.97003126f * fTemp638 - (fTemp261 + fTemp639 + fTemp44 + fTemp642))) + fftdsp_faustpower2_f(std::fabs(fTemp305 + fTemp643 + fTemp95 + fTemp644 + 0.97003126f * fTemp596 - (fTemp645 + fTemp646 + fTemp647 + fTemp648 + fTemp69 + fTemp649 + 0.24298018f * fTemp638))));
			float fTemp650 = 0.70710677f * fTemp391;
			float fTemp651 = fTemp338 - (0.70710677f * fTemp336 + fTemp335 + fTemp370);
			float fTemp652 = fTemp376 + 0.38268343f * fTemp651;
			float fTemp653 = fTemp354 - (0.70710677f * fTemp352 + fTemp351 + fTemp361);
			float fTemp654 = 0.70710677f * fTemp347;
			float fTemp655 = fTemp362 + 0.70710677f * fTemp352 - (fTemp364 + fTemp356);
			float fTemp656 = fTemp360 + 0.38268343f * fTemp653 - (fTemp654 + fTemp378 + fTemp350 + 0.9238795f * fTemp655);
			float fTemp657 = 0.70710677f * fTemp343;
			float fTemp658 = fTemp369 + 0.70710677f * fTemp336 - (fTemp371 + fTemp340);
			float fTemp659 = fTemp346 + 0.9238795f * fTemp658;
			float fTemp660 = 0.70710677f * fTemp347;
			float fTemp661 = fTemp660 + 0.9238795f * fTemp653 + fTemp379 + 0.38268343f * fTemp655 - (fTemp380 + fTemp359);
			float fTemp662 = fTemp652 + 0.8314696f * fTemp656 - (fTemp657 + fTemp406 + fTemp659 + 0.55557024f * fTemp661);
			float fTemp663 = fTemp314 - (0.70710677f * fTemp312 + fTemp311 + fTemp324);
			float fTemp664 = 0.70710677f * fTemp308;
			float fTemp665 = fTemp323 + 0.70710677f * fTemp312 - (fTemp325 + fTemp316);
			float fTemp666 = fTemp318 + 0.38268343f * fTemp663 - (fTemp664 + fTemp330 + fTemp307 + 0.9238795f * fTemp665);
			float fTemp667 = fTemp386 - (0.70710677f * fTemp384 + fTemp383 + fTemp399);
			float fTemp668 = fTemp398 + 0.70710677f * fTemp384 - (fTemp400 + fTemp388);
			float fTemp669 = 0.9238795f * fTemp667 + fTemp394 + 0.38268343f * fTemp668;
			float fTemp670 = 0.70710677f * fTemp308;
			float fTemp671 = fTemp670 + 0.9238795f * fTemp663 + fTemp329 + 0.38268343f * fTemp665 - (fTemp332 + fTemp320);
			float fTemp672 = 0.70710677f * fTemp343;
			float fTemp673 = 0.9238795f * fTemp651 + fTemp407 + 0.38268343f * fTemp658;
			float fTemp674 = fTemp672 + 0.55557024f * fTemp656 + fTemp673 + 0.8314696f * fTemp661 - (fTemp409 + fTemp375);
			float fTemp675 = fTemp416 + 0.38268343f * fTemp667;
			float fTemp676 = 0.70710677f * fTemp391;
			float fTemp677 = fTemp413 + 0.9238795f * fTemp668;
			output6[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp650 + 0.29028466f * fTemp662 + 0.55557024f * fTemp666 + fTemp669 + 0.8314696f * fTemp671 + 0.95694035f * fTemp674 - (fTemp411 + fTemp417))) + fftdsp_faustpower2_f(std::fabs(fTemp675 + 0.8314696f * fTemp666 + 0.95694035f * fTemp662 - (fTemp676 + fTemp396 + fTemp677 + 0.55557024f * fTemp671 + 0.29028466f * fTemp674))));
			float fTemp678 = 0.70710677f * fTemp262;
			float fTemp679 = 0.70710677f * fTemp211;
			float fTemp680 = 0.70710677f * fTemp221;
			float fTemp681 = 0.70710677f * fTemp224 + fTemp16 + fTemp610 - fTemp124;
			float fTemp682 = fTemp49 + 0.70710677f * fTemp226 - (fTemp30 + 0.70710677f * fTemp224);
			float fTemp683 = fTemp680 + fTemp63 + fTemp619 - (0.9238795f * fTemp681 + fTemp89 + 0.38268343f * fTemp682);
			float fTemp684 = 0.70710677f * fTemp215 + fTemp20 + fTemp616 - fTemp85;
			float fTemp685 = fTemp53 + 0.70710677f * fTemp217 - (fTemp34 + 0.70710677f * fTemp215);
			float fTemp686 = 0.9238795f * fTemp684 + fTemp93 + 0.38268343f * fTemp685;
			float fTemp687 = 0.70710677f * fTemp229;
			float fTemp688 = 0.70710677f * fTemp221;
			float fTemp689 = fTemp687 + fTemp57 + 0.38268343f * fTemp681 - (fTemp688 + fTemp38 + 0.9238795f * fTemp682);
			float fTemp690 = fTemp679 + fTemp626 + fTemp67 + 0.19509032f * fTemp683 - (fTemp686 + 0.98078525f * fTemp689);
			float fTemp691 = 0.70710677f * fTemp128;
			float fTemp692 = 0.70710677f * fTemp138;
			float fTemp693 = 0.70710677f * fTemp141 + fTemp17 + fTemp560 - fTemp125;
			float fTemp694 = fTemp50 + 0.70710677f * fTemp143 - (fTemp31 + 0.70710677f * fTemp141);
			float fTemp695 = fTemp692 + fTemp64 + fTemp586 - (0.9238795f * fTemp693 + fTemp90 + 0.38268343f * fTemp694);
			float fTemp696 = 0.19509032f * fTemp695;
			float fTemp697 = 0.70710677f * fTemp155;
			float fTemp698 = 0.70710677f * fTemp165;
			float fTemp699 = 0.70710677f * fTemp168 + fTemp15 + fTemp566 - fTemp123;
			float fTemp700 = fTemp48 + 0.70710677f * fTemp170 - (fTemp29 + 0.70710677f * fTemp168);
			float fTemp701 = fTemp698 + fTemp23 + fTemp574 - (0.9238795f * fTemp699 + fTemp88 + 0.38268343f * fTemp700);
			float fTemp702 = 0.70710677f * fTemp159 + fTemp19 + fTemp571 - fTemp126;
			float fTemp703 = 0.9238795f * fTemp702;
			float fTemp704 = fTemp52 + 0.70710677f * fTemp161 - (fTemp33 + 0.70710677f * fTemp159);
			float fTemp705 = 0.38268343f * fTemp704;
			float fTemp706 = 0.70710677f * fTemp173;
			float fTemp707 = 0.70710677f * fTemp165;
			float fTemp708 = fTemp706 + fTemp56 + 0.38268343f * fTemp699 - (fTemp707 + fTemp37 + 0.9238795f * fTemp700);
			float fTemp709 = fTemp697 + fTemp592 + fTemp66 + 0.19509032f * fTemp701 - (fTemp703 + fTemp92 + fTemp705 + 0.98078525f * fTemp708);
			float fTemp710 = 0.70710677f * fTemp146;
			float fTemp711 = 0.70710677f * fTemp138;
			float fTemp712 = fTemp710 + fTemp58 + 0.38268343f * fTemp693 - (fTemp711 + fTemp39 + 0.9238795f * fTemp694);
			float fTemp713 = 0.98078525f * fTemp712;
			float fTemp714 = 0.70710677f * fTemp131 + fTemp21 + fTemp582 - fTemp86;
			float fTemp715 = 0.9238795f * fTemp714;
			float fTemp716 = fTemp54 + 0.70710677f * fTemp133 - (fTemp35 + 0.70710677f * fTemp131);
			float fTemp717 = 0.38268343f * fTemp716;
			float fTemp718 = 0.70710677f * fTemp181;
			float fTemp719 = 0.38268343f * fTemp702;
			float fTemp720 = 0.70710677f * fTemp155;
			float fTemp721 = 0.9238795f * fTemp704;
			float fTemp722 = fTemp718 + 0.19509032f * fTemp708 + fTemp719 + fTemp60 + 0.98078525f * fTemp701 - (fTemp720 + fTemp41 + fTemp721);
			float fTemp723 = fTemp691 + fTemp635 + fTemp696 + fTemp68 + 0.77301043f * fTemp709 - (fTemp713 + fTemp715 + fTemp717 + fTemp94 + 0.6343933f * fTemp722);
			float fTemp724 = 0.70710677f * fTemp264;
			float fTemp725 = 0.70710677f * fTemp267 + fTemp18 + fTemp599 - fTemp127;
			float fTemp726 = fTemp51 + 0.70710677f * fTemp269 - (fTemp32 + 0.70710677f * fTemp267);
			float fTemp727 = fTemp724 + fTemp65 + fTemp603 - (0.9238795f * fTemp725 + fTemp91 + 0.38268343f * fTemp726);
			float fTemp728 = 0.70710677f * fTemp249 + fTemp22 + fTemp640 - fTemp87;
			float fTemp729 = 0.38268343f * fTemp728;
			float fTemp730 = 0.70710677f * fTemp272;
			float fTemp731 = 0.70710677f * fTemp264;
			float fTemp732 = fTemp730 + fTemp59 + 0.38268343f * fTemp725 - (fTemp731 + fTemp40 + 0.9238795f * fTemp726);
			float fTemp733 = 0.98078525f * fTemp727 + fTemp0 + fTemp729 + 0.19509032f * fTemp732;
			float fTemp734 = 0.70710677f * fTemp237;
			float fTemp735 = fTemp61 + 0.38268343f * fTemp684;
			float fTemp736 = 0.70710677f * fTemp211;
			float fTemp737 = fTemp42 + 0.9238795f * fTemp685;
			float fTemp738 = fTemp734 + 0.98078525f * fTemp683 + fTemp735 + 0.19509032f * fTemp689 - (fTemp736 + fTemp737);
			float fTemp739 = 0.70710677f * fTemp198;
			float fTemp740 = 0.98078525f * fTemp695;
			float fTemp741 = 0.19509032f * fTemp712;
			float fTemp742 = 0.38268343f * fTemp714;
			float fTemp743 = 0.70710677f * fTemp128;
			float fTemp744 = 0.9238795f * fTemp716;
			float fTemp745 = fTemp739 + fTemp740 + fTemp741 + fTemp742 + 0.6343933f * fTemp709 + fTemp62 + 0.77301043f * fTemp722 - (fTemp743 + fTemp43 + fTemp744);
			float fTemp746 = 0.70710677f * fTemp259;
			float fTemp747 = fTemp55 + 0.70710677f * fTemp251 - (fTemp36 + 0.70710677f * fTemp249);
			float fTemp748 = 0.9238795f * fTemp747;
			float fTemp749 = fTemp44 + fTemp748;
			float fTemp750 = 0.70710677f * fTemp259;
			float fTemp751 = fTemp69 + 0.19509032f * fTemp727;
			float fTemp752 = 0.9238795f * fTemp728;
			float fTemp753 = 0.38268343f * fTemp747;
			float fTemp754 = fTemp752 + fTemp95 + fTemp753;
			float fTemp755 = fTemp754 + 0.98078525f * fTemp732;
			output7[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp678 + 0.6343933f * fTemp690 + 0.33688986f * fTemp723 + fTemp733 + 0.77301043f * fTemp738 + 0.94154406f * fTemp745 - (fTemp746 + fTemp749))) + fftdsp_faustpower2_f(std::fabs(fTemp750 + fTemp639 + 0.77301043f * fTemp690 + fTemp751 + 0.94154406f * fTemp723 - (fTemp755 + 0.6343933f * fTemp738 + 0.33688986f * fTemp745))));
			float fTemp756 = fTemp1 + fTemp17 + fTemp31 + fTemp50 + fTemp72 + fTemp98 + fTemp125 + fTemp109;
			float fTemp757 = fTemp7 + fTemp23 + fTemp37 + fTemp56 + fTemp78 + fTemp88 + fTemp511 - (fTemp15 + fTemp29 + fTemp45 + fTemp48 + fTemp70 + fTemp96 + fTemp123 + fTemp107);
			float fTemp758 = 0.0f - fTemp757;
			float fTemp759 = fTemp9 + fTemp39 + fTemp58 + fTemp64 + fTemp80 + fTemp90 + fTemp516;
			float fTemp760 = fTemp11 + fTemp25 + fTemp41 + fTemp60 + fTemp66 + fTemp82 + fTemp92 + fTemp119 - (fTemp3 + fTemp19 + fTemp33 + fTemp52 + fTemp74 + fTemp100 + fTemp126 + fTemp111);
			float fTemp761 = 0.70710677f * fTemp760;
			float fTemp762 = fTemp756 + 0.70710677f * fTemp758 - (fTemp759 + fTemp761);
			float fTemp763 = fTemp8 + fTemp38 + fTemp57 + fTemp63 + fTemp79 + fTemp89 + fTemp531 - (fTemp16 + fTemp30 + fTemp46 + fTemp49 + fTemp71 + fTemp97 + fTemp124 + fTemp108);
			float fTemp764 = 0.0f - fTemp763;
			float fTemp765 = 0.70710677f * fTemp764;
			float fTemp766 = fTemp14 + fTemp28 + fTemp44 + fTemp47 + fTemp0 + fTemp69 + fTemp95 + fTemp122;
			float fTemp767 = fTemp12 + fTemp26 + fTemp42 + fTemp61 + fTemp67 + fTemp83 + fTemp93 + fTemp120 - (fTemp4 + fTemp20 + fTemp34 + fTemp53 + fTemp75 + fTemp85 + fTemp526);
			float fTemp768 = 0.70710677f * fTemp767;
			float fTemp769 = 0.70710677f * fTemp758;
			float fTemp770 = fTemp13 + fTemp27 + fTemp43 + fTemp62 + fTemp68 + fTemp84 + fTemp94 + fTemp121;
			float fTemp771 = 0.70710677f * fTemp760;
			float fTemp772 = fTemp5 + fTemp21 + fTemp35 + fTemp54 + fTemp76 + fTemp86 + fTemp502;
			float fTemp773 = fTemp769 + fTemp770 + fTemp771 - fTemp772;
			float fTemp774 = fTemp6 + fTemp22 + fTemp36 + fTemp55 + fTemp77 + fTemp87 + fTemp553;
			float fTemp775 = fTemp2 + fTemp18 + fTemp32 + fTemp51 + fTemp73 + fTemp99 + fTemp127 + fTemp110;
			float fTemp776 = fTemp10 + fTemp24 + fTemp40 + fTemp59 + fTemp65 + fTemp81 + fTemp91 + fTemp118;
			float fTemp777 = 0.70710677f * fTemp767;
			output8[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(0.38268343f * fTemp762 + fTemp765 + fTemp766 + fTemp768 + 0.9238795f * fTemp773 - fTemp774)) + fftdsp_faustpower2_f(std::fabs(fTemp775 + 0.70710677f * fTemp764 + 0.9238795f * fTemp762 - (fTemp776 + fTemp777 + 0.38268343f * fTemp773))));
			float fTemp778 = 0.38268343f * fTemp242;
			float fTemp779 = 0.9238795f * fTemp219;
			float fTemp780 = fTemp223 + 0.38268343f * fTemp234 - (fTemp231 + 0.9238795f * fTemp228);
			float fTemp781 = fTemp246 - (0.38268343f * fTemp228 + fTemp38 + 0.9238795f * fTemp234);
			float fTemp782 = 0.98078525f * fTemp781;
			float fTemp783 = fTemp214 + fTemp778 - (fTemp779 + fTemp238 + 0.19509032f * fTemp780 + fTemp67 + fTemp782);
			float fTemp784 = 0.98078525f * fTemp780;
			float fTemp785 = 0.38268343f * fTemp219;
			float fTemp786 = 0.9238795f * fTemp242;
			float fTemp787 = fTemp285 + fTemp286 + fTemp61 + fTemp784 - (fTemp785 + fTemp786 + fTemp42 + 0.19509032f * fTemp781);
			float fTemp788 = fTemp266 + 0.38268343f * fTemp277 - (fTemp274 + 0.9238795f * fTemp271);
			float fTemp789 = 0.98078525f * fTemp788;
			float fTemp790 = 0.38268343f * fTemp196;
			float fTemp791 = 0.38268343f * fTemp186;
			float fTemp792 = 0.9238795f * fTemp163;
			float fTemp793 = fTemp167 + 0.38268343f * fTemp178 - (fTemp175 + 0.9238795f * fTemp172);
			float fTemp794 = fTemp190 - (0.38268343f * fTemp172 + fTemp37 + 0.9238795f * fTemp178);
			float fTemp795 = 0.98078525f * fTemp794;
			float fTemp796 = fTemp158 + fTemp791 - (fTemp792 + fTemp182 + 0.19509032f * fTemp793 + fTemp66 + fTemp795);
			float fTemp797 = 0.9238795f * fTemp135;
			float fTemp798 = fTemp140 + 0.38268343f * fTemp151 - (fTemp148 + 0.9238795f * fTemp145);
			float fTemp799 = fTemp202 - (0.38268343f * fTemp145 + fTemp39 + 0.9238795f * fTemp151);
			float fTemp800 = 0.98078525f * fTemp799;
			float fTemp801 = 0.19509032f * fTemp798 + fTemp68 + fTemp800;
			float fTemp802 = 0.98078525f * fTemp793;
			float fTemp803 = 0.38268343f * fTemp163;
			float fTemp804 = 0.9238795f * fTemp186;
			float fTemp805 = fTemp205 + fTemp206 + fTemp60 + fTemp802 - (fTemp803 + fTemp804 + fTemp41 + 0.19509032f * fTemp794);
			float fTemp806 = fTemp790 + fTemp130 + fTemp94 + 0.6343933f * fTemp796 - (fTemp199 + fTemp797 + fTemp801 + 0.77301043f * fTemp805);
			float fTemp807 = 0.98078525f * fTemp798;
			float fTemp808 = fTemp62 + fTemp807;
			float fTemp809 = 0.9238795f * fTemp196;
			float fTemp810 = 0.38268343f * fTemp135;
			float fTemp811 = fTemp43 + 0.19509032f * fTemp799;
			float fTemp812 = fTemp292 + fTemp293 + 0.77301043f * fTemp796 + fTemp808 + 0.6343933f * fTemp805 - (fTemp809 + fTemp810 + fTemp811);
			float fTemp813 = 0.38268343f * fTemp253;
			float fTemp814 = 0.9238795f * fTemp257;
			float fTemp815 = fTemp282 - (0.38268343f * fTemp271 + fTemp40 + 0.9238795f * fTemp277);
			float fTemp816 = fTemp44 + 0.19509032f * fTemp815;
			float fTemp817 = 0.38268343f * fTemp257;
			float fTemp818 = 0.9238795f * fTemp253;
			float fTemp819 = 0.19509032f * fTemp788;
			float fTemp820 = 0.98078525f * fTemp815;
			output9[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp261 + fTemp263 + 0.77301043f * fTemp783 + 0.6343933f * fTemp787 + fTemp789 + 0.42755508f * fTemp806 + fTemp0 + 0.9039893f * fTemp812 - (fTemp813 + fTemp814 + fTemp816))) + fftdsp_faustpower2_f(std::fabs(fTemp817 + fTemp300 + 0.6343933f * fTemp783 + fTemp95 + 0.9039893f * fTemp806 - (fTemp818 + fTemp305 + 0.77301043f * fTemp787 + fTemp819 + fTemp820 + fTemp69 + 0.42755508f * fTemp812))));
			float fTemp821 = 0.70710677f * fTemp309;
			float fTemp822 = fTemp311 + fTemp316 - (fTemp314 + 0.70710677f * fTemp313);
			float fTemp823 = 0.70710677f * fTemp315;
			float fTemp824 = fTemp323 - (fTemp322 + fTemp325 + fTemp823);
			float fTemp825 = fTemp307 + fTemp320 - (fTemp821 + 0.38268343f * fTemp822 + fTemp318 + 0.9238795f * fTemp824);
			float fTemp826 = 0.8314696f * fTemp825;
			float fTemp827 = 0.70710677f * fTemp319;
			float fTemp828 = fTemp329 + 0.9238795f * fTemp822 - (fTemp328 + fTemp827 + fTemp332 + 0.38268343f * fTemp824);
			float fTemp829 = fTemp383 + fTemp388 - (fTemp386 + 0.70710677f * fTemp385);
			float fTemp830 = 0.9238795f * fTemp829;
			float fTemp831 = 0.70710677f * fTemp348;
			float fTemp832 = 0.70710677f * fTemp355;
			float fTemp833 = fTemp362 - (fTemp832 + fTemp364 + fTemp363);
			float fTemp834 = fTemp351 + fTemp356 - (fTemp354 + 0.70710677f * fTemp353);
			float fTemp835 = fTemp350 + fTemp359 - (fTemp831 + 0.9238795f * fTemp833 + fTemp360 + 0.38268343f * fTemp834);
			float fTemp836 = 0.70710677f * fTemp344;
			float fTemp837 = fTemp335 + fTemp340 - (fTemp338 + 0.70710677f * fTemp337);
			float fTemp838 = 0.38268343f * fTemp837;
			float fTemp839 = 0.70710677f * fTemp339;
			float fTemp840 = fTemp369 - (fTemp368 + fTemp371 + fTemp839);
			float fTemp841 = 0.9238795f * fTemp840;
			float fTemp842 = 0.70710677f * fTemp358;
			float fTemp843 = fTemp379 + 0.9238795f * fTemp834 - (fTemp377 + fTemp842 + fTemp380 + 0.38268343f * fTemp833);
			float fTemp844 = 0.8314696f * fTemp843;
			float fTemp845 = fTemp375 + fTemp346 + 0.55557024f * fTemp835 - (fTemp836 + fTemp838 + fTemp841 + fTemp376 + fTemp844);
			float fTemp846 = 0.9238795f * fTemp837;
			float fTemp847 = 0.8314696f * fTemp835;
			float fTemp848 = 0.70710677f * fTemp374;
			float fTemp849 = 0.38268343f * fTemp840;
			float fTemp850 = fTemp846 + fTemp847 + fTemp407 + 0.55557024f * fTemp843 - (fTemp405 + fTemp848 + fTemp409 + fTemp849);
			float fTemp851 = 0.70710677f * fTemp395;
			float fTemp852 = 0.70710677f * fTemp387;
			float fTemp853 = fTemp398 - (fTemp397 + fTemp400 + fTemp852);
			float fTemp854 = fTemp411 + 0.38268343f * fTemp853;
			float fTemp855 = 0.70710677f * fTemp392;
			float fTemp856 = 0.8314696f * fTemp828;
			float fTemp857 = 0.38268343f * fTemp829;
			float fTemp858 = 0.9238795f * fTemp853;
			output10[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp826 + 0.55557024f * fTemp828 + fTemp830 + 0.47139674f * fTemp845 + fTemp394 + 0.8819213f * fTemp850 - (fTemp393 + fTemp851 + fTemp854))) + fftdsp_faustpower2_f(std::fabs(fTemp417 + 0.55557024f * fTemp825 + fTemp413 + 0.8819213f * fTemp845 - (fTemp855 + fTemp856 + fTemp857 + fTemp858 + fTemp416 + 0.47139674f * fTemp850))));
			float fTemp859 = 0.9238795f * fTemp436;
			float fTemp860 = 0.38268343f * fTemp428;
			float fTemp861 = fTemp23 + 0.9238795f * fTemp432 - (fTemp431 + 0.38268343f * fTemp430);
			float fTemp862 = fTemp438 - (0.9238795f * fTemp430 + fTemp439 + 0.38268343f * fTemp432);
			float fTemp863 = fTemp66 + fTemp859 - (fTemp860 + fTemp434 + fTemp206 + 0.55557024f * fTemp861 + fTemp92 + 0.8314696f * fTemp862);
			float fTemp864 = 0.8819213f * fTemp863;
			float fTemp865 = fTemp64 + 0.9238795f * fTemp425 - (fTemp424 + 0.38268343f * fTemp423);
			float fTemp866 = fTemp62 + 0.8314696f * fTemp865;
			float fTemp867 = 0.9238795f * fTemp428;
			float fTemp868 = 0.38268343f * fTemp436;
			float fTemp869 = fTemp451 + fTemp60 + 0.8314696f * fTemp861 - (fTemp867 + fTemp868 + fTemp182 + fTemp41 + 0.55557024f * fTemp862);
			float fTemp870 = 0.9238795f * fTemp420;
			float fTemp871 = 0.38268343f * fTemp444;
			float fTemp872 = fTemp447 - (0.9238795f * fTemp423 + fTemp448 + 0.38268343f * fTemp425);
			float fTemp873 = fTemp43 + 0.55557024f * fTemp872;
			float fTemp874 = fTemp489 + fTemp864 + fTemp866 + 0.47139674f * fTemp869 - (fTemp870 + fTemp871 + fTemp199 + fTemp873);
			float fTemp875 = 0.9238795f * fTemp462;
			float fTemp876 = 0.38268343f * fTemp460;
			float fTemp877 = fTemp63 + 0.9238795f * fTemp458 - (fTemp457 + 0.38268343f * fTemp456);
			float fTemp878 = fTemp464 - (0.9238795f * fTemp456 + fTemp465 + 0.38268343f * fTemp458);
			float fTemp879 = fTemp67 + fTemp875 - (fTemp876 + fTemp484 + fTemp286 + 0.55557024f * fTemp877 + fTemp93 + 0.8314696f * fTemp878);
			float fTemp880 = 0.8819213f * fTemp879;
			float fTemp881 = 0.9238795f * fTemp460;
			float fTemp882 = 0.38268343f * fTemp462;
			float fTemp883 = fTemp461 + fTemp61 + 0.8314696f * fTemp877 - (fTemp881 + fTemp882 + fTemp238 + fTemp42 + 0.55557024f * fTemp878);
			float fTemp884 = fTemp65 + 0.9238795f * fTemp471 - (fTemp470 + 0.38268343f * fTemp469);
			float fTemp885 = fTemp0 + 0.8314696f * fTemp884;
			float fTemp886 = 0.9238795f * fTemp444;
			float fTemp887 = 0.38268343f * fTemp420;
			float fTemp888 = 0.55557024f * fTemp865 + fTemp94 + 0.8314696f * fTemp872;
			float fTemp889 = 0.8819213f * fTemp869;
			float fTemp890 = fTemp886 + fTemp68 + 0.47139674f * fTemp863 - (fTemp887 + fTemp442 + fTemp293 + fTemp888 + fTemp889);
			float fTemp891 = 0.9238795f * fTemp478;
			float fTemp892 = 0.38268343f * fTemp481;
			float fTemp893 = fTemp474 - (0.9238795f * fTemp469 + fTemp475 + 0.38268343f * fTemp471);
			float fTemp894 = fTemp44 + 0.55557024f * fTemp893;
			float fTemp895 = 0.9238795f * fTemp481;
			float fTemp896 = 0.38268343f * fTemp478;
			float fTemp897 = 0.55557024f * fTemp884 + fTemp95 + 0.8314696f * fTemp893;
			float fTemp898 = 0.8819213f * fTemp883;
			output11[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp480 + 0.8577286f * fTemp874 + fTemp880 + 0.47139674f * fTemp883 + fTemp885 + 0.51410276f * fTemp890 - (fTemp891 + fTemp892 + fTemp305 + fTemp894))) + fftdsp_faustpower2_f(std::fabs(fTemp895 + 0.47139674f * fTemp879 + fTemp69 + 0.8577286f * fTemp890 - (fTemp896 + fTemp497 + fTemp263 + fTemp897 + fTemp898 + 0.51410276f * fTemp874))));
			float fTemp899 = 0.70710677f * fTemp535;
			float fTemp900 = fTemp510 - (0.70710677f * fTemp508 + fTemp507 + fTemp522);
			float fTemp901 = 0.70710677f * fTemp504;
			float fTemp902 = fTemp521 + 0.70710677f * fTemp508 - (fTemp523 + fTemp513);
			float fTemp903 = fTemp515 + 0.38268343f * fTemp900 - (fTemp901 + fTemp548 + fTemp503 + 0.9238795f * fTemp902);
			float fTemp904 = fTemp530 - (0.70710677f * fTemp528 + fTemp527 + fTemp543);
			float fTemp905 = fTemp542 + 0.70710677f * fTemp528 - (fTemp544 + fTemp533);
			float fTemp906 = 0.70710677f * fTemp504;
			float fTemp907 = fTemp906 + 0.9238795f * fTemp900 + fTemp547 + 0.38268343f * fTemp902 - (fTemp550 + fTemp518);
			float fTemp908 = 0.70710677f * fTemp535;
			output12[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp899 + 0.55557024f * fTemp903 + 0.9238795f * fTemp904 + fTemp538 + 0.38268343f * fTemp905 + 0.8314696f * fTemp907 - (fTemp552 + fTemp557))) + fftdsp_faustpower2_f(std::fabs(fTemp556 + 0.38268343f * fTemp904 + 0.8314696f * fTemp903 - (fTemp908 + fTemp540 + fTemp554 + 0.9238795f * fTemp905 + 0.55557024f * fTemp907))));
			float fTemp909 = 0.38268343f * fTemp614 + fTemp93 + 0.9238795f * fTemp617;
			float fTemp910 = fTemp230 + 0.38268343f * fTemp609 + fTemp89 + 0.9238795f * fTemp611 - (fTemp63 + fTemp608);
			float fTemp911 = fTemp57 + 0.38268343f * fTemp611 - (fTemp244 + fTemp619 + fTemp38 + 0.9238795f * fTemp609);
			float fTemp912 = 0.55557024f * fTemp911;
			float fTemp913 = fTemp238 + fTemp909 - (fTemp613 + 0.8314696f * fTemp910 + fTemp67 + fTemp912);
			float fTemp914 = 0.95694035f * fTemp913;
			float fTemp915 = fTemp61 + 0.38268343f * fTemp617;
			float fTemp916 = 0.55557024f * fTemp910;
			float fTemp917 = fTemp42 + 0.9238795f * fTemp614;
			float fTemp918 = fTemp915 + fTemp916 - (fTemp285 + fTemp626 + fTemp917 + 0.8314696f * fTemp911);
			float fTemp919 = fTemp273 + 0.38268343f * fTemp598 + fTemp91 + 0.9238795f * fTemp600 - (fTemp65 + fTemp597);
			float fTemp920 = 0.55557024f * fTemp919;
			float fTemp921 = 0.38268343f * fTemp580;
			float fTemp922 = 0.9238795f * fTemp583;
			float fTemp923 = fTemp921 + fTemp94 + fTemp922;
			float fTemp924 = 0.38268343f * fTemp570 + fTemp92 + 0.9238795f * fTemp572;
			float fTemp925 = fTemp56 + 0.38268343f * fTemp567 - (fTemp188 + fTemp574 + fTemp37 + 0.9238795f * fTemp565);
			float fTemp926 = 0.55557024f * fTemp925;
			float fTemp927 = fTemp174 + 0.38268343f * fTemp565 + fTemp88 + 0.9238795f * fTemp567 - (fTemp23 + fTemp564);
			float fTemp928 = fTemp182 + fTemp924 - (fTemp569 + fTemp926 + fTemp66 + 0.8314696f * fTemp927);
			float fTemp929 = fTemp147 + 0.38268343f * fTemp559 + fTemp90 + 0.9238795f * fTemp561 - (fTemp64 + fTemp558);
			float fTemp930 = 0.8314696f * fTemp929;
			float fTemp931 = fTemp58 + 0.38268343f * fTemp561 - (fTemp200 + fTemp586 + fTemp39 + 0.9238795f * fTemp559);
			float fTemp932 = 0.55557024f * fTemp931;
			float fTemp933 = fTemp60 + 0.38268343f * fTemp572;
			float fTemp934 = 0.55557024f * fTemp927;
			float fTemp935 = fTemp41 + 0.9238795f * fTemp570;
			float fTemp936 = fTemp933 + fTemp934 - (fTemp205 + fTemp592 + fTemp935 + 0.8314696f * fTemp925);
			float fTemp937 = 0.95694035f * fTemp936;
			float fTemp938 = fTemp199 + fTemp923 + 0.29028466f * fTemp928 - (fTemp579 + fTemp930 + fTemp932 + fTemp68 + fTemp937);
			float fTemp939 = 0.38268343f * fTemp641;
			float fTemp940 = fTemp0 + fTemp939;
			float fTemp941 = 0.55557024f * fTemp929;
			float fTemp942 = 0.95694035f * fTemp928;
			float fTemp943 = 0.38268343f * fTemp583;
			float fTemp944 = fTemp62 + fTemp943;
			float fTemp945 = 0.9238795f * fTemp580;
			float fTemp946 = fTemp43 + fTemp945;
			float fTemp947 = 0.8314696f * fTemp931;
			float fTemp948 = fTemp941 + fTemp942 + fTemp944 + 0.29028466f * fTemp936 - (fTemp292 + fTemp635 + fTemp946 + fTemp947);
			float fTemp949 = 0.9238795f * fTemp606;
			float fTemp950 = fTemp59 + 0.38268343f * fTemp600 - (fTemp280 + fTemp603 + fTemp40 + 0.9238795f * fTemp598);
			float fTemp951 = fTemp44 + fTemp949 + 0.8314696f * fTemp950;
			float fTemp952 = 0.38268343f * fTemp606;
			float fTemp953 = 0.9238795f * fTemp641;
			float fTemp954 = fTemp952 + fTemp95 + fTemp953;
			float fTemp955 = 0.95694035f * fTemp918;
			float fTemp956 = 0.8314696f * fTemp919;
			float fTemp957 = 0.55557024f * fTemp950;
			output13[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp914 + 0.29028466f * fTemp918 + fTemp920 + 0.5956993f * fTemp938 + fTemp940 + 0.8032075f * fTemp948 - (fTemp261 + fTemp639 + fTemp951))) + fftdsp_faustpower2_f(std::fabs(fTemp305 + 0.29028466f * fTemp913 + fTemp954 + 0.8032075f * fTemp938 - (fTemp645 + fTemp955 + fTemp956 + fTemp957 + fTemp69 + 0.5956993f * fTemp948))));
			float fTemp958 = 0.70710677f * fTemp395;
			float fTemp959 = 0.70710677f * fTemp384 + fTemp386 + fTemp852 - fTemp383;
			float fTemp960 = 0.38268343f * fTemp959;
			float fTemp961 = 0.70710677f * fTemp343;
			float fTemp962 = 0.70710677f * fTemp347;
			float fTemp963 = 0.70710677f * fTemp352 + fTemp354 + fTemp832 - fTemp351;
			float fTemp964 = fTemp362 + 0.70710677f * fTemp355 - (fTemp364 + 0.70710677f * fTemp352);
			float fTemp965 = fTemp962 + fTemp360 + fTemp842 - (0.9238795f * fTemp963 + fTemp350 + 0.38268343f * fTemp964);
			float fTemp966 = 0.70710677f * fTemp336 + fTemp338 + fTemp839 - fTemp335;
			float fTemp967 = fTemp369 + 0.70710677f * fTemp339 - (fTemp371 + 0.70710677f * fTemp336);
			float fTemp968 = 0.9238795f * fTemp966 + fTemp346 + 0.38268343f * fTemp967;
			float fTemp969 = 0.70710677f * fTemp358;
			float fTemp970 = 0.70710677f * fTemp347;
			float fTemp971 = fTemp969 + fTemp379 + 0.38268343f * fTemp963 - (fTemp970 + fTemp380 + 0.9238795f * fTemp964);
			float fTemp972 = fTemp961 + fTemp848 + fTemp376 + 0.19509032f * fTemp965 - (fTemp968 + 0.98078525f * fTemp971);
			float fTemp973 = 0.70710677f * fTemp308;
			float fTemp974 = 0.70710677f * fTemp312 + fTemp314 + fTemp823 - fTemp311;
			float fTemp975 = fTemp323 + 0.70710677f * fTemp315 - (fTemp325 + 0.70710677f * fTemp312);
			float fTemp976 = fTemp973 + fTemp318 + fTemp827 - (0.9238795f * fTemp974 + fTemp307 + 0.38268343f * fTemp975);
			float fTemp977 = 0.70710677f * fTemp319;
			float fTemp978 = 0.70710677f * fTemp308;
			float fTemp979 = fTemp977 + fTemp329 + 0.38268343f * fTemp974 - (fTemp978 + fTemp332 + 0.9238795f * fTemp975);
			float fTemp980 = 0.70710677f * fTemp374;
			float fTemp981 = fTemp407 + 0.38268343f * fTemp966;
			float fTemp982 = 0.70710677f * fTemp343;
			float fTemp983 = fTemp409 + 0.9238795f * fTemp967;
			float fTemp984 = fTemp980 + 0.98078525f * fTemp965 + fTemp981 + 0.19509032f * fTemp971 - (fTemp982 + fTemp983);
			float fTemp985 = 0.70710677f * fTemp391;
			float fTemp986 = fTemp398 + 0.70710677f * fTemp387 - (fTemp400 + 0.70710677f * fTemp384);
			float fTemp987 = 0.9238795f * fTemp986;
			float fTemp988 = 0.70710677f * fTemp391;
			float fTemp989 = 0.9238795f * fTemp959;
			float fTemp990 = 0.38268343f * fTemp986;
			output14[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp958 + fTemp960 + 0.6343933f * fTemp972 + 0.98078525f * fTemp976 + fTemp394 + 0.19509032f * fTemp979 + 0.77301043f * fTemp984 - (fTemp985 + fTemp411 + fTemp987))) + fftdsp_faustpower2_f(std::fabs(fTemp988 + fTemp851 + fTemp416 + 0.19509032f * fTemp976 + 0.77301043f * fTemp972 - (fTemp989 + fTemp990 + fTemp413 + 0.98078525f * fTemp979 + 0.6343933f * fTemp984))));
			float fTemp991 = 0.9238795f * fTemp747;
			float fTemp992 = 0.9238795f * fTemp714;
			float fTemp993 = 0.38268343f * fTemp716;
			float fTemp994 = 0.9238795f * fTemp702;
			float fTemp995 = 0.38268343f * fTemp704;
			float fTemp996 = fTemp698 + fTemp574 + 0.9238795f * fTemp699 + fTemp23 + 0.38268343f * fTemp700 - fTemp88;
			float fTemp997 = fTemp706 + fTemp56 + 0.9238795f * fTemp700 - (fTemp707 + fTemp37 + 0.38268343f * fTemp699);
			float fTemp998 = fTemp697 + fTemp592 + fTemp994 + fTemp66 + fTemp995 - (0.98078525f * fTemp996 + fTemp92 + 0.19509032f * fTemp997);
			float fTemp999 = fTemp692 + fTemp586 + 0.9238795f * fTemp693 + fTemp64 + 0.38268343f * fTemp694 - fTemp90;
			float fTemp1000 = fTemp710 + fTemp58 + 0.9238795f * fTemp694 - (fTemp711 + fTemp39 + 0.38268343f * fTemp693);
			float fTemp1001 = 0.98078525f * fTemp999 + fTemp94 + 0.19509032f * fTemp1000;
			float fTemp1002 = 0.9238795f * fTemp704;
			float fTemp1003 = 0.38268343f * fTemp702;
			float fTemp1004 = fTemp718 + fTemp1002 + fTemp60 + 0.19509032f * fTemp996 - (fTemp720 + fTemp1003 + fTemp41 + 0.98078525f * fTemp997);
			float fTemp1005 = fTemp691 + fTemp635 + fTemp992 + fTemp993 + fTemp68 + 0.09801714f * fTemp998 - (fTemp1001 + 0.9951847f * fTemp1004);
			float fTemp1006 = 0.9238795f * fTemp684;
			float fTemp1007 = 0.38268343f * fTemp685;
			float fTemp1008 = fTemp680 + fTemp619 + 0.9238795f * fTemp681 + fTemp63 + 0.38268343f * fTemp682 - fTemp89;
			float fTemp1009 = fTemp687 + fTemp57 + 0.9238795f * fTemp682 - (fTemp688 + fTemp38 + 0.38268343f * fTemp681);
			float fTemp1010 = fTemp679 + fTemp626 + fTemp1006 + fTemp67 + fTemp1007 - (0.98078525f * fTemp1008 + fTemp93 + 0.19509032f * fTemp1009);
			float fTemp1011 = fTemp724 + fTemp603 + 0.9238795f * fTemp725 + fTemp65 + 0.38268343f * fTemp726 - fTemp91;
			float fTemp1012 = fTemp0 + 0.19509032f * fTemp1011;
			float fTemp1013 = 0.9238795f * fTemp685;
			float fTemp1014 = 0.38268343f * fTemp684;
			float fTemp1015 = fTemp734 + fTemp1013 + fTemp61 + 0.19509032f * fTemp1008 - (fTemp736 + fTemp1014 + fTemp42 + 0.98078525f * fTemp1009);
			float fTemp1016 = 0.9238795f * fTemp716;
			float fTemp1017 = fTemp62 + 0.19509032f * fTemp999;
			float fTemp1018 = 0.38268343f * fTemp714;
			float fTemp1019 = fTemp43 + 0.98078525f * fTemp1000;
			float fTemp1020 = fTemp739 + fTemp1016 + 0.9951847f * fTemp998 + fTemp1017 + 0.09801714f * fTemp1004 - (fTemp743 + fTemp1018 + fTemp1019);
			float fTemp1021 = 0.38268343f * fTemp728;
			float fTemp1022 = fTemp730 + fTemp59 + 0.9238795f * fTemp726 - (fTemp731 + fTemp40 + 0.38268343f * fTemp725);
			float fTemp1023 = fTemp44 + 0.98078525f * fTemp1022;
			float fTemp1024 = 0.9238795f * fTemp728;
			float fTemp1025 = 0.38268343f * fTemp747;
			float fTemp1026 = 0.98078525f * fTemp1011 + fTemp95 + 0.19509032f * fTemp1022;
			output15[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp678 + fTemp991 + 0.671559f * fTemp1005 + 0.9951847f * fTemp1010 + fTemp1012 + 0.09801714f * fTemp1015 + 0.7409511f * fTemp1020 - (fTemp746 + fTemp1021 + fTemp1023))) + fftdsp_faustpower2_f(std::fabs(fTemp750 + fTemp639 + fTemp1024 + fTemp1025 + fTemp69 + 0.09801714f * fTemp1010 + 0.7409511f * fTemp1005 - (fTemp1026 + 0.9951847f * fTemp1015 + 0.671559f * fTemp1020))));
			float fTemp1027 = fTemp3 + fTemp11 + fTemp19 + fTemp25 + fTemp33 + fTemp41 + fTemp52 + fTemp60 + fTemp66 + fTemp74 + fTemp82 + fTemp92 + fTemp100 + fTemp111 + fTemp126 + fTemp119 - (fTemp7 + fTemp15 + fTemp23 + fTemp29 + fTemp37 + fTemp45 + fTemp48 + fTemp56 + fTemp70 + fTemp78 + fTemp88 + fTemp96 + fTemp104 + fTemp107 + fTemp123 + fTemp115);
			float fTemp1028 = 0.0f - fTemp1027;
			float fTemp1029 = fTemp6 + fTemp14 + fTemp22 + fTemp28 + fTemp36 + fTemp44 + fTemp47 + fTemp55 + fTemp0 + fTemp69 + fTemp77 + fTemp87 + fTemp95 + fTemp103 + fTemp122 + fTemp114;
			float fTemp1030 = fTemp5 + fTemp13 + fTemp21 + fTemp27 + fTemp35 + fTemp43 + fTemp54 + fTemp62 + fTemp68 + fTemp76 + fTemp84 + fTemp86 + fTemp94 + fTemp102 + fTemp121 + fTemp113 - (fTemp1 + fTemp9 + fTemp17 + fTemp31 + fTemp39 + fTemp50 + fTemp58 + fTemp64 + fTemp72 + fTemp80 + fTemp90 + fTemp98 + fTemp106 + fTemp109 + fTemp125 + fTemp117);
			float fTemp1031 = 0.70710677f * fTemp1030;
			float fTemp1032 = fTemp2 + fTemp10 + fTemp18 + fTemp24 + fTemp32 + fTemp40 + fTemp51 + fTemp59 + fTemp65 + fTemp73 + fTemp81 + fTemp91 + fTemp99 + fTemp110 + fTemp127 + fTemp118;
			float fTemp1033 = fTemp8 + fTemp16 + fTemp30 + fTemp38 + fTemp46 + fTemp49 + fTemp57 + fTemp63 + fTemp71 + fTemp79 + fTemp89 + fTemp97 + fTemp105 + fTemp108 + fTemp124 + fTemp116;
			float fTemp1034 = fTemp4 + fTemp12 + fTemp20 + fTemp26 + fTemp34 + fTemp42 + fTemp53 + fTemp61 + fTemp67 + fTemp75 + fTemp83 + fTemp85 + fTemp93 + fTemp101 + fTemp120 + fTemp112;
			float fTemp1035 = 0.70710677f * fTemp1030;
			output16[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(0.70710677f * fTemp1028 + fTemp1029 + fTemp1031 - fTemp1032)) + fftdsp_faustpower2_f(std::fabs(fTemp1033 + 0.70710677f * fTemp1028 - (fTemp1034 + fTemp1035))));
			float fTemp1036 = 0.19509032f * fTemp203;
			float fTemp1037 = fTemp164 + 0.19509032f * fTemp191 - (fTemp187 + fTemp180);
			float fTemp1038 = fTemp207 - (0.19509032f * fTemp179 + fTemp41 + fTemp208);
			float fTemp1039 = 0.9951847f * fTemp1038;
			float fTemp1040 = fTemp130 + fTemp137 + fTemp1036 - (fTemp197 + fTemp199 + fTemp153 + 0.09801714f * fTemp1037 + fTemp68 + fTemp1039);
			float fTemp1041 = fTemp220 + 0.19509032f * fTemp247 - (fTemp243 + fTemp236);
			float fTemp1042 = 0.9951847f * fTemp1041;
			float fTemp1043 = 0.9951847f * fTemp1037;
			float fTemp1044 = 0.19509032f * fTemp152;
			float fTemp1045 = fTemp291 + fTemp292 + fTemp293 + fTemp294 + fTemp62 + fTemp1043 - (fTemp1044 + fTemp295 + fTemp43 + 0.09801714f * fTemp1038);
			float fTemp1046 = 0.19509032f * fTemp278;
			float fTemp1047 = fTemp287 - (0.19509032f * fTemp235 + fTemp42 + fTemp288);
			float fTemp1048 = 0.19509032f * fTemp283;
			float fTemp1049 = 0.9951847f * fTemp1047;
			output17[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp254 + fTemp258 + fTemp261 + fTemp263 + 0.7409511f * fTemp1040 + fTemp0 + fTemp1042 + 0.671559f * fTemp1045 - (fTemp1046 + fTemp284 + fTemp44 + 0.09801714f * fTemp1047))) + fftdsp_faustpower2_f(std::fabs(fTemp299 + fTemp300 + fTemp1048 + fTemp95 + 0.671559f * fTemp1040 - (fTemp304 + fTemp305 + fTemp301 + fTemp1049 + fTemp69 + 0.09801714f * fTemp1041 + 0.7409511f * fTemp1045))));
			float fTemp1050 = 0.38268343f * fTemp372;
			float fTemp1051 = 0.9238795f * fTemp341;
			float fTemp1052 = fTemp349 + fTemp350 + 0.38268343f * fTemp365 - (fTemp359 + fTemp360 + 0.9238795f * fTemp357);
			float fTemp1053 = fTemp377 + fTemp379 + fTemp378 - (0.38268343f * fTemp357 + fTemp380 + 0.9238795f * fTemp365);
			float fTemp1054 = 0.98078525f * fTemp1053;
			float fTemp1055 = fTemp346 + fTemp345 + fTemp1050 - (fTemp1051 + fTemp375 + 0.19509032f * fTemp1052 + fTemp376 + fTemp1054);
			float fTemp1056 = fTemp310 + 0.38268343f * fTemp326 - (fTemp321 + 0.9238795f * fTemp317);
			float fTemp1057 = 0.98078525f * fTemp1056;
			float fTemp1058 = 0.98078525f * fTemp1052;
			float fTemp1059 = 0.38268343f * fTemp341;
			float fTemp1060 = 0.9238795f * fTemp372;
			float fTemp1061 = fTemp405 + fTemp406 + fTemp407 + fTemp1058 - (fTemp1059 + fTemp1060 + fTemp409 + 0.19509032f * fTemp1053);
			float fTemp1062 = 0.38268343f * fTemp389;
			float fTemp1063 = 0.9238795f * fTemp401;
			float fTemp1064 = fTemp331 - (0.38268343f * fTemp317 + fTemp332 + 0.9238795f * fTemp326);
			float fTemp1065 = 0.38268343f * fTemp401;
			float fTemp1066 = 0.9238795f * fTemp389;
			float fTemp1067 = 0.98078525f * fTemp1064;
			output18[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp393 + fTemp396 + 0.77301043f * fTemp1055 + fTemp394 + fTemp1057 + 0.6343933f * fTemp1061 - (fTemp1062 + fTemp1063 + fTemp411 + 0.19509032f * fTemp1064))) + fftdsp_faustpower2_f(std::fabs(fTemp1065 + fTemp414 + fTemp413 + 0.6343933f * fTemp1055 - (fTemp1066 + fTemp417 + 0.19509032f * fTemp1056 + fTemp416 + fTemp1067 + 0.77301043f * fTemp1061))));
			float fTemp1068 = fTemp483 + 0.55557024f * fTemp466 - (fTemp486 + 0.8314696f * fTemp459);
			float fTemp1069 = 0.95694035f * fTemp1068;
			float fTemp1070 = 0.55557024f * fTemp449;
			float fTemp1071 = 0.8314696f * fTemp426;
			float fTemp1072 = fTemp429 + 0.55557024f * fTemp440 - (fTemp437 + 0.8314696f * fTemp433);
			float fTemp1073 = fTemp452 - (0.55557024f * fTemp433 + fTemp453 + 0.8314696f * fTemp440);
			float fTemp1074 = 0.95694035f * fTemp1073;
			float fTemp1075 = fTemp422 + fTemp1070 - (fTemp1071 + fTemp445 + fTemp442 + fTemp293 + 0.29028466f * fTemp1072 + fTemp94 + fTemp1074);
			float fTemp1076 = 0.95694035f * fTemp1072;
			float fTemp1077 = 0.55557024f * fTemp426;
			float fTemp1078 = 0.8314696f * fTemp449;
			float fTemp1079 = fTemp488 + fTemp490 + fTemp489 + fTemp62 + fTemp1076 - (fTemp1077 + fTemp1078 + fTemp199 + fTemp43 + 0.29028466f * fTemp1073);
			float fTemp1080 = 0.55557024f * fTemp472;
			float fTemp1081 = 0.8314696f * fTemp476;
			float fTemp1082 = fTemp463 - (0.55557024f * fTemp459 + fTemp467 + 0.8314696f * fTemp466);
			float fTemp1083 = 0.55557024f * fTemp476;
			float fTemp1084 = 0.8314696f * fTemp472;
			float fTemp1085 = 0.95694035f * fTemp1082;
			output19[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp479 + fTemp482 + fTemp480 + fTemp1069 + 0.8032075f * fTemp1075 + fTemp0 + 0.5956993f * fTemp1079 - (fTemp1080 + fTemp1081 + fTemp305 + fTemp44 + 0.29028466f * fTemp1082))) + fftdsp_faustpower2_f(std::fabs(fTemp1083 + fTemp495 + fTemp69 + 0.5956993f * fTemp1075 - (fTemp1084 + fTemp499 + fTemp497 + fTemp263 + 0.29028466f * fTemp1068 + fTemp1085 + fTemp95 + 0.8032075f * fTemp1079))));
			float fTemp1086 = 0.70710677f * fTemp505;
			float fTemp1087 = fTemp507 + fTemp513 - (fTemp510 + 0.70710677f * fTemp509);
			float fTemp1088 = 0.70710677f * fTemp512;
			float fTemp1089 = fTemp521 - (fTemp520 + fTemp523 + fTemp1088);
			float fTemp1090 = fTemp503 + fTemp518 - (fTemp1086 + 0.38268343f * fTemp1087 + fTemp515 + 0.9238795f * fTemp1089);
			float fTemp1091 = fTemp527 + fTemp533 - (fTemp530 + 0.70710677f * fTemp529);
			float fTemp1092 = 0.70710677f * fTemp517;
			float fTemp1093 = fTemp547 + 0.9238795f * fTemp1087 - (fTemp546 + fTemp1092 + fTemp550 + 0.38268343f * fTemp1089);
			float fTemp1094 = 0.70710677f * fTemp539;
			float fTemp1095 = 0.70710677f * fTemp532;
			float fTemp1096 = fTemp542 - (fTemp541 + fTemp544 + fTemp1095);
			float fTemp1097 = 0.70710677f * fTemp536;
			output20[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(0.8314696f * fTemp1090 + fTemp538 + 0.9238795f * fTemp1091 + 0.55557024f * fTemp1093 - (fTemp537 + fTemp1094 + fTemp552 + 0.38268343f * fTemp1096))) + fftdsp_faustpower2_f(std::fabs(fTemp557 + fTemp554 + 0.55557024f * fTemp1090 - (fTemp1097 + 0.38268343f * fTemp1091 + fTemp556 + 0.9238795f * fTemp1096 + 0.8314696f * fTemp1093))));
			float fTemp1098 = 0.55557024f * fTemp562;
			float fTemp1099 = fTemp182 + fTemp92 + fTemp576 - (fTemp569 + fTemp573 + 0.55557024f * fTemp568);
			float fTemp1100 = fTemp591 - (fTemp205 + fTemp592 + fTemp590 + fTemp593 + 0.55557024f * fTemp575);
			float fTemp1101 = 0.8819213f * fTemp1100;
			float fTemp1102 = fTemp199 + fTemp94 + fTemp588 - (fTemp579 + fTemp1098 + fTemp581 + fTemp584 + 0.47139674f * fTemp1099 + fTemp68 + fTemp1101);
			float fTemp1103 = fTemp238 + fTemp93 + fTemp621 - (fTemp613 + fTemp615 + fTemp618 + fTemp67 + 0.55557024f * fTemp612);
			float fTemp1104 = 0.8819213f * fTemp1103;
			float fTemp1105 = 0.8819213f * fTemp1099;
			float fTemp1106 = 0.55557024f * fTemp587;
			float fTemp1107 = fTemp632 + fTemp62 + fTemp1105 - (fTemp292 + fTemp635 + fTemp631 + fTemp1106 + fTemp636 + fTemp43 + 0.47139674f * fTemp1100);
			float fTemp1108 = 0.55557024f * fTemp604;
			float fTemp1109 = fTemp61 + fTemp624 - (fTemp285 + fTemp626 + fTemp627 + fTemp625 + fTemp42 + 0.55557024f * fTemp620);
			float fTemp1110 = 0.55557024f * fTemp601;
			float fTemp1111 = 0.8819213f * fTemp1109;
			output21[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp607 + 0.8577286f * fTemp1102 + fTemp0 + fTemp1104 + 0.51410276f * fTemp1107 - (fTemp261 + fTemp639 + fTemp602 + fTemp1108 + fTemp642 + fTemp44 + 0.47139674f * fTemp1109))) + fftdsp_faustpower2_f(std::fabs(fTemp305 + fTemp646 + fTemp95 + 0.51410276f * fTemp1102 - (fTemp645 + fTemp1110 + fTemp647 + fTemp648 + 0.47139674f * fTemp1103 + fTemp69 + fTemp1111 + 0.8577286f * fTemp1107))));
			float fTemp1112 = 0.9238795f * fTemp658;
			float fTemp1113 = 0.38268343f * fTemp651;
			float fTemp1114 = fTemp360 + 0.9238795f * fTemp655 - (fTemp654 + fTemp378 + fTemp350 + 0.38268343f * fTemp653);
			float fTemp1115 = fTemp379 + fTemp660 - (fTemp359 + 0.9238795f * fTemp653 + fTemp380 + 0.38268343f * fTemp655);
			float fTemp1116 = fTemp376 + fTemp1112 - (fTemp657 + fTemp406 + fTemp1113 + 0.55557024f * fTemp1114 + fTemp346 + 0.8314696f * fTemp1115);
			float fTemp1117 = fTemp318 + 0.9238795f * fTemp665 - (fTemp664 + fTemp330 + fTemp307 + 0.38268343f * fTemp663);
			float fTemp1118 = 0.9238795f * fTemp651;
			float fTemp1119 = 0.38268343f * fTemp658;
			float fTemp1120 = fTemp672 + fTemp407 + 0.8314696f * fTemp1114 - (fTemp375 + fTemp1118 + fTemp1119 + fTemp409 + 0.55557024f * fTemp1115);
			float fTemp1121 = 0.9238795f * fTemp667;
			float fTemp1122 = 0.38268343f * fTemp668;
			float fTemp1123 = fTemp329 + fTemp670 - (fTemp320 + 0.9238795f * fTemp663 + fTemp332 + 0.38268343f * fTemp665);
			float fTemp1124 = 0.9238795f * fTemp668;
			float fTemp1125 = 0.38268343f * fTemp667;
			output22[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp650 + 0.8819213f * fTemp1116 + fTemp394 + 0.8314696f * fTemp1117 + 0.47139674f * fTemp1120 - (fTemp417 + fTemp1121 + fTemp1122 + fTemp411 + 0.55557024f * fTemp1123))) + fftdsp_faustpower2_f(std::fabs(fTemp1124 + fTemp416 + 0.47139674f * fTemp1116 - (fTemp676 + fTemp396 + fTemp1125 + 0.55557024f * fTemp1117 + fTemp413 + 0.8314696f * fTemp1123 + 0.8819213f * fTemp1120))));
			float fTemp1126 = 0.98078525f * fTemp712;
			float fTemp1127 = 0.19509032f * fTemp695;
			float fTemp1128 = fTemp697 + fTemp592 + fTemp66 + 0.98078525f * fTemp708 - (fTemp703 + fTemp705 + fTemp92 + 0.19509032f * fTemp701);
			float fTemp1129 = fTemp718 + fTemp60 + fTemp719 - (fTemp720 + 0.19509032f * fTemp708 + fTemp721 + fTemp41 + 0.98078525f * fTemp701);
			float fTemp1130 = fTemp691 + fTemp635 + fTemp68 + fTemp1126 - (fTemp1127 + fTemp715 + fTemp717 + 0.6343933f * fTemp1128 + fTemp94 + 0.77301043f * fTemp1129);
			float fTemp1131 = fTemp679 + fTemp626 + fTemp67 + 0.98078525f * fTemp689 - (fTemp686 + 0.19509032f * fTemp683);
			float fTemp1132 = 0.98078525f * fTemp695;
			float fTemp1133 = 0.19509032f * fTemp712;
			float fTemp1134 = fTemp739 + fTemp742 + fTemp62 + 0.77301043f * fTemp1128 - (fTemp743 + fTemp1132 + fTemp1133 + fTemp744 + fTemp43 + 0.6343933f * fTemp1129);
			float fTemp1135 = 0.98078525f * fTemp727;
			float fTemp1136 = 0.19509032f * fTemp732;
			float fTemp1137 = fTemp734 + fTemp735 - (fTemp736 + 0.98078525f * fTemp683 + fTemp737 + 0.19509032f * fTemp689);
			float fTemp1138 = 0.98078525f * fTemp732;
			float fTemp1139 = 0.19509032f * fTemp727;
			output23[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp678 + fTemp729 + 0.9039893f * fTemp1130 + fTemp0 + 0.77301043f * fTemp1131 + 0.42755508f * fTemp1134 - (fTemp746 + fTemp1135 + fTemp1136 + fTemp748 + fTemp44 + 0.6343933f * fTemp1137))) + fftdsp_faustpower2_f(std::fabs(fTemp750 + fTemp639 + fTemp1138 + fTemp69 + 0.42755508f * fTemp1130 - (fTemp1139 + fTemp752 + fTemp753 + 0.6343933f * fTemp1131 + fTemp95 + 0.77301043f * fTemp1137 + 0.9039893f * fTemp1134))));
			float fTemp1140 = fTemp770 + 0.70710677f * fTemp757 - (fTemp772 + fTemp761);
			float fTemp1141 = fTemp759 - (0.70710677f * fTemp757 + fTemp756 + fTemp771);
			output24[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(0.70710677f * fTemp763 + 0.38268343f * fTemp1140 + fTemp766 + 0.9238795f * fTemp1141 - (fTemp774 + fTemp777))) + fftdsp_faustpower2_f(std::fabs(fTemp776 + 0.38268343f * fTemp1141 - (0.70710677f * fTemp763 + fTemp768 + fTemp775 + 0.9238795f * fTemp1140))));
			float fTemp1142 = 0.19509032f * fTemp815;
			float fTemp1143 = 0.19509032f * fTemp798;
			float fTemp1144 = fTemp791 + fTemp157 + 0.19509032f * fTemp793 + fTemp92 + fTemp795 - (fTemp183 + fTemp792);
			float fTemp1145 = fTemp205 + fTemp206 + fTemp60 + 0.19509032f * fTemp794 - (fTemp803 + fTemp804 + fTemp41 + fTemp802);
			float fTemp1146 = fTemp790 + fTemp130 + fTemp1143 + fTemp94 + fTemp800 - (fTemp199 + fTemp797 + 0.77301043f * fTemp1144 + fTemp68 + 0.6343933f * fTemp1145);
			float fTemp1147 = fTemp778 + fTemp213 + 0.19509032f * fTemp780 + fTemp93 + fTemp782 - (fTemp239 + fTemp779);
			float fTemp1148 = 0.19509032f * fTemp799;
			float fTemp1149 = fTemp292 + fTemp293 + fTemp1148 + fTemp62 + 0.6343933f * fTemp1144 - (fTemp809 + fTemp810 + fTemp807 + fTemp43 + 0.77301043f * fTemp1145);
			float fTemp1150 = fTemp285 + fTemp286 + fTemp61 + 0.19509032f * fTemp781 - (fTemp785 + fTemp786 + fTemp42 + fTemp784);
			float fTemp1151 = 0.19509032f * fTemp788;
			output25[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp261 + fTemp263 + fTemp1142 + 0.94154406f * fTemp1146 + fTemp0 + 0.6343933f * fTemp1147 + 0.33688986f * fTemp1149 - (fTemp813 + fTemp814 + fTemp789 + fTemp44 + 0.77301043f * fTemp1150))) + fftdsp_faustpower2_f(std::fabs(fTemp817 + fTemp300 + fTemp1151 + fTemp820 + fTemp95 + 0.33688986f * fTemp1146 - (fTemp818 + fTemp305 + 0.77301043f * fTemp1147 + fTemp69 + 0.6343933f * fTemp1150 + 0.94154406f * fTemp1149))));
			float fTemp1152 = 0.38268343f * fTemp853;
			float fTemp1153 = 0.38268343f * fTemp837;
			float fTemp1154 = 0.9238795f * fTemp840;
			float fTemp1155 = fTemp359 + 0.9238795f * fTemp833 + fTemp350 + 0.38268343f * fTemp834 - (fTemp360 + fTemp831);
			float fTemp1156 = fTemp379 + 0.38268343f * fTemp833 - (fTemp377 + fTemp842 + fTemp380 + 0.9238795f * fTemp834);
			float fTemp1157 = 0.55557024f * fTemp1156;
			float fTemp1158 = fTemp375 + fTemp1153 + fTemp346 + fTemp1154 - (fTemp836 + 0.8314696f * fTemp1155 + fTemp376 + fTemp1157);
			float fTemp1159 = fTemp320 + 0.38268343f * fTemp822 + fTemp307 + 0.9238795f * fTemp824 - (fTemp318 + fTemp821);
			float fTemp1160 = 0.55557024f * fTemp1159;
			float fTemp1161 = 0.38268343f * fTemp840;
			float fTemp1162 = 0.55557024f * fTemp1155;
			float fTemp1163 = 0.9238795f * fTemp837;
			float fTemp1164 = fTemp1161 + fTemp407 + fTemp1162 - (fTemp405 + fTemp848 + fTemp1163 + fTemp409 + 0.8314696f * fTemp1156);
			float fTemp1165 = 0.9238795f * fTemp829;
			float fTemp1166 = fTemp329 + 0.38268343f * fTemp824 - (fTemp328 + fTemp827 + fTemp332 + 0.9238795f * fTemp822);
			float fTemp1167 = 0.38268343f * fTemp829;
			float fTemp1168 = 0.9238795f * fTemp853;
			float fTemp1169 = 0.55557024f * fTemp1166;
			output26[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp1152 + 0.95694035f * fTemp1158 + fTemp394 + fTemp1160 + 0.29028466f * fTemp1164 - (fTemp393 + fTemp851 + fTemp1165 + fTemp411 + 0.8314696f * fTemp1166))) + fftdsp_faustpower2_f(std::fabs(fTemp417 + fTemp1167 + fTemp1168 + fTemp413 + 0.29028466f * fTemp1158 - (fTemp855 + 0.8314696f * fTemp1159 + fTemp416 + fTemp1169 + 0.95694035f * fTemp1164))));
			float fTemp1170 = 0.55557024f * fTemp893;
			float fTemp1171 = fTemp875 + 0.55557024f * fTemp877 + fTemp67 + 0.8314696f * fTemp878 - (fTemp485 + fTemp876);
			float fTemp1172 = 0.55557024f * fTemp865;
			float fTemp1173 = 0.8314696f * fTemp872;
			float fTemp1174 = fTemp859 + 0.55557024f * fTemp861 + fTemp66 + 0.8314696f * fTemp862 - (fTemp435 + fTemp860);
			float fTemp1175 = fTemp451 + fTemp60 + 0.55557024f * fTemp862 - (fTemp867 + fTemp868 + fTemp182 + fTemp41 + 0.8314696f * fTemp861);
			float fTemp1176 = fTemp886 + fTemp1172 + fTemp68 + fTemp1173 - (fTemp887 + fTemp442 + fTemp293 + 0.8819213f * fTemp1174 + fTemp94 + 0.47139674f * fTemp1175);
			float fTemp1177 = 0.55557024f * fTemp872;
			float fTemp1178 = 0.8314696f * fTemp865;
			float fTemp1179 = fTemp489 + fTemp1177 + fTemp62 + 0.47139674f * fTemp1174 - (fTemp870 + fTemp871 + fTemp199 + fTemp1178 + fTemp43 + 0.8819213f * fTemp1175);
			float fTemp1180 = 0.8314696f * fTemp884;
			float fTemp1181 = fTemp461 + fTemp61 + 0.55557024f * fTemp878 - (fTemp881 + fTemp882 + fTemp238 + fTemp42 + 0.8314696f * fTemp877);
			float fTemp1182 = 0.55557024f * fTemp884;
			float fTemp1183 = 0.8314696f * fTemp893;
			output27[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp480 + fTemp1170 + 0.47139674f * fTemp1171 + 0.97003126f * fTemp1176 + fTemp0 + 0.24298018f * fTemp1179 - (fTemp891 + fTemp892 + fTemp305 + fTemp1180 + fTemp44 + 0.8819213f * fTemp1181))) + fftdsp_faustpower2_f(std::fabs(fTemp895 + fTemp1182 + fTemp1183 + fTemp69 + 0.24298018f * fTemp1176 - (fTemp896 + fTemp497 + fTemp263 + 0.8819213f * fTemp1171 + 0.47139674f * fTemp1181 + fTemp95 + 0.97003126f * fTemp1179))));
			float fTemp1184 = 0.70710677f * fTemp539;
			float fTemp1185 = 0.70710677f * fTemp504;
			float fTemp1186 = 0.70710677f * fTemp508 + fTemp510 + fTemp1088 - fTemp507;
			float fTemp1187 = fTemp521 + 0.70710677f * fTemp512 - (fTemp523 + 0.70710677f * fTemp508);
			float fTemp1188 = fTemp1185 + fTemp515 + fTemp1092 - (0.9238795f * fTemp1186 + fTemp503 + 0.38268343f * fTemp1187);
			float fTemp1189 = 0.70710677f * fTemp528 + fTemp530 + fTemp1095 - fTemp527;
			float fTemp1190 = 0.70710677f * fTemp517;
			float fTemp1191 = 0.70710677f * fTemp504;
			float fTemp1192 = fTemp1190 + fTemp547 + 0.38268343f * fTemp1186 - (fTemp1191 + fTemp550 + 0.9238795f * fTemp1187);
			float fTemp1193 = 0.70710677f * fTemp535;
			float fTemp1194 = fTemp542 + 0.70710677f * fTemp532 - (fTemp544 + 0.70710677f * fTemp528);
			float fTemp1195 = 0.70710677f * fTemp535;
			output28[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp1184 + 0.98078525f * fTemp1188 + fTemp538 + 0.38268343f * fTemp1189 + 0.19509032f * fTemp1192 - (fTemp1193 + fTemp552 + 0.9238795f * fTemp1194))) + fftdsp_faustpower2_f(std::fabs(fTemp1195 + fTemp1094 + fTemp556 + 0.19509032f * fTemp1188 - (0.9238795f * fTemp1189 + fTemp554 + 0.38268343f * fTemp1194 + 0.98078525f * fTemp1192))));
			float fTemp1196 = 0.8314696f * fTemp950;
			float fTemp1197 = 0.8314696f * fTemp929;
			float fTemp1198 = fTemp182 + fTemp926 + fTemp924 + 0.8314696f * fTemp927 - (fTemp66 + fTemp569);
			float fTemp1199 = 0.95694035f * fTemp1198;
			float fTemp1200 = fTemp933 + 0.8314696f * fTemp925 - (fTemp205 + fTemp592 + fTemp935 + fTemp934);
			float fTemp1201 = fTemp199 + fTemp1197 + fTemp923 + fTemp932 - (fTemp579 + fTemp1199 + fTemp68 + 0.29028466f * fTemp1200);
			float fTemp1202 = fTemp238 + 0.8314696f * fTemp910 + fTemp909 + fTemp912 - (fTemp67 + fTemp613);
			float fTemp1203 = 0.8314696f * fTemp931;
			float fTemp1204 = 0.95694035f * fTemp1200;
			float fTemp1205 = fTemp1203 + fTemp943 + fTemp62 + 0.29028466f * fTemp1198 - (fTemp292 + fTemp635 + fTemp941 + fTemp945 + fTemp43 + fTemp1204);
			float fTemp1206 = fTemp915 + 0.8314696f * fTemp911 - (fTemp285 + fTemp626 + fTemp917 + fTemp916);
			float fTemp1207 = 0.95694035f * fTemp1206;
			float fTemp1208 = 0.8314696f * fTemp919;
			float fTemp1209 = 0.95694035f * fTemp1202;
			output29[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp1196 + fTemp939 + 0.9891765f * fTemp1201 + fTemp0 + 0.29028466f * fTemp1202 + 0.14673047f * fTemp1205 - (fTemp261 + fTemp639 + fTemp920 + fTemp949 + fTemp44 + fTemp1207))) + fftdsp_faustpower2_f(std::fabs(fTemp305 + fTemp1208 + fTemp957 + fTemp952 + fTemp953 + fTemp95 + 0.14673047f * fTemp1201 - (fTemp645 + fTemp1209 + fTemp69 + 0.29028466f * fTemp1206 + 0.9891765f * fTemp1205))));
			float fTemp1210 = 0.9238795f * fTemp986;
			float fTemp1211 = 0.9238795f * fTemp966 + fTemp376 + 0.38268343f * fTemp967;
			float fTemp1212 = fTemp962 + fTemp842 + 0.9238795f * fTemp963 + fTemp360 + 0.38268343f * fTemp964 - fTemp350;
			float fTemp1213 = fTemp969 + fTemp379 + 0.9238795f * fTemp964 - (fTemp970 + fTemp380 + 0.38268343f * fTemp963);
			float fTemp1214 = fTemp961 + fTemp848 + fTemp1211 - (0.98078525f * fTemp1212 + fTemp346 + 0.19509032f * fTemp1213);
			float fTemp1215 = fTemp973 + fTemp827 + 0.9238795f * fTemp974 + fTemp318 + 0.38268343f * fTemp975 - fTemp307;
			float fTemp1216 = fTemp407 + 0.9238795f * fTemp967;
			float fTemp1217 = fTemp409 + 0.38268343f * fTemp966;
			float fTemp1218 = fTemp980 + fTemp1216 + 0.19509032f * fTemp1212 - (fTemp982 + fTemp1217 + 0.98078525f * fTemp1213);
			float fTemp1219 = 0.38268343f * fTemp959;
			float fTemp1220 = fTemp977 + fTemp329 + 0.9238795f * fTemp975 - (fTemp978 + fTemp332 + 0.38268343f * fTemp974);
			float fTemp1221 = 0.9238795f * fTemp959;
			float fTemp1222 = 0.38268343f * fTemp986;
			output30[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp958 + fTemp1210 + 0.9951847f * fTemp1214 + fTemp394 + 0.19509032f * fTemp1215 + 0.09801714f * fTemp1218 - (fTemp985 + fTemp1219 + fTemp411 + 0.98078525f * fTemp1220))) + fftdsp_faustpower2_f(std::fabs(fTemp988 + fTemp851 + fTemp1221 + fTemp1222 + fTemp416 + 0.09801714f * fTemp1214 - (0.98078525f * fTemp1215 + fTemp413 + 0.19509032f * fTemp1220 + 0.9951847f * fTemp1218))));
			float fTemp1223 = 0.98078525f * fTemp999 + fTemp68 + 0.19509032f * fTemp1000;
			float fTemp1224 = fTemp697 + fTemp592 + fTemp994 + fTemp995 + 0.98078525f * fTemp996 + fTemp66 + 0.19509032f * fTemp997 - fTemp92;
			float fTemp1225 = fTemp718 + fTemp1002 + fTemp60 + 0.98078525f * fTemp997 - (fTemp720 + fTemp1003 + fTemp41 + 0.19509032f * fTemp996);
			float fTemp1226 = fTemp691 + fTemp635 + fTemp992 + fTemp993 + fTemp1223 - (0.9951847f * fTemp1224 + fTemp94 + 0.09801714f * fTemp1225);
			float fTemp1227 = fTemp0 + 0.98078525f * fTemp1022;
			float fTemp1228 = fTemp679 + fTemp626 + fTemp1006 + fTemp1007 + 0.98078525f * fTemp1008 + fTemp67 + 0.19509032f * fTemp1009 - fTemp93;
			float fTemp1229 = fTemp62 + 0.98078525f * fTemp1000;
			float fTemp1230 = fTemp43 + 0.19509032f * fTemp999;
			float fTemp1231 = fTemp739 + fTemp1016 + fTemp1229 + 0.09801714f * fTemp1224 - (fTemp743 + fTemp1018 + fTemp1230 + 0.9951847f * fTemp1225);
			float fTemp1232 = fTemp44 + 0.19509032f * fTemp1011;
			float fTemp1233 = fTemp734 + fTemp1013 + fTemp61 + 0.98078525f * fTemp1009 - (fTemp736 + fTemp1014 + fTemp42 + 0.19509032f * fTemp1008);
			float fTemp1234 = 0.98078525f * fTemp1011 + fTemp69 + 0.19509032f * fTemp1022;
			output31[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp678 + fTemp991 + 0.99879545f * fTemp1226 + fTemp1227 + 0.09801714f * fTemp1228 + 0.049067676f * fTemp1231 - (fTemp746 + fTemp1021 + fTemp1232 + 0.9951847f * fTemp1233))) + fftdsp_faustpower2_f(std::fabs(fTemp750 + fTemp639 + fTemp1024 + fTemp1025 + fTemp1234 + 0.049067676f * fTemp1226 - (0.9951847f * fTemp1228 + fTemp95 + 0.09801714f * fTemp1233 + 0.99879545f * fTemp1231))));
			output32[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp2 + fTemp6 + fTemp10 + fTemp14 + fTemp18 + fTemp22 + fTemp24 + fTemp28 + fTemp32 + fTemp36 + fTemp40 + fTemp44 + fTemp47 + fTemp51 + fTemp55 + fTemp59 + fTemp0 + fTemp65 + fTemp69 + fTemp73 + fTemp77 + fTemp81 + fTemp87 + fTemp91 + fTemp95 + fTemp99 + fTemp103 + fTemp110 + fTemp114 + fTemp118 + fTemp127 + fTemp122 - (fTemp4 + fTemp8 + fTemp12 + fTemp16 + fTemp20 + fTemp26 + fTemp30 + fTemp34 + fTemp38 + fTemp42 + fTemp46 + fTemp49 + fTemp53 + fTemp57 + fTemp61 + fTemp63 + fTemp67 + fTemp71 + fTemp75 + fTemp79 + fTemp83 + fTemp85 + fTemp89 + fTemp93 + fTemp97 + fTemp101 + fTemp105 + fTemp108 + fTemp112 + fTemp116 + fTemp124 + fTemp120))) + fftdsp_faustpower2_f(std::fabs(0.0f - (fTemp1 + fTemp5 + fTemp9 + fTemp13 + fTemp17 + fTemp21 + fTemp27 + fTemp31 + fTemp35 + fTemp39 + fTemp43 + fTemp50 + fTemp54 + fTemp58 + fTemp62 + fTemp64 + fTemp68 + fTemp72 + fTemp76 + fTemp80 + fTemp84 + fTemp86 + fTemp90 + fTemp94 + fTemp98 + fTemp102 + fTemp106 + fTemp109 + fTemp113 + fTemp117 + fTemp125 + fTemp121 - (fTemp3 + fTemp7 + fTemp11 + fTemp15 + fTemp19 + fTemp23 + fTemp25 + fTemp29 + fTemp33 + fTemp37 + fTemp41 + fTemp45 + fTemp48 + fTemp52 + fTemp56 + fTemp60 + fTemp66 + fTemp70 + fTemp74 + fTemp78 + fTemp82 + fTemp88 + fTemp92 + fTemp96 + fTemp100 + fTemp104 + fTemp107 + fTemp111 + fTemp115 + fTemp119 + fTemp126 + fTemp123)))));
			float fTemp1235 = fTemp154 + 0.09801714f * fTemp209 - (fTemp204 + fTemp193);
			float fTemp1236 = fTemp296 - (0.09801714f * fTemp192 + fTemp43 + fTemp297);
			output33[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp254 + fTemp258 + fTemp261 + fTemp263 + fTemp279 + fTemp284 + fTemp0 + 0.99879545f * fTemp1235 - (0.09801714f * fTemp248 + fTemp290 + fTemp44 + 0.049067676f * fTemp1236))) + fftdsp_faustpower2_f(std::fabs(fTemp302 + 0.09801714f * fTemp289 - (fTemp303 + fTemp304 + fTemp305 + fTemp306 + 0.049067676f * fTemp1235 + fTemp69 + 0.99879545f * fTemp1236))));
			float fTemp1237 = fTemp342 + fTemp345 + fTemp346 + 0.19509032f * fTemp381 - (fTemp373 + fTemp375 + fTemp376 + fTemp367);
			float fTemp1238 = fTemp403 + fTemp405 + fTemp407 + fTemp406 + fTemp404 - (0.19509032f * fTemp366 + fTemp409 + fTemp408);
			output34[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp390 + fTemp402 + fTemp393 + fTemp396 + fTemp394 + 0.9951847f * fTemp1237 - (0.19509032f * fTemp327 + fTemp334 + fTemp411 + 0.09801714f * fTemp1238))) + fftdsp_faustpower2_f(std::fabs(fTemp415 + 0.19509032f * fTemp333 - (fTemp412 + fTemp419 + fTemp417 + 0.09801714f * fTemp1237 + fTemp416 + 0.9951847f * fTemp1238))));
			float fTemp1239 = fTemp427 + 0.29028466f * fTemp454 - (fTemp450 + 0.95694035f * fTemp441);
			float fTemp1240 = fTemp491 - (0.29028466f * fTemp441 + fTemp492 + 0.95694035f * fTemp454);
			output35[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp473 + fTemp477 + fTemp479 + fTemp482 + fTemp480 + fTemp0 + 0.9891765f * fTemp1239 - (0.95694035f * fTemp468 + 0.29028466f * fTemp487 + fTemp305 + fTemp44 + 0.14673047f * fTemp1240))) + fftdsp_faustpower2_f(std::fabs(fTemp496 + fTemp494 + 0.29028466f * fTemp468 - (fTemp501 + 0.95694035f * fTemp487 + fTemp499 + fTemp497 + fTemp263 + 0.14673047f * fTemp1239 + fTemp95 + 0.9891765f * fTemp1240))));
			float fTemp1241 = fTemp506 + 0.38268343f * fTemp524 - (fTemp519 + 0.9238795f * fTemp514);
			float fTemp1242 = fTemp549 - (0.38268343f * fTemp514 + fTemp550 + 0.9238795f * fTemp524);
			output36[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp537 + fTemp540 + fTemp538 + 0.98078525f * fTemp1241 - (0.38268343f * fTemp534 + 0.9238795f * fTemp545 + fTemp552 + 0.19509032f * fTemp1242))) + fftdsp_faustpower2_f(std::fabs(fTemp555 + 0.38268343f * fTemp545 - (0.9238795f * fTemp534 + fTemp557 + 0.19509032f * fTemp1241 + fTemp556 + 0.98078525f * fTemp1242))));
			float fTemp1243 = fTemp199 + fTemp563 + fTemp595 - (fTemp579 + fTemp589 + fTemp578);
			float fTemp1244 = fTemp633 - (fTemp292 + fTemp635 + fTemp630 + fTemp637 + fTemp634);
			output37[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp602 + fTemp0 + fTemp607 + fTemp605 + 0.97003126f * fTemp1243 - (fTemp261 + fTemp639 + fTemp642 + fTemp623 + fTemp44 + fTemp629 + 0.24298018f * fTemp1244))) + fftdsp_faustpower2_f(std::fabs(fTemp305 + fTemp643 + fTemp95 + fTemp649 - (0.24298018f * fTemp1243 + fTemp645 + fTemp646 + fTemp647 + fTemp648 + fTemp69 + fTemp644 + 0.97003126f * fTemp1244))));
			float fTemp1245 = fTemp652 + 0.55557024f * fTemp661 - (fTemp657 + fTemp406 + fTemp659 + 0.8314696f * fTemp656);
			float fTemp1246 = fTemp672 + fTemp673 - (fTemp375 + 0.55557024f * fTemp656 + fTemp409 + 0.8314696f * fTemp661);
			output38[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp650 + fTemp669 + 0.95694035f * fTemp1245 - (fTemp417 + 0.55557024f * fTemp666 + fTemp411 + 0.8314696f * fTemp671 + 0.29028466f * fTemp1246))) + fftdsp_faustpower2_f(std::fabs(fTemp675 + 0.55557024f * fTemp671 - (0.29028466f * fTemp1245 + fTemp676 + fTemp396 + fTemp677 + 0.8314696f * fTemp666 + 0.95694035f * fTemp1246))));
			float fTemp1247 = fTemp691 + fTemp635 + fTemp696 + fTemp68 + 0.6343933f * fTemp722 - (fTemp713 + fTemp715 + fTemp717 + fTemp94 + 0.77301043f * fTemp709);
			float fTemp1248 = fTemp739 + fTemp740 + fTemp62 + fTemp742 + fTemp741 - (fTemp743 + fTemp744 + 0.6343933f * fTemp709 + fTemp43 + 0.77301043f * fTemp722);
			output39[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp678 + fTemp733 + 0.94154406f * fTemp1247 - (fTemp746 + 0.6343933f * fTemp690 + fTemp749 + 0.77301043f * fTemp738 + 0.33688986f * fTemp1248))) + fftdsp_faustpower2_f(std::fabs(fTemp750 + fTemp639 + fTemp751 + 0.6343933f * fTemp738 - (0.94154406f * fTemp1248 + fTemp755 + 0.77301043f * fTemp690 + 0.33688986f * fTemp1247))));
			float fTemp1249 = fTemp756 + fTemp761 - (fTemp759 + 0.70710677f * fTemp758);
			float fTemp1250 = 0.70710677f * fTemp767;
			float fTemp1251 = 0.70710677f * fTemp760;
			float fTemp1252 = fTemp770 - (fTemp769 + fTemp772 + fTemp1251);
			output40[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp766 + 0.9238795f * fTemp1249 - (fTemp765 + fTemp774 + fTemp1250 + 0.38268343f * fTemp1252))) + fftdsp_faustpower2_f(std::fabs(fTemp775 + fTemp777 - (0.38268343f * fTemp1249 + fTemp776 + 0.70710677f * fTemp764 + 0.9238795f * fTemp1252))));
			float fTemp1253 = fTemp790 + fTemp130 + fTemp94 + 0.77301043f * fTemp805 - (fTemp199 + fTemp797 + fTemp801 + 0.6343933f * fTemp796);
			float fTemp1254 = fTemp292 + fTemp293 + fTemp808 - (fTemp809 + fTemp810 + 0.77301043f * fTemp796 + fTemp811 + 0.6343933f * fTemp805);
			output41[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp261 + fTemp263 + fTemp0 + fTemp789 + 0.9039893f * fTemp1253 - (fTemp813 + fTemp814 + 0.77301043f * fTemp783 + fTemp816 + 0.6343933f * fTemp787 + 0.42755508f * fTemp1254))) + fftdsp_faustpower2_f(std::fabs(fTemp817 + fTemp300 + fTemp95 + 0.77301043f * fTemp787 - (0.42755508f * fTemp1253 + fTemp818 + fTemp305 + fTemp819 + fTemp69 + fTemp820 + 0.6343933f * fTemp783 + 0.9039893f * fTemp1254))));
			float fTemp1255 = fTemp375 + fTemp346 + fTemp844 - (fTemp836 + fTemp838 + fTemp841 + fTemp376 + 0.55557024f * fTemp835);
			float fTemp1256 = fTemp407 + fTemp846 - (fTemp405 + fTemp848 + fTemp849 + fTemp847 + fTemp409 + 0.55557024f * fTemp843);
			output42[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp394 + fTemp830 + 0.8819213f * fTemp1255 - (fTemp393 + fTemp851 + fTemp826 + fTemp854 + 0.55557024f * fTemp828 + 0.47139674f * fTemp1256))) + fftdsp_faustpower2_f(std::fabs(fTemp417 + fTemp413 + fTemp856 - (0.47139674f * fTemp1255 + fTemp855 + fTemp857 + fTemp416 + fTemp858 + 0.55557024f * fTemp825 + 0.8819213f * fTemp1256))));
			float fTemp1257 = fTemp886 + fTemp68 + fTemp889 - (fTemp887 + fTemp442 + fTemp293 + fTemp888 + 0.47139674f * fTemp863);
			float fTemp1258 = fTemp489 + fTemp866 - (fTemp870 + fTemp871 + fTemp199 + fTemp864 + fTemp873 + 0.47139674f * fTemp869);
			output43[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp480 + fTemp885 + 0.8577286f * fTemp1257 - (fTemp891 + fTemp892 + fTemp305 + fTemp880 + fTemp894 + 0.47139674f * fTemp883 + 0.51410276f * fTemp1258))) + fftdsp_faustpower2_f(std::fabs(fTemp895 + fTemp69 + fTemp898 - (0.51410276f * fTemp1257 + fTemp896 + fTemp497 + fTemp263 + fTemp897 + 0.47139674f * fTemp879 + 0.8577286f * fTemp1258))));
			float fTemp1259 = fTemp515 + 0.9238795f * fTemp902 - (fTemp901 + fTemp548 + fTemp503 + 0.38268343f * fTemp900);
			float fTemp1260 = fTemp547 + fTemp906 - (fTemp518 + 0.9238795f * fTemp900 + fTemp550 + 0.38268343f * fTemp902);
			output44[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp538 + fTemp899 + 0.8314696f * fTemp1259 - (fTemp557 + 0.9238795f * fTemp904 + fTemp552 + 0.38268343f * fTemp905 + 0.55557024f * fTemp1260))) + fftdsp_faustpower2_f(std::fabs(fTemp556 + 0.9238795f * fTemp905 - (0.55557024f * fTemp1259 + fTemp908 + fTemp540 + fTemp554 + 0.38268343f * fTemp904 + 0.8314696f * fTemp1260))));
			float fTemp1261 = fTemp199 + fTemp923 + fTemp937 - (fTemp579 + fTemp930 + fTemp932 + fTemp68 + 0.29028466f * fTemp928);
			float fTemp1262 = fTemp944 + fTemp941 - (fTemp292 + fTemp635 + fTemp947 + fTemp942 + fTemp946 + 0.29028466f * fTemp936);
			output45[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp940 + fTemp920 + 0.8032075f * fTemp1261 - (fTemp261 + fTemp639 + fTemp914 + fTemp951 + 0.29028466f * fTemp918 + 0.5956993f * fTemp1262))) + fftdsp_faustpower2_f(std::fabs(fTemp305 + fTemp954 + fTemp955 - (0.5956993f * fTemp1261 + fTemp645 + fTemp956 + fTemp69 + fTemp957 + 0.29028466f * fTemp913 + 0.8032075f * fTemp1262))));
			float fTemp1263 = fTemp961 + fTemp848 + fTemp376 + 0.98078525f * fTemp971 - (fTemp968 + 0.19509032f * fTemp965);
			float fTemp1264 = fTemp980 + fTemp981 - (fTemp982 + 0.98078525f * fTemp965 + fTemp983 + 0.19509032f * fTemp971);
			output46[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp958 + fTemp394 + fTemp960 + 0.77301043f * fTemp1263 - (fTemp985 + fTemp987 + 0.98078525f * fTemp976 + fTemp411 + 0.19509032f * fTemp979 + 0.6343933f * fTemp1264))) + fftdsp_faustpower2_f(std::fabs(fTemp988 + fTemp851 + fTemp416 + 0.98078525f * fTemp979 - (0.6343933f * fTemp1263 + fTemp989 + fTemp990 + fTemp413 + 0.19509032f * fTemp976 + 0.77301043f * fTemp1264))));
			float fTemp1265 = fTemp691 + fTemp635 + fTemp992 + fTemp993 + fTemp68 + 0.9951847f * fTemp1004 - (fTemp1001 + 0.09801714f * fTemp998);
			float fTemp1266 = fTemp739 + fTemp1016 + fTemp1017 - (fTemp743 + fTemp1018 + 0.9951847f * fTemp998 + fTemp1019 + 0.09801714f * fTemp1004);
			output47[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp678 + fTemp991 + fTemp1012 + 0.7409511f * fTemp1265 - (fTemp746 + fTemp1021 + 0.9951847f * fTemp1010 + fTemp1023 + 0.09801714f * fTemp1015 + 0.671559f * fTemp1266))) + fftdsp_faustpower2_f(std::fabs(fTemp750 + fTemp639 + fTemp1024 + fTemp1025 + fTemp69 + 0.9951847f * fTemp1015 - (0.671559f * fTemp1265 + fTemp1026 + 0.09801714f * fTemp1010 + 0.7409511f * fTemp1266))));
			output48[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp1029 + 0.70710677f * fTemp1027 - (fTemp1032 + fTemp1035))) + fftdsp_faustpower2_f(std::fabs(fTemp1034 - (0.70710677f * fTemp1027 + fTemp1033 + fTemp1031))));
			float fTemp1267 = fTemp130 + fTemp1036 + fTemp136 + 0.09801714f * fTemp1037 + fTemp94 + fTemp1039 - (fTemp197 + fTemp199 + fTemp68 + fTemp153);
			float fTemp1268 = fTemp291 + fTemp292 + fTemp293 + fTemp294 + fTemp62 + 0.09801714f * fTemp1038 - (fTemp1044 + fTemp295 + fTemp43 + fTemp1043);
			output49[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp254 + fTemp258 + fTemp261 + fTemp263 + fTemp0 + 0.09801714f * fTemp1047 + 0.671559f * fTemp1267 - (fTemp1046 + fTemp284 + fTemp44 + fTemp1042 + 0.7409511f * fTemp1268))) + fftdsp_faustpower2_f(std::fabs(fTemp299 + fTemp300 + fTemp1048 + fTemp1049 + fTemp95 + 0.09801714f * fTemp1041 - (0.7409511f * fTemp1267 + fTemp304 + fTemp305 + fTemp69 + fTemp301 + 0.671559f * fTemp1268))));
			float fTemp1269 = fTemp1050 + fTemp345 + 0.19509032f * fTemp1052 + fTemp346 + fTemp1054 - (fTemp376 + fTemp375 + fTemp1051);
			float fTemp1270 = fTemp405 + fTemp406 + fTemp407 + 0.19509032f * fTemp1053 - (fTemp1059 + fTemp1060 + fTemp409 + fTemp1058);
			output50[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp393 + fTemp396 + fTemp394 + 0.19509032f * fTemp1064 + 0.6343933f * fTemp1269 - (fTemp1062 + fTemp1063 + fTemp411 + fTemp1057 + 0.77301043f * fTemp1270))) + fftdsp_faustpower2_f(std::fabs(fTemp1065 + fTemp414 + 0.19509032f * fTemp1056 + fTemp413 + fTemp1067 - (0.77301043f * fTemp1269 + fTemp418 + fTemp1066 + 0.6343933f * fTemp1270))));
			float fTemp1271 = fTemp1070 + fTemp421 + 0.29028466f * fTemp1072 + fTemp68 + fTemp1074 - (fTemp446 + fTemp1071);
			float fTemp1272 = fTemp488 + fTemp490 + fTemp489 + fTemp62 + 0.29028466f * fTemp1073 - (fTemp1077 + fTemp1078 + fTemp199 + fTemp43 + fTemp1076);
			output51[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp479 + fTemp482 + fTemp480 + fTemp0 + 0.29028466f * fTemp1082 + 0.5956993f * fTemp1271 - (fTemp1080 + fTemp1081 + fTemp305 + fTemp44 + fTemp1069 + 0.8032075f * fTemp1272))) + fftdsp_faustpower2_f(std::fabs(fTemp1083 + fTemp495 + 0.29028466f * fTemp1068 + fTemp69 + fTemp1085 - (0.8032075f * fTemp1271 + fTemp500 + fTemp1084 + 0.5956993f * fTemp1272))));
			float fTemp1273 = fTemp518 + 0.38268343f * fTemp1087 + fTemp503 + 0.9238795f * fTemp1089 - (fTemp515 + fTemp1086);
			float fTemp1274 = fTemp547 + 0.38268343f * fTemp1089 - (fTemp546 + fTemp1092 + fTemp550 + 0.9238795f * fTemp1087);
			output52[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp538 + 0.38268343f * fTemp1096 + 0.55557024f * fTemp1273 - (fTemp537 + fTemp1094 + fTemp552 + 0.9238795f * fTemp1091 + 0.8314696f * fTemp1274))) + fftdsp_faustpower2_f(std::fabs(fTemp557 + 0.38268343f * fTemp1091 + fTemp554 + 0.9238795f * fTemp1096 - (0.8314696f * fTemp1273 + fTemp556 + fTemp1097 + 0.55557024f * fTemp1274))));
			float fTemp1275 = fTemp199 + fTemp588 + 0.47139674f * fTemp1099 + fTemp94 + fTemp1101 - (fTemp579 + fTemp585 + fTemp1098);
			float fTemp1276 = fTemp632 + fTemp62 + 0.47139674f * fTemp1100 - (fTemp292 + fTemp635 + fTemp631 + fTemp1106 + fTemp636 + fTemp43 + fTemp1105);
			output53[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp607 + fTemp0 + 0.47139674f * fTemp1109 + 0.51410276f * fTemp1275 - (fTemp261 + fTemp639 + fTemp602 + fTemp1108 + fTemp642 + fTemp44 + fTemp1104 + 0.8577286f * fTemp1276))) + fftdsp_faustpower2_f(std::fabs(fTemp305 + fTemp646 + 0.47139674f * fTemp1103 + fTemp95 + fTemp1111 - (0.8577286f * fTemp1275 + fTemp645 + fTemp647 + fTemp69 + fTemp648 + fTemp1110 + 0.51410276f * fTemp1276))));
			float fTemp1277 = fTemp1112 + 0.55557024f * fTemp1114 + fTemp376 + 0.8314696f * fTemp1115 - (fTemp657 + fTemp406 + fTemp346 + fTemp1113);
			float fTemp1278 = fTemp672 + fTemp407 + 0.55557024f * fTemp1115 - (fTemp375 + fTemp1118 + fTemp1119 + fTemp409 + 0.8314696f * fTemp1114);
			output54[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp650 + fTemp394 + 0.55557024f * fTemp1123 + 0.47139674f * fTemp1277 - (fTemp417 + fTemp1121 + fTemp1122 + fTemp411 + 0.8314696f * fTemp1117 + 0.8819213f * fTemp1278))) + fftdsp_faustpower2_f(std::fabs(fTemp1124 + 0.55557024f * fTemp1117 + fTemp416 + 0.8314696f * fTemp1123 - (0.8819213f * fTemp1277 + fTemp676 + fTemp396 + fTemp413 + fTemp1125 + 0.47139674f * fTemp1278))));
			float fTemp1279 = fTemp691 + fTemp635 + fTemp1126 + 0.6343933f * fTemp1128 + fTemp68 + 0.77301043f * fTemp1129 - (fTemp715 + fTemp94 + fTemp717 + fTemp1127);
			float fTemp1280 = fTemp739 + fTemp742 + fTemp62 + 0.6343933f * fTemp1129 - (fTemp743 + fTemp1132 + fTemp1133 + fTemp744 + fTemp43 + 0.77301043f * fTemp1128);
			output55[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp678 + fTemp729 + fTemp0 + 0.6343933f * fTemp1137 + 0.42755508f * fTemp1279 - (fTemp746 + fTemp1135 + fTemp1136 + fTemp748 + fTemp44 + 0.77301043f * fTemp1131 + 0.9039893f * fTemp1280))) + fftdsp_faustpower2_f(std::fabs(fTemp750 + fTemp639 + fTemp1138 + 0.6343933f * fTemp1131 + fTemp69 + 0.77301043f * fTemp1137 - (0.9039893f * fTemp1279 + fTemp754 + fTemp1139 + 0.42755508f * fTemp1280))));
			float fTemp1281 = 0.70710677f * fTemp757 + fTemp759 + fTemp1251 - fTemp756;
			float fTemp1282 = fTemp770 + 0.70710677f * fTemp760 - (fTemp772 + 0.70710677f * fTemp757);
			output56[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp766 + 0.70710677f * fTemp767 + 0.38268343f * fTemp1281 - (fTemp774 + 0.70710677f * fTemp763 + 0.9238795f * fTemp1282))) + fftdsp_faustpower2_f(std::fabs(0.70710677f * fTemp763 + fTemp776 + fTemp1250 - (0.9238795f * fTemp1281 + fTemp775 + 0.38268343f * fTemp1282))));
			float fTemp1283 = fTemp790 + fTemp130 + fTemp1143 + fTemp800 + 0.77301043f * fTemp1144 + fTemp94 + 0.6343933f * fTemp1145 - (fTemp199 + fTemp68 + fTemp797);
			float fTemp1284 = fTemp292 + fTemp293 + fTemp1148 + fTemp62 + 0.77301043f * fTemp1145 - (fTemp809 + fTemp810 + fTemp807 + fTemp43 + 0.6343933f * fTemp1144);
			output57[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp261 + fTemp263 + fTemp1142 + fTemp0 + 0.77301043f * fTemp1150 + 0.33688986f * fTemp1283 - (fTemp813 + fTemp814 + fTemp789 + fTemp44 + 0.6343933f * fTemp1147 + 0.94154406f * fTemp1284))) + fftdsp_faustpower2_f(std::fabs(fTemp817 + fTemp300 + fTemp1151 + fTemp820 + 0.77301043f * fTemp1147 + fTemp95 + 0.6343933f * fTemp1150 - (0.94154406f * fTemp1283 + fTemp69 + fTemp305 + fTemp818 + 0.33688986f * fTemp1284))));
			float fTemp1285 = fTemp375 + fTemp1153 + fTemp1154 + 0.8314696f * fTemp1155 + fTemp346 + fTemp1157 - (fTemp376 + fTemp836);
			float fTemp1286 = fTemp1161 + fTemp407 + 0.8314696f * fTemp1156 - (fTemp405 + fTemp848 + fTemp1163 + fTemp409 + fTemp1162);
			output58[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp1152 + fTemp394 + 0.8314696f * fTemp1166 + 0.29028466f * fTemp1285 - (fTemp393 + fTemp851 + fTemp1165 + fTemp411 + fTemp1160 + 0.95694035f * fTemp1286))) + fftdsp_faustpower2_f(std::fabs(fTemp417 + fTemp1167 + fTemp1168 + 0.8314696f * fTemp1159 + fTemp413 + fTemp1169 - (0.95694035f * fTemp1285 + fTemp416 + fTemp855 + 0.29028466f * fTemp1286))));
			float fTemp1287 = fTemp886 + fTemp1172 + fTemp1173 + 0.8819213f * fTemp1174 + fTemp68 + 0.47139674f * fTemp1175 - (fTemp443 + fTemp887);
			float fTemp1288 = fTemp489 + fTemp1177 + fTemp62 + 0.8819213f * fTemp1175 - (fTemp870 + fTemp871 + fTemp199 + fTemp1178 + fTemp43 + 0.47139674f * fTemp1174);
			output59[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp480 + fTemp1170 + fTemp0 + 0.8819213f * fTemp1181 + 0.24298018f * fTemp1287 - (fTemp891 + fTemp892 + fTemp305 + fTemp1180 + fTemp44 + 0.47139674f * fTemp1171 + 0.97003126f * fTemp1288))) + fftdsp_faustpower2_f(std::fabs(fTemp895 + fTemp1182 + fTemp1183 + 0.8819213f * fTemp1171 + fTemp69 + 0.47139674f * fTemp1181 - (0.97003126f * fTemp1287 + fTemp498 + fTemp896 + 0.24298018f * fTemp1288))));
			float fTemp1289 = fTemp1185 + fTemp1092 + 0.9238795f * fTemp1186 + fTemp515 + 0.38268343f * fTemp1187 - fTemp503;
			float fTemp1290 = fTemp1190 + fTemp547 + 0.9238795f * fTemp1187 - (fTemp1191 + fTemp550 + 0.38268343f * fTemp1186);
			output60[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp1184 + fTemp538 + 0.9238795f * fTemp1194 + 0.19509032f * fTemp1289 - (fTemp1193 + fTemp552 + 0.38268343f * fTemp1189 + 0.98078525f * fTemp1290))) + fftdsp_faustpower2_f(std::fabs(fTemp1195 + fTemp1094 + 0.9238795f * fTemp1189 + fTemp556 + 0.38268343f * fTemp1194 - (0.98078525f * fTemp1289 + fTemp554 + 0.19509032f * fTemp1290))));
			float fTemp1291 = fTemp199 + fTemp1197 + fTemp932 + fTemp921 + fTemp922 + fTemp1199 + fTemp94 + 0.29028466f * fTemp1200 - (fTemp68 + fTemp579);
			float fTemp1292 = fTemp1203 + fTemp943 + fTemp62 + fTemp1204 - (fTemp292 + fTemp635 + fTemp941 + fTemp945 + fTemp43 + 0.29028466f * fTemp1198);
			output61[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp1196 + fTemp939 + fTemp0 + fTemp1207 + 0.14673047f * fTemp1291 - (fTemp261 + fTemp639 + fTemp920 + fTemp949 + fTemp44 + 0.29028466f * fTemp1202 + 0.9891765f * fTemp1292))) + fftdsp_faustpower2_f(std::fabs(fTemp305 + fTemp1208 + fTemp957 + fTemp952 + fTemp953 + fTemp1209 + fTemp95 + 0.29028466f * fTemp1206 - (0.9891765f * fTemp1291 + fTemp69 + fTemp645 + 0.14673047f * fTemp1292))));
			float fTemp1293 = fTemp961 + fTemp848 + 0.98078525f * fTemp1212 + fTemp1211 + 0.19509032f * fTemp1213 - fTemp346;
			float fTemp1294 = fTemp980 + fTemp1216 + 0.98078525f * fTemp1213 - (fTemp982 + fTemp1217 + 0.19509032f * fTemp1212);
			output62[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp958 + fTemp1210 + fTemp394 + 0.98078525f * fTemp1220 + 0.09801714f * fTemp1293 - (fTemp985 + fTemp1219 + fTemp411 + 0.19509032f * fTemp1215 + 0.9951847f * fTemp1294))) + fftdsp_faustpower2_f(std::fabs(fTemp988 + fTemp851 + fTemp1221 + fTemp1222 + 0.98078525f * fTemp1215 + fTemp416 + 0.19509032f * fTemp1220 - (0.9951847f * fTemp1293 + fTemp413 + 0.09801714f * fTemp1294))));
			float fTemp1295 = fTemp691 + fTemp635 + fTemp992 + fTemp993 + 0.9951847f * fTemp1224 + fTemp1223 + 0.09801714f * fTemp1225 - fTemp94;
			float fTemp1296 = fTemp739 + fTemp1016 + fTemp1229 + 0.9951847f * fTemp1225 - (fTemp743 + fTemp1018 + fTemp1230 + 0.09801714f * fTemp1224);
			output63[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp678 + fTemp991 + fTemp1227 + 0.9951847f * fTemp1233 + 0.049067676f * fTemp1295 - (fTemp746 + fTemp1021 + fTemp1232 + 0.09801714f * fTemp1228 + 0.99879545f * fTemp1296))) + fftdsp_faustpower2_f(std::fabs(fTemp750 + fTemp639 + fTemp1024 + fTemp1025 + 0.9951847f * fTemp1228 + fTemp1234 + 0.09801714f * fTemp1233 - (0.99879545f * fTemp1295 + fTemp95 + 0.049067676f * fTemp1296))));
			output64[i0] = FAUSTFLOAT(fftdsp_faustpower2_f(std::fabs(fTemp2 + fTemp4 + fTemp6 + fTemp8 + fTemp10 + fTemp12 + fTemp14 + fTemp16 + fTemp18 + fTemp20 + fTemp22 + fTemp24 + fTemp26 + fTemp28 + fTemp30 + fTemp32 + fTemp34 + fTemp36 + fTemp38 + fTemp40 + fTemp42 + fTemp44 + fTemp46 + fTemp47 + fTemp49 + fTemp51 + fTemp53 + fTemp55 + fTemp57 + fTemp59 + fTemp61 + fTemp0 + fTemp63 + fTemp65 + fTemp67 + fTemp69 + fTemp71 + fTemp73 + fTemp75 + fTemp77 + fTemp79 + fTemp81 + fTemp83 + fTemp85 + fTemp87 + fTemp89 + fTemp91 + fTemp93 + fTemp95 + fTemp97 + fTemp99 + fTemp101 + fTemp103 + fTemp105 + fTemp108 + fTemp110 + fTemp112 + fTemp114 + fTemp116 + fTemp118 + fTemp120 + fTemp122 + fTemp127 + fTemp124 - (fTemp1 + fTemp3 + fTemp5 + fTemp7 + fTemp9 + fTemp11 + fTemp13 + fTemp15 + fTemp17 + fTemp19 + fTemp21 + fTemp23 + fTemp25 + fTemp27 + fTemp29 + fTemp31 + fTemp33 + fTemp35 + fTemp37 + fTemp39 + fTemp41 + fTemp43 + fTemp45 + fTemp48 + fTemp50 + fTemp52 + fTemp54 + fTemp56 + fTemp58 + fTemp60 + fTemp62 + fTemp64 + fTemp66 + fTemp68 + fTemp70 + fTemp72 + fTemp74 + fTemp76 + fTemp78 + fTemp80 + fTemp82 + fTemp84 + fTemp86 + fTemp88 + fTemp90 + fTemp92 + fTemp94 + fTemp96 + fTemp98 + fTemp100 + fTemp102 + fTemp104 + fTemp106 + fTemp107 + fTemp109 + fTemp111 + fTemp113 + fTemp115 + fTemp117 + fTemp119 + fTemp121 + fTemp123 + fTemp126 + fTemp125))));
			IOTA0 = IOTA0 + 1;
		}
	}

};


#endif
