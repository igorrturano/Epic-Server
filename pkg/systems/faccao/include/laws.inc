// =============================================================================
// laws.inc — Gerenciamento de leis de faccao
// =============================================================================
//
// Responsavel por criar, listar, buscar, remover e alternar as leis que os
// guardas de faccao aplicam aos players dentro do territorio.
//
// TIPOS DE LEI
//   Unicos (apenas um por guild): nofight, noweapon, nolockpick, nobility
//   Multiplos (varios por guild): noitem, norace, noguild, nooutsideron, nolawon
//
// FUNCOES
//   AddLaw(guild, law)
//     Adiciona ou atualiza uma lei na guild. Leis de tipo unico sao
//     atualizadas no lugar (nunca duplicadas). Leis multiplas sao sempre
//     adicionadas. Garante que o campo "target" existe (padrao: "foramembros").
//
//   ListLaws(guild)
//     Retorna o array de leis da guild. Realiza migracao automatica: leis
//     antigas sem campo "target" recebem o valor padrao "foramembros" e sao
//     regravadas, garantindo compatibilidade com dados legados.
//
//   GetLaw(guild, type)
//     Retorna todas as leis do tipo informado usando FindAllElemInArray.
//
//   RemLaw(guild, index)
//     Remove a lei pelo indice no array (1-based).
//
//   ToggleLaw(guild, index)
//     Alterna o campo "on" da lei (liga/desliga) sem remover.
// =============================================================================

// ###########################################
// #
// # RemLaw
// # AddLaw
// # GetLaw
// # ListLaw
// # DisableLaw
// #
// ###########################################
// 
// # Laws:
// # $ No LockPick - Guards will not allow lockpick around them
// ## - Doors
// ## - Chests
// # $ No Fight - Guards will not allow fights around them
// # $ No Item - Guards will not allow players using that item around them
// # $ No Weapons - Guards will not allow players to use weapons (if no monsters) around them
// # $ Nobility Above Laws - Guards will ignore all laws if player have title
// # $ No Race 
// # $ No Faction


function AddLaw(guild, law)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif

   if (!guild) return; endif

   // Preserva todos os campos da lei (target, exceto_guilds, opt, etc.) e garante on=1
   var new_law := law;
   new_law.on := 1;
   // Garante target explicito para evitar <uninitialized object> nos logs
   if (!new_law.target || typeof(new_law.target) == "Uninit")
      new_law.target := "foramembros";
   endif

   var laws := ListLaws(guild);
   if (!(new_law.type in array{"noitem", "norace", "noguild", "nooutsideron", "nolawon"/*, "userban"*/}))
      // Tipo unico: atualiza a lei existente em vez de rejeitar
      var _idx := 1;
      foreach _existing in laws
          sleepms(2);
          if (_existing.type == new_law.type)
              laws[_idx] := new_law;
              guild.setProp("laws", laws);
              return 1;
          endif
          _idx := _idx + 1;
      endforeach
   endif

   laws.append(new_law);
   guild.setProp("laws", laws);
   return 1;
endfunction

function ListLaws(guild)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif

   if (!guild) return; endif

   var laws := guild.GetProp("laws");
   if (!laws) laws := array{}; endif

   // Normaliza leis antigas salvas sem o campo target (migracao automatica)
   var _changed := 0;
   var _i := 1;
   foreach _law in laws
      sleepms(2);
      if (!_law.target || typeof(_law.target) == "Uninit")
         laws[_i].+target := "foramembros";
         _changed := 1;
      endif
      _i += 1;
   endforeach
   if (_changed)
      guild.setProp("laws", laws);
   endif

   return laws;
endfunction

function GetLaw(guild, type)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif
   var law_list := ListLaws(guild);

   return FindAllElemInArray(law_list, struct{ "type" := type });
endfunction

function RemLaw(guild, index)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif
   if (typeof(index) == "Integer") 
      var law_list := ListLaws(guild);
      law_list.Erase(index);
      guild.setProp("laws", law_list);
   endif
endfunction

function ToggleLaw(guild, index)
   if (typeof(guild) == "String")
      guild := FindGuild(guild);
   endif
   var law_list := ListLaws(guild);
   law_list[index].on := !law_list[index].on;
   guild.setProp("laws", law_list);
endfunction