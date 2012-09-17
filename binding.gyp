{
  'targets': [
    {
      'target_name': 'otrnat',
      'type': 'shared_library',
      'sources': [ "src/otr.cc", "src/userstate.cc", "src/message.cc", "src/context.cc" ],
      'libraries': ['-lotr'],
      'library_dirs': [
         '/usr/lib','/usr/local/lib'
      ],
    }
  ]
}
