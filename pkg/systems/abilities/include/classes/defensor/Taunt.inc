use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":tn:cooldown";
include ":attributes:attributes";
include "include/eventID";

/**
 * Provocar (Taunt) - ABILITY_ID::PROVOCAR (300)
 *
 * OnUse: Taunts nearby NPCs within 8 tiles to attack the player.
 * Duration based on PARRY/2. Sets "resistenciaheroica" cooldown (PARRY/5)
 * which gives evasion bonus vs multiple hostiles (checked in HitController).
 */
class Taunt(CombatAbility)

    function Taunt(this)
        Super(ABILITY_ID::PROVOCAR);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();

        who.gender ? PlaySoundEffect(who, 0x339) : PlaySoundEffect(who, 0x44B);
        PerformAction(who, HUMAN_ANIM::HIT);

        var nearby := ListMobilesNearLocation(who.x, who.y, who.z, 8, who.realm);
        var taunt_duration := CInt(mobile.GetSkill(PARRY) / 2);
        if (taunt_duration < 1)
            taunt_duration := 1;
        endif

        var count := 0;
        var taunted := array{};

        foreach char in nearby
            if (char == who)
                continue;
            endif
            if (char.acctname)
                continue;
            endif
            if (char.dead || char.hidden || char.concealed)
                continue;
            endif
            if (MobileWrapper(char).GetProperty("noattack"))
                continue;
            endif
            if (!CheckLineOfSight(who, char))
                continue;
            endif
            if (GetCooldown(char, "taunted"))
                continue;
            endif

            taunted.append(char);
            count += 1;
        endforeach

        foreach char in taunted
            SendEvent(char, struct{ "type" := EVID_TAUNT, "source" := who });
            SetCooldown(char, "taunted", taunt_duration);
            MobileWrapper(char).SetProperty("#taunter", who.serial);
        endforeach

        Message($"Voce provocou {count} adversarios, por {taunt_duration} segundos.", Color(SYSTEM_COLOR::INFO)).SendSys(who);

        sleep(2);
        SetCooldown(who, "resistenciaheroica", CInt(mobile.GetSkill(PARRY) / 5));

        DebugMsg(who, DEBUG_HAB, Message($"[PROVOCAR] {count} alvos por {taunt_duration}s", Color(SYSTEM_COLOR::SUCCESS)));

        return true;
    endfunction

endclass