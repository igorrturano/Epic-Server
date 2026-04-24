use uo;
use util;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";
include "include/client";
include "include/sounds";
include ":survival:fishingFunctions";

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

        var targ := TargetCoordinates(who);

        // NPC behavior - jump to random direction from opponent
        if (mobile.IsNPC())
            var directions := array{ array{ 1, 0 }, array{ 0, 1 }, array{ -1, 0 }, array{ 0, -1 } };
            var random_direction := directions[RandomInt(4) + 1];
            targ := struct{
                x := who.opponent.x + random_direction[1] * 5,
                y := who.opponent.y + random_direction[2] * 5,
                z := who.opponent.z,
                realm := who.opponent.realm
            };
        endif

        if (!targ)
            Message("Alvo Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            who.concealed := 0;
            who.frozen := 0;
            return 0;
        endif

        var dist := Distance(who, targ);
        var arcana := CInt(mobile.GetSkill(ARCANA));
        var maxRange := CInt(arcana / 5);

        if (!CheckLosAt(who, targ.x, targ.y, targ.z))
            Message("Voce nao consegue ver o alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            who.concealed := 0;
            who.frozen := 0;
            return 0;
        endif

        if (dist > maxRange)
            Message($"O alvo esta muito distante. Range maximo: {maxRange} tiles.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            who.concealed := 0;
            who.frozen := 0;
            return 0;
        endif

        var locinfo := GetMapInfo(targ.x, targ.y, who.realm);
        if (IsWater(targ, locinfo))
            Message("Voce nao pode teleportar para a agua.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            who.concealed := 0;
            who.frozen := 0;
            return 0;
        endif

        if (CanWalk("L", who.x, who.y, who.z, targ.x, targ.y, who.realm) == error)
            Message("Local Inacessivel.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            who.concealed := 0;
            who.frozen := 0;
            return 0;
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
