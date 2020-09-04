jacktrip_sources = ['DataProtocol.cpp',
	'JackTrip.cpp',
	'jacktrip_globals.cpp',
	'JackTripWorker.cpp',
	'PacketHeader.cpp',
	'ProcessPlugin.cpp',
	'RingBuffer.cpp',
	'UdpDataProtocol.cpp',
	'UdpHubListener.cpp',
	'AudioInterface.cpp',
	'JackAudioInterface.cpp',
	'LoopBack.cpp']

libjacktrip = shared_library('libjacktrip',
                        jacktrip_sources,
                        include_directories : inc,
                        dependencies : dependencies,
                        install : true)
