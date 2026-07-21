@echo off
setlocal

set ERROR_ON=1

set ENABLE_ASAN=1
set USE_CLANG=1

set "clang_warnings=-pedantic -Wall -Wextra -Werror -Wconversion -Wshadow -Wunreachable-code -Wno-variadic-macro-arguments-omitted -Wno-error=unused-function -Wno-error=unused-variable -Wno-error=unused-parameter"
set "clang_common_flags=-std=gnu11 -I. -ferror-limit=0 -O0 -fno-omit-frame-pointer -g -gcodeview -fuse-ld=lld -D_DLL -D_WIN32_WINNT=0x0501 -lmsvcrt"

REM NOTE: '-D_WIN32_WINNT=0x0501' forces compiler to use APIs that are compatible with Windows XP.
REM @ref: https://www.yoctopuce.com/EN/article/running-on-an-antique-windows-xp

set "cl_warnings=/W4 /WX /wd4146 /wd4210 /wd4310"
set "cl_common_flags=/std:c11 /Od /Zi /Zo /I. /nologo"
set "cl_link_flags=/link /INCREMENTAL:NO"

if %ENABLE_ASAN% EQU 1 (
   set "clang_common_flags=%clang_common_flags% -fsanitize=address"
   set "cl_common_flags=%cl_common_flags% /fsanitize=address"
)

if exist build rmdir /S /Q build
if not exist build mkdir build
if not exist build\grammar mkdir build\grammar
if not exist build\tests mkdir build\tests
if not exist build\tests\eon mkdir build\tests\eon
if not exist build\tests\eon\sanitizers mkdir build\tests\eon\sanitizers

call :compile grammar\check_grammar_soundness.c build\grammar\check_grammar_soundness || exit /B 1

build\grammar\check_grammar_soundness.exe grammar\eon-grammar || exit /B 1

call :compile_and_run_unit_test eon\memory_ut.c || exit /B 1
call :compile_and_run_unit_test eon\containers_ut.c || exit /B 1
call :compile_and_run_unit_test eon\string_ut.c || exit /B 1
call :compile_and_run_unit_test eon\diff_ut.c || exit /B 1

if %USE_CLANG% EQU 1 (
   setlocal
   set "clang_common_flags=%clang_common_flags% -fsanitize=address"
   call :compile_and_run_unit_test eon\sanitizers\asan_ut.c 2>NUL || exit /B 1
   endlocal
) else (
   setlocal
   set "cl_common_flags=%cl_common_flags% /fsanitize=address"
   call :compile_and_run_unit_test eon\sanitizers\asan_ut.c 2>NUL || exit /B 1
   endlocal
)

call :compile_and_run_unit_test eon_lexer_ut.c || exit /B 1
call :compile_and_run_unit_test eon_parser_ut.c || exit /B 1
call :compile_and_run_unit_test eon_ast_ut.c || exit /B 1
call :compile_and_run_unit_test eon_lexical_scopes_ut.c || exit /B 1
call :compile_and_run_unit_test eon_types_ut.c || exit /B 1
call :compile_and_run_unit_test eon_tac_ut.c || exit /B 1
call :compile_and_run_unit_test eon_cfg_ut.c || exit /B 1
call :compile_and_run_unit_test eon_ssa_ut.c || exit /B 1

if not exist build\tests\ssa-tests mkdir build\tests\ssa-tests
call :compile tests\ssa-tests\run_ssa_test.c build\tests\ssa-tests\run_ssa_test || exit /B 1

call :run_ssa_test tests\ssa-tests\general-cases || exit /B 1
call :run_ssa_test tests\ssa-tests\constant-folding || exit /B 1
call :run_ssa_test tests\ssa-tests\loops || exit /B 1

call :run_ssa_test tests\ssa-tests\regression-if-statement-with-return || exit /B 1
call :run_ssa_test tests\ssa-tests\regression-nested-if-statement || exit /B 1
call :run_ssa_test tests\ssa-tests\regression-while-loop-with-break-and-continue || exit /B 1

exit /B %ERRORLEVEL%

REM Usage: call :compile <source-file> <output-file>
:compile

setlocal

echo.
echo Compiling '%1'

set source=%~1
set output=%~2

set starttime="%TIME%"
for /f "tokens=1-4 delims=:., " %%a in ("%starttime%") do (
  set /a start_h=1%%a-100
  set /a start_m=1%%b-100
  set /a start_s=1%%c-100
  set start_ms=%%d
)
set /a start_total_ms = (start_h*3600 + start_m*60 + start_s) * 1000 + start_ms

if %USE_CLANG% EQU 1 (
  clang "%source%" -o "%output%.exe" %clang_warnings% %clang_common_flags% || exit /B 1
) else (
  cl %cl_common_flags% %cl_warnings% "%source%" /Fo:"%output%.obj" /Fe:"%output%.exe" /Fd:"%output%.pdb" %cl_link_flags% || exit /B 1
)

set endtime="%TIME%"
for /f "tokens=1-4 delims=:., " %%a in ("%endtime%") do (
  set /a end_h=1%%a-100
  set /a end_m=1%%b-100
  set /a end_s=1%%c-100
  set end_ms=%%d
)
set /a end_total_ms = (end_h*3600 + end_m*60 + end_s) * 1000 + end_ms
set /a delta_ms = end_total_ms - start_total_ms

set /a secs = delta_ms / 1000
set /a rem_ms = delta_ms %% 1000

echo Compilation took %secs%.%rem_ms% seconds

endlocal

goto :eof

REM Usage: call :compile_and_run_unit_test <test-file> [extra arguments]
:compile_and_run_unit_test
setlocal EnableDelayedExpansion

set test_filename=%~1
shift
REM FIXME: Pass additional args to :compile.
set additional_args=%*

set "test_name=!test_filename!"
if "!test_name:~-5!"=="_ut.c" (
    set "test_name=!test_name:~0,-5!"
) else (
    for %%E in ("!test_name!") do set "test_name=%%~nE"
)

call :compile !test_filename! build\tests\!test_name! || exit /B 1

echo Running tests in '%test_name%'

pushd build\tests
!test_name!.exe --hide-stats || exit /B 1
popd

endlocal

goto :eof

REM Usage: call :run_ssa_test <test-directory>.
:run_ssa_test
setlocal

set test_directory=%~1

:: Remove trailing backslash if it exists
if "%test_directory:~-1%"=="\" set "test_directory=%test_directory:~0,-1%"

:: Extract the last folder
for %%I in ("%test_directory:\=" "%") do set "test_name=%%~I"

echo.
echo Running SSA test '%test_name%'

build\tests\ssa-tests\run_ssa_test.exe %test_directory% || exit /B 1

endlocal

goto :eof
REM NOTE: End of ':run_ssa_test'.
