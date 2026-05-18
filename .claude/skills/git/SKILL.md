---
name: git
description: Git workflow rules for AIDAW. MUST be followed for all git operations.
---

# Git Workflow

## Branch Naming

- `feat/<description>` — new features
- `fix/<description>` — bug fixes
- `refactor/<description>` — code restructuring

## Rules

- Never push directly to main
- Never use `--force` or `--no-verify` without explicit user request
- Only commit when user explicitly asks
- Use descriptive commit messages

## Submodules

Third-party deps are git submodules:

```bash
git submodule update --init --recursive  # After clone
git submodule update --remote            # Update to latest
```
