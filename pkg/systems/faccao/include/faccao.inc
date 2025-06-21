use guilds;
use os;
use polsys;

// Incluir o arquivo de constantes
include ":faccao:faccao_constants";
include ":faccao:include/datafile_helpers"; // <--- ADD THIS LINE
include "include/client";
include ":faccao:include/territory_management"; // Dependência para funções de território

// ###########################################
// #
// # CreateFaccao
// # GetFaccao
// # EditFaccao
// # AddMember
// # GetMember
// # RemMember
// # EditMember
// # SetLeader
// # IsLeader
// # 
// ###########################################

function CreateFaccao(name, rect, money := 0)
   if (!name || typeof(name) != "String") return 0; endif
   var guild := CreateGuild();
   guild.setProp(FACTION_PROP_NAME, name);
   
   // CORREÇÃO: Inicializar leader como 0x0 em vez de 0 para evitar problemas
   guild.setProp("leader", 0x0);
   
   // IMPORTANTE: Não definir guild.leader aqui pois ainda não há membros
   // O líder será definido via SetFactionLeader() quando alguém criar a facção
   
   guild.setProp("npcs_active", array{});
   guild.setProp("npcs", array{});
   guild.setProp("report", array{"Faccao Criada"});
   guild.setprop("area", rect);
   guild.setprop("money", cint(money));
   guild.setprop("uniforms", array{});
   guild.setprop("link_to", 0);

   return guild;
endfunction

function GetFaccao(guild)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif

   if (!guild) return; endif   var faccao_obj := struct{
      "name"     := guild.GetProp(FACTION_PROP_NAME),
      "npcs"     := guild.GetProp("npcs"),
      "npcs_active"  := guild.GetProp("npcs_active"),
      "laws"     := guild.GetProp("laws"),
      "leader"   := guild.GetProp("leader"),
      "area"     := guild.GetProp("area"),
      "uniforms" := guild.GetProp("uniforms"),
      "subguilds" := guild.GetProp("subguilds"),
      "money" := guild.GetProp("money")
   };

   return faccao_obj;
endfunction

function EditFaccao(guild, props)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif
   if (!guild) return; endif

   var faccao := GetFaccao(guild);
   if (props.name) // strimg
      faccao.name := props.name;
   endif

   if (props.npcs != null) // array
      faccao.npcs := props.npcs;
   endif

   if (props.laws  != null) // array
      faccao.laws := props.laws;
   endif

   if (props.area  != null) // array x y x2 y2
      faccao.area := props.area;
   endif

   if (props.leader != null) // serial string
      faccao.leader := props.leader;
   endif

   if (props.uniforms) // array
      faccao.uniforms := props.uniforms;
   endif


   if (props.link_to != null) // fac string
      faccao.link_to := props.link_to;
   endif

   foreach key in (faccao.keys())
      guild.SetProp(key, faccao[key]);
      sleepms(2);
   endforeach

   return 1;
endfunction


/*
 * RemFaccao(guild_obj)
 * 
 * Purpose: Realiza a remoção COMPLETA e SEGURA de uma facção do sistema.
 *          Esta função é responsável apenas pela lógica de backend (apagar dados),
 *          NÃO envia mensagens para jogadores. A notificação deve ser feita
 *          pelo script que chama esta função.
 * 
 * Parameters:
 *   guild_obj: Objeto da guilda, ID da guilda (integer) ou nome da guilda (string).
 * 
 * Return:
 *   1 se a guilda foi destruída com sucesso.
 *   0 se a guilda não foi encontrada ou se a destruição falhou.
 */
function RemFaccao(guild_obj)
    // Etapa 1: Garantir que temos um objeto de guilda válido.
    // Esta parte torna a função flexível para aceitar ID, nome ou o objeto direto.
    if (typeof(guild_obj) == "String" or typeof(guild_obj) == "Integer")
        guild_obj := FindGuild(guild_obj);
    endif

    if (!guild_obj) 
        // Se não encontrou a guilda, não há nada a fazer.
        return 0; 
    endif

    // Etapa 2: Obter informações básicas para logs antes de apagar.
    var guild_name := guild_obj.GetProp(FACTION_PROP_NAME) ?: guild_obj.guildname ?: "Facção Desconhecida";
    var guild_id_log := guild_obj.guildid;
    LogToFile("faccoes.log", "REMFACCAO_START: Iniciando remoção da facção '" + guild_name + "' (ID: " + guild_id_log + ")");

    // Etapa 3: Limpar NPCs contratados.
    var npcs_active := guild_obj.GetProp("npcs_active");
    if (npcs_active && npcs_active.size() > 0)
        foreach npc_data in npcs_active
            if (npc_data && npc_data.serial)
                var npc_mob := SystemFindObjectBySerial(npc_data.serial);
                if (npc_mob)
                    npc_mob.kill(); // Mata o NPC para removê-lo do mundo.
                endif
            endif
            sleepms(2);
        endforeach
    endif

    // Etapa 4: Liberar todos os territórios controlados.
    var territories := guild_obj.GetProp("territories");
    if (territories && territories.size() > 0)
        LogToFile("faccoes.log", "--> Limpando " + territories.size() + " territórios da facção '" + guild_name + "'");
        foreach territory_id in territories
            if (!territory_id) continue; endif;
            
            // Usa as funções centralizadas do `territory_management` para fazer a limpeza.
            TM_DeleteTerritoryObjects(territory_id, guild_name); // Apaga baú, mesa, etc.
            TM_SetTerritoryFaction(territory_id, DEFAULT_FACTION); // Define o território como "Selvagem".
            
            sleepms(5);
        endforeach
    endif

    // Etapa 5: Remover todos os membros e suas propriedades de privilégios.
    // Copiamos a lista de membros primeiro para evitar problemas ao modificar a lista enquanto iteramos.
    var members_copy := guild_obj.members;
    foreach member in members_copy
        if (member)
            guild_obj.EraseProp(CStr(member.serial));   // Apaga CProp de privilégios.
            EraseObjProperty(member, "dados_convite"); // Apaga CProp do personagem.
            guild_obj.removemember(member);            // Remove o membro da guilda.
        endif
        sleepms(2);
    endforeach

    // Etapa 6: Remover todas as relações de inimizade.
    var enemy_guilds_copy := guild_obj.enemyguilds;
    foreach enemy in enemy_guilds_copy
        if (enemy)
            guild_obj.removeenemyguild(enemy);
        endif
        sleepms(2);
    endforeach
    
    // Etapa 7: Limpar todas as CProps customizadas da própria guilda.
    var props_to_clean := array{
        "name", "leader", "npcs_active", "npcs", "report", "area", 
        "money", "uniforms", "link_to", "subguilds", "laws", "territories",
        "recursos", "recursos_timestamp", "culture" // Adicionando outras CProps do sistema
    };
    foreach prop in props_to_clean
        guild_obj.EraseProp(prop);
        sleepms(2);
    endforeach

    // Etapa 8: Destruir o objeto da guilda.
    var destruction_result := DestroyGuild(guild_obj);
   
    if (!destruction_result)
        LogError("faccao", "FALHA ao destruir a guilda " + guild_name + " (ID: " + guild_id_log + ").");
    else
        LogToFile("faccoes.log", "REMFACCAO_END: Facção '" + guild_name + "' removida com sucesso.");
    endif
   
    return destruction_result;
endfunction

function AddMember(faccao, mobile)
   faccao.addMember(mobile);   var member_obj := struct{
      "above_guards" := 0, // can control guards
      "above_merchants" := 0, // can control merchants
      "above_members" := 0, // can add members
      "giveprivs" := 0, // can edit privs
      "contractnpc" := 0, // can contract npc
      "fixitems" := 0, // can fix items
      "unlockdoors" := 0, // can unlock doors
      "buyuniforms" := 0, // can buy uniform recipes
      "architect" := 0 // can use architect system
   };
   faccao.SetProp(cstr(mobile.serial), member_obj);
endfunction


function EditMember(guild, serial, props)
   var member := guild.GetProp(serial);
   if (props.name)
      faccao.name := props.name;
   endif

   if (props.npcs)
      faccao.npcs := props.npcs;
   endif

   if (props.laws)
      faccao.laws := props.laws;
   endif

   if (props.area)
      faccao.area := props.area;
   endif

   if (props.leader)
      faccao.leader := props.leader;
   endif

   if (props.uniforms)
      faccao.uniforms := props.uniforms;
   endif

   foreach key in (faccao.keys())
      guild.SetProp(key, faccao[key]);
      sleepms(2);
   endforeach

   return 1;
endfunction

function RemMember(faccao, mobile)
   if (!mobile.isA(POLCLASS_MOBILE))
      mobile := SystemFindObjectBySerial(mobile, SYSFIND_SEARCH_OFFLINE_MOBILES);
   endif

   if (faccao.GetProp("leader") == mobile.serial)
      faccao.SetProp("leader", 0x0);
   endif
   faccao.removemember(mobile);
   faccao.EraseProp(cstr(mobile.serial));
endfunction

function SetLeader(faccao, serial)
   faccao.SetProp("leader", serial);
endfunction

function isLeader(faccao, mobile)
   var leader := faccao.GetProp("leader");
   if (leader == mobile.serial)
      return 1;
   endif

   return 0;
endfunction

function inGuildRect(guild, item := 0)
    if (typeof(guild) == "String")
        guild := FindGuild(guild);
    endif

    if (!guild) return; endif

    // Verificar nas áreas definidas diretamente
    var area := guild.getProp("area");
    foreach rect in area
        sleepms(2);
        if (!rect || rect.size() < 4) 
            continue;
        elseif (item.x >= cint(rect[1]) && item.x <= cint(rect[3]) && item.y >= cint(rect[2]) && item.y <= cint(rect[4]));
            return 1;
        endif
    endforeach

    // Verificar nos territórios controlados
    var territories := guild.GetProp("territories");
if (!territories || territories == error || TypeOf(territories) != "Array")
    territories := array{};
endif

// Garantir que seja um array limpo
var cleaned_territories := array{};
foreach terr in territories
    // Apenas adicionar itens válidos
    if (TypeOf(terr) == "String" && terr != "")
        cleaned_territories.append(terr);
    endif
    sleepms(2);
endforeach
territories := cleaned_territories;
    if (territories)
        var rect_df := SafeOpenDataFile(":architect:areas");
        foreach territory in territories
            var elem := SafeFindElement(rect_df, territory);
            if (!elem) continue; endif
              // Verificar AMBAS as propriedades possíveis para compatibilidade
            var faction := elem.GetProp(TERRITORY_FACTION_PROP);  // Maiúscula (legado)
            if (!faction || faction == error)
                faction := elem.GetProp(TERRITORY_FACTION_PROP);  // Minúscula (novo padrão)
            endif
              // Compare com o nome da guilda de forma segura
            var guild_name := guild.GetProp(FACTION_PROP_NAME);
            if (guild_name == error) guild_name := ""; endif
            
            if (faction == guild_name)
                // Verificar se o item está dentro das coordenadas
                var coords := SplitWords(elem.GetProp("Rect"));
                if (coords.size() >= 4)
                    if (item.x >= CInt(coords[1]) && item.x <= CInt(coords[3]) && 
                        item.y >= CInt(coords[2]) && item.y <= CInt(coords[4]))
                        return 1;
                    endif
                endif
            endif
            
            sleepms(2);
        endforeach
    endif

    return 0;
endfunction

function AddReport(guild, report)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif

   if (!guild) return; endif

   var reports := guild.getProp("reports");
   if (!reports) reports := array{}; endif
   reports.Insert(1, report);

   if (reports.size() > 30)
      reports.Erase(reports.size());
   endif

   guild.setProp("reports", reports);
endfunction

function changeGuildMoney(guild, more_money)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif

   if (!guild) return; endif
   
   var money := cint(guild.getProp("money"));

   money += cint(more_money);

   guild.setProp("money", money);
endfunction

/*

function SetBoss(npc, boss)
	var cfg :=	NPC_GetNPCConfig(boss.npctemplate);

	if (boss.name != "-" && boss.name != "-random-")
		SetName(npc, boss.name);
	endif

	npc.color := boss.color;
	npc.truecolor := boss.color;
   
   var no_tame;
   if (boss.power) npc.damage_physical_mod := boss.power; no_tame := 1; endif
   if (boss.cd_block) AP_SetTrueSkill(npc, "Block", boss.cd_block); no_tame := 1; endif
   if (boss.cd_dodge) AP_SetTrueSkill(npc, "Dodge", boss.cd_dodge); no_tame := 1; endif
   if (boss.critical) AP_SetTrueSkill(npc, "Critical", boss.critical); no_tame := 1; endif
   if (boss.roleplay_desc) SetObjProperty(npc, "roleplay_desc", boss.roleplay_desc); endif

   SetObjProperty(npc, "ArmorResists", boss.res_mods);

	if (boss.customAttacks && boss.customAttacks[1])
		var customAttacks := array{};
		foreach attack in (boss.customAttacks)
			attack := FormatAttack(boss.npctemplate, attack, cint(cfg.AttackDamage) + cint(boss.power));
			customAttacks.append(attack);
		endforeach
		SetObjProperty(npc, "customAttacks", customAttacks);
	endif

   if (boss.hits) SetObjProperty(npc, "hitsmod", cint(boss.hits)); no_tame := 1; endif
  	RecalcVitals(npc);
	AP_RefreshVitals(npc);

   if (no_tame) SetObjProperty(npc, "cant_tame", 1); endif

	if (!npc.backpack)
		var backpack := CreateItemAtLocation(npc.x, npc.y, npc.z,  2482,  1, npc.realm);
		EquipItem(npc, backpack);
	endif

   var skinning_info := array{};
   foreach loot in (boss.loot)
      if(loot.by_group)
         Loot_Generate(npc.backpack, loot);
      elseif (loot.skinning)
         skinning_info.append(loot);
      elseif( RandomDiceRoll("1d100") <= loot.chance)
         var item := CreateItemAtLocationFromTemplate(npc.x, npc.y, npc.z, _DEFAULT_REALM, loot.item );
         MoveItemToContainer(item, npc.backpack);
    		item.movable := 1;
      endif
      sleepms(2);
   endforeach

   if (skinning_info.size())
      SetObjProperty(npc, "skinninginfo", skinning_info);
   endif

	if (boss.deathevent)
		SetObjProperty(npc, "deathevent", boss.deathevent);
	endif

  if (boss.customai)
    npc.script := boss.customai;
  endif

	if (boss.special)
		foreach spec in (boss.special)
         if (spec.name["Armor"])
            SetArmorHitscript(npc, spec.hitopt);
         elseif (spec.name["Critical"])
            SetCriticalHitscript(npc, spec.hitopt);
         elseif (spec.name["Attack"])
            SetHitscript(npc, spec.hitopt);
         elseif (spec.name == "Custom Hitscript")
            SetObjProperty(npc, "generichit", spec.hitopt);
            var hit_scripts := GetObjProperty(npc, "HitScripts");
            if ( !hit_scripts )
               hit_scripts := array{};
            endif
            hit_scripts.Append(":combat:weaponHitScripts/generic");
            SetObjProperty(npc, "HitScripts", hit_scripts);
         elseif (spec.name == "Special AI")
            npc.script := spec.ai;
      //      printtextabove(npc, " " + spec.ai);
            restartscript(npc);
         endif
         sleepms(2);
		endforeach
	endif	

   RestartScript(npc);
	return npc;
  
endfunction

*/

function FormatAttack(npctemplate, new_attack, AttackDamage)
	var attack := SplitWords(attack, " ");

	attack := struct{
		"name"    := attack[1],
		"dicedmg"    := attack[2],
		"DamageType" := attack[3],
		"AttackAnim" := cint(attack[4]),
      "AttackDamage" := AttackDamage,
		"Attribute"  := npctemplate.AttackAttribute,
		"MaxRange"   := npctemplate.MaxRange,
		"MinRange"   := npctemplate.MinRange
	};

	return attack;
endfunction

function SetArmorHitscript(who, hitscript, remove := 0);
   var hitscripts := GetObjProperty(who, "ArmorHitScripts");
   if (!hitscripts)
      hitscripts := array{};
   endif

   if (!remove)
      if (!FindInArray(hitscripts, hitscript))
         hitscripts.append(hitscript);
      endif
   else
      hitscripts := RemoveFromArray(hitscripts, hitscript);
   endif

   if (hitscripts.size() < 1)
      return EraseObjProperty(who, "ArmorHitScripts");
   endif
   SetObjProperty(who, "ArmorHitScripts", hitscripts);
endfunction

function SetCriticalHitscript(who, hitscript, remove := 0);
   var criticalhitscript := GetObjProperty(who, "CriticalHitScripts");
   if (!criticalhitscript)
      criticalhitscript := array{};
   endif

   if (!remove)
      if (!FindInArray(criticalhitscript, hitscript))
         criticalhitscript.append(hitscript);
      endif
   else
      criticalhitscript := RemoveFromArray(criticalhitscript, hitscript);
   endif

   if (criticalhitscript.size() < 1)
      return EraseObjProperty(who, "CriticalHitScripts");
   endif
   SetObjProperty(who, "CriticalHitScripts", criticalhitscript);
endfunction

function SetHitscript(who, hitscript, remove := 0);
   var hitscripts := GetObjProperty(who, "HitScripts");
   if (!hitscripts)
      hitscripts := array{};
   endif

   if (!remove)
      if (!FindInArray(hitscripts, hitscript))
         hitscripts.append(hitscript);
      endif
   else
      hitscripts := RemoveFromArray(hitscripts, hitscript);
   endif

   if (hitscripts.size() < 1)
      return EraseObjProperty(who, "HitScripts");
   endif
   SetObjProperty(who, "HitScripts", hitscripts);
endfunction