// NOTE(vlad): Windows uses the declspec-syntax like '__declspec(noreturn)' which
//             conflicts with the 'noreturn' macros, thus we need to undef it before
//             including the Windows-specific headers.
//             @ref: https://stackoverflow.com/a/57366903
#if defined(noreturn)
#    undef noreturn
#endif
