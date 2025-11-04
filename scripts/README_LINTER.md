# eScript Linter

Simple code quality checker for POL eScript that helps enforce OOP paradigms and detect code smells during your team's transition from procedural to object-oriented programming.

## Features

### 1. OOP Paradigm Violations

Detects when developers use old procedural patterns instead of new OOP wrappers:

- **Direct character property access** - Warns when accessing `.str`, `.dex`, `.int`, `.hp`, etc. directly
  - **Solution**: Use `MobileWrapper` class with methods like `GetStr()`, `GetDex()`, `GetCurrentHP()`

- **Direct item property modification** - Warns when modifying item properties like `.ar`, `.damage`, `.quality` directly
  - **Solution**: Use `EquipWrapperArmor` or `EquipWrapperWeapon` classes

- **Direct AP_* function usage** - Warns when using `AP_SetVital`, `AP_GetVital`, `AP_SetTrueSkill`, etc. outside wrapper classes
  - **Solution**: Use `MobileWrapper` methods instead

- **Procedural function naming** - Detects old-style `Verb_Noun` function names
  - **Solution**: Convert to class methods or use camelCase naming

- **Global state mutation** - Warns about global variable usage
  - **Solution**: Encapsulate state in classes

### 2. Code Smells

Detects common code quality issues:

- **Long parameter lists** (>4 parameters) - Suggests using structs or classes
- **Magic numbers** - Hardcoded numeric values that should be constants
- **Deep nesting** (>3 levels) - Suggests extracting to separate functions
- **God classes** (>20 functions) - Suggests splitting into smaller classes
- **Primitive obsession** - Multiple related primitives that should be grouped
- **Feature envy** - Repeated access to same object's properties
- **Commented-out code** - Dead code that should be removed
- **Code duplication** - Indicated by comments mentioning "duplicate" or "copy"

### 3. Security & Best Practices

- **SendSysMessage usage** - Suggests using `Message.SendSys()` instead
- **Command injection risks** - Detects user input in script execution
- **Hardcoded serial numbers** - Warns about magic serial values
- **Missing error handling** - Checks for error handling on create operations
- **sleep() in combat code** - Warns about performance issues
- **TODO/FIXME markers** - Highlights code marked for attention

## Usage

### Check entire package directory

```bash
./scripts/lint-escript.sh pkg/
```

### Check specific system

```bash
./scripts/lint-escript.sh pkg/systems/combat/
```

### Check single file

```bash
./scripts/lint-escript.sh pkg/systems/combat/mainCombat.src
```

### Check only git changes (staged)

```bash
./scripts/lint-git-diff.sh
```

### Check changes since a commit

```bash
./scripts/lint-git-diff.sh main
./scripts/lint-git-diff.sh HEAD~3
./scripts/lint-git-diff.sh origin/main
```

## Output Example

```
eScript Linter - Scanning pkg/systems/combat
================================================
OOP pkg/systems/combat/damage.src:45 - Direct character property access. Consider using MobileWrapper class
      var str := attacker.str;
  Tip: Use MobileWrapper(character) and call GetStr(), GetDex(), GetInt(), GetCurrentHP(), etc.

WARNING pkg/systems/combat/damage.src:67 - Use Message.SendSys() instead of SendSysMessage()
      SendSysMessage(who, "You deal damage!");

SMELL pkg/systems/combat/damage.src:89 - Magic number detected. Consider using named constants
      var bonus := 50;

================================================
Scan complete:
  0 errors
  1 warnings
  1 OOP violations
  1 code smells

OOP Migration Guide:
  1. Use MobileWrapper for character operations
  2. Use EquipWrapper* classes for equipment
  3. Replace procedural functions with class methods
  4. Encapsulate related data in classes
```

## Integration with Development Workflow

### Git Hooks (Native Git Feature - No Installation Needed!)

Git has built-in support for hooks. They're just bash scripts in the `.git/hooks/` directory.

#### Pre-commit Hook (Checks before commit)

Checks staged changes before you commit:

```bash
# Install the pre-commit hook
cp scripts/pre-commit.example .git/hooks/pre-commit
chmod +x .git/hooks/pre-commit

# Now every git commit will run the linter automatically
git commit -m "your message"

# To bypass (when needed):
git commit --no-verify -m "your message"
```

#### Pre-push Hook (Checks before push)

Checks all commits before pushing to remote:

```bash
# Install the pre-push hook
cp scripts/pre-push.example .git/hooks/pre-push
chmod +x .git/hooks/pre-push

# Now every git push will run the linter automatically
git push origin your-branch

# To bypass (when needed):
git push --no-verify origin your-branch
```

**Recommendation**: Use pre-push hooks for main/master branches (strict enforcement) and pre-commit hooks for feature branches (early feedback)

### CI/CD Pipeline

Add to your CI configuration:

```bash
./scripts/lint-escript.sh pkg/
```

### IDE Integration

You can integrate the linter output with your IDE's problem matcher to get inline warnings.

## Customization

Edit [lint-escript.sh](lint-escript.sh) to:

- Add more OOP rules specific to your wrapper classes
- Adjust code smell thresholds
- Add project-specific naming conventions
- Add more security checks

### Example: Adding Custom Rule

```bash
# Rule: Warn about direct SetName usage
if echo "$line" | grep -E "SetName\(" | grep -qv "//"; then
    if ! echo "$file" | grep -q "Wrapper\.inc\|Wrapper\.src"; then
        echo -e "${MAGENTA}OOP${NC} $file:$line_num - Direct SetName usage. Consider using MobileWrapper"
        echo "  $line"
        ((OOP_WARNINGS++))
    fi
fi
```

## Performance

The linter is designed to be fast:

- Scans ~1000 files in under 10 seconds
- Uses simple grep/regex patterns
- No external dependencies beyond bash

## Limitations

- **Regex-based**: Cannot detect complex logic patterns
- **Line-by-line**: Limited context for multi-line patterns
- **False positives**: May flag legitimate code in some cases
- **No auto-fix**: Only detects issues, doesn't fix them automatically

For more sophisticated analysis, consider integrating with proper static analysis tools in the future.

## Future Enhancements

Potential improvements:

1. **Auto-fix mode** - Automatically convert old patterns to new OOP style
2. **Configuration file** - Allow per-project rule customization
3. **JSON output** - For IDE/tool integration
4. **Metrics dashboard** - Track OOP adoption over time
5. **Complexity metrics** - Cyclomatic complexity, halstead metrics
6. **Duplication detector** - Find actual duplicated code blocks

## Support

If you encounter issues or want to add new rules:

1. Check existing patterns in the script
2. Test your regex patterns carefully
3. Consider false positive rate
4. Document the rule clearly

## License

Part of Epic! Shard - Internal tool
