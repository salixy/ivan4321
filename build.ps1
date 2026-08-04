$env:Path = [System.Environment]::GetEnvironmentVariable('Path','Machine') + ';' + [System.Environment]::GetEnvironmentVariable('Path','User')
cmake "-DCMAKE_POLICY_VERSION_MINIMUM=3.5" -G Ninja -B build
cmake --build build
