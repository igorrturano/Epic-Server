/* $Id$
 *
 * Purpose
 * Handles damages and resistances to damage types
 *
 */

// Internal
include ":combat:armorZones";
// include ":abilities:models/Ability";
include ":combat:Model/Damage";
include "CombatAbilityController";
include "SkillController";
include "ParryController";
include ":combat:Controller/WearDownController";

// External
include ":io:CFGFile";
include ":wrappers:include/Objects/Model/MobileWrapper";
include ":charactercreation:resists";
include ":charactercreation:habilidades";
include ":attributes:attributes";
include "include/eventID";
include "include/client";
include ":fls_core:fls_characters";

function ProccessCombatDamage(byRef attackInfo)
	var corpseLoc := SnapshotDefenderLocation(attackInfo);

	// Pre Damage
	CalculateCombatDamage(attackInfo);
	CheckParry(attackInfo);
	ConcealDoomedDefender(attackInfo);

	// Damage
	ApplyCombatDamage(attackInfo);
	AttackLogger(attackInfo);

	// Post Damage
	CheckPoison(attackInfo);
	ExecuteCombatAbilitiesAfterDamage(attackInfo);
	ExecutePassiveAbilitiesAfterDamage(attackInfo);

	// On Kill
	if (attackInfo.GetDefender().IsDead())
		ExecuteCombatAbilitiesOnKill(attackInfo);
		ExecutePassiveAbilitiesOnKill(attackInfo);
		if (attackInfo.GetDamage().IsCritical())
			TriggerCriticalKillExplosion(attackInfo, corpseLoc);
		endif
	elseif (attackInfo.GetDamage().IsCritical())
		TriggerCriticalHitFeedback(attackInfo);
	endif

endfunction

// Capture the defender's coords up front — once ApplyCombatDamage runs the
// death pipeline invalidates the mobile ref and .x/.y/.z return uninitialized.
function SnapshotDefenderLocation(byRef attackInfo)
	var ref := attackInfo.GetDefender().GetObjRef();
	return struct{
		x := ref.x,
		y := ref.y,
		z := ref.z,
		realm := ref.realm
	};
endfunction

// Pre-damage hook. If a player's crit is going to kill the defender, hide
// the mobile from the client now so the falling-death animation never
// renders — only the body-explosion fired from the on-kill branch does.
// Damage is final here (post-resist, post-parry).
function ConcealDoomedDefender(byRef attackInfo)
	if (!attackInfo.GetDamage().IsCritical()) return; endif
	if (attackInfo.GetAttacker().IsNpc()) return; endif
	var ref := attackInfo.GetDefender().GetObjRef();
	if (attackInfo.GetDamage().GetTotalBaseDamage() < ref.hits) return; endif
	ref.concealed := 1;
endfunction

// Non-killing crit: light attacker shake.
function TriggerCriticalHitFeedback(byRef attackInfo)
	if (attackInfo.GetAttacker().IsNpc()) return; endif
	ScreenShake(attackInfo.GetAttacker().GetObjRef(), 0.4, 5);
endfunction

// Killing crit: heavy shake + body-explosion FX + sound, then locks the
// corpse to the gore graphic once the 10-frame animation finishes.
function TriggerCriticalKillExplosion(byRef attackInfo, byRef corpseLoc)
	if (!attackInfo.GetAttacker().IsNpc())
		ScreenShake(attackInfo.GetAttacker().GetObjRef(), 0.8, 10);
	endif

	// Location-based variants — defender ref is already invalidated, so
	// object-centered effects would silently no-op on it.
	PlayStationaryEffect(corpseLoc.x, corpseLoc.y, corpseLoc.z, FX_BODY_EXPLOSION, 50, 10, 0, corpseLoc.realm);
	PlaySoundEffectXYZ(corpseLoc.x, corpseLoc.y, corpseLoc.z, 0x54F, corpseLoc.realm);

	sleepms(500); // let POL finalize the corpse

	var corpse := 0;
	foreach candidate in (ListItemsNearLocation(corpseLoc.x, corpseLoc.y, corpseLoc.z, 2, corpseLoc.realm))
		Sleepms(2);
		if (candidate.IsA(POLCLASS_CORPSE))
			corpse := candidate;
			break;
		endif
	endforeach

	if (!corpse) return; endif

	// Wait for the 10-frame animation to finish, then lock in its last frame
	// as the persistent gore graphic so the swap is visually seamless.
	sleepms(4500);
	corpse.graphic := 0xC9DE;
	if (corpse.color != 0)
		corpse.color := 0;
	endif
endfunction

function CalculateCombatDamage(byRef attackInfo)
	CalculateAttackerDamage(attackInfo);
	ExecuteCombatAbilitiesBeforeDamage(attackInfo);
	ExecutePassiveAbilitiesAttackerBeforeDamage(attackInfo);
	ExecutePassiveAbilitiesDefenderBeforeDamage(attackInfo);
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("Dano base (após passivas):"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::SLASH)}: {attackInfo.GetDamage().GetSlashBaseDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: {attackInfo.GetDamage().GetPiercingBaseDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: {attackInfo.GetDamage().GetBluntBaseDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::FIRE)}: {attackInfo.GetDamage().GetFireBaseDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::COLD)}: {attackInfo.GetDamage().GetColdBaseDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::POISON)}: {attackInfo.GetDamage().GetPoisonBaseDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::ENERGY)}: {attackInfo.GetDamage().GetEnergyBaseDamage()}"));
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, Message("Dano base:"));
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::SLASH)}: {attackInfo.GetDamage().GetSlashBaseDamage()}"));
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: {attackInfo.GetDamage().GetPiercingBaseDamage()}"));
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: {attackInfo.GetDamage().GetBluntBaseDamage()}"));
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::FIRE)}: {attackInfo.GetDamage().GetFireBaseDamage()}"));
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::COLD)}: {attackInfo.GetDamage().GetColdBaseDamage()}"));
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::POISON)}: {attackInfo.GetDamage().GetPoisonBaseDamage()}"));
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::ENERGY)}: {attackInfo.GetDamage().GetEnergyBaseDamage()}"));
	//DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Chance crítico (arma: {CInt(attackInfo.GetWeapon().GetCriticalChance())}, char: {CInt(attackInfo.GetAttacker().GetCriticalChance())}, skill: {CInt(attackInfo.GetAttacker().GetSkill(skillUsedForCritical) / 20)}, reducao: {CInt(CalculateDefenderCriticalReduction(attackInfo.GetDefender(), attackInfo.GetBodyRegion()))})"));	
	sleepms(2);
	CalculateFlankingDamage(attackInfo);
	CalculateAttackerCriticalHit(attackInfo);
	var _totalBeforeDefense := attackInfo.GetDamage().GetTotalBaseDamage();
	CalculateDefenderResist(attackInfo);
	var _totalAfterDefense := attackInfo.GetDamage().GetTotalBaseDamage();
endfunction

function CalculateAttackerDamage(byRef attackInfo)
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("CÁLCULO DE DANO!"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	attackInfo.SetDamage(Damage());
	var statusDamageInfo := CalculateStatusDamage(attackInfo);
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Bônus de dano por {Upper(statusDamageInfo.stat)}: {statusDamageInfo.damageMod}"));
	var skillDamageInfo := CalculateSkillDamage(attackInfo);
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Bônus de dano por {skillDamageInfo.skill}: {skillDamageInfo.damageMod}"));
	DebugMsg(attackInfo.GetDefender().GetObjRef(), DEBUG_DAMAGE, Message($"Bônus de dano por {skillDamageInfo.skill}: {skillDamageInfo.damageMod}"));
	CalculateWeaponDamage(attackInfo);
	CalculateEntropyDamage(attackInfo);
	CalculatePrimordialDamage(attackInfo);
	CalculateArcaneBowDamage(attackInfo);
	CalculateSpiritualBladeDamage(attackInfo);
	CalculateElementalWeaponDamage(attackInfo);
endfunction

function CalculateEstimatedMinMaxDamage(attackInfo)

	attackInfo.SetAttackType(attackInfo.GetAttacker().GetCombatTraits().GetPosture());
	var statusDamage := CalculateStatusDamage(attackInfo);
	var skillDamage := CalculateSkillDamage(attackInfo);
	var primordialDamage := CalculatePrimordialDamage(attackInfo);
	var arcaneBowBonus := EstimateArcaneBowDamage(attackInfo);
	var spiritualBladeBonus := EstimateSpiritualBladeDamage(attackInfo);
	var elementalWeaponBonus := EstimateElementalWeaponDamage(attackInfo);
	var finalDamage := CalculateEstimatedMinMaxWeaponDamage(attackInfo);

	var flatBonus := statusDamage.damageMod + skillDamage.damageMod + primordialDamage
					+ arcaneBowBonus + spiritualBladeBonus + elementalWeaponBonus;
	
	finalDamage.max += flatBonus;
	finalDamage.min += flatBonus;
	finalDamage.+Damage := attackInfo.GetDamage();

	return finalDamage;
endfunction

function CalculateStatusDamage(byRef attackInfo)

	var statusDamage := struct{
		stat := "",
		damageMod := 0
	};

	if (attackInfo.GetWeapon().GetAttribute() == RANGED)
		if (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::THROWING)
			statusDamage.stat := $"{STRENGTH} + {DEXTERITY}";

			statusDamage.damageMod := attackInfo.GetDamageHand() == DAMAGE_HAND::MAINHAND ?
						 attackInfo.GetAttacker().GetMainDamageModifierBy(STRENGTH, DEXTERITY) :
						 attackInfo.GetAttacker().GetOffDamageModifierBy(STRENGTH, DEXTERITY);
		else
			statusDamage.stat := DEXTERITY;

			statusDamage.damageMod := attackInfo.GetDamageHand() == DAMAGE_HAND::MAINHAND ?
						 attackInfo.GetAttacker().GetMainDamageModifierBy(DEXTERITY) :
						 attackInfo.GetAttacker().GetOffDamageModifierBy(DEXTERITY);
		endif
	elseif (attackInfo.GetWeapon().IsFinesse())
		statusDamage.stat := $"{STRENGTH}/{DEXTERITY}";
		var _finesseStat := Max(attackInfo.GetAttacker().GetStr(), attackInfo.GetAttacker().GetDex());
		var _finesseMain := CInt(_finesseStat / 20);
		statusDamage.damageMod := attackInfo.GetDamageHand() == DAMAGE_HAND::MAINHAND ?
					 _finesseMain :
					 CInt(_finesseMain / 2);
	else
		statusDamage.stat := STRENGTH;

		statusDamage.damageMod := attackInfo.GetDamageHand() == DAMAGE_HAND::MAINHAND ?
					 attackInfo.GetAttacker().GetMainDamageModifierBy(STRENGTH) :
					 attackInfo.GetAttacker().GetOffDamageModifierBy(STRENGTH);
	endif

	attackInfo.GetDamage().IncreaseSlashRawDamage(statusDamage.damageMod)
						  .IncreasePiercingRawDamage(statusDamage.damageMod)
						  .IncreaseBluntRawDamage(statusDamage.damageMod);

	return statusDamage;
endfunction


function CalculateSkillDamage(byRef attackInfo)
	var skill := TACTICS;
	// Check if druid with metamorfose + survival ability
	if (attackInfo.GetAttacker().GetProperty("metamorfose") != error)
		skill := THEOLOGY;
	endif

	var skillDamage := struct{
		skill := skill,
		damageMod := 0
	};

	skillDamage.damageMod := attackInfo.GetDamageHand() == DAMAGE_HAND::MAINHAND ?
					 attackInfo.GetAttacker().GetMainDamageModifierBy(skill) :
					 attackInfo.GetAttacker().GetOffDamageModifierBy(skill);
	
	attackInfo.GetDamage().IncreaseSlashRawDamage(skillDamage.damageMod)
						  .IncreasePiercingRawDamage(skillDamage.damageMod)
						  .IncreaseBluntRawDamage(skillDamage.damageMod);

	return skillDamage;
endfunction

function CalculateWeaponDamage(byRef attackInfo)
	var damageDice;

	if (attackInfo.GetDamageHand() == DAMAGE_HAND::MAINHAND)
		damageDice := attackInfo.GetWeapon().GetMainDamage();
	elseif (attackInfo.GetDamageHand() == DAMAGE_HAND::OFFHAND)
		damageDice := attackInfo.GetWeapon().GetOffDamage();
	endif

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("Dados de dano da arma:"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::SLASH)}: {damageDice.GetSlashDamage().ToString() ?: "N/A"}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: {damageDice.GetPiercingDamage().ToString() ?: "N/A"}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: {damageDice.GetBluntDamage().ToString() ?: "N/A"}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::FIRE)}: {damageDice.GetFireDamage().ToString() ?: "N/A"}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::COLD)}: {damageDice.GetColdDamage().ToString() ?: "N/A"}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::POISON)}: {damageDice.GetPoisonDamage().ToString() ?: "N/A"}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::ENERGY)}: {damageDice.GetEnergyDamage().ToString() ?: "N/A"}"));

	var slashDamage := damageDice.GetSlashDamage().Roll().GetResult() ?: 0;
	var piercingDamage := damageDice.GetPiercingDamage().Roll().GetResult() ?: 0;
	var bluntDamage := damageDice.GetBluntDamage().Roll().GetResult() ?: 0;
	var fireDamage := damageDice.GetFireDamage().Roll().GetResult() ?: 0;
	var coldDamage := damageDice.GetColdDamage().Roll().GetResult() ?: 0;
	var poisonDamage := damageDice.GetPoisonDamage().Roll().GetResult() ?: 0;
	var energyDamage := damageDice.GetEnergyDamage().Roll().GetResult() ?: 0;

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("Dano rolado:"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::SLASH)}: {slashDamage}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: {piercingDamage}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: {bluntDamage}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::FIRE)}: {fireDamage}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::COLD)}: {coldDamage}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::POISON)}: {poisonDamage}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::ENERGY)}: {energyDamage}"));

	attackInfo.GetDamage().IncreaseSlashRawDamage(slashDamage)
						  .IncreasePiercingRawDamage(piercingDamage)
						  .IncreaseBluntRawDamage(bluntDamage)
						  .IncreaseFireRawDamage(fireDamage)
						  .IncreaseColdRawDamage(coldDamage)
						  .IncreasePoisonRawDamage(poisonDamage)
						  .IncreaseEnergyRawDamage(energyDamage);

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("Modificadores de dano:"));

	var damageMod := attackInfo.GetWeapon().GetDamageMod();

	attackInfo.GetDamage().IncreaseSlashRawDamage(damageMod.GetSlashRawDamage())
						  .IncreasePiercingRawDamage(damageMod.GetPiercingRawDamage())
						  .IncreaseBluntRawDamage(damageMod.GetBluntRawDamage())
						  .IncreaseFireRawDamage(damageMod.GetFireRawDamage())
						  .IncreaseColdRawDamage(damageMod.GetColdRawDamage())
						  .IncreasePoisonRawDamage(damageMod.GetPoisonRawDamage())
						  .IncreaseEnergyRawDamage(damageMod.GetEnergyRawDamage());
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::SLASH)}: {damageMod.GetSlashRawDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: {damageMod.GetPiercingRawDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: {damageMod.GetBluntRawDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::FIRE)}: {damageMod.GetFireRawDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::COLD)}: {damageMod.GetColdRawDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::POISON)}: {damageMod.GetPoisonRawDamage()}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::ENERGY)}: {damageMod.GetEnergyRawDamage()}"));

	// Apply ammunition damage if ammo exists
	if (attackInfo.GetAmmo())
		ApplyAmmoDamage(attackInfo);
	endif
endfunction

function CalculateEstimatedMinMaxWeaponDamage(byRef attackInfo)
	var MinMaxWeaponDamage := 
		struct{
			max := 0,
			min := 0
		};

	var weapon := attackInfo.GetWeapon();

	if(weapon)
		var damageDice := 0;
		if (attackInfo.GetDamageHand() == DAMAGE_HAND::MAINHAND)
			damageDice := weapon.GetMainDamage();
		else
			damageDice := weapon.GetOffDamage();
		endif
		
		var damageMod := attackInfo.GetWeapon().GetDamageMod();
		var physicalDamage := 0;

		if(attackInfo.GetAttackType() == DAMAGE_TYPE::SLASH)
			physicalDamage += damageMod.GetSlashRawDamage();
			attackInfo.GetDamage().IncreaseSlashBaseDamage(damageMod.GetSlashRawDamage());
		elseif(attackInfo.GetAttackType() == DAMAGE_TYPE::PIERCING)
			physicalDamage += damageMod.GetPiercingRawDamage();
			attackInfo.GetDamage().IncreasePiercingBaseDamage(damageMod.GetPiercingRawDamage());
		else
			physicalDamage += damageMod.GetBluntRawDamage();
			attackInfo.GetDamage().IncreaseBluntBaseDamage(damageMod.GetBluntRawDamage());
		endif

		var totalModDamage := physicalDamage 
								+ damageMod.GetFireRawDamage() 
								+ damageMod.GetColdRawDamage() 
								+ damageMod.GetPoisonRawDamage() 
								+ damageMod.GetEnergyRawDamage();

		MinMaxWeaponDamage.max := damageDice.GetMaximum(attackInfo.GetAttackType()) + totalModDamage;
		MinMaxWeaponDamage.min := damageDice.GetMinimum(attackInfo.GetAttackType()) + totalModDamage;

		attackInfo.GetDamage().IncreaseFireBaseDamage(damageMod.GetFireRawDamage())
							.IncreaseColdBaseDamage(damageMod.GetColdRawDamage())
							.IncreasePoisonBaseDamage(damageMod.GetPoisonRawDamage())
							.IncreaseEnergyBaseDamage(damageMod.GetEnergyRawDamage());
	endif


	return MinMaxWeaponDamage;
endfunction

function EstimateArcaneBowDamage(attackInfo)
	if (attackInfo.GetWeapon().GetCategory() != WEAPON_CATEGORY::MAGICBOW)
		return 0;
	endif

	var arcana := attackInfo.GetAttacker().GetSkill(ARCANA);

	if (attackInfo.GetAttacker().HasAbility(ABILITY_ID::ARCANE_MASTERY))
		return CInt(arcana / 5);
	endif

	return CInt(arcana / 10);
endfunction

function EstimateSpiritualBladeDamage(attackInfo)
	if (attackInfo.GetWeapon().GetCategory() != WEAPON_CATEGORY::SPIRITUALBLADE)
		return 0;
	endif

	var arcana := attackInfo.GetAttacker().GetSkill(ARCANA);
	var bonus := CInt(arcana / 5);
	if (bonus < 1)
		bonus := 1;
	endif

	return bonus;
endfunction

function EstimateElementalWeaponDamage(attackInfo)
	var weapon := attackInfo.GetWeapon().GetObjRef();
	var elemental := weapon.GetProperty("#elementalweapon");

	if (elemental == error)
		return 0;
	endif

	var arcana := attackInfo.GetAttacker().GetSkill(ARCANA);
	var intBonus := CInt(attackInfo.GetAttacker().GetInt() / 20);
	var arcanaBonus := CInt(arcana / 10);

	return intBonus + arcanaBonus;
endfunction

/**
 * Applies primordial elemental bonus for metamorfose elemental transformations.
 *
 * @param attackInfo AttackInfo Combat information containing attacker and damage data
 */
function CalculatePrimordialDamage(byRef attackInfo)
	if (GetObjProperty(attackInfo.GetAttacker().GetObjRef(), "metamorfose") == error)
		return 0;
	endif
	
	var damage := attackInfo.GetDamage();
	var rawDamage := damage.GetBluntRawDamage();
	
	if (GetObjProperty(attackInfo.GetAttacker().GetObjRef(), "primordial_fire") != error)
		damage.IncreaseFireRawDamage(rawDamage);
	elseif (GetObjProperty(attackInfo.GetAttacker().GetObjRef(), "primordial_cold") != error)
		damage.IncreaseColdRawDamage(rawDamage);
	elseif (GetObjProperty(attackInfo.GetAttacker().GetObjRef(), "primordial_poison") != error)
		damage.IncreasePoisonRawDamage(rawDamage);	
	elseif (GetObjProperty(attackInfo.GetAttacker().GetObjRef(), "primordial_energy") != error)
		damage.IncreaseEnergyRawDamage(rawDamage);
	endif

	attackInfo.SetDamage(damage);
	return rawDamage;
endfunction

/**
 * Applies elemental bonus damage for MagicBow (Arcane Archer).
 * With ARCANE_MASTERY: arcana / 10. Without: arcana / 20.
 * Injected into attackInfo so critical/resist/debug all work automatically.
 */
function CalculateArcaneBowDamage(byRef attackInfo)
	if (attackInfo.GetWeapon().GetCategory() != WEAPON_CATEGORY::MAGICBOW)
		return;
	endif

	var arcana := attackInfo.GetAttacker().GetSkill(ARCANA);
	var bonus;

	if (attackInfo.GetAttacker().HasAbility(ABILITY_ID::ARCANE_MASTERY))
		bonus := CInt(arcana / 5);
	else
		bonus := CInt(arcana / 10);
	endif

	if (bonus < 1)
		bonus := 1;
	endif

	var damage := attackInfo.GetDamage();
	var weapon := attackInfo.GetWeapon().GetObjRef();

	if (weapon.GetProp("fire_arrow"))
		damage.IncreaseFireRawDamage(bonus);
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Bônus elemental do Arco Arcano (fire): +{bonus}"));
	elseif (weapon.GetProp("cold_arrow"))
		damage.IncreaseColdRawDamage(bonus);
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Bônus elemental do Arco Arcano (cold): +{bonus}"));
	elseif (weapon.GetProp("energy_arrow"))
		damage.IncreaseEnergyRawDamage(bonus);
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Bônus elemental do Arco Arcano (energy): +{bonus}"));
	endif

	attackInfo.SetDamage(damage);
endfunction

/**
 * Applies flat physical bonus damage for Spiritual Blade (Lâmina Arcana).
 * Scales with Arcana skill: arcana / 10.
 * Only applies when wielding a SpiritualBlade category weapon.
 */
function CalculateSpiritualBladeDamage(byRef attackInfo)
	if (attackInfo.GetWeapon().GetCategory() != WEAPON_CATEGORY::SPIRITUALBLADE)
		return;
	endif

	var arcana := attackInfo.GetAttacker().GetSkill(ARCANA);
	var bonus := CInt(arcana / 5);
	if (bonus < 1)
		bonus := 1;
	endif

	var damage := attackInfo.GetDamage();
	var posture := attackInfo.GetAttacker().GetCombatTraits().GetPosture();

	if (posture == DAMAGE_TYPE::SLASH)
		damage.IncreaseSlashRawDamage(bonus);
	elseif (posture == DAMAGE_TYPE::PIERCING)
		damage.IncreasePiercingRawDamage(bonus);
	elseif (posture == DAMAGE_TYPE::BLUNT)
		damage.IncreaseBluntRawDamage(bonus);
	endif

	attackInfo.SetDamage(damage);

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Bônus da Lâmina Espiritual ({posture}): +{bonus}"));
endfunction

/**
 * Applies elemental damage for Sword Mage (Lâmina Arcana) enchanted weapons.
 * Converts a percentage of the active posture's physical raw damage to elemental damage,
 * based on Arcana skill (Arcana/2 %). Also adds a flat INT/20 elemental bonus.
 * Consumes 1 arcane charge per hit.
 */
function CalculateElementalWeaponDamage(byRef attackInfo)
	var weapon := attackInfo.GetWeapon().GetObjRef();
	var elemental := weapon.GetProperty("#elementalweapon");

	if (elemental == error)
		return;
	endif

	var damage := attackInfo.GetDamage();
	var posture := attackInfo.GetAttacker().GetCombatTraits().GetPosture();

	// Get the active posture's physical raw damage
	var physicalRaw := 0;
	if (posture == DAMAGE_TYPE::SLASH)
		physicalRaw := damage.GetSlashRawDamage();
	elseif (posture == DAMAGE_TYPE::PIERCING)
		physicalRaw := damage.GetPiercingRawDamage();
	elseif (posture == DAMAGE_TYPE::BLUNT)
		physicalRaw := damage.GetBluntRawDamage();
	endif

	// Calculate conversion: Arcana/2 % of physical damage becomes elemental
	var arcana := attackInfo.GetAttacker().GetSkill(ARCANA);
	var conversionRate := arcana / 200.0;
	var convertedDamage := CInt(physicalRaw * conversionRate);

	// Flat INT bonus + Arcana/10 bonus on top
	var intBonus := CInt(attackInfo.GetAttacker().GetInt() / 20);
	var arcanaBonus := CInt(arcana / 10);
	var totalElemental := convertedDamage + intBonus + arcanaBonus;

	if (totalElemental < 1)
		return;
	endif

	// Subtract converted portion from physical
	if (posture == DAMAGE_TYPE::SLASH)
		damage.DecreaseSlashRawDamage(convertedDamage);
	elseif (posture == DAMAGE_TYPE::PIERCING)
		damage.DecreasePiercingRawDamage(convertedDamage);
	elseif (posture == DAMAGE_TYPE::BLUNT)
		damage.DecreaseBluntRawDamage(convertedDamage);
	endif

	// Add elemental damage
	if (elemental.type == "Fire")
		damage.IncreaseFireRawDamage(totalElemental);
	elseif (elemental.type == "Cold")
		damage.IncreaseColdRawDamage(totalElemental);
	elseif (elemental.type == "Energy")
		damage.IncreaseEnergyRawDamage(totalElemental);
	endif

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Lâmina Arcana ({elemental.type}): Converteu {convertedDamage} físico + {intBonus} INT + {arcanaBonus} Arcana = {totalElemental} elemental"));

	// Consume 1 arcane charge per hit
	elemental.cargas := elemental.cargas - 1;
	if (elemental.cargas <= 0)
		weapon.EraseProp("#elementalweapon");
		weapon.color := elemental.originalColor;
		IncRevision(weapon);
		if (weapon.character_owner != error)
			Message("Sua arma não está mais encantada.", Color(SYSTEM_COLOR::FAIL)).SendSys(weapon.character_owner);
		endif
	else
		weapon.SetProp("#elementalweapon", elemental);
	endif

	attackInfo.SetDamage(damage);
endfunction

function CalculateFlankingDamage(byRef attackInfo)
	if (attackInfo.GetDirection() != ATTACK_DIRECTION::BACK)
		return;
	endif

	var bonus := RandomDiceRoll("1d4");
	var posture := attackInfo.GetAttacker().GetCombatTraits().GetPosture();

	if (posture == DAMAGE_TYPE::SLASH)
		attackInfo.GetDamage().IncreaseSlashRawDamage(bonus);
	elseif (posture == DAMAGE_TYPE::PIERCING)
		attackInfo.GetDamage().IncreasePiercingRawDamage(bonus);
	elseif (posture == DAMAGE_TYPE::BLUNT)
		attackInfo.GetDamage().IncreaseBluntRawDamage(bonus);
	endif

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Bônus de flanquear (costas): +{bonus}"));
endfunction

function CalculateDefenderResist(byRef attackInfo)
	// Check if ability set IgnoreArmor flag (encontrarbrecha, marcadacaca, etc.)
	if (attackInfo.IgnoresArmor())
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("[ABILITY] Ignorando toda armadura!"));
		return; // Early return - skip ALL armor reduction
	endif

	var armorsInBodyRegion := GetEquipsByAttackDirection(attackInfo);
	var bodyResist := attackInfo.GetDefender().GetBodyResist();

	attackInfo.GetDamage().DecreaseSlashBaseDamage(bodyResist.GetSlashResist())
						  .DecreasePiercingBaseDamage(bodyResist.GetPiercingResist())
						  .DecreaseBluntBaseDamage(bodyResist.GetBluntResist())
						  .DecreaseFireBaseDamage(bodyResist.GetFireResist())
						  .DecreaseColdBaseDamage(bodyResist.GetColdResist())
						  .DecreasePoisonBaseDamage(bodyResist.GetPoisonResist())
						  .DecreaseEnergyBaseDamage(bodyResist.GetEnergyResist());
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Defesa natural (BODY) do defensor reduziu dano em: {bodyResist.ToString()}"));

	// Spell resist (virtual armor layer from magic: barkskin, protection, etc.)
	var spellResist := attackInfo.GetDefender().GetSpellResist();
	attackInfo.GetDamage().DecreaseSlashBaseDamage(spellResist.GetSlashResist())
						  .DecreasePiercingBaseDamage(spellResist.GetPiercingResist())
						  .DecreaseBluntBaseDamage(spellResist.GetBluntResist())
						  .DecreaseFireBaseDamage(spellResist.GetFireResist())
						  .DecreaseColdBaseDamage(spellResist.GetColdResist())
						  .DecreasePoisonBaseDamage(spellResist.GetPoisonResist())
						  .DecreaseEnergyBaseDamage(spellResist.GetEnergyResist());
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Proteção mágica do defensor reduziu dano em: {spellResist.ToString()}"));

	if (armorsInBodyRegion == array{})
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("Não há armadura na região atingida. Dano direto no corpo do defensor."));
	else
		foreach equip in (armorsInBodyRegion)
			var armorResist := equip.GetResists();

			// WearDownArmor(attackInfo.GetDefender().GetObjRef(), equip.GetObjRef(), attackInfo.GetDamage().GetTotalBaseDamage(), attackInfo.GetWeapon().GetObjRef());
			WearDownEquip(equip, attackInfo.GetWeapon());
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Equipamento {equip.GetName()} reduziu dano em {armorResist.ToString()} e perdeu {attackInfo.GetDamage().GetTotalBaseDamage()} de durabilidade."));

			attackInfo.GetDamage().DecreaseSlashBaseDamage(armorResist.GetSlashResist())
								  .DecreasePiercingBaseDamage(armorResist.GetPiercingResist())
								  .DecreaseBluntBaseDamage(armorResist.GetBluntResist())
								  .DecreaseFireBaseDamage(armorResist.GetFireResist())
								  .DecreaseColdBaseDamage(armorResist.GetColdResist())
								  .DecreasePoisonBaseDamage(armorResist.GetPoisonResist())
								  .DecreaseEnergyBaseDamage(armorResist.GetEnergyResist());
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Restou: {attackInfo.GetDamage().ToString()}"));
		endforeach
	endif

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("Dano resistido:"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::SLASH)}: {CInt(attackInfo.GetDamage().GetSlashRawDamage() - attackInfo.GetDamage().GetSlashBaseDamage())}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: {CInt(attackInfo.GetDamage().GetPiercingRawDamage() - attackInfo.GetDamage().GetPiercingBaseDamage())}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: {CInt(attackInfo.GetDamage().GetBluntRawDamage() - attackInfo.GetDamage().GetBluntBaseDamage())}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::FIRE)}: {CInt(attackInfo.GetDamage().GetFireRawDamage() - attackInfo.GetDamage().GetFireBaseDamage())}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::COLD)}: {CInt(attackInfo.GetDamage().GetColdRawDamage() - attackInfo.GetDamage().GetColdBaseDamage())}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::POISON)}: {CInt(attackInfo.GetDamage().GetPoisonRawDamage() - attackInfo.GetDamage().GetPoisonBaseDamage())}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::ENERGY)}: {CInt(attackInfo.GetDamage().GetEnergyRawDamage() - attackInfo.GetDamage().GetEnergyBaseDamage())}"));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Dano a ser aplicado (após defesa): {attackInfo.GetDamage().ToString()}"));

	_ApplyPhysicalManaShield(attackInfo);
endfunction

// Protecao Arcana (spell 114) — physical-only mana shield. Absorbs slash/piercing/blunt
// (whichever matches the attacker's posture) at 2 mana per 1 damage. Elemental and forced
// damage bypass this layer entirely.
function _ApplyPhysicalManaShield(byRef attackInfo)
	var defender := attackInfo.GetDefender().GetObjRef();
	var shield := attackInfo.GetDefender().GetProperty("#manashield");
	if (!shield || shield.errortext)
		return;
	endif

	if (shield.endtime <= ReadGameClock())
		EraseObjProperty(defender, "#manashield");
		return;
	endif

	var dmg := attackInfo.GetDamage();
	var posture := attackInfo.GetAttacker().GetCombatTraits().GetPosture();
	var hit;
	case (posture)
		DAMAGE_TYPE::SLASH:    hit := dmg.GetSlashBaseDamage();    break;
		DAMAGE_TYPE::PIERCING: hit := dmg.GetPiercingBaseDamage(); break;
		DAMAGE_TYPE::BLUNT:    hit := dmg.GetBluntBaseDamage();    break;
		default:               return;
	endcase
	if (hit <= 0)
		return;
	endif

	var mana := AP_GetVital(defender, MANA);
	var absorb := (hit * 2 <= mana) ? hit : Cint(mana / 2);
	if (absorb <= 0)
		return;
	endif

	AP_ConsumeVital(defender, MANA, absorb * 2);

	case (posture)
		DAMAGE_TYPE::SLASH:    dmg.DecreaseSlashBaseDamage(absorb);    break;
		DAMAGE_TYPE::PIERCING: dmg.DecreasePiercingBaseDamage(absorb); break;
		DAMAGE_TYPE::BLUNT:    dmg.DecreaseBluntBaseDamage(absorb);    break;
	endcase

	if (AP_GetVital(defender, MANA) <= 0)
		EraseObjProperty(defender, "#manashield");
		PrintTextAbove(defender, "*barreira arcana quebra*");
	endif
endfunction

function GetEquipsByAttackDirection(byRef attackInfo)
	var armorsInBodyRegion := attackInfo.GetDefender().GetEquipBySlot(attackInfo.GetBodyRegion());
	var FRONT_TO_BACK := array{
		LAYER_ROBE,
		LAYER_TUNIC,
		LAYER_SKIRT,
		LAYER_HELM,
		LAYER_GLOVES,
		LAYER_NECK,
		LAYER_WAIST,
		LAYER_ARMS,
		LAYER_CHEST,
		LAYER_SHIRT,
		LAYER_LEGS,
		LAYER_PANTS,
		LAYER_SHOES,
		LAYER_CAPE
	};
	var BACK_TO_FRONT := array{
		LAYER_CAPE,
		LAYER_ROBE,
		LAYER_TUNIC,
		LAYER_SKIRT,
		LAYER_HELM,
		LAYER_ARMS,
		LAYER_WAIST,
		LAYER_CHEST,
		LAYER_SHIRT,
		LAYER_LEGS,
		LAYER_PANTS,
		LAYER_SHOES,
		LAYER_NECK,
		LAYER_GLOVES
	};

	var orderedEquips := array{};
	var orderArray := attackInfo.GetDirection() == ATTACK_DIRECTION::BACK ? BACK_TO_FRONT : FRONT_TO_BACK;
	var equipNamesForDebug := array{};

	foreach layer in (orderArray)
		foreach equip in (armorsInBodyRegion)
			if (equip.GetLayer() == layer)
				orderedEquips.Append(equip);
				equipNamesForDebug.Append(equip.GetName());
			endif
		endforeach
	endforeach

	if (equipNamesForDebug.size() > 0)
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Ordem dos equipamentos a serem atingidos: {equipNamesForDebug}"));
	endif

	return orderedEquips;
endfunction

function CalculateAttackerCriticalHit(byRef attackInfo)
	// Spells default to skipping crit — CalculateCriticalChance reads body region
	// (via CalculateDefenderCriticalReduction) which spells don't set.
	if (attackInfo.ShouldSkipCrit())
		return true;
	endif

	// Only calculate if not already forced by ability
	if (!attackInfo.GetDamage().IsCritical())
		attackInfo.SetCriticalHitChance(CalculateCriticalChance(attackInfo));
		attackInfo.RollCriticalHitChance();
	endif

	if (attackInfo.GetDamage().IsCritical())
		ApplyCriticalDamage(attackInfo);
	endif

	attackInfo.GetAttacker().EraseProperty("#CriticalChance"); // Limpa a CProp de bonus critico para o próximo ataque

	return true;
endfunction

function CalculateCriticalChance(byRef attackInfo)
	var chance := 0;
	var skillUsedForCritical := TACTICS;

	if (attackInfo.GetAttacker().GetProperty("#nextcritical"))
		attackInfo.GetAttacker().EraseProperty("#nextcritical");
		return 100;
	endif

	if (attackInfo.GetDefender().HasAbility(ABILITY_ID::MUMIFICACAO))
		Message("Seu crítico não surte efeito", Color(SYSTEM_COLOR::FAIL)).SendSys(attackInfo.GetAttacker().GetObjRef());
		Message("Seu corpo evita o crítico", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetDefender().GetObjRef());
		return 0;
	endif

	// Check if druid with metamorfose + survival ability
	if (attackInfo.GetAttacker().GetProperty("metamorfose") != error && attackInfo.GetAttacker().HasAbility(ABILITY_ID::INSTINTO_DE_SOBREVIVENCIA))
		skillUsedForCritical := THEOLOGY;
	endif

	var opponentCriticalReduction := CalculateDefenderCriticalReduction(attackInfo.GetDefender(), attackInfo.GetBodyRegion());

	chance := CInt(
		CInt(attackInfo.GetWeapon().GetCriticalChance()) +	// Crítico base da arma
		CInt(attackInfo.GetAttacker().GetCriticalChance()) +						// Crítico extra do Char (base + mod)
		CInt(attackInfo.GetAttacker().GetSkill(skillUsedForCritical) / 20) -			// Crítico extra da Skill
		CInt(opponentCriticalReduction) //Redução de crítico do defensor
	);

	if (attackInfo.GetAttacker().IsNPC())
		chance := CInt(attackInfo.GetAttacker().GetTemplate().GetTier())
				+ CInt(attackInfo.GetAttacker().GetSkill(skillUsedForCritical) / 20)
				- CInt(opponentCriticalReduction);
	endif

	if (attackInfo.GetAttacker().HasAbility(ABILITY_ID::DESTRUIDOR) && GetCooldown(attackInfo.GetAttacker().GetObjRef(), "furia"))
		chance := chance * 2.0;
	endif

	if (GetCooldown(attackInfo.GetDefender().GetObjRef(), "VontadeIndomavel") ||
		GetCooldown(attackInfo.GetDefender().GetObjRef(), "vitalidadeinabalavel") > 0 ||
	 	GetCooldown(attackInfo.GetDefender().GetObjRef(), "balance_restored"))
		Message("Você não leva o crítico").SendSys(attackInfo.GetDefender().GetObjRef());
		return 0;
	endif

	return CInt(chance);
endfunction

function CalculateDefenderCriticalReduction(defender, bodyRegion)
	var reduction := 0;
	
	foreach equip in (defender.GetEquipBySlot(bodyRegion))
		Sleepms(2);
		var wraper := ArmorWrapper(equip);
		reduction += equip.GetCriticalReduction();
	endforeach

	if(defender.IsNPC())
		reduction += defender.GetTemplate().GetTier() - 1;
	endif

	return reduction;
endfunction

function ApplyCriticalDamage(byRef attackInfo)
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("***DANO CRÍTICO***", Color(GENERAL_COLOR::RED)));

	// TODO: Refatorar isso daqui, tá MUITO cagado....
		// Jogar as coisas de pet pra uma função separada
	
	var weapon_scripts := array{};

	var npcCfg := CFGFile("combat", "npcdesc").SetElementName(attackInfo.GetAttacker().GetTemplate());
	var tamingCfg := CFGFile("taming", "taming").SetElementName(attackInfo.GetAttacker().GetObjRef().GetProp("Critic"));
	var weaponCfg := CFGFile("*", "itemdesc").SetElementName(attackInfo.GetWeapon().GetObjType());

	var pet_instinto := attackInfo.GetAttacker().GetObjRef().GetProp("Group_savage"); // animal tamado com a hab Instinto Selvagem para adicionad criticos diferentes
	var pet_selvagem := attackInfo.GetAttacker().GetObjRef().GetProp("Selvagem"); // verifica se é animal selvagem - nunca tamado, para dar dano critico
	var pet_txt := tamingCfg.GetPropertyValue("Text");

	if (pet_instinto || pet_selvagem) // Se for pet
		weapon_scripts := tamingCfg.GetPropertyValue("CriticalHitScripts");
		Message(pet_txt[1]).SendAbove(attackInfo.GetAttacker().GetObjRef());
	elseif (attackInfo.GetWeapon())
		weapon_scripts := weaponCfg.GetPropertyValue("CriticalHitScripts");
	elseif (attackInfo.GetAttacker().IsNPC())
		weapon_scripts := npcCfg.GetPropertyValue("CriticalHitScripts");
	endif

	if (!weapon_scripts || !weapon_scripts.Size())
		weapon_scripts := array{ ":combat:criticalHitScripts/damage2" };
	endif
	// PEnsar numa solucao pra mensagem subir no NPC sem o [Language] como prefixo
	Message("CRÍTICO", Color(GENERAL_COLOR::RED)).SendEmote(attackInfo.GetAttacker().GetObjRef());
	// Calculate effective raw damage using only active posture's physical damage + elementals
	// (stat/skill bonuses are added to all 3 physical types, but only the active posture is applied)
	var posture := attackInfo.GetAttacker().GetCombatTraits().GetPosture();
	var effectiveRawDmg := 0;
	if (posture == DAMAGE_TYPE::SLASH)
		effectiveRawDmg := attackInfo.GetDamage().GetSlashRawDamage();
	elseif (posture == DAMAGE_TYPE::PIERCING)
		effectiveRawDmg := attackInfo.GetDamage().GetPiercingRawDamage();
	elseif (posture == DAMAGE_TYPE::BLUNT)
		effectiveRawDmg := attackInfo.GetDamage().GetBluntRawDamage();
	endif
	effectiveRawDmg += attackInfo.GetDamage().GetFireRawDamage()
					 + attackInfo.GetDamage().GetColdRawDamage()
					 + attackInfo.GetDamage().GetPoisonRawDamage()
					 + attackInfo.GetDamage().GetEnergyRawDamage();

	foreach hitscript in (weapon_scripts)
		// Damage multipliers: multiply the Damage object so it flows through armor + ApplyCombatDamage as one result
		if (hitscript["damage2"] || hitscript["damage3"])
			var multiplier := hitscript["damage3"] ? 2 : 1; // damage2 = 2x total (increase by 1x), damage3 = 3x total (increase by 2x)
			var d := attackInfo.GetDamage();

			if (posture == DAMAGE_TYPE::SLASH)
				d.IncreaseSlashRawDamage(d.GetSlashRawDamage() * multiplier);
			elseif (posture == DAMAGE_TYPE::PIERCING)
				d.IncreasePiercingRawDamage(d.GetPiercingRawDamage() * multiplier);
			elseif (posture == DAMAGE_TYPE::BLUNT)
				d.IncreaseBluntRawDamage(d.GetBluntRawDamage() * multiplier);
			endif

			d.IncreaseFireRawDamage(d.GetFireRawDamage() * multiplier)
			 .IncreaseColdRawDamage(d.GetColdRawDamage() * multiplier)
			 .IncreasePoisonRawDamage(d.GetPoisonRawDamage() * multiplier)
			 .IncreaseEnergyRawDamage(d.GetEnergyRawDamage() * multiplier);

			continue;
		endif

		foreach armor in (attackInfo.GetDefender().GetEquipBySlot(attackInfo.GetBodyRegion()))
			var script := start_script(
				hitscript,
				array{
					attackInfo.GetAttacker().GetObjRef(),
					attackInfo.GetDefender().GetObjRef(),
					attackInfo.GetWeapon().GetObjRef(),
					armor.GetObjRef(),
					attackInfo.GetDamage().GetTotalBaseDamage(),
					effectiveRawDmg
				}
			);
			if (!script || script.errortext)
				Message("*Attacker* Weapon script error starting [" + hitscript + "] :" + script.errortext, Color(SYSTEM_COLOR::FAIL)).SendSys(attackInfo.GetAttacker().GetObjRef());
			endif

			SleepMS(2);
		endforeach
	endforeach

	return true;
endfunction

function ApplyCombatDamage(byRef attackInfo)
	ExecuteCombatAbilitiesOnHit(attackInfo);

	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
	DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"Dano aplicado em: {attackInfo.GetDefender().GetName()}"));

	var posture := attackInfo.GetAttacker().GetCombatTraits().GetPosture();
	if (posture == DAMAGE_TYPE::SLASH)
		attackInfo.GetDamage().SetPiercingRawDamage(0).SetBluntRawDamage(0);
		if (attackInfo.GetDamage().GetSlashBaseDamage() < 1)
			attackInfo.GetDamage().SetSlashBaseDamage(1);
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::SLASH)}: Dano mínimo (1) garantido.", Color(SYSTEM_COLOR::FAIL)));
		else
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::SLASH)}: {attackInfo.GetDamage().GetSlashBaseDamage()}"));
		endif

		DisplayDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetSlashBaseDamage(), attackInfo.GetAttacker().GetObjRef());
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetSlashBaseDamage());
	elseif (posture == DAMAGE_TYPE::PIERCING)
		attackInfo.GetDamage().SetSlashRawDamage(0).SetBluntRawDamage(0);
		if (attackInfo.GetDamage().GetPiercingBaseDamage() < 1)
			attackInfo.GetDamage().SetPiercingBaseDamage(1);
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: Dano mínimo (1) garantido.", Color(SYSTEM_COLOR::FAIL)));
		else
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: {attackInfo.GetDamage().GetPiercingBaseDamage()}"));
		endif

		DisplayDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetPiercingBaseDamage(), attackInfo.GetAttacker().GetObjRef());
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetPiercingBaseDamage());
	elseif (posture == DAMAGE_TYPE::BLUNT)
		attackInfo.GetDamage().SetSlashRawDamage(0).SetPiercingRawDamage(0);
		if (attackInfo.GetDamage().GetBluntBaseDamage() < 1)
			attackInfo.GetDamage().SetBluntBaseDamage(1);
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: Dano mínimo (1) garantido.", Color(SYSTEM_COLOR::FAIL)));
		else
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: {attackInfo.GetDamage().GetBluntBaseDamage()}"));
		endif

		DisplayDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetBluntBaseDamage(), attackInfo.GetAttacker().GetObjRef());
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetBluntBaseDamage());
	endif

	if (attackInfo.GetDamage().GetFireBaseDamage() > 0)
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetFireBaseDamage(), attackInfo.GetAttacker().GetObjRef());
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetFireBaseDamage());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::FIRE)}: {attackInfo.GetDamage().GetFireBaseDamage()}"));
	else
		attackInfo.GetDamage().SetFireBaseDamage(0); // Garante que dano elemental que ficou negativo por causa da resistência não seja aplicado
	endif

	if (attackInfo.GetDamage().GetColdBaseDamage() > 0)
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetColdBaseDamage(), attackInfo.GetAttacker().GetObjRef());
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetColdBaseDamage());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::COLD)}: {attackInfo.GetDamage().GetColdBaseDamage()}"));
	else
		attackInfo.GetDamage().SetColdBaseDamage(0); // Garante que dano elemental que ficou negativo por causa da resistência não seja aplicado
	endif

	if (attackInfo.GetDamage().GetPoisonBaseDamage() > 0)
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetPoisonBaseDamage(), attackInfo.GetAttacker().GetObjRef());
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetPoisonBaseDamage());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::POISON)}: {attackInfo.GetDamage().GetPoisonBaseDamage()}"));
	else
		attackInfo.GetDamage().SetPoisonBaseDamage(0); // Garante que dano elemental que ficou negativo por causa da resistência não seja aplicado
	endif

	if (attackInfo.GetDamage().GetEnergyBaseDamage() > 0)
		DisplayDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetEnergyBaseDamage(), attackInfo.GetAttacker().GetObjRef());
		ApplyRawDamage(attackInfo.GetDefender().GetObjRef(), attackInfo.GetDamage().GetEnergyBaseDamage());
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::ENERGY)}: {attackInfo.GetDamage().GetEnergyBaseDamage()}"));
	else
		attackInfo.GetDamage().SetEnergyBaseDamage(0); // Garante que dano elemental que ficou negativo por causa da resistência não seja aplicado
	endif

	var totalDamage := attackInfo.GetDamage().GetTotalBaseDamage();
	if (totalDamage > 0)
		var owner := ResolvePetOwner(attackInfo.GetAttacker().GetObjRef());
		if (owner)
			MostraDano(owner, attackInfo.GetAttacker().GetObjRef(), totalDamage);
		endif
		AccumulateVinganca(attackInfo.GetDefender().GetObjRef(), totalDamage);
	endif
endfunction

function AccumulateVinganca(defender, amount)
	var current := GetObjProperty(defender, "#vinganca");
	if (current == error)
		return;
	endif
	SetObjProperty(defender, "#vinganca", CInt(current) + CInt(amount));
endfunction

function AttackLogger(attackInfo)
		LogLastHit(attackInfo);
		// Outros logs são criados em cada sua função abaixo
endfunction

function LogLastHit(attackInfo)
		//inserção by Frajolas, para funcionar o VigiliaGM + Sistema de Investigação
		var defender := attackInfo.GetDefender().GetObjRef();
		var attacker := attackInfo.GetAttacker().GetObjRef();
		var lastHit := dictionary;
		lastHit["attacker"] := attacker.serial;
		lastHit["amount"] := attackInfo.GetDamage().GetTotalBaseDamage();
		lastHit["time"] := polcore().systime;
		lastHit["type"] := attackInfo.GetDamage().GetFireBaseDamage() > 0 ? "fire" :
							attackInfo.GetDamage().GetColdBaseDamage() > 0 ? "cold" :
							attackInfo.GetDamage().GetPoisonBaseDamage() > 0 ? "poison" :
							attackInfo.GetDamage().GetEnergyBaseDamage() > 0 ? "energy" : "physical";
		lastHit["bodyregion"] := attackInfo.GetBodyRegion();
		SetObjProperty(defender, "lastHit", lastHit);

		// Feed the NPC threat model. Players don't run brains, so guard on npctemplate.
		if (defender.npctemplate && attacker && attacker != defender)
			SendEvent(defender, struct{
				"type"   := EVID_THREAT_UPDATE,
				"source" := attacker,
				"amount" := lastHit["amount"]
			});
		endif
endfunction

function ResolvePetOwner(attacker)
	if (!attacker || !attacker.npctemplate)
		return 0;
	endif

	var owner := attacker.master;
	if (owner && owner.acctname && owner.connected)
		return owner;
	endif

	var ownerSerial := CInt(GetObjProperty(attacker, "owner"));
	if (!ownerSerial)
		ownerSerial := CInt(GetObjProperty(attacker, "owneri"));
	endif

	if (!ownerSerial)
		return 0;
	endif

	owner := SystemFindObjectBySerial(ownerSerial);
	if (!owner || !owner.acctname || !owner.connected)
		return 0;
	endif

	return owner;
endfunction

function CheckPoison(byRef attackInfo)
	// TODO: Tem várias melhorias que podem ser feitas aqui. Vou deixar pra depois,se der algum bug no combate com veneno. Por enquanto vai tomar muito tempo.
	// TODO: Sugestão de melhoria: Criar um CanPoison() e depois um TryToPoison(), separando a lógica de verificação da aplicação do veneno em si.
	if (attackInfo.GetDamage().GetTotalBaseDamage() < 1)
		return false;
	endif

	// TODO: Não está na lista de habilidades
	// if (attackInfo.GetDefender().HasAbility(ABILITY_ID::SENSO_DA_NATUREZA))
	// 	Message("Você é imune ao efeito dos venenos.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetDefender().GetObjRef());
	// 	return false;
	// endif

	if (attackInfo.GetAttacker().IsNPC()) // Se for NPC, checa pela skill dele
		var poisonchance := GetConfigInt(attackInfo.GetAttacker().GetTemplate(), "Poisoning");
		if (poisonChance > 0)
			if (RandomInt(100) < poisonChance)
				var poisonName := GetConfigString(attackInfo.GetAttacker().GetTemplate(), "PoisonType");
				var level := Cint(GetConfigInt(attackInfo.GetAttacker().GetTemplate(), "PoisonLevel"));

				if (!level)
					level := 1;
				endif

				if (poisonName == error)
					return false;
				endif

				var time := level * 15;
				if (poisonName == "paralyzePoison")
					time := 10;
				endif

				if (IsPoisonImune(attackInfo.GetDefender().GetObjRef(), poisonName))
					Message("Você está imune ao efeito dos venenos.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetDefender().GetObjRef());
					return false;
				endif

				if (CheckResist(attackInfo.GetDefender().GetObjRef(), FORTITUDE, RESIST_EASY, -2 * level))
					Message("Você resistiu ao veneno.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetDefender().GetObjRef());
					return false;
				else
					TS_StartTimer(attackInfo.GetDefender().GetObjRef(), poisonName, time, level, attackInfo.GetAttacker().GetObjRef());
				endif
			endif
		else
			return false;
		endif
	else // Se for player, checa pela arma/munição
		var sourcePoison := attackInfo.GetAmmo().GetItem().GetObjRef() ?: attackInfo.GetWeapon().GetObjRef();
		var poisonInfo := sourcePoison.GetProp("poisonInfo");

		if (poisonInfo == error)
			return false;
		endif

		if (poisonInfo.dosage != "infinito")
			poisonInfo.dosage := poisonInfo.dosage - 1;
			if (poisonInfo.dosage <= 0) // TODO: Talvez dê bug na aplicação do veneno se a dosagem for 1, prestar atenção nisso aqui quando o combate estiver pronto
				Message("Sua carga de veneno acabou.", Color(SYSTEM_COLOR::FAIL)).SendSys(attackInfo.GetAttacker().GetObjRef());
				sourcePoison.EraseProp("poisonInfo");
				return false;
			else
				Message($"Restam {poisonInfo.dosage} doses de {poisonInfo.name}.", Color(SYSTEM_COLOR::ALERT)).SendSys(attackInfo.GetAttacker().GetObjRef());
				sourcePoison.SetProp("poisonInfo", poisonInfo);
			endif
		endif

		if (IsPoisonImune(attackInfo.GetDefender().GetObjRef(), poisonInfo.name))
			Message("Seu veneno parece não ter surtido efeito no alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(attackInfo.GetAttacker().GetObjRef());
			Message("Você está imune ao efeito dos venenos.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetDefender().GetObjRef());
			return false;
		endif

		if (poisonInfo.name == "paralyzePoison")
			var duration := CInt(attackInfo.GetAttacker().GetSkill(SCIENCE) / 20);

			if (duration < 2)
				duration := 2;
			endif

			if (attackInfo.GetAttacker().HasAbility(ABILITY_ID::VENENO_PURO))
				duration := duration * 2;
			endif

			poisonInfo.time := duration;
		endif

		if (CheckResist(attackInfo.GetDefender().GetObjRef(), FORTITUDE, poisonInfo.difficulty, -poisonInfo.difficulty_mod))
			Message("Seu veneno parece não ter surtido efeito no alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(attackInfo.GetAttacker().GetObjRef());
			Message("Você resistiu ao veneno.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetDefender().GetObjRef());
			return false;
		else
			if (TS_GetTimerInfo(attackInfo.GetDefender().GetObjRef(), "defaultPoison")) // confere se está sob default poison para estabelecer o mais forte
				var defaultpoison := cint(TS_GetTimerLevel(attackInfo.GetDefender().GetObjRef(), "defaultPoison"));
				attackInfo.GetDefender().GetObjRef().SetProp("#poisontier", defaultpoison);
			endif
			
			if (attackInfo.GetDefender().GetObjRef().GetProp("#poisontier"))
				var poison := TS_GetTimerNames(attackInfo.GetDefender().GetObjRef(), type := "P");
				var teste := CInt(attackInfo.GetDefender().GetObjRef().GetProp("#poisontier"));

				if (teste <= poisonInfo.tier)
					// compara o poison atual com o novo, substituindo caso o novo seja maior
					TS_LowerDuration(attackInfo.GetDefender().GetObjRef(), poison, -1);
					TS_StartTimer(attackInfo.GetDefender().GetObjRef(), poisonInfo.Name, poisonInfo.time, poisonInfo.level, attackInfo.GetAttacker().GetObjRef());
				endif
			else
				attackInfo.GetDefender().GetObjRef().SetProp("#poisontier", poisonInfo.tier);
				attackInfo.GetDefender().GetObjRef().SetProp("#poisonbonus", poisonInfo.difficulty_mod);
				TS_StartTimer(attackInfo.GetDefender().GetObjRef(), poisonInfo.name, poisonInfo.time, poisonInfo.level, attackInfo.GetAttacker().GetObjRef());
			endif
		endif
	endif

	Message("Seu veneno surtiu efeito no alvo!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(attackInfo.GetAttacker().GetObjRef());
	Message("Você foi envenenado.", Color(SYSTEM_COLOR::FAIL)).SendSys(attackInfo.GetDefender().GetObjRef());
	return true;
endfunction

function CalculateEntropyDamage(byRef attackInfo)
	var damage := attackInfo.GetDamage();
	var bonus_damage := 0;

	// Player entropy bonus: +1 damage per 4k entropy against undead/demons (12k-20k range)
	if (!attackInfo.GetAttacker().IsNPC() && attackInfo.GetDefender().IsNPC())
		var attacker_entropy := attackInfo.GetAttacker().GetEntropy();
		if (attacker_entropy >= 12000)
			var npc_category := Lower(attackInfo.GetDefender().GetEnvironment());
			if (npc_category == "undead" || npc_category == "demon" || npc_category == "void")
				// +1 per 4k entropy, max +5 at 20k
				var entropy_bonus := CInt(attacker_entropy / 4000);
				if (entropy_bonus > 5)
					entropy_bonus := 5;
				endif
				bonus_damage += entropy_bonus;
				DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Bônus de entropia contra {npc_category}: +{entropy_bonus}");
			endif
		endif
	endif

	// Global entropy bonus: NPCs (undead/demon/void) get +1 damage per -20k global entropy
	var globalEntropy := CInt(GetGlobalProperty("GlobalEntropy"));
	if (globalEntropy <= -20000 && attackInfo.GetAttacker().IsNPC())
		var npc_category := Lower(attackInfo.GetAttacker().GetEnvironment());
		if (npc_category == "undead" || npc_category == "demon" || npc_category == "void")
			// +1 per -20k global entropy, max +5 at -100k
			var global_bonus := CInt(Abs(globalEntropy) / 20000);
			if (global_bonus > 5)
				global_bonus := 5;
			endif
			bonus_damage += global_bonus;
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, $"Bônus de entropia global para {npc_category}: +{global_bonus}");
		endif
	endif

	// Apply bonus damage to all damage types proportionally
	if (bonus_damage > 0)
		if (attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::SLASH)
			damage.IncreaseSlashRawDamage(bonus_damage);
		elseif (attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::PIERCING)
			damage.IncreasePiercingRawDamage(bonus_damage);
		elseif (attackInfo.GetAttacker().GetCombatTraits().GetPosture() == DAMAGE_TYPE::BLUNT)
			damage.IncreaseBluntRawDamage(bonus_damage);
		endif

		attackInfo.SetDamage(damage);
	endif
endfunction

function ApplyAmmoDamage(byref attackInfo)
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("================="));
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message("Dano da munição:"));

		var ammoItem := attackInfo.GetAmmo().GetItem().GetObjRef();
		var ammoObjtype := ammoItem.objtype;

		// 1. Apply PHYSICAL damage from crafting (fabricado CProp -> componentes -> dano)
		var fabricado := attackInfo.GetAmmo().GetItem().GetProperty(PROPNAME_FABRICADO);
		if (fabricado && fabricado.componentes)
			var physicalDamage := 0;

			// Extract damage from components where caracteristica == "dano"
			foreach comp in (fabricado.componentes)
				if (comp.propriedades && comp.propriedades.caracteristica == "dano")
					var danoValue := comp.propriedades.valorPrincipal;
					physicalDamage += CInt(danoValue);
				endif
			endforeach

			// Apply as PIERCING for arrows/bolts, BLUNT for fireweapon ammo
			if (attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::BOW ||
				attackInfo.GetWeapon().GetCategory() == WEAPON_CATEGORY::CROSSBOW)
				attackInfo.GetDamage().IncreasePiercingRawDamage(physicalDamage);
				DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::PIERCING)}: {physicalDamage}"));
			else
				attackInfo.GetDamage().IncreaseBluntRawDamage(physicalDamage);
				DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::BLUNT)}: {physicalDamage}"));
			endif
		endif

		// 2. Apply ELEMENTAL damage from enchantments (AmmoDamage CProp)
		var ammoDamage := attackInfo.GetAmmo().GetItem().GetProperty("AmmoDamage");
		var fireDamage := ammoDamage.fire ?: 0;
		var coldDamage := ammoDamage.cold ?: 0;
		var poisonDamage := ammoDamage.poison ?: 0;
		var energyDamage := ammoDamage.energy ?: 0;

		if (ammoDamage)
			attackInfo.GetDamage()
				.IncreaseFireRawDamage(fireDamage)
				.IncreaseColdRawDamage(coldDamage)
				.IncreasePoisonRawDamage(poisonDamage)
				.IncreaseEnergyRawDamage(energyDamage);

			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::FIRE)}: {fireDamage}"));
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::COLD)}: {coldDamage}"));
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::POISON)}: {poisonDamage}"));
			DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_DAMAGE, Message($"{Upper(DAMAGE_TYPE::ENERGY)}: {energyDamage}"));
		endif
endfunction