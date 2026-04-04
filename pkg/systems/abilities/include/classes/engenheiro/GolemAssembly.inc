use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":wrappers:Objects/Model/MobileWrapper";
include "include/sounds";
include "include/facings";
include ":gumps:yesNo";

const GADGET_HANDCANNON := 0xFFC7;
const GADGET_HARPOON := 0xFFC6;
const GADGET_POWDERTRAP := 0xFFC8;

const STEAM_GLOVES := 0xFFC1;
const STEAM_LEGS   := 0xFFC2;
const STEAM_ARMS   := 0xFFC3;
const STEAM_HELM   := 0xFFC4;
const STEAM_CHEST  := 0xFFC5;
const STEAM_GORGET := 0xFFC9;

class GolemAssembly(CombatAbility)

	function GolemAssembly(this)
		Super(ABILITY_ID::GOLEM_ASSEMBLY);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	// Passive: When piloting a golem, add blunt damage based on METALWORKING
	function OnPassiveBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();

		if (!attacker.GetProperty("pilotando"))
			return false;
		endif

		var metalworking := CInt(attacker.GetSkill(METALWORKING));
		var damageBonus := CInt(metalworking / 5);

		if (damageBonus <= 0)
			return false;
		endif

		attackInfo.GetDamage()
			.IncreaseBluntRawDamage(damageBonus);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[GOLEM_ASSEMBLY] Dano do Golem (Blunt): +{damageBonus}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		if (mobile.GetProperty("pilotando"))
			return true;
		endif
		if (mobile.GetProperty("golem_maker"))
			Message("Ter mais de um golem para uso pessoal eh um desperdicio de recursos!").SendSys(who);
			return false;
		endif
		if (who.weapon.objtype != 0x13e3)
			Message("Use um martelo de ferreiro.").SendSys(who);
			return false;
		endif
		var anvil;
		foreach item in ListItemsNearLocation(who.x, who.y, who.z, 1, who.realm)
			if (item.objtype == 0xA102 || item.objtype == 0xA108)
				anvil := item;
				break;
			endif
		endforeach
		if (!anvil)
			Message("Voce precisa estar perto de uma bigorna para criar seu golem.").SendSys(who);
			return false;
		endif
		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		if (mobile.GetProperty("pilotando"))
			return _GolemCleave(mobile, who);
		endif

		var anvil;
		foreach item in ListItemsNearLocation(who.x, who.y, who.z, 1, who.realm)
			if (item.objtype == 0xA102 || item.objtype == 0xA108)
				anvil := item;
				break;
			endif
		endforeach

		// Find gadget in backpack
		var gadget_item := 0;
		var gadget_key := "";
		var cannon := FindObjtypeInContainer(who.backpack, GADGET_HANDCANNON);
		var harpoon := FindObjtypeInContainer(who.backpack, GADGET_HARPOON);
		var trap := FindObjtypeInContainer(who.backpack, GADGET_POWDERTRAP);

		var gadget_count := 0;
		if (cannon)
			gadget_count += 1;
			gadget_item := cannon;
			gadget_key := "handcannon";
		endif
		if (harpoon)
			gadget_count += 1;
			gadget_item := harpoon;
			gadget_key := "harpoon";
		endif
		if (trap)
			gadget_count += 1;
			gadget_item := trap;
			gadget_key := "powdertrap";
		endif

		if (gadget_count == 0)
			Message("Voce precisa de um gadget (canhao, arpao ou mina) para construir o golem!").SendSys(who);
			return false;
		endif

		// If multiple gadgets, let the player choose
		if (gadget_count > 1)
			var choice := _GadgetSelectionGump(who, cannon, harpoon, trap);
			if (!choice)
				return false;
			endif
			gadget_item := choice.item;
			gadget_key := choice.key;
		endif

		// Check all steam armor pieces in backpack
		var armor_parts := array{
			struct{ objtype := STEAM_GLOVES, name := "Manoplas a Vapor" },
			struct{ objtype := STEAM_LEGS,   name := "Grevas a Vapor" },
			struct{ objtype := STEAM_ARMS,   name := "Bracos a Vapor" },
			struct{ objtype := STEAM_HELM,   name := "Elmo a Vapor" },
			struct{ objtype := STEAM_CHEST,  name := "Peitoral a Vapor" },
			struct{ objtype := STEAM_GORGET, name := "Pescoceira a Vapor" }
		};

		foreach part in armor_parts
			if (!FindObjtypeInContainer(who.backpack, part.objtype))
				Message($"Voce precisa de {part.name} na mochila!").SendSys(who);
				return false;
			endif
		endforeach

		if (!yesno(who, "Voce utilizara 100 minerios de Mithril, 50 couros curtidos, todas as pecas de Armadura a Vapor e 1 gadget. Deseja continuar?"))
			return false;
		endif

		var has_mithril := 0;
		var has_leather := 0;
		foreach item in EnumerateItemsInContainer(who.backpack)
			if (item.objtype == 0x6311 && !has_mithril)
				if (!SubtractAmount(item, 100))
					Message("Voce precisa de 100 minerios de Mithril.").SendSys(who);
					return false;
				endif
				has_mithril := 1;
			endif
			if (item.objtype == 0xee45 && !has_leather)
				if (!SubtractAmount(item, 50))
					Message("Voce precisa de 50 couros curtidos.").SendSys(who);
					return false;
				endif
				has_leather := 1;
			endif
		endforeach

		if (!has_mithril)
			Message("Voce precisa de 100 minerios de Mithril.").SendSys(who);
			return false;
		endif
		if (!has_leather)
			Message("Voce precisa de 50 couros curtidos.").SendSys(who);
			return false;
		endif

		// Consume the gadget
		SubtractAmount(gadget_item, 1);

		// Consume all steam armor pieces
		foreach part in armor_parts
			var armor_item := FindObjtypeInContainer(who.backpack, part.objtype);
			DestroyItem(armor_item);
		endforeach

		who.frozen := 1;
		if (anvil.objtype == 0xA108)
			anvil.graphic := 0xA103;
		elseif (anvil.objtype == 0xA102)
			anvil.graphic := 0xA109;
		endif

		var i := 0;
		while (i < 10)
			PerformAction(who, 12);
			Message("*Construindo*").SendAbove(who);
			PlaySoundEffect(who, SFX_ANVIL);
			PlayStationaryEffect(who.x, who.y, who.z, 0x6DF1, 0, 40, 0, who.realm);
			sleep(4);
			i++;
		endwhile

		if (anvil.graphic == 0xA103)
			anvil.graphic := 0xA108;
		elseif (anvil.graphic == 0xA109)
			anvil.graphic := 0xA102;
		endif

		who.frozen := 0;
		PlaySoundEffect(who, SFX_SPELL_SUMMON_ELEMENTAL);

		var template := ":epicai:PowerArmor";
		var golem := CreateNpcFromTemplate(template, who.x, who.y, who.z, 0, who.realm);
		golem.frozen := 1;
		SetObjProperty(golem, "golem_maker", who.serial);
		SetObjProperty(golem, "InstalledGadget", gadget_key);
		mobile.SetProperty("golem_maker", 1);

		Message("O mecanismo esta pronto para ser operado").SendAbove(golem);
		Message("Sua arte. Esculpida em aco!").SendSys(who);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[GOLEM_ASSEMBLY] Golem criado com gadget: {gadget_key}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass

function _GolemCleave(mobile, who)
	// Recursion guard
	if (GetObjProperty(who, "#golem_cleaving"))
		return false;
	endif
	SetObjProperty(who, "#golem_cleaving", 1);

	PerformAction(who, HUMAN_ANIM::ATTACK_2HAND_BLUNT);
	PlaySoundEffect(who, 0x11F);

	var mods := GetModByFacing(who.facing);
	var dx := mods[1];
	var dy := mods[2];
	var attacker := MobileWrapper(who);
	var festa := who.party;

	for i := 0 to 3
		var tx := who.x + (dx * i);
		var ty := who.y + (dy * i);
		// Check 1 tile radius around the line for width
		foreach targ in ListMobilesNearLocation(tx, ty, who.z, 1, who.realm)
			if (targ == who || targ.dead || targ.concealed)
				continue;
			endif
			if (festa && targ in festa.members)
				continue;
			endif
			var char_owner := GetObjProperty(targ, "owner");
			if (char_owner == who.serial)
				continue;
			endif
			attacker.Attack(MobileWrapper(targ));
		endforeach
	endfor

	EraseObjProperty(who, "#golem_cleaving");

	DebugMsg(
		who,
		DEBUG_HAB,
		Message("[GOLEM_ASSEMBLY] Golem Cleave!", Color(SYSTEM_COLOR::SUCCESS))
	);

	return true;
endfunction

function _GadgetSelectionGump(who, cannon, harpoon, trap)
	var gump := GFCreateGump();
	GFClosable(gump, 1);
	GFMovable(gump, 1);
	GFResizePic(gump, 0, 0, 0x0A28, 400, 200);
	GFHTMLArea(gump, 0, 16, 400, 25, "<CENTER><BASEFONT SIZE=5 COLOR=#DDAA44>Escolha o Gadget</BASEFONT></CENTER>", 0, 0);

	var x := 15;
	if (cannon)
		GFTilePic(gump, x + 35, 50, CInt(GADGET_HANDCANNON), 0);
		GFTextLine(gump, x + 5, 105, 0x0, "Canhao de Mao");
		GFAddButton(gump, x + 35, 140, 0xFA5, 0xFA7, GF_CLOSE_BTN, 1);
	endif
	x += 125;
	if (harpoon)
		GFTilePic(gump, x + 35, 50, CInt(GADGET_HARPOON), 0);
		GFTextLine(gump, x + 5, 105, 0x0, "Arpao Mecanico");
		GFAddButton(gump, x + 35, 140, 0xFA5, 0xFA7, GF_CLOSE_BTN, 2);
	endif
	x += 125;
	if (trap)
		GFTilePic(gump, x + 35, 50, CInt(GADGET_POWDERTRAP), 0);
		GFTextLine(gump, x + 5, 105, 0x0, "Mina de Polvora");
		GFAddButton(gump, x + 35, 140, 0xFA5, 0xFA7, GF_CLOSE_BTN, 3);
	endif

	var result := GFSendGump(who, gump);
	var butid := result[0];
	if (!butid)
		return 0;
	endif

	var ret := struct{};
	case (butid)
		1:
			ret.+item := cannon;
			ret.+key := "handcannon";
		2:
			ret.+item := harpoon;
			ret.+key := "harpoon";
		3:
			ret.+item := trap;
			ret.+key := "powdertrap";
		default:
			return 0;
	endcase

	return ret;
endfunction