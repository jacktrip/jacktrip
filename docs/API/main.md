# group `main` {#group__main}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`class `[`JackTrip`](#class_jack_trip) | Main class to creates a SERVER (to listen) or a CLIENT (to connect to a listening server) to send audio streams in the network.
`class `[`Settings`](#class_settings) | Class to set usage options and parse settings from input.
`class `[`UdpHubListener`](#class_udp_hub_listener) | Hub UDP listener on the Server.

## class `JackTrip` {#class_jack_trip}

```
class JackTrip
  : public QObject
```

Main class to creates a SERVER (to listen) or a CLIENT (to connect to a listening server) to send audio streams in the network.

All audio and network settings can be set in this class. This class also acts as a Mediator between all the other class. Classes that uses [JackTrip](#class_jack_trip) methods need to register with it.

### Members

??? example "`public  `[`JackTrip`](#class_jack_trip_1a9ce0d5c38eab1235f362c8eabc15aaba)`(`[`jacktripModeT`](#class_jack_trip_1a062df7ced6d5d7f3dd47b5d05c393982)` JacktripMode,`[`dataProtocolT`](#class_jack_trip_1aba7784e922095de89025a2f6fc14559d)` DataProtocolType,int NumChans,int BufferQueueLength,unsigned int redundancy,`[`AudioInterface::audioBitResolutionT`](audio_interface.md#class_audio_interface_1a926097c3e099add736ea1b8d1b9a8a2b)` AudioBitResolution,`[`DataProtocol::packetHeaderTypeT`](network.md#class_data_protocol_1aed102fbb7239cbf862e6296eefe79492)` PacketHeaderType,`[`underrunModeT`](#class_jack_trip_1a9fd34b5de9751d9bafbb03a7c2250bf5)` UnderRunMode,int receiver_bind_port,int sender_bind_port,int receiver_peer_port,int sender_peer_port,int tcp_peer_port)` <br/>// The class Constructor with Default Parameters."
    #### Parameters
	* `JacktripMode` [JackTrip::CLIENT](#class_jack_trip_1a062df7ced6d5d7f3dd47b5d05c393982adfe81f0a6336dc01d4f9db1c3acba59a) or [JackTrip::SERVER](#class_jack_trip_1a062df7ced6d5d7f3dd47b5d05c393982a55b561bf36053294b717fbf49b71d23c)

	* `DataProtocolType` [JackTrip::dataProtocolT](#class_jack_trip_1aba7784e922095de89025a2f6fc14559d)

	* `NumChans` Number of Audio Channels (same for inputs and outputs) 

	* `BufferQueueLength` Audio Buffer for receiving packets 

	* `AudioBitResolution` Audio Sample Resolutions in bits 

	* `redundancy` redundancy factor for network data

??? example "`public virtual  `[`~JackTrip`](#class_jack_trip_1aa30cb3e781cfb52195f42e5e81fe1d83)`()` <br/>// The class destructor."
    

??? example "`public virtual void `[`setPeerAddress`](#class_jack_trip_1a3555920e8f72a77dea6d3d79265e98b5)`(QString PeerHostOrIP)` <br/>// Starting point for the thread."
    Set the Peer Address for jacktripModeT::CLIENT mode only

??? example "`public virtual void `[`appendProcessPluginToNetwork`](#class_jack_trip_1a39bdc701636e81672bab041dbdf5c8ba)`(`[`ProcessPlugin`](effects.md#class_process_plugin)` * plugin)` <br/>// Append a process plugin. Processes will be appended in order."
    #### Parameters
	* `plugin` Pointer to [ProcessPlugin](effects.md#class_process_plugin) Class

??? example "`public virtual void `[`appendProcessPluginFromNetwork`](#class_jack_trip_1a6a79e2341a43c83ed1be2f0cc73d4806)`(`[`ProcessPlugin`](effects.md#class_process_plugin)` * plugin)` <br/>// "
    

??? example "`public virtual void `[`startProcess`](#class_jack_trip_1a87f6cce2241917cb9dae804e9f0b416d)`()` <br/>// Start the processing threads."
    

??? example "`public virtual void `[`completeConnection`](#class_jack_trip_1a9b5421fa4402ff229f60a929f8d4f390)`()` <br/>// "
    

??? example "`public virtual void `[`stop`](#class_jack_trip_1a5a1c9a46327d7eca0ea8763cf7413b9e)`(QString errorMessage)` <br/>// Stop the processing threads."
    

??? example "`public virtual void `[`waitThreads`](#class_jack_trip_1a9f14f7a651eeb71a35d829d66187cbce)`()` <br/>// Wait for all the threads to finish. This functions is used when [JackTrip](#class_jack_trip) is run as a thread."
    

??? example "`public virtual void `[`checkIfPortIsBinded`](#class_jack_trip_1a224dcbabba45ec353c4ed30714dd83fc)`(int port)` <br/>// Check if UDP port is already binded."
    #### Parameters
	* `port` Port number

??? example "`enum `[`dataProtocolT`](#class_jack_trip_1aba7784e922095de89025a2f6fc14559d) <br/>// Enum for the data Protocol. At this time only UDP is implemented."
     Values                         | Descriptions                                
    --------------------------------|---------------------------------------------
        UDP            | Use UDP (User Datagram Protocol)
    TCP            | **NOT IMPLEMENTED**: Use TCP (Transmission Control Protocol)
    SCTP            | **NOT IMPLEMENTED**: Use SCTP (Stream Control Transmission Protocol)
    

??? example "`enum `[`jacktripModeT`](#class_jack_trip_1a062df7ced6d5d7f3dd47b5d05c393982) <br/>// Enum for the [JackTrip](#class_jack_trip) mode."
     Values                         | Descriptions                                
    --------------------------------|---------------------------------------------
        SERVER            | Run in P2P Server Mode.
    CLIENT            | Run in P2P Client Mode.
    CLIENTTOPINGSERVER            | Client of the Ping Server Mode.
    SERVERPINGSERVER            | Server of the MultiThreaded [JackTrip](#class_jack_trip).
    

??? example "`enum `[`underrunModeT`](#class_jack_trip_1a9fd34b5de9751d9bafbb03a7c2250bf5) <br/>// Enum for the [JackTrip](#class_jack_trip) Underrun Mode, when packets."
     Values                         | Descriptions                                
    --------------------------------|---------------------------------------------
        WAVETABLE            | Loops on the last received packet.
    ZEROS            | Set new buffers to zero if there are no new ones.
    

??? example "`enum `[`audiointerfaceModeT`](#class_jack_trip_1abd27f2208794a3cfea14d41afd2bad28) <br/>// Enum for Audio Interface Mode."
     Values                         | Descriptions                                
    --------------------------------|---------------------------------------------
        JACK            | Jack Mode.
    RTAUDIO            | RtAudio Mode.
    

??? example "`enum `[`connectionModeT`](#class_jack_trip_1a4d337935031bc676863ebbea07c169ab) <br/>// Enum for Connection Mode (in packet header)"
     Values                         | Descriptions                                
    --------------------------------|---------------------------------------------
        NORMAL            | Normal Mode.
    KSTRONG            | Karplus Strong.
    JAMTEST            | Karplus Strong.
    

??? example "`enum `[`hubConnectionModeT`](#class_jack_trip_1a2c6fae8a9d31dea1605b53ac08ce9ed0) <br/>// Enum for Hub Server Audio Connection Mode (connections to hub server are automatically patched in Jack)"
     Values                         | Descriptions                                
    --------------------------------|---------------------------------------------
        SERVERTOCLIENT            | Normal Mode, Sever to All Clients (but not client to any client)
    CLIENTECHO            | Client Echo (client self-to-self)
    CLIENTFOFI            | Client Fan Out to Clients and Fan In from Clients (but not self-to-self)
    RESERVEDMATRIX            | Reserved for custom patch matrix (for TUB ensemble)
    FULLMIX            | Client Fan Out to Clients and Fan In from Clients (including self-to-self)
    NOAUTO            | No automatic patching.
    

## class `Settings` {#class_settings}

```
class Settings
  : public QObject
```

Class to set usage options and parse settings from input.

### Members

??? example "`public  `[`Settings`](#class_settings_1ab7169a6eefce79566dd07db3b1e5e967)`()` <br/>// "
    

??? example "`public virtual  `[`~Settings`](#class_settings_1a4a65be5921dfc9fddc476e5320541d89)`()` <br/>// "
    

??? example "`public void `[`parseInput`](#class_settings_1a7bc7d9d6c0d7ee24d869fba991c3dc0b)`(int argc,char ** argv)` <br/>// Parses command line input."
    

??? example "`public `[`UdpHubListener`](#class_udp_hub_listener)` * `[`getConfiguredHubServer`](#class_settings_1a3da20f27e35b70c274416825f26d48a3)`()` <br/>// "
    

??? example "`public `[`JackTrip`](#class_jack_trip)` * `[`getConfiguredJackTrip`](#class_settings_1a73492ef0c134aba9068113314847014f)`()` <br/>// "
    

??? example "`public void `[`printUsage`](#class_settings_1a9315abef24f993e0a47c444bdf6bd03f)`()` <br/>// Prints usage help."
    

??? example "`public inline bool `[`getLoopBack`](#class_settings_1add866d37691ce269f57c8a659a656891)`()` <br/>// "
    

??? example "`public inline bool `[`isHubServer`](#class_settings_1a8398da8062f377c871fe38b3735942c7)`()` <br/>// "
    

## class `UdpHubListener` {#class_udp_hub_listener}

```
class UdpHubListener
  : public QObject
```

Hub UDP listener on the Server.

This creates a server that will listen on the well know port (the server port) and will spawn [JackTrip](#class_jack_trip) threads into the Thread pool. Clients request a connection.

### Members

??? example "`public unsigned int `[`mHubPatch`](#class_udp_hub_listener_1aef3946bc18580a6c78e5d62af5456e84) <br/>// "
    

??? example "`public bool `[`mAppendThreadID`](#class_udp_hub_listener_1a112cbeef12478227390fd8d1a16755ac) <br/>// "
    

??? example "`public  `[`UdpHubListener`](#class_udp_hub_listener_1a1a661264bbd7e35148566d20f35a5975)`(int server_port,int server_udp_port)` <br/>// "
    

??? example "`public virtual  `[`~UdpHubListener`](#class_udp_hub_listener_1a8a306efd7ae43ad8005f5f3f02fb44e8)`()` <br/>// "
    

??? example "`public void `[`start`](#class_udp_hub_listener_1a14302c1181fc52ee905161f2f4441aff)`()` <br/>// Starts the TCP server."
    

??? example "`public inline void `[`stop`](#class_udp_hub_listener_1abcf1c02b861a77289471da58f105f124)`()` <br/>// Stops the execution of the Thread."
    

??? example "`public int `[`releaseThread`](#class_udp_hub_listener_1a0940af2aa63d40db896e20840f1d8029)`(int id)` <br/>// "
    

??? example "`public inline void `[`setConnectDefaultAudioPorts`](#class_udp_hub_listener_1aeac1db5ebf010f290e50500b7683edbe)`(bool connectDefaultAudioPorts)` <br/>// "
    

??? example "`public inline void `[`setHubPatch`](#class_udp_hub_listener_1a961a0ca9ff7f1840ec1b7cdd9ae24055)`(unsigned int p)` <br/>// "
    

??? example "`public inline unsigned int `[`getHubPatch`](#class_udp_hub_listener_1a548db06a0e900048437d36a8816d05f3)`()` <br/>// "
    

??? example "`public inline void `[`setUnderRunMode`](#class_udp_hub_listener_1ad6f3f47b75bc35d1c287ebb8eb2c049a)`(`[`JackTrip::underrunModeT`](#class_jack_trip_1a9fd34b5de9751d9bafbb03a7c2250bf5)` UnderRunMode)` <br/>// "
    

??? example "`public inline void `[`setBufferQueueLength`](#class_udp_hub_listener_1a8b17895aa1f3788b016a75b1ee0a4039)`(int BufferQueueLength)` <br/>// "
    

??? example "`public inline void `[`setIOStatTimeout`](#class_udp_hub_listener_1a67491c6165e0f4e3f658e3e3a69c8a53)`(int timeout)` <br/>// "
    

??? example "`public inline void `[`setIOStatStream`](#class_udp_hub_listener_1a990ad20982069e2392482e57a25b6700)`(QSharedPointer< std::ofstream > statStream)` <br/>// "
    

??? example "`public inline void `[`setBufferStrategy`](#class_udp_hub_listener_1a05ef98435e7f56e7e0f89c8e14e850b3)`(int BufferStrategy)` <br/>// "
    

??? example "`public inline void `[`setNetIssuesSimulation`](#class_udp_hub_listener_1a3a044b4e620511457bfa10ea083c9e04)`(double loss,double jitter,double delay_rel)` <br/>// "
    

??? example "`public inline void `[`setBroadcast`](#class_udp_hub_listener_1a1da0df938d4e39b1ce05097b5ae20987)`(int broadcast_queue)` <br/>// "
    

??? example "`public inline void `[`setUseRtUdpPriority`](#class_udp_hub_listener_1a070b0d78c8f656523af2456d7a0c0e7b)`(bool use)` <br/>// "
    

??? example "`{signal} public void `[`Listening`](#class_udp_hub_listener_1a5af2f5a3e2859802f95c6dd7b552761e)`()` <br/>// "
    

??? example "`{signal} public void `[`ClientAddressSet`](#class_udp_hub_listener_1ab58bf3699e2ae0f0a1845e5f2a983003)`()` <br/>// "
    

??? example "`{signal} public void `[`signalRemoveThread`](#class_udp_hub_listener_1ace9855059fb7d305e208e658aa8ac502)`(int id)` <br/>// "
    

??? example "`{signal} public void `[`signalStopped`](#class_udp_hub_listener_1a81493e16c1c13915dd7e917fe45a7313)`()` <br/>// "
    

??? example "`{signal} public void `[`signalError`](#class_udp_hub_listener_1ab66da45c363859869526b3406a050394)`(const QString & errorMessage)` <br/>// "
    

