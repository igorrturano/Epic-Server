include ":attributes:advanceCheck";
include ":charactercreation:habilidades";
include ":attributes:attributes";

function CraftCheckSkill(who, craft_skill, difficulty, points, report:=0)

  //difficulty := CDbl(difficulty);
  //points := CDbl(points);
  //if(difficulty == error)
  /*
  elseif(difficulty == 0.0)
    if (report)
      return 100;
    endif
    return 1;
  endif
  */
  
  /*
  if(effective < 0)
    effective := 0.0;
  endif
  */
  //chance :=  35.0 + (effective - difficulty); //antigo craft
  
  /* EVILLINK CRAFT START 1.1 */
  var chance;
  var effective := CDbl(AP_GetSkill(who, craft_skill));
  var itemskill := difficulty - 10; //QUANDO APARECE NO MENU
  
  chance := ((itemskill)-effective)*(-((difficulty+(40-itemskill))/10))+1;
  var rolagem := RandomFloat(100);
  //SendSysMessage(who, "DEBUG CHANCE: "+chance);
  //SendSysMessage(who, "DEBUG ROLOU: "+rolagem);
  /* EVILLINK CRAFT END */

  if(chance < 0)
    chance := 0;
  elseif(chance > 100)
    chance := 100;
  endif
  //SendSysMessage(who, "DEBUG: VALORES: DIFFI" +difficulty+ "- EFFEC: " +effective+ "- CHANCE: " +chance);

  if(report)
    return Cdbl(chance);
  endif

  if(chance >= 100)
    points := 0;
  endif
  if(points > 0)
//	CheckSkillAdvance(who, craft_skill, points, chance);
	SkillCheck(who, craft_skill, difficulty, points);
  //SendSysMessage(who, "DEBUG CHANCEUP: "+SkillCheck(who, craft_skill, difficulty, points));  
  endif
  if(rolagem < chance)
    if (report)
	    return 100;
    endif
    return 1;
  else
    return 0;
  endif

endfunction


function ExceptionalCraftChance(who, skillname, skillvalue, skillreq, theitem)
  var chance := CDbl(skillvalue - skillreq) / 5.0; //Variação de 0% a 20% de chance
  //SendSysMessage(who, "who:"+who.name+" skillvalue:"+ skillvalue+" skillreq:"+ skillreq+ " theitem:" +theitem);
  if(chance > 0.0) //A diferença de skill entre o crafter e o necessário passa a ser o percentual de fazer excepcional
    if(skillname == "MetalWorking")
      if(TemHabilidade(who,"Maestria em Armas") && (theitem.TipoAtaque == "Cortante"))
        chance := (chance * 2.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
      endif

      if(TemHabilidade(who,"Maestria em Armas") && (theitem.TipoAtaque == "Perfurante"))
        chance := (chance * 2.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
      endif

      if(TemHabilidade(who,"Maestria em Armas") && (theitem.TipoAtaque == "Contusivo"))
        chance := (chance * 2.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
      endif

    //if(TemHabilidade(who,"Maestria em Armaduras") && (theitem.TipoProtecao == "ArmaduraPesada" || theitem.TipoProtecao == "ArmaduraMedia" || theitem.TipoProtecao == "ArmaduraLeve"))
    //  chance := (chance * 5.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
    //endif

      if(TemHabilidade(who,"Maestria em Armaduras") && (theitem.TipoProtecao == "Armadura Pesada Fraca" || theitem.TipoProtecao == "Armadura Pesada Media" || theitem.TipoProtecao == "Armadura Pesada Forte"))
        chance := (chance * 2.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
      endif

      if(TemHabilidade(who,"Maestria em Armaduras") && (theitem.TipoProtecao == "Armadura Media Fraca" || theitem.TipoProtecao == "Armadura Media Media" || theitem.TipoProtecao == "Armadura Media Forte"))
        chance := (chance * 2.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
      endif

      if(TemHabilidade(who,"Maestria em Armaduras") && (theitem.TipoProtecao == "Armadura Leve Fraca" || theitem.TipoProtecao == "Armadura Leve Media" || theitem.TipoProtecao == "Armadura Leve Forte"))
        chance := (chance * 2.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
      endif

//    if(TemHabilidade(who,"Maestria em Escudos") && (theitem.TipoProtecao == "EscudoPesado" || theitem.TipoProtecao == "EscudoMedio" || theitem.TipoProtecao == "EscudoLeve"))
//      chance := (chance * 5.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
      if(TemHabilidade(who,"Maestria em Armaduras") && (theitem.TipoProtecao == "Escudo de Corpo Fraco" || theitem.TipoProtecao == "Escudo de Corpo Medio" || theitem.TipoProtecao == "Escudo de Corpo Forte"))
        chance := (chance * 2.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
      endif

      if(TemHabilidade(who,"Maestria em Armaduras") && (theitem.TipoProtecao == "Escudo Pesado Fraco" || theitem.TipoProtecao == "Escudo Pesado Medio" || theitem.TipoProtecao == "Escudo Pesado Forte"))
        chance := (chance * 2.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
      endif
      if(TemHabilidade(who,"Maestria em Armaduras") && (theitem.TipoProtecao == "Escudo Medio Fraco" || theitem.TipoProtecao == "Escudo Medio Medio" || theitem.TipoProtecao == "Escudo Medio Forte"))
        chance := (chance * 2.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
      endif
      if(TemHabilidade(who,"Maestria em Armaduras") && (theitem.TipoProtecao == "Escudo Leve Fraco" || theitem.TipoProtecao == "Escudo Leve Medio" || theitem.TipoProtecao == "Escudo Leve Forte"))
        chance := (chance * 2.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
      endif
    elseif(skillname == "WoodWorking")
      if(TemHabilidade(who,"Maestria em Carpintaria"))
        chance := (chance * 2.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
      endif
      if(TemHabilidade(who,"Maestria em Carpintaria") && !theitem.isa(POLCLASS_WEAPON) && !theitem.isa(POLCLASS_ARMOR))
        chance := 100.0;
      endif
    elseif(skillname == "DomesticWork")
      if((CInt(theitem.resist_physical)==0)) //Não é armadura
        if(TemHabilidade(who,"Maestria em Alfaiataria"))
          chance := (chance * 2.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
        endif
//      else
//      if(TemHabilidade(who,"Maestria em Curticao") && (theitem.TipoProtecao == "ArmaduraPesada" || theitem.TipoProtecao == "ArmaduraMedia" || theitem.TipoProtecao == "ArmaduraLeve"))
//        chance := (chance * 5.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
       if(TemHabilidade(who,"Maestria em Alfaiataria") && (theitem.TipoProtecao == "Armadura Pesada Fraca" || theitem.TipoProtecao == "Armadura Pesada Media" || theitem.TipoProtecao == "Armadura Pesada Forte"))
          chance := (chance * 2.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
        endif
       if(TemHabilidade(who,"Maestria em Alfaiataria") && (theitem.TipoProtecao == "Armadura Media Fraca" || theitem.TipoProtecao == "Armadura Media Media" || theitem.TipoProtecao == "Armadura Media Forte"))
          chance := (chance * 2.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
        endif
       if(TemHabilidade(who,"Maestria em Alfaiataria") && (theitem.TipoProtecao == "Armadura Leve Fraca" || theitem.TipoProtecao == "Armadura Leve Media" || theitem.TipoProtecao == "Armadura Leve Forte"))
          chance := (chance * 2.0) + CDbl(GetObjProperty(who, "craftexcepmod"));
        endif
      endif
    endif
  else
    chance := 0.0;
  endif
  //SendSysMessage(who, "Calculada chance de Excepcional:"+chance);
  return chance;
endfunction

function ExceptionalCraft( who, skillname, theitem, skillvalue, skillreq)
  var cfg:=ReadConfigFile(":combat:itemdesc");
  var elem := FindConfigElem(cfg, theitem.objtype);
  var chance := Cdbl(ExceptionalCraftChance(who, skillname, skillvalue, skillreq, elem));
  var teste := RandomFloat(100.0);
  //SendSysMessage(who, "Chance:"+chance+" Rolagem:"+teste);
  if(teste < chance)
    var margem := chance - teste;
    return 1.2 + (margem/50);
  else
    return 1.0;
  endif
endfunction