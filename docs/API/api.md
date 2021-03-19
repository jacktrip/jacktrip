# namespace `CompressorPresets` {#namespace_compressor_presets}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`enum `[`CompressorPresetNames`](#namespace_compressor_presets_1ad51bc47d8a538416a05233e0e8d0ed18)            | 

## Members

#### `enum `[`CompressorPresetNames`](#namespace_compressor_presets_1ad51bc47d8a538416a05233e0e8d0ed18) {#namespace_compressor_presets_1ad51bc47d8a538416a05233e0e8d0ed18}

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
CPN_VOICE            | 
CPN_BRASS            | 
CPN_SNARE            | 
CPN_NUMPRESETS            | 

# class `AccDownConverter` {#class_acc_down_converter}

```
class AccDownConverter
  : public UpdatableValueConverter
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`AccDownConverter`](#class_acc_down_converter_1a00cad757d6a5918d7b0fafbba65b86cb)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` | 
`public inline virtual double `[`ui2faust`](#class_acc_down_converter_1a57f99b83870b969530cc12287e889db6)`(double x)` | 
`public inline virtual double `[`faust2ui`](#class_acc_down_converter_1a014b56b87dd87778ac6bc447d944c859)`(double x)` | 
`public inline virtual void `[`setMappingValues`](#class_acc_down_converter_1a8154d97f458ea2492d57f3ba9fd388da)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` | 
`public inline virtual void `[`getMappingValues`](#class_acc_down_converter_1a29e776afb8cd2e9d244f9229486977f7)`(double & amin,double & amid,double & amax)` | 

## Members

#### `public inline  `[`AccDownConverter`](#class_acc_down_converter_1a00cad757d6a5918d7b0fafbba65b86cb)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` {#class_acc_down_converter_1a00cad757d6a5918d7b0fafbba65b86cb}

#### `public inline virtual double `[`ui2faust`](#class_acc_down_converter_1a57f99b83870b969530cc12287e889db6)`(double x)` {#class_acc_down_converter_1a57f99b83870b969530cc12287e889db6}

#### `public inline virtual double `[`faust2ui`](#class_acc_down_converter_1a014b56b87dd87778ac6bc447d944c859)`(double x)` {#class_acc_down_converter_1a014b56b87dd87778ac6bc447d944c859}

#### `public inline virtual void `[`setMappingValues`](#class_acc_down_converter_1a8154d97f458ea2492d57f3ba9fd388da)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` {#class_acc_down_converter_1a8154d97f458ea2492d57f3ba9fd388da}

#### `public inline virtual void `[`getMappingValues`](#class_acc_down_converter_1a29e776afb8cd2e9d244f9229486977f7)`(double & amin,double & amid,double & amax)` {#class_acc_down_converter_1a29e776afb8cd2e9d244f9229486977f7}

# class `AccDownUpConverter` {#class_acc_down_up_converter}

```
class AccDownUpConverter
  : public UpdatableValueConverter
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`AccDownUpConverter`](#class_acc_down_up_converter_1ae0fb098508da2537a418944cf9694ce6)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` | 
`public inline virtual double `[`ui2faust`](#class_acc_down_up_converter_1ada9bb59bcf82d18d053516135002f3c0)`(double x)` | 
`public inline virtual double `[`faust2ui`](#class_acc_down_up_converter_1abe5ee83dcc76996bb03a1ff3c816b98b)`(double x)` | 
`public inline virtual void `[`setMappingValues`](#class_acc_down_up_converter_1ad19eb2e7b04edabec73b5a925337d3c5)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` | 
`public inline virtual void `[`getMappingValues`](#class_acc_down_up_converter_1a75bffd4be14828855cb39922ffb0b277)`(double & amin,double & amid,double & amax)` | 

## Members

#### `public inline  `[`AccDownUpConverter`](#class_acc_down_up_converter_1ae0fb098508da2537a418944cf9694ce6)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` {#class_acc_down_up_converter_1ae0fb098508da2537a418944cf9694ce6}

#### `public inline virtual double `[`ui2faust`](#class_acc_down_up_converter_1ada9bb59bcf82d18d053516135002f3c0)`(double x)` {#class_acc_down_up_converter_1ada9bb59bcf82d18d053516135002f3c0}

#### `public inline virtual double `[`faust2ui`](#class_acc_down_up_converter_1abe5ee83dcc76996bb03a1ff3c816b98b)`(double x)` {#class_acc_down_up_converter_1abe5ee83dcc76996bb03a1ff3c816b98b}

#### `public inline virtual void `[`setMappingValues`](#class_acc_down_up_converter_1ad19eb2e7b04edabec73b5a925337d3c5)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` {#class_acc_down_up_converter_1ad19eb2e7b04edabec73b5a925337d3c5}

#### `public inline virtual void `[`getMappingValues`](#class_acc_down_up_converter_1a75bffd4be14828855cb39922ffb0b277)`(double & amin,double & amid,double & amax)` {#class_acc_down_up_converter_1a75bffd4be14828855cb39922ffb0b277}

# class `AccUpConverter` {#class_acc_up_converter}

```
class AccUpConverter
  : public UpdatableValueConverter
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`AccUpConverter`](#class_acc_up_converter_1ac4a9ae71591d86e485d00cfc6eeaae32)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` | 
`public inline virtual double `[`ui2faust`](#class_acc_up_converter_1ad65bd2b7ba4500eacb90539c9e0879d4)`(double x)` | 
`public inline virtual double `[`faust2ui`](#class_acc_up_converter_1a5940b433582dc62f2bd4c39d70f23152)`(double x)` | 
`public inline virtual void `[`setMappingValues`](#class_acc_up_converter_1a2b389c510b564f2b80516cc1711f66ae)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` | 
`public inline virtual void `[`getMappingValues`](#class_acc_up_converter_1a563ed9b95d82989552533b7bd211334c)`(double & amin,double & amid,double & amax)` | 

## Members

#### `public inline  `[`AccUpConverter`](#class_acc_up_converter_1ac4a9ae71591d86e485d00cfc6eeaae32)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` {#class_acc_up_converter_1ac4a9ae71591d86e485d00cfc6eeaae32}

#### `public inline virtual double `[`ui2faust`](#class_acc_up_converter_1ad65bd2b7ba4500eacb90539c9e0879d4)`(double x)` {#class_acc_up_converter_1ad65bd2b7ba4500eacb90539c9e0879d4}

#### `public inline virtual double `[`faust2ui`](#class_acc_up_converter_1a5940b433582dc62f2bd4c39d70f23152)`(double x)` {#class_acc_up_converter_1a5940b433582dc62f2bd4c39d70f23152}

#### `public inline virtual void `[`setMappingValues`](#class_acc_up_converter_1a2b389c510b564f2b80516cc1711f66ae)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` {#class_acc_up_converter_1a2b389c510b564f2b80516cc1711f66ae}

#### `public inline virtual void `[`getMappingValues`](#class_acc_up_converter_1a563ed9b95d82989552533b7bd211334c)`(double & amin,double & amid,double & amax)` {#class_acc_up_converter_1a563ed9b95d82989552533b7bd211334c}

# class `AccUpDownConverter` {#class_acc_up_down_converter}

```
class AccUpDownConverter
  : public UpdatableValueConverter
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`AccUpDownConverter`](#class_acc_up_down_converter_1a6fa30155e14096896c6910d30d0da2b2)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` | 
`public inline virtual double `[`ui2faust`](#class_acc_up_down_converter_1aeb140cc69325aefc34f8168bfdc4117c)`(double x)` | 
`public inline virtual double `[`faust2ui`](#class_acc_up_down_converter_1accd68f9222a6e649ca34af265714f660)`(double x)` | 
`public inline virtual void `[`setMappingValues`](#class_acc_up_down_converter_1aa93a579d6d92064a96a2b3d3522d7cb0)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` | 
`public inline virtual void `[`getMappingValues`](#class_acc_up_down_converter_1a13f13743df55172395b689f90b406226)`(double & amin,double & amid,double & amax)` | 

## Members

#### `public inline  `[`AccUpDownConverter`](#class_acc_up_down_converter_1a6fa30155e14096896c6910d30d0da2b2)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` {#class_acc_up_down_converter_1a6fa30155e14096896c6910d30d0da2b2}

#### `public inline virtual double `[`ui2faust`](#class_acc_up_down_converter_1aeb140cc69325aefc34f8168bfdc4117c)`(double x)` {#class_acc_up_down_converter_1aeb140cc69325aefc34f8168bfdc4117c}

#### `public inline virtual double `[`faust2ui`](#class_acc_up_down_converter_1accd68f9222a6e649ca34af265714f660)`(double x)` {#class_acc_up_down_converter_1accd68f9222a6e649ca34af265714f660}

#### `public inline virtual void `[`setMappingValues`](#class_acc_up_down_converter_1aa93a579d6d92064a96a2b3d3522d7cb0)`(double amin,double amid,double amax,double fmin,double fmid,double fmax)` {#class_acc_up_down_converter_1aa93a579d6d92064a96a2b3d3522d7cb0}

#### `public inline virtual void `[`getMappingValues`](#class_acc_up_down_converter_1a13f13743df55172395b689f90b406226)`(double & amin,double & amid,double & amax)` {#class_acc_up_down_converter_1a13f13743df55172395b689f90b406226}

# class `APIUI` {#class_a_p_i_u_i}

```
class APIUI
  : public PathBuilder
  : public Meta
  : public UI
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`APIUI`](#class_a_p_i_u_i_1ac2c640ba8517d7d2daae6decfff5f67b)`()` | 
`public inline virtual  `[`~APIUI`](#class_a_p_i_u_i_1a7dfac586920579e1f3f7ebee788dfe65)`()` | 
`public inline virtual void `[`openTabBox`](#class_a_p_i_u_i_1acddc57f8a18512011e204899b17a8964)`(const char * label)` | 
`public inline virtual void `[`openHorizontalBox`](#class_a_p_i_u_i_1aae31007b4ff2935f016d3e40f3e1f24e)`(const char * label)` | 
`public inline virtual void `[`openVerticalBox`](#class_a_p_i_u_i_1aa305afc94f3be0b6ebe3b7403f0bfd40)`(const char * label)` | 
`public inline virtual void `[`closeBox`](#class_a_p_i_u_i_1a25b531453e24de242de017d63c51bb56)`()` | 
`public inline virtual void `[`addButton`](#class_a_p_i_u_i_1a1e4e1fd2c09290ff273773ff80cd229a)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone)` | 
`public inline virtual void `[`addCheckButton`](#class_a_p_i_u_i_1a641b5238578421ed6fb2972e26dd4242)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone)` | 
`public inline virtual void `[`addVerticalSlider`](#class_a_p_i_u_i_1a6974686a5292c2ae2800c316782161c3)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` init,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` min,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` max,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` step)` | 
`public inline virtual void `[`addHorizontalSlider`](#class_a_p_i_u_i_1a564b927deb4caf9eabad60a35d65b99d)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` init,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` min,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` max,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` step)` | 
`public inline virtual void `[`addNumEntry`](#class_a_p_i_u_i_1a70807925011a0337e9ee82a4efff0075)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` init,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` min,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` max,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` step)` | 
`public inline virtual void `[`addHorizontalBargraph`](#class_a_p_i_u_i_1a6fd2909a888c47a6e2139f787580c2e8)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` min,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` max)` | 
`public inline virtual void `[`addVerticalBargraph`](#class_a_p_i_u_i_1ac52ecce0ab8f1ef0b50a383a5ee6b97c)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` min,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` max)` | 
`public inline virtual void `[`addSoundfile`](#class_a_p_i_u_i_1ae06af6625e15ef791506a5a2f83cfe71)`(const char * label,const char * filename,Soundfile ** sf_zone)` | 
`public inline virtual void `[`declare`](#class_a_p_i_u_i_1a637e96c1ae0a49fcb3dec055bdad1a3e)`(`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,const char * key,const char * val)` | 
`public inline virtual void `[`declare`](#class_a_p_i_u_i_1a50190b450306dd535381818d97d2b71e)`(const char * key,const char * val)` | 
`public inline int `[`getParamsCount`](#class_a_p_i_u_i_1a19e0d0815ee2476856a1e08efd5bc194)`()` | 
`public inline int `[`getParamIndex`](#class_a_p_i_u_i_1a3b50ffbef3726d7680ce5a6c55bb6671)`(const char * path)` | 
`public inline const char * `[`getParamAddress`](#class_a_p_i_u_i_1a3ded96ffee6ddf77397958fdf83bf5b3)`(int p)` | 
`public inline const char * `[`getParamLabel`](#class_a_p_i_u_i_1a208a58467f7c259cd93d1a3e424ac6d1)`(int p)` | 
`public inline std::map< const char *, const char * > `[`getMetadata`](#class_a_p_i_u_i_1a76e535353d095276b7f12d8caacce617)`(int p)` | 
`public inline const char * `[`getMetadata`](#class_a_p_i_u_i_1a14074c174394ab8abb305f59561e6002)`(int p,const char * key)` | 
`public inline `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` `[`getParamMin`](#class_a_p_i_u_i_1acf796b6fc897802d1b1c43353a33814e)`(int p)` | 
`public inline `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` `[`getParamMax`](#class_a_p_i_u_i_1adcb16b3b0be92699ff4c1577bd872082)`(int p)` | 
`public inline `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` `[`getParamStep`](#class_a_p_i_u_i_1ac4da605c9e707d22795dbece10831514)`(int p)` | 
`public inline `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` `[`getParamInit`](#class_a_p_i_u_i_1a2cd8ef169156cda28a829bf0933ad047)`(int p)` | 
`public inline `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * `[`getParamZone`](#class_a_p_i_u_i_1ad97f125f6847ce3053ef3a55f9e51d0a)`(int p)` | 
`public inline `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` `[`getParamValue`](#class_a_p_i_u_i_1aae0d4c6a114a90a14892d4272caff583)`(int p)` | 
`public inline void `[`setParamValue`](#class_a_p_i_u_i_1aa68864b832ee309c95b8aa26ecbb848e)`(int p,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` v)` | 
`public inline double `[`getParamRatio`](#class_a_p_i_u_i_1a37bc80f79af9f7ae92f4760c6225704f)`(int p)` | 
`public inline void `[`setParamRatio`](#class_a_p_i_u_i_1a27c42dc2a368be9398fad0a8a8d73f04)`(int p,double r)` | 
`public inline double `[`value2ratio`](#class_a_p_i_u_i_1a4b1972bfffec914f3889da0f3374c5b6)`(int p,double r)` | 
`public inline double `[`ratio2value`](#class_a_p_i_u_i_1aced32abc0c8fcc795c2d590c7a079cdb)`(int p,double r)` | 
`public inline `[`Type`](#class_a_p_i_u_i_1a8736a57e1caf23a981c94edf62679037)` `[`getParamType`](#class_a_p_i_u_i_1add17abc26d0be600afb7a4af8c9dac47)`(int p)` | Return the control type (kAcc, kGyr, or -1) for a given parameter
`public inline `[`ItemType`](#class_a_p_i_u_i_1a5da62c1673f075720c27ac4e8b7707a7)` `[`getParamItemType`](#class_a_p_i_u_i_1a3a0669fa719f593484ce6b4f0aba3abb)`(int p)` | Return the Item type (kButton = 0, kCheckButton, kVSlider, kHSlider, kNumEntry, kHBargraph, kVBargraph) for a given parameter
`public inline void `[`propagateAcc`](#class_a_p_i_u_i_1a177bb60475af9e1aa1bfd21e13ff9e11)`(int acc,double value)` | Set a new value coming from an accelerometer, propagate it to all relevant FAUSTFLOAT* zones.
`public inline void `[`setAccConverter`](#class_a_p_i_u_i_1aeb28d8a2624828d9e0e1e27c7f90eed0)`(int p,int acc,int curve,double amin,double amid,double amax)` | Used to edit accelerometer curves and mapping. Set curve and related mapping for a given [UI](#struct_u_i) parameter.
`public inline void `[`setGyrConverter`](#class_a_p_i_u_i_1ab8c4de21c31be69b73307afc0f68ec2c)`(int p,int gyr,int curve,double amin,double amid,double amax)` | Used to edit gyroscope curves and mapping. Set curve and related mapping for a given [UI](#struct_u_i) parameter.
`public inline void `[`getAccConverter`](#class_a_p_i_u_i_1a3b037049d8fd8e1568307c67183aeca4)`(int p,int & acc,int & curve,double & amin,double & amid,double & amax)` | Used to edit accelerometer curves and mapping. Get curve and related mapping for a given [UI](#struct_u_i) parameter.
`public inline void `[`getGyrConverter`](#class_a_p_i_u_i_1a09444c3b5c60b5ac64491a492b0dea68)`(int p,int & gyr,int & curve,double & amin,double & amid,double & amax)` | Used to edit gyroscope curves and mapping. Get curve and related mapping for a given [UI](#struct_u_i) parameter.
`public inline void `[`propagateGyr`](#class_a_p_i_u_i_1aa1f912157c1833b1bfd1c0ea7c6d3af9)`(int gyr,double value)` | Set a new value coming from an gyroscope, propagate it to all relevant FAUSTFLOAT* zones.
`public inline int `[`getAccCount`](#class_a_p_i_u_i_1a9c9b23dc3612b5e7407d2a440b7ad3a9)`(int acc)` | Get the number of FAUSTFLOAT* zones controlled with the accelerometer
`public inline int `[`getGyrCount`](#class_a_p_i_u_i_1a18b14af811c0af747f4f62e1ec8e66f2)`(int gyr)` | Get the number of FAUSTFLOAT* zones controlled with the gyroscope
`public inline int `[`getScreenColor`](#class_a_p_i_u_i_1adc75ff286b9e9c4fbc1f5d701d5dfb18)`()` | 
`protected int `[`fNumParameters`](#class_a_p_i_u_i_1a81872ca1355fd2337a492097ec8176be) | 
`protected std::vector< std::string > `[`fPaths`](#class_a_p_i_u_i_1a660888cab58e6b17f69c52a22e87b78e) | 
`protected std::vector< std::string > `[`fLabels`](#class_a_p_i_u_i_1a80a58402dcb944a5f4c1ce9bc1ba2a69) | 
`protected std::map< std::string, int > `[`fPathMap`](#class_a_p_i_u_i_1a316af0e416f5ae9e15bd1a0022065f1d) | 
`protected std::map< std::string, int > `[`fLabelMap`](#class_a_p_i_u_i_1ac82b3d0569733180ee4ec2e9cc3de70e) | 
`protected std::vector< `[`ValueConverter`](#class_value_converter)` * > `[`fConversion`](#class_a_p_i_u_i_1abfc01ca2b27dd184f9c279ba5d47f505) | 
`protected std::vector< `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * > `[`fZone`](#class_a_p_i_u_i_1ad5a55b5d39d95d0a37ae735cf7da58ad) | 
`protected std::vector< `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` > `[`fInit`](#class_a_p_i_u_i_1a67093639778014dc9a8ac32a146bf9a4) | 
`protected std::vector< `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` > `[`fMin`](#class_a_p_i_u_i_1a459ea23987331f48269ce6d320d044b9) | 
`protected std::vector< `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` > `[`fMax`](#class_a_p_i_u_i_1a074870fe9468c09ed69c40a36134df37) | 
`protected std::vector< `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` > `[`fStep`](#class_a_p_i_u_i_1ad044e423ecc6f608bac6f87a877a0d29) | 
`protected std::vector< `[`ItemType`](#class_a_p_i_u_i_1a5da62c1673f075720c27ac4e8b7707a7)` > `[`fItemType`](#class_a_p_i_u_i_1a86e3409033695a20eaddbcd4a9361c17) | 
`protected std::vector< std::map< std::string, std::string > > `[`fMetaData`](#class_a_p_i_u_i_1a761fd803b9bf6152fb838a06b4754c91) | 
`protected std::vector< `[`ZoneControl`](#class_zone_control)` * > `[`fAcc`](#class_a_p_i_u_i_1ac2fc3d6c7822503414f9d2d182dfdb70) | 
`protected std::vector< `[`ZoneControl`](#class_zone_control)` * > `[`fGyr`](#class_a_p_i_u_i_1a6264585e1ad8f07fd6815e72824a6baf) | 
`protected bool `[`fHasScreenControl`](#class_a_p_i_u_i_1ad12f8b68dd882038dffdd291a12bc4e8) | 
`protected `[`ZoneReader`](#class_zone_reader)` * `[`fRedReader`](#class_a_p_i_u_i_1a95255ef7a0c117ff2affa65f6e595d56) | 
`protected `[`ZoneReader`](#class_zone_reader)` * `[`fGreenReader`](#class_a_p_i_u_i_1a917e24dbaa47b9c7262f320f3d81d337) | 
`protected `[`ZoneReader`](#class_zone_reader)` * `[`fBlueReader`](#class_a_p_i_u_i_1aa9bf2f3def4cc126a7e0c01742b4bb34) | 
`protected std::string `[`fCurrentUnit`](#class_a_p_i_u_i_1ab9661cb0d83a2f4315ea909bee059334) | 
`protected int `[`fCurrentScale`](#class_a_p_i_u_i_1a796bc8306761a565b134be81e88054f9) | 
`protected std::string `[`fCurrentAcc`](#class_a_p_i_u_i_1a02ae9866a0bbbc181190681432d4f513) | 
`protected std::string `[`fCurrentGyr`](#class_a_p_i_u_i_1aa42434f2b16f15889c7d936ad0f3510b) | 
`protected std::string `[`fCurrentColor`](#class_a_p_i_u_i_1a8f75535a5c4b0306132f8b6930fb0c69) | 
`protected std::string `[`fCurrentTooltip`](#class_a_p_i_u_i_1a927310df41f545c187bd2baff55221ee) | 
`protected std::map< std::string, std::string > `[`fCurrentMetadata`](#class_a_p_i_u_i_1ab6d0ceeacf4d335d36faa437243c82b4) | 
`protected inline virtual void `[`addParameter`](#class_a_p_i_u_i_1a560a71f7e010e949567b7af368f2f694)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` init,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` min,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` max,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` step,`[`ItemType`](#class_a_p_i_u_i_1a5da62c1673f075720c27ac4e8b7707a7)` type)` | 
`protected inline int `[`getZoneIndex`](#class_a_p_i_u_i_1a359669d6d362067b80e1db512dac6736)`(std::vector< `[`ZoneControl`](#class_zone_control)` * > * table,int p,int val)` | 
`protected inline void `[`setConverter`](#class_a_p_i_u_i_1a9758619c3243e799773c003754733370)`(std::vector< `[`ZoneControl`](#class_zone_control)` * > * table,int p,int val,int curve,double amin,double amid,double amax)` | 
`protected inline void `[`getConverter`](#class_a_p_i_u_i_1a6e804316f2eac6e7ed3a055ff2d8d852)`(std::vector< `[`ZoneControl`](#class_zone_control)` * > * table,int p,int & val,int & curve,double & amin,double & amid,double & amax)` | 
`enum `[`ItemType`](#class_a_p_i_u_i_1a5da62c1673f075720c27ac4e8b7707a7) | 
`enum `[`Type`](#class_a_p_i_u_i_1a8736a57e1caf23a981c94edf62679037) | 

## Members

#### `public inline  `[`APIUI`](#class_a_p_i_u_i_1ac2c640ba8517d7d2daae6decfff5f67b)`()` {#class_a_p_i_u_i_1ac2c640ba8517d7d2daae6decfff5f67b}

#### `public inline virtual  `[`~APIUI`](#class_a_p_i_u_i_1a7dfac586920579e1f3f7ebee788dfe65)`()` {#class_a_p_i_u_i_1a7dfac586920579e1f3f7ebee788dfe65}

#### `public inline virtual void `[`openTabBox`](#class_a_p_i_u_i_1acddc57f8a18512011e204899b17a8964)`(const char * label)` {#class_a_p_i_u_i_1acddc57f8a18512011e204899b17a8964}

#### `public inline virtual void `[`openHorizontalBox`](#class_a_p_i_u_i_1aae31007b4ff2935f016d3e40f3e1f24e)`(const char * label)` {#class_a_p_i_u_i_1aae31007b4ff2935f016d3e40f3e1f24e}

#### `public inline virtual void `[`openVerticalBox`](#class_a_p_i_u_i_1aa305afc94f3be0b6ebe3b7403f0bfd40)`(const char * label)` {#class_a_p_i_u_i_1aa305afc94f3be0b6ebe3b7403f0bfd40}

#### `public inline virtual void `[`closeBox`](#class_a_p_i_u_i_1a25b531453e24de242de017d63c51bb56)`()` {#class_a_p_i_u_i_1a25b531453e24de242de017d63c51bb56}

#### `public inline virtual void `[`addButton`](#class_a_p_i_u_i_1a1e4e1fd2c09290ff273773ff80cd229a)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone)` {#class_a_p_i_u_i_1a1e4e1fd2c09290ff273773ff80cd229a}

#### `public inline virtual void `[`addCheckButton`](#class_a_p_i_u_i_1a641b5238578421ed6fb2972e26dd4242)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone)` {#class_a_p_i_u_i_1a641b5238578421ed6fb2972e26dd4242}

#### `public inline virtual void `[`addVerticalSlider`](#class_a_p_i_u_i_1a6974686a5292c2ae2800c316782161c3)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` init,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` min,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` max,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` step)` {#class_a_p_i_u_i_1a6974686a5292c2ae2800c316782161c3}

#### `public inline virtual void `[`addHorizontalSlider`](#class_a_p_i_u_i_1a564b927deb4caf9eabad60a35d65b99d)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` init,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` min,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` max,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` step)` {#class_a_p_i_u_i_1a564b927deb4caf9eabad60a35d65b99d}

#### `public inline virtual void `[`addNumEntry`](#class_a_p_i_u_i_1a70807925011a0337e9ee82a4efff0075)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` init,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` min,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` max,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` step)` {#class_a_p_i_u_i_1a70807925011a0337e9ee82a4efff0075}

#### `public inline virtual void `[`addHorizontalBargraph`](#class_a_p_i_u_i_1a6fd2909a888c47a6e2139f787580c2e8)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` min,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` max)` {#class_a_p_i_u_i_1a6fd2909a888c47a6e2139f787580c2e8}

#### `public inline virtual void `[`addVerticalBargraph`](#class_a_p_i_u_i_1ac52ecce0ab8f1ef0b50a383a5ee6b97c)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` min,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` max)` {#class_a_p_i_u_i_1ac52ecce0ab8f1ef0b50a383a5ee6b97c}

#### `public inline virtual void `[`addSoundfile`](#class_a_p_i_u_i_1ae06af6625e15ef791506a5a2f83cfe71)`(const char * label,const char * filename,Soundfile ** sf_zone)` {#class_a_p_i_u_i_1ae06af6625e15ef791506a5a2f83cfe71}

#### `public inline virtual void `[`declare`](#class_a_p_i_u_i_1a637e96c1ae0a49fcb3dec055bdad1a3e)`(`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,const char * key,const char * val)` {#class_a_p_i_u_i_1a637e96c1ae0a49fcb3dec055bdad1a3e}

#### `public inline virtual void `[`declare`](#class_a_p_i_u_i_1a50190b450306dd535381818d97d2b71e)`(const char * key,const char * val)` {#class_a_p_i_u_i_1a50190b450306dd535381818d97d2b71e}

#### `public inline int `[`getParamsCount`](#class_a_p_i_u_i_1a19e0d0815ee2476856a1e08efd5bc194)`()` {#class_a_p_i_u_i_1a19e0d0815ee2476856a1e08efd5bc194}

#### `public inline int `[`getParamIndex`](#class_a_p_i_u_i_1a3b50ffbef3726d7680ce5a6c55bb6671)`(const char * path)` {#class_a_p_i_u_i_1a3b50ffbef3726d7680ce5a6c55bb6671}

#### `public inline const char * `[`getParamAddress`](#class_a_p_i_u_i_1a3ded96ffee6ddf77397958fdf83bf5b3)`(int p)` {#class_a_p_i_u_i_1a3ded96ffee6ddf77397958fdf83bf5b3}

#### `public inline const char * `[`getParamLabel`](#class_a_p_i_u_i_1a208a58467f7c259cd93d1a3e424ac6d1)`(int p)` {#class_a_p_i_u_i_1a208a58467f7c259cd93d1a3e424ac6d1}

#### `public inline std::map< const char *, const char * > `[`getMetadata`](#class_a_p_i_u_i_1a76e535353d095276b7f12d8caacce617)`(int p)` {#class_a_p_i_u_i_1a76e535353d095276b7f12d8caacce617}

#### `public inline const char * `[`getMetadata`](#class_a_p_i_u_i_1a14074c174394ab8abb305f59561e6002)`(int p,const char * key)` {#class_a_p_i_u_i_1a14074c174394ab8abb305f59561e6002}

#### `public inline `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` `[`getParamMin`](#class_a_p_i_u_i_1acf796b6fc897802d1b1c43353a33814e)`(int p)` {#class_a_p_i_u_i_1acf796b6fc897802d1b1c43353a33814e}

#### `public inline `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` `[`getParamMax`](#class_a_p_i_u_i_1adcb16b3b0be92699ff4c1577bd872082)`(int p)` {#class_a_p_i_u_i_1adcb16b3b0be92699ff4c1577bd872082}

#### `public inline `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` `[`getParamStep`](#class_a_p_i_u_i_1ac4da605c9e707d22795dbece10831514)`(int p)` {#class_a_p_i_u_i_1ac4da605c9e707d22795dbece10831514}

#### `public inline `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` `[`getParamInit`](#class_a_p_i_u_i_1a2cd8ef169156cda28a829bf0933ad047)`(int p)` {#class_a_p_i_u_i_1a2cd8ef169156cda28a829bf0933ad047}

#### `public inline `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * `[`getParamZone`](#class_a_p_i_u_i_1ad97f125f6847ce3053ef3a55f9e51d0a)`(int p)` {#class_a_p_i_u_i_1ad97f125f6847ce3053ef3a55f9e51d0a}

#### `public inline `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` `[`getParamValue`](#class_a_p_i_u_i_1aae0d4c6a114a90a14892d4272caff583)`(int p)` {#class_a_p_i_u_i_1aae0d4c6a114a90a14892d4272caff583}

#### `public inline void `[`setParamValue`](#class_a_p_i_u_i_1aa68864b832ee309c95b8aa26ecbb848e)`(int p,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` v)` {#class_a_p_i_u_i_1aa68864b832ee309c95b8aa26ecbb848e}

#### `public inline double `[`getParamRatio`](#class_a_p_i_u_i_1a37bc80f79af9f7ae92f4760c6225704f)`(int p)` {#class_a_p_i_u_i_1a37bc80f79af9f7ae92f4760c6225704f}

#### `public inline void `[`setParamRatio`](#class_a_p_i_u_i_1a27c42dc2a368be9398fad0a8a8d73f04)`(int p,double r)` {#class_a_p_i_u_i_1a27c42dc2a368be9398fad0a8a8d73f04}

#### `public inline double `[`value2ratio`](#class_a_p_i_u_i_1a4b1972bfffec914f3889da0f3374c5b6)`(int p,double r)` {#class_a_p_i_u_i_1a4b1972bfffec914f3889da0f3374c5b6}

#### `public inline double `[`ratio2value`](#class_a_p_i_u_i_1aced32abc0c8fcc795c2d590c7a079cdb)`(int p,double r)` {#class_a_p_i_u_i_1aced32abc0c8fcc795c2d590c7a079cdb}

#### `public inline `[`Type`](#class_a_p_i_u_i_1a8736a57e1caf23a981c94edf62679037)` `[`getParamType`](#class_a_p_i_u_i_1add17abc26d0be600afb7a4af8c9dac47)`(int p)` {#class_a_p_i_u_i_1add17abc26d0be600afb7a4af8c9dac47}

Return the control type (kAcc, kGyr, or -1) for a given parameter

#### Parameters
* `p` - the [UI](#struct_u_i) parameter index

#### Returns
the type

#### `public inline `[`ItemType`](#class_a_p_i_u_i_1a5da62c1673f075720c27ac4e8b7707a7)` `[`getParamItemType`](#class_a_p_i_u_i_1a3a0669fa719f593484ce6b4f0aba3abb)`(int p)` {#class_a_p_i_u_i_1a3a0669fa719f593484ce6b4f0aba3abb}

Return the Item type (kButton = 0, kCheckButton, kVSlider, kHSlider, kNumEntry, kHBargraph, kVBargraph) for a given parameter

#### Parameters
* `p` - the [UI](#struct_u_i) parameter index

#### Returns
the Item type

#### `public inline void `[`propagateAcc`](#class_a_p_i_u_i_1a177bb60475af9e1aa1bfd21e13ff9e11)`(int acc,double value)` {#class_a_p_i_u_i_1a177bb60475af9e1aa1bfd21e13ff9e11}

Set a new value coming from an accelerometer, propagate it to all relevant FAUSTFLOAT* zones.

#### Parameters
* `acc` - 0 for X accelerometer, 1 for Y accelerometer, 2 for Z accelerometer 

* `value` - the new value

#### `public inline void `[`setAccConverter`](#class_a_p_i_u_i_1aeb28d8a2624828d9e0e1e27c7f90eed0)`(int p,int acc,int curve,double amin,double amid,double amax)` {#class_a_p_i_u_i_1aeb28d8a2624828d9e0e1e27c7f90eed0}

Used to edit accelerometer curves and mapping. Set curve and related mapping for a given [UI](#struct_u_i) parameter.

#### Parameters
* `p` - the [UI](#struct_u_i) parameter index 

* `acc` - 0 for X accelerometer, 1 for Y accelerometer, 2 for Z accelerometer (-1 means "no mapping") 

* `curve` - between 0 and 3 

* `amin` - mapping 'min' point 

* `amid` - mapping 'middle' point 

* `amax` - mapping 'max' point

#### `public inline void `[`setGyrConverter`](#class_a_p_i_u_i_1ab8c4de21c31be69b73307afc0f68ec2c)`(int p,int gyr,int curve,double amin,double amid,double amax)` {#class_a_p_i_u_i_1ab8c4de21c31be69b73307afc0f68ec2c}

Used to edit gyroscope curves and mapping. Set curve and related mapping for a given [UI](#struct_u_i) parameter.

#### Parameters
* `p` - the [UI](#struct_u_i) parameter index 

* `acc` - 0 for X gyroscope, 1 for Y gyroscope, 2 for Z gyroscope (-1 means "no mapping") 

* `curve` - between 0 and 3 

* `amin` - mapping 'min' point 

* `amid` - mapping 'middle' point 

* `amax` - mapping 'max' point

#### `public inline void `[`getAccConverter`](#class_a_p_i_u_i_1a3b037049d8fd8e1568307c67183aeca4)`(int p,int & acc,int & curve,double & amin,double & amid,double & amax)` {#class_a_p_i_u_i_1a3b037049d8fd8e1568307c67183aeca4}

Used to edit accelerometer curves and mapping. Get curve and related mapping for a given [UI](#struct_u_i) parameter.

#### Parameters
* `p` - the [UI](#struct_u_i) parameter index 

* `acc` - the acc value to be retrieved (-1 means "no mapping") 

* `curve` - the curve value to be retrieved 

* `amin` - the amin value to be retrieved 

* `amid` - the amid value to be retrieved 

* `amax` - the amax value to be retrieved

#### `public inline void `[`getGyrConverter`](#class_a_p_i_u_i_1a09444c3b5c60b5ac64491a492b0dea68)`(int p,int & gyr,int & curve,double & amin,double & amid,double & amax)` {#class_a_p_i_u_i_1a09444c3b5c60b5ac64491a492b0dea68}

Used to edit gyroscope curves and mapping. Get curve and related mapping for a given [UI](#struct_u_i) parameter.

#### Parameters
* `p` - the [UI](#struct_u_i) parameter index 

* `gyr` - the gyr value to be retrieved (-1 means "no mapping") 

* `curve` - the curve value to be retrieved 

* `amin` - the amin value to be retrieved 

* `amid` - the amid value to be retrieved 

* `amax` - the amax value to be retrieved

#### `public inline void `[`propagateGyr`](#class_a_p_i_u_i_1aa1f912157c1833b1bfd1c0ea7c6d3af9)`(int gyr,double value)` {#class_a_p_i_u_i_1aa1f912157c1833b1bfd1c0ea7c6d3af9}

Set a new value coming from an gyroscope, propagate it to all relevant FAUSTFLOAT* zones.

#### Parameters
* `gyr` - 0 for X gyroscope, 1 for Y gyroscope, 2 for Z gyroscope 

* `value` - the new value

#### `public inline int `[`getAccCount`](#class_a_p_i_u_i_1a9c9b23dc3612b5e7407d2a440b7ad3a9)`(int acc)` {#class_a_p_i_u_i_1a9c9b23dc3612b5e7407d2a440b7ad3a9}

Get the number of FAUSTFLOAT* zones controlled with the accelerometer

#### Parameters
* `acc` - 0 for X accelerometer, 1 for Y accelerometer, 2 for Z accelerometer 

#### Returns
the number of zones

#### `public inline int `[`getGyrCount`](#class_a_p_i_u_i_1a18b14af811c0af747f4f62e1ec8e66f2)`(int gyr)` {#class_a_p_i_u_i_1a18b14af811c0af747f4f62e1ec8e66f2}

Get the number of FAUSTFLOAT* zones controlled with the gyroscope

#### Parameters
* `gyr` - 0 for X gyroscope, 1 for Y gyroscope, 2 for Z gyroscope 

* `the` number of zones

#### `public inline int `[`getScreenColor`](#class_a_p_i_u_i_1adc75ff286b9e9c4fbc1f5d701d5dfb18)`()` {#class_a_p_i_u_i_1adc75ff286b9e9c4fbc1f5d701d5dfb18}

#### `protected int `[`fNumParameters`](#class_a_p_i_u_i_1a81872ca1355fd2337a492097ec8176be) {#class_a_p_i_u_i_1a81872ca1355fd2337a492097ec8176be}

#### `protected std::vector< std::string > `[`fPaths`](#class_a_p_i_u_i_1a660888cab58e6b17f69c52a22e87b78e) {#class_a_p_i_u_i_1a660888cab58e6b17f69c52a22e87b78e}

#### `protected std::vector< std::string > `[`fLabels`](#class_a_p_i_u_i_1a80a58402dcb944a5f4c1ce9bc1ba2a69) {#class_a_p_i_u_i_1a80a58402dcb944a5f4c1ce9bc1ba2a69}

#### `protected std::map< std::string, int > `[`fPathMap`](#class_a_p_i_u_i_1a316af0e416f5ae9e15bd1a0022065f1d) {#class_a_p_i_u_i_1a316af0e416f5ae9e15bd1a0022065f1d}

#### `protected std::map< std::string, int > `[`fLabelMap`](#class_a_p_i_u_i_1ac82b3d0569733180ee4ec2e9cc3de70e) {#class_a_p_i_u_i_1ac82b3d0569733180ee4ec2e9cc3de70e}

#### `protected std::vector< `[`ValueConverter`](#class_value_converter)` * > `[`fConversion`](#class_a_p_i_u_i_1abfc01ca2b27dd184f9c279ba5d47f505) {#class_a_p_i_u_i_1abfc01ca2b27dd184f9c279ba5d47f505}

#### `protected std::vector< `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * > `[`fZone`](#class_a_p_i_u_i_1ad5a55b5d39d95d0a37ae735cf7da58ad) {#class_a_p_i_u_i_1ad5a55b5d39d95d0a37ae735cf7da58ad}

#### `protected std::vector< `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` > `[`fInit`](#class_a_p_i_u_i_1a67093639778014dc9a8ac32a146bf9a4) {#class_a_p_i_u_i_1a67093639778014dc9a8ac32a146bf9a4}

#### `protected std::vector< `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` > `[`fMin`](#class_a_p_i_u_i_1a459ea23987331f48269ce6d320d044b9) {#class_a_p_i_u_i_1a459ea23987331f48269ce6d320d044b9}

#### `protected std::vector< `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` > `[`fMax`](#class_a_p_i_u_i_1a074870fe9468c09ed69c40a36134df37) {#class_a_p_i_u_i_1a074870fe9468c09ed69c40a36134df37}

#### `protected std::vector< `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` > `[`fStep`](#class_a_p_i_u_i_1ad044e423ecc6f608bac6f87a877a0d29) {#class_a_p_i_u_i_1ad044e423ecc6f608bac6f87a877a0d29}

#### `protected std::vector< `[`ItemType`](#class_a_p_i_u_i_1a5da62c1673f075720c27ac4e8b7707a7)` > `[`fItemType`](#class_a_p_i_u_i_1a86e3409033695a20eaddbcd4a9361c17) {#class_a_p_i_u_i_1a86e3409033695a20eaddbcd4a9361c17}

#### `protected std::vector< std::map< std::string, std::string > > `[`fMetaData`](#class_a_p_i_u_i_1a761fd803b9bf6152fb838a06b4754c91) {#class_a_p_i_u_i_1a761fd803b9bf6152fb838a06b4754c91}

#### `protected std::vector< `[`ZoneControl`](#class_zone_control)` * > `[`fAcc`](#class_a_p_i_u_i_1ac2fc3d6c7822503414f9d2d182dfdb70) {#class_a_p_i_u_i_1ac2fc3d6c7822503414f9d2d182dfdb70}

#### `protected std::vector< `[`ZoneControl`](#class_zone_control)` * > `[`fGyr`](#class_a_p_i_u_i_1a6264585e1ad8f07fd6815e72824a6baf) {#class_a_p_i_u_i_1a6264585e1ad8f07fd6815e72824a6baf}

#### `protected bool `[`fHasScreenControl`](#class_a_p_i_u_i_1ad12f8b68dd882038dffdd291a12bc4e8) {#class_a_p_i_u_i_1ad12f8b68dd882038dffdd291a12bc4e8}

#### `protected `[`ZoneReader`](#class_zone_reader)` * `[`fRedReader`](#class_a_p_i_u_i_1a95255ef7a0c117ff2affa65f6e595d56) {#class_a_p_i_u_i_1a95255ef7a0c117ff2affa65f6e595d56}

#### `protected `[`ZoneReader`](#class_zone_reader)` * `[`fGreenReader`](#class_a_p_i_u_i_1a917e24dbaa47b9c7262f320f3d81d337) {#class_a_p_i_u_i_1a917e24dbaa47b9c7262f320f3d81d337}

#### `protected `[`ZoneReader`](#class_zone_reader)` * `[`fBlueReader`](#class_a_p_i_u_i_1aa9bf2f3def4cc126a7e0c01742b4bb34) {#class_a_p_i_u_i_1aa9bf2f3def4cc126a7e0c01742b4bb34}

#### `protected std::string `[`fCurrentUnit`](#class_a_p_i_u_i_1ab9661cb0d83a2f4315ea909bee059334) {#class_a_p_i_u_i_1ab9661cb0d83a2f4315ea909bee059334}

#### `protected int `[`fCurrentScale`](#class_a_p_i_u_i_1a796bc8306761a565b134be81e88054f9) {#class_a_p_i_u_i_1a796bc8306761a565b134be81e88054f9}

#### `protected std::string `[`fCurrentAcc`](#class_a_p_i_u_i_1a02ae9866a0bbbc181190681432d4f513) {#class_a_p_i_u_i_1a02ae9866a0bbbc181190681432d4f513}

#### `protected std::string `[`fCurrentGyr`](#class_a_p_i_u_i_1aa42434f2b16f15889c7d936ad0f3510b) {#class_a_p_i_u_i_1aa42434f2b16f15889c7d936ad0f3510b}

#### `protected std::string `[`fCurrentColor`](#class_a_p_i_u_i_1a8f75535a5c4b0306132f8b6930fb0c69) {#class_a_p_i_u_i_1a8f75535a5c4b0306132f8b6930fb0c69}

#### `protected std::string `[`fCurrentTooltip`](#class_a_p_i_u_i_1a927310df41f545c187bd2baff55221ee) {#class_a_p_i_u_i_1a927310df41f545c187bd2baff55221ee}

#### `protected std::map< std::string, std::string > `[`fCurrentMetadata`](#class_a_p_i_u_i_1ab6d0ceeacf4d335d36faa437243c82b4) {#class_a_p_i_u_i_1ab6d0ceeacf4d335d36faa437243c82b4}

#### `protected inline virtual void `[`addParameter`](#class_a_p_i_u_i_1a560a71f7e010e949567b7af368f2f694)`(const char * label,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` init,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` min,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` max,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` step,`[`ItemType`](#class_a_p_i_u_i_1a5da62c1673f075720c27ac4e8b7707a7)` type)` {#class_a_p_i_u_i_1a560a71f7e010e949567b7af368f2f694}

#### `protected inline int `[`getZoneIndex`](#class_a_p_i_u_i_1a359669d6d362067b80e1db512dac6736)`(std::vector< `[`ZoneControl`](#class_zone_control)` * > * table,int p,int val)` {#class_a_p_i_u_i_1a359669d6d362067b80e1db512dac6736}

#### `protected inline void `[`setConverter`](#class_a_p_i_u_i_1a9758619c3243e799773c003754733370)`(std::vector< `[`ZoneControl`](#class_zone_control)` * > * table,int p,int val,int curve,double amin,double amid,double amax)` {#class_a_p_i_u_i_1a9758619c3243e799773c003754733370}

#### `protected inline void `[`getConverter`](#class_a_p_i_u_i_1a6e804316f2eac6e7ed3a055ff2d8d852)`(std::vector< `[`ZoneControl`](#class_zone_control)` * > * table,int p,int & val,int & curve,double & amin,double & amid,double & amax)` {#class_a_p_i_u_i_1a6e804316f2eac6e7ed3a055ff2d8d852}

#### `enum `[`ItemType`](#class_a_p_i_u_i_1a5da62c1673f075720c27ac4e8b7707a7) {#class_a_p_i_u_i_1a5da62c1673f075720c27ac4e8b7707a7}

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
kButton            | 
kCheckButton            | 
kVSlider            | 
kHSlider            | 
kNumEntry            | 
kHBargraph            | 
kVBargraph            | 
kButton            | 
kCheckButton            | 
kVSlider            | 
kHSlider            | 
kNumEntry            | 
kHBargraph            | 
kVBargraph            | 
kButton            | 
kCheckButton            | 
kVSlider            | 
kHSlider            | 
kNumEntry            | 
kHBargraph            | 
kVBargraph            | 
kButton            | 
kCheckButton            | 
kVSlider            | 
kHSlider            | 
kNumEntry            | 
kHBargraph            | 
kVBargraph            | 
kButton            | 
kCheckButton            | 
kVSlider            | 
kHSlider            | 
kNumEntry            | 
kHBargraph            | 
kVBargraph            | 
kButton            | 
kCheckButton            | 
kVSlider            | 
kHSlider            | 
kNumEntry            | 
kHBargraph            | 
kVBargraph            | 

#### `enum `[`Type`](#class_a_p_i_u_i_1a8736a57e1caf23a981c94edf62679037) {#class_a_p_i_u_i_1a8736a57e1caf23a981c94edf62679037}

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
kAcc            | 
kGyr            | 
kNoType            | 
kAcc            | 
kGyr            | 
kNoType            | 
kAcc            | 
kGyr            | 
kNoType            | 
kAcc            | 
kGyr            | 
kNoType            | 
kAcc            | 
kGyr            | 
kNoType            | 
kAcc            | 
kGyr            | 
kNoType            | 

# class `AudioInterface` {#class_audio_interface}

Base Class that provides an interface with audio.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  `[`AudioInterface`](#class_audio_interface_1ae20746f51bee2913cece514d74cf6d54)`(`[`JackTrip`](#class_jack_trip)` * jacktrip,int NumInChans,int NumOutChans,`[`AudioInterface::audioBitResolutionT`](#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b)` AudioBitResolution)` | The class constructor.
`public virtual  `[`~AudioInterface`](#class_audio_interface_1a985204462880bbacf04c04ce538d9963)`()` | The class destructor.
`public virtual void `[`setup`](#class_audio_interface_1a898f0033f9a6d6d60b5c2f847c53e07a)`()` | Setup the client. This function should be called just before.
`public int `[`startProcess`](#class_audio_interface_1a4d3d0cd118384ed23e85f671ff4f43f6)`() const` | Tell the audio server that we are ready to roll. The process-callback will start running. This runs on its own thread.
`public int `[`stopProcess`](#class_audio_interface_1a2b1ecf7a11e8a92619ab9b35d675b9c7)`() const` | Stops the process-callback thread.
`public virtual void `[`broadcastCallback`](#class_audio_interface_1a8b9017ce11a9e0185e6208c64ae35915)`(QVarLengthArray< `[`sample_t`](#jacktrip__types_8h_1aa0798f7ea0975b143a1d9deac8c05d43)` * > & mon_buffer,unsigned int n_frames)` | Process callback. Subclass should call this callback after obtaining the in_buffer and out_buffer pointers.
`public virtual void `[`callback`](#class_audio_interface_1a5319176a25341ed14dfaae5f875375f2)`(QVarLengthArray< `[`sample_t`](#jacktrip__types_8h_1aa0798f7ea0975b143a1d9deac8c05d43)` * > & in_buffer,QVarLengthArray< `[`sample_t`](#jacktrip__types_8h_1aa0798f7ea0975b143a1d9deac8c05d43)` * > & out_buffer,unsigned int n_frames)` | 
`public virtual void `[`appendProcessPluginToNetwork`](#class_audio_interface_1a582bdaa7555473c7ccad02645b6c0c47)`(`[`ProcessPlugin`](#class_process_plugin)` * plugin)` | [appendProcessPluginToNetwork()](#class_audio_interface_1a582bdaa7555473c7ccad02645b6c0c47): Append a [ProcessPlugin](#class_process_plugin) for outgoing audio. The processing order equals order they were appended. This processing is in the [JackTrip](#class_jack_trip) client before sending to the network.
`public virtual void `[`appendProcessPluginFromNetwork`](#class_audio_interface_1a0b84c7b4b1b12ca655bc830b9cc62635)`(`[`ProcessPlugin`](#class_process_plugin)` * plugin)` | [appendProcessPluginFromNetwork()](#class_audio_interface_1a0b84c7b4b1b12ca655bc830b9cc62635): Same as [appendProcessPluginToNetwork()](#class_audio_interface_1a582bdaa7555473c7ccad02645b6c0c47) except that these plugins operate on the audio received from the network (typically from a [JackTrip](#class_jack_trip) server). The complete processing chain then looks like this: audio -> JACK -> [JackTrip](#class_jack_trip) client -> processPlugin to network -> remote [JackTrip](#class_jack_trip) server -> [JackTrip](#class_jack_trip) client -> processPlugin from network -> JACK -> audio
`public void `[`initPlugins`](#class_audio_interface_1af58b303ae4933dc6ffe0cd2e535b5354)`()` | [initPlugins()](#class_audio_interface_1af58b303ae4933dc6ffe0cd2e535b5354): Initialize all [ProcessPlugin](#class_process_plugin) modules. The audio sampling rate (mSampleRate) must be set at this time.
`public void `[`connectDefaultPorts`](#class_audio_interface_1abbb4f698d0863a038e8156208f917a5b)`()` | 
`public inline virtual void `[`setNumInputChannels`](#class_audio_interface_1abdc6a04f4be17b8c7a790f265e985107)`(int nchannels)` | 
`public inline virtual void `[`setNumOutputChannels`](#class_audio_interface_1a7e9f52c6685f2d24968ef4482dc53bcc)`(int nchannels)` | 
`public inline virtual void `[`setSampleRate`](#class_audio_interface_1af66437c4c3c541abf27248f5aff74754)`(`[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` sample_rate)` | 
`public inline virtual void `[`setDeviceID`](#class_audio_interface_1aeb34ea63c907e397f3f33463adedcfa3)`(`[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` device_id)` | 
`public inline virtual void `[`setBufferSizeInSamples`](#class_audio_interface_1a89d932ccf615dfd6477a87c2b97c437d)`(`[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` buf_size)` | 
`public void `[`setClientName`](#class_audio_interface_1ae5661866c8b1a712ba7a0972ba9623cc)`(QString ClientName)` | Set Client Name to something different that the default ([JackTrip](#class_jack_trip))
`public inline virtual void `[`setLoopBack`](#class_audio_interface_1af8148bc160f637ebed6c3351d6a2c7b6)`(bool b)` | 
`public inline virtual void `[`enableBroadcastOutput`](#class_audio_interface_1ac5cfa001682e6e0826c63b5f7d6350a8)`()` | 
`public inline virtual void `[`setAudioTesterP`](#class_audio_interface_1a7b2a6673220bcb19ada364d629e0c1da)`(`[`AudioTester`](#class_audio_tester)` * atp)` | 
`public inline virtual int `[`getNumInputChannels`](#class_audio_interface_1a6c05fbe2cf820b4a764d7a905c61e5e6)`() const` | Get Number of Input Channels.
`public inline virtual int `[`getNumOutputChannels`](#class_audio_interface_1a4f1dc0048bbf4aa85819f464647df07f)`() const` | Get Number of Output Channels.
`public inline virtual `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`getBufferSizeInSamples`](#class_audio_interface_1af0a0e609826e67d47ca130641ca07fc0)`() const` | 
`public inline virtual `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`getDeviceID`](#class_audio_interface_1a7924a1fc68992f5e46cfd766d45dfe89)`() const` | 
`public virtual size_t `[`getSizeInBytesPerChannel`](#class_audio_interface_1a38393b4d8e032c5e834c888d2697a1ab)`() const` | 
`public inline virtual `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`getSampleRate`](#class_audio_interface_1a1d6abfe44a1e13976aff878bf5e22e9e)`() const` | Get the Jack Server Sampling Rate, in samples/second.
`public virtual `[`samplingRateT`](#class_audio_interface_1a538ea04ceee63f418c629d6a3a0e4fef)` `[`getSampleRateType`](#class_audio_interface_1a2454d2f0b02a67b7fab00481877005a9)`() const` | Get the Jack Server Sampling Rate Enum Type samplingRateT.
`public inline virtual int `[`getAudioBitResolution`](#class_audio_interface_1a728fb37319785ace1c42605d955cab7c)`() const` | Get the Audio Bit Resolution, in bits.
`protected bool `[`mProcessingAudio`](#class_audio_interface_1aa1104803afb9fef3b29dc128ee279d60) | Set when processing an audio callback buffer pair.
`protected const `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`MAX_AUDIO_BUFFER_SIZE`](#class_audio_interface_1a512aaddf196aed8dbbbe46d13ab6499b) | 
`enum `[`audioBitResolutionT`](#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b) | Enum for Audio Resolution in bits.
`enum `[`samplingRateT`](#class_audio_interface_1a538ea04ceee63f418c629d6a3a0e4fef) | Sampling Rates supported by JACK.

## Members

#### `public  `[`AudioInterface`](#class_audio_interface_1ae20746f51bee2913cece514d74cf6d54)`(`[`JackTrip`](#class_jack_trip)` * jacktrip,int NumInChans,int NumOutChans,`[`AudioInterface::audioBitResolutionT`](#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b)` AudioBitResolution)` {#class_audio_interface_1ae20746f51bee2913cece514d74cf6d54}

The class constructor.

#### Parameters
* `jacktrip` Pointer to the [JackTrip](#class_jack_trip) class that connects all classes (mediator) 

* `NumInChans` Number of Input Channels 

* `NumOutChans` Number of Output Channels 

* `AudioBitResolution` Audio Sample Resolutions in bits

#### `public virtual  `[`~AudioInterface`](#class_audio_interface_1a985204462880bbacf04c04ce538d9963)`()` {#class_audio_interface_1a985204462880bbacf04c04ce538d9963}

The class destructor.

#### `public virtual void `[`setup`](#class_audio_interface_1a898f0033f9a6d6d60b5c2f847c53e07a)`()` {#class_audio_interface_1a898f0033f9a6d6d60b5c2f847c53e07a}

Setup the client. This function should be called just before.

starting the audio processes, it will setup the audio client with the class parameters, like Sampling Rate, Packet Size, Bit Resolution, etc... Sub-classes should also call the parent method to ensure correct inizialization.

#### `public int `[`startProcess`](#class_audio_interface_1a4d3d0cd118384ed23e85f671ff4f43f6)`() const` {#class_audio_interface_1a4d3d0cd118384ed23e85f671ff4f43f6}

Tell the audio server that we are ready to roll. The process-callback will start running. This runs on its own thread.

#### Returns
0 on success, otherwise a non-zero error code

#### `public int `[`stopProcess`](#class_audio_interface_1a2b1ecf7a11e8a92619ab9b35d675b9c7)`() const` {#class_audio_interface_1a2b1ecf7a11e8a92619ab9b35d675b9c7}

Stops the process-callback thread.

#### Returns
0 on success, otherwise a non-zero error code

#### `public virtual void `[`broadcastCallback`](#class_audio_interface_1a8b9017ce11a9e0185e6208c64ae35915)`(QVarLengthArray< `[`sample_t`](#jacktrip__types_8h_1aa0798f7ea0975b143a1d9deac8c05d43)` * > & mon_buffer,unsigned int n_frames)` {#class_audio_interface_1a8b9017ce11a9e0185e6208c64ae35915}

Process callback. Subclass should call this callback after obtaining the in_buffer and out_buffer pointers.

#### Parameters
* `in_buffer` Array of input audio samplers for each channel. The user is reponsible to check that each channel has n_frames samplers 

* `in_buffer` Array of output audio samplers for each channel. The user is reponsible to check that each channel has n_frames samplers

#### `public virtual void `[`callback`](#class_audio_interface_1a5319176a25341ed14dfaae5f875375f2)`(QVarLengthArray< `[`sample_t`](#jacktrip__types_8h_1aa0798f7ea0975b143a1d9deac8c05d43)` * > & in_buffer,QVarLengthArray< `[`sample_t`](#jacktrip__types_8h_1aa0798f7ea0975b143a1d9deac8c05d43)` * > & out_buffer,unsigned int n_frames)` {#class_audio_interface_1a5319176a25341ed14dfaae5f875375f2}

#### `public virtual void `[`appendProcessPluginToNetwork`](#class_audio_interface_1a582bdaa7555473c7ccad02645b6c0c47)`(`[`ProcessPlugin`](#class_process_plugin)` * plugin)` {#class_audio_interface_1a582bdaa7555473c7ccad02645b6c0c47}

[appendProcessPluginToNetwork()](#class_audio_interface_1a582bdaa7555473c7ccad02645b6c0c47): Append a [ProcessPlugin](#class_process_plugin) for outgoing audio. The processing order equals order they were appended. This processing is in the [JackTrip](#class_jack_trip) client before sending to the network.

#### Parameters
* `plugin` a [ProcessPlugin](#class_process_plugin) smart pointer. Create the object instance using something like:`std::tr1::shared_ptr<ProcessPluginName> loopback(new ProcessPluginName);`

#### `public virtual void `[`appendProcessPluginFromNetwork`](#class_audio_interface_1a0b84c7b4b1b12ca655bc830b9cc62635)`(`[`ProcessPlugin`](#class_process_plugin)` * plugin)` {#class_audio_interface_1a0b84c7b4b1b12ca655bc830b9cc62635}

[appendProcessPluginFromNetwork()](#class_audio_interface_1a0b84c7b4b1b12ca655bc830b9cc62635): Same as [appendProcessPluginToNetwork()](#class_audio_interface_1a582bdaa7555473c7ccad02645b6c0c47) except that these plugins operate on the audio received from the network (typically from a [JackTrip](#class_jack_trip) server). The complete processing chain then looks like this: audio -> JACK -> [JackTrip](#class_jack_trip) client -> processPlugin to network -> remote [JackTrip](#class_jack_trip) server -> [JackTrip](#class_jack_trip) client -> processPlugin from network -> JACK -> audio

#### `public void `[`initPlugins`](#class_audio_interface_1af58b303ae4933dc6ffe0cd2e535b5354)`()` {#class_audio_interface_1af58b303ae4933dc6ffe0cd2e535b5354}

[initPlugins()](#class_audio_interface_1af58b303ae4933dc6ffe0cd2e535b5354): Initialize all [ProcessPlugin](#class_process_plugin) modules. The audio sampling rate (mSampleRate) must be set at this time.

#### `public void `[`connectDefaultPorts`](#class_audio_interface_1abbb4f698d0863a038e8156208f917a5b)`()` {#class_audio_interface_1abbb4f698d0863a038e8156208f917a5b}

#### `public inline virtual void `[`setNumInputChannels`](#class_audio_interface_1abdc6a04f4be17b8c7a790f265e985107)`(int nchannels)` {#class_audio_interface_1abdc6a04f4be17b8c7a790f265e985107}

#### `public inline virtual void `[`setNumOutputChannels`](#class_audio_interface_1a7e9f52c6685f2d24968ef4482dc53bcc)`(int nchannels)` {#class_audio_interface_1a7e9f52c6685f2d24968ef4482dc53bcc}

#### `public inline virtual void `[`setSampleRate`](#class_audio_interface_1af66437c4c3c541abf27248f5aff74754)`(`[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` sample_rate)` {#class_audio_interface_1af66437c4c3c541abf27248f5aff74754}

#### `public inline virtual void `[`setDeviceID`](#class_audio_interface_1aeb34ea63c907e397f3f33463adedcfa3)`(`[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` device_id)` {#class_audio_interface_1aeb34ea63c907e397f3f33463adedcfa3}

#### `public inline virtual void `[`setBufferSizeInSamples`](#class_audio_interface_1a89d932ccf615dfd6477a87c2b97c437d)`(`[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` buf_size)` {#class_audio_interface_1a89d932ccf615dfd6477a87c2b97c437d}

#### `public void `[`setClientName`](#class_audio_interface_1ae5661866c8b1a712ba7a0972ba9623cc)`(QString ClientName)` {#class_audio_interface_1ae5661866c8b1a712ba7a0972ba9623cc}

Set Client Name to something different that the default ([JackTrip](#class_jack_trip))

#### `public inline virtual void `[`setLoopBack`](#class_audio_interface_1af8148bc160f637ebed6c3351d6a2c7b6)`(bool b)` {#class_audio_interface_1af8148bc160f637ebed6c3351d6a2c7b6}

#### `public inline virtual void `[`enableBroadcastOutput`](#class_audio_interface_1ac5cfa001682e6e0826c63b5f7d6350a8)`()` {#class_audio_interface_1ac5cfa001682e6e0826c63b5f7d6350a8}

#### `public inline virtual void `[`setAudioTesterP`](#class_audio_interface_1a7b2a6673220bcb19ada364d629e0c1da)`(`[`AudioTester`](#class_audio_tester)` * atp)` {#class_audio_interface_1a7b2a6673220bcb19ada364d629e0c1da}

#### `public inline virtual int `[`getNumInputChannels`](#class_audio_interface_1a6c05fbe2cf820b4a764d7a905c61e5e6)`() const` {#class_audio_interface_1a6c05fbe2cf820b4a764d7a905c61e5e6}

Get Number of Input Channels.

#### `public inline virtual int `[`getNumOutputChannels`](#class_audio_interface_1a4f1dc0048bbf4aa85819f464647df07f)`() const` {#class_audio_interface_1a4f1dc0048bbf4aa85819f464647df07f}

Get Number of Output Channels.

#### `public inline virtual `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`getBufferSizeInSamples`](#class_audio_interface_1af0a0e609826e67d47ca130641ca07fc0)`() const` {#class_audio_interface_1af0a0e609826e67d47ca130641ca07fc0}

#### `public inline virtual `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`getDeviceID`](#class_audio_interface_1a7924a1fc68992f5e46cfd766d45dfe89)`() const` {#class_audio_interface_1a7924a1fc68992f5e46cfd766d45dfe89}

#### `public virtual size_t `[`getSizeInBytesPerChannel`](#class_audio_interface_1a38393b4d8e032c5e834c888d2697a1ab)`() const` {#class_audio_interface_1a38393b4d8e032c5e834c888d2697a1ab}

#### `public inline virtual `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`getSampleRate`](#class_audio_interface_1a1d6abfe44a1e13976aff878bf5e22e9e)`() const` {#class_audio_interface_1a1d6abfe44a1e13976aff878bf5e22e9e}

Get the Jack Server Sampling Rate, in samples/second.

#### `public virtual `[`samplingRateT`](#class_audio_interface_1a538ea04ceee63f418c629d6a3a0e4fef)` `[`getSampleRateType`](#class_audio_interface_1a2454d2f0b02a67b7fab00481877005a9)`() const` {#class_audio_interface_1a2454d2f0b02a67b7fab00481877005a9}

Get the Jack Server Sampling Rate Enum Type samplingRateT.

#### Returns
[AudioInterface::samplingRateT](#class_audio_interface_1a538ea04ceee63f418c629d6a3a0e4fef) enum type

#### `public inline virtual int `[`getAudioBitResolution`](#class_audio_interface_1a728fb37319785ace1c42605d955cab7c)`() const` {#class_audio_interface_1a728fb37319785ace1c42605d955cab7c}

Get the Audio Bit Resolution, in bits.

This is one of the audioBitResolutionT set in construction

#### `protected bool `[`mProcessingAudio`](#class_audio_interface_1aa1104803afb9fef3b29dc128ee279d60) {#class_audio_interface_1aa1104803afb9fef3b29dc128ee279d60}

Set when processing an audio callback buffer pair.

#### `protected const `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`MAX_AUDIO_BUFFER_SIZE`](#class_audio_interface_1a512aaddf196aed8dbbbe46d13ab6499b) {#class_audio_interface_1a512aaddf196aed8dbbbe46d13ab6499b}

#### `enum `[`audioBitResolutionT`](#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b) {#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b}

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
BIT8            | 8 bits
BIT16            | 16 bits (default)
BIT24            | 24 bits
BIT32            | 32 bits

Enum for Audio Resolution in bits.

#### `enum `[`samplingRateT`](#class_audio_interface_1a538ea04ceee63f418c629d6a3a0e4fef) {#class_audio_interface_1a538ea04ceee63f418c629d6a3a0e4fef}

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
SR22            | 22050 Hz
SR32            | 32000 Hz
SR44            | 44100 Hz
SR48            | 48000 Hz
SR88            | 88200 Hz
SR96            | 96000 Hz
SR192            | 192000 Hz
UNDEF            | Undefined.

Sampling Rates supported by JACK.

# class `AudioTester` {#class_audio_tester}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`AudioTester`](#class_audio_tester_1a233700977468306cad646c09c22c19fb)`()` | 
`public  `[`~AudioTester`](#class_audio_tester_1a6dcd13b8e8275f2c95a5cf1662fc31c5)`() = default` | 
`public void `[`lookForReturnPulse`](#class_audio_tester_1ab117c8cc877fe3f9f48c26adfa9bbe9a)`(QVarLengthArray< `[`sample_t`](#jacktrip__types_8h_1aa0798f7ea0975b143a1d9deac8c05d43)` * > & out_buffer,unsigned int n_frames)` | 
`public void `[`writeImpulse`](#class_audio_tester_1a2afda2ffdeef772427a63e8615b1b945)`(QVarLengthArray< `[`sample_t`](#jacktrip__types_8h_1aa0798f7ea0975b143a1d9deac8c05d43)` * > & mInBufCopy,unsigned int n_frames)` | 
`public inline bool `[`getEnabled`](#class_audio_tester_1a2259a4dc2ab8bece7deddd261c741ca1)`()` | 
`public inline void `[`setEnabled`](#class_audio_tester_1a006ab8bff999a1715c4fd7be2ba4b96f)`(bool e)` | 
`public inline void `[`setPrintIntervalSec`](#class_audio_tester_1a1f2863c2dd8639b15b0f7f0ad34e716f)`(float s)` | 
`public inline void `[`setSendChannel`](#class_audio_tester_1a2eb22c8a2c57e17ad6b83403bc350f7e)`(int c)` | 
`public inline int `[`getSendChannel`](#class_audio_tester_1aad40b1f215e0429e72e081e891a32b6d)`()` | 
`public inline int `[`getPendingCell`](#class_audio_tester_1abe07a60ab14a68d7c8ddc62b639eb65c)`()` | 
`public inline void `[`setPendingCell`](#class_audio_tester_1a4dba34f69eec3319fcecb0676896109b)`(int pc)` | 
`public inline void `[`setSampleRate`](#class_audio_tester_1a7af6bf21c088f668a247008c6699e02a)`(float fs)` | 
`public inline int `[`getBufferSkip`](#class_audio_tester_1a3adf1fa7a30e0ab1764a060fe4b10527)`()` | 
`public void `[`printHelp`](#class_audio_tester_1a319e4f7b485870f9ba086272c53af217)`(char * command,char helpCase)` | 

## Members

#### `public inline  `[`AudioTester`](#class_audio_tester_1a233700977468306cad646c09c22c19fb)`()` {#class_audio_tester_1a233700977468306cad646c09c22c19fb}

#### `public  `[`~AudioTester`](#class_audio_tester_1a6dcd13b8e8275f2c95a5cf1662fc31c5)`() = default` {#class_audio_tester_1a6dcd13b8e8275f2c95a5cf1662fc31c5}

#### `public void `[`lookForReturnPulse`](#class_audio_tester_1ab117c8cc877fe3f9f48c26adfa9bbe9a)`(QVarLengthArray< `[`sample_t`](#jacktrip__types_8h_1aa0798f7ea0975b143a1d9deac8c05d43)` * > & out_buffer,unsigned int n_frames)` {#class_audio_tester_1ab117c8cc877fe3f9f48c26adfa9bbe9a}

#### `public void `[`writeImpulse`](#class_audio_tester_1a2afda2ffdeef772427a63e8615b1b945)`(QVarLengthArray< `[`sample_t`](#jacktrip__types_8h_1aa0798f7ea0975b143a1d9deac8c05d43)` * > & mInBufCopy,unsigned int n_frames)` {#class_audio_tester_1a2afda2ffdeef772427a63e8615b1b945}

#### `public inline bool `[`getEnabled`](#class_audio_tester_1a2259a4dc2ab8bece7deddd261c741ca1)`()` {#class_audio_tester_1a2259a4dc2ab8bece7deddd261c741ca1}

#### `public inline void `[`setEnabled`](#class_audio_tester_1a006ab8bff999a1715c4fd7be2ba4b96f)`(bool e)` {#class_audio_tester_1a006ab8bff999a1715c4fd7be2ba4b96f}

#### `public inline void `[`setPrintIntervalSec`](#class_audio_tester_1a1f2863c2dd8639b15b0f7f0ad34e716f)`(float s)` {#class_audio_tester_1a1f2863c2dd8639b15b0f7f0ad34e716f}

#### `public inline void `[`setSendChannel`](#class_audio_tester_1a2eb22c8a2c57e17ad6b83403bc350f7e)`(int c)` {#class_audio_tester_1a2eb22c8a2c57e17ad6b83403bc350f7e}

#### `public inline int `[`getSendChannel`](#class_audio_tester_1aad40b1f215e0429e72e081e891a32b6d)`()` {#class_audio_tester_1aad40b1f215e0429e72e081e891a32b6d}

#### `public inline int `[`getPendingCell`](#class_audio_tester_1abe07a60ab14a68d7c8ddc62b639eb65c)`()` {#class_audio_tester_1abe07a60ab14a68d7c8ddc62b639eb65c}

#### `public inline void `[`setPendingCell`](#class_audio_tester_1a4dba34f69eec3319fcecb0676896109b)`(int pc)` {#class_audio_tester_1a4dba34f69eec3319fcecb0676896109b}

#### `public inline void `[`setSampleRate`](#class_audio_tester_1a7af6bf21c088f668a247008c6699e02a)`(float fs)` {#class_audio_tester_1a7af6bf21c088f668a247008c6699e02a}

#### `public inline int `[`getBufferSkip`](#class_audio_tester_1a3adf1fa7a30e0ab1764a060fe4b10527)`()` {#class_audio_tester_1a3adf1fa7a30e0ab1764a060fe4b10527}

#### `public void `[`printHelp`](#class_audio_tester_1a319e4f7b485870f9ba086272c53af217)`(char * command,char helpCase)` {#class_audio_tester_1a319e4f7b485870f9ba086272c53af217}

# class `Compressor` {#class_compressor}

```
class Compressor
  : public ProcessPlugin
```  

Applies compressor_mono from the faustlibraries distribution, compressors.lib.

A [Compressor](#class_compressor) reduces the output dynamic range when the signal level exceeds the threshold.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`Compressor`](#class_compressor_1af1d39f5e1ed2fad9e007748d87b33905)`(int numchans,bool verboseIn,float ratioIn,float thresholdDBIn,float attackMSIn,float releaseMSIn,float makeUpGainDBIn)` | The class constructor sets the number of audio channels and default parameters.
`public inline  `[`Compressor`](#class_compressor_1ab1ecc30a2a54e72a353b01872b968222)`(int numchans,bool verboseIn,`[`CompressorPreset`](#struct_compressor_preset)` preset)` | 
`public inline virtual  `[`~Compressor`](#class_compressor_1a0610fdbd71526609ed20f0a3cd919360)`()` | The class destructor.
`public inline void `[`setParamAllChannels`](#class_compressor_1af299e80a7497fd6739d5e6a068640731)`(const char pName,float p)` | 
`public inline virtual void `[`init`](#class_compressor_1af03605a1596ed1627f483752d6180a9d)`(int samplingRate)` | Do proper Initialization of members and class instances. By default this initializes the Sampling Frequency. If a class instance depends on the sampling frequency, it should be initialize here.
`public inline virtual int `[`getNumInputs`](#class_compressor_1aab4676dbd7b4f4f93a9c6b1657c3deab)`()` | Return Number of Input Channels.
`public inline virtual int `[`getNumOutputs`](#class_compressor_1a700245bcfdf5445b8ec258f14469ab98)`()` | Return Number of Output Channels.
`public virtual void `[`compute`](#class_compressor_1a459fca73767848397a26926e836c2175)`(int nframes,float ** inputs,float ** outputs)` | Compute process.

## Members

#### `public inline  `[`Compressor`](#class_compressor_1af1d39f5e1ed2fad9e007748d87b33905)`(int numchans,bool verboseIn,float ratioIn,float thresholdDBIn,float attackMSIn,float releaseMSIn,float makeUpGainDBIn)` {#class_compressor_1af1d39f5e1ed2fad9e007748d87b33905}

The class constructor sets the number of audio channels and default parameters.

#### `public inline  `[`Compressor`](#class_compressor_1ab1ecc30a2a54e72a353b01872b968222)`(int numchans,bool verboseIn,`[`CompressorPreset`](#struct_compressor_preset)` preset)` {#class_compressor_1ab1ecc30a2a54e72a353b01872b968222}

#### `public inline virtual  `[`~Compressor`](#class_compressor_1a0610fdbd71526609ed20f0a3cd919360)`()` {#class_compressor_1a0610fdbd71526609ed20f0a3cd919360}

The class destructor.

#### `public inline void `[`setParamAllChannels`](#class_compressor_1af299e80a7497fd6739d5e6a068640731)`(const char pName,float p)` {#class_compressor_1af299e80a7497fd6739d5e6a068640731}

#### `public inline virtual void `[`init`](#class_compressor_1af03605a1596ed1627f483752d6180a9d)`(int samplingRate)` {#class_compressor_1af03605a1596ed1627f483752d6180a9d}

Do proper Initialization of members and class instances. By default this initializes the Sampling Frequency. If a class instance depends on the sampling frequency, it should be initialize here.

#### `public inline virtual int `[`getNumInputs`](#class_compressor_1aab4676dbd7b4f4f93a9c6b1657c3deab)`()` {#class_compressor_1aab4676dbd7b4f4f93a9c6b1657c3deab}

Return Number of Input Channels.

#### `public inline virtual int `[`getNumOutputs`](#class_compressor_1a700245bcfdf5445b8ec258f14469ab98)`()` {#class_compressor_1a700245bcfdf5445b8ec258f14469ab98}

Return Number of Output Channels.

#### `public virtual void `[`compute`](#class_compressor_1a459fca73767848397a26926e836c2175)`(int nframes,float ** inputs,float ** outputs)` {#class_compressor_1a459fca73767848397a26926e836c2175}

Compute process.

# class `compressordsp` {#classcompressordsp}

```
class compressordsp
  : public dsp
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline virtual void `[`metadata`](#classcompressordsp_1a029f83bb3455569534bf32f2600e8415)`(`[`Meta`](#struct_meta)` * m)` | Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.
`public inline virtual int `[`getNumInputs`](#classcompressordsp_1a30f39c83df9a2b0ce8d4d59248cc0175)`()` | 
`public inline virtual int `[`getNumOutputs`](#classcompressordsp_1a2a93dc0ec6b55a0a7c00f7134bcaebdd)`()` | 
`public inline virtual int `[`getInputRate`](#classcompressordsp_1a5335c99a06d43dcdabc42eb883d3b3f7)`(int channel)` | 
`public inline virtual int `[`getOutputRate`](#classcompressordsp_1ab9f238324aac1854a40903c8c9f878bf)`(int channel)` | 
`public inline virtual void `[`instanceConstants`](#classcompressordsp_1ad3c15cd0a33829cb704067c9ac6c8614)`(int sample_rate)` | Init instance constant state
`public inline virtual void `[`instanceResetUserInterface`](#classcompressordsp_1a8debdeefb0e18ea6a541deece256dc6c)`()` | 
`public inline virtual void `[`instanceClear`](#classcompressordsp_1affc2a7148113cc4846b36f626842c2c0)`()` | 
`public inline virtual void `[`init`](#classcompressordsp_1a6bec36bce6b5b19d42831a962c3198fc)`(int sample_rate)` | Global init, calls the following methods:
`public inline virtual void `[`instanceInit`](#classcompressordsp_1aff0bd96d5d8fed0317a88a50eacada1d)`(int sample_rate)` | Init instance state
`public inline virtual `[`compressordsp`](#classcompressordsp)` * `[`clone`](#classcompressordsp_1af6872beacbc2ca74f64aea086f477c28)`()` | Return a clone of the instance.
`public inline virtual int `[`getSampleRate`](#classcompressordsp_1a2588906f6570e48639aaab8e18c668c7)`()` | 
`public inline virtual void `[`buildUserInterface`](#classcompressordsp_1ae5d92323201510a22548658d1ce72c19)`(`[`UI`](#struct_u_i)` * ui_interface)` | Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).
`public inline virtual void `[`compute`](#classcompressordsp_1aba9ef35e8220986cc17b40dc0e43bd0c)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` | DSP instance computation, to be called with successive in/out audio buffers.

## Members

#### `public inline virtual void `[`metadata`](#classcompressordsp_1a029f83bb3455569534bf32f2600e8415)`(`[`Meta`](#struct_meta)` * m)` {#classcompressordsp_1a029f83bb3455569534bf32f2600e8415}

Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.

#### Parameters
* `m` - the Meta* meta user

#### `public inline virtual int `[`getNumInputs`](#classcompressordsp_1a30f39c83df9a2b0ce8d4d59248cc0175)`()` {#classcompressordsp_1a30f39c83df9a2b0ce8d4d59248cc0175}

#### `public inline virtual int `[`getNumOutputs`](#classcompressordsp_1a2a93dc0ec6b55a0a7c00f7134bcaebdd)`()` {#classcompressordsp_1a2a93dc0ec6b55a0a7c00f7134bcaebdd}

#### `public inline virtual int `[`getInputRate`](#classcompressordsp_1a5335c99a06d43dcdabc42eb883d3b3f7)`(int channel)` {#classcompressordsp_1a5335c99a06d43dcdabc42eb883d3b3f7}

#### `public inline virtual int `[`getOutputRate`](#classcompressordsp_1ab9f238324aac1854a40903c8c9f878bf)`(int channel)` {#classcompressordsp_1ab9f238324aac1854a40903c8c9f878bf}

#### `public inline virtual void `[`instanceConstants`](#classcompressordsp_1ad3c15cd0a33829cb704067c9ac6c8614)`(int sample_rate)` {#classcompressordsp_1ad3c15cd0a33829cb704067c9ac6c8614}

Init instance constant state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual void `[`instanceResetUserInterface`](#classcompressordsp_1a8debdeefb0e18ea6a541deece256dc6c)`()` {#classcompressordsp_1a8debdeefb0e18ea6a541deece256dc6c}

#### `public inline virtual void `[`instanceClear`](#classcompressordsp_1affc2a7148113cc4846b36f626842c2c0)`()` {#classcompressordsp_1affc2a7148113cc4846b36f626842c2c0}

#### `public inline virtual void `[`init`](#classcompressordsp_1a6bec36bce6b5b19d42831a962c3198fc)`(int sample_rate)` {#classcompressordsp_1a6bec36bce6b5b19d42831a962c3198fc}

Global init, calls the following methods:

* static class 'classInit': static tables initialization

* 'instanceInit': constants and instance state initialization

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual void `[`instanceInit`](#classcompressordsp_1aff0bd96d5d8fed0317a88a50eacada1d)`(int sample_rate)` {#classcompressordsp_1aff0bd96d5d8fed0317a88a50eacada1d}

Init instance state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual `[`compressordsp`](#classcompressordsp)` * `[`clone`](#classcompressordsp_1af6872beacbc2ca74f64aea086f477c28)`()` {#classcompressordsp_1af6872beacbc2ca74f64aea086f477c28}

Return a clone of the instance.

#### Returns
a copy of the instance on success, otherwise a null pointer.

#### `public inline virtual int `[`getSampleRate`](#classcompressordsp_1a2588906f6570e48639aaab8e18c668c7)`()` {#classcompressordsp_1a2588906f6570e48639aaab8e18c668c7}

#### `public inline virtual void `[`buildUserInterface`](#classcompressordsp_1ae5d92323201510a22548658d1ce72c19)`(`[`UI`](#struct_u_i)` * ui_interface)` {#classcompressordsp_1ae5d92323201510a22548658d1ce72c19}

Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).

#### Parameters
* `ui_interface` - the user interface builder

#### `public inline virtual void `[`compute`](#classcompressordsp_1aba9ef35e8220986cc17b40dc0e43bd0c)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` {#classcompressordsp_1aba9ef35e8220986cc17b40dc0e43bd0c}

DSP instance computation, to be called with successive in/out audio buffers.

#### Parameters
* `count` - the number of frames to compute 

* `inputs` - the input audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad) 

* `outputs` - the output audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad)

# class `ConverterZoneControl` {#class_converter_zone_control}

```
class ConverterZoneControl
  : public ZoneControl
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`ConverterZoneControl`](#class_converter_zone_control_1a512838429d7254b40b2e8479971d7342)`(`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,`[`ValueConverter`](#class_value_converter)` * converter)` | 
`public inline virtual  `[`~ConverterZoneControl`](#class_converter_zone_control_1afe7928e5c80a3243020519876731b41b)`()` | 
`public inline virtual void `[`update`](#class_converter_zone_control_1ac1fab49438d941a2a7bbefbb1c4ce170)`(double v) const` | 
`public inline `[`ValueConverter`](#class_value_converter)` * `[`getConverter`](#class_converter_zone_control_1af5ba6478efafb7ac554e8b57252f22ac)`()` | 
`protected `[`ValueConverter`](#class_value_converter)` * `[`fValueConverter`](#class_converter_zone_control_1aa24045eb69fdb9cc4ac72d0306e9e81e) | 

## Members

#### `public inline  `[`ConverterZoneControl`](#class_converter_zone_control_1a512838429d7254b40b2e8479971d7342)`(`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,`[`ValueConverter`](#class_value_converter)` * converter)` {#class_converter_zone_control_1a512838429d7254b40b2e8479971d7342}

#### `public inline virtual  `[`~ConverterZoneControl`](#class_converter_zone_control_1afe7928e5c80a3243020519876731b41b)`()` {#class_converter_zone_control_1afe7928e5c80a3243020519876731b41b}

#### `public inline virtual void `[`update`](#class_converter_zone_control_1ac1fab49438d941a2a7bbefbb1c4ce170)`(double v) const` {#class_converter_zone_control_1ac1fab49438d941a2a7bbefbb1c4ce170}

#### `public inline `[`ValueConverter`](#class_value_converter)` * `[`getConverter`](#class_converter_zone_control_1af5ba6478efafb7ac554e8b57252f22ac)`()` {#class_converter_zone_control_1af5ba6478efafb7ac554e8b57252f22ac}

#### `protected `[`ValueConverter`](#class_value_converter)` * `[`fValueConverter`](#class_converter_zone_control_1aa24045eb69fdb9cc4ac72d0306e9e81e) {#class_converter_zone_control_1aa24045eb69fdb9cc4ac72d0306e9e81e}

# class `CurveZoneControl` {#class_curve_zone_control}

```
class CurveZoneControl
  : public ZoneControl
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`CurveZoneControl`](#class_curve_zone_control_1aacf809869ff9e39e912a2a6007474dec)`(`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,int curve,double amin,double amid,double amax,double min,double init,double max)` | 
`public inline virtual  `[`~CurveZoneControl`](#class_curve_zone_control_1aa5249f7150491a33fc85bc48ce630d9b)`()` | 
`public inline virtual void `[`update`](#class_curve_zone_control_1a119fc7e9407146d7ba49c0f16e828535)`(double v) const` | 
`public inline virtual void `[`setMappingValues`](#class_curve_zone_control_1a3cf343645e6a8b5dcad1681399d24658)`(int curve,double amin,double amid,double amax,double min,double init,double max)` | 
`public inline virtual void `[`getMappingValues`](#class_curve_zone_control_1a86639d667698c0979112b19e014316c9)`(double & amin,double & amid,double & amax)` | 
`public inline virtual void `[`setActive`](#class_curve_zone_control_1a08ae16a16aa33ecfa908b0b11109cd78)`(bool on_off)` | 
`public inline virtual int `[`getCurve`](#class_curve_zone_control_1ad511574ef826af069fddcd0bcf7da17d)`()` | 

## Members

#### `public inline  `[`CurveZoneControl`](#class_curve_zone_control_1aacf809869ff9e39e912a2a6007474dec)`(`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,int curve,double amin,double amid,double amax,double min,double init,double max)` {#class_curve_zone_control_1aacf809869ff9e39e912a2a6007474dec}

#### `public inline virtual  `[`~CurveZoneControl`](#class_curve_zone_control_1aa5249f7150491a33fc85bc48ce630d9b)`()` {#class_curve_zone_control_1aa5249f7150491a33fc85bc48ce630d9b}

#### `public inline virtual void `[`update`](#class_curve_zone_control_1a119fc7e9407146d7ba49c0f16e828535)`(double v) const` {#class_curve_zone_control_1a119fc7e9407146d7ba49c0f16e828535}

#### `public inline virtual void `[`setMappingValues`](#class_curve_zone_control_1a3cf343645e6a8b5dcad1681399d24658)`(int curve,double amin,double amid,double amax,double min,double init,double max)` {#class_curve_zone_control_1a3cf343645e6a8b5dcad1681399d24658}

#### `public inline virtual void `[`getMappingValues`](#class_curve_zone_control_1a86639d667698c0979112b19e014316c9)`(double & amin,double & amid,double & amax)` {#class_curve_zone_control_1a86639d667698c0979112b19e014316c9}

#### `public inline virtual void `[`setActive`](#class_curve_zone_control_1a08ae16a16aa33ecfa908b0b11109cd78)`(bool on_off)` {#class_curve_zone_control_1a08ae16a16aa33ecfa908b0b11109cd78}

#### `public inline virtual int `[`getCurve`](#class_curve_zone_control_1ad511574ef826af069fddcd0bcf7da17d)`()` {#class_curve_zone_control_1ad511574ef826af069fddcd0bcf7da17d}

# class `DataProtocol` {#class_data_protocol}

```
class DataProtocol
  : public QThread
```  

Base class that defines the transmission protocol.

This base class defines most of the common method to setup and connect sockets using the individual protocols (UDP, TCP, SCTP, etc).

The class has to be constructed using one of two modes (runModeT):

* SENDER

* RECEIVER

This has to be specified as a constructor argument. When using, create two instances of the class, one to receive and one to send packets. Each instance will run on a separate thread.

Redundancy and forward error correction should be implemented on each Transport protocol, cause they depend on the protocol itself

> Todo: This Class should contain definition of jacktrip header and basic funcionality to obtain local machine IPs and maybe functions to manipulate IPs. Redundancy and forward error correction should be implemented on each Transport protocol, cause they depend on the protocol itself

> Todo: The transport protocol itself has to be implemented subclassing this class, i.e., using a TCP or UDP protocol.

Even if the underlined transmission protocol is stream oriented (as in TCP), we send packets that are the size of the audio processing buffer. Use AudioInterface::getBufferSize to obtain this value.

Each transmission (i.e., inputs and outputs) run on its own thread.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  `[`DataProtocol`](#class_data_protocol_1a692c494d35c01180758aa6e32222b535)`(`[`JackTrip`](#class_jack_trip)` * jacktrip,const `[`runModeT`](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2c)` runmode,int bind_port,int peer_port)` | The class constructor.
`public virtual  `[`~DataProtocol`](#class_data_protocol_1a7d0b609ce5497affa295c5cc7fc5db25)`()` | The class destructor.
`public void `[`run`](#class_data_protocol_1ad23766c4038720e85f8fe9f83069ae1c)`()` | Implements the thread loop.
`public inline virtual void `[`stop`](#class_data_protocol_1a8bc0b6b855cbaf7d7a7d5991d3566059)`()` | Stops the execution of the Thread.
`public inline void `[`setAudioPacketSize`](#class_data_protocol_1af46c116bd04cc8b06740c6e6fe5fa622)`(const size_t size_bytes)` | Sets the size of the audio part of the packets.
`public inline size_t `[`getAudioPacketSizeInBites`](#class_data_protocol_1a942fcff4796851420420e7976f93c48f)`()` | Get the size of the audio part of the packets.
`public void `[`setPeerAddress`](#class_data_protocol_1aed6de555806ed1cb1a4ab618f28d1c7c)`(const char * peerHostOrIP)` | Set the peer address.
`public void `[`setPeerPort`](#class_data_protocol_1a66a344c0b521beddc8bb510d483515d9)`(int port)` | Set the peer incomming (receiving) port number.
`public void `[`setSocket`](#class_data_protocol_1a65f89d9a60ed0f14b9848ae30179c47b)`(int & socket)` | 
`public inline virtual bool `[`getStats`](#class_data_protocol_1ab663c37d81764780f9045471714c83ec)`(`[`PktStat`](#struct_data_protocol_1_1_pkt_stat)` *)` | 
`public inline virtual void `[`setIssueSimulation`](#class_data_protocol_1a21fdce1f6a6e35f069ce4749fd069866)`(double,double,double)` | 
`public inline void `[`setUseRtPriority`](#class_data_protocol_1a0169677394e53ef74448ecf6d29a01b3)`(bool use)` | 
`protected bool `[`mStopped`](#class_data_protocol_1a2840e4119efdc72870daf34267efd36e) | Boolean stop the execution of the thread.
`protected bool `[`mHasPeerAddress`](#class_data_protocol_1aa2febf67f13757e0876362622aa335b9) | Boolean to indicate if the RECEIVER is waiting to obtain peer address.
`protected bool `[`mHasPacketsToReceive`](#class_data_protocol_1a4efcb035c3376e52faf1f5052ffcccff) | Boolean that indicates if a packet was received.
`protected QMutex `[`mMutex`](#class_data_protocol_1a87be7ce305256235119309d3879416d7) | 
`protected `[`JackTrip`](#class_jack_trip)` * `[`mJackTrip`](#class_data_protocol_1a191bbc5edd6bd92dc2393d36dd365d63) | [JackTrip](#class_jack_trip) mediator class.
`protected bool `[`mUseRtPriority`](#class_data_protocol_1a176e64360be658b3e81081d2386087f0) | 
`protected inline `[`runModeT`](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2c)` `[`getRunMode`](#class_data_protocol_1affebd07fe22569a24ec1ffb261d0fbea)`() const` | Get the Run Mode of the object.
`{signal} public void `[`signalError`](#class_data_protocol_1ada3298bb52b4d69166d2e7406d77071b)`(const char * error_message)` | 
`{signal} public void `[`signalReceivedConnectionFromPeer`](#class_data_protocol_1a7f6c38625966f43279190605437043da)`()` | 
`{signal} public void `[`signalCeaseTransmission`](#class_data_protocol_1a484e57eb3895eeb887992085c2bff167)`(const QString & reason)` | 
`enum `[`packetHeaderTypeT`](#class_data_protocol_1aed102fbb7239cbf862e6296eefe79492) | Enum to define packet header types.
`enum `[`runModeT`](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2c) | Enum to define class modes, SENDER or RECEIVER.

## Members

#### `public  `[`DataProtocol`](#class_data_protocol_1a692c494d35c01180758aa6e32222b535)`(`[`JackTrip`](#class_jack_trip)` * jacktrip,const `[`runModeT`](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2c)` runmode,int bind_port,int peer_port)` {#class_data_protocol_1a692c494d35c01180758aa6e32222b535}

The class constructor.

#### Parameters
* `jacktrip` Pointer to the [JackTrip](#class_jack_trip) class that connects all classes (mediator) 

* `runmode` Sets the run mode, use either [DataProtocol::SENDER](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2ca03af57b57ddf9036d6444ceef3ca3059) or [DataProtocol::RECEIVER](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2ca1a12f7cf3e8bf36932809cb9083e223f)

* `headertype` packetHeaderTypeT header type to use for packets 

* `bind_port` Port number to bind for this socket (this is the receive or send port depending on the runmode) 

* `peer_port` Peer port number (this is the receive or send port depending on the runmode)

#### `public virtual  `[`~DataProtocol`](#class_data_protocol_1a7d0b609ce5497affa295c5cc7fc5db25)`()` {#class_data_protocol_1a7d0b609ce5497affa295c5cc7fc5db25}

The class destructor.

#### `public void `[`run`](#class_data_protocol_1ad23766c4038720e85f8fe9f83069ae1c)`()` {#class_data_protocol_1ad23766c4038720e85f8fe9f83069ae1c}

Implements the thread loop.

Depending on the runmode, with will run a [DataProtocol::SENDER](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2ca03af57b57ddf9036d6444ceef3ca3059) thread or [DataProtocol::RECEIVER](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2ca1a12f7cf3e8bf36932809cb9083e223f) thread

#### `public inline virtual void `[`stop`](#class_data_protocol_1a8bc0b6b855cbaf7d7a7d5991d3566059)`()` {#class_data_protocol_1a8bc0b6b855cbaf7d7a7d5991d3566059}

Stops the execution of the Thread.

#### `public inline void `[`setAudioPacketSize`](#class_data_protocol_1af46c116bd04cc8b06740c6e6fe5fa622)`(const size_t size_bytes)` {#class_data_protocol_1af46c116bd04cc8b06740c6e6fe5fa622}

Sets the size of the audio part of the packets.

#### Parameters
* `size_bytes` Size in bytes

#### `public inline size_t `[`getAudioPacketSizeInBites`](#class_data_protocol_1a942fcff4796851420420e7976f93c48f)`()` {#class_data_protocol_1a942fcff4796851420420e7976f93c48f}

Get the size of the audio part of the packets.

#### Returns
size_bytes Size in bytes

#### `public void `[`setPeerAddress`](#class_data_protocol_1aed6de555806ed1cb1a4ab618f28d1c7c)`(const char * peerHostOrIP)` {#class_data_protocol_1aed6de555806ed1cb1a4ab618f28d1c7c}

Set the peer address.

#### Parameters
* `peerHostOrIP` IPv4 number or host name 

> Todo: implement here instead of in the subclass UDP

#### `public void `[`setPeerPort`](#class_data_protocol_1a66a344c0b521beddc8bb510d483515d9)`(int port)` {#class_data_protocol_1a66a344c0b521beddc8bb510d483515d9}

Set the peer incomming (receiving) port number.

#### Parameters
* `port` Port number 

> Todo: implement here instead of in the subclass UDP

#### `public void `[`setSocket`](#class_data_protocol_1a65f89d9a60ed0f14b9848ae30179c47b)`(int & socket)` {#class_data_protocol_1a65f89d9a60ed0f14b9848ae30179c47b}

#### `public inline virtual bool `[`getStats`](#class_data_protocol_1ab663c37d81764780f9045471714c83ec)`(`[`PktStat`](#struct_data_protocol_1_1_pkt_stat)` *)` {#class_data_protocol_1ab663c37d81764780f9045471714c83ec}

#### `public inline virtual void `[`setIssueSimulation`](#class_data_protocol_1a21fdce1f6a6e35f069ce4749fd069866)`(double,double,double)` {#class_data_protocol_1a21fdce1f6a6e35f069ce4749fd069866}

#### `public inline void `[`setUseRtPriority`](#class_data_protocol_1a0169677394e53ef74448ecf6d29a01b3)`(bool use)` {#class_data_protocol_1a0169677394e53ef74448ecf6d29a01b3}

#### `protected bool `[`mStopped`](#class_data_protocol_1a2840e4119efdc72870daf34267efd36e) {#class_data_protocol_1a2840e4119efdc72870daf34267efd36e}

Boolean stop the execution of the thread.

#### `protected bool `[`mHasPeerAddress`](#class_data_protocol_1aa2febf67f13757e0876362622aa335b9) {#class_data_protocol_1aa2febf67f13757e0876362622aa335b9}

Boolean to indicate if the RECEIVER is waiting to obtain peer address.

#### `protected bool `[`mHasPacketsToReceive`](#class_data_protocol_1a4efcb035c3376e52faf1f5052ffcccff) {#class_data_protocol_1a4efcb035c3376e52faf1f5052ffcccff}

Boolean that indicates if a packet was received.

#### `protected QMutex `[`mMutex`](#class_data_protocol_1a87be7ce305256235119309d3879416d7) {#class_data_protocol_1a87be7ce305256235119309d3879416d7}

#### `protected `[`JackTrip`](#class_jack_trip)` * `[`mJackTrip`](#class_data_protocol_1a191bbc5edd6bd92dc2393d36dd365d63) {#class_data_protocol_1a191bbc5edd6bd92dc2393d36dd365d63}

[JackTrip](#class_jack_trip) mediator class.

> Todo: check a better way to access the header from the subclasses

#### `protected bool `[`mUseRtPriority`](#class_data_protocol_1a176e64360be658b3e81081d2386087f0) {#class_data_protocol_1a176e64360be658b3e81081d2386087f0}

#### `protected inline `[`runModeT`](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2c)` `[`getRunMode`](#class_data_protocol_1affebd07fe22569a24ec1ffb261d0fbea)`() const` {#class_data_protocol_1affebd07fe22569a24ec1ffb261d0fbea}

Get the Run Mode of the object.

#### Returns
SENDER or RECEIVER

#### `{signal} public void `[`signalError`](#class_data_protocol_1ada3298bb52b4d69166d2e7406d77071b)`(const char * error_message)` {#class_data_protocol_1ada3298bb52b4d69166d2e7406d77071b}

#### `{signal} public void `[`signalReceivedConnectionFromPeer`](#class_data_protocol_1a7f6c38625966f43279190605437043da)`()` {#class_data_protocol_1a7f6c38625966f43279190605437043da}

#### `{signal} public void `[`signalCeaseTransmission`](#class_data_protocol_1a484e57eb3895eeb887992085c2bff167)`(const QString & reason)` {#class_data_protocol_1a484e57eb3895eeb887992085c2bff167}

#### `enum `[`packetHeaderTypeT`](#class_data_protocol_1aed102fbb7239cbf862e6296eefe79492) {#class_data_protocol_1aed102fbb7239cbf862e6296eefe79492}

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
DEFAULT            | Default application header.
JAMLINK            | Header to use with Jamlinks.
EMPTY            | Empty Header.

Enum to define packet header types.

#### `enum `[`runModeT`](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2c) {#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2c}

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
SENDER            | Set class as a Sender (send packets)
RECEIVER            | Set class as a Receiver (receives packets)

Enum to define class modes, SENDER or RECEIVER.

# class `decorator_dsp` {#classdecorator__dsp}

```
class decorator_dsp
  : public dsp
```  

Generic DSP decorator.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`decorator_dsp`](#classdecorator__dsp_1a4bb4406e3fce321ce7b752130d0aa890)`(`[`dsp`](#classdsp)` * dsp)` | 
`public inline virtual  `[`~decorator_dsp`](#classdecorator__dsp_1adaf0c5027895f0f89acb3f6b11beb157)`()` | 
`public inline virtual int `[`getNumInputs`](#classdecorator__dsp_1ae37b6960055310fdc92c9b22d59e11c3)`()` | 
`public inline virtual int `[`getNumOutputs`](#classdecorator__dsp_1a7aa48cbd89994942f347a359336eff61)`()` | 
`public inline virtual void `[`buildUserInterface`](#classdecorator__dsp_1a39e6496255f8163b9390c040a984f842)`(`[`UI`](#struct_u_i)` * ui_interface)` | Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).
`public inline virtual int `[`getSampleRate`](#classdecorator__dsp_1ab1187a73d4da1697cf2fc577ad69833d)`()` | 
`public inline virtual void `[`init`](#classdecorator__dsp_1a2d8cb42fcaf9dd3f708998184fd6ea58)`(int sample_rate)` | Global init, calls the following methods:
`public inline virtual void `[`instanceInit`](#classdecorator__dsp_1a854f1eb8b2837389948b7f5292e45097)`(int sample_rate)` | Init instance state
`public inline virtual void `[`instanceConstants`](#classdecorator__dsp_1a41304b45a275e836f6ab272e3616fb23)`(int sample_rate)` | Init instance constant state
`public inline virtual void `[`instanceResetUserInterface`](#classdecorator__dsp_1a7a0b7908f2fd87819bfeba4d42376c1c)`()` | 
`public inline virtual void `[`instanceClear`](#classdecorator__dsp_1ad614bf1e7b89f25ce5a587a3d0dd7274)`()` | 
`public inline virtual `[`decorator_dsp`](#classdecorator__dsp)` * `[`clone`](#classdecorator__dsp_1a8eb6259f47c9eb26af9a0f64f12f159c)`()` | Return a clone of the instance.
`public inline virtual void `[`metadata`](#classdecorator__dsp_1aee9419f6a7e06c8762f5c9a41bbe7e37)`(`[`Meta`](#struct_meta)` * m)` | Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.
`public inline virtual void `[`compute`](#classdecorator__dsp_1a43c63a5541bfbff6eb34525019fa8c09)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` | DSP instance computation, to be called with successive in/out audio buffers.
`public inline virtual void `[`compute`](#classdecorator__dsp_1a850b718de2fa944e8316d36d5f5956a9)`(double,int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` | DSP instance computation: alternative method to be used by subclasses.
`protected `[`dsp`](#classdsp)` * `[`fDSP`](#classdecorator__dsp_1ad3702791f702316c2d0ccc725b0c943b) | 

## Members

#### `public inline  `[`decorator_dsp`](#classdecorator__dsp_1a4bb4406e3fce321ce7b752130d0aa890)`(`[`dsp`](#classdsp)` * dsp)` {#classdecorator__dsp_1a4bb4406e3fce321ce7b752130d0aa890}

#### `public inline virtual  `[`~decorator_dsp`](#classdecorator__dsp_1adaf0c5027895f0f89acb3f6b11beb157)`()` {#classdecorator__dsp_1adaf0c5027895f0f89acb3f6b11beb157}

#### `public inline virtual int `[`getNumInputs`](#classdecorator__dsp_1ae37b6960055310fdc92c9b22d59e11c3)`()` {#classdecorator__dsp_1ae37b6960055310fdc92c9b22d59e11c3}

#### `public inline virtual int `[`getNumOutputs`](#classdecorator__dsp_1a7aa48cbd89994942f347a359336eff61)`()` {#classdecorator__dsp_1a7aa48cbd89994942f347a359336eff61}

#### `public inline virtual void `[`buildUserInterface`](#classdecorator__dsp_1a39e6496255f8163b9390c040a984f842)`(`[`UI`](#struct_u_i)` * ui_interface)` {#classdecorator__dsp_1a39e6496255f8163b9390c040a984f842}

Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).

#### Parameters
* `ui_interface` - the user interface builder

#### `public inline virtual int `[`getSampleRate`](#classdecorator__dsp_1ab1187a73d4da1697cf2fc577ad69833d)`()` {#classdecorator__dsp_1ab1187a73d4da1697cf2fc577ad69833d}

#### `public inline virtual void `[`init`](#classdecorator__dsp_1a2d8cb42fcaf9dd3f708998184fd6ea58)`(int sample_rate)` {#classdecorator__dsp_1a2d8cb42fcaf9dd3f708998184fd6ea58}

Global init, calls the following methods:

* static class 'classInit': static tables initialization

* 'instanceInit': constants and instance state initialization

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual void `[`instanceInit`](#classdecorator__dsp_1a854f1eb8b2837389948b7f5292e45097)`(int sample_rate)` {#classdecorator__dsp_1a854f1eb8b2837389948b7f5292e45097}

Init instance state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual void `[`instanceConstants`](#classdecorator__dsp_1a41304b45a275e836f6ab272e3616fb23)`(int sample_rate)` {#classdecorator__dsp_1a41304b45a275e836f6ab272e3616fb23}

Init instance constant state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual void `[`instanceResetUserInterface`](#classdecorator__dsp_1a7a0b7908f2fd87819bfeba4d42376c1c)`()` {#classdecorator__dsp_1a7a0b7908f2fd87819bfeba4d42376c1c}

#### `public inline virtual void `[`instanceClear`](#classdecorator__dsp_1ad614bf1e7b89f25ce5a587a3d0dd7274)`()` {#classdecorator__dsp_1ad614bf1e7b89f25ce5a587a3d0dd7274}

#### `public inline virtual `[`decorator_dsp`](#classdecorator__dsp)` * `[`clone`](#classdecorator__dsp_1a8eb6259f47c9eb26af9a0f64f12f159c)`()` {#classdecorator__dsp_1a8eb6259f47c9eb26af9a0f64f12f159c}

Return a clone of the instance.

#### Returns
a copy of the instance on success, otherwise a null pointer.

#### `public inline virtual void `[`metadata`](#classdecorator__dsp_1aee9419f6a7e06c8762f5c9a41bbe7e37)`(`[`Meta`](#struct_meta)` * m)` {#classdecorator__dsp_1aee9419f6a7e06c8762f5c9a41bbe7e37}

Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.

#### Parameters
* `m` - the Meta* meta user

#### `public inline virtual void `[`compute`](#classdecorator__dsp_1a43c63a5541bfbff6eb34525019fa8c09)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` {#classdecorator__dsp_1a43c63a5541bfbff6eb34525019fa8c09}

DSP instance computation, to be called with successive in/out audio buffers.

#### Parameters
* `count` - the number of frames to compute 

* `inputs` - the input audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad) 

* `outputs` - the output audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad)

#### `public inline virtual void `[`compute`](#classdecorator__dsp_1a850b718de2fa944e8316d36d5f5956a9)`(double,int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` {#classdecorator__dsp_1a850b718de2fa944e8316d36d5f5956a9}

DSP instance computation: alternative method to be used by subclasses.

#### Parameters
* `date_usec` - the timestamp in microsec given by audio driver. 

* `count` - the number of frames to compute 

* `inputs` - the input audio buffers as an array of non-interleaved FAUSTFLOAT samples (either float, double or quad) 

* `outputs` - the output audio buffers as an array of non-interleaved FAUSTFLOAT samples (either float, double or quad)

#### `protected `[`dsp`](#classdsp)` * `[`fDSP`](#classdecorator__dsp_1ad3702791f702316c2d0ccc725b0c943b) {#classdecorator__dsp_1ad3702791f702316c2d0ccc725b0c943b}

# class `DefaultHeader` {#class_default_header}

```
class DefaultHeader
  : public PacketHeader
```  

Default Header.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  `[`DefaultHeader`](#class_default_header_1a2d1407006ef6d3783cf32e873f159c3d)`(`[`JackTrip`](#class_jack_trip)` * jacktrip)` | 
`public inline virtual  `[`~DefaultHeader`](#class_default_header_1a8c71567104d780b44ac67a2168b51cbb)`()` | 
`public virtual void `[`fillHeaderCommonFromAudio`](#class_default_header_1a8a0f1c704781d62affabaf10c5c3b1f7)`()` | > Todo: Implement this using a [JackTrip](#class_jack_trip) Method (Mediator) member instead of the reference to JackAudio
`public inline virtual void `[`parseHeader`](#class_default_header_1afad35df3b277ad2edb744970456f2beb)`()` | Parse the packet header and take appropriate measures (like change settings, or quit the program if peer settings don't match)
`public virtual void `[`checkPeerSettings`](#class_default_header_1acc84544c5932a10cd00b2f8581f28665)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet)` | 
`public inline virtual void `[`increaseSequenceNumber`](#class_default_header_1a535d0b118153138c957c45a0d47201d1)`()` | Increase sequence number for counter, a 16bit number.
`public inline virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getSequenceNumber`](#class_default_header_1ab2dd76b3f36a15a570b10a74524a8fbb)`() const` | Returns the current sequence number.
`public inline virtual int `[`getHeaderSizeInBytes`](#class_default_header_1abb9d322bdcb1164edaf1065e25560a4d)`() const` | Get the header size in bytes.
`public inline virtual void `[`putHeaderInPacket`](#class_default_header_1abf41156b2edeb250ec260cce8d451faa)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet)` | Put the header in buffer pointed by full_packet.
`public void `[`printHeader`](#class_default_header_1add598241b4b06decbd2153364049f0fb)`() const` | 
`public inline `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getConnectionMode`](#class_default_header_1ac5e5085594cb32149e414cceab1f5b86)`() const` | 
`public inline `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getNumChannels`](#class_default_header_1a275ceed8df3723b5ef7937e16f27a65b)`() const` | 
`public virtual uint64_t `[`getPeerTimeStamp`](#class_default_header_1a43aedd403ca82a60c1fdde8c49db6b1b)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` | 
`public virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerSequenceNumber`](#class_default_header_1ad78e28d49c1a29523885b50b3e6f174d)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` | 
`public virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerBufferSize`](#class_default_header_1a0d68987d96adb608cd5c21d8ac384dba)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` | 
`public virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerSamplingRate`](#class_default_header_1aff5e2b7763e3eea1bba9f625d30d04f7)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` | 
`public virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerBitResolution`](#class_default_header_1a0fb2815db845059992e3990579f8787f)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` | 
`public virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerNumChannels`](#class_default_header_1a4a65580821d9d28814608b192cf8844c)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` | 
`public virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerConnectionMode`](#class_default_header_1a2811cf97104b4f5ed2ab27d9ea7a4f20)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` | 

## Members

#### `public  `[`DefaultHeader`](#class_default_header_1a2d1407006ef6d3783cf32e873f159c3d)`(`[`JackTrip`](#class_jack_trip)` * jacktrip)` {#class_default_header_1a2d1407006ef6d3783cf32e873f159c3d}

#### `public inline virtual  `[`~DefaultHeader`](#class_default_header_1a8c71567104d780b44ac67a2168b51cbb)`()` {#class_default_header_1a8c71567104d780b44ac67a2168b51cbb}

#### `public virtual void `[`fillHeaderCommonFromAudio`](#class_default_header_1a8a0f1c704781d62affabaf10c5c3b1f7)`()` {#class_default_header_1a8a0f1c704781d62affabaf10c5c3b1f7}

> Todo: Implement this using a [JackTrip](#class_jack_trip) Method (Mediator) member instead of the reference to JackAudio

#### `public inline virtual void `[`parseHeader`](#class_default_header_1afad35df3b277ad2edb744970456f2beb)`()` {#class_default_header_1afad35df3b277ad2edb744970456f2beb}

Parse the packet header and take appropriate measures (like change settings, or quit the program if peer settings don't match)

#### `public virtual void `[`checkPeerSettings`](#class_default_header_1acc84544c5932a10cd00b2f8581f28665)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet)` {#class_default_header_1acc84544c5932a10cd00b2f8581f28665}

#### `public inline virtual void `[`increaseSequenceNumber`](#class_default_header_1a535d0b118153138c957c45a0d47201d1)`()` {#class_default_header_1a535d0b118153138c957c45a0d47201d1}

Increase sequence number for counter, a 16bit number.

#### `public inline virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getSequenceNumber`](#class_default_header_1ab2dd76b3f36a15a570b10a74524a8fbb)`() const` {#class_default_header_1ab2dd76b3f36a15a570b10a74524a8fbb}

Returns the current sequence number.

#### Returns
16bit Sequence number

#### `public inline virtual int `[`getHeaderSizeInBytes`](#class_default_header_1abb9d322bdcb1164edaf1065e25560a4d)`() const` {#class_default_header_1abb9d322bdcb1164edaf1065e25560a4d}

Get the header size in bytes.

#### `public inline virtual void `[`putHeaderInPacket`](#class_default_header_1abf41156b2edeb250ec260cce8d451faa)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet)` {#class_default_header_1abf41156b2edeb250ec260cce8d451faa}

Put the header in buffer pointed by full_packet.

#### Parameters
* `full_packet` Pointer to full packet (audio+header). Size must be sizeof(header part) + sizeof(audio part)

#### `public void `[`printHeader`](#class_default_header_1add598241b4b06decbd2153364049f0fb)`() const` {#class_default_header_1add598241b4b06decbd2153364049f0fb}

#### `public inline `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getConnectionMode`](#class_default_header_1ac5e5085594cb32149e414cceab1f5b86)`() const` {#class_default_header_1ac5e5085594cb32149e414cceab1f5b86}

#### `public inline `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getNumChannels`](#class_default_header_1a275ceed8df3723b5ef7937e16f27a65b)`() const` {#class_default_header_1a275ceed8df3723b5ef7937e16f27a65b}

#### `public virtual uint64_t `[`getPeerTimeStamp`](#class_default_header_1a43aedd403ca82a60c1fdde8c49db6b1b)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` {#class_default_header_1a43aedd403ca82a60c1fdde8c49db6b1b}

#### `public virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerSequenceNumber`](#class_default_header_1ad78e28d49c1a29523885b50b3e6f174d)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` {#class_default_header_1ad78e28d49c1a29523885b50b3e6f174d}

#### `public virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerBufferSize`](#class_default_header_1a0d68987d96adb608cd5c21d8ac384dba)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` {#class_default_header_1a0d68987d96adb608cd5c21d8ac384dba}

#### `public virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerSamplingRate`](#class_default_header_1aff5e2b7763e3eea1bba9f625d30d04f7)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` {#class_default_header_1aff5e2b7763e3eea1bba9f625d30d04f7}

#### `public virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerBitResolution`](#class_default_header_1a0fb2815db845059992e3990579f8787f)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` {#class_default_header_1a0fb2815db845059992e3990579f8787f}

#### `public virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerNumChannels`](#class_default_header_1a4a65580821d9d28814608b192cf8844c)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` {#class_default_header_1a4a65580821d9d28814608b192cf8844c}

#### `public virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerConnectionMode`](#class_default_header_1a2811cf97104b4f5ed2ab27d9ea7a4f20)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` {#class_default_header_1a2811cf97104b4f5ed2ab27d9ea7a4f20}

# class `dsp` {#classdsp}

Signal processor definition.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`dsp`](#classdsp_1aa9c58cee7e9cb9493e08e45e899fbfc0)`()` | 
`public inline virtual  `[`~dsp`](#classdsp_1a904f6e16ff07e60e7e938aedfe649fc1)`()` | 
`public int `[`getNumInputs`](#classdsp_1a3399ac81fdfeb067a3471cd946281c5d)`()` | 
`public int `[`getNumOutputs`](#classdsp_1a01bb1642e8b919d2ed35d8d393f33d2f)`()` | 
`public void `[`buildUserInterface`](#classdsp_1ab27caad17a7ca52bba3bcd36c3a85c4e)`(`[`UI`](#struct_u_i)` * ui_interface)` | Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).
`public int `[`getSampleRate`](#classdsp_1ac0ce77ea6910e42b43bfc2263395e1a6)`()` | 
`public void `[`init`](#classdsp_1a2843a2c353aa3567316d5eaa9a636145)`(int sample_rate)` | Global init, calls the following methods:
`public void `[`instanceInit`](#classdsp_1a8ab4470608110ca176a9b38308b84390)`(int sample_rate)` | Init instance state
`public void `[`instanceConstants`](#classdsp_1a60075986fa0134cdc3b0b17016b80b56)`(int sample_rate)` | Init instance constant state
`public void `[`instanceResetUserInterface`](#classdsp_1a63863ef07e1899efad81187dd6956786)`()` | 
`public void `[`instanceClear`](#classdsp_1a5ca27bcf2f39971d3114bed3b37cae0c)`()` | 
`public `[`dsp`](#classdsp)` * `[`clone`](#classdsp_1a6cb4848b6d78e10e27bac4d5c77fb30b)`()` | Return a clone of the instance.
`public void `[`metadata`](#classdsp_1afdcda87c3e7d3036ff7f8a1ba5505469)`(`[`Meta`](#struct_meta)` * m)` | Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.
`public void `[`compute`](#classdsp_1a6b1431a48300dedd89f7b8d316a30a3f)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` | DSP instance computation, to be called with successive in/out audio buffers.
`public inline virtual void `[`compute`](#classdsp_1a80d42585e8050854f2be7c56ec8353bb)`(double,int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` | DSP instance computation: alternative method to be used by subclasses.

## Members

#### `public inline  `[`dsp`](#classdsp_1aa9c58cee7e9cb9493e08e45e899fbfc0)`()` {#classdsp_1aa9c58cee7e9cb9493e08e45e899fbfc0}

#### `public inline virtual  `[`~dsp`](#classdsp_1a904f6e16ff07e60e7e938aedfe649fc1)`()` {#classdsp_1a904f6e16ff07e60e7e938aedfe649fc1}

#### `public int `[`getNumInputs`](#classdsp_1a3399ac81fdfeb067a3471cd946281c5d)`()` {#classdsp_1a3399ac81fdfeb067a3471cd946281c5d}

#### `public int `[`getNumOutputs`](#classdsp_1a01bb1642e8b919d2ed35d8d393f33d2f)`()` {#classdsp_1a01bb1642e8b919d2ed35d8d393f33d2f}

#### `public void `[`buildUserInterface`](#classdsp_1ab27caad17a7ca52bba3bcd36c3a85c4e)`(`[`UI`](#struct_u_i)` * ui_interface)` {#classdsp_1ab27caad17a7ca52bba3bcd36c3a85c4e}

Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).

#### Parameters
* `ui_interface` - the user interface builder

#### `public int `[`getSampleRate`](#classdsp_1ac0ce77ea6910e42b43bfc2263395e1a6)`()` {#classdsp_1ac0ce77ea6910e42b43bfc2263395e1a6}

#### `public void `[`init`](#classdsp_1a2843a2c353aa3567316d5eaa9a636145)`(int sample_rate)` {#classdsp_1a2843a2c353aa3567316d5eaa9a636145}

Global init, calls the following methods:

* static class 'classInit': static tables initialization

* 'instanceInit': constants and instance state initialization

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public void `[`instanceInit`](#classdsp_1a8ab4470608110ca176a9b38308b84390)`(int sample_rate)` {#classdsp_1a8ab4470608110ca176a9b38308b84390}

Init instance state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public void `[`instanceConstants`](#classdsp_1a60075986fa0134cdc3b0b17016b80b56)`(int sample_rate)` {#classdsp_1a60075986fa0134cdc3b0b17016b80b56}

Init instance constant state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public void `[`instanceResetUserInterface`](#classdsp_1a63863ef07e1899efad81187dd6956786)`()` {#classdsp_1a63863ef07e1899efad81187dd6956786}

#### `public void `[`instanceClear`](#classdsp_1a5ca27bcf2f39971d3114bed3b37cae0c)`()` {#classdsp_1a5ca27bcf2f39971d3114bed3b37cae0c}

#### `public `[`dsp`](#classdsp)` * `[`clone`](#classdsp_1a6cb4848b6d78e10e27bac4d5c77fb30b)`()` {#classdsp_1a6cb4848b6d78e10e27bac4d5c77fb30b}

Return a clone of the instance.

#### Returns
a copy of the instance on success, otherwise a null pointer.

#### `public void `[`metadata`](#classdsp_1afdcda87c3e7d3036ff7f8a1ba5505469)`(`[`Meta`](#struct_meta)` * m)` {#classdsp_1afdcda87c3e7d3036ff7f8a1ba5505469}

Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.

#### Parameters
* `m` - the Meta* meta user

#### `public void `[`compute`](#classdsp_1a6b1431a48300dedd89f7b8d316a30a3f)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` {#classdsp_1a6b1431a48300dedd89f7b8d316a30a3f}

DSP instance computation, to be called with successive in/out audio buffers.

#### Parameters
* `count` - the number of frames to compute 

* `inputs` - the input audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad) 

* `outputs` - the output audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad)

#### `public inline virtual void `[`compute`](#classdsp_1a80d42585e8050854f2be7c56ec8353bb)`(double,int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` {#classdsp_1a80d42585e8050854f2be7c56ec8353bb}

DSP instance computation: alternative method to be used by subclasses.

#### Parameters
* `date_usec` - the timestamp in microsec given by audio driver. 

* `count` - the number of frames to compute 

* `inputs` - the input audio buffers as an array of non-interleaved FAUSTFLOAT samples (either float, double or quad) 

* `outputs` - the output audio buffers as an array of non-interleaved FAUSTFLOAT samples (either float, double or quad)

# class `dsp_factory` {#classdsp__factory}

DSP factory class.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public std::string `[`getName`](#classdsp__factory_1a6ddde65a7c1c6e8cb0f4997eb8e2d973)`()` | 
`public std::string `[`getSHAKey`](#classdsp__factory_1a8070a9bf8f5e00bea196bd3f6d4e3b63)`()` | 
`public std::string `[`getDSPCode`](#classdsp__factory_1ab8d9446933f8889a6456261bbc576c0f)`()` | 
`public std::string `[`getCompileOptions`](#classdsp__factory_1aad524353f11190194009146c14f1c896)`()` | 
`public std::vector< std::string > `[`getLibraryList`](#classdsp__factory_1a04bf162e562fe651ac9913c0e64a420c)`()` | 
`public std::vector< std::string > `[`getIncludePathnames`](#classdsp__factory_1a4d7311df83f9671ea261525a67598062)`()` | 
`public `[`dsp`](#classdsp)` * `[`createDSPInstance`](#classdsp__factory_1a3012da65acd38fe2e8a177faec79fa3a)`()` | 
`public void `[`setMemoryManager`](#classdsp__factory_1a9c8b7e522919be3c2c44a667072472b8)`(`[`dsp_memory_manager`](#structdsp__memory__manager)` * manager)` | 
`public `[`dsp_memory_manager`](#structdsp__memory__manager)` * `[`getMemoryManager`](#classdsp__factory_1a056c74b34c4cd963121ac611139f5dfa)`()` | 
`protected inline virtual  `[`~dsp_factory`](#classdsp__factory_1a6dc961a12d9e141626e9a53d4aad5806)`()` | 

## Members

#### `public std::string `[`getName`](#classdsp__factory_1a6ddde65a7c1c6e8cb0f4997eb8e2d973)`()` {#classdsp__factory_1a6ddde65a7c1c6e8cb0f4997eb8e2d973}

#### `public std::string `[`getSHAKey`](#classdsp__factory_1a8070a9bf8f5e00bea196bd3f6d4e3b63)`()` {#classdsp__factory_1a8070a9bf8f5e00bea196bd3f6d4e3b63}

#### `public std::string `[`getDSPCode`](#classdsp__factory_1ab8d9446933f8889a6456261bbc576c0f)`()` {#classdsp__factory_1ab8d9446933f8889a6456261bbc576c0f}

#### `public std::string `[`getCompileOptions`](#classdsp__factory_1aad524353f11190194009146c14f1c896)`()` {#classdsp__factory_1aad524353f11190194009146c14f1c896}

#### `public std::vector< std::string > `[`getLibraryList`](#classdsp__factory_1a04bf162e562fe651ac9913c0e64a420c)`()` {#classdsp__factory_1a04bf162e562fe651ac9913c0e64a420c}

#### `public std::vector< std::string > `[`getIncludePathnames`](#classdsp__factory_1a4d7311df83f9671ea261525a67598062)`()` {#classdsp__factory_1a4d7311df83f9671ea261525a67598062}

#### `public `[`dsp`](#classdsp)` * `[`createDSPInstance`](#classdsp__factory_1a3012da65acd38fe2e8a177faec79fa3a)`()` {#classdsp__factory_1a3012da65acd38fe2e8a177faec79fa3a}

#### `public void `[`setMemoryManager`](#classdsp__factory_1a9c8b7e522919be3c2c44a667072472b8)`(`[`dsp_memory_manager`](#structdsp__memory__manager)` * manager)` {#classdsp__factory_1a9c8b7e522919be3c2c44a667072472b8}

#### `public `[`dsp_memory_manager`](#structdsp__memory__manager)` * `[`getMemoryManager`](#classdsp__factory_1a056c74b34c4cd963121ac611139f5dfa)`()` {#classdsp__factory_1a056c74b34c4cd963121ac611139f5dfa}

#### `protected inline virtual  `[`~dsp_factory`](#classdsp__factory_1a6dc961a12d9e141626e9a53d4aad5806)`()` {#classdsp__factory_1a6dc961a12d9e141626e9a53d4aad5806}

# class `Effects` {#class_effects}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`Effects`](#class_effects_1a5d72213afc8146eb8cf4aa43418ef7a4)`(bool outGoingLimiterOn)` | 
`public inline  `[`~Effects`](#class_effects_1abcac8dac82151a97284e63dcc5919f37)`()` | 
`public inline unsigned int `[`getNumClientsAssumed`](#class_effects_1a8ced4d958d551c0f6485a0de258c7716)`()` | 
`public inline `[`LIMITER_MODE`](#class_effects_1a22f55001aaa42515807c45b4907f9749)` `[`getLimit`](#class_effects_1aad6c87df0c8860779e729e4f1ca0197c)`()` | 
`public inline void `[`setNoLimiters`](#class_effects_1abf97e54dbb59934a68fe0c27fde4dc21)`()` | 
`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getInCompressor`](#class_effects_1ae5abf3ec7fd0ab389252ec76961c67a8)`()` | 
`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getOutCompressor`](#class_effects_1a92918313fdda07bfff507851eceb9f6f)`()` | 
`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getInZitarev`](#class_effects_1ad8e3da4f9af54ad32df9407d9d07678a)`()` | 
`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getOutZitarev`](#class_effects_1ab984ff1181d61559f51733a2f5d081b0)`()` | 
`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getInFreeverb`](#class_effects_1ac9e300436cf09db9dd921ff038bf508b)`()` | 
`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getOutFreeverb`](#class_effects_1a35bbd03014a95e0fea9c1071aa5e8269)`()` | 
`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getInLimiter`](#class_effects_1a479d7a0e602611b43cd19b5768005f1f)`()` | 
`public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getOutLimiter`](#class_effects_1a6f58be8c6cf76bc8bf463abf74e0f2ec)`()` | 
`public inline bool `[`getHaveEffect`](#class_effects_1a4eb993f8d4fd18afdec36efe2c5c5b05)`()` | 
`public inline bool `[`getHaveLimiter`](#class_effects_1aaf75e0fef072758af6af865bdd08ddcc)`()` | 
`public inline void `[`setVerboseFlag`](#class_effects_1aee37eb9a81f5d1ef2591c65bf6e8c581)`(int v)` | 
`public inline int `[`getNumIncomingChans`](#class_effects_1a130c048bfc2f5d922c6d10512eed65fc)`()` | 
`public inline int `[`getOutgoingNumChans`](#class_effects_1a616ae55ae19ebd2a388567c4d71150d9)`()` | 
`public inline std::vector< `[`ProcessPlugin`](#class_process_plugin)` * > `[`allocateIncomingEffects`](#class_effects_1af46ed66a2bc7a682536fa68a920cab09)`(int nIncomingChans)` | 
`public inline std::vector< `[`ProcessPlugin`](#class_process_plugin)` * > `[`allocateOutgoingEffects`](#class_effects_1aff3b5a8b26223c825daa0cc73b8a8d00)`(int nOutgoingChans)` | 
`public inline void `[`printHelp`](#class_effects_1a1c1e4ae0297fdc6d65fca15c361f966c)`(char * command,char helpCase)` | 
`public inline int `[`setCompressorPresetIndexFrom1`](#class_effects_1acebd5f990938b796319382d75451dc59)`(unsigned long presetIndexFrom1,InOrOut io)` | 
`public inline int `[`parseCompresserArgs`](#class_effects_1aaf78bb3ca7258922600d355ab546a834)`(char * args,InOrOut inOrOut)` | 
`public inline int `[`parseEffectsOptArg`](#class_effects_1ad3780e901b7055baf626b0c7406ec66c)`(char * cmd,char * optarg)` | 
`public inline int `[`parseLimiterOptArg`](#class_effects_1a927afb5e6289ff48b33164074e4e244b)`(char * cmd,char * optarg)` | 
`public inline int `[`parseAssumedNumClientsOptArg`](#class_effects_1a52f34d0f75d3f9777bf61b7e4292bb0f)`(char * cmd,char * optarg)` | 
`enum `[`LIMITER_MODE`](#class_effects_1a22f55001aaa42515807c45b4907f9749) | 

## Members

#### `public inline  `[`Effects`](#class_effects_1a5d72213afc8146eb8cf4aa43418ef7a4)`(bool outGoingLimiterOn)` {#class_effects_1a5d72213afc8146eb8cf4aa43418ef7a4}

#### `public inline  `[`~Effects`](#class_effects_1abcac8dac82151a97284e63dcc5919f37)`()` {#class_effects_1abcac8dac82151a97284e63dcc5919f37}

#### `public inline unsigned int `[`getNumClientsAssumed`](#class_effects_1a8ced4d958d551c0f6485a0de258c7716)`()` {#class_effects_1a8ced4d958d551c0f6485a0de258c7716}

#### `public inline `[`LIMITER_MODE`](#class_effects_1a22f55001aaa42515807c45b4907f9749)` `[`getLimit`](#class_effects_1aad6c87df0c8860779e729e4f1ca0197c)`()` {#class_effects_1aad6c87df0c8860779e729e4f1ca0197c}

#### `public inline void `[`setNoLimiters`](#class_effects_1abf97e54dbb59934a68fe0c27fde4dc21)`()` {#class_effects_1abf97e54dbb59934a68fe0c27fde4dc21}

#### `public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getInCompressor`](#class_effects_1ae5abf3ec7fd0ab389252ec76961c67a8)`()` {#class_effects_1ae5abf3ec7fd0ab389252ec76961c67a8}

#### `public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getOutCompressor`](#class_effects_1a92918313fdda07bfff507851eceb9f6f)`()` {#class_effects_1a92918313fdda07bfff507851eceb9f6f}

#### `public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getInZitarev`](#class_effects_1ad8e3da4f9af54ad32df9407d9d07678a)`()` {#class_effects_1ad8e3da4f9af54ad32df9407d9d07678a}

#### `public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getOutZitarev`](#class_effects_1ab984ff1181d61559f51733a2f5d081b0)`()` {#class_effects_1ab984ff1181d61559f51733a2f5d081b0}

#### `public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getInFreeverb`](#class_effects_1ac9e300436cf09db9dd921ff038bf508b)`()` {#class_effects_1ac9e300436cf09db9dd921ff038bf508b}

#### `public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getOutFreeverb`](#class_effects_1a35bbd03014a95e0fea9c1071aa5e8269)`()` {#class_effects_1a35bbd03014a95e0fea9c1071aa5e8269}

#### `public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getInLimiter`](#class_effects_1a479d7a0e602611b43cd19b5768005f1f)`()` {#class_effects_1a479d7a0e602611b43cd19b5768005f1f}

#### `public inline `[`ProcessPlugin`](#class_process_plugin)` * `[`getOutLimiter`](#class_effects_1a6f58be8c6cf76bc8bf463abf74e0f2ec)`()` {#class_effects_1a6f58be8c6cf76bc8bf463abf74e0f2ec}

#### `public inline bool `[`getHaveEffect`](#class_effects_1a4eb993f8d4fd18afdec36efe2c5c5b05)`()` {#class_effects_1a4eb993f8d4fd18afdec36efe2c5c5b05}

#### `public inline bool `[`getHaveLimiter`](#class_effects_1aaf75e0fef072758af6af865bdd08ddcc)`()` {#class_effects_1aaf75e0fef072758af6af865bdd08ddcc}

#### `public inline void `[`setVerboseFlag`](#class_effects_1aee37eb9a81f5d1ef2591c65bf6e8c581)`(int v)` {#class_effects_1aee37eb9a81f5d1ef2591c65bf6e8c581}

#### `public inline int `[`getNumIncomingChans`](#class_effects_1a130c048bfc2f5d922c6d10512eed65fc)`()` {#class_effects_1a130c048bfc2f5d922c6d10512eed65fc}

#### `public inline int `[`getOutgoingNumChans`](#class_effects_1a616ae55ae19ebd2a388567c4d71150d9)`()` {#class_effects_1a616ae55ae19ebd2a388567c4d71150d9}

#### `public inline std::vector< `[`ProcessPlugin`](#class_process_plugin)` * > `[`allocateIncomingEffects`](#class_effects_1af46ed66a2bc7a682536fa68a920cab09)`(int nIncomingChans)` {#class_effects_1af46ed66a2bc7a682536fa68a920cab09}

#### `public inline std::vector< `[`ProcessPlugin`](#class_process_plugin)` * > `[`allocateOutgoingEffects`](#class_effects_1aff3b5a8b26223c825daa0cc73b8a8d00)`(int nOutgoingChans)` {#class_effects_1aff3b5a8b26223c825daa0cc73b8a8d00}

#### `public inline void `[`printHelp`](#class_effects_1a1c1e4ae0297fdc6d65fca15c361f966c)`(char * command,char helpCase)` {#class_effects_1a1c1e4ae0297fdc6d65fca15c361f966c}

#### `public inline int `[`setCompressorPresetIndexFrom1`](#class_effects_1acebd5f990938b796319382d75451dc59)`(unsigned long presetIndexFrom1,InOrOut io)` {#class_effects_1acebd5f990938b796319382d75451dc59}

#### `public inline int `[`parseCompresserArgs`](#class_effects_1aaf78bb3ca7258922600d355ab546a834)`(char * args,InOrOut inOrOut)` {#class_effects_1aaf78bb3ca7258922600d355ab546a834}

#### `public inline int `[`parseEffectsOptArg`](#class_effects_1ad3780e901b7055baf626b0c7406ec66c)`(char * cmd,char * optarg)` {#class_effects_1ad3780e901b7055baf626b0c7406ec66c}

#### `public inline int `[`parseLimiterOptArg`](#class_effects_1a927afb5e6289ff48b33164074e4e244b)`(char * cmd,char * optarg)` {#class_effects_1a927afb5e6289ff48b33164074e4e244b}

#### `public inline int `[`parseAssumedNumClientsOptArg`](#class_effects_1a52f34d0f75d3f9777bf61b7e4292bb0f)`(char * cmd,char * optarg)` {#class_effects_1a52f34d0f75d3f9777bf61b7e4292bb0f}

#### `enum `[`LIMITER_MODE`](#class_effects_1a22f55001aaa42515807c45b4907f9749) {#class_effects_1a22f55001aaa42515807c45b4907f9749}

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
LIMITER_NONE            | 
LIMITER_INCOMING            | 
LIMITER_OUTGOING            | 
LIMITER_BOTH            | 

# class `EmptyHeader` {#class_empty_header}

```
class EmptyHeader
  : public PacketHeader
```  

Empty Header to use with systems that don't include a header.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  `[`EmptyHeader`](#class_empty_header_1acd85d44a1839b354120212d55561d195)`(`[`JackTrip`](#class_jack_trip)` * jacktrip)` | 
`public inline virtual  `[`~EmptyHeader`](#class_empty_header_1ab4d64cf661c84f4972442afdcb79fbcf)`()` | 
`public inline virtual void `[`fillHeaderCommonFromAudio`](#class_empty_header_1ac94011891b088bdfc4593730cd3cc533)`()` | > Todo: Implement this using a [JackTrip](#class_jack_trip) Method (Mediator) member instead of the reference to JackAudio
`public inline virtual void `[`parseHeader`](#class_empty_header_1a91cf9db64cdc29443a32620c53c34f5d)`()` | Parse the packet header and take appropriate measures (like change settings, or quit the program if peer settings don't match)
`public inline virtual void `[`checkPeerSettings`](#class_empty_header_1a35186e3c807439699f6843c75c77d22f)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *)` | 
`public inline virtual void `[`increaseSequenceNumber`](#class_empty_header_1a2148aec9601f457c58a356cf3d278d0a)`()` | Increase sequence number for counter, a 16bit number.
`public inline virtual int `[`getHeaderSizeInBytes`](#class_empty_header_1a3327c3171b967cdf88d2b50626beb807)`() const` | Get the header size in bytes.
`public inline virtual uint64_t `[`getPeerTimeStamp`](#class_empty_header_1a8bbf2b30d4bcaf88b24b98641f91cb37)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` | 
`public inline virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerSequenceNumber`](#class_empty_header_1a664115b5fba68f7a1a7763f623419d20)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` | 
`public inline virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerBufferSize`](#class_empty_header_1a84d746098227a18ce26c6bc5d3395b34)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` | 
`public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerSamplingRate`](#class_empty_header_1a404b07305f522a12a2f6e753e365b3c2)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` | 
`public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerBitResolution`](#class_empty_header_1a701bdf97a1538409c829f79db5f1bdd7)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` | 
`public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerNumChannels`](#class_empty_header_1ad55cc5645387840ab562d8f056eb1df4)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` | 
`public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerConnectionMode`](#class_empty_header_1ad66688e25268eb3049e0fa686f8339fa)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` | 
`public inline virtual void `[`putHeaderInPacket`](#class_empty_header_1a23c5cc1d3f9e044898ebf95b7f327114)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet)` | Put the header in buffer pointed by full_packet.

## Members

#### `public  `[`EmptyHeader`](#class_empty_header_1acd85d44a1839b354120212d55561d195)`(`[`JackTrip`](#class_jack_trip)` * jacktrip)` {#class_empty_header_1acd85d44a1839b354120212d55561d195}

#### `public inline virtual  `[`~EmptyHeader`](#class_empty_header_1ab4d64cf661c84f4972442afdcb79fbcf)`()` {#class_empty_header_1ab4d64cf661c84f4972442afdcb79fbcf}

#### `public inline virtual void `[`fillHeaderCommonFromAudio`](#class_empty_header_1ac94011891b088bdfc4593730cd3cc533)`()` {#class_empty_header_1ac94011891b088bdfc4593730cd3cc533}

> Todo: Implement this using a [JackTrip](#class_jack_trip) Method (Mediator) member instead of the reference to JackAudio

#### `public inline virtual void `[`parseHeader`](#class_empty_header_1a91cf9db64cdc29443a32620c53c34f5d)`()` {#class_empty_header_1a91cf9db64cdc29443a32620c53c34f5d}

Parse the packet header and take appropriate measures (like change settings, or quit the program if peer settings don't match)

#### `public inline virtual void `[`checkPeerSettings`](#class_empty_header_1a35186e3c807439699f6843c75c77d22f)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *)` {#class_empty_header_1a35186e3c807439699f6843c75c77d22f}

#### `public inline virtual void `[`increaseSequenceNumber`](#class_empty_header_1a2148aec9601f457c58a356cf3d278d0a)`()` {#class_empty_header_1a2148aec9601f457c58a356cf3d278d0a}

Increase sequence number for counter, a 16bit number.

#### `public inline virtual int `[`getHeaderSizeInBytes`](#class_empty_header_1a3327c3171b967cdf88d2b50626beb807)`() const` {#class_empty_header_1a3327c3171b967cdf88d2b50626beb807}

Get the header size in bytes.

#### `public inline virtual uint64_t `[`getPeerTimeStamp`](#class_empty_header_1a8bbf2b30d4bcaf88b24b98641f91cb37)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` {#class_empty_header_1a8bbf2b30d4bcaf88b24b98641f91cb37}

#### `public inline virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerSequenceNumber`](#class_empty_header_1a664115b5fba68f7a1a7763f623419d20)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` {#class_empty_header_1a664115b5fba68f7a1a7763f623419d20}

#### `public inline virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerBufferSize`](#class_empty_header_1a84d746098227a18ce26c6bc5d3395b34)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` {#class_empty_header_1a84d746098227a18ce26c6bc5d3395b34}

#### `public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerSamplingRate`](#class_empty_header_1a404b07305f522a12a2f6e753e365b3c2)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` {#class_empty_header_1a404b07305f522a12a2f6e753e365b3c2}

#### `public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerBitResolution`](#class_empty_header_1a701bdf97a1538409c829f79db5f1bdd7)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` {#class_empty_header_1a701bdf97a1538409c829f79db5f1bdd7}

#### `public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerNumChannels`](#class_empty_header_1ad55cc5645387840ab562d8f056eb1df4)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` {#class_empty_header_1ad55cc5645387840ab562d8f056eb1df4}

#### `public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerConnectionMode`](#class_empty_header_1ad66688e25268eb3049e0fa686f8339fa)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` {#class_empty_header_1ad66688e25268eb3049e0fa686f8339fa}

#### `public inline virtual void `[`putHeaderInPacket`](#class_empty_header_1a23c5cc1d3f9e044898ebf95b7f327114)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet)` {#class_empty_header_1a23c5cc1d3f9e044898ebf95b7f327114}

Put the header in buffer pointed by full_packet.

#### Parameters
* `full_packet` Pointer to full packet (audio+header). Size must be sizeof(header part) + sizeof(audio part)

# class `ExpValueConverter` {#class_exp_value_converter}

```
class ExpValueConverter
  : public LinearValueConverter
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`ExpValueConverter`](#class_exp_value_converter_1a6aed9af0314a73bf36f8cd7fb2c567f1)`(double umin,double umax,double fmin,double fmax)` | 
`public inline virtual double `[`ui2faust`](#class_exp_value_converter_1a6c7842cb9d0b741f517ca49978c14e19)`(double x)` | 
`public inline virtual double `[`faust2ui`](#class_exp_value_converter_1a9c480cc939300549a1eacac491c12158)`(double x)` | 

## Members

#### `public inline  `[`ExpValueConverter`](#class_exp_value_converter_1a6aed9af0314a73bf36f8cd7fb2c567f1)`(double umin,double umax,double fmin,double fmax)` {#class_exp_value_converter_1a6aed9af0314a73bf36f8cd7fb2c567f1}

#### `public inline virtual double `[`ui2faust`](#class_exp_value_converter_1a6c7842cb9d0b741f517ca49978c14e19)`(double x)` {#class_exp_value_converter_1a6c7842cb9d0b741f517ca49978c14e19}

#### `public inline virtual double `[`faust2ui`](#class_exp_value_converter_1a9c480cc939300549a1eacac491c12158)`(double x)` {#class_exp_value_converter_1a9c480cc939300549a1eacac491c12158}

# class `freeverbdsp` {#classfreeverbdsp}

```
class freeverbdsp
  : public dsp
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline virtual void `[`metadata`](#classfreeverbdsp_1a9dbcc9453a5cd01188429814a8034e38)`(`[`Meta`](#struct_meta)` * m)` | Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.
`public inline virtual int `[`getNumInputs`](#classfreeverbdsp_1a2eb4623408630140bad1f3013e445011)`()` | 
`public inline virtual int `[`getNumOutputs`](#classfreeverbdsp_1a930314efdf612e4afefddc3be63da692)`()` | 
`public inline virtual int `[`getInputRate`](#classfreeverbdsp_1a62b0a9a76196c56d204275c4f99a5a37)`(int channel)` | 
`public inline virtual int `[`getOutputRate`](#classfreeverbdsp_1a52b195d5306a37b89aebe8f407fdd952)`(int channel)` | 
`public inline virtual void `[`instanceConstants`](#classfreeverbdsp_1a92385e13ecdbd818941f59cbfbe509ab)`(int sample_rate)` | Init instance constant state
`public inline virtual void `[`instanceResetUserInterface`](#classfreeverbdsp_1a1316459e34cf7ba4c3dd31d89e927b60)`()` | 
`public inline virtual void `[`instanceClear`](#classfreeverbdsp_1a100e5e4fd026ae77ca3849b31ac45b32)`()` | 
`public inline virtual void `[`init`](#classfreeverbdsp_1a3fda7c60d172938e68393ef2a1716666)`(int sample_rate)` | Global init, calls the following methods:
`public inline virtual void `[`instanceInit`](#classfreeverbdsp_1a52008de205527f7253ef23282c50d3b2)`(int sample_rate)` | Init instance state
`public inline virtual `[`freeverbdsp`](#classfreeverbdsp)` * `[`clone`](#classfreeverbdsp_1ae7173ea79bf86be474f29184fa22a744)`()` | Return a clone of the instance.
`public inline virtual int `[`getSampleRate`](#classfreeverbdsp_1ab5eea18009befd1ae3e2b0ad824932e8)`()` | 
`public inline virtual void `[`buildUserInterface`](#classfreeverbdsp_1a9f859e1481220e6eae32e9667c149577)`(`[`UI`](#struct_u_i)` * ui_interface)` | Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).
`public inline virtual void `[`compute`](#classfreeverbdsp_1ae0e5ce50d5101daa2b0da312800c7a61)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` | DSP instance computation, to be called with successive in/out audio buffers.

## Members

#### `public inline virtual void `[`metadata`](#classfreeverbdsp_1a9dbcc9453a5cd01188429814a8034e38)`(`[`Meta`](#struct_meta)` * m)` {#classfreeverbdsp_1a9dbcc9453a5cd01188429814a8034e38}

Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.

#### Parameters
* `m` - the Meta* meta user

#### `public inline virtual int `[`getNumInputs`](#classfreeverbdsp_1a2eb4623408630140bad1f3013e445011)`()` {#classfreeverbdsp_1a2eb4623408630140bad1f3013e445011}

#### `public inline virtual int `[`getNumOutputs`](#classfreeverbdsp_1a930314efdf612e4afefddc3be63da692)`()` {#classfreeverbdsp_1a930314efdf612e4afefddc3be63da692}

#### `public inline virtual int `[`getInputRate`](#classfreeverbdsp_1a62b0a9a76196c56d204275c4f99a5a37)`(int channel)` {#classfreeverbdsp_1a62b0a9a76196c56d204275c4f99a5a37}

#### `public inline virtual int `[`getOutputRate`](#classfreeverbdsp_1a52b195d5306a37b89aebe8f407fdd952)`(int channel)` {#classfreeverbdsp_1a52b195d5306a37b89aebe8f407fdd952}

#### `public inline virtual void `[`instanceConstants`](#classfreeverbdsp_1a92385e13ecdbd818941f59cbfbe509ab)`(int sample_rate)` {#classfreeverbdsp_1a92385e13ecdbd818941f59cbfbe509ab}

Init instance constant state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual void `[`instanceResetUserInterface`](#classfreeverbdsp_1a1316459e34cf7ba4c3dd31d89e927b60)`()` {#classfreeverbdsp_1a1316459e34cf7ba4c3dd31d89e927b60}

#### `public inline virtual void `[`instanceClear`](#classfreeverbdsp_1a100e5e4fd026ae77ca3849b31ac45b32)`()` {#classfreeverbdsp_1a100e5e4fd026ae77ca3849b31ac45b32}

#### `public inline virtual void `[`init`](#classfreeverbdsp_1a3fda7c60d172938e68393ef2a1716666)`(int sample_rate)` {#classfreeverbdsp_1a3fda7c60d172938e68393ef2a1716666}

Global init, calls the following methods:

* static class 'classInit': static tables initialization

* 'instanceInit': constants and instance state initialization

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual void `[`instanceInit`](#classfreeverbdsp_1a52008de205527f7253ef23282c50d3b2)`(int sample_rate)` {#classfreeverbdsp_1a52008de205527f7253ef23282c50d3b2}

Init instance state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual `[`freeverbdsp`](#classfreeverbdsp)` * `[`clone`](#classfreeverbdsp_1ae7173ea79bf86be474f29184fa22a744)`()` {#classfreeverbdsp_1ae7173ea79bf86be474f29184fa22a744}

Return a clone of the instance.

#### Returns
a copy of the instance on success, otherwise a null pointer.

#### `public inline virtual int `[`getSampleRate`](#classfreeverbdsp_1ab5eea18009befd1ae3e2b0ad824932e8)`()` {#classfreeverbdsp_1ab5eea18009befd1ae3e2b0ad824932e8}

#### `public inline virtual void `[`buildUserInterface`](#classfreeverbdsp_1a9f859e1481220e6eae32e9667c149577)`(`[`UI`](#struct_u_i)` * ui_interface)` {#classfreeverbdsp_1a9f859e1481220e6eae32e9667c149577}

Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).

#### Parameters
* `ui_interface` - the user interface builder

#### `public inline virtual void `[`compute`](#classfreeverbdsp_1ae0e5ce50d5101daa2b0da312800c7a61)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` {#classfreeverbdsp_1ae0e5ce50d5101daa2b0da312800c7a61}

DSP instance computation, to be called with successive in/out audio buffers.

#### Parameters
* `count` - the number of frames to compute 

* `inputs` - the input audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad) 

* `outputs` - the output audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad)

# class `freeverbmonodsp` {#classfreeverbmonodsp}

```
class freeverbmonodsp
  : public dsp
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline virtual void `[`metadata`](#classfreeverbmonodsp_1ad03871ddd463bf290c858fa5f3a9eb39)`(`[`Meta`](#struct_meta)` * m)` | Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.
`public inline virtual int `[`getNumInputs`](#classfreeverbmonodsp_1a58a8b40a2fd8c69c9493c9ee41239366)`()` | 
`public inline virtual int `[`getNumOutputs`](#classfreeverbmonodsp_1ac8294ea49a7c067aed0d3e6d1efbabb3)`()` | 
`public inline virtual int `[`getInputRate`](#classfreeverbmonodsp_1a4a94ea5c6d56df34332564f41fb7004d)`(int channel)` | 
`public inline virtual int `[`getOutputRate`](#classfreeverbmonodsp_1a90ef7804e931ef9072427cf04f5c5bd5)`(int channel)` | 
`public inline virtual void `[`instanceConstants`](#classfreeverbmonodsp_1af3be5c93c973cc91edff02553e0d881f)`(int sample_rate)` | Init instance constant state
`public inline virtual void `[`instanceResetUserInterface`](#classfreeverbmonodsp_1ae875ade1efefb422c6ea23559e9f66a6)`()` | 
`public inline virtual void `[`instanceClear`](#classfreeverbmonodsp_1aaa871d87abde1c5e0ae8ee9da0c66906)`()` | 
`public inline virtual void `[`init`](#classfreeverbmonodsp_1ac8e3b5c17c53195a097191dbdf9a256e)`(int sample_rate)` | Global init, calls the following methods:
`public inline virtual void `[`instanceInit`](#classfreeverbmonodsp_1ac140223f5366f2f9ebf292beda168000)`(int sample_rate)` | Init instance state
`public inline virtual `[`freeverbmonodsp`](#classfreeverbmonodsp)` * `[`clone`](#classfreeverbmonodsp_1a027b629b7eef2b176b25b9e0515f3ddd)`()` | Return a clone of the instance.
`public inline virtual int `[`getSampleRate`](#classfreeverbmonodsp_1a1f0a90b3635c802f7956b7b2ccf85afa)`()` | 
`public inline virtual void `[`buildUserInterface`](#classfreeverbmonodsp_1ac827e6f43cd7efb62db827f6648b1e63)`(`[`UI`](#struct_u_i)` * ui_interface)` | Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).
`public inline virtual void `[`compute`](#classfreeverbmonodsp_1ae115604c5d01734e99f2c905739e81e5)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` | DSP instance computation, to be called with successive in/out audio buffers.

## Members

#### `public inline virtual void `[`metadata`](#classfreeverbmonodsp_1ad03871ddd463bf290c858fa5f3a9eb39)`(`[`Meta`](#struct_meta)` * m)` {#classfreeverbmonodsp_1ad03871ddd463bf290c858fa5f3a9eb39}

Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.

#### Parameters
* `m` - the Meta* meta user

#### `public inline virtual int `[`getNumInputs`](#classfreeverbmonodsp_1a58a8b40a2fd8c69c9493c9ee41239366)`()` {#classfreeverbmonodsp_1a58a8b40a2fd8c69c9493c9ee41239366}

#### `public inline virtual int `[`getNumOutputs`](#classfreeverbmonodsp_1ac8294ea49a7c067aed0d3e6d1efbabb3)`()` {#classfreeverbmonodsp_1ac8294ea49a7c067aed0d3e6d1efbabb3}

#### `public inline virtual int `[`getInputRate`](#classfreeverbmonodsp_1a4a94ea5c6d56df34332564f41fb7004d)`(int channel)` {#classfreeverbmonodsp_1a4a94ea5c6d56df34332564f41fb7004d}

#### `public inline virtual int `[`getOutputRate`](#classfreeverbmonodsp_1a90ef7804e931ef9072427cf04f5c5bd5)`(int channel)` {#classfreeverbmonodsp_1a90ef7804e931ef9072427cf04f5c5bd5}

#### `public inline virtual void `[`instanceConstants`](#classfreeverbmonodsp_1af3be5c93c973cc91edff02553e0d881f)`(int sample_rate)` {#classfreeverbmonodsp_1af3be5c93c973cc91edff02553e0d881f}

Init instance constant state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual void `[`instanceResetUserInterface`](#classfreeverbmonodsp_1ae875ade1efefb422c6ea23559e9f66a6)`()` {#classfreeverbmonodsp_1ae875ade1efefb422c6ea23559e9f66a6}

#### `public inline virtual void `[`instanceClear`](#classfreeverbmonodsp_1aaa871d87abde1c5e0ae8ee9da0c66906)`()` {#classfreeverbmonodsp_1aaa871d87abde1c5e0ae8ee9da0c66906}

#### `public inline virtual void `[`init`](#classfreeverbmonodsp_1ac8e3b5c17c53195a097191dbdf9a256e)`(int sample_rate)` {#classfreeverbmonodsp_1ac8e3b5c17c53195a097191dbdf9a256e}

Global init, calls the following methods:

* static class 'classInit': static tables initialization

* 'instanceInit': constants and instance state initialization

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual void `[`instanceInit`](#classfreeverbmonodsp_1ac140223f5366f2f9ebf292beda168000)`(int sample_rate)` {#classfreeverbmonodsp_1ac140223f5366f2f9ebf292beda168000}

Init instance state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual `[`freeverbmonodsp`](#classfreeverbmonodsp)` * `[`clone`](#classfreeverbmonodsp_1a027b629b7eef2b176b25b9e0515f3ddd)`()` {#classfreeverbmonodsp_1a027b629b7eef2b176b25b9e0515f3ddd}

Return a clone of the instance.

#### Returns
a copy of the instance on success, otherwise a null pointer.

#### `public inline virtual int `[`getSampleRate`](#classfreeverbmonodsp_1a1f0a90b3635c802f7956b7b2ccf85afa)`()` {#classfreeverbmonodsp_1a1f0a90b3635c802f7956b7b2ccf85afa}

#### `public inline virtual void `[`buildUserInterface`](#classfreeverbmonodsp_1ac827e6f43cd7efb62db827f6648b1e63)`(`[`UI`](#struct_u_i)` * ui_interface)` {#classfreeverbmonodsp_1ac827e6f43cd7efb62db827f6648b1e63}

Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).

#### Parameters
* `ui_interface` - the user interface builder

#### `public inline virtual void `[`compute`](#classfreeverbmonodsp_1ae115604c5d01734e99f2c905739e81e5)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` {#classfreeverbmonodsp_1ae115604c5d01734e99f2c905739e81e5}

DSP instance computation, to be called with successive in/out audio buffers.

#### Parameters
* `count` - the number of frames to compute 

* `inputs` - the input audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad) 

* `outputs` - the output audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad)

# class `Interpolator` {#class_interpolator}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`Interpolator`](#class_interpolator_1aa2c25952193570d656fe6d08598eccaa)`(double lo,double hi,double v1,double v2)` | 
`public inline double `[`operator()`](#class_interpolator_1aaa2edfdf5022fec8de6d914808a0eb17)`(double v)` | 
`public inline void `[`getLowHigh`](#class_interpolator_1a50780738f51ac8069f3931c524261536)`(double & amin,double & amax)` | 

## Members

#### `public inline  `[`Interpolator`](#class_interpolator_1aa2c25952193570d656fe6d08598eccaa)`(double lo,double hi,double v1,double v2)` {#class_interpolator_1aa2c25952193570d656fe6d08598eccaa}

#### `public inline double `[`operator()`](#class_interpolator_1aaa2edfdf5022fec8de6d914808a0eb17)`(double v)` {#class_interpolator_1aaa2edfdf5022fec8de6d914808a0eb17}

#### `public inline void `[`getLowHigh`](#class_interpolator_1a50780738f51ac8069f3931c524261536)`(double & amin,double & amax)` {#class_interpolator_1a50780738f51ac8069f3931c524261536}

# class `Interpolator3pt` {#class_interpolator3pt}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`Interpolator3pt`](#class_interpolator3pt_1aabff59935bf573aaa4c91ac52b498ad1)`(double lo,double mi,double hi,double v1,double vm,double v2)` | 
`public inline double `[`operator()`](#class_interpolator3pt_1a18f6068ae0785cecfbfb2b38d061ee06)`(double x)` | 
`public inline void `[`getMappingValues`](#class_interpolator3pt_1ae9af16f1fed89e4afc3df054398b0f98)`(double & amin,double & amid,double & amax)` | 

## Members

#### `public inline  `[`Interpolator3pt`](#class_interpolator3pt_1aabff59935bf573aaa4c91ac52b498ad1)`(double lo,double mi,double hi,double v1,double vm,double v2)` {#class_interpolator3pt_1aabff59935bf573aaa4c91ac52b498ad1}

#### `public inline double `[`operator()`](#class_interpolator3pt_1a18f6068ae0785cecfbfb2b38d061ee06)`(double x)` {#class_interpolator3pt_1a18f6068ae0785cecfbfb2b38d061ee06}

#### `public inline void `[`getMappingValues`](#class_interpolator3pt_1ae9af16f1fed89e4afc3df054398b0f98)`(double & amin,double & amid,double & amax)` {#class_interpolator3pt_1ae9af16f1fed89e4afc3df054398b0f98}

# class `JackAudioInterface` {#class_jack_audio_interface}

```
class JackAudioInterface
  : public AudioInterface
```  

Class that provides an interface with the Jack Audio Server.

> Todo: implement srate_callback 

> Todo: automatically starts jack with buffer and sample rate settings specified by the user

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  `[`JackAudioInterface`](#class_jack_audio_interface_1a00d156b34cdac0825012b6cde2d3cdaa)`(`[`JackTrip`](#class_jack_trip)` * jacktrip,int NumInChans,int NumOutChans,`[`AudioInterface::audioBitResolutionT`](#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b)` AudioBitResolution,QString ClientName)` | The class constructor.
`public virtual  `[`~JackAudioInterface`](#class_jack_audio_interface_1a90073f74b8642080a88f2bbeb5f78b8d)`()` | The class destructor.
`public virtual void `[`setup`](#class_jack_audio_interface_1a86f2df3ab05bb9cc5b641d8716166046)`()` | Setup the client.
`public virtual int `[`startProcess`](#class_jack_audio_interface_1a9d5b2043029f6b295a2ab67c2a3e07d8)`() const` | Tell the JACK server that we are ready to roll. The process-callback will start running. This runs on its own thread.
`public virtual int `[`stopProcess`](#class_jack_audio_interface_1af8c5861e7678caa0c4b1b5147711b80c)`() const` | Stops the process-callback thread.
`public virtual void `[`connectDefaultPorts`](#class_jack_audio_interface_1a0a389bfb2a047098063691008093e7a2)`()` | Connect the default ports, capture to sends, and receives to playback.
`public inline virtual void `[`setClientName`](#class_jack_audio_interface_1afc2a986630338b3a6fdf1bcf23033c56)`(QString ClientName)` | Set Client Name to something different that the default ([JackTrip](#class_jack_trip))
`public inline virtual void `[`setSampleRate`](#class_jack_audio_interface_1ab189e0e877335a9985532a2628a4ed4f)`(`[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)`)` | 
`public inline virtual void `[`setBufferSizeInSamples`](#class_jack_audio_interface_1a03f9652ee8cff4dd681cb4f839414a14)`(`[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)`)` | 
`public inline virtual void `[`enableBroadcastOutput`](#class_jack_audio_interface_1ade8a2a001638d67747ed51523388d02e)`()` | 
`public virtual `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`getSampleRate`](#class_jack_audio_interface_1af745086680ee649ceef92b5a559e203a)`() const` | Get the Jack Server Sampling Rate, in samples/second.
`public virtual `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`getBufferSizeInSamples`](#class_jack_audio_interface_1a37bdc25c30b63e6401c0b11adfdc33cf)`() const` | Get the Jack Server Buffer Size, in samples.
`public inline virtual `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`getBufferSizeInBytes`](#class_jack_audio_interface_1a2073c313564cf0d307ff5820b3423dfe)`() const` | Get the Jack Server Buffer Size, in bytes.
`public virtual size_t `[`getSizeInBytesPerChannel`](#class_jack_audio_interface_1a53662ef7a0f09861f109b5d3339d0604)`() const` | Get size of each audio per channel, in bytes.

## Members

#### `public  `[`JackAudioInterface`](#class_jack_audio_interface_1a00d156b34cdac0825012b6cde2d3cdaa)`(`[`JackTrip`](#class_jack_trip)` * jacktrip,int NumInChans,int NumOutChans,`[`AudioInterface::audioBitResolutionT`](#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b)` AudioBitResolution,QString ClientName)` {#class_jack_audio_interface_1a00d156b34cdac0825012b6cde2d3cdaa}

The class constructor.

#### Parameters
* `jacktrip` Pointer to the [JackTrip](#class_jack_trip) class that connects all classes (mediator) 

* `NumInChans` Number of Input Channels 

* `NumOutChans` Number of Output Channels 

* `AudioBitResolution` Audio Sample Resolutions in bits 

* `ClientName` Client name in Jack

#### `public virtual  `[`~JackAudioInterface`](#class_jack_audio_interface_1a90073f74b8642080a88f2bbeb5f78b8d)`()` {#class_jack_audio_interface_1a90073f74b8642080a88f2bbeb5f78b8d}

The class destructor.

#### `public virtual void `[`setup`](#class_jack_audio_interface_1a86f2df3ab05bb9cc5b641d8716166046)`()` {#class_jack_audio_interface_1a86f2df3ab05bb9cc5b641d8716166046}

Setup the client.

#### `public virtual int `[`startProcess`](#class_jack_audio_interface_1a9d5b2043029f6b295a2ab67c2a3e07d8)`() const` {#class_jack_audio_interface_1a9d5b2043029f6b295a2ab67c2a3e07d8}

Tell the JACK server that we are ready to roll. The process-callback will start running. This runs on its own thread.

#### Returns
0 on success, otherwise a non-zero error code

#### `public virtual int `[`stopProcess`](#class_jack_audio_interface_1af8c5861e7678caa0c4b1b5147711b80c)`() const` {#class_jack_audio_interface_1af8c5861e7678caa0c4b1b5147711b80c}

Stops the process-callback thread.

#### Returns
0 on success, otherwise a non-zero error code

#### `public virtual void `[`connectDefaultPorts`](#class_jack_audio_interface_1a0a389bfb2a047098063691008093e7a2)`()` {#class_jack_audio_interface_1a0a389bfb2a047098063691008093e7a2}

Connect the default ports, capture to sends, and receives to playback.

#### `public inline virtual void `[`setClientName`](#class_jack_audio_interface_1afc2a986630338b3a6fdf1bcf23033c56)`(QString ClientName)` {#class_jack_audio_interface_1afc2a986630338b3a6fdf1bcf23033c56}

Set Client Name to something different that the default ([JackTrip](#class_jack_trip))

#### `public inline virtual void `[`setSampleRate`](#class_jack_audio_interface_1ab189e0e877335a9985532a2628a4ed4f)`(`[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)`)` {#class_jack_audio_interface_1ab189e0e877335a9985532a2628a4ed4f}

#### `public inline virtual void `[`setBufferSizeInSamples`](#class_jack_audio_interface_1a03f9652ee8cff4dd681cb4f839414a14)`(`[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)`)` {#class_jack_audio_interface_1a03f9652ee8cff4dd681cb4f839414a14}

#### `public inline virtual void `[`enableBroadcastOutput`](#class_jack_audio_interface_1ade8a2a001638d67747ed51523388d02e)`()` {#class_jack_audio_interface_1ade8a2a001638d67747ed51523388d02e}

#### `public virtual `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`getSampleRate`](#class_jack_audio_interface_1af745086680ee649ceef92b5a559e203a)`() const` {#class_jack_audio_interface_1af745086680ee649ceef92b5a559e203a}

Get the Jack Server Sampling Rate, in samples/second.

#### `public virtual `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`getBufferSizeInSamples`](#class_jack_audio_interface_1a37bdc25c30b63e6401c0b11adfdc33cf)`() const` {#class_jack_audio_interface_1a37bdc25c30b63e6401c0b11adfdc33cf}

Get the Jack Server Buffer Size, in samples.

#### `public inline virtual `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`getBufferSizeInBytes`](#class_jack_audio_interface_1a2073c313564cf0d307ff5820b3423dfe)`() const` {#class_jack_audio_interface_1a2073c313564cf0d307ff5820b3423dfe}

Get the Jack Server Buffer Size, in bytes.

#### `public virtual size_t `[`getSizeInBytesPerChannel`](#class_jack_audio_interface_1a53662ef7a0f09861f109b5d3339d0604)`() const` {#class_jack_audio_interface_1a53662ef7a0f09861f109b5d3339d0604}

Get size of each audio per channel, in bytes.

# class `JackTrip` {#class_jack_trip}

```
class JackTrip
  : public QObject
```  

Main class to creates a SERVER (to listen) or a CLIENT (to connect to a listening server) to send audio streams in the network.

All audio and network settings can be set in this class. This class also acts as a Mediator between all the other class. Classes that uses [JackTrip](#class_jack_trip) methods need to register with it.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  `[`JackTrip`](#class_jack_trip_1a9ce0d5c38eab1235f362c8eabc15aaba)`(`[`jacktripModeT`](#class_jack_trip_1a062df7ced6d5d7f3dd47b5d05c393982)` JacktripMode,`[`dataProtocolT`](#class_jack_trip_1aba7784e922095de89025a2f6fc14559d)` DataProtocolType,int NumChans,int BufferQueueLength,unsigned int redundancy,`[`AudioInterface::audioBitResolutionT`](#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b)` AudioBitResolution,`[`DataProtocol::packetHeaderTypeT`](#class_data_protocol_1aed102fbb7239cbf862e6296eefe79492)` PacketHeaderType,`[`underrunModeT`](#class_jack_trip_1a9fd34b5de9751d9bafbb03a7c2250bf5)` UnderRunMode,int receiver_bind_port,int sender_bind_port,int receiver_peer_port,int sender_peer_port,int tcp_peer_port)` | The class Constructor with Default Parameters.
`public virtual  `[`~JackTrip`](#class_jack_trip_1aa30cb3e781cfb52195f42e5e81fe1d83)`()` | The class destructor.
`public virtual void `[`setPeerAddress`](#class_jack_trip_1a3555920e8f72a77dea6d3d79265e98b5)`(QString PeerHostOrIP)` | Starting point for the thread.
`public virtual void `[`appendProcessPluginToNetwork`](#class_jack_trip_1a39bdc701636e81672bab041dbdf5c8ba)`(`[`ProcessPlugin`](#class_process_plugin)` * plugin)` | Append a process plugin. Processes will be appended in order.
`public virtual void `[`appendProcessPluginFromNetwork`](#class_jack_trip_1a6a79e2341a43c83ed1be2f0cc73d4806)`(`[`ProcessPlugin`](#class_process_plugin)` * plugin)` | 
`public virtual void `[`startProcess`](#class_jack_trip_1a87f6cce2241917cb9dae804e9f0b416d)`()` | Start the processing threads.
`public virtual void `[`completeConnection`](#class_jack_trip_1a9b5421fa4402ff229f60a929f8d4f390)`()` | 
`public virtual void `[`stop`](#class_jack_trip_1a5a1c9a46327d7eca0ea8763cf7413b9e)`(QString errorMessage)` | Stop the processing threads.
`public virtual void `[`waitThreads`](#class_jack_trip_1a9f14f7a651eeb71a35d829d66187cbce)`()` | Wait for all the threads to finish. This functions is used when [JackTrip](#class_jack_trip) is run as a thread.
`public virtual void `[`checkIfPortIsBinded`](#class_jack_trip_1a224dcbabba45ec353c4ed30714dd83fc)`(int port)` | Check if UDP port is already binded.
`enum `[`dataProtocolT`](#class_jack_trip_1aba7784e922095de89025a2f6fc14559d) | Enum for the data Protocol. At this time only UDP is implemented.
`enum `[`jacktripModeT`](#class_jack_trip_1a062df7ced6d5d7f3dd47b5d05c393982) | Enum for the [JackTrip](#class_jack_trip) mode.
`enum `[`underrunModeT`](#class_jack_trip_1a9fd34b5de9751d9bafbb03a7c2250bf5) | Enum for the [JackTrip](#class_jack_trip) Underrun Mode, when packets.
`enum `[`audiointerfaceModeT`](#class_jack_trip_1abd27f2208794a3cfea14d41afd2bad28) | Enum for Audio Interface Mode.
`enum `[`connectionModeT`](#class_jack_trip_1a4d337935031bc676863ebbea07c169ab) | Enum for Connection Mode (in packet header)
`enum `[`hubConnectionModeT`](#class_jack_trip_1a2c6fae8a9d31dea1605b53ac08ce9ed0) | Enum for Hub Server Audio Connection Mode (connections to hub server are automatically patched in Jack)

## Members

#### `public  `[`JackTrip`](#class_jack_trip_1a9ce0d5c38eab1235f362c8eabc15aaba)`(`[`jacktripModeT`](#class_jack_trip_1a062df7ced6d5d7f3dd47b5d05c393982)` JacktripMode,`[`dataProtocolT`](#class_jack_trip_1aba7784e922095de89025a2f6fc14559d)` DataProtocolType,int NumChans,int BufferQueueLength,unsigned int redundancy,`[`AudioInterface::audioBitResolutionT`](#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b)` AudioBitResolution,`[`DataProtocol::packetHeaderTypeT`](#class_data_protocol_1aed102fbb7239cbf862e6296eefe79492)` PacketHeaderType,`[`underrunModeT`](#class_jack_trip_1a9fd34b5de9751d9bafbb03a7c2250bf5)` UnderRunMode,int receiver_bind_port,int sender_bind_port,int receiver_peer_port,int sender_peer_port,int tcp_peer_port)` {#class_jack_trip_1a9ce0d5c38eab1235f362c8eabc15aaba}

The class Constructor with Default Parameters.

#### Parameters
* `JacktripMode` [JackTrip::CLIENT](#class_jack_trip_1a062df7ced6d5d7f3dd47b5d05c393982adfe81f0a6336dc01d4f9db1c3acba59a) or [JackTrip::SERVER](#class_jack_trip_1a062df7ced6d5d7f3dd47b5d05c393982a55b561bf36053294b717fbf49b71d23c)

* `DataProtocolType` [JackTrip::dataProtocolT](#class_jack_trip_1aba7784e922095de89025a2f6fc14559d)

* `NumChans` Number of Audio Channels (same for inputs and outputs) 

* `BufferQueueLength` Audio Buffer for receiving packets 

* `AudioBitResolution` Audio Sample Resolutions in bits 

* `redundancy` redundancy factor for network data

#### `public virtual  `[`~JackTrip`](#class_jack_trip_1aa30cb3e781cfb52195f42e5e81fe1d83)`()` {#class_jack_trip_1aa30cb3e781cfb52195f42e5e81fe1d83}

The class destructor.

#### `public virtual void `[`setPeerAddress`](#class_jack_trip_1a3555920e8f72a77dea6d3d79265e98b5)`(QString PeerHostOrIP)` {#class_jack_trip_1a3555920e8f72a77dea6d3d79265e98b5}

Starting point for the thread.

Set the Peer Address for jacktripModeT::CLIENT mode only

#### `public virtual void `[`appendProcessPluginToNetwork`](#class_jack_trip_1a39bdc701636e81672bab041dbdf5c8ba)`(`[`ProcessPlugin`](#class_process_plugin)` * plugin)` {#class_jack_trip_1a39bdc701636e81672bab041dbdf5c8ba}

Append a process plugin. Processes will be appended in order.

#### Parameters
* `plugin` Pointer to [ProcessPlugin](#class_process_plugin) Class

#### `public virtual void `[`appendProcessPluginFromNetwork`](#class_jack_trip_1a6a79e2341a43c83ed1be2f0cc73d4806)`(`[`ProcessPlugin`](#class_process_plugin)` * plugin)` {#class_jack_trip_1a6a79e2341a43c83ed1be2f0cc73d4806}

#### `public virtual void `[`startProcess`](#class_jack_trip_1a87f6cce2241917cb9dae804e9f0b416d)`()` {#class_jack_trip_1a87f6cce2241917cb9dae804e9f0b416d}

Start the processing threads.

#### `public virtual void `[`completeConnection`](#class_jack_trip_1a9b5421fa4402ff229f60a929f8d4f390)`()` {#class_jack_trip_1a9b5421fa4402ff229f60a929f8d4f390}

#### `public virtual void `[`stop`](#class_jack_trip_1a5a1c9a46327d7eca0ea8763cf7413b9e)`(QString errorMessage)` {#class_jack_trip_1a5a1c9a46327d7eca0ea8763cf7413b9e}

Stop the processing threads.

#### `public virtual void `[`waitThreads`](#class_jack_trip_1a9f14f7a651eeb71a35d829d66187cbce)`()` {#class_jack_trip_1a9f14f7a651eeb71a35d829d66187cbce}

Wait for all the threads to finish. This functions is used when [JackTrip](#class_jack_trip) is run as a thread.

#### `public virtual void `[`checkIfPortIsBinded`](#class_jack_trip_1a224dcbabba45ec353c4ed30714dd83fc)`(int port)` {#class_jack_trip_1a224dcbabba45ec353c4ed30714dd83fc}

Check if UDP port is already binded.

#### Parameters
* `port` Port number

#### `enum `[`dataProtocolT`](#class_jack_trip_1aba7784e922095de89025a2f6fc14559d) {#class_jack_trip_1aba7784e922095de89025a2f6fc14559d}

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
UDP            | Use UDP (User Datagram Protocol)
TCP            | **NOT IMPLEMENTED**: Use TCP (Transmission Control Protocol)
SCTP            | **NOT IMPLEMENTED**: Use SCTP (Stream Control Transmission Protocol)

Enum for the data Protocol. At this time only UDP is implemented.

#### `enum `[`jacktripModeT`](#class_jack_trip_1a062df7ced6d5d7f3dd47b5d05c393982) {#class_jack_trip_1a062df7ced6d5d7f3dd47b5d05c393982}

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
SERVER            | Run in P2P Server Mode.
CLIENT            | Run in P2P Client Mode.
CLIENTTOPINGSERVER            | Client of the Ping Server Mode.
SERVERPINGSERVER            | Server of the MultiThreaded [JackTrip](#class_jack_trip).

Enum for the [JackTrip](#class_jack_trip) mode.

#### `enum `[`underrunModeT`](#class_jack_trip_1a9fd34b5de9751d9bafbb03a7c2250bf5) {#class_jack_trip_1a9fd34b5de9751d9bafbb03a7c2250bf5}

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
WAVETABLE            | Loops on the last received packet.
ZEROS            | Set new buffers to zero if there are no new ones.

Enum for the [JackTrip](#class_jack_trip) Underrun Mode, when packets.

#### `enum `[`audiointerfaceModeT`](#class_jack_trip_1abd27f2208794a3cfea14d41afd2bad28) {#class_jack_trip_1abd27f2208794a3cfea14d41afd2bad28}

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
JACK            | Jack Mode.
RTAUDIO            | RtAudio Mode.

Enum for Audio Interface Mode.

#### `enum `[`connectionModeT`](#class_jack_trip_1a4d337935031bc676863ebbea07c169ab) {#class_jack_trip_1a4d337935031bc676863ebbea07c169ab}

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
NORMAL            | Normal Mode.
KSTRONG            | Karplus Strong.
JAMTEST            | Karplus Strong.

Enum for Connection Mode (in packet header)

#### `enum `[`hubConnectionModeT`](#class_jack_trip_1a2c6fae8a9d31dea1605b53ac08ce9ed0) {#class_jack_trip_1a2c6fae8a9d31dea1605b53ac08ce9ed0}

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
SERVERTOCLIENT            | Normal Mode, Sever to All Clients (but not client to any client)
CLIENTECHO            | Client Echo (client self-to-self)
CLIENTFOFI            | Client Fan Out to Clients and Fan In from Clients (but not self-to-self)
RESERVEDMATRIX            | Reserved for custom patch matrix (for TUB ensemble)
FULLMIX            | Client Fan Out to Clients and Fan In from Clients (including self-to-self)
NOAUTO            | No automatic patching.

Enum for Hub Server Audio Connection Mode (connections to hub server are automatically patched in Jack)

# class `JackTripThread` {#class_jack_trip_thread}

```
class JackTripThread
  : public QThread
```  

Test class that runs [JackTrip](#class_jack_trip) inside a thread.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`JackTripThread`](#class_jack_trip_thread_1a82b09ae2f04a5a4df14b1d754df38eaa)`(`[`JackTrip::jacktripModeT`](#class_jack_trip_1a062df7ced6d5d7f3dd47b5d05c393982)` JacktripMode)` | 
`public inline virtual  `[`~JackTripThread`](#class_jack_trip_thread_1a4a192e66061357c489d0ed9316aa3264)`()` | 
`public void `[`run`](#class_jack_trip_thread_1aef1f596c146bcbb1d4bf078432c0046a)`()` | 
`public inline void `[`setPort`](#class_jack_trip_thread_1a689dc66090018cefa3666797dc5ae9a1)`(int port_num)` | 
`public inline void `[`setPeerAddress`](#class_jack_trip_thread_1a1aab9303a522ae5b60557a06cc02b691)`(const char * PeerHostOrIP)` | 

## Members

#### `public inline  `[`JackTripThread`](#class_jack_trip_thread_1a82b09ae2f04a5a4df14b1d754df38eaa)`(`[`JackTrip::jacktripModeT`](#class_jack_trip_1a062df7ced6d5d7f3dd47b5d05c393982)` JacktripMode)` {#class_jack_trip_thread_1a82b09ae2f04a5a4df14b1d754df38eaa}

#### `public inline virtual  `[`~JackTripThread`](#class_jack_trip_thread_1a4a192e66061357c489d0ed9316aa3264)`()` {#class_jack_trip_thread_1a4a192e66061357c489d0ed9316aa3264}

#### `public void `[`run`](#class_jack_trip_thread_1aef1f596c146bcbb1d4bf078432c0046a)`()` {#class_jack_trip_thread_1aef1f596c146bcbb1d4bf078432c0046a}

#### `public inline void `[`setPort`](#class_jack_trip_thread_1a689dc66090018cefa3666797dc5ae9a1)`(int port_num)` {#class_jack_trip_thread_1a689dc66090018cefa3666797dc5ae9a1}

#### `public inline void `[`setPeerAddress`](#class_jack_trip_thread_1a1aab9303a522ae5b60557a06cc02b691)`(const char * PeerHostOrIP)` {#class_jack_trip_thread_1a1aab9303a522ae5b60557a06cc02b691}

# class `JackTripWorker` {#class_jack_trip_worker}

```
class JackTripWorker
  : public QObject
  : public QRunnable
```  

Prototype of the worker class that will be cloned through sending threads to the Thread Pool.

This class can be send to the ThreadPool using the start() method. Each time it is sent, it'll became "independent" of the prototype, which means that the prototype state can be changed, and used to send and start another thread into the pool. setAutoDelete must be set to false in order for this to work.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public bool `[`mAppendThreadID`](#class_jack_trip_worker_1aa817be509000f3fefdfb24cbc73c06a2) | 
`public  `[`JackTripWorker`](#class_jack_trip_worker_1a05a68ebb2cfe3b62119634bc99df9e3a)`(`[`UdpHubListener`](#class_udp_hub_listener)` * udphublistener,int BufferQueueLength,`[`JackTrip::underrunModeT`](#class_jack_trip_1a9fd34b5de9751d9bafbb03a7c2250bf5)` UnderRunMode,QString clientName)` | The class constructor.
`public virtual  `[`~JackTripWorker`](#class_jack_trip_worker_1a63f9e3139958af558be7d2aa7b44375d)`()` | The class destructor.
`public void `[`run`](#class_jack_trip_worker_1aa0905f8624d47299941ab07e3250abc6)`()` | Implements the Thread Loop. To start the thread, call start() ( DO NOT CALL [run()](#class_jack_trip_worker_1aa0905f8624d47299941ab07e3250abc6) ).
`public bool `[`isSpawning`](#class_jack_trip_worker_1a59e45e7c4dcd52a36f194f37c6fcd4c4)`()` | Check if the Thread is Spawning.
`public void `[`setJackTrip`](#class_jack_trip_worker_1adfab864b6ea0d219cce6243105b3cc1e)`(int id,QString client_address,`[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` server_port,`[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` client_port,int num_channels,bool connectDefaultAudioPorts)` | Sets the [JackTripWorker](#class_jack_trip_worker) properties.
`public void `[`stopThread`](#class_jack_trip_worker_1abc04f4bd01383d6e7c09360629445f8d)`()` | Stop and remove thread from pool.
`public inline int `[`getID`](#class_jack_trip_worker_1a0f7cbe18148fb8ffc83b32747f5316a7)`()` | 
`public inline void `[`setBufferStrategy`](#class_jack_trip_worker_1a144e002ce3a4861759117d4f1ff820a3)`(int BufferStrategy)` | 
`public inline void `[`setNetIssuesSimulation`](#class_jack_trip_worker_1a64fbf19fa8880a8015f0f2a7939c5feb)`(double loss,double jitter,double delay_rel)` | 
`public inline void `[`setBroadcast`](#class_jack_trip_worker_1aaf37f3764c0043a63bdd2a029dba4d20)`(int broadcast_queue)` | 
`public inline void `[`setUseRtUdpPriority`](#class_jack_trip_worker_1a660c8d99b9c41e87337a28ab7d0dbb80)`(bool use)` | 
`public inline void `[`setIOStatTimeout`](#class_jack_trip_worker_1a3f44e06a40b35e8fe1c888eb44fe1e26)`(int timeout)` | 
`public inline void `[`setIOStatStream`](#class_jack_trip_worker_1a638580c1f29ffad2af54abd6fc563eb4)`(QSharedPointer< std::ofstream > statStream)` | 
`{signal} public void `[`signalRemoveThread`](#class_jack_trip_worker_1a9acc5749ea776ab652d63e70e94dfad3)`()` | 

## Members

#### `public bool `[`mAppendThreadID`](#class_jack_trip_worker_1aa817be509000f3fefdfb24cbc73c06a2) {#class_jack_trip_worker_1aa817be509000f3fefdfb24cbc73c06a2}

#### `public  `[`JackTripWorker`](#class_jack_trip_worker_1a05a68ebb2cfe3b62119634bc99df9e3a)`(`[`UdpHubListener`](#class_udp_hub_listener)` * udphublistener,int BufferQueueLength,`[`JackTrip::underrunModeT`](#class_jack_trip_1a9fd34b5de9751d9bafbb03a7c2250bf5)` UnderRunMode,QString clientName)` {#class_jack_trip_worker_1a05a68ebb2cfe3b62119634bc99df9e3a}

The class constructor.

#### `public virtual  `[`~JackTripWorker`](#class_jack_trip_worker_1a63f9e3139958af558be7d2aa7b44375d)`()` {#class_jack_trip_worker_1a63f9e3139958af558be7d2aa7b44375d}

The class destructor.

#### `public void `[`run`](#class_jack_trip_worker_1aa0905f8624d47299941ab07e3250abc6)`()` {#class_jack_trip_worker_1aa0905f8624d47299941ab07e3250abc6}

Implements the Thread Loop. To start the thread, call start() ( DO NOT CALL [run()](#class_jack_trip_worker_1aa0905f8624d47299941ab07e3250abc6) ).

#### `public bool `[`isSpawning`](#class_jack_trip_worker_1a59e45e7c4dcd52a36f194f37c6fcd4c4)`()` {#class_jack_trip_worker_1a59e45e7c4dcd52a36f194f37c6fcd4c4}

Check if the Thread is Spawning.

#### Returns
true is it is spawning, false if it's already running

#### `public void `[`setJackTrip`](#class_jack_trip_worker_1adfab864b6ea0d219cce6243105b3cc1e)`(int id,QString client_address,`[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` server_port,`[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` client_port,int num_channels,bool connectDefaultAudioPorts)` {#class_jack_trip_worker_1adfab864b6ea0d219cce6243105b3cc1e}

Sets the [JackTripWorker](#class_jack_trip_worker) properties.

#### Parameters
* `id` ID number 

* `address`

#### `public void `[`stopThread`](#class_jack_trip_worker_1abc04f4bd01383d6e7c09360629445f8d)`()` {#class_jack_trip_worker_1abc04f4bd01383d6e7c09360629445f8d}

Stop and remove thread from pool.

#### `public inline int `[`getID`](#class_jack_trip_worker_1a0f7cbe18148fb8ffc83b32747f5316a7)`()` {#class_jack_trip_worker_1a0f7cbe18148fb8ffc83b32747f5316a7}

#### `public inline void `[`setBufferStrategy`](#class_jack_trip_worker_1a144e002ce3a4861759117d4f1ff820a3)`(int BufferStrategy)` {#class_jack_trip_worker_1a144e002ce3a4861759117d4f1ff820a3}

#### `public inline void `[`setNetIssuesSimulation`](#class_jack_trip_worker_1a64fbf19fa8880a8015f0f2a7939c5feb)`(double loss,double jitter,double delay_rel)` {#class_jack_trip_worker_1a64fbf19fa8880a8015f0f2a7939c5feb}

#### `public inline void `[`setBroadcast`](#class_jack_trip_worker_1aaf37f3764c0043a63bdd2a029dba4d20)`(int broadcast_queue)` {#class_jack_trip_worker_1aaf37f3764c0043a63bdd2a029dba4d20}

#### `public inline void `[`setUseRtUdpPriority`](#class_jack_trip_worker_1a660c8d99b9c41e87337a28ab7d0dbb80)`(bool use)` {#class_jack_trip_worker_1a660c8d99b9c41e87337a28ab7d0dbb80}

#### `public inline void `[`setIOStatTimeout`](#class_jack_trip_worker_1a3f44e06a40b35e8fe1c888eb44fe1e26)`(int timeout)` {#class_jack_trip_worker_1a3f44e06a40b35e8fe1c888eb44fe1e26}

#### `public inline void `[`setIOStatStream`](#class_jack_trip_worker_1a638580c1f29ffad2af54abd6fc563eb4)`(QSharedPointer< std::ofstream > statStream)` {#class_jack_trip_worker_1a638580c1f29ffad2af54abd6fc563eb4}

#### `{signal} public void `[`signalRemoveThread`](#class_jack_trip_worker_1a9acc5749ea776ab652d63e70e94dfad3)`()` {#class_jack_trip_worker_1a9acc5749ea776ab652d63e70e94dfad3}

# class `JackTripWorkerMessages` {#class_jack_trip_worker_messages}

```
class JackTripWorkerMessages
  : public QObject
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`JackTripWorkerMessages`](#class_jack_trip_worker_messages_1a691d6b469b0b7e08238ba2c556f4f665)`()` | 
`public inline virtual  `[`~JackTripWorkerMessages`](#class_jack_trip_worker_messages_1a15e7841d383f226e4d739bed3f353060)`()` | 
`public inline void `[`play`](#class_jack_trip_worker_messages_1a53868d2c0059cfbeb5cca9169c594f88)`()` | 
`{signal} public void `[`signalTest`](#class_jack_trip_worker_messages_1a46aae282c6b1d0c41e7c2bca96c3d82d)`()` | 
`{signal} public void `[`signalStopEventLoop`](#class_jack_trip_worker_messages_1a304f126c9a03f9e8919f355e38dee17e)`()` | Signal to stop the event loop inside the [JackTripWorker](#class_jack_trip_worker) Thread.
`{slot} public inline void `[`slotTest`](#class_jack_trip_worker_messages_1ae7e3f4277e090946e62443eb52973b1c)`()` | 

## Members

#### `public inline  `[`JackTripWorkerMessages`](#class_jack_trip_worker_messages_1a691d6b469b0b7e08238ba2c556f4f665)`()` {#class_jack_trip_worker_messages_1a691d6b469b0b7e08238ba2c556f4f665}

#### `public inline virtual  `[`~JackTripWorkerMessages`](#class_jack_trip_worker_messages_1a15e7841d383f226e4d739bed3f353060)`()` {#class_jack_trip_worker_messages_1a15e7841d383f226e4d739bed3f353060}

#### `public inline void `[`play`](#class_jack_trip_worker_messages_1a53868d2c0059cfbeb5cca9169c594f88)`()` {#class_jack_trip_worker_messages_1a53868d2c0059cfbeb5cca9169c594f88}

#### `{signal} public void `[`signalTest`](#class_jack_trip_worker_messages_1a46aae282c6b1d0c41e7c2bca96c3d82d)`()` {#class_jack_trip_worker_messages_1a46aae282c6b1d0c41e7c2bca96c3d82d}

#### `{signal} public void `[`signalStopEventLoop`](#class_jack_trip_worker_messages_1a304f126c9a03f9e8919f355e38dee17e)`()` {#class_jack_trip_worker_messages_1a304f126c9a03f9e8919f355e38dee17e}

Signal to stop the event loop inside the [JackTripWorker](#class_jack_trip_worker) Thread.

#### `{slot} public inline void `[`slotTest`](#class_jack_trip_worker_messages_1ae7e3f4277e090946e62443eb52973b1c)`()` {#class_jack_trip_worker_messages_1ae7e3f4277e090946e62443eb52973b1c}

# class `JamLinkHeader` {#class_jam_link_header}

```
class JamLinkHeader
  : public PacketHeader
```  

JamLink Header.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  `[`JamLinkHeader`](#class_jam_link_header_1a05cefbbc5896cbfa2570ed7f73f3095c)`(`[`JackTrip`](#class_jack_trip)` * jacktrip)` | 
`public inline virtual  `[`~JamLinkHeader`](#class_jam_link_header_1ad946ef3af4d39ba2f2b488f29e105dea)`()` | 
`public virtual void `[`fillHeaderCommonFromAudio`](#class_jam_link_header_1a1460ffed8c87d309b5f97946137fd2f9)`()` | > Todo: Implement this using a [JackTrip](#class_jack_trip) Method (Mediator) member instead of the reference to JackAudio
`public inline virtual void `[`parseHeader`](#class_jam_link_header_1acc9570d34f4fef928d0a738ce116981f)`()` | Parse the packet header and take appropriate measures (like change settings, or quit the program if peer settings don't match)
`public inline virtual void `[`checkPeerSettings`](#class_jam_link_header_1a118fc8ef2db06a1734a788253d29656b)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *)` | 
`public inline virtual uint64_t `[`getPeerTimeStamp`](#class_jam_link_header_1a8fc2326055c0a55efbd24bf336f9769e)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` | 
`public inline virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerSequenceNumber`](#class_jam_link_header_1aebf3e042ecb938fde3aeb2bbd43791ec)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` | 
`public inline virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerBufferSize`](#class_jam_link_header_1a058bf6f9aa2a793dc8a833a266bc255b)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` | 
`public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerSamplingRate`](#class_jam_link_header_1a7baf5f2b61bd5d63b777ffd75f0f4c0d)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` | 
`public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerBitResolution`](#class_jam_link_header_1a3668fef3cc988c3c73b2e554f92d0748)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` | 
`public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerNumChannels`](#class_jam_link_header_1a7c605681757cf0714661c120498d439a)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` | 
`public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerConnectionMode`](#class_jam_link_header_1aeb16a00bd1a7436e487b0bb4cdb65e2d)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` | 
`public inline virtual void `[`increaseSequenceNumber`](#class_jam_link_header_1a01e4efd2aa98316da0439b89ff5da0c2)`()` | Increase sequence number for counter, a 16bit number.
`public inline virtual int `[`getHeaderSizeInBytes`](#class_jam_link_header_1ad984d1e949b6c7fa4bc5cf5447a05fa0)`() const` | Get the header size in bytes.
`public inline virtual void `[`putHeaderInPacket`](#class_jam_link_header_1a76401a1a0d37e9755d7985bd6e699f89)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet)` | Put the header in buffer pointed by full_packet.

## Members

#### `public  `[`JamLinkHeader`](#class_jam_link_header_1a05cefbbc5896cbfa2570ed7f73f3095c)`(`[`JackTrip`](#class_jack_trip)` * jacktrip)` {#class_jam_link_header_1a05cefbbc5896cbfa2570ed7f73f3095c}

#### `public inline virtual  `[`~JamLinkHeader`](#class_jam_link_header_1ad946ef3af4d39ba2f2b488f29e105dea)`()` {#class_jam_link_header_1ad946ef3af4d39ba2f2b488f29e105dea}

#### `public virtual void `[`fillHeaderCommonFromAudio`](#class_jam_link_header_1a1460ffed8c87d309b5f97946137fd2f9)`()` {#class_jam_link_header_1a1460ffed8c87d309b5f97946137fd2f9}

> Todo: Implement this using a [JackTrip](#class_jack_trip) Method (Mediator) member instead of the reference to JackAudio

#### `public inline virtual void `[`parseHeader`](#class_jam_link_header_1acc9570d34f4fef928d0a738ce116981f)`()` {#class_jam_link_header_1acc9570d34f4fef928d0a738ce116981f}

Parse the packet header and take appropriate measures (like change settings, or quit the program if peer settings don't match)

#### `public inline virtual void `[`checkPeerSettings`](#class_jam_link_header_1a118fc8ef2db06a1734a788253d29656b)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *)` {#class_jam_link_header_1a118fc8ef2db06a1734a788253d29656b}

#### `public inline virtual uint64_t `[`getPeerTimeStamp`](#class_jam_link_header_1a8fc2326055c0a55efbd24bf336f9769e)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` {#class_jam_link_header_1a8fc2326055c0a55efbd24bf336f9769e}

#### `public inline virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerSequenceNumber`](#class_jam_link_header_1aebf3e042ecb938fde3aeb2bbd43791ec)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` {#class_jam_link_header_1aebf3e042ecb938fde3aeb2bbd43791ec}

#### `public inline virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerBufferSize`](#class_jam_link_header_1a058bf6f9aa2a793dc8a833a266bc255b)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` {#class_jam_link_header_1a058bf6f9aa2a793dc8a833a266bc255b}

#### `public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerSamplingRate`](#class_jam_link_header_1a7baf5f2b61bd5d63b777ffd75f0f4c0d)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` {#class_jam_link_header_1a7baf5f2b61bd5d63b777ffd75f0f4c0d}

#### `public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerBitResolution`](#class_jam_link_header_1a3668fef3cc988c3c73b2e554f92d0748)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` {#class_jam_link_header_1a3668fef3cc988c3c73b2e554f92d0748}

#### `public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerNumChannels`](#class_jam_link_header_1a7c605681757cf0714661c120498d439a)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` {#class_jam_link_header_1a7c605681757cf0714661c120498d439a}

#### `public inline virtual `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerConnectionMode`](#class_jam_link_header_1aeb16a00bd1a7436e487b0bb4cdb65e2d)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` *) const` {#class_jam_link_header_1aeb16a00bd1a7436e487b0bb4cdb65e2d}

#### `public inline virtual void `[`increaseSequenceNumber`](#class_jam_link_header_1a01e4efd2aa98316da0439b89ff5da0c2)`()` {#class_jam_link_header_1a01e4efd2aa98316da0439b89ff5da0c2}

Increase sequence number for counter, a 16bit number.

#### `public inline virtual int `[`getHeaderSizeInBytes`](#class_jam_link_header_1ad984d1e949b6c7fa4bc5cf5447a05fa0)`() const` {#class_jam_link_header_1ad984d1e949b6c7fa4bc5cf5447a05fa0}

Get the header size in bytes.

#### `public inline virtual void `[`putHeaderInPacket`](#class_jam_link_header_1a76401a1a0d37e9755d7985bd6e699f89)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet)` {#class_jam_link_header_1a76401a1a0d37e9755d7985bd6e699f89}

Put the header in buffer pointed by full_packet.

#### Parameters
* `full_packet` Pointer to full packet (audio+header). Size must be sizeof(header part) + sizeof(audio part)

# class `JitterBuffer` {#class_jitter_buffer}

```
class JitterBuffer
  : public RingBuffer
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  `[`JitterBuffer`](#class_jitter_buffer_1af938b36a7a96ef91ac4f70e57f624ccf)`(int buf_samples,int qlen,int sample_rate,int strategy,int bcast_qlen,int channels,int bit_res)` | 
`public inline virtual  `[`~JitterBuffer`](#class_jitter_buffer_1a98c5102e6ae7d6ed41693cff4197c6ed)`()` | 
`public virtual bool `[`insertSlotNonBlocking`](#class_jitter_buffer_1aaa2988c291da05963e2d0e3e11186b0a)`(const `[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToSlot,int len,int lostLen)` | Same as insertSlotBlocking but non-blocking (asynchronous)
`public virtual void `[`readSlotNonBlocking`](#class_jitter_buffer_1a782bace3f543be0e5659fe6dc50be4bc)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` | Same as readSlotBlocking but non-blocking (asynchronous)
`public virtual void `[`readBroadcastSlot`](#class_jitter_buffer_1ab132a04a1d122e04d3dbffd957f72583)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` | 
`public virtual bool `[`getStats`](#class_jitter_buffer_1a21e028e3dcf94fb9620f47a283aabf1c)`(`[`IOStat`](#struct_ring_buffer_1_1_i_o_stat)` * stat,bool reset)` | 
`protected int `[`mMaxLatency`](#class_jitter_buffer_1a8c00a7cb5f722e600ccc732daafe5d3a) | 
`protected int `[`mNumChannels`](#class_jitter_buffer_1ae0ee2485022ef3a55c5052cf9a9a65f6) | 
`protected int `[`mAudioBitRes`](#class_jitter_buffer_1a99201de1a975dbb9ff0440a9d580c5c4) | 
`protected int `[`mMinStepSize`](#class_jitter_buffer_1a2ae830cd17acefa92fc96dd626f7f5a9) | 
`protected int `[`mFPP`](#class_jitter_buffer_1ab2908f7ecfd38ac0e0c6c08b3a748ac9) | 
`protected int `[`mSampleRate`](#class_jitter_buffer_1a796063cfade8f77ad44f9241391e24a0) | 
`protected int `[`mInSlotSize`](#class_jitter_buffer_1a195fa1196aadc3b71d2898e57e237e28) | 
`protected bool `[`mActive`](#class_jitter_buffer_1ae463927c1e18b533273161152c21bb95) | 
`protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mBroadcastLatency`](#class_jitter_buffer_1aaa5315581a2631c3f019027a4ec03337) | 
`protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mBroadcastPosition`](#class_jitter_buffer_1ad738e91254e00a44736c249583cd4ed0) | 
`protected double `[`mBroadcastPositionCorr`](#class_jitter_buffer_1acbbd24359f4f5dee9ed67cce205c4c1d) | 
`protected double `[`mUnderrunIncTolerance`](#class_jitter_buffer_1a9c38d00d318c5728412733108168e85c) | 
`protected double `[`mCorrIncTolerance`](#class_jitter_buffer_1ab6cd3a244709a6b705a922fc332827ee) | 
`protected double `[`mOverflowDecTolerance`](#class_jitter_buffer_1a051ea23ffd5b13dd3528ee8acba22c9f) | 
`protected int `[`mOverflowDropStep`](#class_jitter_buffer_1ae1e0714984fa37c0da7ef9061ca48486) | 
`protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mLastCorrCounter`](#class_jitter_buffer_1a24baff465c197a23b73b058e2983de79) | 
`protected int `[`mLastCorrDirection`](#class_jitter_buffer_1a0de7a53543dee89ca90066e80cc62f7f) | 
`protected double `[`mMinLevelThreshold`](#class_jitter_buffer_1a088e303aa49a1d4bc488103a85fc8d64) | 
`protected int `[`mAutoQueue`](#class_jitter_buffer_1a341a4f873b636d0cecb5c5f9e40e952b) | 
`protected double `[`mAutoQueueCorr`](#class_jitter_buffer_1a95ba9ee77ee2580dec909a7371d08bbc) | 
`protected double `[`mAutoQFactor`](#class_jitter_buffer_1ab82ae06640f248379b3a00d1501f6654) | 
`protected double `[`mAutoQRate`](#class_jitter_buffer_1a35d7211869f5067a1cd64554f2b4977d) | 
`protected double `[`mAutoQRateMin`](#class_jitter_buffer_1aa38287b1340b0a6883a5411fe8d92f23) | 
`protected double `[`mAutoQRateDecay`](#class_jitter_buffer_1a7b1822becbd4d0d72ed020b84648cfac) | 
`protected void `[`processPacketLoss`](#class_jitter_buffer_1a6f87039ae6131e54cb15c2fe4d0f77b7)`(int lostLen)` | 
`protected inline double `[`lastCorrFactor`](#class_jitter_buffer_1ada79d016499d1b2814a08d147bb46d81)`() const` | 

## Members

#### `public  `[`JitterBuffer`](#class_jitter_buffer_1af938b36a7a96ef91ac4f70e57f624ccf)`(int buf_samples,int qlen,int sample_rate,int strategy,int bcast_qlen,int channels,int bit_res)` {#class_jitter_buffer_1af938b36a7a96ef91ac4f70e57f624ccf}

#### `public inline virtual  `[`~JitterBuffer`](#class_jitter_buffer_1a98c5102e6ae7d6ed41693cff4197c6ed)`()` {#class_jitter_buffer_1a98c5102e6ae7d6ed41693cff4197c6ed}

#### `public virtual bool `[`insertSlotNonBlocking`](#class_jitter_buffer_1aaa2988c291da05963e2d0e3e11186b0a)`(const `[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToSlot,int len,int lostLen)` {#class_jitter_buffer_1aaa2988c291da05963e2d0e3e11186b0a}

Same as insertSlotBlocking but non-blocking (asynchronous)

#### Parameters
* `ptrToSlot` Pointer to slot to insert into the [RingBuffer](#class_ring_buffer)

#### `public virtual void `[`readSlotNonBlocking`](#class_jitter_buffer_1a782bace3f543be0e5659fe6dc50be4bc)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` {#class_jitter_buffer_1a782bace3f543be0e5659fe6dc50be4bc}

Same as readSlotBlocking but non-blocking (asynchronous)

#### Parameters
* `ptrToReadSlot` Pointer to read slot from the [RingBuffer](#class_ring_buffer)

#### `public virtual void `[`readBroadcastSlot`](#class_jitter_buffer_1ab132a04a1d122e04d3dbffd957f72583)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` {#class_jitter_buffer_1ab132a04a1d122e04d3dbffd957f72583}

#### `public virtual bool `[`getStats`](#class_jitter_buffer_1a21e028e3dcf94fb9620f47a283aabf1c)`(`[`IOStat`](#struct_ring_buffer_1_1_i_o_stat)` * stat,bool reset)` {#class_jitter_buffer_1a21e028e3dcf94fb9620f47a283aabf1c}

#### `protected int `[`mMaxLatency`](#class_jitter_buffer_1a8c00a7cb5f722e600ccc732daafe5d3a) {#class_jitter_buffer_1a8c00a7cb5f722e600ccc732daafe5d3a}

#### `protected int `[`mNumChannels`](#class_jitter_buffer_1ae0ee2485022ef3a55c5052cf9a9a65f6) {#class_jitter_buffer_1ae0ee2485022ef3a55c5052cf9a9a65f6}

#### `protected int `[`mAudioBitRes`](#class_jitter_buffer_1a99201de1a975dbb9ff0440a9d580c5c4) {#class_jitter_buffer_1a99201de1a975dbb9ff0440a9d580c5c4}

#### `protected int `[`mMinStepSize`](#class_jitter_buffer_1a2ae830cd17acefa92fc96dd626f7f5a9) {#class_jitter_buffer_1a2ae830cd17acefa92fc96dd626f7f5a9}

#### `protected int `[`mFPP`](#class_jitter_buffer_1ab2908f7ecfd38ac0e0c6c08b3a748ac9) {#class_jitter_buffer_1ab2908f7ecfd38ac0e0c6c08b3a748ac9}

#### `protected int `[`mSampleRate`](#class_jitter_buffer_1a796063cfade8f77ad44f9241391e24a0) {#class_jitter_buffer_1a796063cfade8f77ad44f9241391e24a0}

#### `protected int `[`mInSlotSize`](#class_jitter_buffer_1a195fa1196aadc3b71d2898e57e237e28) {#class_jitter_buffer_1a195fa1196aadc3b71d2898e57e237e28}

#### `protected bool `[`mActive`](#class_jitter_buffer_1ae463927c1e18b533273161152c21bb95) {#class_jitter_buffer_1ae463927c1e18b533273161152c21bb95}

#### `protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mBroadcastLatency`](#class_jitter_buffer_1aaa5315581a2631c3f019027a4ec03337) {#class_jitter_buffer_1aaa5315581a2631c3f019027a4ec03337}

#### `protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mBroadcastPosition`](#class_jitter_buffer_1ad738e91254e00a44736c249583cd4ed0) {#class_jitter_buffer_1ad738e91254e00a44736c249583cd4ed0}

#### `protected double `[`mBroadcastPositionCorr`](#class_jitter_buffer_1acbbd24359f4f5dee9ed67cce205c4c1d) {#class_jitter_buffer_1acbbd24359f4f5dee9ed67cce205c4c1d}

#### `protected double `[`mUnderrunIncTolerance`](#class_jitter_buffer_1a9c38d00d318c5728412733108168e85c) {#class_jitter_buffer_1a9c38d00d318c5728412733108168e85c}

#### `protected double `[`mCorrIncTolerance`](#class_jitter_buffer_1ab6cd3a244709a6b705a922fc332827ee) {#class_jitter_buffer_1ab6cd3a244709a6b705a922fc332827ee}

#### `protected double `[`mOverflowDecTolerance`](#class_jitter_buffer_1a051ea23ffd5b13dd3528ee8acba22c9f) {#class_jitter_buffer_1a051ea23ffd5b13dd3528ee8acba22c9f}

#### `protected int `[`mOverflowDropStep`](#class_jitter_buffer_1ae1e0714984fa37c0da7ef9061ca48486) {#class_jitter_buffer_1ae1e0714984fa37c0da7ef9061ca48486}

#### `protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mLastCorrCounter`](#class_jitter_buffer_1a24baff465c197a23b73b058e2983de79) {#class_jitter_buffer_1a24baff465c197a23b73b058e2983de79}

#### `protected int `[`mLastCorrDirection`](#class_jitter_buffer_1a0de7a53543dee89ca90066e80cc62f7f) {#class_jitter_buffer_1a0de7a53543dee89ca90066e80cc62f7f}

#### `protected double `[`mMinLevelThreshold`](#class_jitter_buffer_1a088e303aa49a1d4bc488103a85fc8d64) {#class_jitter_buffer_1a088e303aa49a1d4bc488103a85fc8d64}

#### `protected int `[`mAutoQueue`](#class_jitter_buffer_1a341a4f873b636d0cecb5c5f9e40e952b) {#class_jitter_buffer_1a341a4f873b636d0cecb5c5f9e40e952b}

#### `protected double `[`mAutoQueueCorr`](#class_jitter_buffer_1a95ba9ee77ee2580dec909a7371d08bbc) {#class_jitter_buffer_1a95ba9ee77ee2580dec909a7371d08bbc}

#### `protected double `[`mAutoQFactor`](#class_jitter_buffer_1ab82ae06640f248379b3a00d1501f6654) {#class_jitter_buffer_1ab82ae06640f248379b3a00d1501f6654}

#### `protected double `[`mAutoQRate`](#class_jitter_buffer_1a35d7211869f5067a1cd64554f2b4977d) {#class_jitter_buffer_1a35d7211869f5067a1cd64554f2b4977d}

#### `protected double `[`mAutoQRateMin`](#class_jitter_buffer_1aa38287b1340b0a6883a5411fe8d92f23) {#class_jitter_buffer_1aa38287b1340b0a6883a5411fe8d92f23}

#### `protected double `[`mAutoQRateDecay`](#class_jitter_buffer_1a7b1822becbd4d0d72ed020b84648cfac) {#class_jitter_buffer_1a7b1822becbd4d0d72ed020b84648cfac}

#### `protected void `[`processPacketLoss`](#class_jitter_buffer_1a6f87039ae6131e54cb15c2fe4d0f77b7)`(int lostLen)` {#class_jitter_buffer_1a6f87039ae6131e54cb15c2fe4d0f77b7}

#### `protected inline double `[`lastCorrFactor`](#class_jitter_buffer_1ada79d016499d1b2814a08d147bb46d81)`() const` {#class_jitter_buffer_1ada79d016499d1b2814a08d147bb46d81}

# class `JMess` {#class_j_mess}

Class to save and load all jack client connections.

Saves an XML file with all the current jack connections. This same file can be loaded to connect evrything again. The XML file can also be edited.

Has also an option to disconnect all the clients.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  `[`JMess`](#class_j_mess_1a36df8444ef5a2487c1893145f878b214)`()` | Constructs a [JMess](#class_j_mess) object that has a jack client.
`public virtual  `[`~JMess`](#class_j_mess_1a73229d9ef69285d5053b6177a663989f)`()` | Distructor closes the jmess jack audio client.
`public void `[`disconnectAll`](#class_j_mess_1a1d49a120634e44b7e09e295ca6d556d5)`()` | Disconnect all the clients.
`public void `[`writeOutput`](#class_j_mess_1a44d7176210eb8f4bd63b3c42a62045d8)`(QString xmlOutFile)` | Write an XML file with the name specified at xmlOutFile.
`public void `[`connectPorts`](#class_j_mess_1aa20e6ccab2deef71a93245f74d3031b9)`(QString xmlInFile)` | Connect ports specified in input XML file xmlInFile.
`public void `[`setConnectedPorts`](#class_j_mess_1ad1fc9a3fc4ee628a2acf6b3123f26512)`()` | Set list of ouput ports that have connections.
`public void `[`connectSpawnedPorts`](#class_j_mess_1a94f02a307f0f01ffd6c5e35fa0777123)`(int nChans,int hubPatch)` | Cross connect ports between net combs, -l LAIR mode.
`public void `[`connectTUB`](#class_j_mess_1a8c854f014fad349c238dd584ec25d761)`(int nChans)` | 

## Members

#### `public  `[`JMess`](#class_j_mess_1a36df8444ef5a2487c1893145f878b214)`()` {#class_j_mess_1a36df8444ef5a2487c1893145f878b214}

Constructs a [JMess](#class_j_mess) object that has a jack client.

#### `public virtual  `[`~JMess`](#class_j_mess_1a73229d9ef69285d5053b6177a663989f)`()` {#class_j_mess_1a73229d9ef69285d5053b6177a663989f}

Distructor closes the jmess jack audio client.

#### `public void `[`disconnectAll`](#class_j_mess_1a1d49a120634e44b7e09e295ca6d556d5)`()` {#class_j_mess_1a1d49a120634e44b7e09e295ca6d556d5}

Disconnect all the clients.

#### `public void `[`writeOutput`](#class_j_mess_1a44d7176210eb8f4bd63b3c42a62045d8)`(QString xmlOutFile)` {#class_j_mess_1a44d7176210eb8f4bd63b3c42a62045d8}

Write an XML file with the name specified at xmlOutFile.

#### `public void `[`connectPorts`](#class_j_mess_1aa20e6ccab2deef71a93245f74d3031b9)`(QString xmlInFile)` {#class_j_mess_1aa20e6ccab2deef71a93245f74d3031b9}

Connect ports specified in input XML file xmlInFile.

#### `public void `[`setConnectedPorts`](#class_j_mess_1ad1fc9a3fc4ee628a2acf6b3123f26512)`()` {#class_j_mess_1ad1fc9a3fc4ee628a2acf6b3123f26512}

Set list of ouput ports that have connections.

#### `public void `[`connectSpawnedPorts`](#class_j_mess_1a94f02a307f0f01ffd6c5e35fa0777123)`(int nChans,int hubPatch)` {#class_j_mess_1a94f02a307f0f01ffd6c5e35fa0777123}

Cross connect ports between net combs, -l LAIR mode.

#### `public void `[`connectTUB`](#class_j_mess_1a8c854f014fad349c238dd584ec25d761)`(int nChans)` {#class_j_mess_1a8c854f014fad349c238dd584ec25d761}

# class `Limiter` {#class_limiter}

```
class Limiter
  : public ProcessPlugin
```  

Applies limiter_lad_mono from the faustlibraries distribution, compressors.lib.

The [Limiter](#class_limiter) class confines the output dynamic range to a "dynamic range lane" determined by the assumed number of clients.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`Limiter`](#class_limiter_1a45369abfb5dd0c6faaedc69987cf398c)`(int numchans,int numclients,bool verboseFlag)` | The class constructor sets the number of channels to limit.
`public inline virtual  `[`~Limiter`](#class_limiter_1a98a1517d6ab45de67596e2a8f7f95244)`()` | The class destructor.
`public inline virtual void `[`init`](#class_limiter_1a2bc6bc28a1419863a901c9002be2eabc)`(int samplingRate)` | Do proper Initialization of members and class instances. By default this initializes the Sampling Frequency. If a class instance depends on the sampling frequency, it should be initialize here.
`public inline virtual int `[`getNumInputs`](#class_limiter_1affd7eb07526294b02fcde1b0b82fb50b)`()` | Return Number of Input Channels.
`public inline virtual int `[`getNumOutputs`](#class_limiter_1a3c73902b314a06f21ef3a01b03f89d0b)`()` | Return Number of Output Channels.
`public virtual void `[`compute`](#class_limiter_1a7ea48caa85017fed491fea97cd062e5f)`(int nframes,float ** inputs,float ** outputs)` | Compute process.
`public inline void `[`setWarningAmplitude`](#class_limiter_1a3ab64c711f8914edb44dbecbade2db4f)`(double wa)` | 

## Members

#### `public inline  `[`Limiter`](#class_limiter_1a45369abfb5dd0c6faaedc69987cf398c)`(int numchans,int numclients,bool verboseFlag)` {#class_limiter_1a45369abfb5dd0c6faaedc69987cf398c}

The class constructor sets the number of channels to limit.

#### `public inline virtual  `[`~Limiter`](#class_limiter_1a98a1517d6ab45de67596e2a8f7f95244)`()` {#class_limiter_1a98a1517d6ab45de67596e2a8f7f95244}

The class destructor.

#### `public inline virtual void `[`init`](#class_limiter_1a2bc6bc28a1419863a901c9002be2eabc)`(int samplingRate)` {#class_limiter_1a2bc6bc28a1419863a901c9002be2eabc}

Do proper Initialization of members and class instances. By default this initializes the Sampling Frequency. If a class instance depends on the sampling frequency, it should be initialize here.

#### `public inline virtual int `[`getNumInputs`](#class_limiter_1affd7eb07526294b02fcde1b0b82fb50b)`()` {#class_limiter_1affd7eb07526294b02fcde1b0b82fb50b}

Return Number of Input Channels.

#### `public inline virtual int `[`getNumOutputs`](#class_limiter_1a3c73902b314a06f21ef3a01b03f89d0b)`()` {#class_limiter_1a3c73902b314a06f21ef3a01b03f89d0b}

Return Number of Output Channels.

#### `public virtual void `[`compute`](#class_limiter_1a7ea48caa85017fed491fea97cd062e5f)`(int nframes,float ** inputs,float ** outputs)` {#class_limiter_1a7ea48caa85017fed491fea97cd062e5f}

Compute process.

#### `public inline void `[`setWarningAmplitude`](#class_limiter_1a3ab64c711f8914edb44dbecbade2db4f)`(double wa)` {#class_limiter_1a3ab64c711f8914edb44dbecbade2db4f}

# class `limiterdsp` {#classlimiterdsp}

```
class limiterdsp
  : public dsp
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline virtual void `[`metadata`](#classlimiterdsp_1a5406956d9734b157893eb5b9a0c71128)`(`[`Meta`](#struct_meta)` * m)` | Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.
`public inline virtual int `[`getNumInputs`](#classlimiterdsp_1a7e53e8377a6ddbc7a2816c74a81f21cd)`()` | 
`public inline virtual int `[`getNumOutputs`](#classlimiterdsp_1aa15601b68ad6c0786c20e5d8178d205c)`()` | 
`public inline virtual int `[`getInputRate`](#classlimiterdsp_1aa4259f26a386b63264018821a15399b3)`(int channel)` | 
`public inline virtual int `[`getOutputRate`](#classlimiterdsp_1a635d2be2a40ab936b7d65cdbe316f3e4)`(int channel)` | 
`public inline virtual void `[`instanceConstants`](#classlimiterdsp_1a4b4e267b9c22438be0bdc96e5fe6d0ba)`(int sample_rate)` | Init instance constant state
`public inline virtual void `[`instanceResetUserInterface`](#classlimiterdsp_1a3ab07a9a00a93ed1ae0b8ac9af4341ad)`()` | 
`public inline virtual void `[`instanceClear`](#classlimiterdsp_1ae877e43fcf49d34b152af4690c1f0935)`()` | 
`public inline virtual void `[`init`](#classlimiterdsp_1aaf7ec4acbbbcbb5a37c2bcb78ae18a30)`(int sample_rate)` | Global init, calls the following methods:
`public inline virtual void `[`instanceInit`](#classlimiterdsp_1a57f98c20d05f223ed3f7dce584f6c531)`(int sample_rate)` | Init instance state
`public inline virtual `[`limiterdsp`](#classlimiterdsp)` * `[`clone`](#classlimiterdsp_1a3d5346cac62c823e13aed30806fdf8ab)`()` | Return a clone of the instance.
`public inline virtual int `[`getSampleRate`](#classlimiterdsp_1a9698aa8176e4b6abc03b92904e9f8c6c)`()` | 
`public inline virtual void `[`buildUserInterface`](#classlimiterdsp_1a8ab244583d4562bb0f34a775460d815a)`(`[`UI`](#struct_u_i)` * ui_interface)` | Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).
`public inline virtual void `[`compute`](#classlimiterdsp_1a8898295c2f1c38cf95ab00bfce137267)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` | DSP instance computation, to be called with successive in/out audio buffers.

## Members

#### `public inline virtual void `[`metadata`](#classlimiterdsp_1a5406956d9734b157893eb5b9a0c71128)`(`[`Meta`](#struct_meta)` * m)` {#classlimiterdsp_1a5406956d9734b157893eb5b9a0c71128}

Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.

#### Parameters
* `m` - the Meta* meta user

#### `public inline virtual int `[`getNumInputs`](#classlimiterdsp_1a7e53e8377a6ddbc7a2816c74a81f21cd)`()` {#classlimiterdsp_1a7e53e8377a6ddbc7a2816c74a81f21cd}

#### `public inline virtual int `[`getNumOutputs`](#classlimiterdsp_1aa15601b68ad6c0786c20e5d8178d205c)`()` {#classlimiterdsp_1aa15601b68ad6c0786c20e5d8178d205c}

#### `public inline virtual int `[`getInputRate`](#classlimiterdsp_1aa4259f26a386b63264018821a15399b3)`(int channel)` {#classlimiterdsp_1aa4259f26a386b63264018821a15399b3}

#### `public inline virtual int `[`getOutputRate`](#classlimiterdsp_1a635d2be2a40ab936b7d65cdbe316f3e4)`(int channel)` {#classlimiterdsp_1a635d2be2a40ab936b7d65cdbe316f3e4}

#### `public inline virtual void `[`instanceConstants`](#classlimiterdsp_1a4b4e267b9c22438be0bdc96e5fe6d0ba)`(int sample_rate)` {#classlimiterdsp_1a4b4e267b9c22438be0bdc96e5fe6d0ba}

Init instance constant state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual void `[`instanceResetUserInterface`](#classlimiterdsp_1a3ab07a9a00a93ed1ae0b8ac9af4341ad)`()` {#classlimiterdsp_1a3ab07a9a00a93ed1ae0b8ac9af4341ad}

#### `public inline virtual void `[`instanceClear`](#classlimiterdsp_1ae877e43fcf49d34b152af4690c1f0935)`()` {#classlimiterdsp_1ae877e43fcf49d34b152af4690c1f0935}

#### `public inline virtual void `[`init`](#classlimiterdsp_1aaf7ec4acbbbcbb5a37c2bcb78ae18a30)`(int sample_rate)` {#classlimiterdsp_1aaf7ec4acbbbcbb5a37c2bcb78ae18a30}

Global init, calls the following methods:

* static class 'classInit': static tables initialization

* 'instanceInit': constants and instance state initialization

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual void `[`instanceInit`](#classlimiterdsp_1a57f98c20d05f223ed3f7dce584f6c531)`(int sample_rate)` {#classlimiterdsp_1a57f98c20d05f223ed3f7dce584f6c531}

Init instance state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual `[`limiterdsp`](#classlimiterdsp)` * `[`clone`](#classlimiterdsp_1a3d5346cac62c823e13aed30806fdf8ab)`()` {#classlimiterdsp_1a3d5346cac62c823e13aed30806fdf8ab}

Return a clone of the instance.

#### Returns
a copy of the instance on success, otherwise a null pointer.

#### `public inline virtual int `[`getSampleRate`](#classlimiterdsp_1a9698aa8176e4b6abc03b92904e9f8c6c)`()` {#classlimiterdsp_1a9698aa8176e4b6abc03b92904e9f8c6c}

#### `public inline virtual void `[`buildUserInterface`](#classlimiterdsp_1a8ab244583d4562bb0f34a775460d815a)`(`[`UI`](#struct_u_i)` * ui_interface)` {#classlimiterdsp_1a8ab244583d4562bb0f34a775460d815a}

Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).

#### Parameters
* `ui_interface` - the user interface builder

#### `public inline virtual void `[`compute`](#classlimiterdsp_1a8898295c2f1c38cf95ab00bfce137267)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` {#classlimiterdsp_1a8898295c2f1c38cf95ab00bfce137267}

DSP instance computation, to be called with successive in/out audio buffers.

#### Parameters
* `count` - the number of frames to compute 

* `inputs` - the input audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad) 

* `outputs` - the output audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad)

# class `LinearValueConverter` {#class_linear_value_converter}

```
class LinearValueConverter
  : public ValueConverter
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`LinearValueConverter`](#class_linear_value_converter_1a55e57b4788997a0acb8c7e9bfa6a285d)`(double umin,double umax,double fmin,double fmax)` | 
`public inline  `[`LinearValueConverter`](#class_linear_value_converter_1a3a3d093dada19f35641613ffb75fbaf0)`()` | 
`public inline virtual double `[`ui2faust`](#class_linear_value_converter_1a474d89f2f3364ac6fd3fc3307d434040)`(double x)` | 
`public inline virtual double `[`faust2ui`](#class_linear_value_converter_1a3538a042faf90f0d48d52e256509abe6)`(double x)` | 

## Members

#### `public inline  `[`LinearValueConverter`](#class_linear_value_converter_1a55e57b4788997a0acb8c7e9bfa6a285d)`(double umin,double umax,double fmin,double fmax)` {#class_linear_value_converter_1a55e57b4788997a0acb8c7e9bfa6a285d}

#### `public inline  `[`LinearValueConverter`](#class_linear_value_converter_1a3a3d093dada19f35641613ffb75fbaf0)`()` {#class_linear_value_converter_1a3a3d093dada19f35641613ffb75fbaf0}

#### `public inline virtual double `[`ui2faust`](#class_linear_value_converter_1a474d89f2f3364ac6fd3fc3307d434040)`(double x)` {#class_linear_value_converter_1a474d89f2f3364ac6fd3fc3307d434040}

#### `public inline virtual double `[`faust2ui`](#class_linear_value_converter_1a3538a042faf90f0d48d52e256509abe6)`(double x)` {#class_linear_value_converter_1a3538a042faf90f0d48d52e256509abe6}

# class `LinearValueConverter2` {#class_linear_value_converter2}

```
class LinearValueConverter2
  : public UpdatableValueConverter
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`LinearValueConverter2`](#class_linear_value_converter2_1a5538e97e27ad1c8608b2d98ebdf63950)`(double amin,double amid,double amax,double min,double init,double max)` | 
`public inline  `[`LinearValueConverter2`](#class_linear_value_converter2_1abbcf987ec16a4beeed125086eb9315c7)`()` | 
`public inline virtual double `[`ui2faust`](#class_linear_value_converter2_1ae9da7371993170b06bc63f5422cb9a25)`(double x)` | 
`public inline virtual double `[`faust2ui`](#class_linear_value_converter2_1a8c5ab3fc7efbedecf8b6a5f831d36000)`(double x)` | 
`public inline virtual void `[`setMappingValues`](#class_linear_value_converter2_1a6be71bdda4b0560fe06ab736721fa755)`(double amin,double amid,double amax,double min,double init,double max)` | 
`public inline virtual void `[`getMappingValues`](#class_linear_value_converter2_1a32ad8a0a62fa82c253661f946fbd8333)`(double & amin,double & amid,double & amax)` | 

## Members

#### `public inline  `[`LinearValueConverter2`](#class_linear_value_converter2_1a5538e97e27ad1c8608b2d98ebdf63950)`(double amin,double amid,double amax,double min,double init,double max)` {#class_linear_value_converter2_1a5538e97e27ad1c8608b2d98ebdf63950}

#### `public inline  `[`LinearValueConverter2`](#class_linear_value_converter2_1abbcf987ec16a4beeed125086eb9315c7)`()` {#class_linear_value_converter2_1abbcf987ec16a4beeed125086eb9315c7}

#### `public inline virtual double `[`ui2faust`](#class_linear_value_converter2_1ae9da7371993170b06bc63f5422cb9a25)`(double x)` {#class_linear_value_converter2_1ae9da7371993170b06bc63f5422cb9a25}

#### `public inline virtual double `[`faust2ui`](#class_linear_value_converter2_1a8c5ab3fc7efbedecf8b6a5f831d36000)`(double x)` {#class_linear_value_converter2_1a8c5ab3fc7efbedecf8b6a5f831d36000}

#### `public inline virtual void `[`setMappingValues`](#class_linear_value_converter2_1a6be71bdda4b0560fe06ab736721fa755)`(double amin,double amid,double amax,double min,double init,double max)` {#class_linear_value_converter2_1a6be71bdda4b0560fe06ab736721fa755}

#### `public inline virtual void `[`getMappingValues`](#class_linear_value_converter2_1a32ad8a0a62fa82c253661f946fbd8333)`(double & amin,double & amid,double & amax)` {#class_linear_value_converter2_1a32ad8a0a62fa82c253661f946fbd8333}

# class `LogValueConverter` {#class_log_value_converter}

```
class LogValueConverter
  : public LinearValueConverter
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`LogValueConverter`](#class_log_value_converter_1af0cd0c978ddfd30e5c143e97d81aaaa6)`(double umin,double umax,double fmin,double fmax)` | 
`public inline virtual double `[`ui2faust`](#class_log_value_converter_1a9a36441cc2a87f167aea5d663b88a63d)`(double x)` | 
`public inline virtual double `[`faust2ui`](#class_log_value_converter_1aff6106f5905353d6d39995ed4298c684)`(double x)` | 

## Members

#### `public inline  `[`LogValueConverter`](#class_log_value_converter_1af0cd0c978ddfd30e5c143e97d81aaaa6)`(double umin,double umax,double fmin,double fmax)` {#class_log_value_converter_1af0cd0c978ddfd30e5c143e97d81aaaa6}

#### `public inline virtual double `[`ui2faust`](#class_log_value_converter_1a9a36441cc2a87f167aea5d663b88a63d)`(double x)` {#class_log_value_converter_1a9a36441cc2a87f167aea5d663b88a63d}

#### `public inline virtual double `[`faust2ui`](#class_log_value_converter_1aff6106f5905353d6d39995ed4298c684)`(double x)` {#class_log_value_converter_1aff6106f5905353d6d39995ed4298c684}

# class `LoopBack` {#class_loop_back}

```
class LoopBack
  : public ProcessPlugin
```  

Connect Inputs to Outputs.

This Class just copy audio from its inputs to its outputs.

It can be use to do loopback without the need to externally connect channels in JACK. Note that if you *do* connect the channels in jack, you'll be effectively multiplying the signal by 2.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`LoopBack`](#class_loop_back_1ae3e0367b78b9d3e76f0616e679d61a59)`(int numchans)` | The class constructor sets the number of channels to connect as loopback.
`public inline virtual  `[`~LoopBack`](#class_loop_back_1a7d011607c08cf956478306bc768e53bf)`()` | The class destructor.
`public inline virtual int `[`getNumInputs`](#class_loop_back_1a49ce2344cffbb0954efb195cd51afe65)`()` | Return Number of Input Channels.
`public inline virtual int `[`getNumOutputs`](#class_loop_back_1af585868e87ea8e199b0fef72eb442fc0)`()` | Return Number of Output Channels.
`public virtual void `[`compute`](#class_loop_back_1a5e32c1e3aa3695dc79fad28bd73cf95e)`(int nframes,float ** inputs,float ** outputs)` | Compute process.

## Members

#### `public inline  `[`LoopBack`](#class_loop_back_1ae3e0367b78b9d3e76f0616e679d61a59)`(int numchans)` {#class_loop_back_1ae3e0367b78b9d3e76f0616e679d61a59}

The class constructor sets the number of channels to connect as loopback.

#### `public inline virtual  `[`~LoopBack`](#class_loop_back_1a7d011607c08cf956478306bc768e53bf)`()` {#class_loop_back_1a7d011607c08cf956478306bc768e53bf}

The class destructor.

#### `public inline virtual int `[`getNumInputs`](#class_loop_back_1a49ce2344cffbb0954efb195cd51afe65)`()` {#class_loop_back_1a49ce2344cffbb0954efb195cd51afe65}

Return Number of Input Channels.

#### `public inline virtual int `[`getNumOutputs`](#class_loop_back_1af585868e87ea8e199b0fef72eb442fc0)`()` {#class_loop_back_1af585868e87ea8e199b0fef72eb442fc0}

Return Number of Output Channels.

#### `public virtual void `[`compute`](#class_loop_back_1a5e32c1e3aa3695dc79fad28bd73cf95e)`(int nframes,float ** inputs,float ** outputs)` {#class_loop_back_1a5e32c1e3aa3695dc79fad28bd73cf95e}

Compute process.

# class `NetKS` {#class_net_k_s}

```
class NetKS
  : public ProcessPlugin
```  

A simple (basic) network Karplus Strong.

This plugin creates a one channel network karplus strong.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline virtual int `[`getNumInputs`](#class_net_k_s_1aeb39522d392194d4830018aec8ae3aae)`()` | Return Number of Input Channels.
`public inline virtual int `[`getNumOutputs`](#class_net_k_s_1aa4fe17c36115b4bde1a87906c561bc0f)`()` | Return Number of Output Channels.
`public inline virtual void `[`instanceInit`](#class_net_k_s_1a823566485917d45f8e27196d8e144d3d)`(int samplingFreq)` | 
`public inline virtual void `[`init`](#class_net_k_s_1a2b4550cd6cd356d86410463cd59db1ef)`(int samplingRate)` | Do proper Initialization of members and class instances. By default this initializes the Sampling Frequency. If a class instance depends on the sampling frequency, it should be initialize here.
`public inline virtual void `[`compute`](#class_net_k_s_1a76324f4cd487618a55ed6e28a8a47b9d)`(int nframes,float ** inputs,float ** outputs)` | Compute process.

## Members

#### `public inline virtual int `[`getNumInputs`](#class_net_k_s_1aeb39522d392194d4830018aec8ae3aae)`()` {#class_net_k_s_1aeb39522d392194d4830018aec8ae3aae}

Return Number of Input Channels.

#### `public inline virtual int `[`getNumOutputs`](#class_net_k_s_1aa4fe17c36115b4bde1a87906c561bc0f)`()` {#class_net_k_s_1aa4fe17c36115b4bde1a87906c561bc0f}

Return Number of Output Channels.

#### `public inline virtual void `[`instanceInit`](#class_net_k_s_1a823566485917d45f8e27196d8e144d3d)`(int samplingFreq)` {#class_net_k_s_1a823566485917d45f8e27196d8e144d3d}

#### `public inline virtual void `[`init`](#class_net_k_s_1a2b4550cd6cd356d86410463cd59db1ef)`(int samplingRate)` {#class_net_k_s_1a2b4550cd6cd356d86410463cd59db1ef}

Do proper Initialization of members and class instances. By default this initializes the Sampling Frequency. If a class instance depends on the sampling frequency, it should be initialize here.

#### `public inline virtual void `[`compute`](#class_net_k_s_1a76324f4cd487618a55ed6e28a8a47b9d)`(int nframes,float ** inputs,float ** outputs)` {#class_net_k_s_1a76324f4cd487618a55ed6e28a8a47b9d}

Compute process.

# class `PacketHeader` {#class_packet_header}

```
class PacketHeader
  : public QObject
```  

Base class for header type. Subclass this struct to create a new header.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  `[`PacketHeader`](#class_packet_header_1a258260b7f4f1d9d0a316dcf958b95ce4)`(`[`JackTrip`](#class_jack_trip)` * jacktrip)` | The class Constructor.
`public inline virtual  `[`~PacketHeader`](#class_packet_header_1a8b138c87057c959cfa2e5921e0b541a6)`()` | The class Destructor.
`public void `[`fillHeaderCommonFromAudio`](#class_packet_header_1ad601f4d63049dd3489de9b9bd0de12ce)`()` | > Todo: Implement this using a [JackTrip](#class_jack_trip) Method (Mediator) member instead of the reference to JackAudio
`public void `[`parseHeader`](#class_packet_header_1a6809614b5e4b65b58f8cdf157cd548ab)`()` | Parse the packet header and take appropriate measures (like change settings, or quit the program if peer settings don't match)
`public void `[`checkPeerSettings`](#class_packet_header_1aba5e15f68e2529e18d8faaa240e8f55e)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet)` | 
`public uint64_t `[`getPeerTimeStamp`](#class_packet_header_1ac9d82e095652ac13c0c1500488895bcb)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` | 
`public `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerSequenceNumber`](#class_packet_header_1ad80c606d6d40735735c307cb3fc2327e)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` | 
`public `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerBufferSize`](#class_packet_header_1ab9db47211bac13322c9f2ff6529a8f34)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` | 
`public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerSamplingRate`](#class_packet_header_1ab242cb3da9d3573eeac7cf61ce927070)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` | 
`public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerBitResolution`](#class_packet_header_1a6b1704b9b4744ab6f368be2cfdd88a68)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` | 
`public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerNumChannels`](#class_packet_header_1aba4685f3b9b54aff2f020fda5cad92af)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` | 
`public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerConnectionMode`](#class_packet_header_1a72c41ac2e2ffbf2323499bd3911fb969)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` | 
`public inline virtual void `[`increaseSequenceNumber`](#class_packet_header_1a963d7c4066be12e8224421aee4534f09)`()` | Increase sequence number for counter, a 16bit number.
`public inline virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getSequenceNumber`](#class_packet_header_1a157c6f89eee184ef72126128cb32d7ba)`() const` | Returns the current sequence number.
`public int `[`getHeaderSizeInBytes`](#class_packet_header_1afcbe432ee541d39a0695ee5cc8c649a0)`() const` | Get the header size in bytes.
`public inline virtual void `[`putHeaderInPacketBaseClass`](#class_packet_header_1ac970c6a26a312ec9d8c67bf213e1bf7d)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet,const `[`HeaderStruct`](#struct_header_struct)` & header_struct)` | 
`public void `[`putHeaderInPacket`](#class_packet_header_1a31ead0aadb20c2475a3d516f0cdd8830)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet)` | Put the header in buffer pointed by full_packet.
`{signal} public void `[`signalError`](#class_packet_header_1aad520d417ed2c92b0ee029f0d6c2e00a)`(const QString & error_message)` | 

## Members

#### `public  `[`PacketHeader`](#class_packet_header_1a258260b7f4f1d9d0a316dcf958b95ce4)`(`[`JackTrip`](#class_jack_trip)` * jacktrip)` {#class_packet_header_1a258260b7f4f1d9d0a316dcf958b95ce4}

The class Constructor.

#### `public inline virtual  `[`~PacketHeader`](#class_packet_header_1a8b138c87057c959cfa2e5921e0b541a6)`()` {#class_packet_header_1a8b138c87057c959cfa2e5921e0b541a6}

The class Destructor.

#### `public void `[`fillHeaderCommonFromAudio`](#class_packet_header_1ad601f4d63049dd3489de9b9bd0de12ce)`()` {#class_packet_header_1ad601f4d63049dd3489de9b9bd0de12ce}

> Todo: Implement this using a [JackTrip](#class_jack_trip) Method (Mediator) member instead of the reference to JackAudio

#### `public void `[`parseHeader`](#class_packet_header_1a6809614b5e4b65b58f8cdf157cd548ab)`()` {#class_packet_header_1a6809614b5e4b65b58f8cdf157cd548ab}

Parse the packet header and take appropriate measures (like change settings, or quit the program if peer settings don't match)

#### `public void `[`checkPeerSettings`](#class_packet_header_1aba5e15f68e2529e18d8faaa240e8f55e)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet)` {#class_packet_header_1aba5e15f68e2529e18d8faaa240e8f55e}

#### `public uint64_t `[`getPeerTimeStamp`](#class_packet_header_1ac9d82e095652ac13c0c1500488895bcb)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` {#class_packet_header_1ac9d82e095652ac13c0c1500488895bcb}

#### `public `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerSequenceNumber`](#class_packet_header_1ad80c606d6d40735735c307cb3fc2327e)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` {#class_packet_header_1ad80c606d6d40735735c307cb3fc2327e}

#### `public `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getPeerBufferSize`](#class_packet_header_1ab9db47211bac13322c9f2ff6529a8f34)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` {#class_packet_header_1ab9db47211bac13322c9f2ff6529a8f34}

#### `public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerSamplingRate`](#class_packet_header_1ab242cb3da9d3573eeac7cf61ce927070)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` {#class_packet_header_1ab242cb3da9d3573eeac7cf61ce927070}

#### `public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerBitResolution`](#class_packet_header_1a6b1704b9b4744ab6f368be2cfdd88a68)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` {#class_packet_header_1a6b1704b9b4744ab6f368be2cfdd88a68}

#### `public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerNumChannels`](#class_packet_header_1aba4685f3b9b54aff2f020fda5cad92af)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` {#class_packet_header_1aba4685f3b9b54aff2f020fda5cad92af}

#### `public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`getPeerConnectionMode`](#class_packet_header_1a72c41ac2e2ffbf2323499bd3911fb969)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet) const` {#class_packet_header_1a72c41ac2e2ffbf2323499bd3911fb969}

#### `public inline virtual void `[`increaseSequenceNumber`](#class_packet_header_1a963d7c4066be12e8224421aee4534f09)`()` {#class_packet_header_1a963d7c4066be12e8224421aee4534f09}

Increase sequence number for counter, a 16bit number.

#### `public inline virtual `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`getSequenceNumber`](#class_packet_header_1a157c6f89eee184ef72126128cb32d7ba)`() const` {#class_packet_header_1a157c6f89eee184ef72126128cb32d7ba}

Returns the current sequence number.

#### Returns
16bit Sequence number

#### `public int `[`getHeaderSizeInBytes`](#class_packet_header_1afcbe432ee541d39a0695ee5cc8c649a0)`() const` {#class_packet_header_1afcbe432ee541d39a0695ee5cc8c649a0}

Get the header size in bytes.

#### `public inline virtual void `[`putHeaderInPacketBaseClass`](#class_packet_header_1ac970c6a26a312ec9d8c67bf213e1bf7d)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet,const `[`HeaderStruct`](#struct_header_struct)` & header_struct)` {#class_packet_header_1ac970c6a26a312ec9d8c67bf213e1bf7d}

#### `public void `[`putHeaderInPacket`](#class_packet_header_1a31ead0aadb20c2475a3d516f0cdd8830)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_packet)` {#class_packet_header_1a31ead0aadb20c2475a3d516f0cdd8830}

Put the header in buffer pointed by full_packet.

#### Parameters
* `full_packet` Pointer to full packet (audio+header). Size must be sizeof(header part) + sizeof(audio part)

#### `{signal} public void `[`signalError`](#class_packet_header_1aad520d417ed2c92b0ee029f0d6c2e00a)`(const QString & error_message)` {#class_packet_header_1aad520d417ed2c92b0ee029f0d6c2e00a}

# class `PathBuilder` {#class_path_builder}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`PathBuilder`](#class_path_builder_1a219ba41e69206ea8c23abc7f48972e9c)`()` | 
`public inline virtual  `[`~PathBuilder`](#class_path_builder_1ae110b088769c90eb906e18feeea8ceeb)`()` | 
`public inline std::string `[`buildPath`](#class_path_builder_1af1c319e7b3c1abaa767053aaee290576)`(const std::string & label)` | 
`public inline std::string `[`buildLabel`](#class_path_builder_1a98206475a8d855d1bff84261691a9cfa)`(std::string label)` | 
`public inline void `[`pushLabel`](#class_path_builder_1a46e5c204b6765a286152c053ddd2b837)`(const std::string & label)` | 
`public inline void `[`popLabel`](#class_path_builder_1aa5838075558914c067b36a237e24966e)`()` | 
`protected std::vector< std::string > `[`fControlsLevel`](#class_path_builder_1a2ea1958885fbf21af3684684825358ca) | 

## Members

#### `public inline  `[`PathBuilder`](#class_path_builder_1a219ba41e69206ea8c23abc7f48972e9c)`()` {#class_path_builder_1a219ba41e69206ea8c23abc7f48972e9c}

#### `public inline virtual  `[`~PathBuilder`](#class_path_builder_1ae110b088769c90eb906e18feeea8ceeb)`()` {#class_path_builder_1ae110b088769c90eb906e18feeea8ceeb}

#### `public inline std::string `[`buildPath`](#class_path_builder_1af1c319e7b3c1abaa767053aaee290576)`(const std::string & label)` {#class_path_builder_1af1c319e7b3c1abaa767053aaee290576}

#### `public inline std::string `[`buildLabel`](#class_path_builder_1a98206475a8d855d1bff84261691a9cfa)`(std::string label)` {#class_path_builder_1a98206475a8d855d1bff84261691a9cfa}

#### `public inline void `[`pushLabel`](#class_path_builder_1a46e5c204b6765a286152c053ddd2b837)`(const std::string & label)` {#class_path_builder_1a46e5c204b6765a286152c053ddd2b837}

#### `public inline void `[`popLabel`](#class_path_builder_1aa5838075558914c067b36a237e24966e)`()` {#class_path_builder_1aa5838075558914c067b36a237e24966e}

#### `protected std::vector< std::string > `[`fControlsLevel`](#class_path_builder_1a2ea1958885fbf21af3684684825358ca) {#class_path_builder_1a2ea1958885fbf21af3684684825358ca}

# class `ProcessPlugin` {#class_process_plugin}

```
class ProcessPlugin
  : public QThread
```  

Interface for the process plugins to add to the JACK callback process in [JackAudioInterface](#class_jack_audio_interface).

This class contains the same methods of the FAUST dsp class. A mydsp class can inherit from this class the same way it inherits from dsp. Subclass should implement all methods except init, which is optional for processing that are sampling rate dependent or that need specific initialization.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`ProcessPlugin`](#class_process_plugin_1a32a0fd38bbef035ac134d75c2e8a8151)`()` | The Class Constructor.
`public inline virtual  `[`~ProcessPlugin`](#class_process_plugin_1a866c25570a2c087b4d58215595d4abad)`()` | The Class Destructor.
`public int `[`getNumInputs`](#class_process_plugin_1aace454a93a62dfdc96c1f66c9951304f)`()` | Return Number of Input Channels.
`public int `[`getNumOutputs`](#class_process_plugin_1aef66b85a5463a18ada72c225b3edc581)`()` | Return Number of Output Channels.
`public inline virtual char * `[`getName`](#class_process_plugin_1af67059fe391412a30b8748e34140ff7e)`()` | 
`public inline virtual void `[`init`](#class_process_plugin_1ab19b2570a366aa721c5725980bd8d358)`(int samplingRate)` | Do proper Initialization of members and class instances. By default this initializes the Sampling Frequency. If a class instance depends on the sampling frequency, it should be initialize here.
`public inline virtual bool `[`getInited`](#class_process_plugin_1ae1ae57c7aaf7c4b97cface77107fc644)`()` | 
`public inline virtual void `[`setVerbose`](#class_process_plugin_1a2d5a26e2ef41439f16afa76c581c71c8)`(bool v)` | 
`public void `[`compute`](#class_process_plugin_1a2b3e85cf1543f0a66fee56854ff3bb40)`(int nframes,float ** inputs,float ** outputs)` | Compute process.
`protected int `[`fSamplingFreq`](#class_process_plugin_1a3e78263cc0f285155dfc8029ec38813b) | Faust Data member, Sampling Rate.
`protected bool `[`inited`](#class_process_plugin_1af2e51f4e711f6461e3df30cc48dbfb64) | 
`protected bool `[`verbose`](#class_process_plugin_1af52737dcfe83c8ff11de3caba3d2c1e4) | 

## Members

#### `public inline  `[`ProcessPlugin`](#class_process_plugin_1a32a0fd38bbef035ac134d75c2e8a8151)`()` {#class_process_plugin_1a32a0fd38bbef035ac134d75c2e8a8151}

The Class Constructor.

#### `public inline virtual  `[`~ProcessPlugin`](#class_process_plugin_1a866c25570a2c087b4d58215595d4abad)`()` {#class_process_plugin_1a866c25570a2c087b4d58215595d4abad}

The Class Destructor.

#### `public int `[`getNumInputs`](#class_process_plugin_1aace454a93a62dfdc96c1f66c9951304f)`()` {#class_process_plugin_1aace454a93a62dfdc96c1f66c9951304f}

Return Number of Input Channels.

#### `public int `[`getNumOutputs`](#class_process_plugin_1aef66b85a5463a18ada72c225b3edc581)`()` {#class_process_plugin_1aef66b85a5463a18ada72c225b3edc581}

Return Number of Output Channels.

#### `public inline virtual char * `[`getName`](#class_process_plugin_1af67059fe391412a30b8748e34140ff7e)`()` {#class_process_plugin_1af67059fe391412a30b8748e34140ff7e}

#### `public inline virtual void `[`init`](#class_process_plugin_1ab19b2570a366aa721c5725980bd8d358)`(int samplingRate)` {#class_process_plugin_1ab19b2570a366aa721c5725980bd8d358}

Do proper Initialization of members and class instances. By default this initializes the Sampling Frequency. If a class instance depends on the sampling frequency, it should be initialize here.

#### `public inline virtual bool `[`getInited`](#class_process_plugin_1ae1ae57c7aaf7c4b97cface77107fc644)`()` {#class_process_plugin_1ae1ae57c7aaf7c4b97cface77107fc644}

#### `public inline virtual void `[`setVerbose`](#class_process_plugin_1a2d5a26e2ef41439f16afa76c581c71c8)`(bool v)` {#class_process_plugin_1a2d5a26e2ef41439f16afa76c581c71c8}

#### `public void `[`compute`](#class_process_plugin_1a2b3e85cf1543f0a66fee56854ff3bb40)`(int nframes,float ** inputs,float ** outputs)` {#class_process_plugin_1a2b3e85cf1543f0a66fee56854ff3bb40}

Compute process.

#### `protected int `[`fSamplingFreq`](#class_process_plugin_1a3e78263cc0f285155dfc8029ec38813b) {#class_process_plugin_1a3e78263cc0f285155dfc8029ec38813b}

Faust Data member, Sampling Rate.

#### `protected bool `[`inited`](#class_process_plugin_1af2e51f4e711f6461e3df30cc48dbfb64) {#class_process_plugin_1af2e51f4e711f6461e3df30cc48dbfb64}

#### `protected bool `[`verbose`](#class_process_plugin_1af52737dcfe83c8ff11de3caba3d2c1e4) {#class_process_plugin_1af52737dcfe83c8ff11de3caba3d2c1e4}

# class `Reverb` {#class_reverb}

```
class Reverb
  : public ProcessPlugin
```  

Applies freeverb or zitarev from the faustlibraries distribution: reverbs.lib.

A [Reverb](#class_reverb) is an echo-based delay effect, providing a virtual acoustic listening space.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`Reverb`](#class_reverb_1a3fca00dcc39b8de07bae131d6c0dddd4)`(int numInChans,int numOutChans,float reverbLevel,bool verboseFlag)` | The class constructor sets the number of channels to limit.
`public inline virtual  `[`~Reverb`](#class_reverb_1a16d44945f1d5be965154e6500358f38a)`()` | The class destructor.
`public inline virtual void `[`init`](#class_reverb_1a534765f7b21b3eb636656bf4ef5ba62c)`(int samplingRate)` | Do proper Initialization of members and class instances. By default this initializes the Sampling Frequency. If a class instance depends on the sampling frequency, it should be initialize here.
`public inline virtual int `[`getNumInputs`](#class_reverb_1abf98eebfdf33ef7c01f2a71b9e7e40e5)`()` | Return Number of Input Channels.
`public inline virtual int `[`getNumOutputs`](#class_reverb_1a14e2e9e11f8502e1c26a4fd7aba9d2ce)`()` | Return Number of Output Channels.
`public virtual void `[`compute`](#class_reverb_1a17e2d35335053052e887b6387be6ae80)`(int nframes,float ** inputs,float ** outputs)` | Compute process.

## Members

#### `public inline  `[`Reverb`](#class_reverb_1a3fca00dcc39b8de07bae131d6c0dddd4)`(int numInChans,int numOutChans,float reverbLevel,bool verboseFlag)` {#class_reverb_1a3fca00dcc39b8de07bae131d6c0dddd4}

The class constructor sets the number of channels to limit.

#### `public inline virtual  `[`~Reverb`](#class_reverb_1a16d44945f1d5be965154e6500358f38a)`()` {#class_reverb_1a16d44945f1d5be965154e6500358f38a}

The class destructor.

#### `public inline virtual void `[`init`](#class_reverb_1a534765f7b21b3eb636656bf4ef5ba62c)`(int samplingRate)` {#class_reverb_1a534765f7b21b3eb636656bf4ef5ba62c}

Do proper Initialization of members and class instances. By default this initializes the Sampling Frequency. If a class instance depends on the sampling frequency, it should be initialize here.

#### `public inline virtual int `[`getNumInputs`](#class_reverb_1abf98eebfdf33ef7c01f2a71b9e7e40e5)`()` {#class_reverb_1abf98eebfdf33ef7c01f2a71b9e7e40e5}

Return Number of Input Channels.

#### `public inline virtual int `[`getNumOutputs`](#class_reverb_1a14e2e9e11f8502e1c26a4fd7aba9d2ce)`()` {#class_reverb_1a14e2e9e11f8502e1c26a4fd7aba9d2ce}

Return Number of Output Channels.

#### `public virtual void `[`compute`](#class_reverb_1a17e2d35335053052e887b6387be6ae80)`(int nframes,float ** inputs,float ** outputs)` {#class_reverb_1a17e2d35335053052e887b6387be6ae80}

Compute process.

# class `RingBuffer` {#class_ring_buffer}

Provides a ring-buffer (or circular-buffer) that can be written to and read from asynchronously (blocking) or synchronously (non-blocking).

The [RingBuffer](#class_ring_buffer) is an array of **NumSlots** slots of memory each of which is of size **SlotSize** bytes (8-bits). Slots can be read and written asynchronously/synchronously by multiple threads.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  `[`RingBuffer`](#class_ring_buffer_1a64733f6fb0f21e5322a73e48c5074630)`(int SlotSize,int NumSlots)` | The class constructor.
`public virtual  `[`~RingBuffer`](#class_ring_buffer_1a2715b2e99ea24521ef7a586c2f33e1c9)`()` | The class destructor.
`public void `[`insertSlotBlocking`](#class_ring_buffer_1a8f2e273c7ec107b4ffbb39b75cb0c58e)`(const `[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToSlot)` | Insert a slot into the [RingBuffer](#class_ring_buffer) from ptrToSlot. This method will block until there's space in the buffer.
`public void `[`readSlotBlocking`](#class_ring_buffer_1a9345a4fb1520abee6db7cf1ae79119f9)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` | Read a slot from the [RingBuffer](#class_ring_buffer) into ptrToReadSlot. This method will block until there's space in the buffer.
`public virtual bool `[`insertSlotNonBlocking`](#class_ring_buffer_1a6a45eac1a0ee188c2554375a7b7f4440)`(const `[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToSlot,int len,int lostLen)` | Same as insertSlotBlocking but non-blocking (asynchronous)
`public virtual void `[`readSlotNonBlocking`](#class_ring_buffer_1a5abaeda3b1bab8ddadcfcb5225649853)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` | Same as readSlotBlocking but non-blocking (asynchronous)
`public virtual void `[`readBroadcastSlot`](#class_ring_buffer_1a51008be7b5dfd9d222418ff6fa51b6c2)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` | 
`public virtual bool `[`getStats`](#class_ring_buffer_1a9c4364d7dd503ceff1197f27f8fe118e)`(`[`IOStat`](#struct_ring_buffer_1_1_i_o_stat)` * stat,bool reset)` | 
`protected int `[`mSlotSize`](#class_ring_buffer_1a97d21a2d5c64d818258d69e2b356ea6d) | The size of one slot in byes.
`protected int `[`mNumSlots`](#class_ring_buffer_1a26918ada2ac7c16e86dfdedc7792bcb0) | Number of Slots.
`protected int `[`mTotalSize`](#class_ring_buffer_1a7af32873fff101269c65f8946f2489e3) | Total size of the mRingBuffer = mSlotSize*mNumSlotss.
`protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mReadPosition`](#class_ring_buffer_1a38869b6a28b562daa50d9a0e3f55dbd1) | Read Positions in the [RingBuffer](#class_ring_buffer) (Tail)
`protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mWritePosition`](#class_ring_buffer_1aba835bdf8fe9988376a75200299ef69c) | Write Position in the [RingBuffer](#class_ring_buffer) (Head)
`protected int `[`mFullSlots`](#class_ring_buffer_1ac84c18a0695aae44d0c3da9628ff58fd) | Number of used (full) slots, in slot-size.
`protected `[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * `[`mRingBuffer`](#class_ring_buffer_1a5bcd87978fdf9ff2b6922d245f52f8fc) | 8-bit array of data (1-byte)
`protected `[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * `[`mLastReadSlot`](#class_ring_buffer_1af2d46520b405499a6ee77d0c6f5a1a98) | Last slot read.
`protected QMutex `[`mMutex`](#class_ring_buffer_1af16728157c54e722b74417a6428648e1) | Mutex to protect read and write operations.
`protected QWaitCondition `[`mBufferIsNotFull`](#class_ring_buffer_1a804c25e2c5b59f1385288458298678a9) | Buffer not full condition to monitor threads.
`protected QWaitCondition `[`mBufferIsNotEmpty`](#class_ring_buffer_1ae1e669f78e821973c490cb3f20d8aede) | Buffer not empty condition to monitor threads.
`protected int `[`mStatUnit`](#class_ring_buffer_1afe5d080bc86eceae0028f8409bfda0cd) | 
`protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mUnderruns`](#class_ring_buffer_1ace5ea1f91a02085e7330d447ffdcf016) | 
`protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mOverflows`](#class_ring_buffer_1a1fa2a19ab586a5fb92233cecf694ed5e) | 
`protected `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`mSkewRaw`](#class_ring_buffer_1acb6755987d86fa7497cad34afca7d575) | 
`protected double `[`mLevelCur`](#class_ring_buffer_1a0b0c235a10c711d41d78646bdac744c2) | 
`protected double `[`mLevelDownRate`](#class_ring_buffer_1a1c8adca1f4ae760537b2856dcd79fe30) | 
`protected `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`mLevel`](#class_ring_buffer_1a4b928b56dfeaf67812a193f52a0e085a) | 
`protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mBufDecOverflow`](#class_ring_buffer_1a5af477f277d8acdb2f10161faf26db98) | 
`protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mBufDecPktLoss`](#class_ring_buffer_1aea0ca39ab6d1051266f0e0b3ffcd2dc4) | 
`protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mBufIncUnderrun`](#class_ring_buffer_1aae8f3a24a42d08589a48160a12c2d887) | 
`protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mBufIncCompensate`](#class_ring_buffer_1a072831777ae48c84aceb9bc9659f1207) | 
`protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mReadsNew`](#class_ring_buffer_1aa33f6a109b73cc9cf4778c3cd06c48b3) | 
`protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mUnderrunsNew`](#class_ring_buffer_1a8bca304d2af59247ff86be7774b6add9) | 
`protected `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`mSkew0`](#class_ring_buffer_1ada4302f6b5bd6731dd4da819261d19e6) | 
`protected `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`mBroadcastSkew`](#class_ring_buffer_1a5ddf1339ed82e7a85fee8002729898b4) | 
`protected `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`mBroadcastDelta`](#class_ring_buffer_1a79e2e3bb52c4484cdad7f96664f2ffba) | 
`protected virtual void `[`setUnderrunReadSlot`](#class_ring_buffer_1aef3d8f404901086d9190b0acec2a5443)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` | Sets the memory in the Read Slot when uderrun occurs. By default, this sets it to 0. Override this method in a subclass for a different behavior.
`protected virtual void `[`setMemoryInReadSlotWithLastReadSlot`](#class_ring_buffer_1ab6124a2d65e64656962168f14832c04c)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` | Uses the last read slot to set the memory in the Read Slot.
`protected void `[`underrunReset`](#class_ring_buffer_1aa511a08f13237bd1612135ed4b46d389)`()` | Resets the ring buffer for reads under-runs non-blocking.
`protected void `[`overflowReset`](#class_ring_buffer_1acbc4eeccd874fc0c6a9d6d29c141471f)`()` | Resets the ring buffer for writes over-flows non-blocking.
`protected void `[`debugDump`](#class_ring_buffer_1a27c8d1d7e5778c6eefd4cc7da5e33d13)`() const` | Helper method to debug, prints member variables to terminal.
`protected void `[`updateReadStats`](#class_ring_buffer_1a766bc7dbb4b04484b34f538f3e7a3f8a)`()` | 

## Members

#### `public  `[`RingBuffer`](#class_ring_buffer_1a64733f6fb0f21e5322a73e48c5074630)`(int SlotSize,int NumSlots)` {#class_ring_buffer_1a64733f6fb0f21e5322a73e48c5074630}

The class constructor.

#### Parameters
* `SlotSize` Size of one slot in bytes 

* `NumSlots` Number of slots

#### `public virtual  `[`~RingBuffer`](#class_ring_buffer_1a2715b2e99ea24521ef7a586c2f33e1c9)`()` {#class_ring_buffer_1a2715b2e99ea24521ef7a586c2f33e1c9}

The class destructor.

#### `public void `[`insertSlotBlocking`](#class_ring_buffer_1a8f2e273c7ec107b4ffbb39b75cb0c58e)`(const `[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToSlot)` {#class_ring_buffer_1a8f2e273c7ec107b4ffbb39b75cb0c58e}

Insert a slot into the [RingBuffer](#class_ring_buffer) from ptrToSlot. This method will block until there's space in the buffer.

The caller is responsible to make sure sizeof(WriteSlot) = SlotSize. This method should be use when the caller can block against its output, like sending/receiving UDP packets. It shouldn't be used by audio. For that, use the insertSlotNonBlocking. 
#### Parameters
* `ptrToSlot` Pointer to slot to insert into the [RingBuffer](#class_ring_buffer)

#### `public void `[`readSlotBlocking`](#class_ring_buffer_1a9345a4fb1520abee6db7cf1ae79119f9)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` {#class_ring_buffer_1a9345a4fb1520abee6db7cf1ae79119f9}

Read a slot from the [RingBuffer](#class_ring_buffer) into ptrToReadSlot. This method will block until there's space in the buffer.

The caller is responsible to make sure sizeof(ptrToReadSlot) = SlotSize. This method should be use when the caller can block against its input, like sending/receiving UDP packets. It shouldn't be used by audio. For that, use the readSlotNonBlocking. 
#### Parameters
* `ptrToReadSlot` Pointer to read slot from the [RingBuffer](#class_ring_buffer)

#### `public virtual bool `[`insertSlotNonBlocking`](#class_ring_buffer_1a6a45eac1a0ee188c2554375a7b7f4440)`(const `[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToSlot,int len,int lostLen)` {#class_ring_buffer_1a6a45eac1a0ee188c2554375a7b7f4440}

Same as insertSlotBlocking but non-blocking (asynchronous)

#### Parameters
* `ptrToSlot` Pointer to slot to insert into the [RingBuffer](#class_ring_buffer)

#### `public virtual void `[`readSlotNonBlocking`](#class_ring_buffer_1a5abaeda3b1bab8ddadcfcb5225649853)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` {#class_ring_buffer_1a5abaeda3b1bab8ddadcfcb5225649853}

Same as readSlotBlocking but non-blocking (asynchronous)

#### Parameters
* `ptrToReadSlot` Pointer to read slot from the [RingBuffer](#class_ring_buffer)

#### `public virtual void `[`readBroadcastSlot`](#class_ring_buffer_1a51008be7b5dfd9d222418ff6fa51b6c2)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` {#class_ring_buffer_1a51008be7b5dfd9d222418ff6fa51b6c2}

#### `public virtual bool `[`getStats`](#class_ring_buffer_1a9c4364d7dd503ceff1197f27f8fe118e)`(`[`IOStat`](#struct_ring_buffer_1_1_i_o_stat)` * stat,bool reset)` {#class_ring_buffer_1a9c4364d7dd503ceff1197f27f8fe118e}

#### `protected int `[`mSlotSize`](#class_ring_buffer_1a97d21a2d5c64d818258d69e2b356ea6d) {#class_ring_buffer_1a97d21a2d5c64d818258d69e2b356ea6d}

The size of one slot in byes.

#### `protected int `[`mNumSlots`](#class_ring_buffer_1a26918ada2ac7c16e86dfdedc7792bcb0) {#class_ring_buffer_1a26918ada2ac7c16e86dfdedc7792bcb0}

Number of Slots.

#### `protected int `[`mTotalSize`](#class_ring_buffer_1a7af32873fff101269c65f8946f2489e3) {#class_ring_buffer_1a7af32873fff101269c65f8946f2489e3}

Total size of the mRingBuffer = mSlotSize*mNumSlotss.

#### `protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mReadPosition`](#class_ring_buffer_1a38869b6a28b562daa50d9a0e3f55dbd1) {#class_ring_buffer_1a38869b6a28b562daa50d9a0e3f55dbd1}

Read Positions in the [RingBuffer](#class_ring_buffer) (Tail)

#### `protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mWritePosition`](#class_ring_buffer_1aba835bdf8fe9988376a75200299ef69c) {#class_ring_buffer_1aba835bdf8fe9988376a75200299ef69c}

Write Position in the [RingBuffer](#class_ring_buffer) (Head)

#### `protected int `[`mFullSlots`](#class_ring_buffer_1ac84c18a0695aae44d0c3da9628ff58fd) {#class_ring_buffer_1ac84c18a0695aae44d0c3da9628ff58fd}

Number of used (full) slots, in slot-size.

#### `protected `[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * `[`mRingBuffer`](#class_ring_buffer_1a5bcd87978fdf9ff2b6922d245f52f8fc) {#class_ring_buffer_1a5bcd87978fdf9ff2b6922d245f52f8fc}

8-bit array of data (1-byte)

#### `protected `[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * `[`mLastReadSlot`](#class_ring_buffer_1af2d46520b405499a6ee77d0c6f5a1a98) {#class_ring_buffer_1af2d46520b405499a6ee77d0c6f5a1a98}

Last slot read.

#### `protected QMutex `[`mMutex`](#class_ring_buffer_1af16728157c54e722b74417a6428648e1) {#class_ring_buffer_1af16728157c54e722b74417a6428648e1}

Mutex to protect read and write operations.

#### `protected QWaitCondition `[`mBufferIsNotFull`](#class_ring_buffer_1a804c25e2c5b59f1385288458298678a9) {#class_ring_buffer_1a804c25e2c5b59f1385288458298678a9}

Buffer not full condition to monitor threads.

#### `protected QWaitCondition `[`mBufferIsNotEmpty`](#class_ring_buffer_1ae1e669f78e821973c490cb3f20d8aede) {#class_ring_buffer_1ae1e669f78e821973c490cb3f20d8aede}

Buffer not empty condition to monitor threads.

#### `protected int `[`mStatUnit`](#class_ring_buffer_1afe5d080bc86eceae0028f8409bfda0cd) {#class_ring_buffer_1afe5d080bc86eceae0028f8409bfda0cd}

#### `protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mUnderruns`](#class_ring_buffer_1ace5ea1f91a02085e7330d447ffdcf016) {#class_ring_buffer_1ace5ea1f91a02085e7330d447ffdcf016}

#### `protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mOverflows`](#class_ring_buffer_1a1fa2a19ab586a5fb92233cecf694ed5e) {#class_ring_buffer_1a1fa2a19ab586a5fb92233cecf694ed5e}

#### `protected `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`mSkewRaw`](#class_ring_buffer_1acb6755987d86fa7497cad34afca7d575) {#class_ring_buffer_1acb6755987d86fa7497cad34afca7d575}

#### `protected double `[`mLevelCur`](#class_ring_buffer_1a0b0c235a10c711d41d78646bdac744c2) {#class_ring_buffer_1a0b0c235a10c711d41d78646bdac744c2}

#### `protected double `[`mLevelDownRate`](#class_ring_buffer_1a1c8adca1f4ae760537b2856dcd79fe30) {#class_ring_buffer_1a1c8adca1f4ae760537b2856dcd79fe30}

#### `protected `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`mLevel`](#class_ring_buffer_1a4b928b56dfeaf67812a193f52a0e085a) {#class_ring_buffer_1a4b928b56dfeaf67812a193f52a0e085a}

#### `protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mBufDecOverflow`](#class_ring_buffer_1a5af477f277d8acdb2f10161faf26db98) {#class_ring_buffer_1a5af477f277d8acdb2f10161faf26db98}

#### `protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mBufDecPktLoss`](#class_ring_buffer_1aea0ca39ab6d1051266f0e0b3ffcd2dc4) {#class_ring_buffer_1aea0ca39ab6d1051266f0e0b3ffcd2dc4}

#### `protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mBufIncUnderrun`](#class_ring_buffer_1aae8f3a24a42d08589a48160a12c2d887) {#class_ring_buffer_1aae8f3a24a42d08589a48160a12c2d887}

#### `protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mBufIncCompensate`](#class_ring_buffer_1a072831777ae48c84aceb9bc9659f1207) {#class_ring_buffer_1a072831777ae48c84aceb9bc9659f1207}

#### `protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mReadsNew`](#class_ring_buffer_1aa33f6a109b73cc9cf4778c3cd06c48b3) {#class_ring_buffer_1aa33f6a109b73cc9cf4778c3cd06c48b3}

#### `protected `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`mUnderrunsNew`](#class_ring_buffer_1a8bca304d2af59247ff86be7774b6add9) {#class_ring_buffer_1a8bca304d2af59247ff86be7774b6add9}

#### `protected `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`mSkew0`](#class_ring_buffer_1ada4302f6b5bd6731dd4da819261d19e6) {#class_ring_buffer_1ada4302f6b5bd6731dd4da819261d19e6}

#### `protected `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`mBroadcastSkew`](#class_ring_buffer_1a5ddf1339ed82e7a85fee8002729898b4) {#class_ring_buffer_1a5ddf1339ed82e7a85fee8002729898b4}

#### `protected `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`mBroadcastDelta`](#class_ring_buffer_1a79e2e3bb52c4484cdad7f96664f2ffba) {#class_ring_buffer_1a79e2e3bb52c4484cdad7f96664f2ffba}

#### `protected virtual void `[`setUnderrunReadSlot`](#class_ring_buffer_1aef3d8f404901086d9190b0acec2a5443)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` {#class_ring_buffer_1aef3d8f404901086d9190b0acec2a5443}

Sets the memory in the Read Slot when uderrun occurs. By default, this sets it to 0. Override this method in a subclass for a different behavior.

#### Parameters
* `ptrToReadSlot` Pointer to read slot from the [RingBuffer](#class_ring_buffer)

#### `protected virtual void `[`setMemoryInReadSlotWithLastReadSlot`](#class_ring_buffer_1ab6124a2d65e64656962168f14832c04c)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` {#class_ring_buffer_1ab6124a2d65e64656962168f14832c04c}

Uses the last read slot to set the memory in the Read Slot.

The last read slot is the last packet that arrived, so if no new packets are received, it keeps looping the same packet. 
#### Parameters
* `ptrToReadSlot` Pointer to read slot from the [RingBuffer](#class_ring_buffer)

#### `protected void `[`underrunReset`](#class_ring_buffer_1aa511a08f13237bd1612135ed4b46d389)`()` {#class_ring_buffer_1aa511a08f13237bd1612135ed4b46d389}

Resets the ring buffer for reads under-runs non-blocking.

#### `protected void `[`overflowReset`](#class_ring_buffer_1acbc4eeccd874fc0c6a9d6d29c141471f)`()` {#class_ring_buffer_1acbc4eeccd874fc0c6a9d6d29c141471f}

Resets the ring buffer for writes over-flows non-blocking.

#### `protected void `[`debugDump`](#class_ring_buffer_1a27c8d1d7e5778c6eefd4cc7da5e33d13)`() const` {#class_ring_buffer_1a27c8d1d7e5778c6eefd4cc7da5e33d13}

Helper method to debug, prints member variables to terminal.

#### `protected void `[`updateReadStats`](#class_ring_buffer_1a766bc7dbb4b04484b34f538f3e7a3f8a)`()` {#class_ring_buffer_1a766bc7dbb4b04484b34f538f3e7a3f8a}

# class `RingBufferWavetable` {#class_ring_buffer_wavetable}

```
class RingBufferWavetable
  : public RingBuffer
```  

Same as [RingBuffer](#class_ring_buffer), except that it uses the Wavetable mode for lost or late packets.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`RingBufferWavetable`](#class_ring_buffer_wavetable_1a7d100b483154cae83b1d7b2f1f974fed)`(int SlotSize,int NumSlots)` | The class constructor.
`public inline virtual  `[`~RingBufferWavetable`](#class_ring_buffer_wavetable_1a6e38cddb5eb0c4a5b8146a38094156fa)`()` | The class destructor.
`protected inline virtual void `[`setUnderrunReadSlot`](#class_ring_buffer_wavetable_1a853d18899e8f82c79e876a7fb691675b)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` | Sets the memory in the Read Slot when uderrun occurs. This loops as a wavetable in the last received packet.

## Members

#### `public inline  `[`RingBufferWavetable`](#class_ring_buffer_wavetable_1a7d100b483154cae83b1d7b2f1f974fed)`(int SlotSize,int NumSlots)` {#class_ring_buffer_wavetable_1a7d100b483154cae83b1d7b2f1f974fed}

The class constructor.

#### Parameters
* `SlotSize` Size of one slot in bytes 

* `NumSlots` Number of slots

#### `public inline virtual  `[`~RingBufferWavetable`](#class_ring_buffer_wavetable_1a6e38cddb5eb0c4a5b8146a38094156fa)`()` {#class_ring_buffer_wavetable_1a6e38cddb5eb0c4a5b8146a38094156fa}

The class destructor.

#### `protected inline virtual void `[`setUnderrunReadSlot`](#class_ring_buffer_wavetable_1a853d18899e8f82c79e876a7fb691675b)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * ptrToReadSlot)` {#class_ring_buffer_wavetable_1a853d18899e8f82c79e876a7fb691675b}

Sets the memory in the Read Slot when uderrun occurs. This loops as a wavetable in the last received packet.

#### Parameters
* `ptrToReadSlot` Pointer to read slot from the [RingBuffer](#class_ring_buffer)

# class `RtAudioInterface` {#class_rt_audio_interface}

```
class RtAudioInterface
  : public AudioInterface
```  

Base Class that provides an interface with RtAudio.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  `[`RtAudioInterface`](#class_rt_audio_interface_1ac907cc7da634bb8aea6b849b498ff93b)`(`[`JackTrip`](#class_jack_trip)` * jacktrip,int NumInChans,int NumOutChans,`[`audioBitResolutionT`](#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b)` AudioBitResolution)` | The class constructor.
`public virtual  `[`~RtAudioInterface`](#class_rt_audio_interface_1aa1ce96da4a64fd1a272d5a11a37b9f46)`()` | The class destructor.
`public virtual void `[`listAllInterfaces`](#class_rt_audio_interface_1a9dbce530ab0d07ada45cb795eb33178e)`()` | List all avialable audio interfaces, with its properties.
`public virtual void `[`setup`](#class_rt_audio_interface_1a33a9d21ada7464c484c1e7c75b26630f)`()` | Setup the client. This function should be called just before.
`public virtual int `[`startProcess`](#class_rt_audio_interface_1a375c7bb8811a7c238da159dbf5116780)`() const` | Tell the audio server that we are ready to roll. The process-callback will start running. This runs on its own thread.
`public virtual int `[`stopProcess`](#class_rt_audio_interface_1af03530184e3b29d49b5159edcc7bad75)`() const` | Stops the process-callback thread.
`public inline virtual void `[`connectDefaultPorts`](#class_rt_audio_interface_1ad4214d65d22d8f5625f4130985216429)`()` | This has no effect in RtAudio.
`public inline virtual void `[`setClientName`](#class_rt_audio_interface_1a2e3a089e0318a73cb68854a7010214b0)`(QString)` | This has no effect in RtAudio.

## Members

#### `public  `[`RtAudioInterface`](#class_rt_audio_interface_1ac907cc7da634bb8aea6b849b498ff93b)`(`[`JackTrip`](#class_jack_trip)` * jacktrip,int NumInChans,int NumOutChans,`[`audioBitResolutionT`](#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b)` AudioBitResolution)` {#class_rt_audio_interface_1ac907cc7da634bb8aea6b849b498ff93b}

The class constructor.

#### Parameters
* `jacktrip` Pointer to the [JackTrip](#class_jack_trip) class that connects all classes (mediator) 

* `NumInChans` Number of Input Channels 

* `NumOutChans` Number of Output Channels 

* `AudioBitResolution` Audio Sample Resolutions in bits

#### `public virtual  `[`~RtAudioInterface`](#class_rt_audio_interface_1aa1ce96da4a64fd1a272d5a11a37b9f46)`()` {#class_rt_audio_interface_1aa1ce96da4a64fd1a272d5a11a37b9f46}

The class destructor.

#### `public virtual void `[`listAllInterfaces`](#class_rt_audio_interface_1a9dbce530ab0d07ada45cb795eb33178e)`()` {#class_rt_audio_interface_1a9dbce530ab0d07ada45cb795eb33178e}

List all avialable audio interfaces, with its properties.

#### `public virtual void `[`setup`](#class_rt_audio_interface_1a33a9d21ada7464c484c1e7c75b26630f)`()` {#class_rt_audio_interface_1a33a9d21ada7464c484c1e7c75b26630f}

Setup the client. This function should be called just before.

starting the audio processes, it will setup the audio client with the class parameters, like Sampling Rate, Packet Size, Bit Resolution, etc... Sub-classes should also call the parent method to ensure correct inizialization.

#### `public virtual int `[`startProcess`](#class_rt_audio_interface_1a375c7bb8811a7c238da159dbf5116780)`() const` {#class_rt_audio_interface_1a375c7bb8811a7c238da159dbf5116780}

Tell the audio server that we are ready to roll. The process-callback will start running. This runs on its own thread.

#### Returns
0 on success, otherwise a non-zero error code

#### `public virtual int `[`stopProcess`](#class_rt_audio_interface_1af03530184e3b29d49b5159edcc7bad75)`() const` {#class_rt_audio_interface_1af03530184e3b29d49b5159edcc7bad75}

Stops the process-callback thread.

#### Returns
0 on success, otherwise a non-zero error code

#### `public inline virtual void `[`connectDefaultPorts`](#class_rt_audio_interface_1ad4214d65d22d8f5625f4130985216429)`()` {#class_rt_audio_interface_1ad4214d65d22d8f5625f4130985216429}

This has no effect in RtAudio.

#### `public inline virtual void `[`setClientName`](#class_rt_audio_interface_1a2e3a089e0318a73cb68854a7010214b0)`(QString)` {#class_rt_audio_interface_1a2e3a089e0318a73cb68854a7010214b0}

This has no effect in RtAudio.

# class `Settings` {#class_settings}

```
class Settings
  : public QObject
```  

Class to set usage options and parse settings from input.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  `[`Settings`](#class_settings_1ab7169a6eefce79566dd07db3b1e5e967)`()` | 
`public virtual  `[`~Settings`](#class_settings_1a4a65be5921dfc9fddc476e5320541d89)`()` | 
`public void `[`parseInput`](#class_settings_1a7bc7d9d6c0d7ee24d869fba991c3dc0b)`(int argc,char ** argv)` | Parses command line input.
`public `[`UdpHubListener`](#class_udp_hub_listener)` * `[`getConfiguredHubServer`](#class_settings_1a3da20f27e35b70c274416825f26d48a3)`()` | 
`public `[`JackTrip`](#class_jack_trip)` * `[`getConfiguredJackTrip`](#class_settings_1a73492ef0c134aba9068113314847014f)`()` | 
`public void `[`printUsage`](#class_settings_1a9315abef24f993e0a47c444bdf6bd03f)`()` | Prints usage help.
`public inline bool `[`getLoopBack`](#class_settings_1add866d37691ce269f57c8a659a656891)`()` | 
`public inline bool `[`isHubServer`](#class_settings_1a8398da8062f377c871fe38b3735942c7)`()` | 

## Members

#### `public  `[`Settings`](#class_settings_1ab7169a6eefce79566dd07db3b1e5e967)`()` {#class_settings_1ab7169a6eefce79566dd07db3b1e5e967}

#### `public virtual  `[`~Settings`](#class_settings_1a4a65be5921dfc9fddc476e5320541d89)`()` {#class_settings_1a4a65be5921dfc9fddc476e5320541d89}

#### `public void `[`parseInput`](#class_settings_1a7bc7d9d6c0d7ee24d869fba991c3dc0b)`(int argc,char ** argv)` {#class_settings_1a7bc7d9d6c0d7ee24d869fba991c3dc0b}

Parses command line input.

#### `public `[`UdpHubListener`](#class_udp_hub_listener)` * `[`getConfiguredHubServer`](#class_settings_1a3da20f27e35b70c274416825f26d48a3)`()` {#class_settings_1a3da20f27e35b70c274416825f26d48a3}

#### `public `[`JackTrip`](#class_jack_trip)` * `[`getConfiguredJackTrip`](#class_settings_1a73492ef0c134aba9068113314847014f)`()` {#class_settings_1a73492ef0c134aba9068113314847014f}

#### `public void `[`printUsage`](#class_settings_1a9315abef24f993e0a47c444bdf6bd03f)`()` {#class_settings_1a9315abef24f993e0a47c444bdf6bd03f}

Prints usage help.

#### `public inline bool `[`getLoopBack`](#class_settings_1add866d37691ce269f57c8a659a656891)`()` {#class_settings_1add866d37691ce269f57c8a659a656891}

#### `public inline bool `[`isHubServer`](#class_settings_1a8398da8062f377c871fe38b3735942c7)`()` {#class_settings_1a8398da8062f377c871fe38b3735942c7}

# class `TestRingBufferRead` {#class_test_ring_buffer_read}

```
class TestRingBufferRead
  : public QThread
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline void `[`run`](#class_test_ring_buffer_read_1a69432b4a50267139806d9991a2d213f3)`()` | 

## Members

#### `public inline void `[`run`](#class_test_ring_buffer_read_1a69432b4a50267139806d9991a2d213f3)`()` {#class_test_ring_buffer_read_1a69432b4a50267139806d9991a2d213f3}

# class `TestRingBufferWrite` {#class_test_ring_buffer_write}

```
class TestRingBufferWrite
  : public QThread
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline void `[`run`](#class_test_ring_buffer_write_1abcb6b55d37a514c38c6d8ad06376dda6)`()` | 

## Members

#### `public inline void `[`run`](#class_test_ring_buffer_write_1abcb6b55d37a514c38c6d8ad06376dda6)`()` {#class_test_ring_buffer_write_1abcb6b55d37a514c38c6d8ad06376dda6}

# class `ThreadPoolTest` {#class_thread_pool_test}

```
class ThreadPoolTest
  : public QObject
  : public QRunnable
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`ThreadPoolTest`](#class_thread_pool_test_1a0cfd5a643025010984c27bd5abe3510f)`()` | 
`public inline void `[`run`](#class_thread_pool_test_1aa16110663230b3f4cbdd3a5d688f353e)`()` | 
`public inline void `[`stop`](#class_thread_pool_test_1abfea7e375df20e617b6a606ebc761fde)`()` | 
`{signal} public void `[`stopELoop`](#class_thread_pool_test_1a12bcbbbc44a85f8657a938b6a1679377)`()` | 

## Members

#### `public inline  `[`ThreadPoolTest`](#class_thread_pool_test_1a0cfd5a643025010984c27bd5abe3510f)`()` {#class_thread_pool_test_1a0cfd5a643025010984c27bd5abe3510f}

#### `public inline void `[`run`](#class_thread_pool_test_1aa16110663230b3f4cbdd3a5d688f353e)`()` {#class_thread_pool_test_1aa16110663230b3f4cbdd3a5d688f353e}

#### `public inline void `[`stop`](#class_thread_pool_test_1abfea7e375df20e617b6a606ebc761fde)`()` {#class_thread_pool_test_1abfea7e375df20e617b6a606ebc761fde}

#### `{signal} public void `[`stopELoop`](#class_thread_pool_test_1a12bcbbbc44a85f8657a938b6a1679377)`()` {#class_thread_pool_test_1a12bcbbbc44a85f8657a938b6a1679377}

# class `UdpDataProtocol` {#class_udp_data_protocol}

```
class UdpDataProtocol
  : public DataProtocol
```  

UDP implementation of [DataProtocol](#class_data_protocol) class.

The class has a `bind port` and a `peer port`. The meaning of these depends on the runModeT. If it's a SENDER, `bind port` is the source port and `peer port` is the destination port for each UDP packet. If it's a RECEIVER, the `bind port` destination port (for incoming packets) and the `peer port` is the source port.

The SENDER and RECEIVER socket can share the same port/address pair (for compatibility with the JamLink boxes). This is achieved setting the resusable property in the socket for address and port. You have to externaly check if the port is already binded if you want to avoid re-binding to the same port.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  `[`UdpDataProtocol`](#class_udp_data_protocol_1a8fe02a9611b22615c8adcacbdaa9e3c8)`(`[`JackTrip`](#class_jack_trip)` * jacktrip,const `[`runModeT`](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2c)` runmode,int bind_port,int peer_port,unsigned int udp_redundancy_factor)` | The class constructor.
`public virtual  `[`~UdpDataProtocol`](#class_udp_data_protocol_1a87bfe158a7b4113ebdc05aabc11656e4)`()` | The class destructor.
`public virtual void `[`setPeerAddress`](#class_udp_data_protocol_1afb2790b3586847f7d3516392a0dfc840)`(const char * peerHostOrIP)` | Set the Peer address to connect to.
`public virtual void `[`setSocket`](#class_udp_data_protocol_1aa19f7a633fa77bb2866740e33fb7db80)`(int & socket)` | 
`public virtual int `[`receivePacket`](#class_udp_data_protocol_1a284a07c9181b579b061deedcc37af1b6)`(char * buf,const size_t n)` | Receives a packet. It blocks until a packet is received.
`public virtual int `[`sendPacket`](#class_udp_data_protocol_1ae6dce595b01110f3de0fd3789e076009)`(const char * buf,const size_t n)` | Sends a packet.
`public virtual void `[`getPeerAddressFromFirstPacket`](#class_udp_data_protocol_1a270454a13ba5edaa5c08bc30e5f4b750)`(QHostAddress & peerHostAddress,`[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` & port)` | Obtains the peer address from the first UDP packet received. This address is used by the SERVER mode to connect back to the client.
`public inline void `[`setBindPort`](#class_udp_data_protocol_1a1a3774b6e2d4a6b54b188fa0da2d42ad)`(int port)` | Sets the bind port number.
`public inline virtual void `[`setPeerPort`](#class_udp_data_protocol_1af5426710f87e9be9223cee673a2c3129)`(int port)` | Sets the peer port number.
`public virtual void `[`run`](#class_udp_data_protocol_1ad2bab0e15d7d47dd50183047756e7ebb)`()` | Implements the Thread Loop. To start the thread, call start() ( DO NOT CALL [run()](#class_udp_data_protocol_1ad2bab0e15d7d47dd50183047756e7ebb) )
`public virtual bool `[`getStats`](#class_udp_data_protocol_1aba79747e34573b1df80f4ba9930954f5)`(`[`PktStat`](#struct_data_protocol_1_1_pkt_stat)` * stat)` | 
`public virtual void `[`setIssueSimulation`](#class_udp_data_protocol_1aeb07e126364599281b39938491f808cb)`(double loss,double jitter,double max_delay)` | 
`protected int `[`bindSocket`](#class_udp_data_protocol_1a06bfb762aa42f5136af37c877c5073d8)`()` | Binds the UDP socket to the available address and specified port.
`protected void `[`waitForReady`](#class_udp_data_protocol_1a1666be5719c18459aa169b7c1e0b99e1)`(int timeout_msec)` | This function blocks until data is available for reading in the socket. The function will timeout after timeout_msec microseconds.
`protected virtual void `[`receivePacketRedundancy`](#class_udp_data_protocol_1a4a5c2a4242b67481e59ca90ffb59a8a2)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_redundant_packet,int full_redundant_packet_size,int full_packet_size,`[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` & current_seq_num,`[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` & last_seq_num,`[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` & newer_seq_num)` | Redundancy algorythm at the receiving end.
`protected virtual void `[`sendPacketRedundancy`](#class_udp_data_protocol_1abfeaa93f4af217ff016a42afb083c414)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_redundant_packet,int full_redundant_packet_size,int full_packet_size)` | Redundancy algorythm at the sender's end.
`{signal} public void `[`signalWaitingTooLong`](#class_udp_data_protocol_1ae93742b3f3af0d543d7044c37eae5d5a)`(int wait_msec)` | Signals when waiting every 10 milliseconds, with the total wait on wait_msec.
`{signal} public void `[`signalUdpWaitingTooLong`](#class_udp_data_protocol_1ae868ea5265c0f61c032bdcd2fa91f704)`()` | 

## Members

#### `public  `[`UdpDataProtocol`](#class_udp_data_protocol_1a8fe02a9611b22615c8adcacbdaa9e3c8)`(`[`JackTrip`](#class_jack_trip)` * jacktrip,const `[`runModeT`](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2c)` runmode,int bind_port,int peer_port,unsigned int udp_redundancy_factor)` {#class_udp_data_protocol_1a8fe02a9611b22615c8adcacbdaa9e3c8}

The class constructor.

#### Parameters
* `jacktrip` Pointer to the [JackTrip](#class_jack_trip) class that connects all classes (mediator) 

* `runmode` Sets the run mode, use either SENDER or RECEIVER 

* `bind_port` Port number to bind for this socket (this is the receive or send port depending on the runmode) 

* `peer_port` Peer port number (this is the receive or send port depending on the runmode) 

* `udp_redundancy_factor` Number of redundant packets

#### `public virtual  `[`~UdpDataProtocol`](#class_udp_data_protocol_1a87bfe158a7b4113ebdc05aabc11656e4)`()` {#class_udp_data_protocol_1a87bfe158a7b4113ebdc05aabc11656e4}

The class destructor.

#### `public virtual void `[`setPeerAddress`](#class_udp_data_protocol_1afb2790b3586847f7d3516392a0dfc840)`(const char * peerHostOrIP)` {#class_udp_data_protocol_1afb2790b3586847f7d3516392a0dfc840}

Set the Peer address to connect to.

#### Parameters
* `peerHostOrIP` IPv4 number or host name

#### `public virtual void `[`setSocket`](#class_udp_data_protocol_1aa19f7a633fa77bb2866740e33fb7db80)`(int & socket)` {#class_udp_data_protocol_1aa19f7a633fa77bb2866740e33fb7db80}

#### `public virtual int `[`receivePacket`](#class_udp_data_protocol_1a284a07c9181b579b061deedcc37af1b6)`(char * buf,const size_t n)` {#class_udp_data_protocol_1a284a07c9181b579b061deedcc37af1b6}

Receives a packet. It blocks until a packet is received.

This function makes sure we recieve a complete packet of size n 
#### Parameters
* `buf` Buffer to store the recieved packet 

* `n` size of packet to receive 

#### Returns
number of bytes read, -1 on error

#### `public virtual int `[`sendPacket`](#class_udp_data_protocol_1ae6dce595b01110f3de0fd3789e076009)`(const char * buf,const size_t n)` {#class_udp_data_protocol_1ae6dce595b01110f3de0fd3789e076009}

Sends a packet.

This function meakes sure we send a complete packet of size n 
#### Parameters
* `buf` Buffer to send 

* `n` size of packet to receive 

#### Returns
number of bytes read, -1 on error

#### `public virtual void `[`getPeerAddressFromFirstPacket`](#class_udp_data_protocol_1a270454a13ba5edaa5c08bc30e5f4b750)`(QHostAddress & peerHostAddress,`[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` & port)` {#class_udp_data_protocol_1a270454a13ba5edaa5c08bc30e5f4b750}

Obtains the peer address from the first UDP packet received. This address is used by the SERVER mode to connect back to the client.

#### Parameters
* `peerHostAddress` QHostAddress to store the peer address 

* `port` Receiving port

#### `public inline void `[`setBindPort`](#class_udp_data_protocol_1a1a3774b6e2d4a6b54b188fa0da2d42ad)`(int port)` {#class_udp_data_protocol_1a1a3774b6e2d4a6b54b188fa0da2d42ad}

Sets the bind port number.

#### `public inline virtual void `[`setPeerPort`](#class_udp_data_protocol_1af5426710f87e9be9223cee673a2c3129)`(int port)` {#class_udp_data_protocol_1af5426710f87e9be9223cee673a2c3129}

Sets the peer port number.

#### `public virtual void `[`run`](#class_udp_data_protocol_1ad2bab0e15d7d47dd50183047756e7ebb)`()` {#class_udp_data_protocol_1ad2bab0e15d7d47dd50183047756e7ebb}

Implements the Thread Loop. To start the thread, call start() ( DO NOT CALL [run()](#class_udp_data_protocol_1ad2bab0e15d7d47dd50183047756e7ebb) )

This function creats and binds all the socket and start the connection loop thread.

#### `public virtual bool `[`getStats`](#class_udp_data_protocol_1aba79747e34573b1df80f4ba9930954f5)`(`[`PktStat`](#struct_data_protocol_1_1_pkt_stat)` * stat)` {#class_udp_data_protocol_1aba79747e34573b1df80f4ba9930954f5}

#### `public virtual void `[`setIssueSimulation`](#class_udp_data_protocol_1aeb07e126364599281b39938491f808cb)`(double loss,double jitter,double max_delay)` {#class_udp_data_protocol_1aeb07e126364599281b39938491f808cb}

#### `protected int `[`bindSocket`](#class_udp_data_protocol_1a06bfb762aa42f5136af37c877c5073d8)`()` {#class_udp_data_protocol_1a06bfb762aa42f5136af37c877c5073d8}

Binds the UDP socket to the available address and specified port.

#### `protected void `[`waitForReady`](#class_udp_data_protocol_1a1666be5719c18459aa169b7c1e0b99e1)`(int timeout_msec)` {#class_udp_data_protocol_1a1666be5719c18459aa169b7c1e0b99e1}

This function blocks until data is available for reading in the socket. The function will timeout after timeout_msec microseconds.

This function is intended to replace QAbstractSocket::waitForReadyRead which has some problems with multithreading.

#### Returns
returns true if there is data available for reading; otherwise it returns false (if an error occurred or the operation timed out)

#### `protected virtual void `[`receivePacketRedundancy`](#class_udp_data_protocol_1a4a5c2a4242b67481e59ca90ffb59a8a2)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_redundant_packet,int full_redundant_packet_size,int full_packet_size,`[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` & current_seq_num,`[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` & last_seq_num,`[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` & newer_seq_num)` {#class_udp_data_protocol_1a4a5c2a4242b67481e59ca90ffb59a8a2}

Redundancy algorythm at the receiving end.

#### `protected virtual void `[`sendPacketRedundancy`](#class_udp_data_protocol_1abfeaa93f4af217ff016a42afb083c414)`(`[`int8_t`](#jacktrip__types_8h_1ad41f47fad3eada48c98309cf2c3a6d83)` * full_redundant_packet,int full_redundant_packet_size,int full_packet_size)` {#class_udp_data_protocol_1abfeaa93f4af217ff016a42afb083c414}

Redundancy algorythm at the sender's end.

#### `{signal} public void `[`signalWaitingTooLong`](#class_udp_data_protocol_1ae93742b3f3af0d543d7044c37eae5d5a)`(int wait_msec)` {#class_udp_data_protocol_1ae93742b3f3af0d543d7044c37eae5d5a}

Signals when waiting every 10 milliseconds, with the total wait on wait_msec.

#### Parameters
* `wait_msec` Total wait in milliseconds

#### `{signal} public void `[`signalUdpWaitingTooLong`](#class_udp_data_protocol_1ae868ea5265c0f61c032bdcd2fa91f704)`()` {#class_udp_data_protocol_1ae868ea5265c0f61c032bdcd2fa91f704}

# class `UdpHubListener` {#class_udp_hub_listener}

```
class UdpHubListener
  : public QObject
```  

Hub UDP listener on the Server.

This creates a server that will listen on the well know port (the server port) and will spawn [JackTrip](#class_jack_trip) threads into the Thread pool. Clients request a connection.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public unsigned int `[`mHubPatch`](#class_udp_hub_listener_1aef3946bc18580a6c78e5d62af5456e84) | 
`public bool `[`mAppendThreadID`](#class_udp_hub_listener_1a112cbeef12478227390fd8d1a16755ac) | 
`public  `[`UdpHubListener`](#class_udp_hub_listener_1a1a661264bbd7e35148566d20f35a5975)`(int server_port,int server_udp_port)` | 
`public virtual  `[`~UdpHubListener`](#class_udp_hub_listener_1a8a306efd7ae43ad8005f5f3f02fb44e8)`()` | 
`public void `[`start`](#class_udp_hub_listener_1a14302c1181fc52ee905161f2f4441aff)`()` | Starts the TCP server.
`public inline void `[`stop`](#class_udp_hub_listener_1abcf1c02b861a77289471da58f105f124)`()` | Stops the execution of the Thread.
`public int `[`releaseThread`](#class_udp_hub_listener_1a0940af2aa63d40db896e20840f1d8029)`(int id)` | 
`public inline void `[`setConnectDefaultAudioPorts`](#class_udp_hub_listener_1aeac1db5ebf010f290e50500b7683edbe)`(bool connectDefaultAudioPorts)` | 
`public inline void `[`setHubPatch`](#class_udp_hub_listener_1a961a0ca9ff7f1840ec1b7cdd9ae24055)`(unsigned int p)` | 
`public inline unsigned int `[`getHubPatch`](#class_udp_hub_listener_1a548db06a0e900048437d36a8816d05f3)`()` | 
`public inline void `[`setUnderRunMode`](#class_udp_hub_listener_1ad6f3f47b75bc35d1c287ebb8eb2c049a)`(`[`JackTrip::underrunModeT`](#class_jack_trip_1a9fd34b5de9751d9bafbb03a7c2250bf5)` UnderRunMode)` | 
`public inline void `[`setBufferQueueLength`](#class_udp_hub_listener_1a8b17895aa1f3788b016a75b1ee0a4039)`(int BufferQueueLength)` | 
`public inline void `[`setIOStatTimeout`](#class_udp_hub_listener_1a67491c6165e0f4e3f658e3e3a69c8a53)`(int timeout)` | 
`public inline void `[`setIOStatStream`](#class_udp_hub_listener_1a990ad20982069e2392482e57a25b6700)`(QSharedPointer< std::ofstream > statStream)` | 
`public inline void `[`setBufferStrategy`](#class_udp_hub_listener_1a05ef98435e7f56e7e0f89c8e14e850b3)`(int BufferStrategy)` | 
`public inline void `[`setNetIssuesSimulation`](#class_udp_hub_listener_1a3a044b4e620511457bfa10ea083c9e04)`(double loss,double jitter,double delay_rel)` | 
`public inline void `[`setBroadcast`](#class_udp_hub_listener_1a1da0df938d4e39b1ce05097b5ae20987)`(int broadcast_queue)` | 
`public inline void `[`setUseRtUdpPriority`](#class_udp_hub_listener_1a070b0d78c8f656523af2456d7a0c0e7b)`(bool use)` | 
`{signal} public void `[`Listening`](#class_udp_hub_listener_1a5af2f5a3e2859802f95c6dd7b552761e)`()` | 
`{signal} public void `[`ClientAddressSet`](#class_udp_hub_listener_1ab58bf3699e2ae0f0a1845e5f2a983003)`()` | 
`{signal} public void `[`signalRemoveThread`](#class_udp_hub_listener_1ace9855059fb7d305e208e658aa8ac502)`(int id)` | 
`{signal} public void `[`signalStopped`](#class_udp_hub_listener_1a81493e16c1c13915dd7e917fe45a7313)`()` | 
`{signal} public void `[`signalError`](#class_udp_hub_listener_1ab66da45c363859869526b3406a050394)`(const QString & errorMessage)` | 

## Members

#### `public unsigned int `[`mHubPatch`](#class_udp_hub_listener_1aef3946bc18580a6c78e5d62af5456e84) {#class_udp_hub_listener_1aef3946bc18580a6c78e5d62af5456e84}

#### `public bool `[`mAppendThreadID`](#class_udp_hub_listener_1a112cbeef12478227390fd8d1a16755ac) {#class_udp_hub_listener_1a112cbeef12478227390fd8d1a16755ac}

#### `public  `[`UdpHubListener`](#class_udp_hub_listener_1a1a661264bbd7e35148566d20f35a5975)`(int server_port,int server_udp_port)` {#class_udp_hub_listener_1a1a661264bbd7e35148566d20f35a5975}

#### `public virtual  `[`~UdpHubListener`](#class_udp_hub_listener_1a8a306efd7ae43ad8005f5f3f02fb44e8)`()` {#class_udp_hub_listener_1a8a306efd7ae43ad8005f5f3f02fb44e8}

#### `public void `[`start`](#class_udp_hub_listener_1a14302c1181fc52ee905161f2f4441aff)`()` {#class_udp_hub_listener_1a14302c1181fc52ee905161f2f4441aff}

Starts the TCP server.

#### `public inline void `[`stop`](#class_udp_hub_listener_1abcf1c02b861a77289471da58f105f124)`()` {#class_udp_hub_listener_1abcf1c02b861a77289471da58f105f124}

Stops the execution of the Thread.

#### `public int `[`releaseThread`](#class_udp_hub_listener_1a0940af2aa63d40db896e20840f1d8029)`(int id)` {#class_udp_hub_listener_1a0940af2aa63d40db896e20840f1d8029}

#### `public inline void `[`setConnectDefaultAudioPorts`](#class_udp_hub_listener_1aeac1db5ebf010f290e50500b7683edbe)`(bool connectDefaultAudioPorts)` {#class_udp_hub_listener_1aeac1db5ebf010f290e50500b7683edbe}

#### `public inline void `[`setHubPatch`](#class_udp_hub_listener_1a961a0ca9ff7f1840ec1b7cdd9ae24055)`(unsigned int p)` {#class_udp_hub_listener_1a961a0ca9ff7f1840ec1b7cdd9ae24055}

#### `public inline unsigned int `[`getHubPatch`](#class_udp_hub_listener_1a548db06a0e900048437d36a8816d05f3)`()` {#class_udp_hub_listener_1a548db06a0e900048437d36a8816d05f3}

#### `public inline void `[`setUnderRunMode`](#class_udp_hub_listener_1ad6f3f47b75bc35d1c287ebb8eb2c049a)`(`[`JackTrip::underrunModeT`](#class_jack_trip_1a9fd34b5de9751d9bafbb03a7c2250bf5)` UnderRunMode)` {#class_udp_hub_listener_1ad6f3f47b75bc35d1c287ebb8eb2c049a}

#### `public inline void `[`setBufferQueueLength`](#class_udp_hub_listener_1a8b17895aa1f3788b016a75b1ee0a4039)`(int BufferQueueLength)` {#class_udp_hub_listener_1a8b17895aa1f3788b016a75b1ee0a4039}

#### `public inline void `[`setIOStatTimeout`](#class_udp_hub_listener_1a67491c6165e0f4e3f658e3e3a69c8a53)`(int timeout)` {#class_udp_hub_listener_1a67491c6165e0f4e3f658e3e3a69c8a53}

#### `public inline void `[`setIOStatStream`](#class_udp_hub_listener_1a990ad20982069e2392482e57a25b6700)`(QSharedPointer< std::ofstream > statStream)` {#class_udp_hub_listener_1a990ad20982069e2392482e57a25b6700}

#### `public inline void `[`setBufferStrategy`](#class_udp_hub_listener_1a05ef98435e7f56e7e0f89c8e14e850b3)`(int BufferStrategy)` {#class_udp_hub_listener_1a05ef98435e7f56e7e0f89c8e14e850b3}

#### `public inline void `[`setNetIssuesSimulation`](#class_udp_hub_listener_1a3a044b4e620511457bfa10ea083c9e04)`(double loss,double jitter,double delay_rel)` {#class_udp_hub_listener_1a3a044b4e620511457bfa10ea083c9e04}

#### `public inline void `[`setBroadcast`](#class_udp_hub_listener_1a1da0df938d4e39b1ce05097b5ae20987)`(int broadcast_queue)` {#class_udp_hub_listener_1a1da0df938d4e39b1ce05097b5ae20987}

#### `public inline void `[`setUseRtUdpPriority`](#class_udp_hub_listener_1a070b0d78c8f656523af2456d7a0c0e7b)`(bool use)` {#class_udp_hub_listener_1a070b0d78c8f656523af2456d7a0c0e7b}

#### `{signal} public void `[`Listening`](#class_udp_hub_listener_1a5af2f5a3e2859802f95c6dd7b552761e)`()` {#class_udp_hub_listener_1a5af2f5a3e2859802f95c6dd7b552761e}

#### `{signal} public void `[`ClientAddressSet`](#class_udp_hub_listener_1ab58bf3699e2ae0f0a1845e5f2a983003)`()` {#class_udp_hub_listener_1ab58bf3699e2ae0f0a1845e5f2a983003}

#### `{signal} public void `[`signalRemoveThread`](#class_udp_hub_listener_1ace9855059fb7d305e208e658aa8ac502)`(int id)` {#class_udp_hub_listener_1ace9855059fb7d305e208e658aa8ac502}

#### `{signal} public void `[`signalStopped`](#class_udp_hub_listener_1a81493e16c1c13915dd7e917fe45a7313)`()` {#class_udp_hub_listener_1a81493e16c1c13915dd7e917fe45a7313}

#### `{signal} public void `[`signalError`](#class_udp_hub_listener_1ab66da45c363859869526b3406a050394)`(const QString & errorMessage)` {#class_udp_hub_listener_1ab66da45c363859869526b3406a050394}

# class `UpdatableValueConverter` {#class_updatable_value_converter}

```
class UpdatableValueConverter
  : public ValueConverter
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`UpdatableValueConverter`](#class_updatable_value_converter_1a65a9e3d1aaf236bdcc59c7c2469c015c)`()` | 
`public inline virtual  `[`~UpdatableValueConverter`](#class_updatable_value_converter_1a2ebbb70c18c40a3dd40809bdb7c023a6)`()` | 
`public void `[`setMappingValues`](#class_updatable_value_converter_1a21b03b0ea0622496d1c285e2f8d91978)`(double amin,double amid,double amax,double min,double init,double max)` | 
`public void `[`getMappingValues`](#class_updatable_value_converter_1a685d6c80fbb2d5ab50458b7962761c18)`(double & amin,double & amid,double & amax)` | 
`public inline void `[`setActive`](#class_updatable_value_converter_1af25b7ca086968d1266f9c9d094172107)`(bool on_off)` | 
`public inline bool `[`getActive`](#class_updatable_value_converter_1a1099b0161d2e63d26e3a0f6471699ab2)`()` | 
`protected bool `[`fActive`](#class_updatable_value_converter_1affd9030cd31b011b105ed3efda071502) | 

## Members

#### `public inline  `[`UpdatableValueConverter`](#class_updatable_value_converter_1a65a9e3d1aaf236bdcc59c7c2469c015c)`()` {#class_updatable_value_converter_1a65a9e3d1aaf236bdcc59c7c2469c015c}

#### `public inline virtual  `[`~UpdatableValueConverter`](#class_updatable_value_converter_1a2ebbb70c18c40a3dd40809bdb7c023a6)`()` {#class_updatable_value_converter_1a2ebbb70c18c40a3dd40809bdb7c023a6}

#### `public void `[`setMappingValues`](#class_updatable_value_converter_1a21b03b0ea0622496d1c285e2f8d91978)`(double amin,double amid,double amax,double min,double init,double max)` {#class_updatable_value_converter_1a21b03b0ea0622496d1c285e2f8d91978}

#### `public void `[`getMappingValues`](#class_updatable_value_converter_1a685d6c80fbb2d5ab50458b7962761c18)`(double & amin,double & amid,double & amax)` {#class_updatable_value_converter_1a685d6c80fbb2d5ab50458b7962761c18}

#### `public inline void `[`setActive`](#class_updatable_value_converter_1af25b7ca086968d1266f9c9d094172107)`(bool on_off)` {#class_updatable_value_converter_1af25b7ca086968d1266f9c9d094172107}

#### `public inline bool `[`getActive`](#class_updatable_value_converter_1a1099b0161d2e63d26e3a0f6471699ab2)`()` {#class_updatable_value_converter_1a1099b0161d2e63d26e3a0f6471699ab2}

#### `protected bool `[`fActive`](#class_updatable_value_converter_1affd9030cd31b011b105ed3efda071502) {#class_updatable_value_converter_1affd9030cd31b011b105ed3efda071502}

# class `ValueConverter` {#class_value_converter}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline virtual  `[`~ValueConverter`](#class_value_converter_1a25f2f089177c9ba3c95055e766aa26ed)`()` | 
`public double `[`ui2faust`](#class_value_converter_1a58aa2b4a002df8bfd601e7863373a787)`(double x)` | 
`public double `[`faust2ui`](#class_value_converter_1ac16329cecdf3d517c5007e2c648ed5ef)`(double x)` | 

## Members

#### `public inline virtual  `[`~ValueConverter`](#class_value_converter_1a25f2f089177c9ba3c95055e766aa26ed)`()` {#class_value_converter_1a25f2f089177c9ba3c95055e766aa26ed}

#### `public double `[`ui2faust`](#class_value_converter_1a58aa2b4a002df8bfd601e7863373a787)`(double x)` {#class_value_converter_1a58aa2b4a002df8bfd601e7863373a787}

#### `public double `[`faust2ui`](#class_value_converter_1ac16329cecdf3d517c5007e2c648ed5ef)`(double x)` {#class_value_converter_1ac16329cecdf3d517c5007e2c648ed5ef}

# class `zitarevdsp` {#classzitarevdsp}

```
class zitarevdsp
  : public dsp
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline virtual void `[`metadata`](#classzitarevdsp_1a2d5413bdee012d1d9c57e85778846ff1)`(`[`Meta`](#struct_meta)` * m)` | Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.
`public inline virtual int `[`getNumInputs`](#classzitarevdsp_1a3e55c40692c04278d9f990239b4edec8)`()` | 
`public inline virtual int `[`getNumOutputs`](#classzitarevdsp_1ae463dd04f6e1af4daa43754a94b0016a)`()` | 
`public inline virtual int `[`getInputRate`](#classzitarevdsp_1a9d1177840c4d25f5f9fc55034496dadc)`(int channel)` | 
`public inline virtual int `[`getOutputRate`](#classzitarevdsp_1a27184117f4290339693571874ee70fb0)`(int channel)` | 
`public inline virtual void `[`instanceConstants`](#classzitarevdsp_1aff41624ac754f2cccfbd34d1335e8de7)`(int sample_rate)` | Init instance constant state
`public inline virtual void `[`instanceResetUserInterface`](#classzitarevdsp_1a01ebf811dc225aa172d0eed02da17e18)`()` | 
`public inline virtual void `[`instanceClear`](#classzitarevdsp_1aa1a6c841a54e57415b7767e9a2a938c1)`()` | 
`public inline virtual void `[`init`](#classzitarevdsp_1a5ce3e7e6e1fbd061449fba5bbc9dc99b)`(int sample_rate)` | Global init, calls the following methods:
`public inline virtual void `[`instanceInit`](#classzitarevdsp_1a7d2e9e8536dedfbfdaa1df970f5054ac)`(int sample_rate)` | Init instance state
`public inline virtual `[`zitarevdsp`](#classzitarevdsp)` * `[`clone`](#classzitarevdsp_1aaa456610a80331cb069ae994daa8cc47)`()` | Return a clone of the instance.
`public inline virtual int `[`getSampleRate`](#classzitarevdsp_1ae31a7ddae9b65e9f56b5d7a45c63c317)`()` | 
`public inline virtual void `[`buildUserInterface`](#classzitarevdsp_1ae2f20d2523aa4d5c4ee1232f1b5f3e06)`(`[`UI`](#struct_u_i)` * ui_interface)` | Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).
`public inline virtual void `[`compute`](#classzitarevdsp_1a9303d13aaaab87c63ff8a09a306c1d6e)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` | DSP instance computation, to be called with successive in/out audio buffers.

## Members

#### `public inline virtual void `[`metadata`](#classzitarevdsp_1a2d5413bdee012d1d9c57e85778846ff1)`(`[`Meta`](#struct_meta)` * m)` {#classzitarevdsp_1a2d5413bdee012d1d9c57e85778846ff1}

Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.

#### Parameters
* `m` - the Meta* meta user

#### `public inline virtual int `[`getNumInputs`](#classzitarevdsp_1a3e55c40692c04278d9f990239b4edec8)`()` {#classzitarevdsp_1a3e55c40692c04278d9f990239b4edec8}

#### `public inline virtual int `[`getNumOutputs`](#classzitarevdsp_1ae463dd04f6e1af4daa43754a94b0016a)`()` {#classzitarevdsp_1ae463dd04f6e1af4daa43754a94b0016a}

#### `public inline virtual int `[`getInputRate`](#classzitarevdsp_1a9d1177840c4d25f5f9fc55034496dadc)`(int channel)` {#classzitarevdsp_1a9d1177840c4d25f5f9fc55034496dadc}

#### `public inline virtual int `[`getOutputRate`](#classzitarevdsp_1a27184117f4290339693571874ee70fb0)`(int channel)` {#classzitarevdsp_1a27184117f4290339693571874ee70fb0}

#### `public inline virtual void `[`instanceConstants`](#classzitarevdsp_1aff41624ac754f2cccfbd34d1335e8de7)`(int sample_rate)` {#classzitarevdsp_1aff41624ac754f2cccfbd34d1335e8de7}

Init instance constant state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual void `[`instanceResetUserInterface`](#classzitarevdsp_1a01ebf811dc225aa172d0eed02da17e18)`()` {#classzitarevdsp_1a01ebf811dc225aa172d0eed02da17e18}

#### `public inline virtual void `[`instanceClear`](#classzitarevdsp_1aa1a6c841a54e57415b7767e9a2a938c1)`()` {#classzitarevdsp_1aa1a6c841a54e57415b7767e9a2a938c1}

#### `public inline virtual void `[`init`](#classzitarevdsp_1a5ce3e7e6e1fbd061449fba5bbc9dc99b)`(int sample_rate)` {#classzitarevdsp_1a5ce3e7e6e1fbd061449fba5bbc9dc99b}

Global init, calls the following methods:

* static class 'classInit': static tables initialization

* 'instanceInit': constants and instance state initialization

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual void `[`instanceInit`](#classzitarevdsp_1a7d2e9e8536dedfbfdaa1df970f5054ac)`(int sample_rate)` {#classzitarevdsp_1a7d2e9e8536dedfbfdaa1df970f5054ac}

Init instance state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual `[`zitarevdsp`](#classzitarevdsp)` * `[`clone`](#classzitarevdsp_1aaa456610a80331cb069ae994daa8cc47)`()` {#classzitarevdsp_1aaa456610a80331cb069ae994daa8cc47}

Return a clone of the instance.

#### Returns
a copy of the instance on success, otherwise a null pointer.

#### `public inline virtual int `[`getSampleRate`](#classzitarevdsp_1ae31a7ddae9b65e9f56b5d7a45c63c317)`()` {#classzitarevdsp_1ae31a7ddae9b65e9f56b5d7a45c63c317}

#### `public inline virtual void `[`buildUserInterface`](#classzitarevdsp_1ae2f20d2523aa4d5c4ee1232f1b5f3e06)`(`[`UI`](#struct_u_i)` * ui_interface)` {#classzitarevdsp_1ae2f20d2523aa4d5c4ee1232f1b5f3e06}

Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).

#### Parameters
* `ui_interface` - the user interface builder

#### `public inline virtual void `[`compute`](#classzitarevdsp_1a9303d13aaaab87c63ff8a09a306c1d6e)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` {#classzitarevdsp_1a9303d13aaaab87c63ff8a09a306c1d6e}

DSP instance computation, to be called with successive in/out audio buffers.

#### Parameters
* `count` - the number of frames to compute 

* `inputs` - the input audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad) 

* `outputs` - the output audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad)

# class `zitarevmonodsp` {#classzitarevmonodsp}

```
class zitarevmonodsp
  : public dsp
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline virtual void `[`metadata`](#classzitarevmonodsp_1a8360796e41bf463f12919ce402889011)`(`[`Meta`](#struct_meta)` * m)` | Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.
`public inline virtual int `[`getNumInputs`](#classzitarevmonodsp_1a5ddb2afff6d21da50bb6f5af69b464e5)`()` | 
`public inline virtual int `[`getNumOutputs`](#classzitarevmonodsp_1afa88b60fb54fd350cd3da13ea696a949)`()` | 
`public inline virtual int `[`getInputRate`](#classzitarevmonodsp_1a1b2b764e6bf432d7892140b93b331e0c)`(int channel)` | 
`public inline virtual int `[`getOutputRate`](#classzitarevmonodsp_1acef3be28d85fe7e5e6f48b8cef6abec9)`(int channel)` | 
`public inline virtual void `[`instanceConstants`](#classzitarevmonodsp_1a3b9e8b57c493dc5748e496f67c6583ce)`(int sample_rate)` | Init instance constant state
`public inline virtual void `[`instanceResetUserInterface`](#classzitarevmonodsp_1ad33be90d5b183290e56d2caa9ae2e9cb)`()` | 
`public inline virtual void `[`instanceClear`](#classzitarevmonodsp_1a10a931a6b468901f608c7bd8e2858727)`()` | 
`public inline virtual void `[`init`](#classzitarevmonodsp_1a741fe12d77b15be779d0a61763274465)`(int sample_rate)` | Global init, calls the following methods:
`public inline virtual void `[`instanceInit`](#classzitarevmonodsp_1a679d34cbd124648c57a04a5e77f5b8e0)`(int sample_rate)` | Init instance state
`public inline virtual `[`zitarevmonodsp`](#classzitarevmonodsp)` * `[`clone`](#classzitarevmonodsp_1aca2596f29bbfef371fe16c3b62371b57)`()` | Return a clone of the instance.
`public inline virtual int `[`getSampleRate`](#classzitarevmonodsp_1a31b22da14be0889b30eda92cbe8c0e25)`()` | 
`public inline virtual void `[`buildUserInterface`](#classzitarevmonodsp_1a3892ee2445000af644638670e2c4c823)`(`[`UI`](#struct_u_i)` * ui_interface)` | Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).
`public inline virtual void `[`compute`](#classzitarevmonodsp_1a95a94011743d35f1e55f8d1a4d3676b0)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` | DSP instance computation, to be called with successive in/out audio buffers.

## Members

#### `public inline virtual void `[`metadata`](#classzitarevmonodsp_1a8360796e41bf463f12919ce402889011)`(`[`Meta`](#struct_meta)` * m)` {#classzitarevmonodsp_1a8360796e41bf463f12919ce402889011}

Trigger the Meta* parameter with instance specific calls to 'declare' (key, value) metadata.

#### Parameters
* `m` - the Meta* meta user

#### `public inline virtual int `[`getNumInputs`](#classzitarevmonodsp_1a5ddb2afff6d21da50bb6f5af69b464e5)`()` {#classzitarevmonodsp_1a5ddb2afff6d21da50bb6f5af69b464e5}

#### `public inline virtual int `[`getNumOutputs`](#classzitarevmonodsp_1afa88b60fb54fd350cd3da13ea696a949)`()` {#classzitarevmonodsp_1afa88b60fb54fd350cd3da13ea696a949}

#### `public inline virtual int `[`getInputRate`](#classzitarevmonodsp_1a1b2b764e6bf432d7892140b93b331e0c)`(int channel)` {#classzitarevmonodsp_1a1b2b764e6bf432d7892140b93b331e0c}

#### `public inline virtual int `[`getOutputRate`](#classzitarevmonodsp_1acef3be28d85fe7e5e6f48b8cef6abec9)`(int channel)` {#classzitarevmonodsp_1acef3be28d85fe7e5e6f48b8cef6abec9}

#### `public inline virtual void `[`instanceConstants`](#classzitarevmonodsp_1a3b9e8b57c493dc5748e496f67c6583ce)`(int sample_rate)` {#classzitarevmonodsp_1a3b9e8b57c493dc5748e496f67c6583ce}

Init instance constant state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual void `[`instanceResetUserInterface`](#classzitarevmonodsp_1ad33be90d5b183290e56d2caa9ae2e9cb)`()` {#classzitarevmonodsp_1ad33be90d5b183290e56d2caa9ae2e9cb}

#### `public inline virtual void `[`instanceClear`](#classzitarevmonodsp_1a10a931a6b468901f608c7bd8e2858727)`()` {#classzitarevmonodsp_1a10a931a6b468901f608c7bd8e2858727}

#### `public inline virtual void `[`init`](#classzitarevmonodsp_1a741fe12d77b15be779d0a61763274465)`(int sample_rate)` {#classzitarevmonodsp_1a741fe12d77b15be779d0a61763274465}

Global init, calls the following methods:

* static class 'classInit': static tables initialization

* 'instanceInit': constants and instance state initialization

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual void `[`instanceInit`](#classzitarevmonodsp_1a679d34cbd124648c57a04a5e77f5b8e0)`(int sample_rate)` {#classzitarevmonodsp_1a679d34cbd124648c57a04a5e77f5b8e0}

Init instance state

#### Parameters
* `sample_rate` - the sampling rate in Hertz

#### `public inline virtual `[`zitarevmonodsp`](#classzitarevmonodsp)` * `[`clone`](#classzitarevmonodsp_1aca2596f29bbfef371fe16c3b62371b57)`()` {#classzitarevmonodsp_1aca2596f29bbfef371fe16c3b62371b57}

Return a clone of the instance.

#### Returns
a copy of the instance on success, otherwise a null pointer.

#### `public inline virtual int `[`getSampleRate`](#classzitarevmonodsp_1a31b22da14be0889b30eda92cbe8c0e25)`()` {#classzitarevmonodsp_1a31b22da14be0889b30eda92cbe8c0e25}

#### `public inline virtual void `[`buildUserInterface`](#classzitarevmonodsp_1a3892ee2445000af644638670e2c4c823)`(`[`UI`](#struct_u_i)` * ui_interface)` {#classzitarevmonodsp_1a3892ee2445000af644638670e2c4c823}

Trigger the ui_interface parameter with instance specific calls to 'openTabBox', 'addButton', 'addVerticalSlider'... in order to build the [UI](#struct_u_i).

#### Parameters
* `ui_interface` - the user interface builder

#### `public inline virtual void `[`compute`](#classzitarevmonodsp_1a95a94011743d35f1e55f8d1a4d3676b0)`(int count,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** inputs,`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` ** outputs)` {#classzitarevmonodsp_1a95a94011743d35f1e55f8d1a4d3676b0}

DSP instance computation, to be called with successive in/out audio buffers.

#### Parameters
* `count` - the number of frames to compute 

* `inputs` - the input audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad) 

* `outputs` - the output audio buffers as an array of non-interleaved FAUSTFLOAT samples (eiher float, double or quad)

# class `ZoneControl` {#class_zone_control}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`ZoneControl`](#class_zone_control_1a3a9dedad0c6c86baf3367a05f60c6ee3)`(`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone)` | 
`public inline virtual  `[`~ZoneControl`](#class_zone_control_1a7cc439b10d68b5913eaab778023443ce)`()` | 
`public inline virtual void `[`update`](#class_zone_control_1a81c0645a40c74944b6eb29f312739cce)`(double v) const` | 
`public inline virtual void `[`setMappingValues`](#class_zone_control_1a401f790e5cb7531c2da371226d120a51)`(int curve,double amin,double amid,double amax,double min,double init,double max)` | 
`public inline virtual void `[`getMappingValues`](#class_zone_control_1a59fab347f1d9fd7eac07afdef4daaadf)`(double & amin,double & amid,double & amax)` | 
`public inline `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * `[`getZone`](#class_zone_control_1ab934a0225548889fbd5effcc86ff48eb)`()` | 
`public inline virtual void `[`setActive`](#class_zone_control_1aaa72f04058a8a3ff2fcf7e316b6b4a3d)`(bool on_off)` | 
`public inline virtual bool `[`getActive`](#class_zone_control_1a45b48b429a156f749b053398d0607038)`()` | 
`public inline virtual int `[`getCurve`](#class_zone_control_1a2a36e605f4501f1f32913f0431502155)`()` | 
`protected `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * `[`fZone`](#class_zone_control_1a49dc68b0dafb77ab7c5b575242e30012) | 

## Members

#### `public inline  `[`ZoneControl`](#class_zone_control_1a3a9dedad0c6c86baf3367a05f60c6ee3)`(`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone)` {#class_zone_control_1a3a9dedad0c6c86baf3367a05f60c6ee3}

#### `public inline virtual  `[`~ZoneControl`](#class_zone_control_1a7cc439b10d68b5913eaab778023443ce)`()` {#class_zone_control_1a7cc439b10d68b5913eaab778023443ce}

#### `public inline virtual void `[`update`](#class_zone_control_1a81c0645a40c74944b6eb29f312739cce)`(double v) const` {#class_zone_control_1a81c0645a40c74944b6eb29f312739cce}

#### `public inline virtual void `[`setMappingValues`](#class_zone_control_1a401f790e5cb7531c2da371226d120a51)`(int curve,double amin,double amid,double amax,double min,double init,double max)` {#class_zone_control_1a401f790e5cb7531c2da371226d120a51}

#### `public inline virtual void `[`getMappingValues`](#class_zone_control_1a59fab347f1d9fd7eac07afdef4daaadf)`(double & amin,double & amid,double & amax)` {#class_zone_control_1a59fab347f1d9fd7eac07afdef4daaadf}

#### `public inline `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * `[`getZone`](#class_zone_control_1ab934a0225548889fbd5effcc86ff48eb)`()` {#class_zone_control_1ab934a0225548889fbd5effcc86ff48eb}

#### `public inline virtual void `[`setActive`](#class_zone_control_1aaa72f04058a8a3ff2fcf7e316b6b4a3d)`(bool on_off)` {#class_zone_control_1aaa72f04058a8a3ff2fcf7e316b6b4a3d}

#### `public inline virtual bool `[`getActive`](#class_zone_control_1a45b48b429a156f749b053398d0607038)`()` {#class_zone_control_1a45b48b429a156f749b053398d0607038}

#### `public inline virtual int `[`getCurve`](#class_zone_control_1a2a36e605f4501f1f32913f0431502155)`()` {#class_zone_control_1a2a36e605f4501f1f32913f0431502155}

#### `protected `[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * `[`fZone`](#class_zone_control_1a49dc68b0dafb77ab7c5b575242e30012) {#class_zone_control_1a49dc68b0dafb77ab7c5b575242e30012}

# class `ZoneReader` {#class_zone_reader}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`ZoneReader`](#class_zone_reader_1a250e6c5d0e8874e74fd0b06adecf28ee)`(`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,double lo,double hi)` | 
`public inline virtual  `[`~ZoneReader`](#class_zone_reader_1aff5d6b359941d97ff497a7c6539fa72e)`()` | 
`public inline int `[`getValue`](#class_zone_reader_1a07241e744976dc9886146e322702c6aa)`()` | 

## Members

#### `public inline  `[`ZoneReader`](#class_zone_reader_1a250e6c5d0e8874e74fd0b06adecf28ee)`(`[`FAUSTFLOAT`](#zitarevmonodsp_8h_1aff8d1306e22d8dab1fbd5d24477139f5)` * zone,double lo,double hi)` {#class_zone_reader_1a250e6c5d0e8874e74fd0b06adecf28ee}

#### `public inline virtual  `[`~ZoneReader`](#class_zone_reader_1aff5d6b359941d97ff497a7c6539fa72e)`()` {#class_zone_reader_1aff5d6b359941d97ff497a7c6539fa72e}

#### `public inline int `[`getValue`](#class_zone_reader_1a07241e744976dc9886146e322702c6aa)`()` {#class_zone_reader_1a07241e744976dc9886146e322702c6aa}

# struct `addressPortPair` {#structaddress_port_pair}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public QString `[`address`](#structaddress_port_pair_1a6c59ccf241224dac4c291f14965d0a5e) | 
`public `[`int16_t`](#jacktrip__types_8h_1a8f7d0681af2149b9d906ee4526683824)` `[`port`](#structaddress_port_pair_1adc72311d924dbc945f6a2c2b10fe47c3) | 

## Members

#### `public QString `[`address`](#structaddress_port_pair_1a6c59ccf241224dac4c291f14965d0a5e) {#structaddress_port_pair_1a6c59ccf241224dac4c291f14965d0a5e}

#### `public `[`int16_t`](#jacktrip__types_8h_1a8f7d0681af2149b9d906ee4526683824)` `[`port`](#structaddress_port_pair_1adc72311d924dbc945f6a2c2b10fe47c3) {#structaddress_port_pair_1adc72311d924dbc945f6a2c2b10fe47c3}

# struct `CompressorPreset` {#struct_compressor_preset}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public float `[`ratio`](#struct_compressor_preset_1a32c46169b718286b27abad4bb0e49e65) | 
`public float `[`thresholdDB`](#struct_compressor_preset_1acb36a20f14fabff0952b0b070436ad97) | 
`public float `[`attackMS`](#struct_compressor_preset_1a29fdfb273caa89dc22a018c9241e73f4) | 
`public float `[`releaseMS`](#struct_compressor_preset_1abd9278ab0f57c69bee813b1d5e9cfc2e) | 
`public float `[`makeUpGainDB`](#struct_compressor_preset_1ada5aa4343c4749fd3eb02844dd6d0535) | 
`public inline  `[`CompressorPreset`](#struct_compressor_preset_1aac5dd4b9a020605d5c25d76d528089b8)`(float r,float t,float a,float rel,float m)` | 
`public  `[`~CompressorPreset`](#struct_compressor_preset_1a6c4a2929e4d82e1865dfbe7246940cf1)`() = default` | 

## Members

#### `public float `[`ratio`](#struct_compressor_preset_1a32c46169b718286b27abad4bb0e49e65) {#struct_compressor_preset_1a32c46169b718286b27abad4bb0e49e65}

#### `public float `[`thresholdDB`](#struct_compressor_preset_1acb36a20f14fabff0952b0b070436ad97) {#struct_compressor_preset_1acb36a20f14fabff0952b0b070436ad97}

#### `public float `[`attackMS`](#struct_compressor_preset_1a29fdfb273caa89dc22a018c9241e73f4) {#struct_compressor_preset_1a29fdfb273caa89dc22a018c9241e73f4}

#### `public float `[`releaseMS`](#struct_compressor_preset_1abd9278ab0f57c69bee813b1d5e9cfc2e) {#struct_compressor_preset_1abd9278ab0f57c69bee813b1d5e9cfc2e}

#### `public float `[`makeUpGainDB`](#struct_compressor_preset_1ada5aa4343c4749fd3eb02844dd6d0535) {#struct_compressor_preset_1ada5aa4343c4749fd3eb02844dd6d0535}

#### `public inline  `[`CompressorPreset`](#struct_compressor_preset_1aac5dd4b9a020605d5c25d76d528089b8)`(float r,float t,float a,float rel,float m)` {#struct_compressor_preset_1aac5dd4b9a020605d5c25d76d528089b8}

#### `public  `[`~CompressorPreset`](#struct_compressor_preset_1a6c4a2929e4d82e1865dfbe7246940cf1)`() = default` {#struct_compressor_preset_1a6c4a2929e4d82e1865dfbe7246940cf1}

# struct `DefaultHeaderStruct` {#struct_default_header_struct}

```
struct DefaultHeaderStruct
  : public HeaderStruct
```  

Default Header Struct.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public uint64_t `[`TimeStamp`](#struct_default_header_struct_1aec92284c78e729f57e43dd6566a266a5) | Time Stamp.
`public `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`SeqNumber`](#struct_default_header_struct_1ab2efb238e0487b1d70c5daa56bbca5dd) | Sequence Number.
`public `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`BufferSize`](#struct_default_header_struct_1ab5fbb52ffb5b15005fe487c44627cb3d) | Buffer Size in Samples.
`public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`SamplingRate`](#struct_default_header_struct_1a3b9b80786cd9376c195a52e08aa366ed) | Sampling Rate in [JackAudioInterface::samplingRateT](#class_audio_interface_1a538ea04ceee63f418c629d6a3a0e4fef).
`public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`BitResolution`](#struct_default_header_struct_1a6636e8bc0c3673877178e66cb524e0c4) | Audio Bit Resolution.
`public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`NumChannels`](#struct_default_header_struct_1ac2f3794bb7d1d69c0f2f2a24813f3f19) | Number of Channels, we assume input and outputs are the same.
`public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`ConnectionMode`](#struct_default_header_struct_1a25e03ecdeab4acfbd8411383f7ab69fd) | 

## Members

#### `public uint64_t `[`TimeStamp`](#struct_default_header_struct_1aec92284c78e729f57e43dd6566a266a5) {#struct_default_header_struct_1aec92284c78e729f57e43dd6566a266a5}

Time Stamp.

#### `public `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`SeqNumber`](#struct_default_header_struct_1ab2efb238e0487b1d70c5daa56bbca5dd) {#struct_default_header_struct_1ab2efb238e0487b1d70c5daa56bbca5dd}

Sequence Number.

#### `public `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`BufferSize`](#struct_default_header_struct_1ab5fbb52ffb5b15005fe487c44627cb3d) {#struct_default_header_struct_1ab5fbb52ffb5b15005fe487c44627cb3d}

Buffer Size in Samples.

#### `public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`SamplingRate`](#struct_default_header_struct_1a3b9b80786cd9376c195a52e08aa366ed) {#struct_default_header_struct_1a3b9b80786cd9376c195a52e08aa366ed}

Sampling Rate in [JackAudioInterface::samplingRateT](#class_audio_interface_1a538ea04ceee63f418c629d6a3a0e4fef).

#### `public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`BitResolution`](#struct_default_header_struct_1a6636e8bc0c3673877178e66cb524e0c4) {#struct_default_header_struct_1a6636e8bc0c3673877178e66cb524e0c4}

Audio Bit Resolution.

#### `public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`NumChannels`](#struct_default_header_struct_1ac2f3794bb7d1d69c0f2f2a24813f3f19) {#struct_default_header_struct_1ac2f3794bb7d1d69c0f2f2a24813f3f19}

Number of Channels, we assume input and outputs are the same.

#### `public `[`uint8_t`](#jacktrip__types_8h_1a8d0d6f8b52c7a42ec879e5b1b57aa8d0)` `[`ConnectionMode`](#struct_default_header_struct_1a25e03ecdeab4acfbd8411383f7ab69fd) {#struct_default_header_struct_1a25e03ecdeab4acfbd8411383f7ab69fd}

# struct `dsp_memory_manager` {#structdsp__memory__manager}

DSP memory manager.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline virtual  `[`~dsp_memory_manager`](#structdsp__memory__manager_1a2be116e6d1693b4b7dade86dce387a17)`()` | 
`public void * `[`allocate`](#structdsp__memory__manager_1a4266efbb5df86b8b0d60eab4daa57bde)`(size_t size)` | 
`public void `[`destroy`](#structdsp__memory__manager_1a58349e8177315a3e79576856059c59c6)`(void * ptr)` | 

## Members

#### `public inline virtual  `[`~dsp_memory_manager`](#structdsp__memory__manager_1a2be116e6d1693b4b7dade86dce387a17)`()` {#structdsp__memory__manager_1a2be116e6d1693b4b7dade86dce387a17}

#### `public void * `[`allocate`](#structdsp__memory__manager_1a4266efbb5df86b8b0d60eab4daa57bde)`(size_t size)` {#structdsp__memory__manager_1a4266efbb5df86b8b0d60eab4daa57bde}

#### `public void `[`destroy`](#structdsp__memory__manager_1a58349e8177315a3e79576856059c59c6)`(void * ptr)` {#structdsp__memory__manager_1a58349e8177315a3e79576856059c59c6}

# struct `HeaderStruct` {#struct_header_struct}

Abstract Header Struct, Header Stucts should subclass it.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------

## Members

# struct `RingBuffer::IOStat` {#struct_ring_buffer_1_1_i_o_stat}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`underruns`](#struct_ring_buffer_1_1_i_o_stat_1acf98d33d0db7ecffa0a4d2aaf65fc30e) | 
`public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`overflows`](#struct_ring_buffer_1_1_i_o_stat_1a096751a5b50bb64933c19807aade0afd) | 
`public `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`skew`](#struct_ring_buffer_1_1_i_o_stat_1a57e8b5fbfd077d10a0c10fae15ad92ba) | 
`public `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`skew_raw`](#struct_ring_buffer_1_1_i_o_stat_1a1a1b635ec190a4e5c69263d5765e32c5) | 
`public `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`level`](#struct_ring_buffer_1_1_i_o_stat_1afd242688f9162d81083e296eb0b24336) | 
`public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`buf_dec_overflows`](#struct_ring_buffer_1_1_i_o_stat_1af832931f7d67951cfa544b166734c92e) | 
`public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`buf_dec_pktloss`](#struct_ring_buffer_1_1_i_o_stat_1ac1aea71735d9c307a74548593e8d47dc) | 
`public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`buf_inc_underrun`](#struct_ring_buffer_1_1_i_o_stat_1add265a46130e56322f1a71e5730e73ac) | 
`public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`buf_inc_compensate`](#struct_ring_buffer_1_1_i_o_stat_1a46f70db88795560eb25eeb88bfc450d2) | 
`public `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`broadcast_skew`](#struct_ring_buffer_1_1_i_o_stat_1ae21aacd3dce893269cb3b6e1a8dc2782) | 
`public `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`broadcast_delta`](#struct_ring_buffer_1_1_i_o_stat_1aa379d04331d3f3d1421cfbcd87272184) | 
`public `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`autoq_corr`](#struct_ring_buffer_1_1_i_o_stat_1ac067bd3da569a7ba8713df9312136572) | 
`public `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`autoq_rate`](#struct_ring_buffer_1_1_i_o_stat_1af59993d9eb08568cb5026edc8922c1b0) | 

## Members

#### `public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`underruns`](#struct_ring_buffer_1_1_i_o_stat_1acf98d33d0db7ecffa0a4d2aaf65fc30e) {#struct_ring_buffer_1_1_i_o_stat_1acf98d33d0db7ecffa0a4d2aaf65fc30e}

#### `public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`overflows`](#struct_ring_buffer_1_1_i_o_stat_1a096751a5b50bb64933c19807aade0afd) {#struct_ring_buffer_1_1_i_o_stat_1a096751a5b50bb64933c19807aade0afd}

#### `public `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`skew`](#struct_ring_buffer_1_1_i_o_stat_1a57e8b5fbfd077d10a0c10fae15ad92ba) {#struct_ring_buffer_1_1_i_o_stat_1a57e8b5fbfd077d10a0c10fae15ad92ba}

#### `public `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`skew_raw`](#struct_ring_buffer_1_1_i_o_stat_1a1a1b635ec190a4e5c69263d5765e32c5) {#struct_ring_buffer_1_1_i_o_stat_1a1a1b635ec190a4e5c69263d5765e32c5}

#### `public `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`level`](#struct_ring_buffer_1_1_i_o_stat_1afd242688f9162d81083e296eb0b24336) {#struct_ring_buffer_1_1_i_o_stat_1afd242688f9162d81083e296eb0b24336}

#### `public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`buf_dec_overflows`](#struct_ring_buffer_1_1_i_o_stat_1af832931f7d67951cfa544b166734c92e) {#struct_ring_buffer_1_1_i_o_stat_1af832931f7d67951cfa544b166734c92e}

#### `public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`buf_dec_pktloss`](#struct_ring_buffer_1_1_i_o_stat_1ac1aea71735d9c307a74548593e8d47dc) {#struct_ring_buffer_1_1_i_o_stat_1ac1aea71735d9c307a74548593e8d47dc}

#### `public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`buf_inc_underrun`](#struct_ring_buffer_1_1_i_o_stat_1add265a46130e56322f1a71e5730e73ac) {#struct_ring_buffer_1_1_i_o_stat_1add265a46130e56322f1a71e5730e73ac}

#### `public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`buf_inc_compensate`](#struct_ring_buffer_1_1_i_o_stat_1a46f70db88795560eb25eeb88bfc450d2) {#struct_ring_buffer_1_1_i_o_stat_1a46f70db88795560eb25eeb88bfc450d2}

#### `public `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`broadcast_skew`](#struct_ring_buffer_1_1_i_o_stat_1ae21aacd3dce893269cb3b6e1a8dc2782) {#struct_ring_buffer_1_1_i_o_stat_1ae21aacd3dce893269cb3b6e1a8dc2782}

#### `public `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`broadcast_delta`](#struct_ring_buffer_1_1_i_o_stat_1aa379d04331d3f3d1421cfbcd87272184) {#struct_ring_buffer_1_1_i_o_stat_1aa379d04331d3f3d1421cfbcd87272184}

#### `public `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`autoq_corr`](#struct_ring_buffer_1_1_i_o_stat_1ac067bd3da569a7ba8713df9312136572) {#struct_ring_buffer_1_1_i_o_stat_1ac067bd3da569a7ba8713df9312136572}

#### `public `[`int32_t`](#jacktrip__types_8h_1a0f27bfce9035e3b925320d9c2e4e36a1)` `[`autoq_rate`](#struct_ring_buffer_1_1_i_o_stat_1af59993d9eb08568cb5026edc8922c1b0) {#struct_ring_buffer_1_1_i_o_stat_1af59993d9eb08568cb5026edc8922c1b0}

# struct `JamLinkHeaderStuct` {#struct_jam_link_header_stuct}

```
struct JamLinkHeaderStuct
  : public HeaderStruct
```  

JamLink Header Struct.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`Common`](#struct_jam_link_header_stuct_1adef1d9aa0aa64ac506c76f68e4b99d94) | Common part of the header, 16 bit.
`public `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`SeqNumber`](#struct_jam_link_header_stuct_1a046addc1707ca9af63f6d7f1d998e18e) | Sequence Number.
`public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`TimeStamp`](#struct_jam_link_header_stuct_1ac352a9d47ca0298ac1e8ba683fabcc72) | Time Stamp.

## Members

#### `public `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`Common`](#struct_jam_link_header_stuct_1adef1d9aa0aa64ac506c76f68e4b99d94) {#struct_jam_link_header_stuct_1adef1d9aa0aa64ac506c76f68e4b99d94}

Common part of the header, 16 bit.

#### `public `[`uint16_t`](#jacktrip__types_8h_1a8fe5c1826d06c4678bd9f3064331fd47)` `[`SeqNumber`](#struct_jam_link_header_stuct_1a046addc1707ca9af63f6d7f1d998e18e) {#struct_jam_link_header_stuct_1a046addc1707ca9af63f6d7f1d998e18e}

Sequence Number.

#### `public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`TimeStamp`](#struct_jam_link_header_stuct_1ac352a9d47ca0298ac1e8ba683fabcc72) {#struct_jam_link_header_stuct_1ac352a9d47ca0298ac1e8ba683fabcc72}

Time Stamp.

# struct `Meta` {#struct_meta}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline virtual  `[`~Meta`](#struct_meta_1a878f0a44496d8b6881c17b48af29bbf9)`()` | 
`public void `[`declare`](#struct_meta_1a0563c19d121accc3bccd0dba98bc5815)`(const char * key,const char * value)` | 

## Members

#### `public inline virtual  `[`~Meta`](#struct_meta_1a878f0a44496d8b6881c17b48af29bbf9)`()` {#struct_meta_1a878f0a44496d8b6881c17b48af29bbf9}

#### `public void `[`declare`](#struct_meta_1a0563c19d121accc3bccd0dba98bc5815)`(const char * key,const char * value)` {#struct_meta_1a0563c19d121accc3bccd0dba98bc5815}

# struct `DataProtocol::PktStat` {#struct_data_protocol_1_1_pkt_stat}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`tot`](#struct_data_protocol_1_1_pkt_stat_1ae0c66c92bfbd0ac83d2b0a7e8e01168a) | 
`public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`lost`](#struct_data_protocol_1_1_pkt_stat_1ab9bf993d8fdfc3711dcc9cfd77d3dda6) | 
`public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`outOfOrder`](#struct_data_protocol_1_1_pkt_stat_1a031dd3180e03deb8fb4733c3c5b60d1e) | 
`public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`revived`](#struct_data_protocol_1_1_pkt_stat_1a2a550972aa3fb70cf6d81a075f8915ae) | 
`public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`statCount`](#struct_data_protocol_1_1_pkt_stat_1a85f9e2c4a260c6581180c3aa2b574c8e) | 

## Members

#### `public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`tot`](#struct_data_protocol_1_1_pkt_stat_1ae0c66c92bfbd0ac83d2b0a7e8e01168a) {#struct_data_protocol_1_1_pkt_stat_1ae0c66c92bfbd0ac83d2b0a7e8e01168a}

#### `public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`lost`](#struct_data_protocol_1_1_pkt_stat_1ab9bf993d8fdfc3711dcc9cfd77d3dda6) {#struct_data_protocol_1_1_pkt_stat_1ab9bf993d8fdfc3711dcc9cfd77d3dda6}

#### `public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`outOfOrder`](#struct_data_protocol_1_1_pkt_stat_1a031dd3180e03deb8fb4733c3c5b60d1e) {#struct_data_protocol_1_1_pkt_stat_1a031dd3180e03deb8fb4733c3c5b60d1e}

#### `public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`revived`](#struct_data_protocol_1_1_pkt_stat_1a2a550972aa3fb70cf6d81a075f8915ae) {#struct_data_protocol_1_1_pkt_stat_1a2a550972aa3fb70cf6d81a075f8915ae}

#### `public `[`uint32_t`](#jacktrip__types_8h_1ac8f07382e3afafe21deb8a3f5d27d1a9)` `[`statCount`](#struct_data_protocol_1_1_pkt_stat_1a85f9e2c4a260c6581180c3aa2b574c8e) {#struct_data_protocol_1_1_pkt_stat_1a85f9e2c4a260c6581180c3aa2b574c8e}

# struct `Interpolator::Range` {#struct_interpolator_1_1_range}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public double `[`fLo`](#struct_interpolator_1_1_range_1a13d919f85e22485a332b4e34e6e96fe2) | 
`public double `[`fHi`](#struct_interpolator_1_1_range_1aff74551cc07879c5c69a49a03a19c3a2) | 
`public inline  `[`Range`](#struct_interpolator_1_1_range_1a8aa7ead6decff6d24ec3a0c3c8d0ced6)`(double x,double y)` | 
`public inline double `[`operator()`](#struct_interpolator_1_1_range_1abd6b01ad3353ecec00d1ea01fd9c1d7b)`(double x)` | 

## Members

#### `public double `[`fLo`](#struct_interpolator_1_1_range_1a13d919f85e22485a332b4e34e6e96fe2) {#struct_interpolator_1_1_range_1a13d919f85e22485a332b4e34e6e96fe2}

#### `public double `[`fHi`](#struct_interpolator_1_1_range_1aff74551cc07879c5c69a49a03a19c3a2) {#struct_interpolator_1_1_range_1aff74551cc07879c5c69a49a03a19c3a2}

#### `public inline  `[`Range`](#struct_interpolator_1_1_range_1a8aa7ead6decff6d24ec3a0c3c8d0ced6)`(double x,double y)` {#struct_interpolator_1_1_range_1a8aa7ead6decff6d24ec3a0c3c8d0ced6}

#### `public inline double `[`operator()`](#struct_interpolator_1_1_range_1abd6b01ad3353ecec00d1ea01fd9c1d7b)`(double x)` {#struct_interpolator_1_1_range_1abd6b01ad3353ecec00d1ea01fd9c1d7b}

# struct `UI` {#struct_u_i}

```
struct UI
  : public UIReal< FAUSTFLOAT >
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`UI`](#struct_u_i_1a675985a56b5e87ebdc8e5884b9f2ee09)`()` | 
`public inline virtual  `[`~UI`](#struct_u_i_1a47e7b6111faba049dfee4738d067cc42)`()` | 

## Members

#### `public inline  `[`UI`](#struct_u_i_1a675985a56b5e87ebdc8e5884b9f2ee09)`()` {#struct_u_i_1a675985a56b5e87ebdc8e5884b9f2ee09}

#### `public inline virtual  `[`~UI`](#struct_u_i_1a47e7b6111faba049dfee4738d067cc42)`()` {#struct_u_i_1a47e7b6111faba049dfee4738d067cc42}

# struct `UIReal` {#struct_u_i_real}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`UIReal`](#struct_u_i_real_1a5c51436b94adfe00b0a7432fb81fd0ef)`()` | 
`public inline virtual  `[`~UIReal`](#struct_u_i_real_1aeb29d6bdae3400ee225cfcba85f41091)`()` | 
`public void `[`openTabBox`](#struct_u_i_real_1ab7288377989254053f72fbda60de8d7a)`(const char * label)` | 
`public void `[`openHorizontalBox`](#struct_u_i_real_1af194ef646b4f3133e69da668a622ffea)`(const char * label)` | 
`public void `[`openVerticalBox`](#struct_u_i_real_1a002067d373fe958272044a035923d977)`(const char * label)` | 
`public void `[`closeBox`](#struct_u_i_real_1ac06dd55e1feb7da94eb36b12cfa421cd)`()` | 
`public void `[`addButton`](#struct_u_i_real_1a9216ce526b735e48714996ff471da228)`(const char * label,REAL * zone)` | 
`public void `[`addCheckButton`](#struct_u_i_real_1a4f64fa3c6543bb3746d1dc3c747014e6)`(const char * label,REAL * zone)` | 
`public void `[`addVerticalSlider`](#struct_u_i_real_1af54401b7660e325b08bab210ad9c12f3)`(const char * label,REAL * zone,REAL init,REAL min,REAL max,REAL step)` | 
`public void `[`addHorizontalSlider`](#struct_u_i_real_1a3fbb806c855c8ab349c6474c53e7494a)`(const char * label,REAL * zone,REAL init,REAL min,REAL max,REAL step)` | 
`public void `[`addNumEntry`](#struct_u_i_real_1a207443ca932abd221c6c30da8f6340a8)`(const char * label,REAL * zone,REAL init,REAL min,REAL max,REAL step)` | 
`public void `[`addHorizontalBargraph`](#struct_u_i_real_1a3d2ab578240f17435760860430290cd7)`(const char * label,REAL * zone,REAL min,REAL max)` | 
`public void `[`addVerticalBargraph`](#struct_u_i_real_1a5ae8c50f98aeb4877dbcff8031774fbe)`(const char * label,REAL * zone,REAL min,REAL max)` | 
`public void `[`addSoundfile`](#struct_u_i_real_1a2894416db38a4783119c5853baae8604)`(const char * label,const char * filename,Soundfile ** sf_zone)` | 
`public inline virtual void `[`declare`](#struct_u_i_real_1a68d5adb64c76480846904f015af8b8ae)`(REAL * zone,const char * key,const char * val)` | 

## Members

#### `public inline  `[`UIReal`](#struct_u_i_real_1a5c51436b94adfe00b0a7432fb81fd0ef)`()` {#struct_u_i_real_1a5c51436b94adfe00b0a7432fb81fd0ef}

#### `public inline virtual  `[`~UIReal`](#struct_u_i_real_1aeb29d6bdae3400ee225cfcba85f41091)`()` {#struct_u_i_real_1aeb29d6bdae3400ee225cfcba85f41091}

#### `public void `[`openTabBox`](#struct_u_i_real_1ab7288377989254053f72fbda60de8d7a)`(const char * label)` {#struct_u_i_real_1ab7288377989254053f72fbda60de8d7a}

#### `public void `[`openHorizontalBox`](#struct_u_i_real_1af194ef646b4f3133e69da668a622ffea)`(const char * label)` {#struct_u_i_real_1af194ef646b4f3133e69da668a622ffea}

#### `public void `[`openVerticalBox`](#struct_u_i_real_1a002067d373fe958272044a035923d977)`(const char * label)` {#struct_u_i_real_1a002067d373fe958272044a035923d977}

#### `public void `[`closeBox`](#struct_u_i_real_1ac06dd55e1feb7da94eb36b12cfa421cd)`()` {#struct_u_i_real_1ac06dd55e1feb7da94eb36b12cfa421cd}

#### `public void `[`addButton`](#struct_u_i_real_1a9216ce526b735e48714996ff471da228)`(const char * label,REAL * zone)` {#struct_u_i_real_1a9216ce526b735e48714996ff471da228}

#### `public void `[`addCheckButton`](#struct_u_i_real_1a4f64fa3c6543bb3746d1dc3c747014e6)`(const char * label,REAL * zone)` {#struct_u_i_real_1a4f64fa3c6543bb3746d1dc3c747014e6}

#### `public void `[`addVerticalSlider`](#struct_u_i_real_1af54401b7660e325b08bab210ad9c12f3)`(const char * label,REAL * zone,REAL init,REAL min,REAL max,REAL step)` {#struct_u_i_real_1af54401b7660e325b08bab210ad9c12f3}

#### `public void `[`addHorizontalSlider`](#struct_u_i_real_1a3fbb806c855c8ab349c6474c53e7494a)`(const char * label,REAL * zone,REAL init,REAL min,REAL max,REAL step)` {#struct_u_i_real_1a3fbb806c855c8ab349c6474c53e7494a}

#### `public void `[`addNumEntry`](#struct_u_i_real_1a207443ca932abd221c6c30da8f6340a8)`(const char * label,REAL * zone,REAL init,REAL min,REAL max,REAL step)` {#struct_u_i_real_1a207443ca932abd221c6c30da8f6340a8}

#### `public void `[`addHorizontalBargraph`](#struct_u_i_real_1a3d2ab578240f17435760860430290cd7)`(const char * label,REAL * zone,REAL min,REAL max)` {#struct_u_i_real_1a3d2ab578240f17435760860430290cd7}

#### `public void `[`addVerticalBargraph`](#struct_u_i_real_1a5ae8c50f98aeb4877dbcff8031774fbe)`(const char * label,REAL * zone,REAL min,REAL max)` {#struct_u_i_real_1a5ae8c50f98aeb4877dbcff8031774fbe}

#### `public void `[`addSoundfile`](#struct_u_i_real_1a2894416db38a4783119c5853baae8604)`(const char * label,const char * filename,Soundfile ** sf_zone)` {#struct_u_i_real_1a2894416db38a4783119c5853baae8604}

#### `public inline virtual void `[`declare`](#struct_u_i_real_1a68d5adb64c76480846904f015af8b8ae)`(REAL * zone,const char * key,const char * val)` {#struct_u_i_real_1a68d5adb64c76480846904f015af8b8ae}

Generated by [Moxygen](https://sourcey.com/moxygen)