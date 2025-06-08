// $Id: architect.inc,v 1.10 2006/07/19 06:35:18 bytehawk Exp $

use uo;
use datafile;
use util;
include "include/string";
include ":charactercreation:habilidades";
include ":faccao:include/shared_functions";

/*
HILITE_MARK       This is the color that marked parts will be hilited with.
                  Since I'm colorblind, the default might hurt "normal" peoples eyes :)
LANGUAGE          Two languages are available atm: "ENG" and "GER". Should you
                  translate the entries in :architect:config/language.cfg to another
                  language, I'd ask you to email me the updated language.cfg for
                  including it in an updated version of Sigena architect.
MAXSAVES          The max number of designs each player can save. A high number would
                  overflow the gump and you'd have to script a pagination. So better
                  keep it below 20 or so :)
SEEPLAYERDESIGNS  staff with a least this level can load player designs
                  default: 3 (GM)
SITESIZE          The x and y size of the construction site.

startloc          The upper left corner of the construction site. The lower right corner
                  is calculated by adding SITESIZE to these coords.
websites          The URIs of wiki entries in the available languages. Again, should you
                  add an entry in a new language, please email me.
*/

const ARCHITECT_VERSION:= "1.11";
const CALLFROMCITY:= 0;           // call Architect from within cities only
const HILITE_MARK:= 42;           // color for highlighting marked parts
const LANGUAGE:= "PT";           // guess what :)  (DEU or ENG)
const MAXSAVES:= 10;              // maximum number of saved designs per player
const SEEPLAYERDESIGNS:= 5;       // cmdlevel needed to see player designs
const SITESIZE:= 30;              // height and width of construction site
const STAFFMODELEVEL:= 1;         // min level for staff mode

var startloc:= array{1510, 1242, 0};
var websites:= dictionary {"DEU" -> "http://www.uosigena.de/wiki/index.php/architect",
                           "ENG" -> "http://www.uosigena.de/wiki/index.php/architect_english"};

function GetResidenciasRegion(who)
  SysLog("GetResidenciasRegion: Iniciando busca para " + who.name + " em " + who.x + "," + who.y + " realm:" + who.realm);

  var df := opendatafile(":architect:areas");
  if (!df)
    SysLog("GetResidenciasRegion: ERRO - Nao foi possivel abrir datafile");
    return "Whole_World";
  endif
  
  SysLog("GetResidenciasRegion: Datafile aberto, verificando " + len(df.Keys()) + " territorios");
  
  foreach elem in (df.Keys())
    SysLog("GetResidenciasRegion: Verificando territorio '" + elem + "'");
    var region := df.FindElement(elem);
    if (!region)
      SysLog("GetResidenciasRegion: ERRO - Nao foi possivel encontrar regiao " + elem);
      continue;
    endif
    
    var rect := region.getProp("Rect");
    SysLog("GetResidenciasRegion: Territorio " + elem + " - Rect: '" + rect + "'");
    
    if (AT_InRect(who, SplitWords(rect)))
      SysLog("GetResidenciasRegion: ENCONTRADO! Jogador esta no territorio: " + elem);
      return elem;
    endif
  endforeach
  
  SysLog("GetResidenciasRegion: Nenhum territorio encontrado, retornando Whole_World");
  return "Whole_World";

endfunction

function AT_InRect(who, rect)
  SysLog("AT_InRect: Verificando coordenadas");
  SysLog("AT_InRect: rect array tem " + len(rect) + " elementos: " + rect);
  
  // Verificar se temos elementos suficientes
  if (len(rect) < 4)
    SysLog("AT_InRect: ERRO - Array rect tem menos de 4 elementos necessarios");
    return 0;
  endif
  
	var x1 := CInt(rect[1]);
	var y1 := CInt(rect[2]);
	var x2 := CInt(rect[3]);
	var y2 := CInt(rect[4]);
  
  // Verificar se o realm está presente no array (índice 5)
  var realm := "";
  if (len(rect) >= 5)
    realm := rect[5];
  else
    // Se não tem realm especificado, assumir que é o mesmo realm do jogador
    realm := who.realm;
    SysLog("AT_InRect: Realm nao especificado no territorio, assumindo realm do jogador: '" + realm + "'");
  endif

  SysLog("AT_InRect: Coordenadas parseadas - x1:" + x1 + " y1:" + y1 + " x2:" + x2 + " y2:" + y2 + " realm:'" + realm + "'");
  SysLog("AT_InRect: Jogador - x:" + who.x + " y:" + who.y + " realm:'" + who.realm + "'");
  
  // Normalize realm names to handle variations like "britannia" vs "sbritannia"
  var normalized_realm := Lower(realm);
  var normalized_who_realm := Lower(who.realm);
  
  // Handle common realm name variations
  if (normalized_realm == "sbritannia")
    normalized_realm := "britannia";
  endif
  if (normalized_who_realm == "sbritannia")
    normalized_who_realm := "britannia";
  endif
  
  SysLog("AT_InRect: Realms normalizados - territorio:'" + normalized_realm + "' jogador:'" + normalized_who_realm + "'");
  
  var realm_match := (normalized_who_realm == normalized_realm);
  var x_in_bounds := (CInt(who.x) >= x1 && CInt(who.x) <= x2);
  var y_in_bounds := (CInt(who.y) >= y1 && CInt(who.y) <= y2);
  
  SysLog("AT_InRect: realm_match:" + realm_match + " x_in_bounds:" + x_in_bounds + " y_in_bounds:" + y_in_bounds);
  
  var result := (realm_match && x_in_bounds && y_in_bounds);
  SysLog("AT_InRect: Resultado final: " + result);

	return result;
endfunction

function OpenAreasDataFile(create := 0)
    var df := OpenDataFile(":architect:areas");
    if (!df && create)
        df := CreateDataFile(":architect:areas");
    endif
    return df;
endfunction

function GetTerritoryInfo(territory_name)
    var df := OpenAreasDataFile();
    if (!df)
        return 0;
    endif
    
    return df.FindElement(territory_name);
endfunction

function IsInTerritory(mobile, territory_name)
    var territory := GetTerritoryInfo(territory_name);
    if (!territory)
        return 0;
    endif
    
    var rect := SplitWords(territory.GetProp("Rect"));
    return IsInRect(mobile, rect, territory.GetProp("Realm"));
endfunction

//
// Architect Privilege Configuration
//

function LoadArchitectConfig()
    var cfg := ReadConfigFile(":architect:config/privileges");
    if (!cfg)
        SysLog("VerifyArchitect: AVISO - Arquivo de configuracao privileges.cfg nao encontrado, usando valores padrao");
        return struct{
            "max_architects" := 5,
            "staff_bypass_level" := 1,
            "enable_logging" := 1,
            "enforce_limits" := 1,
            "default_privilege" := 0,
            "require_faction" := 1
        };
    endif
    
    return struct{
        "max_architects" := GetConfigInt(cfg["Settings"], "MaxArchitectsPerFaction") ?: 5,
        "staff_bypass_level" := GetConfigInt(cfg["Settings"], "StaffBypassLevel") ?: 1,
        "enable_logging" := GetConfigInt(cfg["Settings"], "EnablePrivilegeLogging") ?: 1,
        "enforce_limits" := GetConfigInt(cfg["Settings"], "EnforceLimits") ?: 1,
        "default_privilege" := GetConfigInt(cfg["Settings"], "DefaultArchitectPrivilege") ?: 0,
        "require_faction" := GetConfigInt(cfg["Settings"], "RequireFactionMembership") ?: 1
    };
endfunction

//
// Architect Privilege Verification Functions
//

function VerifyArchitect(who)
    // Check if player has architect privilege in their faction
    var config := LoadArchitectConfig();
    
    if (config.enable_logging)
        SysLog("VerifyArchitect: Verificando privilegios de arquiteto para " + who.name + " (Serial: " + who.serial + ")");
    endif
    
    // Staff members always have architect access
    if (who.cmdlevel >= config.staff_bypass_level)
        if (config.enable_logging)
            SysLog("VerifyArchitect: Acesso concedido - usuario e staff (level " + who.cmdlevel + ")");
        endif
        return 1;
    endif
    
    // Check if player has the Arquiteto skill
    if (!TemHabilidade(who, "Arquiteto"))
        if (config.enable_logging)
            SysLog("VerifyArchitect: Acesso negado - jogador nao tem habilidade de Arquiteto");
        endif
        return 0;
    endif
    
    // Get player's faction
    var player_faction := GetPlayerFaction(who);
    if (!player_faction && config.require_faction)
        if (config.enable_logging)
            SysLog("VerifyArchitect: Acesso negado - jogador nao pertence a uma faccao");
        endif
        return 0;
    endif
    
    // If faction is not required and player has no faction, allow access
    if (!player_faction && !config.require_faction)
        if (config.enable_logging)
            SysLog("VerifyArchitect: Acesso concedido - faccao nao e obrigatoria e jogador tem skill");
        endif
        return 1;
    endif
      // Get player's guild (faction)
    var guild := who.guild;
    if (!guild)
        if (config.enable_logging)
            SysLog("VerifyArchitect: Acesso negado - guild nao encontrada");
        endif
        return 0;
    endif
      // Check if player is the guild leader - leaders always have architect privileges
    
    // First check faction-style leadership (custom system)
    var guild_leader := guild.GetProp("leader");
    if (guild_leader == who.serial)
        if (config.enable_logging)
            SysLog("VerifyArchitect: Acesso concedido - jogador e lider da faccao " + player_faction);
        endif
        return 1;
    endif
    
    // Then check POL's built-in guild leadership
    if (guild.leader == who.serial)
        if (config.enable_logging)
            SysLog("VerifyArchitect: Acesso concedido - jogador e lider da guild nativa " + player_faction);
        endif
        return 1;
    endif
    
    // Get player's privileges in the faction
    var member_privs := guild.GetProp(cstr(who.serial));
    if (!member_privs)
        if (config.enable_logging)
            SysLog("VerifyArchitect: Acesso negado - privilegios do membro nao encontrados");
        endif
        return 0;
    endif
    
    // Check if player has architect privilege
    if (member_privs.architect)
        if (config.enable_logging)
            SysLog("VerifyArchitect: Acesso concedido - jogador tem privilegio de arquiteto na faccao " + player_faction);
        endif
        return 1;
    else
        if (config.enable_logging)
            SysLog("VerifyArchitect: Acesso negado - jogador nao tem privilegio de arquiteto na faccao " + player_faction);
        endif
        return 0;
    endif
endfunction

function GetFactionArchitects(faction_name)
    // Get list of all architects in a faction
    var config := LoadArchitectConfig();
    
    if (config.enable_logging)
        SysLog("GetFactionArchitects: Buscando arquitetos da faccao " + faction_name);
    endif
    
    var guild_id := FindGuildByName(faction_name);
    if (!guild_id)
        if (config.enable_logging)
            SysLog("GetFactionArchitects: Faccao nao encontrada: " + faction_name);
        endif
        return array{};
    endif
    
    var guild := FindGuild(guild_id);
    if (!guild)
        if (config.enable_logging)
            SysLog("GetFactionArchitects: Objeto da guild nao encontrado para ID: " + guild_id);
        endif
        return array{};
    endif
    
    var architects := array{};
    var member_list := guild.members;
    
    foreach member in member_list
        var member_privs := guild.GetProp(cstr(member.serial));
        if (member_privs && member_privs.architect)
            architects.append(struct{
                "serial" := member.serial,
                "name" := member.name,
                "account" := member.acctname
            });
            if (config.enable_logging)
                SysLog("GetFactionArchitects: Arquiteto encontrado - " + member.name + " (" + member.serial + ")");
            endif
        endif
    endforeach
    
    if (config.enable_logging)
        SysLog("GetFactionArchitects: Total de arquitetos encontrados na faccao " + faction_name + ": " + len(architects));
    endif
    return architects;
endfunction

function CountFactionArchitects(faction_name)
    // Count the number of architects in a faction
    var architects := GetFactionArchitects(faction_name);
    var count := len(architects);
    var config := LoadArchitectConfig();
    
    if (config.enable_logging)
        SysLog("CountFactionArchitects: Faccao " + faction_name + " tem " + count + " arquitetos");
    endif
    return count;
endfunction

function CheckArchitectLimit(faction_name, max_architects := 0)
    // Check if faction has reached the architect limit
    var config := LoadArchitectConfig();
    
    // Use config value if no limit specified
    if (max_architects == 0)
        max_architects := config.max_architects;
    endif
    
    // If limits are disabled, always return true
    if (!config.enforce_limits)
        return 1;
    endif
    
    var current_count := CountFactionArchitects(faction_name);
    if (config.enable_logging)
        SysLog("CheckArchitectLimit: Faccao " + faction_name + " tem " + current_count + "/" + max_architects + " arquitetos");
    endif
    return current_count < max_architects;
endfunction

function GetArchitectInfo(who)
    // Get detailed architect information for a player
    var config := LoadArchitectConfig();
    
    var info := struct{
        "has_skill" := TemHabilidade(who, "Arquiteto"),
        "faction" := GetPlayerFaction(who),
        "has_privilege" := 0,
        "is_staff" := (who.cmdlevel >= config.staff_bypass_level),
        "is_leader" := 0,
        "config" := config
    };
      if (info.faction)
        var guild := who.guild;
        if (guild)
            // Check if player is guild leader (faction-style or POL built-in)
            var guild_leader := guild.GetProp("leader");
            if (guild_leader == who.serial || guild.leader == who.serial)
                info.is_leader := 1;
                info.has_privilege := 1; // Leaders always have architect privilege
            else
                var member_privs := guild.GetProp(cstr(who.serial));
                if (member_privs)
                    info.has_privilege := member_privs.architect;
                endif
            endif
        endif
    endif
    
    return info;
endfunction