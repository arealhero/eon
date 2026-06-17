@echo off
setlocal

set ERROR_ON=1

set USE_CLANG=1

set clang_warnings=^
    -pedantic ^
    -Wall ^
    -Wextra ^
    -Werror ^
    -Wconversion ^
    -Wshadow ^
    -Wunreachable-code ^
    -Wno-variadic-macro-arguments-omitted ^
    -Wno-error=unused-function ^
    -Wno-error=unused-variable ^
    -Wno-error=unused-parameter

REM TODO(vlad): Find a way to enable ASAN here.
set clang_common_flags=^
    -std=gnu11 ^
    -O0 ^
    -g ^
    -I. ^
    -fno-omit-frame-pointer ^
    -Wl,/INCREMENTAL:NO

set cl_warnings=^
    /W4 ^
    /WX ^
    /wd4146 ^
    /wd4210 ^
    /wd4310

set cl_common_flags=^
    /std:c11 ^
    /Od ^
    /Zi ^
    /Zo ^
    /I. ^
    /nologo ^
    /fsanitize=address

set cl_link_flags=/INCREMENTAL:NO

if not exist build mkdir build
if not exist build\grammar mkdir build\grammar
if not exist build\tests mkdir build\tests
if not exist build\tests\eon mkdir build\tests\eon
if not exist build\tests\eon\sanitizers mkdir build\tests\eon\sanitizers

call :compile grammar\check_grammar_soundness.c ^
              build\grammar\check_grammar_soundness

build\grammar\check_grammar_soundness.exe grammar\eon-grammar

call :compile_and_run_unit_test eon\memory_ut.c || exit /B 1
call :compile_and_run_unit_test eon\string_ut.c || exit /B 1
call :compile_and_run_unit_test eon_lexer_ut.c || exit /B 1
call :compile_and_run_unit_test eon_parser_ut.c || exit /B 1
call :compile_and_run_unit_test eon_lexical_scopes_ut.c || exit /B 1
call :compile_and_run_unit_test eon_types_ut.c || exit /B 1
call :compile_and_run_unit_test eon_tac_ut.c || exit /B 1

REM FIXME(vlad): Enable this test on Windows (clang-cl's ASAN does not work for some reason;
REM              also this test does not work for cl.exe because it uses unix-specific headers).
REM call :compile_and_run_unit_test eon/sanitizers/asan_ut.c -fsanitize=address -fsanitize-recover=address

exit /B %ERRORLEVEL%

REM NOTE(vlad): Usage: call :compile <source-file> <output-file>
:compile
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
  clang "%source%" -o "%output%.exe" ^
         %clang_warnings% ^
         %clang_common_flags% ^
         || exit /B 1
) else (
  cl %cl_common_flags% %cl_warnings% ^
     "%source%" ^
     /Fo:"%output%.obj" ^
     /Fe:"%output%.exe" ^
     /Fd:"%output%.pdb" ^
     %cl_link_flags% ^
     || exit /B 1
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

goto :eof

REM NOTE(vlad): Usage: call :compile_and_run_unit_test <test-file> [extra arguments]
:compile_and_run_unit_test
setlocal EnableDelayedExpansion

set test_filename=%~1
shift
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

endlocal
