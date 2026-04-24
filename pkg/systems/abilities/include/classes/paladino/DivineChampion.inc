include ":abilities:models/CombatAbility";
include ":hotbar:custombuff"; // For SendAbilityBuff
/**
 * Campeao Divino (Divine Champion) - ABILITY_ID::CAMPEAO_DIVINO (404)
 *
 * Active (BeforeHit - as DEFENDER):
 * - Transformation: equips divine wings, applies stat buffs (Theology/2)
 * - Duration: 20 seconds (fixed)
 * - While active, retaliates on each incoming hit (rebuke) with
 *   religion-specific effects + Theology/10/2 raw damage
 *
 * Lifecycle:
 *   CanActivate: first-time wing selection (ChooseWings gump, stored permanently)
 *   OnActivate: store back item, equip wings, apply stat buffs, VFX, set #campeaodivino
 *   OnBeforeHit: rebuke - religion-specific retaliation to attacker
 *   OnDeactivate: remove wings, re-equip stored back item, erase flags
 */
class DivineChampion(CombatAbility)

    function DivineChampion(this)
        Super(ABILITY_ID::CAMPEAO_DIVINO);
        this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_HIT;
    endfunction

    function GetDurationFor(this, mobile)
        return 20;
    endfunction

    function OnActivate(this, mobile)
        var who := mobile.GetObjRef();
        var theology := mobile.GetSkill(THEOLOGY);
        var stat_bonus := CInt(theology / 2);
        var duration := 20;
        var wing_type := who.GetProp("chosen_wings");

        // Save current back layer item (layer 0x0A)
        var back_item := GetEquipmentByLayer(who, 0x0A);
        if (back_item)
            mobile.SetProperty("#campeao_stored_back", back_item.serial);
            MoveItemToContainer(back_item, who.backpack);
        endif

        // Create and equip wings
        var wings := CreateItemAtLocation(who.x, who.y, who.z, wing_type, 1, who.realm);
        var equip := EquipItem(who, wings);
        if (!equip)
            Message("Erro ao equipar as asas.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            DestroyItem(wings);
            return;
        endif
        mobile.SetProperty("#campeao_wings_serial", wings.serial);

        // Apply stat bonuses through timed scripts
        TS_StartTimer(who, "strength", duration, stat_bonus, who, 1);
        TS_StartTimer(who, "agility", duration, stat_bonus, who, 1);
        TS_StartTimer(who, "intelligence", duration, stat_bonus, who, 1);

        // Set flag for rebuke
        mobile.SetProperty("#campeaodivino", 1);

        // Visual and sound effects
        PlaySoundEffect(who, 0x208);
        PlayObjectCenteredEffect(who, 0x376A, 1, 32);

        // Add Buff Icon
        SendAbilityBuff(who, ABILITY_ID::CAMPEAO_DIVINO, duration, $"+{stat_bonus} STR/AGI/INT");
    endfunction

    function OnBeforeHit(this, attackInfo)
        var defender := attackInfo.GetDefender();
        var attacker := attackInfo.GetAttacker();
        var paladin := defender.GetObjRef();
        var enemy := attacker.GetObjRef();
        var religiao := defender.GetReligion();
        var theology_bonus := CInt(defender.GetSkill(THEOLOGY) / 10);

        if (theology_bonus < 1)
            theology_bonus := 1;
        endif

        // Divine retaliation visual
        Message("*repreensao divina!*").SendAbove(paladin);
        PlayObjectCenteredEffect(enemy, 0x376A, 1, 16);
        PlaySoundEffect(enemy, 0x208);

        // Apply religion-specific rebuke effect to attacker
        case (religiao)
            VENERATIO:
                attacker.ConsumeMana(theology_bonus * 5);

            BOKK_BHARAZ:
                TS_StartTimer(enemy, "lentidao", theology_bonus);

            HALLEUAD:
                TS_StartTimer(enemy, "blinded", CInt(theology_bonus / 2), 0, paladin);

            ELUITHORTH:
                SetCooldown(enemy, "darkness", theology_bonus);

            KAI:
                attackInfo.GetDamage()
                    .IncreaseSlashRawDamage(-theology_bonus)
                    .IncreasePiercingRawDamage(-theology_bonus)
                    .IncreaseBluntRawDamage(-theology_bonus);
        endcase

        // Apply divine damage to attacker
        var retaliation_damage := CInt(theology_bonus / 2);
        if (retaliation_damage > 0)
            ApplyRawDamage(enemy, retaliation_damage);
        endif

        DebugMsg(
            paladin,
            DEBUG_HAB,
            Message($"[CAMPEAO_DIVINO] Rebuke! Dano: {retaliation_damage}, Religiao: {religiao}", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

    function OnDeactivate(this, mobile)
        var who := mobile.GetObjRef();

        // Remove wings
        var wings_serial := mobile.GetProperty("#campeao_wings_serial");
        if (wings_serial)
            var wings := SystemFindObjectBySerial(wings_serial);
            if (wings)
                MoveItemToContainer(wings, who.backpack);
                DestroyItem(wings);
            endif
            mobile.GetObjRef().EraseProp("#campeao_wings_serial");
        endif

        // Re-equip original back item
        var stored_serial := mobile.GetProperty("#campeao_stored_back");
        if (stored_serial)
            var stored_item := SystemFindObjectBySerial(stored_serial);
            if (stored_item)
                EquipItem(who, stored_item);
            endif
            mobile.GetObjRef().EraseProp("#campeao_stored_back");
        endif

        // Remove rebuke flag
        mobile.GetObjRef().EraseProp("#campeaodivino");
    endfunction

endclass
