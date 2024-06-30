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

   if (!guild) return; endif

   var npcs := guild.GetProp("npcs");
   if (!npcs) npcs := array{}; endif

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
   SetObjProperty(npc, "GuardKill", 1);
   SetObjProperty(npc, "NoCorpse", 1);
   ApplyRawDamage(npc, 500);
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
       var agent := CreateNPCFromTemplate(template.template, loc.x, loc.y, loc.z+2, 0, who.realm);
      if (agent.npctemplaye["merchant"])
         SetObjProperty(agent, "noequip", 1);
      endif

		if (agent.name != "Vendedor")

			agent.script := ":ghaia:guard";
			SetObjproperty(agent, "script", agent.script);
			RestartScript(agent);

			SetObjProperty(agent, "guild", guild.guildid);
       		SetObjProperty(agent, "contract_by", who.serial);
       		SetObjProperty(agent, "conf_ref",npc_i);

			foreach item_ref in (template.equipment)
				sleepms(2);
				var item := CreateIteminBackpackFromTemplate(agent, item_ref);
				var equipped := EquipItem(agent, item);
       		endforeach

			PrintText(agent, "As suas ordens");

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


			foreach item_ref in (template.equipment)
				sleepms(2);
				var item := CreateIteminBackpackFromTemplate(agent, item_ref);
				var equipped := EquipItem(agent, item);
			endforeach
/*
       if (template.bossStruct.keys().size() >= 1)
          SetBoss(agent, template.bossStruct);
       endif
*/
			PrintText(agent, "As suas ordens");

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