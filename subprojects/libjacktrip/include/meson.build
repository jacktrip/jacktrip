moc_h = ['DataProtocol.h',
	'JackTrip.h',
	'JackTripWorker.h',
	'PacketHeader.h',
	'UdpDataProtocol.h',
	'UdpHubListener.h']
moc_files = qt5.preprocess(moc_headers : moc_h)

install_headers(['JackTrip.h',
	'UdpHubListener.h',
	'DataProtocol.h',
	'ProcessPlugin.h',
	'AudioInterface.h',
	'JackAudioInterface.h',
	'PacketHeader.h',
	'jacktrip_globals.h',
	'RingBuffer.h',
	'LoopBack.h'],
	subdir: 'jacktrip')
