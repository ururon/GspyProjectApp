{
	"targets": [
	{
	  "target_name": "driverWin",
    'include_dirs+': [
      'src/',
    ],
	  "sources": [
      "src/VirtualKeyTable.h"
      "src/filterDriver.h",
      "src/virtualDriver.h",
      "src/win32Driver.cpp"
	  ],
	  'defines':[
      'WIN32_LEAN_AND_MEAN',
      'DEBUG'
    ],
    'msvs_settings': {
      'VCLinkerTool': {
        'AdditionalDependencies': [
          'setupapi.lib',
          'version.lib'
        ]
      },
      'VCCLCompilerTool': {
        'AdditionalOptions': [ '/EHsc' ],
      },
    }
	}
  ]
}
