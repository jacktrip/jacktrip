# group `network` {#group__network}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`class `[`DataProtocol`](#class_data_protocol) | Base class that defines the transmission protocol.
`class `[`PacketHeader`](#class_packet_header) | Base class for header type. Subclass this struct to create a new header.
`class `[`DefaultHeader`](#class_default_header) | Default Header.
`class `[`JamLinkHeader`](#class_jam_link_header) | JamLink Header.
`class `[`EmptyHeader`](#class_empty_header) | Empty Header to use with systems that don't include a header.
`class `[`UdpDataProtocol`](#class_udp_data_protocol) | UDP implementation of [DataProtocol](#class_data_protocol) class.
`struct `[`HeaderStruct`](#struct_header_struct) | Abstract Header Struct, Header Stucts should subclass it.
`struct `[`DefaultHeaderStruct`](#struct_default_header_struct) | Default Header Struct.
`struct `[`JamLinkHeaderStuct`](#struct_jam_link_header_stuct) | JamLink Header Struct.

## class `DataProtocol` {#class_data_protocol}

```
class DataProtocol
  : public QThread
```

Base class that defines the transmission protocol.

This base class defines most of the common method to setup and connect sockets using the individual protocols (UDP, TCP, SCTP, etc).

The class has to be constructed using one of two modes (runModeT):<br/>

* SENDER

* RECEIVER

This has to be specified as a constructor argument. When using, create two instances of the class, one to receive and one to send packets. Each instance will run on a separate thread.

Redundancy and forward error correction should be implemented on each Transport protocol, cause they depend on the protocol itself

> Todo: This Class should contain definition of jacktrip header and basic funcionality to obtain local machine IPs and maybe functions to manipulate IPs. Redundancy and forward error correction should be implemented on each Transport protocol, cause they depend on the protocol itself

> Todo: The transport protocol itself has to be implemented subclassing this class, i.e., using a TCP or UDP protocol.

Even if the underlined transmission protocol is stream oriented (as in TCP), we send packets that are the size of the audio processing buffer. Use AudioInterface::getBufferSize to obtain this value.

Each transmission (i.e., inputs and outputs) run on its own thread.

### Members

??? example "`public  `[`DataProtocol`](#class_data_protocol_1a692c494d35c01180758aa6e32222b535)`(`[`JackTrip`](main.md#class_jack_trip)` * jacktrip,const `[`runModeT`](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2c)` runmode,int bind_port,int peer_port)` <br/>// The class constructor."
    #### Parameters
	* `jacktrip` Pointer to the [JackTrip](main.md#class_jack_trip) class that connects all classes (mediator) 

	* `runmode` Sets the run mode, use either [DataProtocol::SENDER](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2ca03af57b57ddf9036d6444ceef3ca3059) or [DataProtocol::RECEIVER](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2ca1a12f7cf3e8bf36932809cb9083e223f)

	* `headertype` packetHeaderTypeT header type to use for packets 

	* `bind_port` Port number to bind for this socket (this is the receive or send port depending on the runmode) 

	* `peer_port` Peer port number (this is the receive or send port depending on the runmode)

??? example "`public virtual  `[`~DataProtocol`](#class_data_protocol_1a7d0b609ce5497affa295c5cc7fc5db25)`()` <br/>// The class destructor."
    

??? example "`public void `[`run`](#class_data_protocol_1ad23766c4038720e85f8fe9f83069ae1c)`()` <br/>// Implements the thread loop."
    Depending on the runmode, with will run a [DataProtocol::SENDER](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2ca03af57b57ddf9036d6444ceef3ca3059) thread or [DataProtocol::RECEIVER](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2ca1a12f7cf3e8bf36932809cb9083e223f) thread

??? example "`public inline virtual void `[`stop`](#class_data_protocol_1a8bc0b6b855cbaf7d7a7d5991d3566059)`()` <br/>// Stops the execution of the Thread."
    

??? example "`public inline void `[`setAudioPacketSize`](#class_data_protocol_1af46c116bd04cc8b06740c6e6fe5fa622)`(const size_t size_bytes)` <br/>// Sets the size of the audio part of the packets."
    #### Parameters
	* `size_bytes` Size in bytes

??? example "`public inline size_t `[`getAudioPacketSizeInBites`](#class_data_protocol_1a942fcff4796851420420e7976f93c48f)`()` <br/>// Get the size of the audio part of the packets."
    #### Returns
	size_bytes Size in bytes

??? example "`public void `[`setPeerAddress`](#class_data_protocol_1aed6de555806ed1cb1a4ab618f28d1c7c)`(const char * peerHostOrIP)` <br/>// Set the peer address."
    #### Parameters
	* `peerHostOrIP` IPv4 number or host name 

> Todo: implement here instead of in the subclass UDP

??? example "`public void `[`setPeerPort`](#class_data_protocol_1a66a344c0b521beddc8bb510d483515d9)`(int port)` <br/>// Set the peer incomming (receiving) port number."
    #### Parameters
	* `port` Port number 

> Todo: implement here instead of in the subclass UDP

??? example "`public void `[`setSocket`](#class_data_protocol_1a65f89d9a60ed0f14b9848ae30179c47b)`(int & socket)` <br/>// "
    

??? example "`public inline virtual bool `[`getStats`](#class_data_protocol_1ab663c37d81764780f9045471714c83ec)`(`[`PktStat`](undefined.md#struct_data_protocol_1_1_pkt_stat)` *)` <br/>// "
    

??? example "`public inline virtual void `[`setIssueSimulation`](#class_data_protocol_1a21fdce1f6a6e35f069ce4749fd069866)`(double,double,double)` <br/>// "
    

??? example "`public inline void `[`setUseRtPriority`](#class_data_protocol_1a0169677394e53ef74448ecf6d29a01b3)`(bool use)` <br/>// "
    

??? example "`protected bool `[`mStopped`](#class_data_protocol_1a2840e4119efdc72870daf34267efd36e) <br/>// Boolean stop the execution of the thread."
    

??? example "`protected bool `[`mHasPeerAddress`](#class_data_protocol_1aa2febf67f13757e0876362622aa335b9) <br/>// Boolean to indicate if the RECEIVER is waiting to obtain peer address."
    

??? example "`protected bool `[`mHasPacketsToReceive`](#class_data_protocol_1a4efcb035c3376e52faf1f5052ffcccff) <br/>// Boolean that indicates if a packet was received."
    

??? example "`protected QMutex `[`mMutex`](#class_data_protocol_1a87be7ce305256235119309d3879416d7) <br/>// "
    

??? example "`protected `[`JackTrip`](main.md#class_jack_trip)` * `[`mJackTrip`](#class_data_protocol_1a191bbc5edd6bd92dc2393d36dd365d63) <br/>// [JackTrip](main.md#class_jack_trip) mediator class."
    > Todo: check a better way to access the header from the subclasses

??? example "`protected bool `[`mUseRtPriority`](#class_data_protocol_1a176e64360be658b3e81081d2386087f0) <br/>// "
    

??? example "`protected inline `[`runModeT`](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2c)` `[`getRunMode`](#class_data_protocol_1affebd07fe22569a24ec1ffb261d0fbea)`() const` <br/>// Get the Run Mode of the object."
    #### Returns
	SENDER or RECEIVER

??? example "`{signal} public void `[`signalError`](#class_data_protocol_1ada3298bb52b4d69166d2e7406d77071b)`(const char * error_message)` <br/>// "
    

??? example "`{signal} public void `[`signalReceivedConnectionFromPeer`](#class_data_protocol_1a7f6c38625966f43279190605437043da)`()` <br/>// "
    

??? example "`{signal} public void `[`signalCeaseTransmission`](#class_data_protocol_1a484e57eb3895eeb887992085c2bff167)`(const QString & reason)` <br/>// "
    

??? example "`enum `[`packetHeaderTypeT`](#class_data_protocol_1aed102fbb7239cbf862e6296eefe79492) <br/>// Enum to define packet header types."
     Values                         | Descriptions                                
    --------------------------------|---------------------------------------------
        DEFAULT            | Default application header.
    JAMLINK            | Header to use with Jamlinks.
    EMPTY            | Empty Header.
    

??? example "`enum `[`runModeT`](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2c) <br/>// Enum to define class modes, SENDER or RECEIVER."
     Values                         | Descriptions                                
    --------------------------------|---------------------------------------------
        SENDER            | Set class as a Sender (send packets)
    RECEIVER            | Set class as a Receiver (receives packets)
    

## class `PacketHeader` {#class_packet_header}

```
class PacketHeader
  : public QObject
```

Base class for header type. Subclass this struct to create a new header.

### Members

??? example "`public  `[`PacketHeader`](#class_packet_header_1a258260b7f4f1d9d0a316dcf958b95ce4)`(`[`JackTrip`](main.md#class_jack_trip)` * jacktrip)` <br/>// The class Constructor."
    

??? example "`public inline virtual  `[`~PacketHeader`](#class_packet_header_1a8b138c87057c959cfa2e5921e0b541a6)`()` <br/>// The class Destructor."
    

??? example "`public void `[`fillHeaderCommonFromAudio`](#class_packet_header_1ad601f4d63049dd3489de9b9bd0de12ce)`()` <br/>// "
    > Todo: Implement this using a [JackTrip](main.md#class_jack_trip) Method (Mediator) member instead of the reference to JackAudio

??? example "`public void `[`parseHeader`](#class_packet_header_1a6809614b5e4b65b58f8cdf157cd548ab)`()` <br/>// Parse the packet header and take appropriate measures (like change settings, or quit the program if peer settings don't match)"
    

??? example "`public void `[`checkPeerSettings`](#class_packet_header_1aba5e15f68e2529e18d8faaa240e8f55e)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet)` <br/>// "
    

??? example "`public uint64_t `[`getPeerTimeStamp`](#class_packet_header_1ac9d82e095652ac13c0c1500488895bcb)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet) const` <br/>// "
    

??? example "`public `[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` `[`getPeerSequenceNumber`](#class_packet_header_1ad80c606d6d40735735c307cb3fc2327e)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet) const` <br/>// "
    

??? example "`public `[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` `[`getPeerBufferSize`](#class_packet_header_1ab9db47211bac13322c9f2ff6529a8f34)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet) const` <br/>// "
    

??? example "`public `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerSamplingRate`](#class_packet_header_1ab242cb3da9d3573eeac7cf61ce927070)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet) const` <br/>// "
    

??? example "`public `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerBitResolution`](#class_packet_header_1a6b1704b9b4744ab6f368be2cfdd88a68)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet) const` <br/>// "
    

??? example "`public `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerNumChannels`](#class_packet_header_1aba4685f3b9b54aff2f020fda5cad92af)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet) const` <br/>// "
    

??? example "`public `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerConnectionMode`](#class_packet_header_1a72c41ac2e2ffbf2323499bd3911fb969)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet) const` <br/>// "
    

??? example "`public inline virtual void `[`increaseSequenceNumber`](#class_packet_header_1a963d7c4066be12e8224421aee4534f09)`()` <br/>// Increase sequence number for counter, a 16bit number."
    

??? example "`public inline virtual `[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` `[`getSequenceNumber`](#class_packet_header_1a157c6f89eee184ef72126128cb32d7ba)`() const` <br/>// Returns the current sequence number."
    #### Returns
	16bit Sequence number

??? example "`public int `[`getHeaderSizeInBytes`](#class_packet_header_1afcbe432ee541d39a0695ee5cc8c649a0)`() const` <br/>// Get the header size in bytes."
    

??? example "`public inline virtual void `[`putHeaderInPacketBaseClass`](#class_packet_header_1ac970c6a26a312ec9d8c67bf213e1bf7d)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet,const `[`HeaderStruct`](#struct_header_struct)` & header_struct)` <br/>// "
    

??? example "`public void `[`putHeaderInPacket`](#class_packet_header_1a31ead0aadb20c2475a3d516f0cdd8830)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet)` <br/>// Put the header in buffer pointed by full_packet."
    #### Parameters
	* `full_packet` Pointer to full packet (audio+header). Size must be sizeof(header part) + sizeof(audio part)

??? example "`{signal} public void `[`signalError`](#class_packet_header_1aad520d417ed2c92b0ee029f0d6c2e00a)`(const QString & error_message)` <br/>// "
    

## class `DefaultHeader` {#class_default_header}

```
class DefaultHeader
  : public PacketHeader
```

Default Header.

### Members

??? example "`public  `[`DefaultHeader`](#class_default_header_1a2d1407006ef6d3783cf32e873f159c3d)`(`[`JackTrip`](main.md#class_jack_trip)` * jacktrip)` <br/>// "
    

??? example "`public inline virtual  `[`~DefaultHeader`](#class_default_header_1a8c71567104d780b44ac67a2168b51cbb)`()` <br/>// "
    

??? example "`public virtual void `[`fillHeaderCommonFromAudio`](#class_default_header_1a8a0f1c704781d62affabaf10c5c3b1f7)`()` <br/>// "
    > Todo: Implement this using a [JackTrip](main.md#class_jack_trip) Method (Mediator) member instead of the reference to JackAudio

??? example "`public inline virtual void `[`parseHeader`](#class_default_header_1afad35df3b277ad2edb744970456f2beb)`()` <br/>// Parse the packet header and take appropriate measures (like change settings, or quit the program if peer settings don't match)"
    

??? example "`public virtual void `[`checkPeerSettings`](#class_default_header_1acc84544c5932a10cd00b2f8581f28665)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet)` <br/>// "
    

??? example "`public inline virtual void `[`increaseSequenceNumber`](#class_default_header_1a535d0b118153138c957c45a0d47201d1)`()` <br/>// Increase sequence number for counter, a 16bit number."
    

??? example "`public inline virtual `[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` `[`getSequenceNumber`](#class_default_header_1ab2dd76b3f36a15a570b10a74524a8fbb)`() const` <br/>// Returns the current sequence number."
    #### Returns
	16bit Sequence number

??? example "`public inline virtual int `[`getHeaderSizeInBytes`](#class_default_header_1abb9d322bdcb1164edaf1065e25560a4d)`() const` <br/>// Get the header size in bytes."
    

??? example "`public inline virtual void `[`putHeaderInPacket`](#class_default_header_1abf41156b2edeb250ec260cce8d451faa)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet)` <br/>// Put the header in buffer pointed by full_packet."
    #### Parameters
	* `full_packet` Pointer to full packet (audio+header). Size must be sizeof(header part) + sizeof(audio part)

??? example "`public void `[`printHeader`](#class_default_header_1add598241b4b06decbd2153364049f0fb)`() const` <br/>// "
    

??? example "`public inline `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getConnectionMode`](#class_default_header_1ac5e5085594cb32149e414cceab1f5b86)`() const` <br/>// "
    

??? example "`public inline `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getNumChannels`](#class_default_header_1a275ceed8df3723b5ef7937e16f27a65b)`() const` <br/>// "
    

??? example "`public virtual uint64_t `[`getPeerTimeStamp`](#class_default_header_1a43aedd403ca82a60c1fdde8c49db6b1b)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet) const` <br/>// "
    

??? example "`public virtual `[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` `[`getPeerSequenceNumber`](#class_default_header_1ad78e28d49c1a29523885b50b3e6f174d)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet) const` <br/>// "
    

??? example "`public virtual `[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` `[`getPeerBufferSize`](#class_default_header_1a0d68987d96adb608cd5c21d8ac384dba)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet) const` <br/>// "
    

??? example "`public virtual `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerSamplingRate`](#class_default_header_1aff5e2b7763e3eea1bba9f625d30d04f7)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet) const` <br/>// "
    

??? example "`public virtual `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerBitResolution`](#class_default_header_1a0fb2815db845059992e3990579f8787f)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet) const` <br/>// "
    

??? example "`public virtual `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerNumChannels`](#class_default_header_1a4a65580821d9d28814608b192cf8844c)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet) const` <br/>// "
    

??? example "`public virtual `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerConnectionMode`](#class_default_header_1a2811cf97104b4f5ed2ab27d9ea7a4f20)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet) const` <br/>// "
    

## class `JamLinkHeader` {#class_jam_link_header}

```
class JamLinkHeader
  : public PacketHeader
```

JamLink Header.

### Members

??? example "`public  `[`JamLinkHeader`](#class_jam_link_header_1a05cefbbc5896cbfa2570ed7f73f3095c)`(`[`JackTrip`](main.md#class_jack_trip)` * jacktrip)` <br/>// "
    

??? example "`public inline virtual  `[`~JamLinkHeader`](#class_jam_link_header_1ad946ef3af4d39ba2f2b488f29e105dea)`()` <br/>// "
    

??? example "`public virtual void `[`fillHeaderCommonFromAudio`](#class_jam_link_header_1a1460ffed8c87d309b5f97946137fd2f9)`()` <br/>// "
    > Todo: Implement this using a [JackTrip](main.md#class_jack_trip) Method (Mediator) member instead of the reference to JackAudio

??? example "`public inline virtual void `[`parseHeader`](#class_jam_link_header_1acc9570d34f4fef928d0a738ce116981f)`()` <br/>// Parse the packet header and take appropriate measures (like change settings, or quit the program if peer settings don't match)"
    

??? example "`public inline virtual void `[`checkPeerSettings`](#class_jam_link_header_1a118fc8ef2db06a1734a788253d29656b)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *)` <br/>// "
    

??? example "`public inline virtual uint64_t `[`getPeerTimeStamp`](#class_jam_link_header_1a8fc2326055c0a55efbd24bf336f9769e)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *) const` <br/>// "
    

??? example "`public inline virtual `[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` `[`getPeerSequenceNumber`](#class_jam_link_header_1aebf3e042ecb938fde3aeb2bbd43791ec)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *) const` <br/>// "
    

??? example "`public inline virtual `[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` `[`getPeerBufferSize`](#class_jam_link_header_1a058bf6f9aa2a793dc8a833a266bc255b)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *) const` <br/>// "
    

??? example "`public inline virtual `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerSamplingRate`](#class_jam_link_header_1a7baf5f2b61bd5d63b777ffd75f0f4c0d)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *) const` <br/>// "
    

??? example "`public inline virtual `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerBitResolution`](#class_jam_link_header_1a3668fef3cc988c3c73b2e554f92d0748)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *) const` <br/>// "
    

??? example "`public inline virtual `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerNumChannels`](#class_jam_link_header_1a7c605681757cf0714661c120498d439a)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *) const` <br/>// "
    

??? example "`public inline virtual `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerConnectionMode`](#class_jam_link_header_1aeb16a00bd1a7436e487b0bb4cdb65e2d)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *) const` <br/>// "
    

??? example "`public inline virtual void `[`increaseSequenceNumber`](#class_jam_link_header_1a01e4efd2aa98316da0439b89ff5da0c2)`()` <br/>// Increase sequence number for counter, a 16bit number."
    

??? example "`public inline virtual int `[`getHeaderSizeInBytes`](#class_jam_link_header_1ad984d1e949b6c7fa4bc5cf5447a05fa0)`() const` <br/>// Get the header size in bytes."
    

??? example "`public inline virtual void `[`putHeaderInPacket`](#class_jam_link_header_1a76401a1a0d37e9755d7985bd6e699f89)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet)` <br/>// Put the header in buffer pointed by full_packet."
    #### Parameters
	* `full_packet` Pointer to full packet (audio+header). Size must be sizeof(header part) + sizeof(audio part)

## class `EmptyHeader` {#class_empty_header}

```
class EmptyHeader
  : public PacketHeader
```

Empty Header to use with systems that don't include a header.

### Members

??? example "`public  `[`EmptyHeader`](#class_empty_header_1acd85d44a1839b354120212d55561d195)`(`[`JackTrip`](main.md#class_jack_trip)` * jacktrip)` <br/>// "
    

??? example "`public inline virtual  `[`~EmptyHeader`](#class_empty_header_1ab4d64cf661c84f4972442afdcb79fbcf)`()` <br/>// "
    

??? example "`public inline virtual void `[`fillHeaderCommonFromAudio`](#class_empty_header_1ac94011891b088bdfc4593730cd3cc533)`()` <br/>// "
    > Todo: Implement this using a [JackTrip](main.md#class_jack_trip) Method (Mediator) member instead of the reference to JackAudio

??? example "`public inline virtual void `[`parseHeader`](#class_empty_header_1a91cf9db64cdc29443a32620c53c34f5d)`()` <br/>// Parse the packet header and take appropriate measures (like change settings, or quit the program if peer settings don't match)"
    

??? example "`public inline virtual void `[`checkPeerSettings`](#class_empty_header_1a35186e3c807439699f6843c75c77d22f)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *)` <br/>// "
    

??? example "`public inline virtual void `[`increaseSequenceNumber`](#class_empty_header_1a2148aec9601f457c58a356cf3d278d0a)`()` <br/>// Increase sequence number for counter, a 16bit number."
    

??? example "`public inline virtual int `[`getHeaderSizeInBytes`](#class_empty_header_1a3327c3171b967cdf88d2b50626beb807)`() const` <br/>// Get the header size in bytes."
    

??? example "`public inline virtual uint64_t `[`getPeerTimeStamp`](#class_empty_header_1a8bbf2b30d4bcaf88b24b98641f91cb37)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *) const` <br/>// "
    

??? example "`public inline virtual `[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` `[`getPeerSequenceNumber`](#class_empty_header_1a664115b5fba68f7a1a7763f623419d20)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *) const` <br/>// "
    

??? example "`public inline virtual `[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` `[`getPeerBufferSize`](#class_empty_header_1a84d746098227a18ce26c6bc5d3395b34)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *) const` <br/>// "
    

??? example "`public inline virtual `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerSamplingRate`](#class_empty_header_1a404b07305f522a12a2f6e753e365b3c2)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *) const` <br/>// "
    

??? example "`public inline virtual `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerBitResolution`](#class_empty_header_1a701bdf97a1538409c829f79db5f1bdd7)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *) const` <br/>// "
    

??? example "`public inline virtual `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerNumChannels`](#class_empty_header_1ad55cc5645387840ab562d8f056eb1df4)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *) const` <br/>// "
    

??? example "`public inline virtual `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`getPeerConnectionMode`](#class_empty_header_1ad66688e25268eb3049e0fa686f8339fa)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` *) const` <br/>// "
    

??? example "`public inline virtual void `[`putHeaderInPacket`](#class_empty_header_1a23c5cc1d3f9e044898ebf95b7f327114)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_packet)` <br/>// Put the header in buffer pointed by full_packet."
    #### Parameters
	* `full_packet` Pointer to full packet (audio+header). Size must be sizeof(header part) + sizeof(audio part)

## class `UdpDataProtocol` {#class_udp_data_protocol}

```
class UdpDataProtocol
  : public DataProtocol
```

UDP implementation of [DataProtocol](#class_data_protocol) class.

The class has a `bind port` and a `peer port`. The meaning of these depends on the runModeT. If it's a SENDER, `bind port` is the source port and `peer port` is the destination port for each UDP packet. If it's a RECEIVER, the `bind port` destination port (for incoming packets) and the `peer port` is the source port.

The SENDER and RECEIVER socket can share the same port/address pair (for compatibility with the JamLink boxes). This is achieved setting the resusable property in the socket for address and port. You have to externaly check if the port is already binded if you want to avoid re-binding to the same port.

The Redundancy Algorythmn works as follows. We send a packet that contains a mUdpRedundancyFactor number of packets (header+audio). This big packet looks as follows:

 | UDP[n] | | UDP[n-1] | ... | UDP[n-(mUdpRedundancyFactor-1)] |

Then, for the new audio buffer, we shift everything to the right and send:

 | UDP[n+1] | | UDP[n] | ... | UDP[n-(mUdpRedundancyFactor-1)+1] |

etc...

For a redundancy factor of 4, this will look as follows:

 | UDP[4] | | UDP[3] | | UDP[2] | | UDP[1] |

 | UDP[5] | | UDP[4] | | UDP[3] | | UDP[2] |

 | UDP[6] | | UDP[5] | | UDP[4] | | UDP[3] |

etc...

Then, the receiving end checks if the firs packet in the list is the one it should use, otherwise it continure reding the mUdpRedundancyFactor packets until it finds the one that should come next (this can better perfected by just jumping until the correct packet). If it has more than one packet that it hasn't yet received, it sends it to the soundcard one by one.

### Members

??? example "`public  `[`UdpDataProtocol`](#class_udp_data_protocol_1a8fe02a9611b22615c8adcacbdaa9e3c8)`(`[`JackTrip`](main.md#class_jack_trip)` * jacktrip,const `[`runModeT`](#class_data_protocol_1ab9ee4d6977b0a8f302014de720a02c2c)` runmode,int bind_port,int peer_port,unsigned int udp_redundancy_factor)` <br/>// The class constructor."
    #### Parameters
	* `jacktrip` Pointer to the [JackTrip](main.md#class_jack_trip) class that connects all classes (mediator) 

	* `runmode` Sets the run mode, use either SENDER or RECEIVER 

	* `bind_port` Port number to bind for this socket (this is the receive or send port depending on the runmode) 

	* `peer_port` Peer port number (this is the receive or send port depending on the runmode) 

	* `udp_redundancy_factor` Number of redundant packets

??? example "`public virtual  `[`~UdpDataProtocol`](#class_udp_data_protocol_1a87bfe158a7b4113ebdc05aabc11656e4)`()` <br/>// The class destructor."
    

??? example "`public virtual void `[`setPeerAddress`](#class_udp_data_protocol_1afb2790b3586847f7d3516392a0dfc840)`(const char * peerHostOrIP)` <br/>// Set the Peer address to connect to."
    #### Parameters
	* `peerHostOrIP` IPv4 number or host name

??? example "`public virtual void `[`setSocket`](#class_udp_data_protocol_1aa19f7a633fa77bb2866740e33fb7db80)`(int & socket)` <br/>// "
    

??? example "`public virtual int `[`receivePacket`](#class_udp_data_protocol_1a284a07c9181b579b061deedcc37af1b6)`(char * buf,const size_t n)` <br/>// Receives a packet. It blocks until a packet is received."
    This function makes sure we recieve a complete packet of size n 
	#### Parameters
	* `buf` Buffer to store the recieved packet 

	* `n` size of packet to receive 

	#### Returns
	number of bytes read, -1 on error

??? example "`public virtual int `[`sendPacket`](#class_udp_data_protocol_1ae6dce595b01110f3de0fd3789e076009)`(const char * buf,const size_t n)` <br/>// Sends a packet."
    This function meakes sure we send a complete packet of size n 
	#### Parameters
	* `buf` Buffer to send 

	* `n` size of packet to receive 

	#### Returns
	number of bytes read, -1 on error

??? example "`public virtual void `[`getPeerAddressFromFirstPacket`](#class_udp_data_protocol_1a270454a13ba5edaa5c08bc30e5f4b750)`(QHostAddress & peerHostAddress,`[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` & port)` <br/>// Obtains the peer address from the first UDP packet received. This address is used by the SERVER mode to connect back to the client."
    #### Parameters
	* `peerHostAddress` QHostAddress to store the peer address 

	* `port` Receiving port

??? example "`public inline void `[`setBindPort`](#class_udp_data_protocol_1a1a3774b6e2d4a6b54b188fa0da2d42ad)`(int port)` <br/>// Sets the bind port number."
    

??? example "`public inline virtual void `[`setPeerPort`](#class_udp_data_protocol_1af5426710f87e9be9223cee673a2c3129)`(int port)` <br/>// Sets the peer port number."
    

??? example "`public virtual void `[`run`](#class_udp_data_protocol_1ad2bab0e15d7d47dd50183047756e7ebb)`()` <br/>// Implements the Thread Loop. To start the thread, call start() ( DO NOT CALL [run()](#class_udp_data_protocol_1ad2bab0e15d7d47dd50183047756e7ebb) )"
    This function creats and binds all the socket and start the connection loop thread.

??? example "`public virtual bool `[`getStats`](#class_udp_data_protocol_1aba79747e34573b1df80f4ba9930954f5)`(`[`PktStat`](undefined.md#struct_data_protocol_1_1_pkt_stat)` * stat)` <br/>// "
    

??? example "`public virtual void `[`setIssueSimulation`](#class_udp_data_protocol_1aeb07e126364599281b39938491f808cb)`(double loss,double jitter,double max_delay)` <br/>// "
    

??? example "`protected int `[`bindSocket`](#class_udp_data_protocol_1a06bfb762aa42f5136af37c877c5073d8)`()` <br/>// Binds the UDP socket to the available address and specified port."
    

??? example "`protected void `[`waitForReady`](#class_udp_data_protocol_1a1666be5719c18459aa169b7c1e0b99e1)`(int timeout_msec)` <br/>// This function blocks until data is available for reading in the socket. The function will timeout after timeout_msec microseconds."
    This function is intended to replace QAbstractSocket::waitForReadyRead which has some problems with multithreading.

	#### Returns
	returns true if there is data available for reading; otherwise it returns false (if an error occurred or the operation timed out)

??? example "`protected virtual void `[`receivePacketRedundancy`](#class_udp_data_protocol_1a4a5c2a4242b67481e59ca90ffb59a8a2)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_redundant_packet,int full_redundant_packet_size,int full_packet_size,`[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` & current_seq_num,`[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` & last_seq_num,`[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` & newer_seq_num)` <br/>// Redundancy algorythm at the receiving end."
    

??? example "`protected virtual void `[`sendPacketRedundancy`](#class_udp_data_protocol_1abfeaa93f4af217ff016a42afb083c414)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * full_redundant_packet,int full_redundant_packet_size,int full_packet_size)` <br/>// Redundancy algorythm at the sender's end."
    

??? example "`{signal} public void `[`signalWaitingTooLong`](#class_udp_data_protocol_1ae93742b3f3af0d543d7044c37eae5d5a)`(int wait_msec)` <br/>// Signals when waiting every 10 milliseconds, with the total wait on wait_msec."
    #### Parameters
	* `wait_msec` Total wait in milliseconds

??? example "`{signal} public void `[`signalUdpWaitingTooLong`](#class_udp_data_protocol_1ae868ea5265c0f61c032bdcd2fa91f704)`()` <br/>// "
    

## struct `HeaderStruct` {#struct_header_struct}

Abstract Header Struct, Header Stucts should subclass it.

### Members

## struct `DefaultHeaderStruct` {#struct_default_header_struct}

```
struct DefaultHeaderStruct
  : public HeaderStruct
```

Default Header Struct.

### Members

??? example "`public uint64_t `[`TimeStamp`](#struct_default_header_struct_1aec92284c78e729f57e43dd6566a266a5) <br/>// Time Stamp."
    

??? example "`public `[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` `[`SeqNumber`](#struct_default_header_struct_1ab2efb238e0487b1d70c5daa56bbca5dd) <br/>// Sequence Number."
    

??? example "`public `[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` `[`BufferSize`](#struct_default_header_struct_1ab5fbb52ffb5b15005fe487c44627cb3d) <br/>// Buffer Size in Samples."
    

??? example "`public `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`SamplingRate`](#struct_default_header_struct_1a3b9b80786cd9376c195a52e08aa366ed) <br/>// Sampling Rate in [JackAudioInterface::samplingRateT](audio_interface.md#class_audio_interface_1a538ea04ceee63f418c629d6a3a0e4fef)."
    

??? example "`public `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`BitResolution`](#struct_default_header_struct_1a6636e8bc0c3673877178e66cb524e0c4) <br/>// Audio Bit Resolution."
    

??? example "`public `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`NumChannels`](#struct_default_header_struct_1ac2f3794bb7d1d69c0f2f2a24813f3f19) <br/>// Number of Channels, we assume input and outputs are the same."
    

??? example "`public `[`uint8_t`](undefined.md#jacktrip__types__alt_8h_1aba7bc1797add20fe3efdf37ced1182c5)` `[`ConnectionMode`](#struct_default_header_struct_1a25e03ecdeab4acfbd8411383f7ab69fd) <br/>// "
    

## struct `JamLinkHeaderStuct` {#struct_jam_link_header_stuct}

```
struct JamLinkHeaderStuct
  : public HeaderStruct
```

JamLink Header Struct.

### Members

??? example "`public `[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` `[`Common`](#struct_jam_link_header_stuct_1adef1d9aa0aa64ac506c76f68e4b99d94) <br/>// Common part of the header, 16 bit."
    

??? example "`public `[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` `[`SeqNumber`](#struct_jam_link_header_stuct_1a046addc1707ca9af63f6d7f1d998e18e) <br/>// Sequence Number."
    

??? example "`public `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`TimeStamp`](#struct_jam_link_header_stuct_1ac352a9d47ca0298ac1e8ba683fabcc72) <br/>// Time Stamp."
    

