use uo;
use util;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";
include "include/client";
include "include/sounds";

/**
 * Prisao Dimensional (Dimensional Prison) - ABILITY_ID::PRISAO_DIMENSIONAL (2203)
 *
 * Active (ON_USE):
 * - Teleport a target to a dimensional prison for arcana/5 seconds (capped 2-30)
 * - Then return them to their original position
 * - NPC behavior: target random nearby hostile
 */
class DimensionalPrison(CombatAbility)

    function DimensionalPrison(this)
        Super(ABILITY_ID::PRISAO_DIMENSIONAL);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();

        Message("Escolha quem voce deseja aprisionar.", Color(SYSTEM_COLOR::INFO)).SendSys(who);

        var targ := Target(who);

        // NPC behavior - target random nearby hostile
        if (mobile.IsNPC())
            targ := GetRandomNearbyHostile(who);
        endif

        if (!targ)
            Message("Alvo Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return 0;
        endif

        if (!targ.isA(POLCLASS_MOBILE))
            Message("Alvo Invalido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return 0;
        endif

        var dist := Distance(who, targ);
        var arcana := CInt(mobile.GetSkill(ARCANA));
        var maxRange := CInt(arcana / 5);

        if (dist > maxRange)
            Message($"O alvo esta muito distante. Range maximo: {maxRange} tiles.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return 0;
        endif

        PerformAction(who, HUMAN_ANIM::CAST_DIR);
        sleep(2);

        var globe := 0x6E19;
        var hell_gate := 0x6D1D;

        // Store original position
        var origX := targ.x;
        var origY := targ.y;
        var origZ := targ.z;
        var origRealm := targ.realm;

        // Freeze and show effect
        targ.frozen := 1;
        PlayStationaryEffect(targ.x, targ.y, targ.z, globe, 0, 40, 0, who.realm);
        sleep(2);

        // Calculate prison duration
        var duration := CInt(arcana / 5);
        if (duration < 2)
            duration := 2;
        endif
        if (duration > 30)
            duration := 30;
        endif

        // Teleport to prison dimension
        MoveObjectToLocation(targ, 2210, 2653, 97, "britannia_alt", MOVEOBJECT_FORCELOCATION);

        Message($"*aprisiona {targ.name} na dimensao por {duration}s*").SendAbove(who);
        Message("*voce foi aprisionado em outra dimensao!*", Color(SYSTEM_COLOR::FAIL)).SendAbove(targ);

        sleep(duration);

        // Return from prison
        targ.concealed := 1;
        PlayStationaryEffect(origX, origY, origZ, hell_gate, 200, 25, 0, origRealm);
        MoveObjectToLocation(targ, origX, origY, origZ, origRealm, MOVEOBJECT_FORCELOCATION);
        sleep(2);

        targ.concealed := 0;
        targ.frozen := 0;
        PlaySoundEffect(targ, 0x1f2);

        Message("*voce foi libertado da prisao dimensional*", Color(SYSTEM_COLOR::SUCCESS)).SendAbove(targ);

        return 1;
    endfunction

endclass
