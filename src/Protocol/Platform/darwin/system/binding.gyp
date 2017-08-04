{
	"targets": [
	{
	  "target_name": "sysDarwin",
    'include_dirs+': [
      'src/',
    ],
	  "sources": [
      "src/system.cc",
      'src/runas_darwin.cc',
      'src/runas.h',
      'src/fork.cc',
      'src/fork.h',
	  ],
	  'defines':[
      'DEBUG'
    ],
    'ldflags': [
      '-framework',
      'IOKit',
      '-framework',
      'CoreFoundation'
    ],
    'xcode_settings': {
      'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
    },
    'link_settings':{
      'libraries':[
        '-framework',
        'IOKit',
        '$(SDKROOT)/System/Library/Frameworks/Security.framework',
      ]
    }
	}
  ]
}
