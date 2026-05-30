use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":tn:cooldown";
include ":attributes:attributes";
include ":blood:blood";

/**
 * Lamina da Escuridao (Shadow Blade) - ABILITY_ID::LAMINA_DA_ESCURIDAO (1801)
 *
 * Active (BEFORE_DAMAGE, SingleUse):
 * - On next hit, deals mlore/2 forced damage, causes bleed, and silences target for mlore/5s
 */
class ShadowBlade(CombatAbility)

    function ShadowBlade(this)
        Super(ABILITY_ID::LAMINA_DA_ESCURIDAO);
        this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
        this.SingleUse := true;
    endfunction

    function GetDurationFor(this, mobile)
        var duration := CInt(mobile.GetSkill(ARCANA) / 5);
        if (duration < 2)
            duration := 2;
        endif
        return duration;
    endfunction

    function OnBeforeDamage(this, attackInfo)
        var attacker := attackInfo.GetAttacker();
        var defender := attackInfo.GetDefender();
        var mlore := CInt(attacker.GetSkill(ARCANA));

        // Forced shadow damage (bypasses resistances) — fed through combat pipeline
        var shadowDmg := CInt(mlore / 2);
        attackInfo.GetDamage().IncreaseForcedRawDamage(shadowDmg);

        // Bleed effect
        MakeBleed(defender.GetObjRef());

        // Silence
        var silenceDuration := CInt(mlore / 5);
        SetCooldown(defender.GetObjRef(), "silence", silenceDuration);

        // Visual/sound effects
        PlaySoundEffect(defender.GetObjRef(), 0x51D);
        PlayObjectCenteredEffectEx(defender.GetObjRef(), 0x54b3, 10, 20, 1254, 1);
        PlaySoundEffect(defender.GetObjRef(), 0x1FB);

        Message("*lamina sombria*").SendAbove(attacker.GetObjRef());

        DebugMsg(
            attacker.GetObjRef(),
            DEBUG_HAB,
            Message($"[LAMINA_DA_ESCURIDAO] Dano: {shadowDmg}, Silence: {silenceDuration}s", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
