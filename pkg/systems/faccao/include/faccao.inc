use guilds;
use os;
use polsys;

// Incluir o arquivo de constantes
include ":faccao:faccao_constants";
include ":faccao:include/datafile_helpers"; // <--- ADD THIS LINE
include "include/client";
include ":faccao:include/territory_management"; // Dependência para funções de território
include ":faccao:construcao/include/feudo_buildings";

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
// No arquivo pkg/systems/faccao/include/faccao.inc

/*
 * RemFaccao(guild_obj)
 * 
 * Purpose: Realiza a remoção COMPLETA e SEGURA de uma facção do sistema.
 *          Esta versão é robusta contra chamadas múltiplas para a mesma guilda.
 * 
 * Parameters:
 *   guild_obj: Objeto da guilda, ID da guilda (integer) ou nome da guilda (string).
 * 
 * Return:
 *   1 se a guilda foi destruída com sucesso ou se já não existia.
 *   0 se a guilda não foi encontrada ou se a destruição falhou.
 */
/*
 * RemFaccao(guild_obj)
 * 
 * Purpose: Realiza a remoção COMPLETA e SEGURA de uma facção do sistema.
 *          Esta versão é robusta contra chamadas múltiplas para a mesma guilda
 *          e limpa os datafiles mais críticos associados a ela.
 * 
 * Parameters:
 *   guild_obj: Objeto da guilda, ID da guilda (integer), nome da guilda (string) ou um objeto de erro.
 * 
 * Return:
 *   1 se a guilda foi destruída com sucesso ou se já não existia.
 *   0 se ocorreu uma falha inesperada.
 */
/*
 * RemFaccao(guild_obj) - VERSÃO 3.1 (DELEÇÃO SINCRONIZADA E LIMPA)
 * 
 * Purpose: Realiza a remoção COMPLETA e SEGURA de uma facção e TODOS os seus ativos,
 *          incluindo territórios, NPCs e construções de feudo.
 */
function RemFaccao(guild_obj)

    // ETAPA 1: Validação Robusta do Objeto Guilda
    var param_type := TypeOf(guild_obj);
    if (!guild_obj || param_type == "Error")
        LogError("RemFaccao_v3.1", "Chamada com parâmetro inválido. Abortando.");
        if(param_type == "Error")
            LogError("RemFaccao_v3.1", "  -> Erro recebido: " + guild_obj.errortext);
        endif
        return 0;
    endif

    if (param_type != "GuildRef")
        guild_obj := FindGuild(guild_obj);
    endif

    if (!guild_obj || TypeOf(guild_obj) != "GuildRef") 
        LogWarning("RemFaccao_v3.1", "Tentativa de apagar uma guilda que já não existe.");
        return 1;
    endif


    // ETAPA 2: Obter informações para Logs
    var guild_name := SafeGetGuildName(guild_obj);
    var guild_id_log := guild_obj.guildid;
    LogCritical("RemFaccao_v3.1", "==========================================================");
    LogCritical("RemFaccao_v3.1", "INICIANDO REMOÇÃO COMPLETA da facção '" + guild_name + "' (ID: " + guild_id_log + ")");


    // ETAPA 3: Limpar NPCs Contratados
    LogInfo("RemFaccao_v3.1", "--> ETAPA 3: Limpando NPCs contratados...");
    var npcs_active := guild_obj.GetProp("npcs_active");
    if (npcs_active && npcs_active.size() > 0)
        foreach npc_data in npcs_active
            if (npc_data && npc_data.serial)
                var npc_mob := SystemFindObjectBySerial(npc_data.serial);
                if (npc_mob)
                    npc_mob.kill();
                endif
            endif
            sleepms(2);
        endforeach
    endif
    LogInfo("RemFaccao_v3.1", "--> ETAPA 3 CONCLUÍDA.");


    // ETAPA 4: Liberar todos os territórios controlados
    LogInfo("RemFaccao_v3.1", "--> ETAPA 4: Liberando territórios controlados...");
    var territories := guild_obj.GetProp("territories");
    if (territories && TypeOf(territories) == "Array" && territories.size() > 0)
        foreach territory_id in territories
            if (territory_id)
                TM_DeleteTerritoryObjects(territory_id, guild_name);
                TM_SetTerritoryFaction(territory_id, DEFAULT_FACTION);
            endif
            sleepms(5);
        endforeach
    endif
    LogInfo("RemFaccao_v3.1", "--> ETAPA 4 CONCLUÍDA.");
    

    // ETAPA 5: Demolir todas as construções do feudo (USANDO A LISTA)
    LogInfo("RemFaccao_v3.1", "--> ETAPA 5: Lendo lista 'FeudoOwnedConstructions' para demolir construções...");
    var construcoes_demolidas := 0;
    var owned_constructions_seriais := guild_obj.GetProp("FeudoOwnedConstructions") ?: array{};

    if (owned_constructions_seriais.size() > 0)
        LogWarning("RemFaccao_v3.1", "    Encontradas " + owned_constructions_seriais.size() + " construções para demolir.");
        
        // Limpa a CProp da guilda ANTES de começar a demolir.
        guild_obj.EraseProp("FeudoOwnedConstructions");
        
        foreach serial_str in owned_constructions_seriais
            var serial := CInt(serial_str);
            var placa_mestre := SystemFindObjectBySerial(serial);
            if (placa_mestre)
                DemolirConstrucao(placa_mestre);
                construcoes_demolidas += 1;
            else
                LogWarning("RemFaccao_v3.1", "    Placa mestre com serial " + serial + " não encontrada no mundo.");
            endif
            LogInfo("RemFaccao_v3.1", "--> ETAPA 5 CONCLUÍDA. Total de construções que receberam ordem de demolição: " + construcoes_demolidas);
        endforeach
        
        // PAUSA CRÍTICA: Dá tempo para o motor do jogo processar os DestroyItem()
        LogInfo("RemFaccao_v3.1", "    Aguardando 2 segundos para a conclusão da demolição dos itens...");
        sleep(2);
    endif
    LogInfo("RemFaccao_v3.1", "--> ETAPA 5 CONCLUÍDA. Construções demolidas: " + construcoes_demolidas);


    // ETAPA 6: Remover todos os membros
    LogInfo("RemFaccao_v3.1", "--> ETAPA 6: Removendo membros da guilda...");
    var members_copy := guild_obj.members;
    if (members_copy)
        foreach member in members_copy
            if (member)
                guild_obj.EraseProp(CStr(member.serial));
                EraseObjProperty(member, "dados_convite");
                guild_obj.removemember(member);
            endif
            sleepms(2);
        endforeach
    endif
    LogInfo("RemFaccao_v3.1", "--> ETAPA 6 CONCLUÍDA.");


    // ETAPA 7: Remover todas as relações de inimizade
    LogInfo("RemFaccao_v3.1", "--> ETAPA 7: Removendo relações de inimizade...");
    var enemy_guilds_copy := guild_obj.enemyguilds;
    if (enemy_guilds_copy)
        foreach enemy in enemy_guilds_copy
            if (enemy)
                guild_obj.removeenemyguild(enemy);
            endif
            sleepms(2);
        endforeach
    endif
    LogInfo("RemFaccao_v3.1", "--> ETAPA 7 CONCLUÍDA.");

    
    // ETAPA 8: Limpar todas as CProps customizadas da própria guilda
    LogInfo("RemFaccao_v3.1", "--> ETAPA 8: Limpando CProps da guilda...");
    var props_to_clean := array{
        "name", "leader", "npcs_active", "npcs", "report", "area", 
        "money", "uniforms", "link_to", "subguilds", "laws", "territories",
        "recursos", "recursos_timestamp", "culture", "caravan_contracts"
        // A CProp "FeudoOwnedConstructions" já foi limpa na Etapa 5.
    };
    foreach prop in props_to_clean
        guild_obj.EraseProp(prop);
        sleepms(2);
    endforeach
    LogInfo("RemFaccao_v3.1", "--> ETAPA 8 CONCLUÍDA.");


    // ETAPA 9: Limpeza de Datafiles indexados
    LogInfo("RemFaccao_v3.1", "--> ETAPA 9: Limpando datafiles indexados...");
    if (guild_name && guild_name != "Facção Desconhecida")
        var elem_name_recursos := NormalizarNomeElementoFaccao(guild_name); 
        if (elem_name_recursos != error)
            DeleteDatafileEntry(KINGDOM_RESOURCE_DATAFILE, elem_name_recursos);
        endif

        DeleteDatafileEntry(":death:guild_xp_stats", guild_name);
        
        var registry_df := SafeOpenDataFile(":faccao:guild_registry", 1);
        if(registry_df)
            var elem_reg := SafeFindElement(registry_df, "guilds", 1);
            if(elem_reg)
                var registry := GetElementProp(elem_reg, "registry", dictionary{});
                if(registry.exists(guild_name))
                    registry.erase(guild_name);
                    SetElementProp(elem_reg, "registry", registry);
                endif
            endif
        endif
    endif
    LogInfo("RemFaccao_v3.1", "--> ETAPA 9 CONCLUÍDA.");


    // ETAPA 10: Destruir o objeto da guilda.
    LogInfo("RemFaccao_v3.1", "--> ETAPA 10: Destruindo objeto da guilda...");
    var destruction_result := DestroyGuild(guild_obj);
   
    if (!destruction_result)
        LogError("RemFaccao_v3.1", "    FALHA ao destruir a guilda " + guild_name);
    else
        LogInfo("RemFaccao_v3.1", "    ...Guilda destruída com sucesso.");
    endif
    
    LogCritical("RemFaccao_v3.1", "REMOÇÃO COMPLETA FINALIZADA para '" + guild_name + "'.");
    LogCritical("RemFaccao_v3.1", "==========================================================");
    
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