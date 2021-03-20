# group `threading` {#group__threading}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`class `[`JackTripThread`](#class_jack_trip_thread) | Test class that runs [JackTrip](main.md#class_jack_trip) inside a thread.
`class `[`JackTripWorker`](#class_jack_trip_worker) | Prototype of the worker class that will be cloned through sending threads to the Thread Pool.
`class `[`JackTripWorkerMessages`](#class_jack_trip_worker_messages) | 

## class `JackTripThread` {#class_jack_trip_thread}

```
class JackTripThread
  : public QThread
```

Test class that runs [JackTrip](main.md#class_jack_trip) inside a thread.

### Members

??? example "`public inline  `[`JackTripThread`](#class_jack_trip_thread_1a82b09ae2f04a5a4df14b1d754df38eaa)`(`[`JackTrip::jacktripModeT`](main.md#class_jack_trip_1a062df7ced6d5d7f3dd47b5d05c393982)` JacktripMode)` <br/>// "
    

??? example "`public inline virtual  `[`~JackTripThread`](#class_jack_trip_thread_1a4a192e66061357c489d0ed9316aa3264)`()` <br/>// "
    

??? example "`public void `[`run`](#class_jack_trip_thread_1aef1f596c146bcbb1d4bf078432c0046a)`()` <br/>// "
    

??? example "`public inline void `[`setPort`](#class_jack_trip_thread_1a689dc66090018cefa3666797dc5ae9a1)`(int port_num)` <br/>// "
    

??? example "`public inline void `[`setPeerAddress`](#class_jack_trip_thread_1a1aab9303a522ae5b60557a06cc02b691)`(const char * PeerHostOrIP)` <br/>// "
    

## class `JackTripWorker` {#class_jack_trip_worker}

```
class JackTripWorker
  : public QObject
  : public QRunnable
```

Prototype of the worker class that will be cloned through sending threads to the Thread Pool.

This class can be send to the ThreadPool using the start() method. Each time it is sent, it'll became "independent" of the prototype, which means that the prototype state can be changed, and used to send and start another thread into the pool. setAutoDelete must be set to false in order for this to work.

### Members

??? example "`public bool `[`mAppendThreadID`](#class_jack_trip_worker_1aa817be509000f3fefdfb24cbc73c06a2) <br/>// "
    

??? example "`public  `[`JackTripWorker`](#class_jack_trip_worker_1a05a68ebb2cfe3b62119634bc99df9e3a)`(`[`UdpHubListener`](main.md#class_udp_hub_listener)` * udphublistener,int BufferQueueLength,`[`JackTrip::underrunModeT`](main.md#class_jack_trip_1a9fd34b5de9751d9bafbb03a7c2250bf5)` UnderRunMode,QString clientName)` <br/>// The class constructor."
    

??? example "`public virtual  `[`~JackTripWorker`](#class_jack_trip_worker_1a63f9e3139958af558be7d2aa7b44375d)`()` <br/>// The class destructor."
    

??? example "`public void `[`run`](#class_jack_trip_worker_1aa0905f8624d47299941ab07e3250abc6)`()` <br/>// Implements the Thread Loop. To start the thread, call start() ( DO NOT CALL [run()](#class_jack_trip_worker_1aa0905f8624d47299941ab07e3250abc6) )."
    

??? example "`public bool `[`isSpawning`](#class_jack_trip_worker_1a59e45e7c4dcd52a36f194f37c6fcd4c4)`()` <br/>// Check if the Thread is Spawning."
    #### Returns
	true is it is spawning, false if it's already running

??? example "`public void `[`setJackTrip`](#class_jack_trip_worker_1adfab864b6ea0d219cce6243105b3cc1e)`(int id,QString client_address,`[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` server_port,`[`uint16_t`](undefined.md#jacktrip__types__alt_8h_1adf4d876453337156dde61095e1f20223)` client_port,int num_channels,bool connectDefaultAudioPorts)` <br/>// Sets the [JackTripWorker](#class_jack_trip_worker) properties."
    #### Parameters
	* `id` ID number 

	* `address`

??? example "`public void `[`stopThread`](#class_jack_trip_worker_1abc04f4bd01383d6e7c09360629445f8d)`()` <br/>// Stop and remove thread from pool."
    

??? example "`public inline int `[`getID`](#class_jack_trip_worker_1a0f7cbe18148fb8ffc83b32747f5316a7)`()` <br/>// "
    

??? example "`public inline void `[`setBufferStrategy`](#class_jack_trip_worker_1a144e002ce3a4861759117d4f1ff820a3)`(int BufferStrategy)` <br/>// "
    

??? example "`public inline void `[`setNetIssuesSimulation`](#class_jack_trip_worker_1a64fbf19fa8880a8015f0f2a7939c5feb)`(double loss,double jitter,double delay_rel)` <br/>// "
    

??? example "`public inline void `[`setBroadcast`](#class_jack_trip_worker_1aaf37f3764c0043a63bdd2a029dba4d20)`(int broadcast_queue)` <br/>// "
    

??? example "`public inline void `[`setUseRtUdpPriority`](#class_jack_trip_worker_1a660c8d99b9c41e87337a28ab7d0dbb80)`(bool use)` <br/>// "
    

??? example "`public inline void `[`setIOStatTimeout`](#class_jack_trip_worker_1a3f44e06a40b35e8fe1c888eb44fe1e26)`(int timeout)` <br/>// "
    

??? example "`public inline void `[`setIOStatStream`](#class_jack_trip_worker_1a638580c1f29ffad2af54abd6fc563eb4)`(QSharedPointer< std::ofstream > statStream)` <br/>// "
    

??? example "`{signal} public void `[`signalRemoveThread`](#class_jack_trip_worker_1a9acc5749ea776ab652d63e70e94dfad3)`()` <br/>// "
    

## class `JackTripWorkerMessages` {#class_jack_trip_worker_messages}

```
class JackTripWorkerMessages
  : public QObject
```

### Members

??? example "`public inline  `[`JackTripWorkerMessages`](#class_jack_trip_worker_messages_1a691d6b469b0b7e08238ba2c556f4f665)`()` <br/>// "
    

??? example "`public inline virtual  `[`~JackTripWorkerMessages`](#class_jack_trip_worker_messages_1a15e7841d383f226e4d739bed3f353060)`()` <br/>// "
    

??? example "`public inline void `[`play`](#class_jack_trip_worker_messages_1a53868d2c0059cfbeb5cca9169c594f88)`()` <br/>// "
    

??? example "`{signal} public void `[`signalTest`](#class_jack_trip_worker_messages_1a46aae282c6b1d0c41e7c2bca96c3d82d)`()` <br/>// "
    

??? example "`{signal} public void `[`signalStopEventLoop`](#class_jack_trip_worker_messages_1a304f126c9a03f9e8919f355e38dee17e)`()` <br/>// Signal to stop the event loop inside the [JackTripWorker](#class_jack_trip_worker) Thread."
    

??? example "`{slot} public inline void `[`slotTest`](#class_jack_trip_worker_messages_1ae7e3f4277e090946e62443eb52973b1c)`()` <br/>// "
    

