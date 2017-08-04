{
  'variables': {
    'driver%': 'libusb'
  },
  'targets': [
    {
      'defines': [
        '_FILE_OFFSET_BITS=64',
        '_LARGEFILE_SOURCE',
      ],
      'include_dirs+': [
        'src/',
      ],
      'dependencies': [
        'hidapi',
      ],
       'conditions': [
        [ 'OS=="mac"', {
          'target_name': 'hidDarwin',
          'sources': [
            'src/darwin.cc',
            'src/darwinDevice.h',
            'src/darwinDevice.cc'
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
              'IOKit'
            ]
          }
        }],
        [ 'OS=="linux"', {
          'target_name': 'hidLinux',
          'sources': [
            'src/linux.cc',
            'src/linuxDevice.h',
            'src/linuxDevice.cc'
          ],
          'conditions': [
            [ 'driver=="libusb"', {
              'libraries': [
                '-lusb-1.0'
              ]
            }],
            [ 'driver=="hidraw"', {
              'libraries': [
                '-ludev',
                '-lusb-1.0'
              ]
            }]
          ],
        }],
        [ 'OS=="win"', {
          'target_name': 'hidWin',
          'sources': [
            'src/win32.cc',
            'src/win32Device.h',
            'src/win32Device.cc'
          ],
          'defines':[
            'WIN32_LEAN_AND_MEAN',
            'DEBUG'
          ],
          'msvs_settings': {
            'VCLinkerTool': {
              'AdditionalDependencies': [
                'setupapi.lib',
                'WtsApi32.lib'
              ]
            },
            'VCCLCompilerTool': {
              'AdditionalOptions': [ '/EHsc' ],
            },
          }
        }]
      ]
    },
    {
      'target_name': 'hidapi',
      'type': 'static_library',
      'conditions': [
        [ 'OS=="mac"', {
          'sources': [ '../../common/hidapi/mac/hid.c' ],
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
        }],
        [ 'OS=="linux"', {
          'conditions': [
            [ 'driver=="libusb"', {
              'sources': [ '../../common/hidapi/libusb/hid.c' ],
              'include_dirs+': [
                '/usr/include/libusb-1.0/'
              ]
            }],
            [ 'driver=="hidraw"', {
              'sources': [ '../../common/hidapi/linux/hid.c' ]
            }]
          ]
        }],
        [ 'OS=="win"', {
          'sources': [ '../../common/hidapi/windows/hid.c' ],
          'msvs_settings': {
            'VCLinkerTool': {
              'AdditionalDependencies': [
                'setupapi.lib',
                'Cfgmgr32.lib'
              ]
            }
          }
        }]
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../../common/hidapi/hidapi'
        ]
      },
      'include_dirs': [
        '../../common/hidapi/hidapi'
      ],
      'defines': [
        '_LARGEFILE_SOURCE',
        '_FILE_OFFSET_BITS=64',
      ],
      'cflags': ['-g'],
      'cflags!': [
        '-ansi'
      ]
    },
  ]
}
