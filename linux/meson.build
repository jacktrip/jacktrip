if host_machine.system() == 'linux'
	install_data('org.jacktrip.JackTrip.desktop', install_dir: get_option('datadir') / 'applications')
	install_data('jacktrip.svg', install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps')
	install_data('jacktrip_48x48.png', rename: 'jacktrip.png', install_dir: get_option('datadir') / 'icons' / 'hicolor' / '48x48' / 'apps')

	install_data('org.jacktrip.JackTrip.metainfo.xml', install_dir: get_option('datadir') / 'metainfo')
endif
