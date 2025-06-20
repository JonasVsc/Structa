@echo off

set project_directory=
set output_directory=
set msvc_compiler_flags=/MTd /nologo /Od /W4 /Za /Zi /wd4100 /wd4101 /wd4189
set msvc_linker_flags=/DEBUG:FASTLINK /INCREMENTAL:NO /OPT:REF

pushd %cd%
cd %~dp0
set project_directory=%cd%
popd

set output_directory=%project_directory%\build

rmdir "%output_directory%"
mkdir "%output_directory%"

pushd "%output_directory%"
    cls
    if %errorlevel% == 0 (cl %msvc_compiler_flags% /LD "%project_directory%\src\experimental\platform.c" /link %msvc_linker_flags% kernel32.lib user32.lib)
    if %errorlevel% == 0 (cl %msvc_compiler_flags% "%project_directory%\src\experimental\main.c" /link %msvc_linker_flags% platform.lib)
popd