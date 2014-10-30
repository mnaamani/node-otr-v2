{
  'targets': [
    {
      'target_name': 'otrnat',
      'sources': [ "src/otr.cc", "src/userstate.cc", "src/message.cc", "src/context.cc" ],
      'libraries': ['-lotr'],
      'include_dirs': ['/usr/local/include'],
      'library_dirs': [
         '/usr/lib','/usr/local/lib'
      ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          }
        }]
      ]
    }
  ]
}
