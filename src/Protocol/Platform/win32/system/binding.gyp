{
	"targets": [
	{
	  "target_name": "sysWin",
    'include_dirs+': [
      'src/',
    ],
	  "sources": [ 
      "src/hidApi.h",
      "src/system.cc"
	  ],
	  'defines':[
      'WIN32_LEAN_AND_MEAN',
      'DEBUG'
    ],
    'msvs_disabled_warnings': [
      4530,  # C++ exception handler used, but unwind semantics are not enabled
      4506,  # no definition for inline function
    ],
    'msvs_settings': {
      'VCLinkerTool': {
        'AdditionalDependencies': [
          'setupapi.lib',
          'WtsApi32.lib',
          'psapi.lib',
          'ole32.lib',
          'shell32.lib'
        ]
      },
      'VCCLCompilerTool': {
        'AdditionalOptions': [ '/EHsc' ],
      },
    }
	}
  ]
}
