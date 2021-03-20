# group `effects` {#group__effects}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`class `[`Compressor`](#class_compressor) | Applies compressor_mono from the faustlibraries distribution, compressors.lib.
`class `[`Effects`](#class_effects) | [Effects](#class_effects) Class.
`class `[`Limiter`](#class_limiter) | Applies limiter_lad_mono from the faustlibraries distribution, compressors.lib.
`class `[`NetKS`](#class_net_k_s) | A simple (basic) network Karplus Strong.
`class `[`ProcessPlugin`](#class_process_plugin) | Interface for the process plugins to add to the JACK callback process in [JackAudioInterface](audio_interface.md#class_jack_audio_interface).
`class `[`Reverb`](#class_reverb) | Applies freeverb or zitarev from the faustlibraries distribution: reverbs.lib.

## class `Compressor` {#class_compressor}

```
class Compressor
  : public ProcessPlugin
```

Applies compressor_mono from the faustlibraries distribution, compressors.lib.

A [Compressor](#class_compressor) reduces the output dynamic range when the signal level exceeds the threshold.

### Members

??? example "`public inline  `[`Compressor`](#class_compressor_1af1d39f5e1ed2fad9e007748d87b33905)`(int numchans,bool verboseIn,float ratioIn,float thresholdDBIn,float attackMSIn,float releaseMSIn,float makeUpGainDBIn)` <br/>// The class constructor sets the number of audio channels and default parameters."
    

??? example "`public inline  `[`Compressor`](#class_compressor_1ab1ecc30a2a54e72a353b01872b968222)`(int numchans,bool verboseIn,`[`CompressorPreset`](undefined.md#struct_compressor_preset)` preset)` <br/>// "
    

??? example "`public inline virtual  `[`~Compressor`](#class_compressor_1a0610fdbd71526609ed20f0a3cd919360)`()` <br/>// The class destructor."
    

??? example "`public inline void `[`setParamAllChannels`](#class_compressor_1af299e80a7497fd6739d5e6a068640731)`(const char pName,float p)` <br/>// "
    

??? example "`public inline virtual void `[`init`](#class_compressor_1af03605a1596ed1627f483752d6180a9d)`(int samplingRate)` <br/>// Do proper Initialization of members and class instances. By default this initializes the Sampling Frequency. If a class instance depends on the sampling frequency, it should be initialize here."
    

??? example "`public inline virtual int `[`getNumInputs`](#class_compressor_1aab4676dbd7b4f4f93a9c6b1657c3deab)`()` <br/>// Return Number of Input Channels."
    

??? example "`public inline virtual int `[`getNumOutputs`](#class_compressor_1a700245bcfdf5445b8ec258f14469ab98)`()` <br/>// Return Number of Output Channels."
    

??? example "`public virtual void `[`compute`](#class_compressor_1a459fca73767848397a26926e836c2175)`(int nframes,float ** inputs,float ** outputs)` <br/>// Compute process."
    

## class `Effects` {#class_effects}

[Effects](#class_effects) Class.

### Members

??? example "`public inline  `[`Effects`](#class_effects_1a5d72213afc8146eb8cf4aa43418ef7a4)`(bool outGoingLimiterOn)` <br/>// "
    

??? example "`public inline  `[`~Effects`](#class_effects_1abcac8dac82151a97284e63dcc5919f37)`()` <br/>// "
    

??? example "`public inline unsigned int `[`getNumClientsAssumed`](#class_effects_1a8ced4d958d551c0f6485a0de258c7716)`()` <br/>// "
    

??? example "`public inline `[`LIMITER_MODE`](#class_effects_1a22f55001aaa42515807c45b4907f9749)` `[`getLimit`](#class_effects_1aad6c87df0c8860779e729e4f1ca0197c)`()` <br/>// "
    

??? example "`public inline void `[`setNoLimiters`](#class_effects_1abf97e54dbb59934a68fe0c27fde4dc21)`()` <br/>// "
    

??? example "`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getInCompressor`](#class_effects_1ae5abf3ec7fd0ab389252ec76961c67a8)`()` <br/>// "
    

??? example "`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getOutCompressor`](#class_effects_1a92918313fdda07bfff507851eceb9f6f)`()` <br/>// "
    

??? example "`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getInZitarev`](#class_effects_1ad8e3da4f9af54ad32df9407d9d07678a)`()` <br/>// "
    

??? example "`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getOutZitarev`](#class_effects_1ab984ff1181d61559f51733a2f5d081b0)`()` <br/>// "
    

??? example "`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getInFreeverb`](#class_effects_1ac9e300436cf09db9dd921ff038bf508b)`()` <br/>// "
    

??? example "`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getOutFreeverb`](#class_effects_1a35bbd03014a95e0fea9c1071aa5e8269)`()` <br/>// "
    

??? example "`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getInLimiter`](#class_effects_1a479d7a0e602611b43cd19b5768005f1f)`()` <br/>// "
    

??? example "`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getOutLimiter`](#class_effects_1a6f58be8c6cf76bc8bf463abf74e0f2ec)`()` <br/>// "
    

??? example "`public inline bool `[`getHaveEffect`](#class_effects_1a4eb993f8d4fd18afdec36efe2c5c5b05)`()` <br/>// "
    

??? example "`public inline bool `[`getHaveLimiter`](#class_effects_1aaf75e0fef072758af6af865bdd08ddcc)`()` <br/>// "
    

??? example "`public inline void `[`setVerboseFlag`](#class_effects_1aee37eb9a81f5d1ef2591c65bf6e8c581)`(int v)` <br/>// "
    

??? example "`public inline int `[`getNumIncomingChans`](#class_effects_1a130c048bfc2f5d922c6d10512eed65fc)`()` <br/>// "
    

??? example "`public inline int `[`getOutgoingNumChans`](#class_effects_1a616ae55ae19ebd2a388567c4d71150d9)`()` <br/>// "
    

??? example "`public inline std::vector< `[`ProcessPlugin`](#class_process_plugin)` * > `[`allocateIncomingEffects`](#class_effects_1af46ed66a2bc7a682536fa68a920cab09)`(int nIncomingChans)` <br/>// "
    

??? example "`public inline std::vector< `[`ProcessPlugin`](#class_process_plugin)` * > `[`allocateOutgoingEffects`](#class_effects_1aff3b5a8b26223c825daa0cc73b8a8d00)`(int nOutgoingChans)` <br/>// "
    

??? example "`public inline void `[`printHelp`](#class_effects_1a1c1e4ae0297fdc6d65fca15c361f966c)`(char * command,char helpCase)` <br/>// "
    

??? example "`public inline int `[`setCompressorPresetIndexFrom1`](#class_effects_1acebd5f990938b796319382d75451dc59)`(unsigned long presetIndexFrom1,InOrOut io)` <br/>// "
    

??? example "`public inline int `[`parseCompresserArgs`](#class_effects_1aaf78bb3ca7258922600d355ab546a834)`(char * args,InOrOut inOrOut)` <br/>// "
    

??? example "`public inline int `[`parseEffectsOptArg`](#class_effects_1ad3780e901b7055baf626b0c7406ec66c)`(char * cmd,char * optarg)` <br/>// "
    

??? example "`public inline int `[`parseLimiterOptArg`](#class_effects_1a927afb5e6289ff48b33164074e4e244b)`(char * cmd,char * optarg)` <br/>// "
    

??? example "`public inline int `[`parseAssumedNumClientsOptArg`](#class_effects_1a52f34d0f75d3f9777bf61b7e4292bb0f)`(char * cmd,char * optarg)` <br/>// "
    

??? example "`enum `[`LIMITER_MODE`](#class_effects_1a22f55001aaa42515807c45b4907f9749) <br/>// "
     Values                         | Descriptions                                
    --------------------------------|---------------------------------------------
        LIMITER_NONE            | 
    LIMITER_INCOMING            | 
    LIMITER_OUTGOING            | 
    LIMITER_BOTH            | 
    

## class `Limiter` {#class_limiter}

```
class Limiter
  : public ProcessPlugin
```

Applies limiter_lad_mono from the faustlibraries distribution, compressors.lib.

The [Limiter](#class_limiter) class confines the output dynamic range to a "dynamic range lane" determined by the assumed number of clients.

### Members

??? example "`public inline  `[`Limiter`](#class_limiter_1a45369abfb5dd0c6faaedc69987cf398c)`(int numchans,int numclients,bool verboseFlag)` <br/>// The class constructor sets the number of channels to limit."
    

??? example "`public inline virtual  `[`~Limiter`](#class_limiter_1a98a1517d6ab45de67596e2a8f7f95244)`()` <br/>// The class destructor."
    

??? example "`public inline virtual void `[`init`](#class_limiter_1a2bc6bc28a1419863a901c9002be2eabc)`(int samplingRate)` <br/>// Do proper Initialization of members and class instances. By default this initializes the Sampling Frequency. If a class instance depends on the sampling frequency, it should be initialize here."
    

??? example "`public inline virtual int `[`getNumInputs`](#class_limiter_1affd7eb07526294b02fcde1b0b82fb50b)`()` <br/>// Return Number of Input Channels."
    

??? example "`public inline virtual int `[`getNumOutputs`](#class_limiter_1a3c73902b314a06f21ef3a01b03f89d0b)`()` <br/>// Return Number of Output Channels."
    

??? example "`public virtual void `[`compute`](#class_limiter_1a7ea48caa85017fed491fea97cd062e5f)`(int nframes,float ** inputs,float ** outputs)` <br/>// Compute process."
    

??? example "`public inline void `[`setWarningAmplitude`](#class_limiter_1a3ab64c711f8914edb44dbecbade2db4f)`(double wa)` <br/>// "
    

## class `NetKS` {#class_net_k_s}

```
class NetKS
  : public ProcessPlugin
```

A simple (basic) network Karplus Strong.

This plugin creates a one channel network karplus strong.

### Members

??? example "`public inline virtual int `[`getNumInputs`](#class_net_k_s_1aeb39522d392194d4830018aec8ae3aae)`()` <br/>// Return Number of Input Channels."
    

??? example "`public inline virtual int `[`getNumOutputs`](#class_net_k_s_1aa4fe17c36115b4bde1a87906c561bc0f)`()` <br/>// Return Number of Output Channels."
    

??? example "`public inline virtual void `[`instanceInit`](#class_net_k_s_1a823566485917d45f8e27196d8e144d3d)`(int samplingFreq)` <br/>// "
    

??? example "`public inline virtual void `[`init`](#class_net_k_s_1a2b4550cd6cd356d86410463cd59db1ef)`(int samplingRate)` <br/>// Do proper Initialization of members and class instances. By default this initializes the Sampling Frequency. If a class instance depends on the sampling frequency, it should be initialize here."
    

??? example "`public inline virtual void `[`compute`](#class_net_k_s_1a76324f4cd487618a55ed6e28a8a47b9d)`(int nframes,float ** inputs,float ** outputs)` <br/>// Compute process."
    

## class `ProcessPlugin` {#class_process_plugin}

```
class ProcessPlugin
  : public QThread
```

Interface for the process plugins to add to the JACK callback process in [JackAudioInterface](audio_interface.md#class_jack_audio_interface).

This class contains the same methods of the FAUST dsp class. A mydsp class can inherit from this class the same way it inherits from dsp. Subclass should implement all methods except init, which is optional for processing that are sampling rate dependent or that need specific initialization.

### Members

??? example "`public inline  `[`ProcessPlugin`](#class_process_plugin_1a32a0fd38bbef035ac134d75c2e8a8151)`()` <br/>// The Class Constructor."
    

??? example "`public inline virtual  `[`~ProcessPlugin`](#class_process_plugin_1a866c25570a2c087b4d58215595d4abad)`()` <br/>// The Class Destructor."
    

??? example "`public int `[`getNumInputs`](#class_process_plugin_1aace454a93a62dfdc96c1f66c9951304f)`()` <br/>// Return Number of Input Channels."
    

??? example "`public int `[`getNumOutputs`](#class_process_plugin_1aef66b85a5463a18ada72c225b3edc581)`()` <br/>// Return Number of Output Channels."
    

??? example "`public inline virtual char * `[`getName`](#class_process_plugin_1af67059fe391412a30b8748e34140ff7e)`()` <br/>// "
    

??? example "`public inline virtual void `[`init`](#class_process_plugin_1ab19b2570a366aa721c5725980bd8d358)`(int samplingRate)` <br/>// Do proper Initialization of members and class instances. By default this initializes the Sampling Frequency. If a class instance depends on the sampling frequency, it should be initialize here."
    

??? example "`public inline virtual bool `[`getInited`](#class_process_plugin_1ae1ae57c7aaf7c4b97cface77107fc644)`()` <br/>// "
    

??? example "`public inline virtual void `[`setVerbose`](#class_process_plugin_1a2d5a26e2ef41439f16afa76c581c71c8)`(bool v)` <br/>// "
    

??? example "`public void `[`compute`](#class_process_plugin_1a2b3e85cf1543f0a66fee56854ff3bb40)`(int nframes,float ** inputs,float ** outputs)` <br/>// Compute process."
    

??? example "`protected int `[`fSamplingFreq`](#class_process_plugin_1a3e78263cc0f285155dfc8029ec38813b) <br/>// Faust Data member, Sampling Rate."
    

??? example "`protected bool `[`inited`](#class_process_plugin_1af2e51f4e711f6461e3df30cc48dbfb64) <br/>// "
    

??? example "`protected bool `[`verbose`](#class_process_plugin_1af52737dcfe83c8ff11de3caba3d2c1e4) <br/>// "
    

## class `Reverb` {#class_reverb}

```
class Reverb
  : public ProcessPlugin
```

Applies freeverb or zitarev from the faustlibraries distribution: reverbs.lib.

A [Reverb](#class_reverb) is an echo-based delay effect, providing a virtual acoustic listening space.

### Members

??? example "`public inline  `[`Reverb`](#class_reverb_1a3fca00dcc39b8de07bae131d6c0dddd4)`(int numInChans,int numOutChans,float reverbLevel,bool verboseFlag)` <br/>// The class constructor sets the number of channels to limit."
    

??? example "`public inline virtual  `[`~Reverb`](#class_reverb_1a16d44945f1d5be965154e6500358f38a)`()` <br/>// The class destructor."
    

??? example "`public inline virtual void `[`init`](#class_reverb_1a534765f7b21b3eb636656bf4ef5ba62c)`(int samplingRate)` <br/>// Do proper Initialization of members and class instances. By default this initializes the Sampling Frequency. If a class instance depends on the sampling frequency, it should be initialize here."
    

??? example "`public inline virtual int `[`getNumInputs`](#class_reverb_1abf98eebfdf33ef7c01f2a71b9e7e40e5)`()` <br/>// Return Number of Input Channels."
    

??? example "`public inline virtual int `[`getNumOutputs`](#class_reverb_1a14e2e9e11f8502e1c26a4fd7aba9d2ce)`()` <br/>// Return Number of Output Channels."
    

??? example "`public virtual void `[`compute`](#class_reverb_1a17e2d35335053052e887b6387be6ae80)`(int nframes,float ** inputs,float ** outputs)` <br/>// Compute process."
    

