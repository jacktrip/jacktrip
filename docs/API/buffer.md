# group `buffer` {#group__buffer}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`class `[`JitterBuffer`](#class_jitter_buffer) | [JitterBuffer](#class_jitter_buffer) class.
`class `[`RingBuffer`](#class_ring_buffer) | Provides a ring-buffer (or circular-buffer) that can be written to and read from asynchronously (blocking) or synchronously (non-blocking).
`class `[`RingBufferWavetable`](#class_ring_buffer_wavetable) | Same as [RingBuffer](#class_ring_buffer), except that it uses the Wavetable mode for lost or late packets.

## class `JitterBuffer` {#class_jitter_buffer}

```
class JitterBuffer
  : public RingBuffer
```

[JitterBuffer](#class_jitter_buffer) class.

### Members

??? example "`public  `[`JitterBuffer`](#class_jitter_buffer_1af938b36a7a96ef91ac4f70e57f624ccf)`(int buf_samples,int qlen,int sample_rate,int strategy,int bcast_qlen,int channels,int bit_res)` <br/>// "
    

??? example "`public inline virtual  `[`~JitterBuffer`](#class_jitter_buffer_1a98c5102e6ae7d6ed41693cff4197c6ed)`()` <br/>// "
    

??? example "`public virtual bool `[`insertSlotNonBlocking`](#class_jitter_buffer_1aaa2988c291da05963e2d0e3e11186b0a)`(const `[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * ptrToSlot,int len,int lostLen)` <br/>// Same as insertSlotBlocking but non-blocking (asynchronous)"
    #### Parameters
	* `ptrToSlot` Pointer to slot to insert into the [RingBuffer](#class_ring_buffer)

??? example "`public virtual void `[`readSlotNonBlocking`](#class_jitter_buffer_1a782bace3f543be0e5659fe6dc50be4bc)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * ptrToReadSlot)` <br/>// Same as readSlotBlocking but non-blocking (asynchronous)"
    #### Parameters
	* `ptrToReadSlot` Pointer to read slot from the [RingBuffer](#class_ring_buffer)

??? example "`public virtual void `[`readBroadcastSlot`](#class_jitter_buffer_1ab132a04a1d122e04d3dbffd957f72583)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * ptrToReadSlot)` <br/>// "
    

??? example "`public virtual bool `[`getStats`](#class_jitter_buffer_1a21e028e3dcf94fb9620f47a283aabf1c)`(`[`IOStat`](undefined.md#struct_ring_buffer_1_1_i_o_stat)` * stat,bool reset)` <br/>// "
    

??? example "`protected int `[`mMaxLatency`](#class_jitter_buffer_1a8c00a7cb5f722e600ccc732daafe5d3a) <br/>// "
    

??? example "`protected int `[`mNumChannels`](#class_jitter_buffer_1ae0ee2485022ef3a55c5052cf9a9a65f6) <br/>// "
    

??? example "`protected int `[`mAudioBitRes`](#class_jitter_buffer_1a99201de1a975dbb9ff0440a9d580c5c4) <br/>// "
    

??? example "`protected int `[`mMinStepSize`](#class_jitter_buffer_1a2ae830cd17acefa92fc96dd626f7f5a9) <br/>// "
    

??? example "`protected int `[`mFPP`](#class_jitter_buffer_1ab2908f7ecfd38ac0e0c6c08b3a748ac9) <br/>// "
    

??? example "`protected int `[`mSampleRate`](#class_jitter_buffer_1a796063cfade8f77ad44f9241391e24a0) <br/>// "
    

??? example "`protected int `[`mInSlotSize`](#class_jitter_buffer_1a195fa1196aadc3b71d2898e57e237e28) <br/>// "
    

??? example "`protected bool `[`mActive`](#class_jitter_buffer_1ae463927c1e18b533273161152c21bb95) <br/>// "
    

??? example "`protected `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`mBroadcastLatency`](#class_jitter_buffer_1aaa5315581a2631c3f019027a4ec03337) <br/>// "
    

??? example "`protected `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`mBroadcastPosition`](#class_jitter_buffer_1ad738e91254e00a44736c249583cd4ed0) <br/>// "
    

??? example "`protected double `[`mBroadcastPositionCorr`](#class_jitter_buffer_1acbbd24359f4f5dee9ed67cce205c4c1d) <br/>// "
    

??? example "`protected double `[`mUnderrunIncTolerance`](#class_jitter_buffer_1a9c38d00d318c5728412733108168e85c) <br/>// "
    

??? example "`protected double `[`mCorrIncTolerance`](#class_jitter_buffer_1ab6cd3a244709a6b705a922fc332827ee) <br/>// "
    

??? example "`protected double `[`mOverflowDecTolerance`](#class_jitter_buffer_1a051ea23ffd5b13dd3528ee8acba22c9f) <br/>// "
    

??? example "`protected int `[`mOverflowDropStep`](#class_jitter_buffer_1ae1e0714984fa37c0da7ef9061ca48486) <br/>// "
    

??? example "`protected `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`mLastCorrCounter`](#class_jitter_buffer_1a24baff465c197a23b73b058e2983de79) <br/>// "
    

??? example "`protected int `[`mLastCorrDirection`](#class_jitter_buffer_1a0de7a53543dee89ca90066e80cc62f7f) <br/>// "
    

??? example "`protected double `[`mMinLevelThreshold`](#class_jitter_buffer_1a088e303aa49a1d4bc488103a85fc8d64) <br/>// "
    

??? example "`protected int `[`mAutoQueue`](#class_jitter_buffer_1a341a4f873b636d0cecb5c5f9e40e952b) <br/>// "
    

??? example "`protected double `[`mAutoQueueCorr`](#class_jitter_buffer_1a95ba9ee77ee2580dec909a7371d08bbc) <br/>// "
    

??? example "`protected double `[`mAutoQFactor`](#class_jitter_buffer_1ab82ae06640f248379b3a00d1501f6654) <br/>// "
    

??? example "`protected double `[`mAutoQRate`](#class_jitter_buffer_1a35d7211869f5067a1cd64554f2b4977d) <br/>// "
    

??? example "`protected double `[`mAutoQRateMin`](#class_jitter_buffer_1aa38287b1340b0a6883a5411fe8d92f23) <br/>// "
    

??? example "`protected double `[`mAutoQRateDecay`](#class_jitter_buffer_1a7b1822becbd4d0d72ed020b84648cfac) <br/>// "
    

??? example "`protected void `[`processPacketLoss`](#class_jitter_buffer_1a6f87039ae6131e54cb15c2fe4d0f77b7)`(int lostLen)` <br/>// "
    

??? example "`protected inline double `[`lastCorrFactor`](#class_jitter_buffer_1ada79d016499d1b2814a08d147bb46d81)`() const` <br/>// "
    

## class `RingBuffer` {#class_ring_buffer}

Provides a ring-buffer (or circular-buffer) that can be written to and read from asynchronously (blocking) or synchronously (non-blocking).

The [RingBuffer](#class_ring_buffer) is an array of **NumSlots** slots of memory each of which is of size **SlotSize** bytes (8-bits). Slots can be read and written asynchronously/synchronously by multiple threads.

### Members

??? example "`public  `[`RingBuffer`](#class_ring_buffer_1a64733f6fb0f21e5322a73e48c5074630)`(int SlotSize,int NumSlots)` <br/>// The class constructor."
    #### Parameters
	* `SlotSize` Size of one slot in bytes 

	* `NumSlots` Number of slots

??? example "`public virtual  `[`~RingBuffer`](#class_ring_buffer_1a2715b2e99ea24521ef7a586c2f33e1c9)`()` <br/>// The class destructor."
    

??? example "`public void `[`insertSlotBlocking`](#class_ring_buffer_1a8f2e273c7ec107b4ffbb39b75cb0c58e)`(const `[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * ptrToSlot)` <br/>// Insert a slot into the [RingBuffer](#class_ring_buffer) from ptrToSlot. This method will block until there's space in the buffer."
    The caller is responsible to make sure sizeof(WriteSlot) = SlotSize. This method should be use when the caller can block against its output, like sending/receiving UDP packets. It shouldn't be used by audio. For that, use the insertSlotNonBlocking. 
	#### Parameters
	* `ptrToSlot` Pointer to slot to insert into the [RingBuffer](#class_ring_buffer)

??? example "`public void `[`readSlotBlocking`](#class_ring_buffer_1a9345a4fb1520abee6db7cf1ae79119f9)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * ptrToReadSlot)` <br/>// Read a slot from the [RingBuffer](#class_ring_buffer) into ptrToReadSlot. This method will block until there's space in the buffer."
    The caller is responsible to make sure sizeof(ptrToReadSlot) = SlotSize. This method should be use when the caller can block against its input, like sending/receiving UDP packets. It shouldn't be used by audio. For that, use the readSlotNonBlocking. 
	#### Parameters
	* `ptrToReadSlot` Pointer to read slot from the [RingBuffer](#class_ring_buffer)

??? example "`public virtual bool `[`insertSlotNonBlocking`](#class_ring_buffer_1a6a45eac1a0ee188c2554375a7b7f4440)`(const `[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * ptrToSlot,int len,int lostLen)` <br/>// Same as insertSlotBlocking but non-blocking (asynchronous)"
    #### Parameters
	* `ptrToSlot` Pointer to slot to insert into the [RingBuffer](#class_ring_buffer)

??? example "`public virtual void `[`readSlotNonBlocking`](#class_ring_buffer_1a5abaeda3b1bab8ddadcfcb5225649853)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * ptrToReadSlot)` <br/>// Same as readSlotBlocking but non-blocking (asynchronous)"
    #### Parameters
	* `ptrToReadSlot` Pointer to read slot from the [RingBuffer](#class_ring_buffer)

??? example "`public virtual void `[`readBroadcastSlot`](#class_ring_buffer_1a51008be7b5dfd9d222418ff6fa51b6c2)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * ptrToReadSlot)` <br/>// "
    

??? example "`public virtual bool `[`getStats`](#class_ring_buffer_1a9c4364d7dd503ceff1197f27f8fe118e)`(`[`IOStat`](undefined.md#struct_ring_buffer_1_1_i_o_stat)` * stat,bool reset)` <br/>// "
    

??? example "`protected int `[`mSlotSize`](#class_ring_buffer_1a97d21a2d5c64d818258d69e2b356ea6d) <br/>// The size of one slot in byes."
    

??? example "`protected int `[`mNumSlots`](#class_ring_buffer_1a26918ada2ac7c16e86dfdedc7792bcb0) <br/>// Number of Slots."
    

??? example "`protected int `[`mTotalSize`](#class_ring_buffer_1a7af32873fff101269c65f8946f2489e3) <br/>// Total size of the mRingBuffer = mSlotSize*mNumSlotss."
    

??? example "`protected `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`mReadPosition`](#class_ring_buffer_1a38869b6a28b562daa50d9a0e3f55dbd1) <br/>// Read Positions in the [RingBuffer](#class_ring_buffer) (Tail)"
    

??? example "`protected `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`mWritePosition`](#class_ring_buffer_1aba835bdf8fe9988376a75200299ef69c) <br/>// Write Position in the [RingBuffer](#class_ring_buffer) (Head)"
    

??? example "`protected int `[`mFullSlots`](#class_ring_buffer_1ac84c18a0695aae44d0c3da9628ff58fd) <br/>// Number of used (full) slots, in slot-size."
    

??? example "`protected `[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * `[`mRingBuffer`](#class_ring_buffer_1a5bcd87978fdf9ff2b6922d245f52f8fc) <br/>// 8-bit array of data (1-byte)"
    

??? example "`protected `[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * `[`mLastReadSlot`](#class_ring_buffer_1af2d46520b405499a6ee77d0c6f5a1a98) <br/>// Last slot read."
    

??? example "`protected QMutex `[`mMutex`](#class_ring_buffer_1af16728157c54e722b74417a6428648e1) <br/>// Mutex to protect read and write operations."
    

??? example "`protected QWaitCondition `[`mBufferIsNotFull`](#class_ring_buffer_1a804c25e2c5b59f1385288458298678a9) <br/>// Buffer not full condition to monitor threads."
    

??? example "`protected QWaitCondition `[`mBufferIsNotEmpty`](#class_ring_buffer_1ae1e669f78e821973c490cb3f20d8aede) <br/>// Buffer not empty condition to monitor threads."
    

??? example "`protected int `[`mStatUnit`](#class_ring_buffer_1afe5d080bc86eceae0028f8409bfda0cd) <br/>// "
    

??? example "`protected `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`mUnderruns`](#class_ring_buffer_1ace5ea1f91a02085e7330d447ffdcf016) <br/>// "
    

??? example "`protected `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`mOverflows`](#class_ring_buffer_1a1fa2a19ab586a5fb92233cecf694ed5e) <br/>// "
    

??? example "`protected `[`int32_t`](undefined.md#jacktrip__types__alt_8h_1a32f2e37ee053cf2ce8ca28d1f74630e5)` `[`mSkewRaw`](#class_ring_buffer_1acb6755987d86fa7497cad34afca7d575) <br/>// "
    

??? example "`protected double `[`mLevelCur`](#class_ring_buffer_1a0b0c235a10c711d41d78646bdac744c2) <br/>// "
    

??? example "`protected double `[`mLevelDownRate`](#class_ring_buffer_1a1c8adca1f4ae760537b2856dcd79fe30) <br/>// "
    

??? example "`protected `[`int32_t`](undefined.md#jacktrip__types__alt_8h_1a32f2e37ee053cf2ce8ca28d1f74630e5)` `[`mLevel`](#class_ring_buffer_1a4b928b56dfeaf67812a193f52a0e085a) <br/>// "
    

??? example "`protected `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`mBufDecOverflow`](#class_ring_buffer_1a5af477f277d8acdb2f10161faf26db98) <br/>// "
    

??? example "`protected `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`mBufDecPktLoss`](#class_ring_buffer_1aea0ca39ab6d1051266f0e0b3ffcd2dc4) <br/>// "
    

??? example "`protected `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`mBufIncUnderrun`](#class_ring_buffer_1aae8f3a24a42d08589a48160a12c2d887) <br/>// "
    

??? example "`protected `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`mBufIncCompensate`](#class_ring_buffer_1a072831777ae48c84aceb9bc9659f1207) <br/>// "
    

??? example "`protected `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`mReadsNew`](#class_ring_buffer_1aa33f6a109b73cc9cf4778c3cd06c48b3) <br/>// "
    

??? example "`protected `[`uint32_t`](undefined.md#jacktrip__types__alt_8h_1a435d1572bf3f880d55459d9805097f62)` `[`mUnderrunsNew`](#class_ring_buffer_1a8bca304d2af59247ff86be7774b6add9) <br/>// "
    

??? example "`protected `[`int32_t`](undefined.md#jacktrip__types__alt_8h_1a32f2e37ee053cf2ce8ca28d1f74630e5)` `[`mSkew0`](#class_ring_buffer_1ada4302f6b5bd6731dd4da819261d19e6) <br/>// "
    

??? example "`protected `[`int32_t`](undefined.md#jacktrip__types__alt_8h_1a32f2e37ee053cf2ce8ca28d1f74630e5)` `[`mBroadcastSkew`](#class_ring_buffer_1a5ddf1339ed82e7a85fee8002729898b4) <br/>// "
    

??? example "`protected `[`int32_t`](undefined.md#jacktrip__types__alt_8h_1a32f2e37ee053cf2ce8ca28d1f74630e5)` `[`mBroadcastDelta`](#class_ring_buffer_1a79e2e3bb52c4484cdad7f96664f2ffba) <br/>// "
    

??? example "`protected virtual void `[`setUnderrunReadSlot`](#class_ring_buffer_1aef3d8f404901086d9190b0acec2a5443)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * ptrToReadSlot)` <br/>// Sets the memory in the Read Slot when uderrun occurs. By default, this sets it to 0. Override this method in a subclass for a different behavior."
    #### Parameters
	* `ptrToReadSlot` Pointer to read slot from the [RingBuffer](#class_ring_buffer)

??? example "`protected virtual void `[`setMemoryInReadSlotWithLastReadSlot`](#class_ring_buffer_1ab6124a2d65e64656962168f14832c04c)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * ptrToReadSlot)` <br/>// Uses the last read slot to set the memory in the Read Slot."
    The last read slot is the last packet that arrived, so if no new packets are received, it keeps looping the same packet. 
	#### Parameters
	* `ptrToReadSlot` Pointer to read slot from the [RingBuffer](#class_ring_buffer)

??? example "`protected void `[`underrunReset`](#class_ring_buffer_1aa511a08f13237bd1612135ed4b46d389)`()` <br/>// Resets the ring buffer for reads under-runs non-blocking."
    

??? example "`protected void `[`overflowReset`](#class_ring_buffer_1acbc4eeccd874fc0c6a9d6d29c141471f)`()` <br/>// Resets the ring buffer for writes over-flows non-blocking."
    

??? example "`protected void `[`debugDump`](#class_ring_buffer_1a27c8d1d7e5778c6eefd4cc7da5e33d13)`() const` <br/>// Helper method to debug, prints member variables to terminal."
    

??? example "`protected void `[`updateReadStats`](#class_ring_buffer_1a766bc7dbb4b04484b34f538f3e7a3f8a)`()` <br/>// "
    

## class `RingBufferWavetable` {#class_ring_buffer_wavetable}

```
class RingBufferWavetable
  : public RingBuffer
```

Same as [RingBuffer](#class_ring_buffer), except that it uses the Wavetable mode for lost or late packets.

### Members

??? example "`public inline  `[`RingBufferWavetable`](#class_ring_buffer_wavetable_1a7d100b483154cae83b1d7b2f1f974fed)`(int SlotSize,int NumSlots)` <br/>// The class constructor."
    #### Parameters
	* `SlotSize` Size of one slot in bytes 

	* `NumSlots` Number of slots

??? example "`public inline virtual  `[`~RingBufferWavetable`](#class_ring_buffer_wavetable_1a6e38cddb5eb0c4a5b8146a38094156fa)`()` <br/>// The class destructor."
    

??? example "`protected inline virtual void `[`setUnderrunReadSlot`](#class_ring_buffer_wavetable_1a853d18899e8f82c79e876a7fb691675b)`(`[`int8_t`](undefined.md#jacktrip__types__alt_8h_1aef44329758059c91c76d334e8fc09700)` * ptrToReadSlot)` <br/>// Sets the memory in the Read Slot when uderrun occurs. This loops as a wavetable in the last received packet."
    #### Parameters
	* `ptrToReadSlot` Pointer to read slot from the [RingBuffer](#class_ring_buffer)

