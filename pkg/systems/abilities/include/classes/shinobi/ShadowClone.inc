use uo;
use os;
use polsys;
use util;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";
include "include/client";
include "include/sysEvent";

/**
 * Clone das Sombras (Shadow Clone) - ABILITY_ID::SHADOW_CLONE (1303)
 *
 * Active (OnUse): Creates Tactics/20 clones that confuse enemies,
 * making them switch targets. Clones last Tactics/5 seconds.
 * Reuses logic from spell_logic/illusionist/clones.src.
 */
class ShadowClone(CombatAbility)

	function ShadowClone(this)
		Super(ABILITY_ID::SHADOW_CLONE);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var tactics := CInt(mobile.GetSkill(TACTICS));

		var numClones := CInt(tactics / 20);
		if (numClones < 1)
			numClones := 1;
		endif

		var cloneDuration := CInt(tactics / 5);
		if (cloneDuration < 3)
			cloneDuration := 3;
		endif

		// Animation
		PerformAction(who, HUMAN_ANIM::CAST_AREA);
		PlaySoundEffect(who, 0x228);

		var clones := array{};
		who.hidden := 1;

		// Create clones
		for i := 1 to numClones
			var pos := this._GetRandomPosition(who);
			var clone := this._CreateClone(who, pos);
			if (clone)
				clones.append(clone);
			endif
		endfor

		// Swap position with a random clone
		if (len(clones) > 0)
			var randomClone := clones[RandomInt(len(clones)) + 1];
			this._SwapPositions(who, randomClone);

			// Assign hostile targets to clones
			var hostiles := ListHostiles(who, 8);
			if (len(hostiles) > 0)
				this._AssignTargets(clones, hostiles);
			endif
		endif

		sleep(1);
		foreach clone in clones
			clone.hidden := 0;
			clone.frozen := 0;
		endforeach
		who.hidden := 0;

		Message("*Cria clones de sombra!*").SendEmote(who);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[SHADOW_CLONE] Clones: {numClones}, Duracao: {cloneDuration}s", Color(SYSTEM_COLOR::SUCCESS))
		);

		// Wait for duration then destroy clones
		sleep(cloneDuration);
		this._DestroyClones(clones);

		return true;
	endfunction

	function _CreateClone(this, original, position)
		var template := ":brainai:humanmale";
		if (original.gender == 1)
			template := ":brainai:humanfemale";
		endif

		var clone := CreateNPCFromTemplate(template, position.x, position.y, position.z, 0, original.realm);
		if (!clone)
			return 0;
		endif

		clone.frozen := 1;
		clone.hidden := 1;
		clone.alignment := 2;

		// 1 HP - clones are fragile
		SetVital(clone, "hits", 1);
		SetVital(clone, "stamina", CInt(GetVitalMaximumValue(clone, "stamina")));

		// Copy appearance and equipment
		this._CopyAppearance(original, clone);
		this._CopyEquipment(original, clone);

		// Special properties
		SetObjProperty(clone, "NoCorpse", 1);
		SetObjProperty(clone, "MirrorImage", 1);
		SetObjProperty(clone, "Creator", original.serial);

		// Copy class properties for visual consistency
		if (GetObjProperty(original, "classe") != error)
			SetObjProperty(clone, "classe", GetObjProperty(original, "classe"));
		endif
		if (GetObjProperty(original, "kit") != error)
			SetObjProperty(clone, "kit", GetObjProperty(original, "kit"));
		endif
		if (GetObjProperty(original, "myhabs") != error)
			SetObjProperty(clone, "myhabs", GetObjProperty(original, "myhabs"));
		endif

		// Copy attributes for visual consistency
		foreach attrib_name in AP_ListAttributesByType()
			var valor := GetAttribute(original, attrib_name);
			var mod := AP_GetStatMod(original, attrib_name) * -1;
			valor := valor + mod;
			AP_SetTrueSkill(clone, attrib_name, valor);
			sleepms(10);
		endforeach

		clone.setMaster(original);
		return clone;
	endfunction

	function _CopyAppearance(this, source, dest)
		dest.gender := source.gender;
		dest.graphic := source.graphic;
		dest.color := source.color;
		dest.name := source.name;
		dest.title_race := source.title_race;
		dest.title_suffix := source.title_suffix;
		dest.title_prefix := source.title_prefix;
		dest.TrueColor := source.TrueColor;
		dest.trueobjtype := source.trueobjtype;
	endfunction

	function _CopyEquipment(this, source, dest)
		var mypack := CreateItemAtLocation(20, 20, 0, 0xe75, 1, dest.realm);

		foreach item in ListEquippedItems(source)
			var newItem := CreateItemCopyAtLocation(20, 20, 0, item);
			if (newItem)
				newItem.color := item.color;
				newItem.name := item.name;
				EquipItem(dest, newItem);
			endif
		endforeach

		EquipItem(dest, mypack);
	endfunction

	function _AssignTargets(this, clones, hostiles)
		Set_Critical(1);
		foreach clone in clones
			if (len(hostiles) > 0)
				var targetIndex := CInt(RandomInt(len(hostiles)) + 1);
				var targ := hostiles[targetIndex];

				var event := struct{ "type" := SYSEVENT_ENGAGED, "source" := targ };
				SendEvent(clone, event);

				event := struct{ "type" := SYSEVENT_ENGAGED, "source" := clone };
				SendEvent(targ, event);

				sleepms(20);
			endif
		endforeach
		Set_Critical(0);
	endfunction

	function _SwapPositions(this, char1, char2)
		var tempX := char1.x;
		var tempY := char1.y;
		var tempZ := char1.z;

		MoveObjectToLocation(char1, char2.x, char2.y, char2.z, char1.realm, MOVEOBJECT_FORCELOCATION);
		MoveObjectToLocation(char2, tempX, tempY, tempZ, char2.realm, MOVEOBJECT_FORCELOCATION);
		char1.facing := char2.facing;
	endfunction

	function _GetRandomPosition(this, who)
		return struct{
			"x" := CInt(who.x + (RandomInt(8) - 2)),
			"y" := CInt(who.y + (RandomInt(8) - 2)),
			"z" := CInt(who.z)
		};
	endfunction

	function _DestroyClones(this, clones)
		foreach clone in clones
			if (clone)
				SetObjProperty(clone, "guardkill", 1);
				ApplyDamage(clone, 9999);
			endif
			sleepms(2);
		endforeach
	endfunction

endclass
