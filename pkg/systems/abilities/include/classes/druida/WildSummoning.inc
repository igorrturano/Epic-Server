use uo;
use os;
use util;
use cfgfile;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":gumps:gumps";
include ":attributes:attributes";
include ":attributes:skillCheck";
include ":taming:petidentity";
include ":spells:summon";
include ":timedScripts:timedScripts";
include "include/epicConstants";
include "include/packets";
include "include/sounds";
include "include/epicNPCUtils";

const _WS_BTN_FORGET := 300;
const _WS_BTN_SUMMON := 200;
const _WS_BTN_BOND   := 100;

/**
 * Convocacao Selvagem (Wild Summoning) - ABILITY_ID::CONVOCACAO_SELVAGEM (2101)
 *
 * Active (ON_USE):
 * - Bond with wild creatures to create spirit bonds
 * - Summon bonded creatures as spirit allies
 * - Max bonds = Theology/20 (cap 5)
 * - Summon duration = 60 + Theology*3
 */
class WildSummoning(CombatAbility)

    function WildSummoning(this)
        Super(ABILITY_ID::CONVOCACAO_SELVAGEM);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();

        if (mobile.IsNPC())
            return false;
        endif

        var bonded_creatures := GetObjProperty(who, "spirit_bonds");

        if (!bonded_creatures)
            bonded_creatures := array;
        endif

        if (bonded_creatures.size() == 0)
            PerformAction(who, HUMAN_ANIM::CAST_AREA);
            return this._BondWithCreature(who);
        else
            return this._ShowSpiritBondsGump(who, bonded_creatures);
        endif
    endfunction

    function _ShowSpiritBondsGump(this, who, bonded_creatures)
        var input := this._CreateSummonGump(who, bonded_creatures);

        if (!input[0] || input[0] == 0)
            return false;
        endif

        if (input[0] == _WS_BTN_BOND)
            return this._BondWithCreature(who);
        elseif (input[0] >= _WS_BTN_FORGET)
            var forget_index := input[0] - _WS_BTN_FORGET;
            return this._ForgetBond(who, forget_index, bonded_creatures);
        elseif (input[0] >= _WS_BTN_SUMMON)
            var selected_index := input[0] - _WS_BTN_SUMMON;
            if (selected_index < 1 || selected_index > bonded_creatures.size())
                return false;
            endif

            var selected_creature := bonded_creatures[selected_index];
            return this._SummonSelectedCreature(who, selected_creature);
        endif

        return false;
    endfunction

    function _BondWithCreature(this, who)
        Message("Escolha uma criatura selvagem para criar um vinculo espiritual.").SendSys(who);
        var targ := Target(who);

        if (!this._IsValidBondTarget(who, targ))
            return false;
        endif

        var theology := AP_GetSkill(who, THEOLOGY);
        var max_tier := CInt(theology / 20);

        if (max_tier < 1)
            max_tier := 1;
        elseif (max_tier > 5)
            max_tier := 5;
        endif

        var creature_tier := CInt(targ.getTier());
        if (!creature_tier)
            creature_tier := 1;
        endif

        if (creature_tier > max_tier)
            Message($"A criatura {targ.name} e poderosa demais para voce criar um vinculo. (Tier {creature_tier} requer {creature_tier * 20} de Theology)").SendSys(who);
            return false;
        endif

        SetObjProperty(who, "#bondwait", ReadGameClock() + 5);

        var info := struct;
        info.+x := who.x;
        info.+y := who.y;
        info.+facing := who.facing;
        info.+difficulty := creature_tier * 20;

        while (this._BondLoop(who, targ, info))
            SetObjProperty(who, "#bondwait", ReadGameClock() + 5);
            sleep(3);

            if (SkillCheck(who, THEOLOGY, CInt(info.difficulty)) > 0)
                Message("Voce criou um vinculo espiritual com " + targ.name + ".").SendSys(who);

                PerformAction(who, HUMAN_ANIM::CAST_DIR);
                PlayMovingEffectXYZHued(who, targ, 0xF79, 10, 10, 0, 0);

                this._StoreBond(who, targ);

                PlaySoundEffect(who, SFX_SPELL_SUMMON_ELEMENTAL);
                PlayStationaryEffect(targ.x, targ.y, targ.z, FX_TELEPORT, 10, 30, 0, who.realm);
                targ.hidden := 1;
                MoveObjectToLocation(targ, 6143, 0, 0, "britannia_alt", MOVEOBJECT_FORCELOCATION);

                ApplyRawDamage(targ, AP_GetVital(targ, "Hits") + 100);
                break;
            else
                Message("Voce falhou em criar um vinculo espiritual com " + targ.name + ".").SendSys(who);
                return false;
            endif
        endwhile

        return true;
    endfunction

    function _SummonSelectedCreature(this, who, selected_creature)
        var theology := AP_GetSkill(who, THEOLOGY);
        var max_slots := CInt(theology / 20);
        if (max_slots < 1)
            max_slots := 1;
        elseif (max_slots > 5)
            max_slots := 5;
        endif

        var used_slots := _GetNumSummons(who);
        var creature_tier := CInt(selected_creature.tier);
        if (creature_tier < 1)
            creature_tier := 1;
        endif

        if (used_slots + creature_tier > max_slots)
            Message($"Voce nao tem espacos suficientes para invocar {selected_creature.name}. (Usando {used_slots}/{max_slots}, precisa de {creature_tier})").SendSys(who);
            return false;
        endif

        Message("Escolha o local para invocar " + selected_creature.name + ".").SendSys(who);
        var targ := TargetCoordinates(who);

        if (!targ)
            Message("Cancelado.").SendSys(who);
            return false;
        endif

        PerformAction(who, HUMAN_ANIM::CAST_DIR);
        PlaySoundEffect(who, SFX_SPELL_SUMMON_CREATURE);
        PlayStationaryEffect(targ.x, targ.y, targ.z, FX_SUMMON_BEAST, 10, 30, 0, who.realm);

        sleepms(1500);

        var creature := summon(selected_creature.template, targ, who);
        if (!creature)
            Message("Falha ao invocar o espirito.").SendSys(who);
            return false;
        endif

        creature.color := 30000;
        creature.name := selected_creature.name;
        customizeNpc(creature, CInt(selected_creature.tier));

        PrintTextAbove(who, "*invocando " + selected_creature.name + "*");
        Message("Voce invocou " + selected_creature.name + "!").SendSys(who);

        var duration := 60 + (theology * 3);
        var level := CInt(theology / 5) + 5;

        TS_StartTimer(creature, "summon", duration, level);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[CONVOCACAO_SELVAGEM] Invocou {selected_creature.name} por {duration}s", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

    function _CreateSummonGump(this, who, bonded_creatures)
        var gump := GFCreateGump();

        var theology := AP_GetSkill(who, THEOLOGY);
        var max_bonds := CInt(theology / 20);
        if (max_bonds < 1)
            max_bonds := 1;
        elseif (max_bonds > 5)
            max_bonds := 5;
        endif

        var size := bonded_creatures.size() * 26 + 60;

        GFResizePic(gump, 17, 31, 9390, 359, 125 + size);
        GFHTMLArea(gump, 94, 69, 232, 20, "<BASEFONT Size=5 Color=#999999 >Convocacao Selvagem", 0, 0);
        GFHTMLArea(gump, 96, 71, 232, 20, "<BASEFONT Size=5 Color=#999999 >Convocacao Selvagem", 0, 0);
        GFHTMLArea(gump, 95, 70, 232, 20, "<BASEFONT Size=5 Color=#2d8a2d >Convocacao Selvagem", 0, 0);
        GFGumpPic(gump, 63, 91, 1469);
        GFGumpPic(gump, 80, 91, 1469);
        GFTextLine(gump, 63, 100, 0, "Espiritos Vinculados");
        GFTextLine(gump, 220, 100, 0, "Tier");
        GFTextLine(gump, 255, 100, 0, "Invocar");
        GFTextLine(gump, 299, 100, 0, "Esquecer");
        GFGumpPic(gump, 38, 103, 9773, 0);
        GFGumpPic(gump, 296, 0, 9000, 0);

        var y := 121;
        foreach creature in bonded_creatures
            var color := "2d8a2d";
            GFHTMLArea(gump, 63, y, 152, 24, "<BASEFONT color=#{}>{}".format(color, creature.name), 1, 0);
            GFHTMLArea(gump, 220, y, 30, 24, "<center>{}".format(creature.tier), 1, 0);
            GFAddButton(gump, 255, y + 4, 10006, 10006, GF_CLOSE_BTN, _WS_BTN_SUMMON + _creature_iter);
            GFAddButton(gump, 299, y + 4, 10006, 10006, GF_CLOSE_BTN, _WS_BTN_FORGET  + _creature_iter);
            sleepms(2);
            y += 26;
        endforeach

        y += 10;
        GFGumpPic(gump, 38, y, 9773, 0);
        y += 10;

        var bond_info := "Vinculos: {}/{}".format(bonded_creatures.size(), max_bonds);
        GFHTMLArea(gump, 63, y, 192, 24, "<BASEFONT color=#2d8a2d>{}".format(bond_info), 0, 0);

        if (bonded_creatures.size() < max_bonds)
            GFAddButton(gump, 319, y + 4, 10006, 10006, GF_CLOSE_BTN, _WS_BTN_BOND);
            GFTextLine(gump, 304, y - 20, 0, "Vincular");
        else
            GFGumpPic(gump, 319, y + 5, 10006, 1891);
            GFTextLine(gump, 304, y - 20, 1890, "Vincular");
        endif

        return GFSendGump(who, gump);
    endfunction

    function _IsValidBondTarget(this, who, targ)
        if (!targ.IsA(POLCLASS_NPC))
            Message("Essa nao e uma criatura valida.").SendSys(who);
            return false;
        endif

        if (!targ.isAnimalBrain())
            Message("Voce so pode criar vinculos com animais selvagens.").SendSys(who);
            return false;
        endif

        if (IsTamedPet(targ))
            Message("Voce so pode criar vinculos com criaturas selvagens.").SendSys(who);
            return false;
        endif

        if (!CheckLineOfSight(who, targ))
            Message("Voce nao pode ver claramente a criatura!").SendSys(who);
            return false;
        endif

        if (_IsTrueBoss(targ))
            Message("Essa criatura poderosa nao pode ser vinculada.").SendSys(who);
            return false;
        endif

        return true;
    endfunction

    function _StoreBond(this, who, targ)
        var theology := AP_GetSkill(who, THEOLOGY);
        var max_bonds := CInt(theology / 20);

        if (max_bonds < 1)
            max_bonds := 1;
        elseif (max_bonds > 5)
            max_bonds := 5;
        endif

        var bond := struct;
        bond.+name := targ.name;
        bond.+template := targ.npctemplate;
        bond.+tier := CInt(targ.getTier());

        var bond_list := GetObjProperty(who, "spirit_bonds");
        if (bond_list == error)
            bond_list := array;
        endif

        // Check if bond with same template already exists
        foreach old_bond in bond_list
            if (old_bond.template == targ.npctemplate)
                Message("Voce ja tem um vinculo com " + targ.name + ". O vinculo foi renovado.").SendSys(who);
                bond_list.erase(_old_bond_iter);
                break;
            endif
        endforeach

        if (bond_list.size() >= max_bonds)
            var old_bond := bond_list[1];
            Message("Ao criar um novo vinculo com " + targ.name + ", voce perde o vinculo com " + old_bond.name + ".").SendSys(who);
            bond_list.erase(1);
        endif

        bond_list.append(bond);
        SetObjProperty(who, "spirit_bonds", bond_list);

        Message("Voce agora tem " + bond_list.size() + " vinculo(s) espiritual(is).").SendSys(who);
    endfunction

    function _BondLoop(this, who, targ, byref info)
        var my_hostiles := MobileWrapper(who).GetNearbyHostiles(15);
        var targ_hostiles := MobileWrapper(targ).GetNearbyHostiles(15);

        if (targ.master)
            Message("Voce so pode criar vinculos com criaturas selvagens.").SendSys(who);
            return false;
        elseif (targ_hostiles.size() > 0)
            Message("O animal " + targ.name + " esta sob ataque e nao pode ser vinculado agora.").SendSys(who);
            return false;
        elseif (my_hostiles.size() > 0)
            Message("Voce nao pode criar um vinculo com " + targ.name + " enquanto esta sendo atacado.").SendSys(who);
            return false;
        elseif (Distance(who, targ) > 6)
            Message("Voce esta muito longe da criatura para criar um vinculo.").SendSys(who);
            return false;
        endif

        PerformAction(who, HUMAN_ANIM::CAST_AREA);
        PrintTextAbove(who, "*criando um vinculo espiritual com " + targ.name + "*");

        return true;
    endfunction

    function _ForgetBond(this, who, index, bonds)
        if (index < 1 || index > bonds.size())
            return false;
        endif

        var forgotten := bonds[index];
        bonds.erase(index);
        SetObjProperty(who, "spirit_bonds", bonds);
        Message("Voce quebrou o vinculo espiritual com " + forgotten.name + ".").SendSys(who);
        return true;
    endfunction

endclass

function _IsTrueBoss(targ)
    var spawnpoint := SystemFindObjectBySerial(GetObjProperty(targ, "spawnpoint"));
    if (spawnpoint != error)
        if (GetObjProperty(spawnpoint, "true_boss") != error)
            return 1;
        endif
    endif
    return 0;
endfunction

function _GetNumSummons(who)
    var num_summoned := 0;
    var summoned_pets := GetObjProperty(who, "SummonedPets");

    if (!summoned_pets)
        return 0;
    endif

    foreach serial in summoned_pets
        var pet := SystemFindObjectBySerial(serial);

        if (pet && pet.master == who && GetObjProperty(pet, "summoned"))
            var pet_slot := CInt(pet.getTier());
            if (!pet_slot)
                pet_slot := 1;
            endif
            num_summoned := num_summoned + pet_slot;
        else
            summoned_pets.erase(_serial_iter);
        endif

        sleepms(2);
    endforeach

    SetObjProperty(who, "SummonedPets", summoned_pets);

    return num_summoned;
endfunction
