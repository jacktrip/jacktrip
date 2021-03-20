# group `audio_interface` {#group__audio__interface}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`class `[`AudioInterface`](#class_audio_interface) | Base Class that provides an interface with audio.
`class `[`JackAudioInterface`](#class_jack_audio_interface) | Class that provides an interface with the Jack Audio Server.
`class `[`JMess`](#class_j_mess) | Class to save and load all jack client connections.
`class `[`RtAudioInterface`](#class_rt_audio_interface) | Base Class that provides an interface with RtAudio.

## class `AudioInterface` {#class_audio_interface}

Base Class that provides an interface with audio.

### Members

??? example "`public  `[`AudioInterface`](#class_audio_interface_1ae20746f51bee2913cece514d74cf6d54)`(`[`JackTrip`](main.md#class_jack_trip)` * jacktrip,int NumInChans,int NumOutChans,`[`AudioInterface::audioBitResolutionT`](#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b)` AudioBitResolution)` <br/>// The class constructor."
    #### Parameters
	* `jacktrip` Pointer to the [JackTrip](main.md#class_jack_trip) class that connects all classes (mediator) 

	* `NumInChans` Number of Input Channels 

	* `NumOutChans` Number of Output Channels 

	* `AudioBitResolution` Audio Sample Resolutions in bits

??? example "`public virtual  `[`~AudioInterface`](#class_audio_interface_1a985204462880bbacf04c04ce538d9963)`()` <br/>// The class destructor."
    

??? example "`public virtual void `[`setup`](#class_audio_interface_1a898f0033f9a6d6d60b5c2f847c53e07a)`()` <br/>// Setup the client. This function should be called just before."
    starting the audio processes, it will setup the audio client with the class parameters, like Sampling Rate, Packet Size, Bit Resolution, etc... Sub-classes should also call the parent method to ensure correct inizialization.

??? example "`public int `[`startProcess`](#class_audio_interface_1a4d3d0cd118384ed23e85f671ff4f43f6)`() const` <br/>// Tell the audio server that we are ready to roll. The process-callback will start running. This runs on its own thread."
    #### Returns
	0 on success, otherwise a non-zero error code

??? example "`public int `[`stopProcess`](#class_audio_interface_1a2b1ecf7a11e8a92619ab9b35d675b9c7)`() const` <br/>// Stops the process-callback thread."
    #### Returns
	0 on success, otherwise a non-zero error code

??? example "`public virtual void `[`broadcastCallback`](#class_audio_interface_1a8b9017ce11a9e0185e6208c64ae35915)`(QVarLengthArray< `[`sample_t`](types.md#group__types_1ga0f311a188f855236c67af74f04dd1574)` * > & mon_buffer,unsigned int n_frames)` <br/>// Process callback. Subclass should call this callback after obtaining the in_buffer and out_buffer pointers."
    #### Parameters
	* `in_buffer` Array of input audio samplers for each channel. The user is reponsible to check that each channel has n_frames samplers 

	* `in_buffer` Array of output audio samplers for each channel. The user is reponsible to check that each channel has n_frames samplers

??? example "`public virtual void `[`callback`](#class_audio_interface_1a5319176a25341ed14dfaae5f875375f2)`(QVarLengthArray< `[`sample_t`](types.md#group__types_1ga0f311a188f855236c67af74f04dd1574)` * > & in_buffer,QVarLengthArray< `[`sample_t`](types.md#group__types_1ga0f311a188f855236c67af74f04dd1574)` * > & out_buffer,unsigned int n_frames)` <br/>// "
    

??? example "`public virtual void `[`appendProcessPluginToNetwork`](#class_audio_interface_1a582bdaa7555473c7ccad02645b6c0c47)`(`[`ProcessPlugin`](effects.md#class_process_plugin)` * plugin)` <br/>// [appendProcessPluginToNetwork()](#class_audio_interface_1a582bdaa7555473c7ccad02645b6c0c47): Append a [ProcessPlugin](effects.md#class_process_plugin) for outgoing audio. The processing order equals order they were appended. This processing is in the [JackTrip](main.md#class_jack_trip) client before sending to the network."
    #### Parameters
	* `plugin` a [ProcessPlugin](effects.md#class_process_plugin) smart pointer. Create the object instance using something like:<br/>`std::tr1::shared_ptr<ProcessPluginName> loopback(new ProcessPluginName);`

??? example "`public virtual void `[`appendProcessPluginFromNetwork`](#class_audio_interface_1a0b84c7b4b1b12ca655bc830b9cc62635)`(`[`ProcessPlugin`](effects.md#class_process_plugin)` * plugin)` <br/>// [appendProcessPluginFromNetwork()](#class_audio_interface_1a0b84c7b4b1b12ca655bc830b9cc62635): Same as [appendProcessPluginToNetwork()](#class_audio_interface_1a582bdaa7555473c7ccad02645b6c0c47) except that these plugins operate on the audio received from the network (typically from a [JackTrip](main.md#class_jack_trip) server). The complete processing chain then looks like this: audio -> JACK -> [JackTrip](main.md#class_jack_trip) client -> processPlugin to network -> remote [JackTrip](main.md#class_jack_trip) server -> [JackTrip](main.md#class_jack_trip) client -> processPlugin from network -> JACK -> audio"
    

??? example "`public void `[`initPlugins`](#class_audio_interface_1af58b303ae4933dc6ffe0cd2e535b5354)`()` <br/>// [initPlugins()](#class_audio_interface_1af58b303ae4933dc6ffe0cd2e535b5354): Initialize all [ProcessPlugin](effects.md#class_process_plugin) modules. The audio sampling rate (mSampleRate) must be set at this time."
    

??? example "`public void `[`connectDefaultPorts`](#class_audio_interface_1abbb4f698d0863a038e8156208f917a5b)`()` <br/>// "
    

??? example "`public inline virtual void `[`setNumInputChannels`](#class_audio_interface_1abdc6a04f4be17b8c7a790f265e985107)`(int nchannels)` <br/>// "
    

??? example "`public inline virtual void `[`setNumOutputChannels`](#class_audio_interface_1a7e9f52c6685f2d24968ef4482dc53bcc)`(int nchannels)` <br/>// "
    

??? example "`public inline virtual void `[`setSampleRate`](#class_audio_interface_1af66437c4c3c541abf27248f5aff74754)`(`[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` sample_rate)` <br/>// "
    

??? example "`public inline virtual void `[`setDeviceID`](#class_audio_interface_1aeb34ea63c907e397f3f33463adedcfa3)`(`[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` device_id)` <br/>// "
    

??? example "`public inline virtual void `[`setBufferSizeInSamples`](#class_audio_interface_1a89d932ccf615dfd6477a87c2b97c437d)`(`[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` buf_size)` <br/>// "
    

??? example "`public void `[`setClientName`](#class_audio_interface_1ae5661866c8b1a712ba7a0972ba9623cc)`(QString ClientName)` <br/>// Set Client Name to something different that the default ([JackTrip](main.md#class_jack_trip))"
    

??? example "`public inline virtual void `[`setLoopBack`](#class_audio_interface_1af8148bc160f637ebed6c3351d6a2c7b6)`(bool b)` <br/>// "
    

??? example "`public inline virtual void `[`enableBroadcastOutput`](#class_audio_interface_1ac5cfa001682e6e0826c63b5f7d6350a8)`()` <br/>// "
    

??? example "`public inline virtual void `[`setAudioTesterP`](#class_audio_interface_1a7b2a6673220bcb19ada364d629e0c1da)`(`[`AudioTester`](undefined.md#class_audio_tester)` * atp)` <br/>// "
    

??? example "`public inline virtual int `[`getNumInputChannels`](#class_audio_interface_1a6c05fbe2cf820b4a764d7a905c61e5e6)`() const` <br/>// Get Number of Input Channels."
    

??? example "`public inline virtual int `[`getNumOutputChannels`](#class_audio_interface_1a4f1dc0048bbf4aa85819f464647df07f)`() const` <br/>// Get Number of Output Channels."
    

??? example "`public inline virtual `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`getBufferSizeInSamples`](#class_audio_interface_1af0a0e609826e67d47ca130641ca07fc0)`() const` <br/>// "
    

??? example "`public inline virtual `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`getDeviceID`](#class_audio_interface_1a7924a1fc68992f5e46cfd766d45dfe89)`() const` <br/>// "
    

??? example "`public virtual size_t `[`getSizeInBytesPerChannel`](#class_audio_interface_1a38393b4d8e032c5e834c888d2697a1ab)`() const` <br/>// "
    

??? example "`public inline virtual `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`getSampleRate`](#class_audio_interface_1a1d6abfe44a1e13976aff878bf5e22e9e)`() const` <br/>// Get the Jack Server Sampling Rate, in samples/second."
    

??? example "`public virtual `[`samplingRateT`](#class_audio_interface_1a538ea04ceee63f418c629d6a3a0e4fef)` `[`getSampleRateType`](#class_audio_interface_1a2454d2f0b02a67b7fab00481877005a9)`() const` <br/>// Get the Jack Server Sampling Rate Enum Type samplingRateT."
    #### Returns
	[AudioInterface::samplingRateT](#class_audio_interface_1a538ea04ceee63f418c629d6a3a0e4fef) enum type

??? example "`public inline virtual int `[`getAudioBitResolution`](#class_audio_interface_1a728fb37319785ace1c42605d955cab7c)`() const` <br/>// Get the Audio Bit Resolution, in bits."
    This is one of the audioBitResolutionT set in construction

??? example "`protected bool `[`mProcessingAudio`](#class_audio_interface_1aa1104803afb9fef3b29dc128ee279d60) <br/>// Set when processing an audio callback buffer pair."
    

??? example "`protected const `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`MAX_AUDIO_BUFFER_SIZE`](#class_audio_interface_1a512aaddf196aed8dbbbe46d13ab6499b) <br/>// "
    

??? example "`enum `[`audioBitResolutionT`](#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b) <br/>// Enum for Audio Resolution in bits."
     Values                         | Descriptions                                
    --------------------------------|---------------------------------------------
        BIT8            | 8 bits
    BIT16            | 16 bits (default)
    BIT24            | 24 bits
    BIT32            | 32 bits
    

??? example "`enum `[`samplingRateT`](#class_audio_interface_1a538ea04ceee63f418c629d6a3a0e4fef) <br/>// Sampling Rates supported by JACK."
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
    

## class `JackAudioInterface` {#class_jack_audio_interface}

```
class JackAudioInterface
  : public AudioInterface
```

Class that provides an interface with the Jack Audio Server.

> Todo: implement srate_callback 

> Todo: automatically starts jack with buffer and sample rate settings specified by the user

### Members

??? example "`public  `[`JackAudioInterface`](#class_jack_audio_interface_1a00d156b34cdac0825012b6cde2d3cdaa)`(`[`JackTrip`](main.md#class_jack_trip)` * jacktrip,int NumInChans,int NumOutChans,`[`AudioInterface::audioBitResolutionT`](#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b)` AudioBitResolution,QString ClientName)` <br/>// The class constructor."
    #### Parameters
	* `jacktrip` Pointer to the [JackTrip](main.md#class_jack_trip) class that connects all classes (mediator) 

	* `NumInChans` Number of Input Channels 

	* `NumOutChans` Number of Output Channels 

	* `AudioBitResolution` Audio Sample Resolutions in bits 

	* `ClientName` Client name in Jack

??? example "`public virtual  `[`~JackAudioInterface`](#class_jack_audio_interface_1a90073f74b8642080a88f2bbeb5f78b8d)`()` <br/>// The class destructor."
    

??? example "`public virtual void `[`setup`](#class_jack_audio_interface_1a86f2df3ab05bb9cc5b641d8716166046)`()` <br/>// Setup the client."
    

??? example "`public virtual int `[`startProcess`](#class_jack_audio_interface_1a9d5b2043029f6b295a2ab67c2a3e07d8)`() const` <br/>// Tell the JACK server that we are ready to roll. The process-callback will start running. This runs on its own thread."
    #### Returns
	0 on success, otherwise a non-zero error code

??? example "`public virtual int `[`stopProcess`](#class_jack_audio_interface_1af8c5861e7678caa0c4b1b5147711b80c)`() const` <br/>// Stops the process-callback thread."
    #### Returns
	0 on success, otherwise a non-zero error code

??? example "`public virtual void `[`connectDefaultPorts`](#class_jack_audio_interface_1a0a389bfb2a047098063691008093e7a2)`()` <br/>// Connect the default ports, capture to sends, and receives to playback."
    

??? example "`public inline virtual void `[`setClientName`](#class_jack_audio_interface_1afc2a986630338b3a6fdf1bcf23033c56)`(QString ClientName)` <br/>// Set Client Name to something different that the default ([JackTrip](main.md#class_jack_trip))"
    

??? example "`public inline virtual void `[`setSampleRate`](#class_jack_audio_interface_1ab189e0e877335a9985532a2628a4ed4f)`(`[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)`)` <br/>// "
    

??? example "`public inline virtual void `[`setBufferSizeInSamples`](#class_jack_audio_interface_1a03f9652ee8cff4dd681cb4f839414a14)`(`[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)`)` <br/>// "
    

??? example "`public inline virtual void `[`enableBroadcastOutput`](#class_jack_audio_interface_1ade8a2a001638d67747ed51523388d02e)`()` <br/>// "
    

??? example "`public virtual `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`getSampleRate`](#class_jack_audio_interface_1af745086680ee649ceef92b5a559e203a)`() const` <br/>// Get the Jack Server Sampling Rate, in samples/second."
    

??? example "`public virtual `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`getBufferSizeInSamples`](#class_jack_audio_interface_1a37bdc25c30b63e6401c0b11adfdc33cf)`() const` <br/>// Get the Jack Server Buffer Size, in samples."
    

??? example "`public inline virtual `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`getBufferSizeInBytes`](#class_jack_audio_interface_1a2073c313564cf0d307ff5820b3423dfe)`() const` <br/>// Get the Jack Server Buffer Size, in bytes."
    

??? example "`public virtual size_t `[`getSizeInBytesPerChannel`](#class_jack_audio_interface_1a53662ef7a0f09861f109b5d3339d0604)`() const` <br/>// Get size of each audio per channel, in bytes."
    

## class `JMess` {#class_j_mess}

Class to save and load all jack client connections.

Saves an XML file with all the current jack connections. This same file can be loaded to connect evrything again. The XML file can also be edited.

Has also an option to disconnect all the clients.

### Members

??? example "`public  `[`JMess`](#class_j_mess_1a36df8444ef5a2487c1893145f878b214)`()` <br/>// Constructs a [JMess](#class_j_mess) object that has a jack client."
    

??? example "`public virtual  `[`~JMess`](#class_j_mess_1a73229d9ef69285d5053b6177a663989f)`()` <br/>// Distructor closes the jmess jack audio client."
    

??? example "`public void `[`disconnectAll`](#class_j_mess_1a1d49a120634e44b7e09e295ca6d556d5)`()` <br/>// Disconnect all the clients."
    

??? example "`public void `[`writeOutput`](#class_j_mess_1a44d7176210eb8f4bd63b3c42a62045d8)`(QString xmlOutFile)` <br/>// Write an XML file with the name specified at xmlOutFile."
    

??? example "`public void `[`connectPorts`](#class_j_mess_1aa20e6ccab2deef71a93245f74d3031b9)`(QString xmlInFile)` <br/>// Connect ports specified in input XML file xmlInFile."
    

??? example "`public void `[`setConnectedPorts`](#class_j_mess_1ad1fc9a3fc4ee628a2acf6b3123f26512)`()` <br/>// Set list of ouput ports that have connections."
    

??? example "`public void `[`connectSpawnedPorts`](#class_j_mess_1a94f02a307f0f01ffd6c5e35fa0777123)`(int nChans,int hubPatch)` <br/>// Cross connect ports between net combs, -l LAIR mode."
    

??? example "`public void `[`connectTUB`](#class_j_mess_1a8c854f014fad349c238dd584ec25d761)`(int nChans)` <br/>// "
    

## class `RtAudioInterface` {#class_rt_audio_interface}

```
class RtAudioInterface
  : public AudioInterface
```

Base Class that provides an interface with RtAudio.

### Members

??? example "`public  `[`RtAudioInterface`](#class_rt_audio_interface_1ac907cc7da634bb8aea6b849b498ff93b)`(`[`JackTrip`](main.md#class_jack_trip)` * jacktrip,int NumInChans,int NumOutChans,`[`audioBitResolutionT`](#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b)` AudioBitResolution)` <br/>// The class constructor."
    #### Parameters
	* `jacktrip` Pointer to the [JackTrip](main.md#class_jack_trip) class that connects all classes (mediator) 

	* `NumInChans` Number of Input Channels 

	* `NumOutChans` Number of Output Channels 

	* `AudioBitResolution` Audio Sample Resolutions in bits

??? example "`public virtual  `[`~RtAudioInterface`](#class_rt_audio_interface_1aa1ce96da4a64fd1a272d5a11a37b9f46)`()` <br/>// The class destructor."
    

??? example "`public virtual void `[`listAllInterfaces`](#class_rt_audio_interface_1a9dbce530ab0d07ada45cb795eb33178e)`()` <br/>// List all avialable audio interfaces, with its properties."
    

??? example "`public virtual void `[`setup`](#class_rt_audio_interface_1a33a9d21ada7464c484c1e7c75b26630f)`()` <br/>// Setup the client. This function should be called just before."
    starting the audio processes, it will setup the audio client with the class parameters, like Sampling Rate, Packet Size, Bit Resolution, etc... Sub-classes should also call the parent method to ensure correct inizialization.

??? example "`public virtual int `[`startProcess`](#class_rt_audio_interface_1a375c7bb8811a7c238da159dbf5116780)`() const` <br/>// Tell the audio server that we are ready to roll. The process-callback will start running. This runs on its own thread."
    #### Returns
	0 on success, otherwise a non-zero error code

??? example "`public virtual int `[`stopProcess`](#class_rt_audio_interface_1af03530184e3b29d49b5159edcc7bad75)`() const` <br/>// Stops the process-callback thread."
    #### Returns
	0 on success, otherwise a non-zero error code

??? example "`public inline virtual void `[`connectDefaultPorts`](#class_rt_audio_interface_1ad4214d65d22d8f5625f4130985216429)`()` <br/>// This has no effect in RtAudio."
    

??? example "`public inline virtual void `[`setClientName`](#class_rt_audio_interface_1a2e3a089e0318a73cb68854a7010214b0)`(QString)` <br/>// This has no effect in RtAudio."
    

