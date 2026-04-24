// ###########################################
// # npc.inc - Sistema de Facção
// ###########################################

include ":datafile:include/datafile";
include ":merchants:item_template";
include ":brainAI:include/npcInit";
include ":faccao:include/racegump";
include ":attributes:attributes";

// ===== CONSTANTS =====
const BACKPACK_OBJTYPE := 0xe75;

// ============================================================================
// MAPEAMENTO DE SCRIPTS E SKILLS POR TIPO DE NPC
// ============================================================================

// Retorna o script correto para o template dado.
// Reconhece ladrao/assassino pela substring no nome; default = guardfaccao.
function _GetNPCScript(tname)
    var t := lower(tname);
    if (t["ladraoruf"] != error)       return ":ghaia:ladraorufiao";     endif
    if (t["assassinosinodo"] != error) return ":ghaia:assassinosinodo";  endif
    if (t["assassinoarqueiro"] != error) return ":ghaia:assassinoarqueiro"; endif
    if (t["assassinocazador"] != error)  return ":ghaia:assassinocazador";  endif
    return ":ghaia:guardfaccao";
endfunction

// Detecta tier (1/2/3) pelo sufixo do nome do template.
// Tier 1 = sufixo "fraco", "fr" (ex: guardakaijinfraco, ladraorufiaofr)
// Tier 2 = sufixo "medio", "med"
// Tier 3 = sufixo "forte", "fort", "ft" (default)
function _GetNPCTier(tname)
    var t := lower(tname);
    // Tier 1: contém "fraco" (ex: guardafaccaofraco) OU termina com "fr" (ex: ladraorufiaofr)
    if (t["fraco"] != error) return 1; endif
    // Para short-suffix "fr": garantir que nao seja confundido com "forte" (contem "for")
    if (t["fr"] != error && t["forte"] == error && t["medio"] == error) return 1; endif
    if (t["medio"] != error || t["med"] != error) return 2; endif
    return 3;
endfunction

// Configura skills que nao podem ser definidas no npcdesc.cfg via AP_SetTrueSkill.
// Chamado apos o NPC ser registrado na faccao.
function _SetupNPCSkills(agent, tname)
    var t    := lower(tname);
    var tier := _GetNPCTier(tname);

    // ----- CONSELHO SOFOCRATICO (guardamago, guardakaijin, guardashaman, etc.) -----
    if (t["guarda"] != error && (t["kaijin"] != error || t["shaman"] != error ||
        t["aradal"] != error || t["velkyn"] != error || t["evocatium"] != error))
        var mgr  := array{ 0, 40, 65, 90 }[tier];
        var tact := array{ 0, 30, 50, 70 }[tier];
        var wres := array{ 0, 25, 40, 55 }[tier];
        var oh   := array{ 0, 20, 30, 40 }[tier];
        var theo := array{ 0, 10, 20, 30 }[tier];
        AP_SetTrueSkill(agent, MAGERY,    mgr);
        AP_SetTrueSkill(agent, ARCANA,    mgr);
        AP_SetTrueSkill(agent, TACTICS,   tact);
        AP_SetTrueSkill(agent, WRESTLING, wres);
        AP_SetTrueSkill(agent, THEOLOGY,  theo);
        AP_SetTrueSkill(agent, ONEHANDED, oh);
        return;
    endif

    // ----- REGENTES (guardafaccao) -----
    if (t["guardafaccao"] != error)
        var t_val  := array{ 0, 40, 60, 80 }[tier];
        var oh_val := array{ 0, 50, 65, 80 }[tier];
        var tw_val := array{ 0, 30, 50, 70 }[tier];
        var dw_val := array{ 0, 15, 35, 55 }[tier];
        var wr_val := array{ 0, 25, 45, 65 }[tier];
        var rn_val := array{ 0,  0, 15, 30 }[tier];
        AP_SetTrueSkill(agent, TACTICS,    t_val);
        AP_SetTrueSkill(agent, ONEHANDED,  oh_val);
        AP_SetTrueSkill(agent, TWOHANDED,  tw_val);
        AP_SetTrueSkill(agent, DUALWIELD,  dw_val);
        AP_SetTrueSkill(agent, WRESTLING,  wr_val);
        AP_SetTrueSkill(agent, RANGED,     rn_val);
        return;
    endif

    // ----- RUFIOES (ladraorufiao) -----
    if (t["ladraoruf"] != error)
        var pp_val := array{ 0, 50, 70, 90 }[tier];
        var t2_val := array{ 0, 40, 55, 70 }[tier];
        var oh2    := array{ 0, 50, 65, 80 }[tier];
        var dw2    := array{ 0, 20, 40, 60 }[tier];
        var wr2    := array{ 0, 20, 30, 40 }[tier];
        AP_SetTrueSkill(agent, PICKPOCKETING, pp_val);
        AP_SetTrueSkill(agent, TACTICS,       t2_val);
        AP_SetTrueSkill(agent, ONEHANDED,     oh2);
        AP_SetTrueSkill(agent, DUALWIELD,     dw2);
        AP_SetTrueSkill(agent, WRESTLING,     wr2);
        return;
    endif

    // ----- SINODO MELEE (assassinosinodo) -----
    if (t["assassinosinodo"] != error)
        var ta3 := array{ 0, 50, 65, 80 }[tier];
        var oh3 := array{ 0, 55, 70, 85 }[tier];
        var dw3 := array{ 0, 30, 45, 60 }[tier];
        var wr3 := array{ 0, 20, 35, 50 }[tier];
        var tw3 := array{ 0, 20, 30, 40 }[tier];
        AP_SetTrueSkill(agent, TACTICS,   ta3);
        AP_SetTrueSkill(agent, ONEHANDED, oh3);
        AP_SetTrueSkill(agent, DUALWIELD, dw3);
        AP_SetTrueSkill(agent, WRESTLING, wr3);
        AP_SetTrueSkill(agent, TWOHANDED, tw3);
        return;
    endif

    // ----- SINODO ARQUEIRO (assassinoarqueiro) -----
    if (t["assassinoarqueiro"] != error)
        var rn4 := array{ 0, 55, 70, 85 }[tier];
        var ta4 := array{ 0, 45, 60, 75 }[tier];
        var dw4 := array{ 0, 20, 35, 50 }[tier];
        var oh4 := array{ 0, 20, 30, 40 }[tier];
        var wr4 := array{ 0, 15, 25, 35 }[tier];
        var tw4 := array{ 0, 15, 25, 35 }[tier];
        AP_SetTrueSkill(agent, RANGED,    rn4);
        AP_SetTrueSkill(agent, TACTICS,   ta4);
        AP_SetTrueSkill(agent, DUALWIELD, dw4);
        AP_SetTrueSkill(agent, ONEHANDED, oh4);
        AP_SetTrueSkill(agent, WRESTLING, wr4);
        AP_SetTrueSkill(agent, TWOHANDED, tw4);
        return;
    endif

    // ----- SINODO CAZADOR (assassinocazador — melee + pantera) -----
    if (t["assassinocazador"] != error)
        var ta5 := array{ 0, 50, 65, 80 }[tier];
        var oh5 := array{ 0, 55, 70, 85 }[tier];
        var dw5 := array{ 0, 25, 40, 55 }[tier];
        var wr5 := array{ 0, 20, 30, 45 }[tier];
        var tw5 := array{ 0, 15, 25, 35 }[tier];
        AP_SetTrueSkill(agent, TACTICS,   ta5);
        AP_SetTrueSkill(agent, ONEHANDED, oh5);
        AP_SetTrueSkill(agent, DUALWIELD, dw5);
        AP_SetTrueSkill(agent, WRESTLING, wr5);
        AP_SetTrueSkill(agent, TWOHANDED, tw5);
        return;
    endif
endfunction

// ============================================================================
// LISTAGEM DE NPCs DISPONIVEIS POR TIPO DE FACCAO
// ============================================================================

// Retorna lista de NPCs de servico da Liga do Pacto Comum
function _ListNPCLiga()
    return array{
        struct{ "template" := ":merchants:merchantfaccao",    "price" := 10, "name" := "Vendedor da Faccao"    },
        struct{ "template" := ":merchants:curandeirofaccao",  "price" := 10, "name" := "Curandeiro"            },
        struct{ "template" := ":merchants:gemvendorfaccao",   "price" := 10, "name" := "Comprador de Gemas"    },
        struct{ "template" := ":merchants:talkerfaccao",      "price" := 10, "name" := "Arauto"                },
        struct{ "template" := ":merchants:rumorfaccao",       "price" := 10, "name" := "Arauto de Rumores"     },
        struct{ "template" := ":merchants:bibliotecariofaccao","price":= 10, "name" := "Bibliotecario"         },
        struct{ "template" := ":merchants:stablemasterfaccao", "price":= 10, "name" := "Tratador de Animais"   }
    };
endfunction

// Despacha para a lista correta baseado no staff_tipo da guild
function _ListNPCByStaffTipo(staff_tipo)
    var t := lower(cstr(staff_tipo));
    case (t)
        "regentes":  return _ListNPCRegentes();
        "liga":      return _ListNPCLiga();
        "rufioes":   return _ListNPCRufioes();
        "sinodo":    return _ListNPCSinodo();
    endcase
    // Conselho: pode ter escola como sufixo (ex: "conselho_kaijin")
    if (t["conselho"] != error)
        var escola := "kaijin"; // default
        foreach e in array{ "kaijin", "shaman", "aradalore", "velkyn", "evocatium" }
            sleepms(2);
            if (t[e] != error) escola := e; break; endif
        endforeach
        return _ListNPCConselho(escola);
    endif
    return array{};
endfunction

// Retorna lista de templates disponíveis para Regentes (guardas guerreiros)
function _ListNPCRegentes()
    return array{
        struct{ "template" := ":epicai:guardafaccaofraco",  "price" := 25,  "name" := "Guarda Recruta" },
        struct{ "template" := ":epicai:guardafaccaomedio",  "price" := 70,  "name" := "Guarda Veterano" },
        struct{ "template" := ":epicai:guardafaccaoforte",  "price" := 100, "name" := "Guarda Elite" },
        struct{ "template" := ":merchants:merchantfaccao",  "price" := 10,  "name" := "Vendedor da Faccao" }
    };
endfunction

// Retorna lista de templates para Conselho Sofocratico (escola passada como parametro).
// Escola: "kaijin", "shaman", "aradalore", "velkyn", "evocatium"
// Para aradalore e evocatium os sufixos de tier sao abreviados (fr/med/ft).
function _ListNPCConselho(escola := "kaijin")
    var e := lower(escola);
    var fr_sfx   := "fraco";
    var med_sfx  := "medio";
    var fort_sfx := "forte";
    if (e == "aradalore")
        fr_sfx   := "fr";
        med_sfx  := "med";
        fort_sfx := "ft";
    elseif (e == "evocatium")
        fr_sfx   := "fr";
        med_sfx  := "med";
        fort_sfx := "ft";
    endif
    return array{
        struct{ "template" := ":epicai:guarda{}{}".format(e, fr_sfx),   "price" := 25,  "name" := "Mago Iniciante ({})".format(escola) },
        struct{ "template" := ":epicai:guarda{}{}".format(e, med_sfx),  "price" := 70,  "name" := "Mago Arcano ({})".format(escola) },
        struct{ "template" := ":epicai:guarda{}{}".format(e, fort_sfx), "price" := 100, "name" := "Mago Mestre ({})".format(escola) },
        struct{ "template" := ":merchants:merchantfaccao",               "price" := 10,  "name" := "Vendedor da Faccao" }
    };
endfunction

// Retorna lista de templates para Rufioes
function _ListNPCRufioes()
    return array{
        struct{ "template" := ":epicai:ladraorufiaofr",    "price" := 25,  "name" := "Ladrao Recruta" },
        struct{ "template" := ":epicai:ladraorufiaomed",   "price" := 70,  "name" := "Ladrao Furtivo" },
        struct{ "template" := ":epicai:ladraorufiafort",   "price" := 100, "name" := "Ladrao Sombrio" },
        struct{ "template" := ":merchants:merchantfaccao", "price" := 10,  "name" := "Vendedor da Faccao" },
        struct{ "template" := ":ligafaccao:mercadonegrofaccao", "price" := 50, "name" := "Mercado Negro" }
    };
endfunction

// Retorna lista de templates para Sinodo (melee + arqueiro)
function _ListNPCSinodo()
    return array{
        struct{ "template" := ":epicai:assassinosinodofr",    "price" := 25,  "name" := "Assassino Novato" },
        struct{ "template" := ":epicai:assassinosinodomed",   "price" := 70,  "name" := "Assassino Habilidoso" },
        struct{ "template" := ":epicai:assassinosinodofort",  "price" := 100, "name" := "Assassino Mestre" },
        struct{ "template" := ":epicai:assassinoarqueirofr",  "price" := 25,  "name" := "Arqueiro Sombra (Fraco)" },
        struct{ "template" := ":epicai:assassinoarqueiromed", "price" := 70,  "name" := "Arqueiro Sombra (Medio)" },
        struct{ "template" := ":epicai:assassinoarqueirofort","price" := 100, "name" := "Arqueiro Sombra (Forte)" },
        struct{ "template" := ":epicai:assassinocazadorfr",   "price" := 35,  "name" := "Cazador Novato" },
        struct{ "template" := ":epicai:assassinocazadormed",  "price" := 80,  "name" := "Cazador Habilidoso" },
        struct{ "template" := ":epicai:assassinocazadorfort", "price" := 120, "name" := "Cazador Mestre" }
    };
endfunction

// ============================================================================
// GESTÃO DE FACÇÃO (FUNÇÕES DE BANCO DE DADOS)
// ============================================================================

function _EnsureGuild(guild)
    if (typeof(guild) == "String") return FindGuild(guild); endif
    return guild;
endfunction

function _IsVendor(template)
    var t := lower(template.template);
    var n := lower(template.name);
    // Vendedores clássicos
    if (n["vendedor"] != error || t["merchant"] != error) return 1; endif
    // NPCs de serviço da Liga (ligafaccao package)
    if (t["curandeiro"] != error || t["gemvendor"] != error || t["talker"] != error)  return 1; endif
    if (t["rumor"] != error || t["bibliotecario"] != error)                           return 1; endif
    if (t["stablemaster"] != error)                                                   return 1; endif
    return 0;
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

   // Faccoes de staff retornam lista canonica por tipo (imutavel, nao salva no guild)
   var staff_tipo := guild.GetProp("staff_tipo");
   if (staff_tipo)
      var lista := _ListNPCByStaffTipo(staff_tipo);
      if (lista.size() > 0) return lista; endif
   endif

   var npcs := guild.GetProp("npcs");
   if (!npcs) npcs := array{}; endif

   // Validação: lista com 0 entradas é dados corrompidos; reinicializa com defaults.
   // Limite generoso (30) para suportar todos os tipos de NPC das faccoes.
   if (npcs.size() == 0 || npcs.size() > 30)
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

// Retorna o grupo de equip do equip2.cfg para o tipo de NPC dado.
// Mapeia templates novos para grupos existentes (evita duplicar entries no cfg).
function _GetEquipGroup(tname, gender)
    var t   := lower(tname);
    var sfx := (gender == 0) ? "M" : "F";

    // Conselho Sofocratico: usa Vampiro_MagoM (so existe M por ora)
    if (t["guarda"] != error && (t["kaijin"] != error || t["shaman"] != error ||
        t["aradal"] != error || t["velkyn"] != error || t["evocatium"] != error))
        return "Vampiro_MagoM";
    endif

    // Ladroes e Assassinos Sinodo: Vampiro_ladino
    if (t["ladraoruf"] != error || t["assassinosinodo"] != error || t["assassinocazador"] != error)
        return "Vampiro_ladino{}".format(sfx);
    endif

    // Assassino Arqueiro: grupo proprio (definido no equip2.cfg) ou fallback Vampiro_ladino
    if (t["assassinoarqueiro"] != error)
        return "assassinoarqueiro{}".format(sfx);
    endif

    // Guardas Regentes: grupo proprio por tier
    if (t["guardafaccaofraco"] != error)  return "guardafaccaofraco{}".format(sfx); endif
    if (t["guardafaccaomedio"] != error)  return "guardafaccaomedio{}".format(sfx); endif
    if (t["guardafaccaoforte"] != error)  return "guardafaccaoforte{}".format(sfx); endif

    // Fallback: monta grupo pelo nome do template (comportamento original)
    var parts := SplitWords(tname, ":");
    return parts[parts.size()] + sfx;
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
    if (!agent)
        return 0;
    endif

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
    var npc_script := _GetNPCScript(template.template);
    if (!_IsVendor(template))
        agent.script := npc_script;
        SetObjProperty(agent, "script", agent.script);
        RestartScript(agent);
    endif

    sleepms(500);

    _SelectRaceOrFallback(agent, who);

    if (!_IsVendor(template))
        var equip_group := _GetEquipGroup(template.template, agent.gender);
        _VestirDoGhaia(agent, equip_group);
    endif

    // Registra o NPC (define guild, contract_by, etc.) ANTES de reiniciar o script,
    // para que Setup() leia o guild correto.
    _RegisterActiveNPC(agent, template, guild, who, npc_i);

    // Configura skills que nao podem ser definidas no npcdesc.cfg
    _SetupNPCSkills(agent, template.template);

    // Reinicia o script do NPC para que Setup() leia o guild recem definido.
    // Vendedores nao reiniciam aqui pois o init.src rodaria duas vezes (sobrescreveria
    // aparencia escolhida). O main.src do vendedor re-le o guild no primeiro loop.
    if (!_IsVendor(template))
        agent.script := npc_script;
        RestartScript(agent);
    endif

    return 1;
endfunction

function _RegisterActiveNPC(agent, template, guild, who, npc_i)
    SetObjProperty(agent, "guild",        guild.guildid);
    SetObjProperty(agent, "contract_by",  who.serial);
    SetObjProperty(agent, "conf_ref",     npc_i);

    // Propaga o tipo da facção ao NPC para que sistemas externos
    // (architect, leis, etc.) não precisem consultar a guild.
    var tipo := guild.GetProp("tipo_faccao");
    if (!tipo) tipo := "player"; endif
    SetObjProperty(agent, "tipo_faccao", tipo);

    PropagateNomeFaccao(guild, agent);

    // NPCs de sombra (assassinos) têm o botão "Sombras" no gump; nascem visíveis
    var _tpl_name := lower(cstr(template.template));
    if (_tpl_name["assassino"] != error || _tpl_name["ladrao"] != error)
        SetObjProperty(agent, "is_shadow_npc", 1);
    endif

    var npc_list := listContractedNPC(guild);
    npc_list.append(struct{ "serial" := agent.serial, "price" := template.price, "ref" := npc_i });
    guild.setProp("npcs_active", npc_list);

    PrintTextAbove(agent, "As suas ordens!");
endfunction
