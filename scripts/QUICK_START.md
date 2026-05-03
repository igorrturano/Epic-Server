# eScript Linter - Quick Start Guide

## 🚀 Installation (One Command)

```bash
./scripts/install-hooks.sh both
```

That's it! You now have automatic code quality checks on every commit and push.

---

## 📋 What You Get

### 1. **lint-escript.sh** - Full Codebase Linter
Scans files or directories for OOP violations and code smells.

```bash
# Check entire package directory
./scripts/lint-escript.sh pkg/

# Check specific system
./scripts/lint-escript.sh pkg/systems/combat/

# Check single file
./scripts/lint-escript.sh pkg/systems/combat/mainCombat.src
```

### 2. **lint-git-diff.sh** - Git Change Linter
Only checks the lines you changed (much faster!).

```bash
# Check staged changes
./scripts/lint-git-diff.sh

# Check all changes since main branch
./scripts/lint-git-diff.sh main

# Check changes since last commit
./scripts/lint-git-diff.sh HEAD~1
```

### 3. **oop-metrics.sh** - Progress Tracker
See how well your team is adopting OOP patterns.

```bash
./scripts/oop-metrics.sh pkg/
```

Output example:
```
================================================
OOP Adoption Score: 67%

   [█████████████████████████████████░░░░░░░░░░░░░░░░░]

🎯 Recommendation: Good progress! Continue migrating AP_* functions
================================================
```

### 4. **Git Hooks** (Automatic Checks)
- **pre-commit**: Runs before `git commit` (checks staged changes)
- **pre-push**: Runs before `git push` (checks all commits to main)

---

## 🎯 What It Detects

### OOP Violations (Your Main Concern!)

#### ❌ Bad (Old Procedural Way):
```escript
var strength := character.str;
var dexterity := character.dex;
AP_SetVital(character, "Hits", 100);
SendSysMessage(character, "Done!");
```

#### ✅ Good (New OOP Way):
```escript
var wrapper := MobileWrapper(character);
var strength := wrapper.GetStr();
var dexterity := wrapper.GetDex();
wrapper.SetCurrentHP(100);
Message.SendSys(character, "Done!");
```

### Code Smells It Catches

- **Long parameter lists** (>4 params) → Use structs/classes
- **Magic numbers** (hardcoded 50, 100, etc.) → Use constants
- **Deep nesting** (>3 levels) → Extract to functions
- **God classes** (>20 functions) → Split into smaller classes
- **Commented-out code** → Delete it (git remembers!)
- **Feature envy** → Repeated access to same object

---

## 🛠️ Daily Usage

### When Committing Code

```bash
git add .
git commit -m "add new combat feature"
# ← Linter runs automatically here!
# If issues found, fix them and commit again
```

### When Pushing to Main

```bash
git push origin main
# ← Linter runs automatically and checks ALL your commits
# Stricter checks for main branch!
```

### Manual Check Before PR

```bash
# Check everything you changed since branching from main
./scripts/lint-git-diff.sh origin/main
```

### Bypass When Needed (Emergency Only!)

```bash
git commit --no-verify -m "hotfix: critical bug"
git push --no-verify origin main
```

---

## 📊 Team Adoption Tracking

Run this weekly to track progress:

```bash
./scripts/oop-metrics.sh pkg/ > oop-report.txt
```

Share the report with your team to show progress!

---

## 🔧 Customization

Want to add your own rules? Edit [lint-escript.sh](lint-escript.sh):

```bash
# Add after line 150 (in the OOP section):

# Custom Rule: Warn about old combat system
if echo "$line" | grep -E "OldCombatFunction" | grep -qv "//"; then
    echo -e "${MAGENTA}OOP${NC} $file:$line_num - Use new CombatWrapper class"
    echo "  $line"
    ((OOP_WARNINGS++))
fi
```

---

## 🆘 Troubleshooting

### "Hook not running"
```bash
# Check if hook is executable
ls -la .git/hooks/pre-commit
# Should show -rwxr-xr-x

# If not, make it executable:
chmod +x .git/hooks/pre-commit
```

### "Too many false positives"
The linter is intentionally strict to help with OOP migration. If a specific file legitimately needs old patterns, you can:

1. Add a comment to explain why: `// Legacy code - migration planned`
2. Adjust the pattern in lint-escript.sh to exclude that specific case
3. Use `--no-verify` for that one commit (not recommended)

### "Linter is slow"
Use the git-diff version instead:
```bash
./scripts/lint-git-diff.sh  # Only checks changed lines
```

---

## 📚 Learn More

- Full documentation: [README_LINTER.md](README_LINTER.md)
- Pre-commit example: [pre-commit.example](pre-commit.example)
- Pre-push example: [pre-push.example](pre-push.example)

---

## 🎓 Teaching Your Team

Share this with new developers:

1. **Why we lint**: Consistent code quality, catch bugs early, enforce OOP patterns
2. **How to install**: `./scripts/install-hooks.sh both`
3. **What to do when it fails**: Read the message, fix the issue, commit again
4. **When to bypass**: Never on main, rarely on features, only for emergencies

---

## ✅ Checklist for New Team Members

- [ ] Run `./scripts/install-hooks.sh both`
- [ ] Test it: Make a commit with `character.str` and see the warning
- [ ] Check your OOP score: `./scripts/oop-metrics.sh pkg/`
- [ ] Read about MobileWrapper: `pkg/systems/charsys/include/ViewModel/`
- [ ] Read about EquipWrapper: `pkg/systems/equipsys/include/ViewModel/`

---

**Questions?** Check the main [README_LINTER.md](README_LINTER.md) or ask the team lead!
