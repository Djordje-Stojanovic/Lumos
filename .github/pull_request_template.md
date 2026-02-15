## Why

- Explain why this change matters now and how it improves user value, reliability, or delivery speed.

## What

- Summarize the scoped changes made in this PR.

## Validation

- List exact checks run and outcomes.
- Include command output summaries for build/test when applicable.

### Local Verification Commands

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLUMOS_BUILD_UI=OFF -DLUMOS_BUILD_TESTS=ON
cmake --build build --config Release -j
ctest --test-dir build -C Release --output-on-failure
```
