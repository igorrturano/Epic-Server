// ###########################################
// # npc.inc - Sistema de Facção
// ###########################################

include ":datafile:include/datafile";
include ":merchants:item_template";
include ":brainAI:include/npcInit";
include ":faccao:include/racegump";

// ===== CONSTANTS =====
const BACKPACK_OBJTYPE := 0xe75;

// ============================================================================
// GESTÃO DE FACÇÃO (FUNÇÕES DE BANCO DE DADOS)
// ============================================================================

function AddNpc(guild, npc)
   if (typeof(guild) == "String") guild := FindGuild(guild); endif
   if (!guild) return; endif

   var new_npc := struct{
      "template"   := npc.template,
      "equipment"  := npc.equipment,
      "bossStruct" := npc.bossStruct,
      "price"      := npc.price,
      "name"       := npc.name
   };

   if (!new_npc.template || !new_npc.price || !new_npc.name) return 0; endif

   var npclist := listNPC(guild);
   npclist.append(new_npc);
   guild.setProp("npcs", npclist);
   return 1;
endfunction

function listNPC(guild)
   if (typeof(guild) == "String") guild := FindGuild(guild); endif
   if (!guild) return array{}; endif

   var npcs := guild.GetProp("npcs");
   if (!npcs) npcs := array{}; endif

   // Migração/Upgrade de templates antigos
   var upgraded := 0;
   foreach template in (npcs)
      if (template.template == ":merchants:uosevendor")
         template.template := ":merchants:merchantfaccao";
         upgraded := 1;
      endif
   endforeach
   if (upgraded) guild.SetProp("npcs", npcs); endif

   return npcs;
endfunction

function remNPC(guild, index)
   if (typeof(guild) == "String") guild := FindGuild(guild); endif
   if (typeof(index) == "Integer")
      var npc_list := listNPC(guild);
      npc_list.Erase(index);
      guild.setProp("npcs", npc_list);
   endif
endfunction

function listContractedNPC(guild)
   if (typeof(guild) == "String") guild := FindGuild(guild); endif
   if (!guild) return array{}; endif

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
   
   // Move o NPC para o limbo antes de remover para evitar ganho de XP/RP por proximidade
   MoveObjectToLocation(npc, 6084, 1, 1, npc.realm, MOVEOBJECT_FORCELOCATION);
   ApplyRawDamage(npc, 8000);
endfunction

// ============================================================================
// INTERNAL HELPERS (DRY / CLEAN CODE / BUG FIXES)
// ============================================================================

function _ApplyItem(agent, item_id, color := 0)
    var item := CreateItemInBackpack(agent, item_id, 1);
    if (item)
        if (color) item.color := color; endif
        if (!EquipItem(agent, item))
            return 0; // Item ficou na mochila mas não vestiu
        endif
        return 1;
    endif
    return 0;
endfunction

function _VestirDoGhaia(agent, grupo)
    var cfg := ReadConfigFile(":ghaia:config/equip2");
    var elem := cfg[grupo];
    if (!elem) return 0; endif

    var lista := GetConfigStringArray(elem, "Equip");
    lista += GetConfigStringArray(elem, "Weapon");

    foreach linha in lista
        var div := splitwords(linha); 
        var objname := div[1];
        var cor     := CInt(div[2]);
        var chance  := CInt(div[3]);

        if (!chance) chance := 100; endif

        // FIX: Agora respeita a chance real do arquivo do Ghaia
        if (RandomInt(100) < chance)
            _ApplyItem(agent, objname, cor);
        endif
        sleepms(2);
    endforeach
    return 1;
endfunction

function _ForcaRoupaEmergencia(agent)
    var kit := array{0x13bf, 0x13c4, 0x13c3, 0x13be, 0x13bb, 0x1415, 0x1711}; 
    foreach id in kit
        _ApplyItem(agent, id);
    endforeach
endfunction

function _SelectRaceOrFallback(agent, who)
    var race_choice := FaccaoRaceGump(who);
    if (race_choice)
        ApplyRaceToNPC(agent, race_choice);
    else
        // Se o player deu ESC, força humano para não bugar o nome <random>
        ApplyRaceToNPC(agent, FACCAO_BTN_HUMANO_M); 
    endif
    return 1;
endfunction

// ============================================================================
// CONTRATAÇÃO (FLUXO PRINCIPAL)
// ============================================================================

function ContractNPC(guild, npc_i, who, loc, school := 0)
    if (typeof(guild) == "String") guild := FindGuild(guild); endif
    var template := listNpc(guild)[npc_i];
    if (!template) return 0; endif

    // 1. Cria o Mobile
    var agent := CreateNPCFromTemplate(template.template, loc.x, loc.y, loc.z, 0, who.realm);
    if (!agent) return 0; endif

    // 2. Trava IA (started=1 engana o KillPlayers.src/brain.ecl)
    SetObjProperty(agent, "started", 1);
    SetObjProperty(agent, "ignore_make_race", 1);
    SetObjProperty(agent, "NoRandomAppearance", 1);
    if (school)
        SetObjProperty(agent, "SpellSchool", school);
    endif

    // 3. Garanta Mochila
    if (!agent.backpack)
        var bp := CreateItemAtLocation(agent.x, agent.y, agent.z, BACKPACK_OBJTYPE, 1, agent.realm);
        EquipItem(agent, bp);
    endif

    sleepms(1000); // Espera estabilização

    // 4. Fluxo de Equipamento
    if (template.equipment.size() > 0)
        foreach item_ref in (template.equipment)
            EquipItem(agent, CreateItemInBackpackFromTemplate(agent, item_ref));
            sleepms(2);
        endforeach
    else
        // Extrai nome simples do template (KISS: remove leading :package:)
        var parts := SplitWords(template.template, ":");
        var base_name := parts.size() > 1 ? parts[parts.size()] : template.template;
        var gender_suffix := agent.gender ? "F" : "M";
        var config_group := base_name + gender_suffix;
        if (!_VestirDoGhaia(agent, config_group))
            NPC_SetupEquipment(agent, "Villager_1");
        endif
    endif

    // 5. Checagem de Nudez (KISS)
    if (ListEquippedItems(agent).size() < 2)
        _ForcaRoupaEmergencia(agent);
    endif

    // 6. Configurações de Raça e IA
    _SelectRaceOrFallback(agent, who);
    
    // Se não for vendedor, seta o script de guarda
    if (lower(template.name)["vendedor"] == error && lower(template.template)["merchant"] == error)
        agent.script := ":ghaia:guard";
        SetObjProperty(agent, "script", agent.script);
        RestartScript(agent);
    endif

    // 7. Registro na Facção
    _RegisterActiveNPC(agent, template, guild, who, npc_i);
    
    return 1;
endfunction

function _RegisterActiveNPC(agent, template, guild, who, npc_i)
    SetObjProperty(agent, "guild", guild.guildid);
    SetObjProperty(agent, "contract_by", who.serial);
    SetObjProperty(agent, "conf_ref", npc_i);
    
    var npc_list := listContractedNPC(guild);
    npc_list.append(struct{ "serial" := agent.serial, "price" := template.price, "ref" := npc_i });
    guild.setProp("npcs_active", npc_list);
    
    PrintTextAbove(agent, "As suas ordens!");
endfunction