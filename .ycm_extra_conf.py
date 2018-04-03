def FlagsForFile( filename, **kwargs ):
  return {
    'flags': [
        '-x',
        'c++',
        '-std=c++17',
        '-Iff/include',
        '-Ideps/libgflags/include',
    ],
  }
