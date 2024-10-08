// //$Id: toolWear.inc 386 2005-10-03 00:22:39Z muaddiblsd $

include ":charactercreation:habilidades";

function CheckToolWear (who, tool, craft_skill)
  var swings := GetObjProperty(tool,"swings");
  if(!swings)
    var mods := GetObjProperty(tool, "durability");
    var bonus := 0;
    case(mods)
      "durable":        bonus := 10;
      "substantial":    bonus := 20;
      "massive":        bonus := 30;
      "fortified":      bonus := 40;
      "indestructable": bonus := 50;
    endcase
    swings := Cint(50 * tool.quality) + bonus;
    SetObjProperty(tool,"swings", swings);
  endif
  if(swings <= 0)
    DestroyItem(tool);
    SendSysMessage(who,"Your tool broke.");
  else
    var skill := AP_GetSkill(who, craft_skill);
    if(skill > 80)
      skill := 80;
    elseif(skill < 20)
      skill := 20;
    endif
    if((RandomInt(100)+1) > skill)
      if(tool.isa(POLCLASS_WEAPON))
        if(tool.hp > 1)
	        if (TemHabilidade(who, "Preservar Ferramentas") || TemHabilidade(who, "Lenhador Eficaz"))
		        if (randomint(2) == 0)
			        return;
		        endif
          endif
          SetObjProperty(tool,"swings", (swings - 1));
          tool.hp := tool.hp - 1;
          var nome := tool.name;
          SetName(tool, nome + " -");
          sleepms(30);
          SetName(tool, nome);
        elseif((tool.hp > tool.maxhp) || (tool.hp == 1))
          SendSysMessage(who,"Your tool broke.");
          DestroyItem(tool);
        endif
      else
        SetObjProperty(tool,"swings", (swings - 1));
      endif
    endif
  endif
endfunction

function SetToolDurability(who, tool, craft_skill)

  var toolfile := ReadConfigFile(":*:itemdesc");
  var toolscript := toolfile[tool.objtype].Script;

  if(!toolscript.errortext)
    if(!tool.isA(POLCLASS_CONTAINER) && !tool.isA(POLCLASS_ARMOR))
      var myskill := AP_GetSkill(who, craft_skill);
      var swings := 0;
      var mods := GetObjProperty(tool, "durability");
      var bonus := 0;
      case(mods)
        "durable":        bonus := 10;
        "substantial":    bonus := 20;
        "massive":        bonus := 30;
        "fortified":      bonus := 40;
        "indestructable": bonus := 50;
        default: bonus := 0;
      endcase
      swings := Cint(50 + myskill) + bonus;
      SetObjProperty(tool, "swings", swings);
      if(!tool.quality || tool.quality == error)
        tool.quality := 1.0;
      endif
    endif
  endif

endfunction
