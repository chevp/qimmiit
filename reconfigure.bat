@echo off
REM Reconfigure CMake project to regenerate protocol buffers
echo Reconfiguring CMake project...
echo.

cd build
cmake ..

echo.
echo Done! Protocol buffers should now be generated.
echo Check:
echo   - arctic-formats/generated/cpp/
echo   - cryo-protocol/generated/cpp/
