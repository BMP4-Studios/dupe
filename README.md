[![Build](https://github.com/vberthiaume/dupe/actions/workflows/build_and_test.yml/badge.svg?branch=main)](https://github.com/vberthiaume/dupe/actions/workflows/build_and_test.yml)
[![License: AGPL v3](https://img.shields.io/badge/License-AGPL_v3-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)

# Dupe — micro pitch-shifter
A JUCE audio plugin that doubles a mono signal across the stereo field with two micro pitch-shifted voices. One voice shifts up by `Pitch` cents, the other down by the same amount; the up-shifted voice goes to the left channel and the down-shifted voice to the right. The result is the wide, lush, slightly chorus-y doubling effect heard on countless guitar and vocal recordings — cheap to compute, low-latency, and convincing.

Parameters: `Pitch` (0–30 cents, default 7), `Mix` (0–1, default 1.0), `Mono Listen` (sums output to mono after processing — useful for checking mono-compatibility).

Built on the [Starty](https://github.com/vberthiaume/starty) template, which itself derives from [Pamplejuce](https://github.com/sudara/pamplejuce).

## Install dependencies
### macOS
```bash
brew install cmake ninja clang-format          # Homebrew: https://brew.sh
```

### Linux (Ubuntu / Debian)
```bash
sudo apt update
sudo apt install -y \
  cmake ninja-build clang clang-format lld \
  libasound2-dev libx11-dev libxinerama-dev libxext-dev \
  libfreetype6-dev libwebkit2gtk-4.1-dev libglu1-mesa-dev
```

### Windows
- **[CMake](https://cmake.org/download/)** (add to PATH during install).
- **[Ninja](https://github.com/ninja-build/ninja/releases)** on PATH (or `choco install ninja`).

## Install the pre-commit hook
One-time, per clone. Refuses commits whose staged C/C++ files aren't clang-format clean (see `.githooks/pre-commit`):

```bash
git config core.hooksPath .githooks
```

## Build (and run tests)
```bash
cmake -B Builds -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build Builds
ctest --test-dir Builds --output-on-failure
```

For a universal macOS binary, add `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"` to the configure step.

## Run RTSan locally (macOS)
CI runs RealtimeSanitizer on Linux. To check locally on macOS, install Homebrew LLVM — Apple Clang doesn't ship the RTSan runtime:
```bash
brew install llvm
```

Configure a separate build dir using brew's clang and the realtime flags:
```bash
CC=/opt/homebrew/opt/llvm/bin/clang \
CXX=/opt/homebrew/opt/llvm/bin/clang++ \
cmake -B Builds-rtsan -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS="-fsanitize=realtime" \
  -DCMAKE_CXX_FLAGS="-fsanitize=realtime" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=realtime"
```

Build and run:
```bash
cmake --build Builds-rtsan --target Tests
ctest --test-dir Builds-rtsan --output-on-failure --verbose -E NOT_BUILT
```

## CI
Every push and PR triggers:
- `build_and_test` — Linux/macOS/Windows, `pluginval` validation, artifact upload
- `instrumented_tests` — ASan / UBSan / TSan / RTSan (clang-20 for the latter), plus a Coverage report (gcovr → HTML artifact + step-summary numbers)
- `clang-tidy` — posts review comments on PRs

`nightly.yml` is wired up but its `schedule:` block is currently commented out — push-driven CI is enough while development is active. Re-enable the cron in that file once the project is in maintenance mode and external drift (JUCE on `develop`, apt packages, runner images) becomes the main breakage risk.

## License
Dupe is released under the [GNU Affero General Public License, version 3](LICENSE) (AGPLv3). Copyright (C) 2026 Vincent Berthiaume.

This project links against [JUCE](https://juce.com/), used under the AGPLv3 free-use option of JUCE Ltd's dual-license terms.

### Third-party attribution
Portions of the build system and project scaffolding derive from the [Pamplejuce](https://github.com/sudara/pamplejuce) template, which is distributed under the MIT License:

> MIT License
>
> Copyright (c) 2022 Sudara Williams
>
> Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
