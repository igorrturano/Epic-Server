use uo;
use os;
use cfgfile;

include ":gumps:gumps_ex";
include ":gumps:epicGumps";
include "include/say";
include ":spells:SpellRegistry";
include ":epicitem:properties";

// Button IDs
const BTN_CANCEL       := 0;
const BTN_CONFIRM      := 1;
const BTN_TAB_TYPE     := 10;
const BTN_TAB_MODS     := 11;
const BTN_TAB_EFFECT   := 12;
const BTN_TAB_CONFIRM  := 13;
const BTN_EFFECT_PREV  := 20;
const BTN_EFFECT_NEXT  := 21;
const BTN_CLEAR_EFFECT := 22;

// Radio IDs - Category
const RADIO_SACRED  := 100;
const RADIO_CURSED  := 101;
const RADIO_MAGIC   := 102;

// Text Entry IDs
const TXT_ENTROPY    := 200;
const TXT_STRMOD     := 201;
const TXT_DEXMOD     := 202;
const TXT_INTMOD     := 203;
const TXT_HITSMOD    := 204;
const TXT_MANAMOD    := 205;
const TXT_STAMMOD    := 206;
const TXT_COLDRES    := 207;
const TXT_FIRERES    := 208;
const TXT_PHYSRES    := 209;
const TXT_ENERGYRES  := 210;
const TXT_POISONRES  := 211;
const TXT_CHARGES    := 212;
const TXT_COOLDOWN   := 213;
const TXT_DURATION   := 214;
const CHK_DESTROY_ON_USE := 215;

// Effect list buttons start at 300
const BTN_EFFECT_BASE := 300;

// Gump dimensions
const GUMP_W := 620;
const GUMP_H := 480;

// Colors
const CLR_TITLE   := 53;
const CLR_LABEL   := 2100;
const CLR_VALUE   := 2100;
const CLR_SECTION := 1153;
const CLR_TAB_ON  := 53;
const CLR_TAB_OFF := 2100;
const CLR_WARN    := 33;

function CreateEpicItemState()
	var state := struct{};
	state.+category := "none";
	state.+entropy_threshold := 0;

	state.+strmod := 0;
	state.+dexmod := 0;
	state.+intmod := 0;
	state.+hitsmod := 0;
	state.+manamod := 0;
	state.+stammod := 0;
	state.+ColdResistMod := 0;
	state.+FireResistMod := 0;
	state.+PhysicalResistMod := 0;
	state.+EnergyResistMod := 0;
	state.+PoisonResistMod := 0;

	state.+effect_type := "none";
	state.+effect_id := 0;
	state.+effect_name := "";
	state.+effect_script := "";
	state.+effect_target := "Self";
	state.+charges := 0;
	state.+cooldown := 0;
	state.+duration := 60;
	state.+destroy_on_use := 0;

	state.+current_page := 1;
	state.+effect_list_page := 0;

	return state;
endfunction

function LoadExistingProperties(item)
	var state := CreateEpicItemState();

	var equip := GetObjProperty(item, "equip");
	if (equip && TypeOfInt(equip) == OT_DICTIONARY)
		if (equip.Exists("sacred"))
			state.category := "sacred";
		elseif (equip.Exists("cursed"))
			state.category := "cursed";
		endif

		if (equip.Exists("strmod"))
			state.strmod := CInt(equip["strmod"]);
		endif
		if (equip.Exists("dexmod"))
			state.dexmod := CInt(equip["dexmod"]);
		endif
		if (equip.Exists("intmod"))
			state.intmod := CInt(equip["intmod"]);
		endif
		if (equip.Exists("hitsmod"))
			state.hitsmod := CInt(equip["hitsmod"]);
		endif
		if (equip.Exists("manamod"))
			state.manamod := CInt(equip["manamod"]);
		endif
		if (equip.Exists("stammod"))
			state.stammod := CInt(equip["stammod"]);
		endif
		if (equip.Exists("ColdResistMod"))
			state.ColdResistMod := CInt(equip["ColdResistMod"]);
		endif
		if (equip.Exists("FireResistMod"))
			state.FireResistMod := CInt(equip["FireResistMod"]);
		endif
		if (equip.Exists("PhysicalResistMod"))
			state.PhysicalResistMod := CInt(equip["PhysicalResistMod"]);
		endif
		if (equip.Exists("EnergyResistMod"))
			state.EnergyResistMod := CInt(equip["EnergyResistMod"]);
		endif
		if (equip.Exists("PoisonResistMod"))
			state.PoisonResistMod := CInt(equip["PoisonResistMod"]);
		endif
	endif

	if (GetObjProperty(item, "magic") == 1 || GetObjProperty(item, "magic") == 2)
		if (state.category == "none")
			state.category := "magic";
		endif
	endif

	var entropy := GetObjProperty(item, "epicEntropy");
	if (entropy != error)
		state.entropy_threshold := CInt(entropy);
	endif

	var effect := GetObjProperty(item, "epiceffect");
	if (effect)
		state.effect_type := effect.effect_type;
		state.effect_id := effect.effect_id;
		state.effect_name := effect.effect_name;
		state.effect_script := effect.effect_script;
		state.effect_target := effect.target;
		state.charges := CInt(effect.charges_left);
		state.cooldown := CInt(effect.cooldown);
		if (effect.duration != error)
			state.duration := CInt(effect.duration);
		endif
		if (effect.destroy_on_use != error)
			state.destroy_on_use := CInt(effect.destroy_on_use);
		endif
	endif

	return state;
endfunction

function EpicItemEditor(who, item)
	var state := LoadExistingProperties(item);
	var spell_list := LoadSpellList();
	var ts_list := LoadTimedScriptList();

	while (who.connected)
		var gump := BuildEpicItemGump(item, state, spell_list, ts_list);
		var input := GFSendGump(who, gump);

		if (!input || input[0] == BTN_CANCEL)
			SendSysMessageEx(who, "Edicao cancelada.", SSM_FAIL);
			return 0;
		endif

		if (input[0] == BTN_CONFIRM)
			state := ExtractStateFromInput(input, state);
			ApplyEpicProperties(item, state);
			SendSysMessageEx(who, "Propriedades aplicadas com sucesso!", SSM_INFO);
			return 1;
		endif

		// Tab navigation
		if (input[0] == BTN_TAB_TYPE)
			state := ExtractStateFromInput(input, state);
			state.current_page := 1;
		elseif (input[0] == BTN_TAB_MODS)
			state := ExtractStateFromInput(input, state);
			state.current_page := 2;
		elseif (input[0] == BTN_TAB_EFFECT)
			state := ExtractStateFromInput(input, state);
			state.current_page := 3;
		elseif (input[0] == BTN_TAB_CONFIRM)
			state := ExtractStateFromInput(input, state);
			state.current_page := 4;
		endif

		// Effect list pagination
		if (input[0] == BTN_EFFECT_NEXT)
			state := ExtractStateFromInput(input, state);
			state.effect_list_page := state.effect_list_page + 1;
		elseif (input[0] == BTN_EFFECT_PREV)
			state := ExtractStateFromInput(input, state);
			if (state.effect_list_page > 0)
				state.effect_list_page := state.effect_list_page - 1;
			endif
		elseif (input[0] == BTN_CLEAR_EFFECT)
			state := ExtractStateFromInput(input, state);
			state.effect_type := "none";
			state.effect_id := 0;
			state.effect_name := "";
			state.effect_script := "";
			state.effect_target := "Self";
		endif

		// Effect selection (spell or timedscript)
		if (input[0] >= BTN_EFFECT_BASE && input[0] < BTN_EFFECT_BASE + 200)
			state := ExtractStateFromInput(input, state);
			var idx := input[0] - BTN_EFFECT_BASE;
			var combined := array{};
			foreach spell in spell_list
				combined.Append(struct{ type := "spell", data := spell });
			endforeach
			foreach ts in ts_list
				combined.Append(struct{ type := "timedscript", data := ts });
			endforeach

			if (idx > 0 && idx <= combined.Size())
				var selected := combined[idx];
				if (selected.type == "spell")
					state.effect_type := "spell";
					state.effect_id := selected.data.id;
					state.effect_name := selected.data.name;
					state.effect_script := selected.data.script;
					state.effect_target := selected.data.target;
				else
					state.effect_type := "timedscript";
					state.effect_id := selected.data.name;
					state.effect_name := selected.data.name;
					state.effect_script := "";
					state.effect_target := "Self";
				endif
			endif
		endif

		sleepms(2);
	endwhile

	return 0;
endfunction

function ExtractStateFromInput(input, state)
	var page := state.current_page;

	// Page 1: Category radios + entropy
	if (page == 1)
		if (input[RADIO_SACRED])
			state.category := "sacred";
		elseif (input[RADIO_CURSED])
			state.category := "cursed";
		elseif (input[RADIO_MAGIC])
			state.category := "magic";
		endif

		var val := Trim(GFExtractData(input, TXT_ENTROPY));
		if (val != error && val != "")
			state.entropy_threshold := CInt(val);
		endif
	endif

	// Page 2: Stat and resist mods
	if (page == 2)
		state.strmod := ExtractIntField(input, TXT_STRMOD);
		state.dexmod := ExtractIntField(input, TXT_DEXMOD);
		state.intmod := ExtractIntField(input, TXT_INTMOD);
		state.hitsmod := ExtractIntField(input, TXT_HITSMOD);
		state.manamod := ExtractIntField(input, TXT_MANAMOD);
		state.stammod := ExtractIntField(input, TXT_STAMMOD);
		state.ColdResistMod := ExtractIntField(input, TXT_COLDRES);
		state.FireResistMod := ExtractIntField(input, TXT_FIRERES);
		state.PhysicalResistMod := ExtractIntField(input, TXT_PHYSRES);
		state.EnergyResistMod := ExtractIntField(input, TXT_ENERGYRES);
		state.PoisonResistMod := ExtractIntField(input, TXT_POISONRES);
	endif

	// Page 3: Effect charges/cooldown/duration
	if (page == 3)
		state.charges := ExtractIntField(input, TXT_CHARGES);
		state.cooldown := ExtractIntField(input, TXT_COOLDOWN);
		state.duration := ExtractIntField(input, TXT_DURATION);
		if (input[CHK_DESTROY_ON_USE])
			state.destroy_on_use := 1;
		else
			state.destroy_on_use := 0;
		endif
	endif

	return state;
endfunction

function ExtractIntField(input, field_id)
	var val := Trim(GFExtractData(input, field_id));
	if (val != error && val != "")
		return CInt(val);
	endif
	return 0;
endfunction

function BuildEpicItemGump(item, state, spell_list, ts_list)
	var gump := GFECreateGump("Epic Item Editor", GUMP_W, GUMP_H);
	var page := state.current_page;

	// Tab bar (below the title)
	var tab_y := 72;
	BuildTabBar(gump, page, tab_y);

	// Content area
	var content_y := tab_y + 45;

	case (page)
		1: BuildPageType(gump, item, state, content_y);
		2: BuildPageMods(gump, state, content_y);
		3: BuildPageEffect(gump, state, spell_list, ts_list, content_y);
		4: BuildPageConfirm(gump, item, state, content_y);
	endcase

	return gump;
endfunction

function BuildTabBar(byref gump, current_page, y)
	var tabs := array{
		struct{ label := "Tipo", btn := BTN_TAB_TYPE, page := 1 },
		struct{ label := "Modificadores", btn := BTN_TAB_MODS, page := 2 },
		struct{ label := "Efeito", btn := BTN_TAB_EFFECT, page := 3 },
		struct{ label := "Confirmar", btn := BTN_TAB_CONFIRM, page := 4 }
	};

	var x := 30;
	foreach tab in tabs
		var color := CLR_TAB_OFF;
		if (tab.page == current_page)
			color := CLR_TAB_ON;
			GFResizePic(gump, x - 5, y, 9350, 130, 30);
		endif
		GFAddButton(gump, x, y + 5, 2117, 2118, GF_CLOSE_BTN, tab.btn);
		GFTextLine(gump, x + 18, y + 5, color, tab.label);
		x += 140;
	endforeach

	// Divider
	GFResizePic(gump, 25, y + 35, 3000, GUMP_W - 60, 2);
endfunction

// ============================================================
// PAGE 1: Type & Identification
// ============================================================
function BuildPageType(byref gump, item, state, y)
	var x := 40;

	// Item info
	GFTextMid(gump, 0, y, GUMP_W, CLR_SECTION, "INFORMACOES DO ITEM");
	y += 25;

	GFTextLine(gump, x, y, CLR_LABEL, "Nome: " + item.desc);
	y += 20;
	GFTextLine(gump, x, y, CLR_LABEL, "ObjType: " + Hex(item.objtype));
	GFTextLine(gump, x + 200, y, CLR_LABEL, "Graphic: " + Hex(item.graphic));
	y += 30;

	// Category selection
	GFResizePic(gump, 25, y, 3000, GUMP_W - 60, 2);
	y += 10;
	GFTextMid(gump, 0, y, GUMP_W, CLR_SECTION, "CATEGORIA DO ITEM");
	y += 25;

	GFSetRadioGroup(gump, 1);
	GFRadioButton(gump, x, y, 9722, 9723, (state.category == "sacred"), RADIO_SACRED);
	GFTextLine(gump, x + 35, y, CLR_LABEL, "Sagrado (Sacred)");
	y += 25;

	GFRadioButton(gump, x, y, 9722, 9723, (state.category == "cursed"), RADIO_CURSED);
	GFTextLine(gump, x + 35, y, CLR_LABEL, "Amaldicoado (Cursed)");
	y += 25;

	GFRadioButton(gump, x, y, 9722, 9723, (state.category == "magic"), RADIO_MAGIC);
	GFTextLine(gump, x + 35, y, CLR_LABEL, "Magico (Magic)");
	y += 35;

	// Entropy threshold
	GFResizePic(gump, 25, y, 3000, GUMP_W - 60, 2);
	y += 10;
	GFTextMid(gump, 0, y, GUMP_W, CLR_SECTION, "ENTROPIA (THRESHOLD PARA EQUIPAR)");
	y += 25;

	GFTextLine(gump, x, y, CLR_LABEL, "Valor (positivo p/ Sacred, negativo p/ Cursed):");
	GFEAddTextBox(gump, x + 350, y - 3, 80, 16, CLR_VALUE, CStr(state.entropy_threshold), TXT_ENTROPY);
	y += 20;
	GFTextLine(gump, x, y+10, CLR_WARN, "Sacred: jogador precisa ter entropia >= valor. Cursed: entropia <= valor.");
endfunction

// ============================================================
// PAGE 2: Equip Modifiers
// ============================================================
function BuildPageMods(byref gump, state, y)
	var x := 40;
	var col2 := 320;

	GFTextMid(gump, 0, y, GUMP_W, CLR_SECTION, "MODIFICADORES DE ATRIBUTOS");
	y += 30;

	// Stats
	GFTextLine(gump, x, y, CLR_LABEL, "Forca (STR):");
	GFEAddTextBox(gump, x + 130, y - 3, 60, 16, CLR_VALUE, CStr(state.strmod), TXT_STRMOD);

	GFTextLine(gump, col2, y, CLR_LABEL, "Vida (Hits):");
	GFEAddTextBox(gump, col2 + 130, y - 3, 60, 16, CLR_VALUE, CStr(state.hitsmod), TXT_HITSMOD);
	y += 35;

	GFTextLine(gump, x, y, CLR_LABEL, "Destreza (DEX):");
	GFEAddTextBox(gump, x + 130, y - 3, 60, 16, CLR_VALUE, CStr(state.dexmod), TXT_DEXMOD);

	GFTextLine(gump, col2, y, CLR_LABEL, "Mana:");
	GFEAddTextBox(gump, col2 + 130, y - 3, 60, 16, CLR_VALUE, CStr(state.manamod), TXT_MANAMOD);
	y += 35;

	GFTextLine(gump, x, y, CLR_LABEL, "Inteligencia (INT):");
	GFEAddTextBox(gump, x + 130, y - 3, 60, 16, CLR_VALUE, CStr(state.intmod), TXT_INTMOD);

	GFTextLine(gump, col2, y, CLR_LABEL, "Stamina:");
	GFEAddTextBox(gump, col2 + 130, y - 3, 60, 16, CLR_VALUE, CStr(state.stammod), TXT_STAMMOD);
	y += 45;

	// Resists
	GFResizePic(gump, 25, y, 3000, GUMP_W - 60, 2);
	y += 10;
	GFTextMid(gump, 0, y, GUMP_W, CLR_SECTION, "MODIFICADORES DE RESISTENCIA");
	y += 30;

	GFTextLine(gump, x, y, CLR_LABEL, "Fisico:");
	GFEAddTextBox(gump, x + 80, y - 3, 60, 16, CLR_VALUE, CStr(state.PhysicalResistMod), TXT_PHYSRES);

	GFTextLine(gump, col2, y, CLR_LABEL, "Fogo:");
	GFEAddTextBox(gump, col2 + 80, y - 3, 60, 16, CLR_VALUE, CStr(state.FireResistMod), TXT_FIRERES);
	y += 35;

	GFTextLine(gump, x, y, CLR_LABEL, "Frio:");
	GFEAddTextBox(gump, x + 80, y - 3, 60, 16, CLR_VALUE, CStr(state.ColdResistMod), TXT_COLDRES);

	GFTextLine(gump, col2, y, CLR_LABEL, "Energia:");
	GFEAddTextBox(gump, col2 + 80, y - 3, 60, 16, CLR_VALUE, CStr(state.EnergyResistMod), TXT_ENERGYRES);
	y += 35;

	GFTextLine(gump, x, y, CLR_LABEL, "Veneno:");
	GFEAddTextBox(gump, x + 80, y - 3, 60, 16, CLR_VALUE, CStr(state.PoisonResistMod), TXT_POISONRES);
endfunction

// ============================================================
// PAGE 3: Double-Click Effect
// ============================================================
function BuildPageEffect(byref gump, state, spell_list, ts_list, y)
	var x := 40;

	// Current effect info
	GFTextMid(gump, 0, y, GUMP_W, CLR_SECTION, "EFEITO NO DUPLO-CLIQUE");
	y += 25;

	if (state.effect_type != "none")
		var type_label := "Nenhum";
		if (state.effect_type == "spell")
			type_label := "Magia";
		elseif (state.effect_type == "timedscript")
			type_label := "TimedScript";
		endif
		GFTextLine(gump, x, y, CLR_LABEL, "Selecionado: [" + type_label + "] " + state.effect_name);
		GFTextLine(gump, x + 420, y, CLR_LABEL, "Alvo: " + state.effect_target);
		y += 22;
		GFAddButton(gump, x, y, 2117, 2118, GF_CLOSE_BTN, BTN_CLEAR_EFFECT);
		GFTextLine(gump, x + 18, y, CLR_WARN, "Limpar efeito");
		y += 25;
	else
		GFTextLine(gump, x, y, CLR_WARN, "Nenhum efeito selecionado. Escolha da lista abaixo.");
		y += 25;
	endif

	// Charges, cooldown, duration
	GFTextLine(gump, x, y, CLR_LABEL, "Cargas:");
	GFEAddTextBox(gump, x + 70, y - 3, 50, 16, CLR_VALUE, CStr(state.charges), TXT_CHARGES);

	GFTextLine(gump, x + 160, y, CLR_LABEL, "Cooldown (seg):");
	GFEAddTextBox(gump, x + 290, y - 3, 50, 16, CLR_VALUE, CStr(state.cooldown), TXT_COOLDOWN);

	GFTextLine(gump, x + 380, y, CLR_LABEL, "Duracao (seg):");
	GFEAddTextBox(gump, x + 500, y - 3, 50, 16, CLR_VALUE, CStr(state.duration), TXT_DURATION);
	y += 30;

	GFCheckBox(gump, x, y, 2361, 2362, state.destroy_on_use, CHK_DESTROY_ON_USE);
	GFTextLine(gump, x + 30, y + 3, CLR_LABEL, "Destruir o item apos o uso");
	y += 30;

	// Divider
	GFResizePic(gump, 25, y, 3000, GUMP_W - 60, 2);
	y += 10;

	// Build combined list
	var combined := array{};
	foreach spell in spell_list
		combined.Append(struct{ type := "spell", label := "[Magia] " + spell.school + " - " + spell.name, target := spell.target });
	endforeach
	foreach ts in ts_list
		combined.Append(struct{ type := "ts", label := "[Timer] " + ts.name, target := "Self" });
	endforeach

	// Pagination
	var items_per_page := 10;
	var total_pages := CInt((combined.Size() + items_per_page - 1) / items_per_page);
	if (total_pages < 1)
		total_pages := 1;
	endif
	if (state.effect_list_page >= total_pages)
		state.effect_list_page := total_pages - 1;
	endif

	var start_idx := state.effect_list_page * items_per_page + 1;
	var end_idx := start_idx + items_per_page - 1;
	if (end_idx > combined.Size())
		end_idx := combined.Size();
	endif

	GFTextLine(gump, x, y, CLR_SECTION, "LISTA DE EFEITOS (pag " + (state.effect_list_page + 1) + "/" + total_pages + ")");

	// Prev/Next buttons
	if (state.effect_list_page > 0)
		GFAddButton(gump, GUMP_W - 140, y, 2650, 2651, GF_CLOSE_BTN, BTN_EFFECT_PREV);
		GFTextLine(gump, GUMP_W - 120, y, CLR_LABEL, "Anterior");
	endif
	if (state.effect_list_page < total_pages - 1)
		GFAddButton(gump, GUMP_W - 70, y, 2648, 2649, GF_CLOSE_BTN, BTN_EFFECT_NEXT);
		GFTextLine(gump, GUMP_W - 50, y, CLR_LABEL, "Proximo");
	endif
	y += 25;

	var idx := start_idx;
	while (idx <= end_idx)
		var entry := combined[idx];
		var btn_id := BTN_EFFECT_BASE + idx;
		GFAddButton(gump, x, y + 2, 2117, 2118, GF_CLOSE_BTN, btn_id);
		GFTextLine(gump, x + 18, y, CLR_LABEL, entry.label);
		GFTextLine(gump, GUMP_W - 120, y, CLR_VALUE, "(" + entry.target + ")");
		y += 20;
		idx += 1;
	endwhile
endfunction

// ============================================================
// PAGE 4: Confirmation
// ============================================================
function BuildPageConfirm(byref gump, item, state, y)
	var x := 40;

	GFTextMid(gump, 0, y, GUMP_W, CLR_SECTION, "RESUMO - CONFIRMAR ALTERACOES");
	y += 30;

	GFTextLine(gump, x, y, CLR_LABEL, "Item: " + item.desc + " (" + Hex(item.objtype) + ")");
	y += 25;

	// Category
	var cat_label := "Nenhuma";
	if (state.category == "sacred")
		cat_label := "Sagrado";
	elseif (state.category == "cursed")
		cat_label := "Amaldicoado";
	elseif (state.category == "magic")
		cat_label := "Magico";
	endif
	GFTextLine(gump, x, y, CLR_LABEL, "Categoria: " + cat_label);
	y += 22;

	if (state.entropy_threshold != 0)
		GFTextLine(gump, x, y, CLR_LABEL, "Entropia necessaria: " + CStr(state.entropy_threshold));
		y += 22;
	endif

	// Mods summary
	y += 5;
	GFResizePic(gump, 25, y, 3000, GUMP_W - 60, 2);
	y += 10;
	GFTextLine(gump, x, y, CLR_SECTION, "Modificadores:");
	y += 22;

	var has_mods := 0;
	if (state.strmod)
		GFTextLine(gump, x, y, CLR_LABEL, "STR: " + CStr(state.strmod));
		has_mods := 1;
	endif
	if (state.dexmod)
		GFTextLine(gump, x + 100, y, CLR_LABEL, "DEX: " + CStr(state.dexmod));
		has_mods := 1;
	endif
	if (state.intmod)
		GFTextLine(gump, x + 200, y, CLR_LABEL, "INT: " + CStr(state.intmod));
		has_mods := 1;
	endif
	if (state.hitsmod)
		GFTextLine(gump, x + 300, y, CLR_LABEL, "HP: " + CStr(state.hitsmod));
		has_mods := 1;
	endif
	if (state.manamod)
		GFTextLine(gump, x + 400, y, CLR_LABEL, "Mana: " + CStr(state.manamod));
		has_mods := 1;
	endif
	if (has_mods)
		y += 22;
	endif

	has_mods := 0;
	if (state.stammod)
		GFTextLine(gump, x, y, CLR_LABEL, "Stam: " + CStr(state.stammod));
		has_mods := 1;
	endif
	if (state.PhysicalResistMod)
		GFTextLine(gump, x + 100, y, CLR_LABEL, "Fisico: " + CStr(state.PhysicalResistMod));
		has_mods := 1;
	endif
	if (state.FireResistMod)
		GFTextLine(gump, x + 210, y, CLR_LABEL, "Fogo: " + CStr(state.FireResistMod));
		has_mods := 1;
	endif
	if (state.ColdResistMod)
		GFTextLine(gump, x + 310, y, CLR_LABEL, "Frio: " + CStr(state.ColdResistMod));
		has_mods := 1;
	endif
	if (state.EnergyResistMod)
		GFTextLine(gump, x + 400, y, CLR_LABEL, "Energ: " + CStr(state.EnergyResistMod));
		has_mods := 1;
	endif
	if (has_mods)
		y += 22;
	endif
	if (state.PoisonResistMod)
		GFTextLine(gump, x, y, CLR_LABEL, "Veneno: " + CStr(state.PoisonResistMod));
		y += 22;
	endif

	// Effect summary
	y += 5;
	GFResizePic(gump, 25, y, 3000, GUMP_W - 60, 2);
	y += 10;
	GFTextLine(gump, x, y, CLR_SECTION, "Efeito no Uso:");
	y += 22;

	if (state.effect_type != "none")
		GFTextLine(gump, x, y, CLR_LABEL, "Efeito: " + state.effect_name);
		y += 22;
		GFTextLine(gump, x, y, CLR_LABEL, "Tipo: " + state.effect_type + "  |  Alvo: " + state.effect_target);
		y += 22;
		GFTextLine(gump, x, y, CLR_LABEL, "Cargas: " + CStr(state.charges) + "  |  Cooldown: " + CStr(state.cooldown) + "s  |  Duracao: " + CStr(state.duration) + "s");
		y += 22;
		if (state.destroy_on_use)
			GFTextLine(gump, x, y, CLR_WARN, "Destruir apos o uso: SIM");
			y += 22;
		endif
	else
		GFTextLine(gump, x, y, CLR_LABEL, "Nenhum efeito configurado.");
		y += 22;
	endif

	// Confirm button
	y += 15;
	GFAddButton(gump, GUMP_W / 2 - 60, y, 2128, 2129, GF_CLOSE_BTN, BTN_CONFIRM);
endfunction

// ============================================================
// Apply properties to item
// ============================================================
// ApplyEpicProperties / SetOrErase live in :epicitem:properties so the
// loot generator can build epic items without dragging in the GM gump.
// See pkg/items/epicitem/include/properties.inc.

// ============================================================
// Load spell list from all config files
// ============================================================
function LoadSpellList()
	var spells := array{};
	var configs := array{
		struct{ path := ":spells:config/cantrips", school := "Cantrips" },
		struct{ path := ":spells:config/evocation", school := "Evocacao" },
		struct{ path := ":spells:config/necromancy", school := "Necromancia" },
		struct{ path := ":spells:config/illusion", school := "Ilusao" },
		struct{ path := ":spells:config/swordmage", school := "Swordmage" },
		struct{ path := ":spells:config/priest", school := "Sacerdote" },
		struct{ path := ":spells:config/shaman", school := "Shaman" },
		struct{ path := ":spells:config/druid", school := "Druida" },
		struct{ path := ":spells:config/paladin", school := "Paladino" },
		struct{ path := ":spells:config/demonhunter", school := "DemonHunter" },
		struct{ path := ":spells:config/trickster", school := "Trickster" },
		struct{ path := ":spells:config/shinobi", school := "Shinobi" },
		struct{ path := ":spells:config/bardo", school := "Bardo" }
	};

	foreach cfg_info in configs
		var cfg := ReadConfigFile(cfg_info.path);
		if (!cfg)
			continue;
		endif

		foreach spell_id in (GetConfigIntKeys(cfg))
			var elem := FindConfigElem(cfg, spell_id);
			if (!elem)
				continue;
			endif

			var spell_script := GetConfigString(elem, "SpellScript");
			if (!spell_script || spell_script == "")
				continue;
			endif

			var entry := struct{};
			entry.+id := spell_id;
			entry.+name := GetConfigString(elem, "Name");
			entry.+school := cfg_info.school;
			entry.+target := GetConfigString(elem, "Target");
			entry.+script := spell_script;

			if (!entry.target || entry.target == "")
				entry.target := "Self";
			endif

			spells.Append(entry);
		endforeach
		sleepms(2);
	endforeach

	return spells;
endfunction

// ============================================================
// Load timed scripts list
// ============================================================
function LoadTimedScriptList()
	var ts_list := array{};
	var cfg_paths := array{
		":timedscripts:config/timedScripts",
		":musicianship:config/timedScripts"
	};

	foreach cfg_path in cfg_paths
		var cfg := ReadConfigFile(cfg_path);
		if (!cfg)
			continue;
		endif

		foreach key in (GetConfigStringKeys(cfg))
			var elem := FindConfigElem(cfg, key);
			var entry := struct{};
			entry.+name := CStr(key);
			entry.+type := GetConfigString(elem, "Type");
			if (!entry.type)
				entry.type := "";
			endif
			ts_list.Append(entry);
		endforeach
		sleepms(2);
	endforeach

	return ts_list;
endfunction
