// NOTE(vlad): Windows uses the declspec-syntax like '__declspec(noreturn)' which
//             conflicts with the 'noreturn' macros, thus we need to undef it before
//             including the Windows-specific headers.
//             @ref: https://stackoverflow.com/a/57366903
#if defined(noreturn)
#    undef noreturn
#endif

// NOTE(vlad): This prevents the warning C5105 inside 'winbase.h': macro expansion producing 'defined' has undefined
//             behavior. AFAIK this was fixed in newer SDK versions (newer than 10.0.19041.0, that is), but we will
//             silence it anyway.
MSVC_PUSH_DIAGNOSTIC()
MSVC_IGNORE_WARNING(5105)
