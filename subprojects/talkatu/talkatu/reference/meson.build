talkatu_doc_content_files = []

if get_option('doc')
	talkatu_toml = configure_file(
		input : 'talkatu.toml.in',
		output : 'talkatu.toml',
		configuration : version_conf,
		install : true,
		install_dir : docs_dir / 'talkatu',
	)

	talkatu_doc = custom_target('talkatu-doc',
		input : [ talkatu_toml, talkatu_gir[0] ],
		output : 'talkatu',
		command : [
			gidocgen,
			'generate',
			'--quiet',
			'--fatal-warnings',
			'--config=@INPUT0@',
			'--output-dir=@OUTPUT@',
			'--no-namespace-dir',
			'--content-dir=@0@'.format(meson.current_source_dir()),
			'@INPUT1@'
		],
		depend_files : [ talkatu_doc_content_files ],
		build_by_default : true,
		install : true,
		install_dir : docs_dir,
	)
endif

