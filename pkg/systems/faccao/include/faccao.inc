use guilds;
use os;

include ":faccao:membership";

// ── Facções staff disponíveis (nomefaccao = índice 1..5) ─────────────────────
var FACCAO_NOMES := array{
    "Regentes de Valtheris",  // 1
    "Sinodo",                  // 2
    "Rufioes",                 // 3
    "Conselho Sofocratico",    // 4
    "A Liga do Pacto Comum"   // 5
};

// ── Rect de movimento de todos os NPCs de facção (guard e merchant) ───────────
// Defina aqui a área navegável do mapa. Edite os campos x1, y1, x2, y2.
var FACCAO_WALK_RECT := struct{ "x1":=1117, "y1":=1028, "x2":=1417, "y2":=1105 };

// Retorna 1 se o NPC está dentro do rect de movimento.
function IsNPCInWalkRect(npc)
    var r := FACCAO_WALK_RECT;
    return (npc.x >= r.x1 && npc.x <= r.x2 && npc.y >= r.y1 && npc.y <= r.y2);
endfunction

// Retorna o centro do rect de movimento.
function GetWalkRectCenter(z_ref := 0)
    var r := FACCAO_WALK_RECT;
    return struct{ "x" := (r.x1 + r.x2) / 2, "y" := (r.y1 + r.y2) / 2, "z" := z_ref };
endfunction

// Retorna um ponto aleatório dentro do rect de movimento.
function GetRandomWanderPoint()
    var r := FACCAO_WALK_RECT;
    return struct{
        "x" := r.x1 + RandomInt(r.x2 - r.x1 + 1),
        "y" := r.y1 + RandomInt(r.y2 - r.y1 + 1)
    };
endfunction

// Se o NPC saiu do walk rect, navega de volta ao centro. Retorna 1 se estava fora.
function HandleWalkRectBreach(me, byref wait_time)
    if (IsNPCInWalkRect(me)) return 0; endif
    var _c := GetWalkRectCenter(me.z);
    RunTowardLocation(_c.x, _c.y);
    wait_time := 0;
    return 1;
endfunction

// Copia a prop nomefaccao da guild para um objeto (NPC, item, monumento, etc.).
function PropagateNomeFaccao(guild, obj)
    var n := guild.GetProp("nomefaccao");
    if (n) SetObjProperty(obj, "nomefaccao", n); endif
endfunction

// Registra um item como pertencente a esta facção (mural, mesa, convite, etc.).
// Serial armazenado em guild prop "itens_faccao".
function RegisterFaccaoItem(guild, item)
    var lista := guild.GetProp("itens_faccao");
    if (!lista) lista := array{}; endif
    if (!(item.serial in lista))
        lista.append(item.serial);
        guild.SetProp("itens_faccao", lista);
    endif
endfunction

// Remove um item do registro da facção (chamar ao destruir o item).
function UnregisterFaccaoItem(guild, item_serial)
    var lista := guild.GetProp("itens_faccao");
    if (!lista) return; endif
    var nova := array{};
    foreach s in lista
        if (s != item_serial) nova.append(s); endif
        sleepms(2);
    endforeach
    guild.SetProp("itens_faccao", nova);
endfunction

// Retorna "staff" ou "player", normalizando dados legados (inteiro 1 = staff).
function GetFaccaoTipo(guild)
    var t := guild.GetProp("tipo_faccao");
    if (t == "staff" || (t && t != "player")) return "staff"; endif
    return "player";
endfunction

// Faz o parse de uma string "x1 y1 x2 y2" retornando struct com os campos nomeados.
// Retorna 0 se a string for inválida.
function ParseRect(rect_str)
    var c := SplitWords(rect_str);
    if (c.size() < 4) return 0; endif
    return struct{
        "x1" := CInt(c[1]), "y1" := CInt(c[2]),
        "x2" := CInt(c[3]), "y2" := CInt(c[4])
    };
endfunction

// Normaliza um retangulo garantindo x1<=x2 e y1<=y2.
// Aceita coordenadas em qualquer ordem (inclusive invertidas).
function NormalizeRect(x1, y1, x2, y2)
    if (x1 > x2) var t := x1; x1 := x2; x2 := t; endif
    if (y1 > y2) var t := y1; y1 := y2; y2 := t; endif
    return struct{ "x1" := x1, "y1" := y1, "x2" := x2, "y2" := y2 };
endfunction

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
   guild.setProp("name", name);
   guild.setProp("leader", 0);
   guild.setProp("npcs_active", array{});
   guild.setProp("npcs", array{});
   guild.setProp("report", array{"Faccao Criada"});
   guild.setprop("area", rect);
   guild.setprop("money", cint(money));
   guild.setprop("uniforms", array{});
   guild.setprop("link_to", 0);
   guild.setprop("tipo_faccao", "player"); // "player" ou "staff"

   return guild;
endfunction

function GetFaccao(guild)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif

   if (!guild) return; endif

   var faccao_obj := struct{
      "name"        := guild.GetProp("name"),
      "npcs"        := guild.GetProp("npcs"),
      "npcs_active" := guild.GetProp("npcs_active"),
      "laws"        := guild.GetProp("laws"),
      "leader"      := guild.GetProp("leader"),
      "area"        := guild.GetProp("area"),
      "uniforms"    := guild.GetProp("uniforms"),
      "subguilds"   := guild.GetProp("subguilds"),
      "money"       := guild.GetProp("money"),
      "tipo_faccao" := GetFaccaoTipo(guild)
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

   if (props.tipo_faccao != null) // "player" ou "staff"
      faccao.tipo_faccao := props.tipo_faccao;
   endif

   foreach key in (faccao.keys())
      guild.SetProp(key, faccao[key]);
      sleepms(2);
   endforeach

   return 1;
endfunction

function RemFaccao(guild_obj)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif

   if (!guild) return; endif

   foreach member in (guild_obj.members)
      guild_obj.removemember(member);
      sleepms(2);
   endforeach

   foreach g in (guild_obj.enemyguilds)
      guild_obj.removeenemyguild(g);
      sleepms(2);
   endforeach

    
    // Liberar todos os territórios
    var territories := guild_obj.GetProp("territories");
    if (territories)
        var rect_df := OpenDataFile(":architect:areas");
        foreach territory in territories
            var elem := rect_df.FindElement(territory);
            if (elem)
                elem.SetProp("Faction", "Selvagem");
                
                // Remover monumentos
                var rect := ParseRect(elem.GetProp("Rect"));
                if (rect)
                    var center_x := CInt((rect.x1 + rect.x2) / 2);
                    var center_y := CInt((rect.y1 + rect.y2) / 2);
                    
                    foreach item in ListItemsNearLocation(center_x, center_y, GetWorldHeight(center_x, center_y, elem.GetProp("Realm")), 3, elem.GetProp("Realm"))
                        if (item.objtype == 0xA104 && GetObjProperty(item, "RectName") == territory)
                            DestroyItem(item);
                            break;
                        endif
                    endforeach
                endif
            endif
            sleepms(2);
        endforeach
    endif
    
   return DestroyGuild(guild);
endfunction


// Valida se o mobile pode entrar nesta facção.
// Retorna 0 se permitido, ou string de erro se bloqueado.
function CanJoinFaccao(faccao, mobile)
   if (IsMemberOfFaccao(faccao, mobile))
      return "Você já é membro desta facção.";
   endif
   // Duas vagas já preenchidas — não aceita uma terceira
   var _hon_check := GetObjProperty(mobile, "honorary_guild_id");
   if (mobile.guild && _hon_check && FindGuild(_hon_check))
      return "Você não pode aceitar por já fazer parte de duas facções.";
   endif
   var new_tipo := GetFaccaoTipo(faccao);
   if (mobile.guild)
      var full_tipo := GetFaccaoTipo(mobile.guild);
      if (full_tipo == new_tipo)
         return "Você já é membro full de uma facção do tipo " + new_tipo + ".";
      endif
   endif
   var hon_id := GetObjProperty(mobile, "honorary_guild_id");
   if (hon_id)
      var hon_guild := FindGuild(hon_id);
      if (!hon_guild)
         EraseObjProperty(mobile, "honorary_guild_id");
      else
         var hon_tipo := GetFaccaoTipo(hon_guild);
         if (hon_tipo == new_tipo)
            return "Você já é membro honorário de uma facção do tipo " + new_tipo + ".";
         endif
      endif
   endif
   return 0;
endfunction

// ── Entrada na facção ─────────────────────────────────────────────────────────

// Retorna 1 se auto-honorário, 0 se auto-full, -1 se o jogador precisa escolher.
function GetAutoMemberTipo(mobile)
   if (mobile.guild)
      return 1;
   endif
   var _hon_id := GetObjProperty(mobile, "honorary_guild_id");
   if (_hon_id)
      if (!FindGuild(_hon_id))
         EraseObjProperty(mobile, "honorary_guild_id");
      else
         return 0;
      endif
   endif
   return -1;
endfunction

// Gump para o jogador escolher como quer entrar.
// Retorna 0 = full, 1 = honorário, -1 = cancelado.
function EscolherTipoMembro(mobile, guild_name)
   var gump := GFCreateGump();
   GFSetId(gump, 0x12352);
   GFResizePic(gump, 0, 0, 9200, 360, 220);
   GFResizePic(gump, 15, 15, 9350, 330, 190);
   GFHtmlShadowed(gump, 20, 25, 320, 20, "#FFC33C", "<center>Como deseja entrar?", 8);
   GFHtmlShadowed(gump, 20, 50, 320, 18, "#AAAAAA", "<center>" + guild_name, 6);

   GFAddButton(gump, 30, 90, 4005, 4006, GF_CLOSE_BTN, 1);
   GFTextLine(gump, 60, 88, 0, "Membro Full");
   GFHtmlShadowed(gump, 60, 106, 250, 16, "#888888", "Acesso completo: cargos e privilégios", 5);

   GFAddButton(gump, 30, 140, 4005, 4006, GF_CLOSE_BTN, 2);
   GFTextLine(gump, 60, 138, 0, "Membro Honorário");
   GFHtmlShadowed(gump, 60, 156, 250, 16, "#888888", "Apenas membro, sem cargos ou privilégios", 5);

   GFAddButton(gump, 260, 175, 4017, 4018, GF_CLOSE_BTN, 0);
   GFTextLine(gump, 290, 173, 0, "Cancelar");

   var input := GFSendGump(mobile, gump);
   if (input[0] == 1) return 0; endif
   if (input[0] == 2) return 1; endif
   return -1;
endfunction

// honorario: 0 = full, 1 = honorário.
function AddMember(faccao, mobile, honorario := 0)
   if (honorario)
      _AddHonoraryMember(faccao, mobile);
   else
      _AddFullMember(faccao, mobile);
   endif
endfunction

function _AddFullMember(faccao, mobile)
   faccao.addMember(mobile);
   var member_obj := struct{
      "above_guards" := 0,
      "above_merchants" := 0,
      "above_members" := 0,
      "giveprivs" := 0,
      "contractnpc" := 0,
      "fixitems" := 0,
      "unlockdoors" := 0,
      "buyuniforms" := 0
   };
   faccao.SetProp(cstr(mobile.serial), member_obj);
   if (GetFaccaoTipo(faccao) == "staff")
      PropagateNomeFaccao(faccao, mobile);
   endif
endfunction

function _AddHonoraryMember(faccao, mobile)
   var honorary := faccao.GetProp("honorary_members");
   if (!honorary) honorary := array{}; endif
   if (!(mobile.serial in honorary))
      honorary.append(mobile.serial);
      faccao.SetProp("honorary_members", honorary);
   endif
   SetObjProperty(mobile, "honorary_guild_id", faccao.guildid);
   if (GetFaccaoTipo(faccao) == "staff")
      PropagateNomeFaccao(faccao, mobile);
   endif
endfunction


// ── Saída da facção ───────────────────────────────────────────────────────────

function RemMember(faccao, mobile)
   if (!mobile.isA(POLCLASS_MOBILE))
      mobile := SystemFindObjectBySerial(mobile, SYSFIND_SEARCH_OFFLINE_MOBILES);
   endif
   if (IsHonoraryMember(faccao, mobile))
      _RemHonoraryMember(faccao, mobile);
   else
      _RemFullMember(faccao, mobile);
   endif
endfunction

function _RemFullMember(faccao, mobile)
   if (faccao.GetProp("leader") == mobile.serial)
      faccao.SetProp("leader", 0x0);
   endif
   faccao.removemember(mobile);
   faccao.EraseProp(cstr(mobile.serial));
   if (GetFaccaoTipo(faccao) == "staff")
      EraseObjProperty(mobile, "nomefaccao");
   endif
endfunction

function _RemHonoraryMember(faccao, mobile)
   var honorary := faccao.GetProp("honorary_members");
   if (honorary)
      var nova := array{};
      foreach s in honorary
         if (s != mobile.serial) nova.append(s); endif
         sleepms(2);
      endforeach
      faccao.SetProp("honorary_members", nova);
   endif
   EraseObjProperty(mobile, "honorary_guild_id");
   if (GetFaccaoTipo(faccao) == "staff")
      EraseObjProperty(mobile, "nomefaccao");
   endif
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

function inGuildRect(guild, item := 0, padding := 0)
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
        endif
        var r := NormalizeRect(cint(rect[1]), cint(rect[2]), cint(rect[3]), cint(rect[4]));
        if (item.x >= r.x1 - padding && item.x <= r.x2 + padding &&
          item.y >= r.y1 - padding && item.y <= r.y2 + padding)
            return 1;
        endif
    endforeach

    // Verificar nos territórios controlados
    var territories := guild.GetProp("territories");
    if (territories)
        var rect_df := OpenDataFile(":architect:areas");
        foreach territory in territories
            var elem := rect_df.FindElement(territory);
            if (elem && elem.GetProp("Faction") == guild.GetProp("name"))
                var rect := ParseRect(elem.GetProp("Rect"));
                if (rect && item.realm == elem.GetProp("Realm"))
                    var r := NormalizeRect(rect.x1, rect.y1, rect.x2, rect.y2);
                    if (item.x >= r.x1 - padding && item.x <= r.x2 + padding &&
                      item.y >= r.y1 - padding && item.y <= r.y2 + padding)
                        return 1;
                    endif
                endif
            endif
            sleepms(2);
        endforeach
    endif

    return 0;
endfunction

// Retorna struct{x,y,z} com o centro do retangulo da guild.
// Parametro z_ref: coordenada z de referencia (ex: npc.z).
function GetGuildCenter(guild, z_ref := 0)
    if (typeof(guild) == "String") guild := FindGuild(guild); endif
    if (!guild) return 0; endif

    var area := guild.getProp("area");
    if (area)
        foreach rect in area
            if (rect && rect.size() >= 4)
                var r := NormalizeRect(cint(rect[1]), cint(rect[2]), cint(rect[3]), cint(rect[4]));
                return struct{ "x" := (r.x1 + r.x2) / 2, "y" := (r.y1 + r.y2) / 2, "z" := z_ref };
            endif
            sleepms(2);
        endforeach
    endif

    var territories := guild.GetProp("territories");
    if (territories)
        var rect_df := OpenDataFile(":architect:areas");
        foreach territory in territories
            var elem := rect_df.FindElement(territory);
            if (elem && elem.GetProp("Faction") == guild.GetProp("name"))
                var parsed := ParseRect(elem.GetProp("Rect"));
                if (parsed)
                    var r := NormalizeRect(parsed.x1, parsed.y1, parsed.x2, parsed.y2);
                    return struct{ "x" := (r.x1 + r.x2) / 2, "y" := (r.y1 + r.y2) / 2, "z" := z_ref };
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
      //      PrintTextAbove(npc, " " + spec.ai);
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
		"Attribute"  := WRESTLING,
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