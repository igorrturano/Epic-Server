use uo;
use util;

include ":abilities:models/CombatAbility";
include ":abilities:utils/tileChecks";
include ":message:model/Message";
include ":attributes:attributes";
include "include/client";
include "include/sounds";

/**
 * Salto Etereo (Ethereal Jump) - ABILITY_ID::SALTO_ETEREO (2201)
 *
 * Active (ON_USE):
 * - Teleport to a targeted location within arcana/5 range
 * - Requires line of sight, non-water, walkable path
 * - NPC behavior: jump to random direction from opponent
 */
class EtherealJump(CombatAbility)

    function EtherealJump(this)
        Super(ABILITY_ID::SALTO_ETEREO);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();

        who.concealed := 1;
        who.frozen := 1;
        PerformAction(who, HUMAN_ANIM::CAST_AREA);

        PlayStationaryEffect(who.x, who.y, who.z, 0xD11B, 0, 35, 0, who.realm);
        PlaySoundEffect(who, 0x1f2);
        sleep(2);

        var arcana := CInt(mobile.GetSkill(ARCANA));
        var maxRange := CInt(arcana / 5);
        var targ;

        if (mobile.IsNPC())
            // NPC: deterministic pick + single-pass validation; abort silently on fail.
            var directions := array{ array{ 1, 0 }, array{ 0, 1 }, array{ -1, 0 }, array{ 0, -1 } };
            var random_direction := directions[RandomInt(4) + 1];
            targ := struct{
                x := who.opponent.x + random_direction[1] * 5,
                y := who.opponent.y + random_direction[2] * 5,
                z := who.opponent.z,
                realm := who.opponent.realm
            };

            if (!CheckLosAt(who, targ.x, targ.y, targ.z)
                || Distance(who, targ) > maxRange
                || _IsWaterTile(targ, GetMapInfo(targ.x, targ.y, who.realm))
                || CanWalk("L", who.x, who.y, who.z, targ.x, targ.y, who.realm) == error)
                who.concealed := 0;
                who.frozen := 0;
                return 0;
            endif
        else
            // Player: re-prompt on bad clicks (LoS / range / water / unwalkable);
            // only explicit cancel aborts.
            while (1)
                targ := TargetCoordinates(who);
                if (!targ)
                    Message("Alvo cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
                    who.concealed := 0;
                    who.frozen := 0;
                    return 0;
                endif
                if (!CheckLosAt(who, targ.x, targ.y, targ.z))
                    Message("Voce nao consegue ver o destino. Escolha outro local.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
                    continue;
                endif
                if (Distance(who, targ) > maxRange)
                    Message($"O destino esta muito distante. Range maximo: {maxRange} tiles. Escolha outro local.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
                    continue;
                endif
                if (_IsWaterTile(targ, GetMapInfo(targ.x, targ.y, who.realm)))
                    Message("Voce nao pode teleportar para a agua. Escolha outro local.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
                    continue;
                endif
                if (CanWalk("L", who.x, who.y, who.z, targ.x, targ.y, who.realm) == error)
                    Message("Local inacessivel. Escolha outro local.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
                    continue;
                endif
                break;
            endwhile
        endif

        MoveObjectToLocation(who, targ.x, targ.y, targ.z, who.realm);

        PlayStationaryEffect(who.x, who.y, who.z, 0xD11B, 0, 35, 0, who.realm);
        PlaySoundEffect(who, 0x1f2);
        sleep(1);

        who.concealed := 0;
        who.frozen := 0;

        return 1;
    endfunction

endclass
