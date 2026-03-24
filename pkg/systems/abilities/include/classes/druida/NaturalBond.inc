use uo;
use os;
use util;
use cfgfile;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":taming:taming";
include ":attributes:attributes";
include ":tn:boss";
include ":tn:npcutil";
include ":timedscripts:timedScripts";
include ":epicai:epicai";
include ":datafile:datafile";
include "include/epicNPCUtils";

/**
 * Elo Natural (Natural Bond) - ABILITY_ID::ELO_NATURAL (2100)
 *
 * Active (ON_USE):
 * - Study wild creatures to learn their wildshape forms
 * - Target a valid wild creature, skill check (Magery vs TameDifficulty)
 * - Stores learned forms in wildshape_forms property
 * - Max forms = Magery/20 (doubled with Pack Heart)
 *
 * Passive (via HasAbility checks in other files):
 * - Allows casting spells while polymorphed (cast_spell.src)
 * - Allows keeping rings/necklaces/jewelry while polymorphed
 * - Allows speech while in animal form
 */
class NaturalBond(CombatAbility)

    function NaturalBond(this)
        Super(ABILITY_ID::ELO_NATURAL);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();

        // Handle NPCs differently - automatic transformation
        if (who.npctemplate)
            return this._HandleNPCTransformation(who);
        endif

        // Regular player wildshape study
        Message("Qual criatura selvagem voce deseja estudar para aprender sua forma?").SendSys(who);
        var targ := target(who);

        if (!this._IsValidStudyTarget(who, targ))
            return false;
        endif

        if (!targ.getTameDifficulty())
            Message("Voce nao pode estudar a forma de " + targ.name + ".").SendSys(who);
            return false;
        endif

        var skill := AP_GetSkill(who, MAGERY);

        var info := struct;
        info.+x := who.x;
        info.+y := who.y;
        info.+facing := who.facing;
        info.+difficulty := CInt(targ.getTameDifficulty());

        if (info.difficulty > (skill * 1.2 + 15))
            Message("A criatura " + targ.name + " e complexa demais para voce estudar no momento.").SendSys(who);
            return false;
        endif

        SetObjProperty(who, "#studywait", ReadGameClock() + 5);

        while (this._StudyLoop(who, targ, info))
            SetObjProperty(who, "#studywait", ReadGameClock() + 5);
            sleep(3);

            if (SkillCheck(who, MAGERY, CInt(info.difficulty * 1.2)) > 0)
                Message("Voce estudou cuidadosamente a forma de " + targ.name + ".").SendSys(who);
                this._StoreTransformation(mobile, who, targ);
                break;
            else
                Message("Voce falhou em entender completamente a forma de " + targ.name + ".").SendSys(who);
                return false;
            endif
        endwhile

        return true;
    endfunction

    function _HandleNPCTransformation(this, who)
        var tier := CInt(who.getTier());
        if (!tier)
            tier := 1;
        endif

        var template := GetAnimalTemplateByTier(tier);
        if (template)
            var animal := CreateNpcFromTemplate(":epicai:" + template, who.x, who.y, who.z, 0, who.realm);
            if (animal)
                SetObjProperty(animal, "#original_npc", who.serial);

                who.hidden := 1;
                MoveObjectToLocation(who, 0, 0, 0, "britannia", MOVEOBJECT_FORCELOCATION);

                var params := struct{
                    "animal" := animal.serial,
                    "druid" := who.serial,
                    "x" := animal.x,
                    "y" := animal.y,
                    "z" := animal.z,
                    "realm" := animal.realm
                };

                Start_Script(":abilities:scripts/revertWildshape", params);
                return true;
            endif
        endif

        return false;
    endfunction

    function _IsValidStudyTarget(this, who, targ)
        if (!targ.IsA(POLCLASS_NPC))
            Message("Essa nao e uma criatura valida para estudo.").SendSys(who);
            return false;
        endif

        if (targ.script[":ghaia:tamed"])
            Message("Voce so pode estudar criaturas selvagens.").SendSys(who);
            return false;
        endif

        if (!CheckLineOfSight(who, targ))
            Message("Voce nao pode ver claramente a criatura!").SendSys(who);
            return false;
        endif

        if (isTrueBoss(targ))
            Message("Essa criatura poderosa nao pode ser estudada.").SendSys(who);
            return false;
        endif

        return true;
    endfunction

    function _StudyLoop(this, who, targ, byref info)
        var my_hostiles := ListHostiles(who, 15);
        var targ_hostiles := ListHostiles(targ, 15);

        if (targ.master)
            Message("Voce so pode estudar criaturas selvagens.").SendSys(who);
            return false;
        elseif (targ_hostiles.size() > 0)
            Message("O animal " + targ.name + " esta sob ataque e nao pode ser estudado agora.").SendSys(who);
            return false;
        elseif (my_hostiles.size() > 0)
            Message("Voce nao pode estudar " + targ.name + " enquanto esta sendo atacado.").SendSys(who);
            return false;
        elseif (Distance(who, targ) > 2)
            Message("Voce esta muito longe da criatura para estuda-la.").SendSys(who);
            return false;
        endif

        PrintTextAbove(who, "*observando atentamente " + targ.name + "*");
        return true;
    endfunction

    function _StoreTransformation(this, mobile, who, targ)
        var max_transformations := CInt(AP_GetSkill(who, MAGERY) / 20);
        if (mobile.HasAbility(ABILITY_ID::CORACAO_DA_MATILHA))
            max_transformations := max_transformations * 2;
        endif

        var targ_strength := AP_GetStat(targ, "Strength");
        var targ_dexterity := AP_GetStat(targ, "Dexterity");
        var targ_intelligence := AP_GetStat(targ, "Intelligence");

        var elem := NPC_GetNPCConfig(targ.npctemplate);
        var targ_category := GetConfigString(elem, "Category");
        var targ_taxonomy := elem.Taxonomy;

        Message($"Forca: {targ_strength}").SendSys(who);
        Message($"Destreza: {targ_dexterity}").SendSys(who);
        Message($"Inteligencia: {targ_intelligence}").SendSys(who);

        // Parse physical resist tier number (T0–T5 → 0–5) from config
        var tier_number := 0;
        var fis_str := GetConfigString(elem, "EpicFisResists");
        if (fis_str && !fis_str.errortext)
            var fis_parts := SplitWords(fis_str);
            var tier_str := CStr(fis_parts[1]);  // e.g. "T1"
            if (tier_str.size() >= 2)
                tier_number := CInt(tier_str[2]);  // '1' from "T1"
            endif
        endif

        var transformation := struct;
        transformation.+name := targ.name;
        transformation.+graphic := targ.graphic;
        transformation.+color := targ.color;
        transformation.+gender := targ.gender;
        transformation.+strength := targ_strength;
        transformation.+dexterity := targ_dexterity;
        transformation.+intelligence := targ_intelligence;
        transformation.+category := targ_category;
        transformation.+taxonomy := targ_taxonomy;
        transformation.+resistTier := tier_number;  // 0–5, used for wildshape AR

        var transformation_list := GetObjProperty(who, "wildshape_forms");
        if (transformation_list == error)
            transformation_list := array;
        endif

        // Check if form already exists
        var found_index := 0;
        for i := 1 to transformation_list.size()
            if (transformation_list[i].name == targ.name)
                Message("Voce ja conhece a forma de " + targ.name + ". Atualizando com a nova forma estudada.").SendSys(who);
                found_index := i;
                break;
            endif
        endfor

        if (found_index > 0)
            transformation_list.erase(found_index);
        endif

        // Remove oldest if at max
        if (transformation_list.size() >= max_transformations)
            var old_transformation := transformation_list[1];
            Message("Ao aprender a nova forma " + targ.name + ", voce se esquece da forma " + old_transformation.name + ".").SendSys(who);
            transformation_list.erase(1);
        endif

        transformation_list.append(transformation);
        SetObjProperty(who, "wildshape_forms", transformation_list);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[ELO_NATURAL] Aprendeu forma: {targ.name}", Color(SYSTEM_COLOR::SUCCESS))
        );
    endfunction

endclass
