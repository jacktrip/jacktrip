if host_machine.system() == 'linux'
	install_data('org.jacktrip.JackTrip.desktop', install_dir: get_option('datadir') / 'applications')
	install_data('icons/jacktrip.svg', rename: 'org.jacktrip.JackTrip.svg', install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps')
	install_data('icons/jacktrip_48x48.png', rename: 'org.jacktrip.JackTrip.png', install_dir: get_option('datadir') / 'icons' / 'hicolor' / '48x48' / 'apps')

	install_data('org.jacktrip.JackTrip.metainfo.xml', install_dir: get_option('datadir') / 'metainfo')
endif
