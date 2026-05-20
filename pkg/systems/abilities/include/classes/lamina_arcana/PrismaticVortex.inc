use uo;

include ":abilities:models/CombatAbility";
include ":combat:Controller/DamageController";
include ":combat:Model/Damage";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":destroyable:include/destroyItem";
include ":tn:cooldown";
include ":timedScripts:timedScripts";
include "include/sounds";
include "include/epicConstants";

/**
 * Vortice Prismatico (Prismatic Vortex) - ABILITY_ID::VORTICE_PRISMATICO (2304)
 *
 * Active (ON_USE):
 * - Start 10s absorption window
 * - After 10s, AoE explode with accumulated damage + arcana
 * - Elemental damage type based on weapon color
 *
 * Passive (OnPassiveBeforeDamage - damage accumulation):
 * - While vortice_prismatico cooldown active, accumulate incoming damage
 * - Damage still applies normally (tracking, not absorbing)
 */
class PrismaticVortex(CombatAbility)

    function PrismaticVortex(this)
        Super(ABILITY_ID::VORTICE_PRISMATICO);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
        this.IsDefenderSide := 1;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();

        Message("Voce carrega seu vortice prismatico.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        SetCooldown(who, "vortice_prismatico", 10);
        SetObjProperty(who, "#vortice_dmg", 0);
        PlayObjectCenteredEffectHuefx(who, 0xB468, 10, 60, 0, 1);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message("[VORTICE_PRISMATICO] Iniciando acumulacao de dano por 10s", Color(SYSTEM_COLOR::SUCCESS))
        );

        sleep(10);

        // Release accumulated damage
        var range := CInt(AP_GetSkill(who, ARCANA) / 10);
        var accumulated := CInt(GetObjProperty(who, "#vortice_dmg"));
        var total_dmg := accumulated + CInt(AP_GetSkill(who, ARCANA));
        var weaponElemental := GetObjProperty(who.weapon, "#elementalweapon");
        var effect := weaponElemental.type;

        PerformAction(who, HUMAN_ANIM::CAST_AREA);

        // // AoE damage to nearby objects
        // DanificaObjetosProximos(who, who.x, who.y, who.z, who.realm,
        //     struct{ dano := total_dmg, desc := "Vortice Prismatico." }, range);

        // Elemental damage to nearby hostiles
        var nearby := mobile.GetNearbyMobiles(range);
        var party_members := array{};
        if (who.party)
            party_members := who.party.members;
        endif

        var allPets := array{};
        foreach member in party_members
            if (member != who)
                var memberObj := SystemFindObjectBySerial(member.serial);
                if (memberObj)
                    var pets := ListAllPetsAndSummonedPetsNear(memberObj, 8);
                    foreach pet in pets
                        allPets.append(pet);
                    endforeach
                endif
            endif
        endforeach

        foreach char in nearby
            var char_owner := GetObjProperty(char, "owner");
            if (!(char in party_members) && char != who)
                if (!(char in allPets) && !(char_owner == who.serial))
                    if (effect == "Fire")
                        PlaySoundEffect(char, 0x5D0);
                        PlayObjectCenteredEffect(char, 0x6d77, 7, 60);
                        DealDamage(char, total_dmg, DAMAGE_TYPE::FIRE, who);
                        TS_StartTimer(char, "queimar", 10);
                    elseif (effect == "Cold")
                        PlaySoundEffect(char, 0x65b);
                        PlayObjectCenteredEffectHuefx(char, 0x54f7, 30, 60, COLOR_ELEMENTAL_ICE, 0);
                        DealDamage(char, total_dmg, DAMAGE_TYPE::COLD, who);
                        TS_StartTimer(char, "congelamento", 10);
                    elseif (effect == "Energy")
                        PlaySoundEffect(char, 0x5CF);
                        DealDamage(char, CInt(total_dmg * 1.2), DAMAGE_TYPE::ENERGY, who);
                        PlayObjectCenteredEffect(char, 0xcdaf, 7, 30);
                    endif
                    sleepms(20);
                endif
            endif
        endforeach

        EraseObjProperty(who, "#vortice_dmg");

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[VORTICE_PRISMATICO] Explosao: {total_dmg} dano (acumulado: {accumulated})", Color(SYSTEM_COLOR::SUCCESS))
        );

        return 1;
    endfunction

    function OnPassiveBeforeDamage(this, attackInfo)
        var defender := attackInfo.GetDefender();
        var who := defender.GetObjRef();

        if (!GetCooldown(who, "vortice_prismatico"))
            return false;
        endif

        var incoming := attackInfo.GetDamage().GetTotalBaseDamage();
        var current := CInt(GetObjProperty(who, "#vortice_dmg"));
        SetObjProperty(who, "#vortice_dmg", current + incoming);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[VORTICE_PRISMATICO] Acumulou +{incoming} (total: {current + incoming})", Color(SYSTEM_COLOR::INFO))
        );

        return true;
    endfunction

endclass
