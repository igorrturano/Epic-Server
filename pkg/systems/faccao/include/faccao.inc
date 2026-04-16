use guilds;
use os;

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
      "tipo_faccao" := guild.GetProp("tipo_faccao") // "player" ou "staff"
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


function AddMember(faccao, mobile)
   faccao.addMember(mobile);
   var member_obj := struct{
      "above_guards" := 0, // can control guards
      "above_merchants" := 0, // can control merchants
      "above_members" := 0, // can add members
      "giveprivs" := 0, // can edit privs
      "contractnpc" := 0, // can contract npc
      "fixitems" := 0, // can fix items
      "unlockdoors" := 0, // can unlock doors
      "buyuniforms" := 0 // can buy uniform recipes
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