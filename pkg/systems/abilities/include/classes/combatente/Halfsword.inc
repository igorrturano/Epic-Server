use uo;

include ":abilities:models/CombatAbility";
include ":tn:cooldown";
include ":message:model/Message";
include ":debug:Controller/DebugController"; 

class Halfsword(CombatAbility)

    function Halfsword(this)
        Super(ABILITY_ID::HALFSWORD);
        // Set in constructor - class var initializers with enum values have issues
        this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
    endfunction

    function GetDurationFor(this, mobile)
        // Duration = Tactics / 5
        return CInt(mobile.GetSkill(TACTICS) / 5);
    endfunction

    function OnBeforeDamage(this, attackInfo)
        var attacker := attackInfo.GetAttacker();
        var strBonus := attacker.GetMainDamageModifierBy(STRENGTH) * 2;

        if (strBonus > 0)
            attackInfo.GetDamage()
                .IncreaseSlashRawDamage(strBonus)
                .IncreasePiercingRawDamage(strBonus)
                .IncreaseBluntRawDamage(strBonus);

            DebugMsg(
                attacker.GetObjRef(),
                DEBUG_HAB,
                Message($"[HALFSWORD] Bônus de dano por STR x2: +{strBonus}", Color(SYSTEM_COLOR::SUCCESS))
            );
        endif
        return true;
    endfunction

    function OnParry(this, attackInfo)
        var parryEquip := attackInfo.GetParryEquip();
        var parryObjRef := parryEquip.GetObjRef();

        // Must be parrying with a two-handed weapon (not shield)
        if (!parryObjRef.IsA(POLCLASS_WEAPON))
            return false;
        endif

        var attackerWeapon := attackInfo.GetAttacker().GetEquipBySlot(WEAPON);
        if (!attackerWeapon || !attackerWeapon.GetObjRef())
            return false;
        endif

        // Don't disarm unarmed combat (fists)
        if (attackerWeapon.GetCategory() == WEAPON_CATEGORY::UNARMED)
            return false;
        endif

        if (Distance(attackinfo.GetAttacker().GetObjRef(), attackInfo.GetDefender().GetObjRef()) > 2)
            return false;
        endif

        // Calculate disarm timer based on defender's tactics
        var tactics_bonus := CInt(attackInfo.GetDefender().GetSkill(TACTICS) / 5);
        if (tactics_bonus < 1)
            tactics_bonus := 1;
        endif

        // Determine where to drop the weapon (near attacker)
        var attackerPos := attackInfo.GetAttacker().GetPosition();
        var dropX := attackerPos.GetX() + RandomIntMinMax(-1, 1);
        var dropY := attackerPos.GetY() + RandomIntMinMax(-1, 1);
        var realm := attackerPos.GetRealm();
        var dropZ := GetWorldHeight(dropX, dropY, realm);

        // Move attacker's weapon to ground
        if (MoveObjectToLocation(
            attackerWeapon.GetObjRef(),
            dropX,
            dropY,
            dropZ,
            attackerPos.GetRealm(),
            MOVEOBJECT_FORCELOCATION
        ))
            TS_StartTimer(attackInfo.GetAttacker().GetObjRef(), "desarmar", tactics_bonus, 0, attackInfo.GetDefender().GetObjRef());

            Message("*sua arma foi desarmada*")
                .SendAbove(attackInfo.GetAttacker().GetObjRef());

            Message("*desarma a arma do oponente!*")
                .SendAbove(attackInfo.GetDefender().GetObjRef());

            return true;
        endif

        return false;
    endfunction
endclass