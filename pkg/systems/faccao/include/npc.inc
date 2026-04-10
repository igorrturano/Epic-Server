// ###########################################
// #
// # AddNPC
// # remNPC
// # editNPC
// # listNPC
// # ContractNpc
// # listContractedNPC
// # DismissNPC
// # CheckPrivilegies
// # GetReports
// #
// ###########################################

include ":datafile:include/datafile";
include ":merchants:item_template";
include ":brainAI:include/npcInit";

function AddNpc(guild, npc)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif

   if (!guild) return; endif

   var new_npc := struct{
      "template" := npc.template,
      "equipment" := npc.equipment,
      "bossStruct" := npc.bossStruct,
      "price" := npc.price,
      "name" := npc.name
   };
   if (!new_npc.template)
      return 0;
   elseif (!new_npc.equipment)
      return 0;
   elseif (!new_npc.price)
      return 0;
   elseif (!new_npc.name)
      return 0;
   endif

   var npclist := listNPC(guild);
   npclist.append(new_npc);

   guild.setProp("npcs", npclist);
   return 1;
endfunction

function listNPC(guild)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif

   if (!guild) return array{}; endif

   var npcs := guild.GetProp("npcs");
   if (!npcs) npcs := array{}; endif

	var upgraded := 0;
	foreach template in (npcs)
		if (template.template == ":merchants:merchant")
			template.template := ":merchants:uosevendor";
			upgraded := 1;
		endif
	endforeach
	
	if (upgraded)
		guild.SetProp("npcs", npcs);
	endif

   return npcs;
endfunction

function remNPC(guild, index)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif
   if (typeof(index) == "Integer")
      var npc_list := listNPC(guild);
      npc_list.Erase(index);
      guild.setProp("npcs", npc_list);
   endif
endfunction

function listContractedNPC(guild)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif

   if (!guild) return; endif

   var npcs_active := guild.GetProp("npcs_active");
   if (!npcs_active) npcs_active := array{}; endif

   return npcs_active;
endfunction

function DismissNPC(guild, npc_i, npc := 0)
   var npcs := listContractedNPC(guild);
   if (!npc || !npc.isA(POLCLASS_MOBILE))
      npc := SystemFindObjectBySerial(npcs[npc_i].serial);
   endif

   npcs.Erase(npc_i);
   guild.setProp("npcs_active", npcs);

   npc.Disable("invul");
   RevokePrivilege(npc, "invul");
   SetObjProperty(npc, "GuardKill", 1);
   SetObjProperty(npc, "NoCorpse", 1);
   EraseObjProperty(npc, "lastHit");

   // Move o NPC para o limbo antes de remover para evitar ganho de XP/RP por proximidade
   MoveObjectToLocation(npc, 6084, 1, 1, npc.realm, MOVEOBJECT_FORCELOCATION);
   ApplyRawDamage(npc, 8000);
endfunction



function ContractNPC(guild, npc_i, who, loc)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif

   if (!guild) return; endif

   var npcs := listNpc(guild);
   var template := npcs[npc_i];
   if (!template) return; endif

    var npc_list := listContractedNPC(guild);
    if(len(npc_list) < 15)
        var agent := CreateNPCFromTemplate(template.template, loc.x, loc.y, loc.z, 0, who.realm);
        if (!agent || agent == error)
            SendSysMessageEx(who, "Erro ao criar NPC ("+template.template+"): " + agent.errortext, SSM_FAIL);
            return 0;
        else
            SendSysMessageEx(who, "NPC " + template.template + " criado com sucesso.", SSM_INFO);
        endif

      if (agent.npctemplate && lower(agent.npctemplate)["merchant"])
         SetObjProperty(agent, "noequip", 1);
      endif

      sleepms(500); // Aguarda o script C do NPC rodar e gerar nome, roupa e raça corretamente.
      
      if (agent.name["<random>"])
         var name_template := "Human_Male";
         if (agent.gender == 1 || agent.graphic == 401 || agent.graphic == 1829 || agent.graphic == 1831 || agent.graphic == 1833)
             name_template := "Human_Female";
         endif
         NPC_SetupName(agent, name_template);
      endif

		// Corrige o bug critico onde o Vendedor virava Guarda
		var template_name_lower := lower(template.name);
		var template_id_lower := lower(template.template);
		if (template_name_lower["vendedor"] == error && template_name_lower["mercador"] == error && template_name_lower["vendor"] == error && template_id_lower["merchant"] == error && template_id_lower["vendor"] == error)

			agent.script := ":ghaia:guard";
			SetObjproperty(agent, "script", agent.script);
			RestartScript(agent);

			SetObjProperty(agent, "guild", guild.guildid);
       		SetObjProperty(agent, "contract_by", who.serial);
       		SetObjProperty(agent, "conf_ref",npc_i);

			if (template.equipment && template.equipment.size() > 0)
				foreach item_ref in (template.equipment)
					sleepms(2);
					var item := CreateIteminBackpackFromTemplate(agent, item_ref);
					var equipped := EquipItem(agent, item);
	       		endforeach
			endif

			PrintTextAbove(agent, "As suas ordens");

			npc_list.append(struct{
				"serial" := agent.serial,
				"price" := template.price,
				"ref" := npc_i
			});
			guild.setProp("npcs_active", npc_list);
	   		return 1;
		endif

			SetObjProperty(agent, "guild", guild.guildid);
			SetObjProperty(agent, "contract_by", who.serial);
			SetObjProperty(agent, "conf_ref",npc_i);


			if (template.equipment && template.equipment.size() > 0)
				foreach item_ref in (template.equipment)
					sleepms(2);
					var item := CreateIteminBackpackFromTemplate(agent, item_ref);
					var equipped := EquipItem(agent, item);
				endforeach
			endif
/*
       if (template.bossStruct.keys().size() >= 1)
          SetBoss(agent, template.bossStruct);
       endif
*/
			PrintTextAbove(agent, "As suas ordens");

			npc_list.append(struct{
				"serial" := agent.serial,
				"price" := template.price,
				"ref" := npc_i
			});
			guild.setProp("npcs_active", npc_list);
    else
        SendSysMessageEx(who, "Quantidade maxima de agentes atingida.");
    endif

   return 1;
endfunction