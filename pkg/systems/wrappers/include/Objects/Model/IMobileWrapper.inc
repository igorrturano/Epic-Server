class IMobileWrapper()
	uninit function IMobileWrapper(this, byRef mobRef);

	uninit function GetName(this);
	uninit function GetStr(this);
	uninit function GetDex(this);
	uninit function GetInt(this);
	uninit function GetCurrentHP(this);
	uninit function GetCurrentStamina(this);
	uninit function GetCurrentMana(this);
	uninit function GetMaxHP(this);
	uninit function GetMaxStamina(this);
	uninit function GetMaxMana(this);
	uninit function GetSkill(this, skillName);
	uninit function GetSkillBase(this, skillName);
	uninit function GetSkillMod(this, skillName);

	uninit function SetSkillBase(this, skillName, value);
	uninit function SetSkillMod(this, skillName, value);

	uninit function IsNPC(this);

	uninit function GetNearbyMobiles(this, range, flags, ignoreZ);
	uninit function GetNearbyHostiles(this, range, flags);
	uninit function GetNearbyEnemies(this, range);
	uninit function GetNearbyAllies(this, range);
	uninit function IsAllyOf(this, other);
	uninit function IsRaging(this);
	uninit function HasReligiousBannerEquipped(this);
	uninit function TeleportBehind(this, target);
	uninit function IsInSmokeZone(this);
	uninit function ApplySmokeProtection(this);
	uninit function RemoveSmokeProtection(this);
	uninit function ClearSmokeProtection(this);

	uninit function IsAbilityOnCooldown(this, abilityId);
	uninit function GetAbilityCooldownRemaining(this, abilityId);
	uninit function SetAbilityCooldown(this, abilityId, seconds);
	uninit function ClearAbilityCooldown(this, abilityId);
	uninit function RollbackAbilityActivation(this, abilityId);

	uninit function ApplyItemModifiers(this, byRef itemWrapped);
	uninit function RemoveItemModifiers(this, byRef itemWrapped);
	uninit function RecalcCarryWeightMod(this, excludeSerial);
endclass