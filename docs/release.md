# Release and packaging

## Create distributable archives

From the repository root, create a clean Release build and invoke CPack:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
cpack --config build/CPackConfig.cmake -B dist
```

CPack emits platform-labelled `.zip` and `.tar.gz` archives in `dist/`. They contain the `sanbucraft_ai` executable plus the README and license. Runtime world databases, log files, configuration files, API keys, and RCON passwords are deliberately not packaged.

## Release gate

Publish only after all CTest tests pass and after manually testing against a copy of a Minecraft world. The production RCON transport is currently disabled by design: release artifacts are safe for read-only analysis but do not claim remote world-control support.
