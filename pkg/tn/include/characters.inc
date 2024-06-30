use uo;	
use cfgfile;
include ":attributes:attributes";

function checkskillcap(who)

  foreach skill in AP_ListAttributesByType()
		if ( (skill != "Dexterity") && (skill != "Strength") && (skill != "Intelligence"))
      var skillvalue := AP_GetTrueSkill(who, skill);
			if ( skillvalue > 0 )
        if ( skillvalue > GetLocalSkillCap(who, skill) )
          AP_SetTrueSkill(who, skill, GetLocalSkillCap(who, skill));
        endif
      endif
    endif
  endforeach

endfunction

function GetLocalSkillCap(mobile, skill_name)

  var elem := AP_GetAttributeCfgElem(skill_name);
	var stat := GetConfigString(elem, "Stat");
  
  var att := AP_GetTrueStat(mobile, stat);
  return att;

endfunction

function SetKitsSkillsMods(who)

	var kit := GetObjProperty(who, "kit");

	if (kit == "veterano")
		AP_ModifySkillMod(who, TACTICS, 15);
		AP_ModifySkillMod(who, SWORDSMANSHIP, 5);
		AP_ModifySkillMod(who, PARRY, 5);
		AP_ModifySkillMod(who, AXE, 5);
		AP_ModifySkillMod(who, STAFF, 5);
		AP_ModifySkillMod(who, WRESTLING, 5);
		AP_ModifySkillMod(who, ARCHERY, 5);
		AP_ModifySkillMod(who, FENCING, 5);
		AP_ModifySkillMod(who, MACE_FIGHTING, 5);
		AP_ModifySkillMod(who, POLE_ARMS, 5);
	elseif (kit == "covarde" )
 	       AP_SetSkillMod(who, HIDING, 10);
        	AP_ModifySkillMod(who, STEALTH, 10);
	elseif (kit == "filho de burgues")
	        AP_ModifySkillMod(who, CARPENTRY, 10);
        	AP_ModifySkillMod(who, BLACKSMITHY, 10);
	        AP_ModifySkillMod(who, TINKERING, 10);
	        AP_ModifySkillMod(who, TAILORING, 10);
	elseif (kit == "treinado no exercito")
	        AP_ModifySkillMod(who, TACTICS, 15);
	elseif (kit == "alquimista")
	        AP_ModifySkillMod(who, ALCHEMY, 25);
	elseif (kit == "desbravador")
	        AP_ModifySkillMod(who, CARTOGRAPHY, 10);
	        AP_ModifySkillMod(who, REMOVE_TRAP, 10);
	        AP_ModifySkillMod(who, LOCKPICKING, 10);
	        AP_ModifySkillMod(who, DETECTING_HIDDEN, 10);
	endif

endfunction

function RemoveKitsSkillsMods(who)

	var kit := GetObjProperty(who, "kit");

	if (kit == "veterano")
		AP_ModifySkillMod(who, TACTICS, -15);
		AP_ModifySkillMod(who, SWORDSMANSHIP, -5);
		AP_ModifySkillMod(who, PARRY, -5);
		AP_ModifySkillMod(who, AXE, -5);
		AP_ModifySkillMod(who, STAFF, -5);
		AP_ModifySkillMod(who, WRESTLING, -5);
		AP_ModifySkillMod(who, ARCHERY, -5);
		AP_ModifySkillMod(who, FENCING, -5);
		AP_ModifySkillMod(who, MACE_FIGHTING, -5);
		AP_ModifySkillMod(who, POLE_ARMS, -5);
	elseif (kit == "covarde" )
 	       AP_ModifySkillMod(who, HIDING, -10);
        	AP_ModifySkillMod(who, STEALTH, -10);
	elseif (kit == "filho de burgues")
	        AP_ModifySkillMod(who, CARPENTRY, -10);
        	AP_ModifySkillMod(who, BLACKSMITHY, -10);
	        AP_ModifySkillMod(who, TINKERING, -10);
	        AP_ModifySkillMod(who, TAILORING, -10);
	elseif (kit == "treinado no exercito")
	        AP_ModifySkillMod(who, TACTICS, -15);
	elseif (kit == "alquimista")
	        AP_ModifySkillMod(who, ALCHEMY, -25);
	elseif (kit == "desbravador")
	        AP_ModifySkillMod(who, CARTOGRAPHY, -10);
	        AP_ModifySkillMod(who, REMOVE_TRAP, -10);
	        AP_ModifySkillMod(who, LOCKPICKING, -10);
	        AP_ModifySkillMod(who, DETECTING_HIDDEN, -10);
	endif

endfunction

//as funções abaixo sao old. usadas no antigo sistema de classes
// nada de util
// pode deletar acho

function SetSkillType(who, skill, type, init := 0)

	var chardata := GetObjProperty(who, "chardata");
	
	chardata.Skills[skill] := type;

	SetObjProperty(who, "chardata", chardata);

	if (init)
		AP_SetTrueSkill(who, skill, init);
	endif

endfunction

//SUBCLASSES SKILL

function SetSubClassSkills(who)
	
	var chardata := GetObjProperty(who, "chardata");
	var subclasse := GetObjProperty(who, "subclasse");
//COMBATENTES

	if (subclasse == "Bruto")
 	   sendsysmessage(who, "Voce escolheu bruto");
          SetObjProperty(who, "Vitality", chardata.Vit+10);
          AP_SetTrueStat(who, INTELLIGENCE, chardata.Int-10);
          AP_SetTrueStat(who, STRENGTH, chardata.Str+5);
		SetSkillType(who, TACTICS, 3, 40);
		SetSkillType(who, MACE_FIGHTING, 3, 30);
		SetSkillType(who, WRESTLING, 3, 30);
		SetSkillType(who, AXE, 3, 30);

		SetSkillType(who, SWORDSMANSHIP, 2, 10);
		SetSkillType(who, CAMPING, 2, 10);
		SetSkillType(who, ARMS_LORE, 2, 10);
    SetSkillType(who, ANATOMY, 2, 10);
    SetSkillType(who, HEALING, 2, 10);

		SetSkillType(who, HIDING, 1);
		SetSkillType(who, SKINNING, 1);
		SetSkillType(who, FISHING, 1);
		SetSkillType(who, DETECTING_HIDDEN, 1);
    SetSkillType(who, MUSICIANSHIP, 1);
    SetSkillType(who, MINING, 1);
    SetSkillType(who, LUMBERJACKING, 1);
    SetSkillType(who, COOKING, 1);

	elseif (subclasse == "Selvagem")
 	   sendsysmessage(who, "Voce escolheu Selvagem");
          SetObjProperty(who, "Vitality", chardata.Vit+5);
          AP_SetTrueStat(who, STRENGTH, chardata.Str+5);
          AP_SetTrueStat(who, INTELLIGENCE, chardata.Int-10);
          AP_SetTrueStat(who, DEXTERITY, (chardata.Dex+5));
          
		SetSkillType(who, MACE_FIGHTING, 3, 25);
		SetSkillType(who, WRESTLING, 3, 30);
		SetSkillType(who, AXE, 3, 30);
		SetSkillType(who, SKINNING, 3, 30);
		SetSkillType(who, TACTICS, 3, 30);
    
		SetSkillType(who, ARCHERY, 2, 10);
		SetSkillType(who, CAMPING, 2, 10);
		SetSkillType(who, HEALING, 2, 10);
		SetSkillType(who, HIDING, 2, 10);
    SetSkillType(who, ANATOMY, 2, 10);
    SetSkillType(who, TRACKING, 2, 10);
    SetSkillType(who, RESISTING_SPELLS, 2, 10);

		SetSkillType(who, LUMBERJACKING, 1);
		SetSkillType(who, ANIMAL_TAMING, 1);
		SetSkillType(who, ANIMAL_LORE, 1);
		SetSkillType(who, FISHING, 1);
		SetSkillType(who, COOKING, 1);
		SetSkillType(who, DETECTING_HIDDEN, 1);
    SetSkillType(who, MINING, 1);
		SetSkillType(who, MUSICIANSHIP, 1);

	elseif (subclasse == "Vingador")
          SetObjProperty(who, "Vitality", chardata.Vit+10);
          AP_SetTrueStat(who, STRENGTH, chardata.Str+5);
          AP_SetTrueStat(who, INTELLIGENCE, chardata.Int+5);
          AP_SetTrueStat(who, DEXTERITY, (chardata.Dex+5));
          
		SetSkillType(who, SWORDSMANSHIP, 3, 30);
		SetSkillType(who, WRESTLING, 3, 30);
		SetSkillType(who, SKINNING, 3, 30);
		SetSkillType(who, TACTICS, 3, 30);
    SetSkillType(who, TRACKING, 3, 30);
    SetSkillType(who, PARRY, 3, 30);
    SetSkillType(who, RESISTING_SPELLS, 3, 30);

		SetSkillType(who, ANIMAL_LORE, 2,20);
		SetSkillType(who, ARCHERY, 2, 10);
		SetSkillType(who, CAMPING, 2, 10);
		SetSkillType(who, HEALING, 2, 10);
		SetSkillType(who, HIDING, 2, 10);
    SetSkillType(who, ANATOMY, 2, 10);

		SetSkillType(who, LUMBERJACKING, 1);
		SetSkillType(who, FISHING, 1);
		SetSkillType(who, COOKING, 1);
		SetSkillType(who, DETECTING_HIDDEN, 1);
    SetSkillType(who, MINING, 1);
    SetSkillType(who, MUSICIANSHIP, 1);

	elseif (subclasse == "Protetor")
          SetObjProperty(who, "Vitality", chardata.Vit+15);
          AP_SetTrueStat(who, STRENGTH, chardata.Str+5);
          
		SetSkillType(who, TACTICS, 3, 30);
		SetSkillType(who, SWORDSMANSHIP, 3, 30);
		SetSkillType(who, MACE_FIGHTING, 3, 30);
		SetSkillType(who, PARRY, 3, 40);
    SetSkillType(who, DETECTING_HIDDEN,  3, 30);
    SetSkillType(who, RESISTING_SPELLS, 3, 30);

		SetSkillType(who, WRESTLING, 2, 10);
		SetSkillType(who, ARMS_LORE, 2, 10);
    SetSkillType(who, HEALING, 2, 10);
    SetSkillType(who, ANATOMY, 2, 10);

    SetSkillType(who, CAMPING, 1);
		SetSkillType(who, COOKING, 1);
		SetSkillType(who, SKINNING, 1);
		SetSkillType(who, FISHING, 1);
    SetSkillType(who, LUMBERJACKING, 1);
    SetSkillType(who, MINING, 1);
    SetSkillType(who, MUSICIANSHIP, 1);


	elseif (subclasse == "Guardiao")
          AP_SetTrueStat(who, DEXTERITY, (chardata.Dex+10));
          AP_SetTrueStat(who, INTELLIGENCE, chardata.Int+5);
		SetSkillType(who, TACTICS, 3, 30);
		SetSkillType(who, ARCHERY, 3, 30);
		SetSkillType(who, CAMPING, 3, 30);
		SetSkillType(who, SKINNING, 3, 30);
		SetSkillType(who, ANIMAL_LORE, 3, 30);
    SetSkillType(who, TRACKING, 3, 30);

		SetSkillType(who, HIDING, 2, 10);
		SetSkillType(who, STEALTH, 2, 10);
		SetSkillType(who, HEALING, 2, 10);
		SetSkillType(who, WRESTLING, 2, 10);
    SetSkillType(who, ANATOMY ,2, 10);
		SetSkillType(who, SWORDSMANSHIP, 2, 10);

		SetSkillType(who, MUSICIANSHIP, 1);
		SetSkillType(who, DETECTING_HIDDEN, 1);
		SetSkillType(who, FISHING, 1);
		SetSkillType(who, ANIMAL_TAMING, 1);
    SetSkillType(who, LUMBERJACKING, 1);
    SetSkillType(who, MINING, 1);
    SetSkillType(who, COOKING, 1);
    SetSkillType(who, BOWCRAFT_FLETCHING, 1);

	elseif (subclasse == "Espadachim")
          AP_SetTrueStat(who, DEXTERITY, (chardata.Dex+5));
          AP_SetTrueStat(who, STRENGTH, chardata.Str+10);
		SetSkillType(who, TACTICS, 3, 45);
		SetSkillType(who, SWORDSMANSHIP, 3, 40);
    SetSkillType(who, ANATOMY, 3, 30);
		SetSkillType(who, PARRY, 3, 40);

		SetSkillType(who, WRESTLING, 2, 10);
		SetSkillType(who, CAMPING, 2, 10);
		SetSkillType(who, HEALING, 2, 10);
		SetSkillType(who, ARMS_LORE, 2, 10);

		SetSkillType(who, EVALUATING_INTELLIGENCE, 1);
		SetSkillType(who, DETECTING_HIDDEN, 1);
		SetSkillType(who, HIDING, 1);
		SetSkillType(who, SKINNING, 1);
		SetSkillType(who, FISHING, 1);
		SetSkillType(who, COOKING, 1);
		SetSkillType(who, MINING, 1);
		SetSkillType(who, LUMBERJACKING, 1);
    SetSkillType(who, MUSICIANSHIP, 1);
    

	elseif (subclasse == "Swashbuckler")
             AP_SetTrueStat(who, STRENGTH, (chardata.Str-5));
            AP_SetTrueStat(who, DEXTERITY, (chardata.Dex+15));
		SetSkillType(who, TACTICS, 3, 40);
		SetSkillType(who, FENCING, 3, 40);
		SetSkillType(who, ANATOMY, 3, 30);
    SetSkillType(who, RESISTING_SPELLS, 3, 30);

		SetSkillType(who, WRESTLING, 2, 10);
		SetSkillType(who, CAMPING, 2, 10);
		SetSkillType(who, HEALING, 2, 10);
		SetSkillType(who, ARMS_LORE, 2, 10);
		SetSkillType(who, MUSICIANSHIP, 2, 10);
		SetSkillType(who, HIDING, 2, 10);
    SetSkillType(who, DETECTING_HIDDEN, 2, 10);

		SetSkillType(who, COOKING, 1);
		SetSkillType(who, SKINNING, 1);
		SetSkillType(who, MINING, 1);
		SetSkillType(who, LUMBERJACKING, 1);

	elseif (subclasse == "Sentinela")
            AP_SetTrueStat(who, DEXTERITY, (chardata.Dex+10));
          AP_SetTrueStat(who, INTELLIGENCE, chardata.Int+5);
		SetSkillType(who, ARCHERY, 3, 35);
		SetSkillType(who, TACTICS, 3, 30);
		SetSkillType(who, ANATOMY, 3, 30);

		SetSkillType(who, SWORDSMANSHIP, 2, 10);
		SetSkillType(who, WRESTLING, 2, 5);
		SetSkillType(who, CAMPING, 2, 10);
		SetSkillType(who, HEALING, 2, 10);
		SetSkillType(who, ARMS_LORE, 2, 10);

		SetSkillType(who, HIDING, 1);
		SetSkillType(who, COOKING, 1);
		SetSkillType(who, SKINNING, 1);
		SetSkillType(who, DETECTING_HIDDEN, 1);
		SetSkillType(who, FISHING, 1);
		SetSkillType(who, MINING, 1);
		SetSkillType(who, LUMBERJACKING, 1);
		SetSkillType(who, BOWCRAFT_FLETCHING, 1);    

	elseif (subclasse == "Cavaleiro")
          SetObjProperty(who, "Vitality", chardata.Vit+5);
          AP_SetTrueStat(who, STRENGTH, chardata.Str+5);
		SetSkillType(who, POLE_ARMS, 3, 40);
		SetSkillType(who, TACTICS, 3, 35);
		SetSkillType(who, SWORDSMANSHIP, 3, 30);
    SetSkillType(who, PARRY, 3, 30);
    
		SetSkillType(who, ANATOMY, 2, 10);
		SetSkillType(who, WRESTLING, 2, 10);
		SetSkillType(who, CAMPING, 2, 10);
		SetSkillType(who, HEALING, 2, 10);
		SetSkillType(who, ARMS_LORE, 2, 10);

		SetSkillType(who, SKINNING, 1);
		SetSkillType(who, FISHING, 1);
		SetSkillType(who, HIDING, 1);
		SetSkillType(who, DETECTING_HIDDEN, 1);
		SetSkillType(who, COOKING, 1);
		SetSkillType(who, LUMBERJACKING, 1);
		SetSkillType(who, MUSICIANSHIP, 1);
		SetSkillType(who, MINING, 1);

	elseif (subclasse == "Gladiador")
          SetObjProperty(who, "Vitality", chardata.Vit+10);
          AP_SetTrueStat(who, STRENGTH, chardata.Str+5);
		SetSkillType(who, MACE_FIGHTING, 3, 20);
		SetSkillType(who, AXE, 3, 20);
		SetSkillType(who, POLE_ARMS, 3, 20);
		SetSkillType(who, TACTICS, 3, 20);
		SetSkillType(who, SWORDSMANSHIP, 3, 20);
		SetSkillType(who, FENCING, 3, 20);
		SetSkillType(who, PARRY, 3, 20);
		SetSkillType(who, WRESTLING, 3, 35);

		SetSkillType(who, CAMPING, 2, 10);
		SetSkillType(who, ARMS_LORE, 2, 25);
		SetSkillType(who, ANATOMY, 2, 10);
    SetSkillType(who, HEALING, 2, 10);

		SetSkillType(who, ARCHERY, 1);
		SetSkillType(who, HIDING, 1);
		SetSkillType(who, SKINNING, 1);
		SetSkillType(who, FISHING, 1);
		SetSkillType(who, DETECTING_HIDDEN, 1);
		SetSkillType(who, COOKING, 1);
		SetSkillType(who, LUMBERJACKING, 1);
		SetSkillType(who, MUSICIANSHIP, 1);
		SetSkillType(who, MINING, 1);

	elseif (subclasse == "Bandoleiro")
          SetObjProperty(who, "Vitality", chardata.Vit-5);
          AP_SetTrueStat(who, STRENGTH, chardata.Str-5);
          AP_SetTrueStat(who, DEXTERITY, (chardata.Dex+5));
          AP_SetTrueStat(who, INTELLIGENCE, chardata.Int+5);
		SetSkillType(who, TACTICS, 3, 25);
		SetSkillType(who, SWORDSMANSHIP, 3, 25);
		SetSkillType(who, FENCING, 3, 25);
		SetSkillType(who, BACKSTAB, 3, 30);

		SetSkillType(who, WRESTLING, 2, 10);
    SetSkillType(who, ANATOMY, 2, 10);
		SetSkillType(who, CAMPING, 2, 10);
		SetSkillType(who, ARMS_LORE, 2, 10);
    SetSkillType(who, HIDING, 2, 10);
    SetSkillType(who, POISONING, 2, 10);

		SetSkillType(who, HEALING, 1);
		SetSkillType(who, STEALTH, 1);
		SetSkillType(who, COOKING, 1);
		SetSkillType(who, DETECTING_HIDDEN, 1);
		SetSkillType(who, SKINNING, 1);
		SetSkillType(who, FISHING, 1);
		SetSkillType(who, MINING, 1);
		SetSkillType(who, LUMBERJACKING, 1);
		SetSkillType(who, MUSICIANSHIP, 1);
//LADINOS

	elseif (subclasse == "Assassino")
          SetObjProperty(who, "Vitality", chardata.Vit-5);
          AP_SetTrueStat(who, STRENGTH, chardata.Str-10);
          AP_SetTrueStat(who, DEXTERITY, (chardata.Dex+10));
          AP_SetTrueStat(who, INTELLIGENCE, chardata.Int+10);
		SetSkillType(who, POISONING, 3, 40);
		SetSkillType(who, BACKSTAB, 3, 40);
		SetSkillType(who, FENCING, 3, 30);
		SetSkillType(who, ANATOMY, 3, 30);
		SetSkillType(who, STEALTH, 3, 30);		
		SetSkillType(who, HIDING, 3, 30);		

		SetSkillType(who, DETECTING_HIDDEN, 2, 10);
		SetSkillType(who, TACTICS, 2, 10);
		SetSkillType(who, EVALUATING_INTELLIGENCE, 2, 10);
	
		SetSkillType(who, SKINNING, 1);
		SetSkillType(who, FISHING, 1);
		SetSkillType(who, COOKING, 1);
		SetSkillType(who, WRESTLING, 1);
		SetSkillType(who, CAMPING, 1);
		SetSkillType(who, MUSICIANSHIP, 1);

	elseif (subclasse == "Pirata")
          SetObjProperty(who, "Vitality", chardata.Vit+5);
          AP_SetTrueStat(who, STRENGTH, chardata.Str+5);
          AP_SetTrueStat(who, INTELLIGENCE, chardata.Int-5);
		SetSkillType(who, FENCING, 3, 30);
		SetSkillType(who, SWORDSMANSHIP, 3, 30);
		SetSkillType(who, PARRY, 3, 30);
		SetSkillType(who, WRESTLING, 3, 30);
		SetSkillType(who, TACTICS, 3, 20);

		SetSkillType(who, ANATOMY, 2, 10);
		SetSkillType(who, DETECTING_HIDDEN, 2, 10);
		SetSkillType(who, REMOVE_TRAP, 2, 10);
		SetSkillType(who, LOCKPICKING, 2, 10);
		SetSkillType(who, ARMS_LORE, 3, 30);

		SetSkillType(who, MUSICIANSHIP, 1);		
		SetSkillType(who, SKINNING, 1);
		SetSkillType(who, FISHING, 1);
		SetSkillType(who, COOKING, 1);
		SetSkillType(who, CAMPING, 1);
    

	elseif (subclasse == "Cacador de Tesouros")
          SetObjProperty(who, "Vitality", chardata.Vit-10);
          AP_SetTrueStat(who, STRENGTH, chardata.Str-10);
          AP_SetTrueStat(who, DEXTERITY, (chardata.Dex+10));
          AP_SetTrueStat(who, INTELLIGENCE, chardata.Int+10);
		SetSkillType(who, DETECTING_HIDDEN, 3, 30);
		SetSkillType(who, REMOVE_TRAP, 3, 30);
		SetSkillType(who, LOCKPICKING, 3, 30);
		SetSkillType(who, ARMS_LORE, 3, 30);
    SetSkillType(who, CARTOGRAPHY, 3, 30);

		SetSkillType(who, FENCING, 2, 10);
		SetSkillType(who, HIDING, 2, 10);
		SetSkillType(who, ARCHERY, 2, 10);
		SetSkillType(who, STEALTH, 2, 10);
		SetSkillType(who, ANATOMY, 2, 10);
		SetSkillType(who, EVALUATING_INTELLIGENCE, 2, 10);
		SetSkillType(who, TACTICS, 2, 10);
    SetSkillType(who, RESISTING_SPELLS, 2, 10);

		SetSkillType(who, MUSICIANSHIP, 1);		
		SetSkillType(who, SKINNING, 1);
		SetSkillType(who, FISHING, 1);
		SetSkillType(who, COOKING, 1);
		SetSkillType(who, WRESTLING, 1);
		SetSkillType(who, CAMPING, 1);

	elseif (subclasse == "Espiao")
          AP_SetTrueStat(who, DEXTERITY, (chardata.Dex+5));
          AP_SetTrueStat(who, INTELLIGENCE, chardata.Int+10);
		SetSkillType(who, DETECTING_HIDDEN, 3, 30);
		SetSkillType(who, LOCKPICKING, 3, 20);
		SetSkillType(who, HIDING, 3, 40);
		SetSkillType(who, STEALTH, 3, 40);
    SetSkillType(who, TRACKING, 3, 40);
		SetSkillType(who, EVALUATING_INTELLIGENCE, 3, 30);

		SetSkillType(who, REMOVE_TRAP, 2, 10);
		SetSkillType(who, FENCING, 2, 10);
		SetSkillType(who, ARCHERY, 2, 10);
		SetSkillType(who, ANATOMY, 2, 10);
		SetSkillType(who, TACTICS, 2, 10);
    SetSkillType(who, ARMS_LORE, 2, 10);
		
		SetSkillType(who, SKINNING, 1);
		SetSkillType(who, FISHING, 1);
		SetSkillType(who, COOKING, 1);
		SetSkillType(who, WRESTLING, 1);
		SetSkillType(who, CAMPING, 1);

//SABIOS
    
	elseif (subclasse == "Mago")
          SetObjProperty(who, "Vitality", chardata.Vit-5);
          AP_SetTrueStat(who, INTELLIGENCE, chardata.Int+10);
		SetSkillType(who, MAGERY, 3, 30);
		SetSkillType(who, ANATOMY, 3, 30);
		SetSkillType(who, HEALING, 3, 30);
		SetSkillType(who, EVALUATING_INTELLIGENCE, 3, 30);
		SetSkillType(who, RESISTING_SPELLS, 3, 30);
    SetSkillType(who, STAFF, 3, 30);
		SetSkillType(who, ALCHEMY, 3, 30);
    SetSkillType(who, ARMS_LORE, 3, 30);
    
		SetSkillType(who, ANIMAL_LORE, 2, 10);
		SetSkillType(who, COOKING, 2, 10);
		SetSkillType(who, MUSICIANSHIP, 2, 10);
		SetSkillType(who, CAMPING, 2, 10);
		SetSkillType(who, DETECTING_HIDDEN, 2, 10);

		SetSkillType(who, FISHING, 1);
		SetSkillType(who, SKINNING, 1);
		SetSkillType(who, LUMBERJACKING, 1);
		SetSkillType(who, MINING, 1);

	elseif (subclasse == "Druida")
          SetObjProperty(who, "Vitality", chardata.Vit+5);
          AP_SetTrueStat(who, INTELLIGENCE, chardata.Int+10);
          
		SetSkillType(who, MAGERY, 3, 30);
		SetSkillType(who, VETERINARY, 3, 30);
		SetSkillType(who, ANIMAL_LORE, 3, 30);
		SetSkillType(who, ANIMAL_TAMING, 3, 30);
		SetSkillType(who, RESISTING_SPELLS, 3, 30);
    
    SetSkillType(who, STAFF, 2, 10);
		SetSkillType(who, EVALUATING_INTELLIGENCE, 2, 20);
		SetSkillType(who, ALCHEMY, 2, 10);
		SetSkillType(who, MUSICIANSHIP, 2, 10);
		SetSkillType(who, CAMPING, 2, 10);
		SetSkillType(who, DETECTING_HIDDEN, 2, 10);
		SetSkillType(who, HEALING, 2, 10);

		SetSkillType(who, HIDING, 1);
		SetSkillType(who, SKINNING, 1);
		SetSkillType(who, FISHING, 1);
		SetSkillType(who, COOKING, 1);
		SetSkillType(who, LUMBERJACKING, 1);
		SetSkillType(who, MINING, 1);    


	elseif (subclasse == "Feiticeiro")
          SetObjProperty(who, "Vitality", chardata.Vit+5);
          AP_SetTrueStat(who, STRENGTH, chardata.Str+5);
          AP_SetTrueStat(who, DEXTERITY, (chardata.Dex+5));
          AP_SetTrueStat(who, INTELLIGENCE, chardata.Int+5);
		SetSkillType(who, ARMS_LORE ,3, 30);
		SetSkillType(who, ANATOMY, 3, 30);
		SetSkillType(who, HEALING, 3, 30);
		SetSkillType(who, TACTICS, 3, 15);
		SetSkillType(who, RESISTING_SPELLS, 3, 30);
              SetSkillType(who, SWORDSMANSHIP, 3, 25);


		SetSkillType(who, EVALUATING_INTELLIGENCE, 3, 30);
		SetSkillType(who, MAGERY, 2, 20);
		SetSkillType(who, ALCHEMY, 2, 10);
		SetSkillType(who, MUSICIANSHIP, 2, 10);
		SetSkillType(who, CAMPING, 2, 10);
		SetSkillType(who, DETECTING_HIDDEN, 2, 10);

		SetSkillType(who, HIDING, 1);
		SetSkillType(who, SKINNING, 1);
		SetSkillType(who, FISHING, 1);
		SetSkillType(who, WRESTLING, 1);
		SetSkillType(who, LUMBERJACKING, 1);
		SetSkillType(who, MINING, 1);
	endif

endfunction

function GetSkillCap(who, skillname)

	if (!GetObjProperty(who, "chardata"))
		return 100.0;
	endif

	var primarias := GetSkills(who, "primarias");
	var secundarias := GetSkills(who, "secundarias");
	var terciarias := GetSkills(who, "terciarias");

	foreach skill in secundarias
		if (skillname == skill)
			return 70.0;
		endif
	endforeach

	foreach skill in terciarias
		if (skillname == skill)
			return 40.0;
		endif
	endforeach

	foreach skill in primarias
		if (skillname == skill)
			return 100.0;
		endif
	endforeach

	return 0.0;

endfunction

function GetSkills(who, category)

	var grad := 0;
	if (category == "primarias")
		grad := 3;
	elseif (category == "secundarias")
		grad := 2;
	elseif (category == "terciarias")
		grad := 1;
	endif

	if (!who.npctemplate)

		var chardata := GetObjProperty(who, "chardata");
		if ( (!chardata) || (chardata == error))
			return 0;
		else
			var skills := chardata.Skills;
			var skillsName := skills.keys();
			var primarias := array;
			foreach skill in skillsName
				if (skills[skill] == grad)
					primarias.append(skill);
				endif
			endforeach
			return primarias;
		endif
	endif

	return 0;
endfunction

function GetFalseSkillName(skill)

	if (skill == "EvaluatingIntelligence")
		return "Evidance";
	elseif ( skill == "ArmsLore")
		return "ItemLore";
        elseif ( skill == "Camping")
           return "Survival";
      elseif ( skill == "Carpentry")
          return "Woodwork";
      elseif ( skill == "Inscription")
		return "Magic Lore";
	else
		return skill;
	endif

endfunction


function ReduceSkillJob(who)
	
	var chardata := GetObjProperty(who, "chardata");
	var subclasse := GetObjProperty(who, "subclasse");
	var kit := GetObjProperty(who, "kit");

//BOAS (2 Primarias a Secundaria)

	if ((kit == "Ferreiro") || (kit == "Armeiro") || (kit == "Construtor"))
          if ((subclasse == "Bruto") || (subclasse == "Selvagem"))
		SetSkillType(who, MACE_FIGHTING, 2, 10);
		SetSkillType(who, AXE, 2, 10);
          elseif (subclasse == "Cacador")
		SetSkillType(who, SWORDSMANSHIP, 2, 10);
          	SetSkillType(who, AXE, 2, 10);
          elseif (subclasse == "Protetor")
		SetSkillType(who, SWORDSMANSHIP, 2, 10);
		SetSkillType(who, PARRY, 2, 15);
          elseif (subclasse == "Guardiao")
		SetSkillType(who, ARCHERY, 2, 10);
		SetSkillType(who, ANIMAL_LORE, 2, 10);
          elseif (subclasse == "Espadachim")
		SetSkillType(who, SWORDSMANSHIP, 2, 10);
		SetSkillType(who, PARRY, 2, 10);
          elseif (subclasse == "Swashbuckler")
		SetSkillType(who, FENCING, 2, 15);
		SetSkillType(who, TACTICS, 2, 10);
          elseif (subclasse == "Sentinela")
		SetSkillType(who, SWORDSMANSHIP, 2, 10);
          	SetSkillType(who, ARCHERY, 2, 10);
          elseif (subclasse == "Cavaleiro")
		SetSkillType(who, SWORDSMANSHIP, 2, 10);
          	SetSkillType(who, POLE_ARMS, 2, 10);
          elseif (subclasse == "Gladiador")
		SetSkillType(who, SWORDSMANSHIP, 2, 10);
          	SetSkillType(who, MACE_FIGHTING, 2, 10);
          elseif (subclasse == "Bandoleiro")
		SetSkillType(who, SWORDSMANSHIP, 2, 10);
          	SetSkillType(who, FENCING, 2, 10);
          elseif (subclasse == "Assassino")
		SetSkillType(who, BACKSTAB, 2, 10);
          	SetSkillType(who, POISONING, 2, 10);
          elseif (subclasse == "Pirata")
		SetSkillType(who, SWORDSMANSHIP, 2, 10);
          	SetSkillType(who, FENCING, 2, 10);
          elseif (subclasse == "Cacador de Tesouros")
		SetSkillType(who, REMOVE_TRAP, 2, 10);
          	SetSkillType(who, LOCKPICKING, 2, 10);
          elseif (subclasse == "Espiao")
		SetSkillType(who, HIDING, 2, 10);
          	SetSkillType(who, LOCKPICKING, 2, 10);
	   elseif (subclasse == "Mago")
		SetSkillType(who, MAGERY, 2, 10);
              SetSkillType(who, STAFF, 2, 10);
	   elseif (subclasse == "Druida")
		SetSkillType(who, MAGERY, 2, 10);
              SetSkillType(who, ANIMAL_TAMING, 2, 10);
	   elseif (subclasse == "Bruxo")
		SetSkillType(who, MAGERY, 2, 10);
              SetSkillType(who, RESISTING_SPELLS, 2, 10);
	   elseif (subclasse == "Feiticeiro")
		SetSkillType(who, MAGERY, 2, 10);
		SetSkillType(who, SWORDSMANSHIP, 2, 10);
          endif

//MEDIAS (Reduz 1 primaria)

	elseif ((kit == "Carpinteiro") || (kit == "Alfaiate") || (kit == "Inventor") || (kit == "Domador") || (kit == "Curandeiro") || (kit == "Fazendeiro"))
          if( (subclasse == "Bruto") || (subclasse == "Selvagem"))
		SetSkillType(who, AXE, 2, 10);
          elseif (subclasse == "Cacador")
		SetSkillType(who, SWORDSMANSHIP, 2, 10);
          elseif (subclasse == "Protetor")
		SetSkillType(who, PARRY, 2, 15);
          elseif (subclasse == "Guardiao")
		SetSkillType(who, ARCHERY, 2, 10);
          elseif (subclasse == "Espadachim")
		SetSkillType(who, SWORDSMANSHIP, 2, 10);
          elseif (subclasse == "Swashbuckler")
		SetSkillType(who, FENCING, 2, 15);
          elseif (subclasse == "Sentinela")
          	SetSkillType(who, ARCHERY, 2, 10);
          elseif (subclasse == "Cavaleiro")
          	SetSkillType(who, POLE_ARMS, 2, 10);
          elseif (subclasse == "Gladiador")
          	SetSkillType(who, MACE_FIGHTING, 2, 10);
          elseif (subclasse == "Bandoleiro")
          	SetSkillType(who, FENCING, 2, 10);
          elseif (subclasse == "Assassino")
		SetSkillType(who, BACKSTAB, 2, 10);
          elseif (subclasse == "Pirata")
		SetSkillType(who, SWORDSMANSHIP, 2, 10);
          elseif (subclasse == "Cacador de Tesouros")
          	SetSkillType(who, LOCKPICKING, 2, 10);
          elseif (subclasse == "Espiao")
		SetSkillType(who, HIDING, 2, 10);
	   elseif (subclasse == "Mago")
		SetSkillType(who, MAGERY, 2, 10);
	   elseif (subclasse == "Druida")
		SetSkillType(who, MAGERY, 2, 10);
	   elseif (subclasse == "Bruxo")
		SetSkillType(who, MAGERY, 2, 10);
	   elseif (subclasse == "Feiticeiro")
		SetSkillType(who, SWORDSMANSHIP, 2, 10);
          endif

//REGULARES (reduz 1 secundaria)

	elseif ((kit == "Cartografo") || (kit == "Avaliador") || (kit == "Pescador") || (kit == "Cozinheiro") || (kit == "Minerador") || (kit == "Lenhador"))
          if (subclasse == "Bruto") 
		SetSkillType(who, ARMS_LORE, 1);
          elseif (subclasse == "Selvagem")
		SetSkillType(who, ARCHERY, 1);
          elseif (subclasse == "Cacador")
		SetSkillType(who, ARCHERY, 1);
          elseif (subclasse == "Protetor")
		SetSkillType(who, WRESTLING, 1);
          elseif (subclasse == "Guardiao")
		SetSkillType(who, SWORDSMANSHIP, 1);
          elseif (subclasse == "Espadachim")
		SetSkillType(who, ARMS_LORE, 1);
          elseif (subclasse == "Swashbuckler")
		SetSkillType(who, WRESTLING, 1);
          elseif (subclasse == "Sentinela")
		SetSkillType(who, WRESTLING, 1);
          elseif (subclasse == "Cavaleiro")
		SetSkillType(who, WRESTLING, 1);
          elseif (subclasse == "Gladiador")
		SetSkillType(who, HEALING, 1);
          elseif (subclasse == "Bandoleiro")
		SetSkillType(who, WRESTLING, 1);
          elseif (subclasse == "Assassino")
		SetSkillType(who, DETECTING_HIDDEN, 1);
          elseif (subclasse == "Pirata")
		SetSkillType(who, DETECTING_HIDDEN, 1);
          elseif (subclasse == "Cacador de Tesouros")
		SetSkillType(who, ARCHERY, 1);
          elseif (subclasse == "Espiao")
		SetSkillType(who, ARCHERY, 1);
	   elseif (subclasse == "Mago")
		SetSkillType(who, ALCHEMY, 1);
	   elseif (subclasse == "Druida")
		SetSkillType(who, ALCHEMY, 1);
	   elseif (subclasse == "Bruxo")
		SetSkillType(who, ALCHEMY, 1);
	   elseif (subclasse == "Feiticeiro")
		SetSkillType(who, ALCHEMY, 1);
          endif
endif
endfunction
