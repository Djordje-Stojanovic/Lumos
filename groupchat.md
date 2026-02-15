# groupchat.md

Append-only backup coordination channel.
Primary coordination is PR comments; use this as async audit trail.

## Message Format

```text
[timestamp+tz] | from:<role> | type:<info|request|ack|handoff|release> | msg:<text>
```

## Messages

```text
[2026-02-15T00:00:00Z] | from:system | type:info | msg:groupchat initialized
```
