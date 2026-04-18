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

function _EnsureGuild(guild)
    if (typeof(guild) == "String") return FindGuild(guild); endif
    return guild;
endfunction

function _IsVendor(template)
    return (lower(template.name)["vendedor"] != error || lower(template.template)["merchant"] != error);
endfunction

function AddNpc(guild, npc)
   guild := _EnsureGuild(guild);
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
   guild := _EnsureGuild(guild);
   if (!guild) return array{}; endif

   var npcs := guild.GetProp("npcs");
   if (!npcs) npcs := array{}; endif

   // Validação: Se houver mais de 4 NPCs, são dados corrompidos
   if (npcs.size() > 4)
      npcs := array{
         struct{ "template" := ":epicai:guardafaccaofraco",   "price" := 25,  "name" := "Guarda Recruta (Fraco)",      "equipment" := array{} },
         struct{ "template" := ":epicai:guardafaccaomedio",   "price" := 70,  "name" := "Guarda Veterano (Medio)",     "equipment" := array{} },
         struct{ "template" := ":epicai:guardafaccaoforte",   "price" := 100, "name" := "Guarda Elite (Forte)",         "equipment" := array{} },
         struct{ "template" := ":merchants:merchantfaccao",   "price" := 10,  "name" := "Vendedor da Faccao",           "equipment" := array{} }
      };
      guild.SetProp("npcs", npcs);
   endif

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
   guild := _EnsureGuild(guild);
   if (typeof(index) == "Integer")
      var npc_list := listNPC(guild);
      npc_list.Erase(index);
      guild.setProp("npcs", npc_list);
   endif
endfunction

function listContractedNPC(guild)
   guild := _EnsureGuild(guild);
   if (!guild) return array{}; endif

   var npcs_active := guild.GetProp("npcs_active");
   if (!npcs_active) npcs_active := array{}; endif
   return npcs_active;
endfunction

function DismissNPC(guild, npc_i, npc := 0, who := 0)
   var npcs := listContractedNPC(guild);

   if (!npc || !npc.isA(POLCLASS_MOBILE))
      npc := SystemFindObjectBySerial(npcs[npc_i].serial);
   endif

   npcs.Erase(npc_i);
   guild.setProp("npcs_active", npcs);

   if (npc)
      npc.Disable("invul");
      RevokePrivilege(npc, "invul");
      SetObjProperty(npc, "GuardKill", 1);
      SetObjProperty(npc, "NoCorpse", 1);
      MoveObjectToLocation(npc, 6084, 1, 1, npc.realm, MOVEOBJECT_FORCELOCATION);
      ApplyRawDamage(npc, 8000);
   endif
endfunction

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

function _ApplyItem(agent, item_id, color := 0)
    var item := CreateItemInBackpack(agent, item_id, 1);
    if (item)
        if (color) item.color := color; endif
        if (!EquipItem(agent, item))
            return 0;
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

        if (RandomInt(100) < chance)
            _ApplyItem(agent, objname, cor);
        endif
        sleepms(2);
    endforeach
    return 1;
endfunction

function _SelectRaceOrFallback(agent, who)
    var race_choice := FaccaoRaceGump(who);
    ApplyRaceToNPC(agent, race_choice ? race_choice : FACCAO_BTN_HUMANO_M);
    return 1;
endfunction

// ============================================================================
// CONTRATAÇÃO (FLUXO PRINCIPAL)
// ============================================================================

function ContractNPC(guild, npc_i, who, loc, school := 0)
    guild := _EnsureGuild(guild);
    if (!guild) return 0; endif

    var template := listNpc(guild)[npc_i];
    if (!template) return 0; endif

    var agent := CreateNPCFromTemplate(template.template, loc.x, loc.y, loc.z, 0, who.realm);
    if (!agent) return 0; endif

    SetObjProperty(agent, "started", 1);
    SetObjProperty(agent, "ignore_make_race", 1);
    SetObjProperty(agent, "NoRandomAppearance", 1);
    if (school)
        SetObjProperty(agent, "SpellSchool", school);
    endif

    if (!agent.backpack)
        var bp := CreateItemAtLocation(agent.x, agent.y, agent.z, BACKPACK_OBJTYPE, 1, agent.realm);
        if (bp) EquipItem(agent, bp); endif
    endif

    // Troca imediatamente para o script correto antes do gump de raça,
    // para evitar que a brainAI (humanoid) ataque o contratante durante a espera.
    // Sem a propriedade "guild" definida ainda, o guard não inicia combate.
    if (!_IsVendor(template))
        agent.script := ":ghaia:guardfaccao";
        SetObjProperty(agent, "script", agent.script);
        RestartScript(agent);
    endif

    sleepms(500);

    _SelectRaceOrFallback(agent, who);

    if (!_IsVendor(template))
        var template_parts := SplitWords(template.template, ":");
        var gender_suffix := (agent.gender == 0) ? "M" : "F";
        _VestirDoGhaia(agent, template_parts[template_parts.size()] + gender_suffix);
    endif

    // Registra o NPC (define guild, contract_by, etc.) ANTES de reiniciar o script,
    // para que Setup() leia o guild correto.
    _RegisterActiveNPC(agent, template, guild, who, npc_i);

    // Reinicia o script do guarda para que Setup() leia o guild recem definido.
    // Vendedores nao reiniciam aqui pois o init.src rodaria duas vezes (sobrescreveria
    // aparencia escolhida). O main.src do vendedor re-le o guild no primeiro loop.
    if (!_IsVendor(template))
        RestartScript(agent);
    endif

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
