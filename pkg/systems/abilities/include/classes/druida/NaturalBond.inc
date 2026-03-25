use uo;
use os;
use util;
use cfgfile;
use vitals;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":gumps:gumps";
include ":taming:taming";
include ":attributes:attributes";
include ":tn:boss";
include ":tn:npcutil";
include ":tn:cooldown";
include ":timedscripts:timedScripts";
include ":epicai:epicai";
include ":datafile:datafile";
include ":wrappers:Objects/Model/MobileWrapper";
include ":abilities:models/Ability";
include "include/epicNPCUtils";
include "include/client";
include "include/sounds";
include "include/say";
include "include/packets";
include "include/epicConstants";

const _NB_BTN_ESTUDAR   := 1;
const _NB_BTN_VOLTAR    := 2;
const _NB_BTN_TRANSFORM := 100;
const _NB_BTN_ESQUECER  := 200;

/**
 * Elo Natural (Natural Bond) - ABILITY_ID::ELO_NATURAL (2100)
 *
 * Active (ON_USE) — abre gump com 4 ações:
 *   1. Estudar criatura selvagem (aprende nova forma)
 *   2. Transformar em forma aprendida
 *   3. Esquecer forma específica
 *   4. Voltar ao normal (se transformado)
 *
 * Passive (via HasAbility checks):
 *   - Permite lançar magias polimorfado (cast_spell.src)
 *   - Permite manter anéis/colares durante polimorfose
 *   - Permite falar em forma animal
 */
class NaturalBond(CombatAbility)

    function NaturalBond(this)
        Super(ABILITY_ID::ELO_NATURAL);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();

        if (who.npctemplate)
            return this._HandleNPCTransformation(who);
        endif

        var forms := GetObjProperty(who, "wildshape_forms");
        if (forms == error || forms.size() == 0)
            return this._StudyCreature(mobile, who);
        endif

        return this._ShowMainGump(mobile, who, forms);
    endfunction

    // =========================================================================
    // GUMP PRINCIPAL
    // =========================================================================

    function _ShowMainGump(this, mobile, who, forms)
        var is_transformed := (GetObjProperty(who, "metamorfose") != error);
        var row_height := 26;
        var header_h := is_transformed ? 160 : 120;
        var total_h := header_h + forms.size() * row_height + 60;

        var gump := GFCreateGump();

        GFResizePic(gump, 17, 31, 9390, 359, total_h);
        GFHTMLArea(gump, 94, 69, 232, 20, "<BASEFONT Size=5 Color=#999999 >Elo Natural", 0, 0);
        GFHTMLArea(gump, 96, 71, 232, 20, "<BASEFONT Size=5 Color=#999999 >Elo Natural", 0, 0);
        GFHTMLArea(gump, 95, 70, 232, 20, "<BASEFONT Size=5 Color=#2d8a2d >Elo Natural", 0, 0);
        GFGumpPic(gump, 63, 91, 1469);
        GFGumpPic(gump, 296, 0, 9000, 0);

        var y := 100;

        if (is_transformed)
            GFHTMLArea(gump, 63, y, 240, 24, "<BASEFONT color=#cc6633>Voce esta transformado", 0, 0);
            GFAddButton(gump, 319, y + 4, 10006, 10006, GF_CLOSE_BTN, _NB_BTN_VOLTAR);
            GFTextLine(gump, 300, y - 16, 0, "Voltar");
            y += row_height + 10;
            GFGumpPic(gump, 63, y, 9773, 0);
            y += 10;
        endif

        GFTextLine(gump, 63,  y, 0, "Formas Aprendidas");
        GFTextLine(gump, 237, y, 0, "Forma");
        GFTextLine(gump, 296, y, 0, "Esquecer");
        y += 20;
        GFGumpPic(gump, 63, y, 9773, 0);
        y += 10;

        foreach form in forms
            GFHTMLArea(gump, 63, y, 170, 24, "<BASEFONT color=#2d8a2d>{}".format(form.name), 1, 0);
            GFAddButton(gump, 237, y + 4, 10006, 10006, GF_CLOSE_BTN, _NB_BTN_TRANSFORM + _form_iter);
            GFAddButton(gump, 297, y + 4, 10006, 10006, GF_CLOSE_BTN, _NB_BTN_ESQUECER  + _form_iter);
            sleepms(2);
            y += row_height;
        endforeach

        y += 10;
        GFGumpPic(gump, 63, y, 9773, 0);
        y += 10;

        GFHTMLArea(gump, 63, y, 190, 24, "<BASEFONT color=#2d8a2d>Estudar nova forma", 0, 0);
        GFAddButton(gump, 297, y + 4, 10006, 10006, GF_CLOSE_BTN, _NB_BTN_ESTUDAR);

        var input := GFSendGump(who, gump);

        if (!input[0] || input[0] == 0)
            return false;
        endif

        var btn := input[0];

        if (btn == _NB_BTN_ESTUDAR)
            return this._StudyCreature(mobile, who);
        elseif (btn == _NB_BTN_VOLTAR)
            return this._Revert(who);
        elseif (btn >= _NB_BTN_ESQUECER)
            var esq_idx := btn - _NB_BTN_ESQUECER;
            return this._ForgetForm(who, esq_idx, forms);
        elseif (btn >= _NB_BTN_TRANSFORM)
            var tr_idx := btn - _NB_BTN_TRANSFORM;
            if (tr_idx < 1 || tr_idx > forms.size())
                return false;
            endif
            return this._TransformIntoForm(mobile, who, forms[tr_idx]);
        endif

        return false;
    endfunction

    // =========================================================================
    // ESTUDAR CRIATURA
    // =========================================================================

    function _StudyCreature(this, mobile, who)
        Message("Qual criatura selvagem voce deseja estudar para aprender sua forma?").SendSys(who);
        var targ := Target(who);

        if (!this._IsValidStudyTarget(who, targ))
            return false;
        endif

        if (!targ.getTameDifficulty())
            Message("Voce nao pode estudar a forma de " + targ.name + ".").SendSys(who);
            return false;
        endif

        var skill := AP_GetSkill(who, MAGERY);

        var info := struct;
        info.+x         := who.x;
        info.+y         := who.y;
        info.+facing    := who.facing;
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

    // =========================================================================
    // TRANSFORMAR
    // =========================================================================

    function _TransformIntoForm(this, mobile, who, form)
        if (GetObjProperty(who, "metamorfose") != error)
            Message("Voce ja esta transformado. Use Voltar primeiro.").SendSys(who);
            return false;
        endif

        this._UnequipWild(who, mobile);
        this._PlayTransformEffects(who);
        this._DoTransform(who, form);

        var duration := this._CalcDuration(who, mobile);
        TS_StartTimer(who, "metamorfosedruida", duration);

        this._ApplyBoosts(who, form, duration);
        this._ApplyModifiers(who, form, duration, mobile);

        Message("Voce se transforma em " + form.name + ".").SendSys(who);
        return true;
    endfunction

    function _PlayTransformEffects(this, who)
        PlaySoundEffect(who, SFX_SPELL_HEAL);
        PerformAction(who, HUMAN_ANIM::CAST_AREA);
    endfunction

    function _DoTransform(this, who, form)
        var metamorfose := struct;
        metamorfose.+originalcolor   := who.color;
        metamorfose.+originalgraphic := who.graphic;
        metamorfose.+originalname    := who.name;
        metamorfose.+newgraphic      := form.graphic;
        SetObjProperty(who, "metamorfose", metamorfose);

        PlaySoundEffect(who, 0x0015);
        PerformAction(who, HUMAN_ANIM::CAST_AREA);
        PrintTextAbove(who, "*transformando-se*");

        who.frozen := 1;
        sleepms(500);

        PlayObjectCenteredEffect(who, 0x6D01, 10, 30);
        who.hidden := 1;
        sleepms(1200);

        who.graphic := form.graphic;
        who.color   := form.color;

        if (GetObjProperty(who, "charname") == error)
            SetObjProperty(who, "charname", who.name);
        endif

        who.squelch(-1);

        var wildspeech_duration := TS_GetTimerDuration(who, "wildspeech");
        if (wildspeech_duration != error && wildspeech_duration > 0)
            who.squelch(0);
        endif

        who.hidden := 0;
        PlayObjectCenteredEffect(who, 0xCE72, 10, 20);
        sleepms(500);
        who.frozen := 0;
    endfunction

    function _UnequipWild(this, who, mobile)
        var equips := EnumeratePhysicalItems(who);
        if (mobile.HasAbility(ABILITY_ID::ELO_NATURAL))
            foreach equip in equips
                if (equip.isa(POLCLASS_ARMOR) && equip.resist_physical > 0)
                    MoveItemToContainer(equip, who.backpack);
                elseif (equip.isa(POLCLASS_WEAPON))
                    MoveItemToContainer(equip, who.backpack);
                endif
            endforeach
        else
            foreach equip in equips
                MoveItemToContainer(equip, who.backpack);
            endforeach
        endif
    endfunction

    function _CalcDuration(this, who, mobile)
        var duration := 60 + 540 * AP_GetSkill(who, THEOLOGY) / 100;
        if (mobile.HasAbility(ABILITY_ID::CORACAO_DA_MATILHA))
            duration := duration * 2;
        endif
        return duration;
    endfunction

    function _ApplyBoosts(this, who, form, duration)
        TS_StartTimer(who, "agility",      CInt(duration), CInt(form.dexterity    / 4));
        TS_StartTimer(who, "strength",     CInt(duration), CInt(form.strength     / 4));
        TS_StartTimer(who, "intelligence", CInt(duration), CInt(form.intelligence / 4));
    endfunction

    function _ApplyModifiers(this, who, form, duration, mobile)
        var skill_a_calc   := CInt(0.1318 * form.strength + 0.2316 * form.dexterity + 0.5974 * 40);
        var resistmod_calc := this._ResolveResistTier(form);
        var hitsmod_calc   := CInt(form.strength / 3);
        var manamod_calc   := CInt(form.dexterity / 5);
        var stammod_calc   := CInt(form.intelligence / 5);
        var dicedmg_calc   := this._CalcDice(form.strength);

        var bonusMagicLore  := CInt(AP_GetSkill(who, THEOLOGY) / 10);
        var bonusInstintoHP := CInt(AP_GetSkill(who, THEOLOGY) / 3);

        if (mobile.HasAbility(ABILITY_ID::INSTINTO_DE_SOBREVIVENCIA))
            skill_a_calc   := CInt(skill_a_calc + bonusMagicLore);
            hitsmod_calc   := CInt(hitsmod_calc + bonusInstintoHP);
            resistmod_calc := resistmod_calc + CInt(AP_GetSkill(who, THEOLOGY) / 30);
        endif

        // if (form.category && Lower(form.category) == "giantanimal")
        //     // hitsmod_calc := CInt(hitsmod_calc + form.strength);
        //     Message("Voce se sente lento!").SendSys(who);
        //     TS_StartTimer(who, "lentidao", CInt(duration));
        // endif

        var hitscript := this._GetTaxonomyScript(form.taxonomy);
        if (hitscript)
            SetObjProperty(who, "HitScripts", array{hitscript});
        endif

        if (form.dexterity / form.strength >= 2)
            hitsmod_calc   := 0;
            resistmod_calc := 0;
            Message("Voce se sente mais veloz!").SendSys(who);
            TS_StartTimer(who, "velocidade", 30);
        endif

        if (form.intelligence >= 65)
            Message("Voce sente sua mente expandindo!").SendSys(who);
            manamod_calc := CInt(form.intelligence / 3);
        endif

        if (form.dexterity >= 100)
            Message("Voce se sente mais sagaz!").SendSys(who);
            stammod_calc := CInt(form.dexterity / 3);
        endif

        var moon_fase := GetGlobalProperty("moonfase");
        if (moon_fase == "Idus")
            SetObjProperty(who, "polymorphbonus_moon", bonusMagicLore);
            skill_a_calc := CInt(skill_a_calc + bonusMagicLore);
            hitsmod_calc := CInt(hitsmod_calc + bonusMagicLore);
        endif

        if (moon_fase == "Kalendae")
            Message("Voce nao consegue manter sua transformacao em lua nova! (Lua: Kalendae - duracao reduzida para 5s)").SendSys(who);
            TS_StartTimer(who, "metamorfosedruida", 5);
        endif

        if (Lower(form.name) == "cavalo manso" || Lower(form.name) == "cavalo selvagem")
            Message("Voce se transforma em um cavalo e sente-se mais rapido!").SendSys(who);
            skill_a_calc := CInt(skill_a_calc - bonusMagicLore);
            hitsmod_calc := CInt(hitsmod_calc - bonusMagicLore);
            TS_StartTimer(who, "metamorfosedruida", CInt(duration / 2));
            SendPacket(who, "BF0006002601");
        endif

        var cur_hitsmod   := GetObjProperty(who, "hitsmod");
        var cur_manamod   := GetObjProperty(who, "manamod");
        var cur_stammod   := GetObjProperty(who, "stammod");
        var cur_resistmod := GetObjProperty(who, "resistmod");

        if (cur_hitsmod == error)
            cur_hitsmod := 0;
        endif
        if (cur_manamod == error)
            cur_manamod := 0;
        endif
        if (cur_stammod == error)
            cur_stammod := 0;
        endif
        if (cur_resistmod == error)
            cur_resistmod := 0;
        endif

        SetObjProperty(who, "skill_a",             skill_a_calc);
        SetObjProperty(who, "dicedmg",             dicedmg_calc);
        SetObjProperty(who, "polymorph_hitsmod",   hitsmod_calc);
        SetObjProperty(who, "polymorph_manamod",   manamod_calc);
        SetObjProperty(who, "polymorph_stammod",   stammod_calc);
        SetObjProperty(who, "polymorph_resistmod", resistmod_calc);
        SetObjProperty(who, "hitsmod",             cur_hitsmod   + hitsmod_calc);
        SetObjProperty(who, "manamod",             cur_manamod   + manamod_calc);
        SetObjProperty(who, "stammod",             cur_stammod   + stammod_calc);
        SetObjProperty(who, "resistmod",           cur_resistmod + resistmod_calc);

        this._ApplyResistMod(who, resistmod_calc);
        recalcvitals(who);
        applyEpicHealing(who, hitsmod_calc);
    endfunction

    function _ResolveResistTier(this, form)
        var tier_number := CInt(form.resistTier);
        if (tier_number > 0)
            return tier_number;
        endif

        if (!form.template)
            return 0;
        endif

        var parsed_template := NPC_ParseTemplateName(form.template);
        var elem := NPC_GetNPCConfig(":epicai:" + parsed_template.template);

        if (!elem || elem.errortext)
            return 0;
        endif

        var fis_parts := GetConfigStringArray(elem, "EpicFisResists");
        if (!fis_parts || fis_parts.errortext || fis_parts.size() == 0)
            return 0;
        endif

        var tier_str := CStr(fis_parts[1]);
        if (tier_str.size() >= 2)
            tier_number := CInt(tier_str[2]);
        endif

        return tier_number;
    endfunction

    function _ApplyResistMod(this, who, resistmod_calc)
        // Apply to engine-level resist mods (visible on char sheet, consumed by combat)
        who.resist_physical_mod := who.resist_physical_mod + resistmod_calc;
        who.resist_fire_mod     := who.resist_fire_mod     + resistmod_calc;
        who.resist_cold_mod     := who.resist_cold_mod     + resistmod_calc;
        who.resist_poison_mod   := who.resist_poison_mod   + resistmod_calc;
        who.resist_energy_mod   := who.resist_energy_mod   + resistmod_calc;

        // Keep #SpellResist in sync (used by DamageController and status bar)
        var mw := MobileWrapper(who);
        var sr := mw.GetProperty("#SpellResist");
        if (!sr || sr.errortext)
            sr := struct{slash := 0, piercing := 0, blunt := 0, fire := 0, cold := 0, poison := 0, energy := 0};
        endif
        sr.slash    += resistmod_calc;
        sr.piercing += resistmod_calc;
        sr.blunt    += resistmod_calc;
        sr.fire     += resistmod_calc;
        sr.cold     += resistmod_calc;
        sr.poison   += resistmod_calc;
        sr.energy   += resistmod_calc;
        mw.SetProperty("#SpellResist", sr);
    endfunction

    function _CalcDice(this, strength)
        var s := CInt(strength);
        if (s < 10)
            return array{"1d2", "1d2", "1d2"};
        elseif (s <= 50)
            return array{"2d2", "2d2", "2d2"};
        elseif (s <= 100)
            return array{"3d2", "3d2", "3d2"};
        elseif (s <= 150)
            return array{"4d2", "4d2", "4d2"};
        elseif (s <= 200)
            return array{"5d2", "5d2", "5d2"};
        else
            return array{"6d2", "6d2", "6d2"};
        endif
    endfunction

    function _GetTaxonomyScript(this, taxonomy)
        case (Lower(taxonomy))
            "canidae":   return ":combat:weaponHitScripts/bleedingstrike";
            "felidae":   return ":combat:weaponHitScripts/bleedingstrike";
            "ursidae":   return ":combat:weaponHitScripts/pushback";
            "serpentes": return ":combat:weaponHitScripts/spitacid";
            "arachnida": return ":combat:weaponHitScripts/ctffrozen";
            "suidae":    return ":combat:weaponHitScripts/bleedingstrike";
            "bovidae":   return ":combat:weaponHitScripts/ConcussionHit";
            "reptilia":  return ":combat:weaponHitScripts/draintouch";
            default:     return 0;
        endcase
    endfunction

    // =========================================================================
    // VOLTAR
    // =========================================================================

    function _Revert(this, who)
        if (GetObjProperty(who, "metamorfose") == error)
            Message("Voce nao esta metamorfoseado.").SendSys(who);
            return false;
        endif

        if (TS_GetCooldown(who, "voltarcd") > 0)
            Message("Voce deve aguardar para voltar ao normal.").SendSys(who);
            return false;
        endif

        PrintTextAbove(who, "*revertendo a forma*");
        sleepms(400);
        PerformAction(who, HUMAN_ANIM::CAST_AREA);

        PlayObjectCenteredEffect(who, 0x3728, 10, 20);
        PlaySoundEffect(who, 0x0015);

        TS_StartCoolDown(who, "voltarcd", 30);
        Message("Voce volta ao normal.").SendSys(who);

        var fist := who.weapon;
        if (fist && fist != error)
            EraseObjProperty(fist, "#elementalweapon");
            EraseObjProperty(fist, "#ataqueelemental");
        endif

        TS_LowerDuration(who, "metamorfosedruida", -1);
        return true;
    endfunction

    // =========================================================================
    // ESQUECER FORMA
    // =========================================================================

    function _ForgetForm(this, who, index, forms)
        if (index < 1 || index > forms.size())
            return false;
        endif

        var forgotten := forms[index];
        forms.erase(index);
        SetObjProperty(who, "wildshape_forms", forms);
        Message("Voce esquece a forma de " + forgotten.name + ".").SendSys(who);
        return true;
    endfunction

    function _ShowForgetGump(this, who, forms)
        var size := forms.size() * 26 + 90;
        var gump := GFCreateGump();

        GFResizePic(gump, 17, 31, 9390, 359, size);
        GFHTMLArea(gump, 94, 69, 232, 20, "<BASEFONT Size=5 Color=#999999 >Limite de Formas", 0, 0);
        GFHTMLArea(gump, 95, 70, 232, 20, "<BASEFONT Size=5 Color=#cc6633 >Limite de Formas", 0, 0);
        GFGumpPic(gump, 63, 91, 1469);
        GFHTMLArea(gump, 63, 95, 280, 24, "<BASEFONT color=#cccccc>Escolha qual forma esquecer:", 0, 0);

        var y := 120;
        foreach form in forms
            GFHTMLArea(gump, 63, y, 232, 24, "<BASEFONT color=#2d8a2d>{}".format(form.name), 1, 0);
            GFAddButton(gump, 319, y + 4, 10006, 10006, GF_CLOSE_BTN, _form_iter);
            sleepms(2);
            y += 26;
        endforeach

        var input := GFSendGump(who, gump);

        if (!input[0] || input[0] == 0)
            return 0;
        endif

        return input[0];
    endfunction

    // =========================================================================
    // ARMAZENAR TRANSFORMAÇÃO
    // =========================================================================

    function _StoreTransformation(this, mobile, who, targ)
        var max_transformations := CInt(AP_GetSkill(who, MAGERY) / 20);
        if (mobile.HasAbility(ABILITY_ID::CORACAO_DA_MATILHA))
            max_transformations := max_transformations * 2;
        endif

        var targ_strength     := AP_GetStat(targ, "Strength");
        var targ_dexterity    := AP_GetStat(targ, "Dexterity");
        var targ_intelligence := AP_GetStat(targ, "Intelligence");

        var parsed_template := NPC_ParseTemplateName(targ.npctemplate);
        var elem            := NPC_GetNPCConfig(":epicai:" + parsed_template.template);
        var targ_category   := GetConfigString(elem, "Category");
        var targ_taxonomy   := elem.Taxonomy;

        Message($"Forca: {targ_strength}").SendSys(who);
        Message($"Destreza: {targ_dexterity}").SendSys(who);
        Message($"Inteligencia: {targ_intelligence}").SendSys(who);

        var tier_number := 0;
        var fis_str := GetConfigString(elem, "EpicFisResists");
        if (fis_str && !fis_str.errortext)
            var fis_parts := SplitWords(fis_str);
            var tier_str  := CStr(fis_parts[1]);
            if (tier_str.size() >= 2)
                tier_number := CInt(tier_str[2]);
            endif
        endif

        var transformation := struct;
        transformation.+name         := targ.name;
        transformation.+graphic      := targ.graphic;
        transformation.+color        := targ.color;
        transformation.+gender       := targ.gender;
        transformation.+strength     := targ_strength;
        transformation.+dexterity    := targ_dexterity;
        transformation.+intelligence := targ_intelligence;
        transformation.+category     := targ_category;
        transformation.+taxonomy     := targ_taxonomy;
        transformation.+resistTier   := tier_number;

        var transformation_list := GetObjProperty(who, "wildshape_forms");
        if (transformation_list == error)
            transformation_list := array;
        endif

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

        if (transformation_list.size() >= max_transformations)
            Message("Voce ja conhece o maximo de formas. Escolha qual esquecer para aprender " + targ.name + ".").SendSys(who);
            var forget_idx := this._ShowForgetGump(who, transformation_list);
            if (!forget_idx)
                Message("Aprendizado cancelado.").SendSys(who);
                return;
            endif
            var old_name := transformation_list[forget_idx].name;
            transformation_list.erase(forget_idx);
            Message("Voce esqueceu a forma de " + old_name + ".").SendSys(who);
        endif

        transformation_list.append(transformation);
        SetObjProperty(who, "wildshape_forms", transformation_list);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[ELO_NATURAL] Aprendeu forma: {targ.name}", Color(SYSTEM_COLOR::SUCCESS))
        );
    endfunction

    // =========================================================================
    // VALIDAÇÃO DE ESTUDO
    // =========================================================================

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
        var my_hostiles   := ListHostiles(who, 15);
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

    // =========================================================================
    // TRANSFORMAÇÃO NPC (mantido)
    // =========================================================================

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
                    "druid"  := who.serial,
                    "x"      := animal.x,
                    "y"      := animal.y,
                    "z"      := animal.z,
                    "realm"  := animal.realm
                };

                Start_Script(":abilities:scripts/revertWildshape", params);
                return true;
            endif
        endif

        return false;
    endfunction

endclass
