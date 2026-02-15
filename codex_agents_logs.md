# codex_agents_logs.md

Append-only execution log.

## Entry Format

```text
START  | <timestamp+tz> | <role> | task: <one-line summary>
SCOPE  | <paths>
BRANCH | feature/<scope>-<desc>
GIT-1  | <type>(<scope>): <what and why>
GIT-2  | files: <changed files>
GIT-3  | verify: <commands and results>
GIT-4  | branch: feature/<scope>-<desc>
GIT-5  | pr: <number or URL>
END    | <timestamp+tz> | <role> | commit: <message>
```

## Log

```text
START  | 2026-02-15T00:00:00Z | system | task: initialize log
SCOPE  | docs
BRANCH | n/a
END    | 2026-02-15T00:00:00Z | system | commit: n/a
```
