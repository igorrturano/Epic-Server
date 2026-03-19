use uo;
use os;

include ":abilities:models/CombatAbility";
include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":fls_core:packets";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":tn:cooldown";
include "include/epicConstants";

/**
 * Vinculo Elemental (Elemental Bond) - ABILITY_ID::VINCULO_ELEMENTAL (2300)
 *
 * Active (ON_USE): Summon a spiritual blade (0x090C template).
 * - Requires at least one weapon absorbed in pool (via Absorver Arma spell)
 * - Player picks which absorbed weapon to use as visual
 * - Two-step ritual: spiritual form appears first, then transforms to absorbed graphic
 * - Damage scales with Arcana via CalculateSpiritualBladeDamage() in DamageController
 * - Unequip destroys the blade (unequipSpiritualBlade)
 *
 * Passive (via HasAbility checks):
 * - Allows casting spells while wielding weapons [cast_spell.src]
 * - Prevents disarm [desarmar.src]
 * - Bypasses movement penalties for spellcasting [cast_spell.src]
 */
class ElementalBond(CombatAbility)

	function ElementalBond(this)
		Super(ABILITY_ID::VINCULO_ELEMENTAL);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();

		if (GetCooldown(who, "spiritual_weapon"))
			Message("Voce ja possui uma lamina espiritual invocada!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		var pool := GetObjProperty(who, "spiritualWeaponPool");
		if (!pool || pool.size() < 1)
			Message("Voce nao possui armas absorvidas! Use Absorver Arma primeiro.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		var pool := GetObjProperty(who, "spiritualWeaponPool");
		if (!pool || pool.size() < 1)
			return false;
		endif

		// Show weapon pool selection gump
		var result := _ShowWeaponPoolGump(who, pool);
		var buttonId := result[0];

		if (!buttonId || buttonId == 0)
			Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		// Handle "forget weapon" buttons (300+)
		if (buttonId >= 300)
			var forgetIdx := buttonId - 300;
			if (forgetIdx >= 1 && forgetIdx <= pool.size())
				var forgotten := pool[forgetIdx];
				pool.Erase(forgetIdx);
				SetObjProperty(who, "spiritualWeaponPool", pool);
				Message($"A essencia de {forgotten.name} foi dissipada.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			endif
			return false;
		endif

		// Handle "select weapon" buttons (200+)
		if (buttonId < 200 || buttonId >= 300)
			return false;
		endif

		var selectedIdx := buttonId - 200;
		if (selectedIdx < 1 || selectedIdx > pool.size())
			return false;
		endif

		var weaponData := pool[selectedIdx];

		// Summoning ritual - freeze and cast animation
		who.frozen := 1;
		PerformAction(who, HUMAN_ANIM::CAST_DIR);
		Message("*Invocando lamina espiritual*").SendAbove(who);
		PlaySoundEffect(who, 0x5cb);
		PlayStationaryEffectEx(who.x, who.y, who.z, who.realm, 0xB462, 8, 40, COLOR_ELEMENTAL_ENERGY);
		sleep(2);

		// Step 1: Create spiritual blade in original form (0x090C)
		var weapon := CreateItemInBackpack(who, 0x090C, 1);
		if (!weapon)
			who.frozen := 0;
			Message("Erro ao criar lamina espiritual.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return 0;
		endif

		EquipItem(who, weapon);

		// Ensure full HP after equip
		if (weapon.maxhp > 0)
			weapon.hp := weapon.maxhp;
		endif

		// Brief pause — spiritual essence is visible
		sleep(2);
		// Step 2: Transform to absorbed weapon's graphic (keep spiritual color)
		weapon.graphic := weaponData.graphic;
		weapon.color := 30000;

		// VFX for transformation
		PlaySoundEffect(who, 0x245);
		PlayStationaryEffectEx(who.x, who.y, who.z, who.realm, 0xB462, 8, 40, COLOR_ELEMENTAL_ENERGY);

		who.frozen := 0;

		SetCooldown(who, "spiritual_weapon", 99999);

		Message("Sua lamina espiritual se materializa!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[VINCULO_ELEMENTAL] Lamina espiritual invocada (visual: {weaponData.name})", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass

function _ShowWeaponPoolGump(player, pool)
	var gump := GFCreateGump();

	var rowHeight := 26;
	var gumpHeight := 125 + pool.size() * rowHeight;

	GFResizePic(gump, 17, 31, 9390, 400, gumpHeight);
	GFHTMLArea(gump, 114, 69, 250, 20, "<BASEFONT Size=5 Color=#999999 >Lamina Espiritual", 0, 0);
	GFHTMLArea(gump, 116, 71, 250, 20, "<BASEFONT Size=5 Color=#999999 >Lamina Espiritual", 0, 0);
	GFHTMLArea(gump, 115, 70, 250, 20, "<BASEFONT Size=5 Color=#ffffff >Lamina Espiritual", 0, 0);
	GFGumpPic(gump, 63, 91, 1469);
	GFGumpPic(gump, 80, 91, 1469);
	GFTextLine(gump, 63, 100, 0, "Armas Absorvidas");
	GFTextLine(gump, 259, 100, 0, "Invocar");
	GFTextLine(gump, 330, 100, 0, "Esquecer");
	GFGumpPic(gump, 38, 103, 9773, 0);

	var y := 121;
	var i;
	for (i := 1; i <= pool.size(); i++)
		var entry := pool[i];
		GFHTMLArea(gump, 63, y, 192, 24, "<BASEFONT color=#000000>{} (0x{:04X})".format(entry.name, entry.graphic), 1, 0);
		GFAddButton(gump, 274, y + 5, 10006, 10006, GF_CLOSE_BTN, 200 + i); // Invoke
		GFAddButton(gump, 345, y + 5, 10006, 10006, GF_CLOSE_BTN, 300 + i); // Forget
		y += rowHeight;
	endfor

	return GFSendGump(player, gump);
endfunction
