if host_machine.system() == 'linux'
	install_data('icons/jacktrip.svg', rename: '@0@.svg'.format(application_id), install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps')
	install_data('icons/jacktrip_48x48.png', rename: '@0@.png'.format(application_id), install_dir: get_option('datadir') / 'icons' / 'hicolor' / '48x48' / 'apps')
	install_data('icons/jacktrip-symbolic.svg', rename: '@0@-symbolic.svg'.format(application_id), install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'symbolic' / 'apps')
	desktop_conf = configuration_data()
	desktop_conf.set('icon', application_id)
	desktop_conf.set('wmclass', application_id.to_lower())
	desktop_conf.set('name_suffix', name_suffix)
	configure_file(
	    input: files('org.jacktrip.JackTrip.desktop.in'),
	    output: '@0@.desktop'.format(application_id),
	    configuration: desktop_conf,
	    install_dir: get_option('datadir') / 'applications'
	)

	prog_python = import('python').find_installation('python3')

	metainfo_in = configure_file(
		input: ['add_changelog_to_metainfo.py', 'org.jacktrip.JackTrip.metainfo.xml.in.in'],
		output: 'org.jacktrip.JackTrip.metainfo.xml.in',
		command: [prog_python, '@INPUT@', '@OUTPUT@']
	)

	appdata_conf = configuration_data()
	appdata_conf.set('appid', application_id)
	configure_file(
	    input: metainfo_in,
	    output: '@0@.metainfo.xml'.format(application_id),
	    configuration: appdata_conf,
	    install_dir: get_option('datadir') / 'metainfo'
	)

endif
