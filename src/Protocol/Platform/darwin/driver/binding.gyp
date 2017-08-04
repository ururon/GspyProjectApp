{
	"targets": [
	{
	  "target_name": "driverDarwin",
    'include_dirs+': [
      'src/',
    ],
	  "sources": [
      "MouseDriverShareData.h",
      "MouseKeyCommon.h",
      "src/darwinDriver.cpp"
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
        'IOKit'
      ]
    }
	}
  ]
}
