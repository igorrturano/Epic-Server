# 🌿 Feat: Enhanced Druid Metamorphosis with Primordial Forms

## 📋 Overview

This PR significantly enhances the Druid metamorphosis system by introducing **Primordial Elemental Forms** - a new high-tier transformation system that allows druids to take on powerful elemental manifestations tied to specific terrain types. This builds upon the existing wildshape system while adding terrain-based restrictions, elemental combat mechanics, and improved visual feedback.

## ✨ Key Features

### 1. **Primordial Form System** 🔥❄️⚡🌿

A new `.formaprimordial` command that enables druids with the "Chamado da forma primordial" ability to transform into 15 different elemental forms:

#### Fire Element Forms

- **Salamandra** - Fire Salamander (Mountains)
- **Fogo** - Living Flame (Forests, Deserts, Mountains)
- **Cinza** - Ash Elemental (Mountains, Deserts)

#### Water/Ice Element Forms

- **Gelo** - Ice Elemental (Tundra, Mountains)
- **Água** - Water Elemental (Oceans, Swamps)

#### Earth Element Forms

- **Terra** - Earth Elemental (Mountains, Deserts, Plains)
- **Obsidiana** - Obsidian Elemental (Mountains)
- **Lava** - Lava Elemental (Mountains, Deserts)
- **Cristal** - Crystal Elemental (Mountains, Caves)

#### Nature/Poison Element Forms

- **Cogumelo** - Mushroom Elemental (Forests, Swamps)
- **Fungos** - Fungal Colony (Forests, Swamps)
- **Veneno** - Poison Elemental (Swamps, Jungles)

#### Energy Element Forms

- **Raio** - Lightning Elemental (Mountains, Plains)
- **Luz** - Light Elemental (Plains, Tundra)
- **Totem** - Totem Spirit (All terrains)

### 2. **Terrain-Based Restrictions** 🗺️

Each form can only be invoked in specific terrain types, creating strategic choices:

- **Mountains**: Salamandra, Fogo, Cinza, Gelo, Terra, Obsidiana, Lava, Cristal, Raio
- **Forests**: Fogo, Cogumelo, Fungos
- **Deserts**: Fogo, Cinza, Terra, Lava
- **Swamps**: Água, Cogumelo, Fungos, Veneno
- **And more...**

### 3. **Elemental Combat Integration** ⚔️

#### For Druids with "Instinto de Sobrevivência" Ability:

- Base physical damage (Blunt) is converted to elemental damage matching the form
- Theology skill replaces Tactics for damage calculation
- Fire forms add fire damage, Ice adds cold damage, etc.

#### Elemental Resistance Bonuses:

Each form grants +25% resistance to its corresponding element:

- Fire forms: +25% Fire Resistance
- Ice forms: +25% Cold Resistance
- Poison forms: +25% Poison Resistance
- Energy forms: +25% Energy Resistance

### 4. **Enhanced Wildshape System** 🐺

Improvements to the standard `.assumirforma` command:

- Taxonomy-based weapon properties (claws, bites, etc.)
- Enhanced visual feedback and animations
- Better stat scaling and duration calculation
- Proper cleanup of transformation properties

### 5. **Wild Speech Integration** 🗣️

The new `.wildspeech` spell (Spell ID 15, "Fala Selvagem") allows druids to prepare speech while in beast form:

- Maintains communication capability during transformations
- Proper cleanup when reverting forms
- Integrated with the metamorphose timer system

## 🔧 Technical Changes

### New Files

- `pkg/systems/spells/commands/player/formaprimordial.src` - Primordial form transformation system
- `pkg/systems/spells/spell_logic/druid/wildspeech.src` - Wild speech spell logic
- `pkg/systems/spells/maintenance/wildspeechmaintenance.src` - Wild speech maintenance
- `internal-tools/extract-landtiles-tool.js` - Landtile extraction tool
- `internal-tools/npcdesc_parser.py` - NPC description parser

### Modified Files

#### Combat System

- `pkg/systems/combat/include/Controller/DamageController.inc`
  - Added `CalculatePrimordialDamage()` for elemental damage conversion
  - Modified `CalculateSkillDamage()` to use Theology for metamorphosed druids with survival ability
  - Improved damage calculation with `GetDamageModifierBy()`

#### Transformation Management

- `pkg/mobiles/timedScripts/subScripts/magery/endmetamorfose.src`
  - Added cleanup for primordial elemental properties
  - Added Wild Speech property cleanup
  - Added HitScripts property removal
  - Enhanced resistance cleanup logic

#### Spell Configuration

- `pkg/systems/spells/config/druidspells.cfg`
  - Added Wild Speech (Spell 15) as a cantrip
  - Moved from character creation to spell system

#### Character Creation

- `pkg/systems/charactercreation/config/habilidades.cfg`
  - Updated "Chamado da forma primordial" reference from character creation to spells package

#### Wrappers

- `pkg/systems/wrappers/include/Objects/Model/MobileWrapper.inc`

  - Enhanced `GetProperty()` method
  - Added support for taxonomy-based weapon properties

- `pkg/systems/wrappers/include/Objects/Model/WeaponWrapper.inc`
  - Improved dice validation and parsing

### Removed Files

- `pkg/systems/charactercreation/habilidades/sabio/formaprimordial.src` (moved to spells package)
- `.editorconfig` (reverted)
- `.prettierignore` (reverted)

## 🎮 Usage Examples

### Transforming into a Primordial Form

```
.formaprimordial fogo          # Transform into Living Flame (in forests/deserts/mountains)
.formaprimordial gelo          # Transform into Ice Elemental (in tundra/mountains)
.formaprimordial totem         # Transform into Totem Spirit (anywhere)
```

### Using Wild Speech

```
.cast 15                       # Cast Wild Speech to prepare communication
                              # Then transform and speak normally
```

### Reverting Transformation

```
.voltar                        # Revert to original form (works for all transformations)
```

## 🐛 Bug Fixes

- Fixed array indexing bug in combat system
- Fixed dice validation in weapon damage calculation
- Improved property cleanup on transformation end
- Fixed skill modifier calculation for offhand attacks

## 🧪 Testing Notes

- ✅ All primordial forms properly apply visual changes
- ✅ Terrain restrictions work correctly
- ✅ Elemental damage conversion functions as expected
- ✅ Resistance bonuses apply and clean up properly
- ✅ Wild Speech enables communication in beast form
- ✅ `.voltar` command properly reverts all forms
- ✅ Properties clean up correctly on timer expiration
- ✅ Druid combat abilities integrate with primordial forms

## 🔄 Breaking Changes

None. This is a purely additive feature that enhances existing systems without breaking compatibility.

## 📝 Related Commits

- `fbafed4f` - Initial primordial forms implementation
- `29643033` - Code review comments addressed
- `1f22eac2` - Wildshape properties update
- `95a8c1a5` - Additional wildshape properties update
- `4da007f5` - Form refinements: removed tentáculos/veneno, added totem
- `56f83750` - Removed ent_medio and ent_alto forms
- `6ec67383` - Cleanup of development files
- `fdec2d9b` - Reverted refactor pattern for stability
- `5174b3b8` - Merge with develop branch

## 📚 Documentation

The system is fully documented in the source files with:

- Detailed function descriptions
- Inline comments explaining logic
- TODO notes for future enhancements

## 🎯 Future Improvements (TODs in code)

- Add entropy system for same-biome transformation variations
- Add criteria based on z-index, items, and minerals for certain transformations
- Expand terrain type detection
- Add more visual effects for transformations

---

**Ready for review!** 🚀

This PR significantly expands druid gameplay options while maintaining system stability and backward compatibility.
