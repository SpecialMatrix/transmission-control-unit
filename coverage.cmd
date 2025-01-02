@echo off

for /r %%G in (*.gcda) do del "%%G"

pio test -v -e native

IF %ERRORLEVEL% EQU 0 (
    IF EXIST "coverage" (
        rmdir /s /q "coverage"
    )

    mkdir "coverage"

    gcovr -r . --html --html-details -o ./coverage/coverage_report.html ^
        --exclude ".pio" ^
        --exclude "test" ^
        --exclude "lib/dbc"
)
