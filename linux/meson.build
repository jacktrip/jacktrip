if host_machine.system() == 'linux'
	install_data('icons/jacktrip.svg', rename: '@0@.svg'.format(application_id), install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps')
	install_data('icons/jacktrip_48x48.png', rename: '@0@.png'.format(application_id), install_dir: get_option('datadir') / 'icons' / 'hicolor' / '48x48' / 'apps')

	desktop_conf = configuration_data()
	desktop_conf.set('icon', application_id)
	desktop_conf.set('name_suffix', name_suffix)
	configure_file(
	    input: files('org.jacktrip.JackTrip.desktop.in'),
	    output: '@0@.desktop'.format(application_id),
	    configuration: desktop_conf,
	    install_dir: get_option('datadir') / 'applications'
	)

	appdata_conf = configuration_data()
	appdata_conf.set('appid', application_id)
	configure_file(
	    input: files('org.jacktrip.JackTrip.metainfo.xml.in'),
	    output: '@0@.metainfo.xml'.format(application_id),
	    configuration: appdata_conf,
	    install_dir: get_option('datadir') / 'metainfo'
	)

endif
