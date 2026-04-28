// =============================================================================
// npc.inc — Sistema de NPCs de faccao
// =============================================================================
//
// Gerencia todo o ciclo de vida dos NPCs contratados pelas faccoes: deteccao
// de tipo/tier, listagem por faccao, contratacao, registro e demissao.
//
// DETECCAO DE TIPO E TIER
//   _GetNPCScript(template)  — mapeia nome do template para o script correto
//                              (ex: "cazador" -> panteracazador, "ladrao" -> ladrao)
//   _GetNPCTier(name)        — detecta tier pelo sufixo do nome do NPC:
//                              sem sufixo=1 (fraco), "II"=2 (medio), "III"=3 (forte)
//
// LISTAGEM POR FACCAO
//   ListNPCs_Liga, ListNPCs_Regentes, ListNPCs_Sinodo, ListNPCs_Conselho
//   — retornam arrays com os templates disponiveis para contratacao por cada faccao.
//
// GERENCIAMENTO DE NPCs DA GUILD
//   AddNpc(guild, npc_serial)       — registra NPC na lista da guild
//   listNPC(guild)                  — lista NPCs ativos da guild
//   remNPC(guild, npc_serial)       — remove NPC da lista da guild
//   listContractedNPC(guild)        — lista NPCs com contrato ativo
//   DismissNPC(npc)                 — demite e remove o NPC do mundo
//
// CONTRATACAO
//   ContractNPC(guild, template, x, y, z, realm)
//   — cria o NPC na localizacao, aplica equipamentos (_VestirDoGhaia),
//     define raca, registra na guild (_RegisterActiveNPC) e retorna o serial.
//
// REGISTRO
//   _RegisterActiveNPC(guild, npc, contract_duration)
//   — define props de guild, contrato, tipo de faccao e inicia o script do NPC.
// =============================================================================

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

// _SetupNPCSkills foi movida para npc_laws_enforce.inc e integrada a SetupFaccaoNPCBase.

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
        struct{ "template" := ":epicai:guardafaccaoforte",  "price" := 100, "name" := "Guarda Elite" }
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
        struct{ "template" := ":epicai:guarda{}{}".format(e, fort_sfx), "price" := 100, "name" := "Mago Mestre ({})".format(escola) }
    };
endfunction

// Retorna lista de templates para Rufioes
function _ListNPCRufioes()
    return array{
        struct{ "template" := ":epicai:ladraorufiaofr",    "price" := 25,  "name" := "Ladrao Recruta" },
        struct{ "template" := ":epicai:ladraorufiaomed",   "price" := 70,  "name" := "Ladrao Furtivo" },
        struct{ "template" := ":epicai:ladraorufiafort",   "price" := 100, "name" := "Ladrao Sombrio" },
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

   var staff_tipo  := guild.GetProp("staff_tipo");
   var tipo_faccao := GetFaccaoTipo(guild);

   // Faccoes de staff retornam lista canonica por tipo (imutavel, nao salva no guild)
   if (staff_tipo && staff_tipo != error)
      var lista := _ListNPCByStaffTipo(staff_tipo);
      if (lista.size() > 0) return lista; endif
   endif

   // Faccao de staff sem staff_tipo reconhecido nao tem NPCs contratáveis
   if (tipo_faccao == "staff")
      return array{};
   endif

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
    SetObjProperty(agent, "guild",        guild.guildid);
    SetObjProperty(agent, "contract_by",  who.serial);
    SetObjProperty(agent, "conf_ref",     npc_i);

    // Propaga o tipo da facção ao NPC para que sistemas externos
    // (architect, leis, etc.) não precisem consultar a guild.
    var tipo := guild.GetProp("tipo_faccao");
    if (!tipo) tipo := "player"; endif
    SetObjProperty(agent, "tipo_faccao", tipo);

    PropagateNomeFaccao(guild, agent);

    var npc_list := listContractedNPC(guild);
    npc_list.append(struct{ "serial" := agent.serial, "price" := template.price, "ref" := npc_i });
    guild.setProp("npcs_active", npc_list);

    PrintTextAbove(agent, "As suas ordens!");
endfunction
