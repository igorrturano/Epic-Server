use uo;
use os;
use vitals;
use math;

include "include/say";
include ":gumps:gumps";
include ":attributes:attributes";
include ":timedscripts:timedScripts";
include "include/utils";
include ":tn:tngumps";
include ":charactercreation:habilidades";
include ":gumps:epicGumps";

function UpdateXPBar(who)
   if (!GetObjProperty(who, "chardata")) 
      return 0;
   endif
   
   const XPBAR_GUMP_ID := 0xE7B; // Fixed gump ID for XP bar
   
   // First ensure any existing process is stopped
   _checkPid(who, "xpbar");
   
   var xpbar := GetObjProperty(who, "xpbar");
   if (!xpbar)
      xpbar := struct{
         pos_x := 105,
         pos_y := 480
      };
      SetObjProperty(who, "xpbar", xpbar);
   endif

   while (who.connected)
      var gump := BuildXPGump(who, xpbar, XPBAR_GUMP_ID);
      var input := SendDialogGump(who, gump.layout, gump.data, gump.base.x, gump.base.y, 0, XPBAR_GUMP_ID);

      if (input[0] == 2) // Move gump button
         // First close the current gump
         CloseGump(who, XPBAR_GUMP_ID);
         sleepms(50);
         
         var new_pos := PositionGump(who, xpbar.pos_x, xpbar.pos_y);
         xpbar.pos_x := new_pos[1];
         xpbar.pos_y := new_pos[2];
         SetObjProperty(who, "xpbar", xpbar);
      endif

      sleepms(100);
      xpbar := GetObjProperty(who, "xpbar");
   endwhile
endfunction

function BuildXPGump(who, xpbar, gumpid)
   var gump := GFCreateGump(0, 0);
   GFMovable(gump, 0);
   GFDisposable(gump, 0);
   GFClosable(gump, 0);
   GFSetId(gump, gumpid); // Set the fixed gump ID

   var curr_xp := CInt(GetObjProperty(who, "Experience"));
   var curr_level := CInt(GetObjProperty(who, "Level"));
   var next_level_xp := CalculateNextLevelXP(curr_level);

   // Calculate progress percentage
   var progress := CDbl(curr_xp) / CDbl(next_level_xp) * 100;
   var width := CInt((progress * 597) / 100);
   
   // Background bar
   GFResizePic(gump, xpbar.pos_x+10, xpbar.pos_y+64, 9200, 597, 40);
   
   // Progress fill
   if (width > 0)
      GFResizePic(gump, xpbar.pos_x+10, xpbar.pos_y+64, 9350, width, 40);
   endif

   // Level text
   GFTextLine(gump, xpbar.pos_x+18, xpbar.pos_y+80, 1152, "Level {}".format(curr_level));
   
   // XP Progress text
   var xp_text := "{}/{}".format(curr_xp, next_level_xp);
   GFTextLine(gump, xpbar.pos_x+500, xpbar.pos_y+74, 1152, xp_text);

   // Move button
   GFAddButton(gump, xpbar.pos_x+14, xpbar.pos_y+67, 2103, 2103, GF_CLOSE_BTN, 2);

   return gump;
endfunction

// Calculate XP needed for next level using Fibonacci (1,3,5,8,13...)
function CalculateNextLevelXP(level)
   if (level == 0)
      return 1000; // 1k for first level
   elseif (level == 1) 
      return 3000; // 3k for second level
   elseif (level == 20)
      return 0;
   endif
   
   var prev := 2;
   var curr := 3;
   for i := 2 to level
      var next := prev + curr;
      prev := curr;
      curr := next;
   endfor
   
   return curr * 1000;
endfunction

function AwardXPFromKill(killers, corpse)
 	var template := GetObjProperty(corpse, "npctemplate"); // Set by the core.
	var npc_cfg := NPC_GetNPCConfig(template);

	var tier := npc_cfg.Tier;
   var xp_gain := 5;
	if (tier)
		xp_gain := tier * 25;
   endif

  // Bônus por grupo
   if (killers.size() > 1)
      xp_gain := xp_gain * (1 + killers.size() * 0.1);
   endif

   // Divide entre os participantes
   xp_gain := CInt(xp_gain / killers.size());
   foreach killer in killers
   	AddXP(killer, xp_gain);
   endforeach

   return 1;
endfunction

// Add XP and check for level up
function AddXP(who, amount)
   var curr_xp := CInt(GetObjProperty(who, "Experience"));
   var curr_level := CInt(GetObjProperty(who, "Level")) ? CInt(GetObjProperty(who, "Level")) : 0;

   // if level 20, then do nothing
   if (curr_level == 20)
      return 0;
   endif
   
   // Add XP
   curr_xp += amount;
   SetObjProperty(who, "Experience", curr_xp);
   
   // Check for level up
   var next_level_xp := CalculateNextLevelXP(curr_level);
   
   if (curr_xp >= next_level_xp)
      LevelUp(who);
   endif

   // Update XP bar display
	var result := start_script(":expbar:expbar", who);

   return 1;
endfunction

// Handle level up effects and rewards
function LevelUp(who)
   var curr_level := CInt(GetObjProperty(who, "Level") + 1);
   SetObjProperty(who, "Level", curr_level);

   // Create scroll reward
   // if curr_level is even, give scroll, if is odd
   if (curr_level % 2 == 0)
      var scroll := CreateItemInBackpack(who, "pergaminho_"+curr_level);
      if (scroll)
         SetObjProperty(scroll, "owner", who.serial);
         scroll.color := 2117;
         scroll.newbie := 1;
      endif
   else
      var chardata := GetObjProperty(who, "chardata");
      var hp_up := 0;
      if (chardata.classe == GUERREIRO)
         hp_up := 10;
      elseif (chardata.classe == LADINO)
         hp_up := 8;
      elseif (chardata.classe == SABIO)
         hp_up := 6;
      endif
      var hitsmod := Cint(GetObjProperty(who, "hitsmod"));
      hitsmod := hitsmod + hp_up;
      SetObjProperty(who, "hitsmod",  hitsmod);
    
      RecalcVitals(who);
   endif

   // Visual and audio effects
   PlaySoundEffect(who, 0x1F3);
   PlayObjectCenteredEffect(who, 0xB462, 10, 60);
   
   // Floating text
   PrintTextAbove(who, "*LEVEL UP!*", _DEFAULT_TEXT_FONT, 1152);
   AP_RefreshVitals(who);

   SendSysMessage(who, "Voce alcancou o nivel "+curr_level+"!");

   discrete_gump(who);
   return 1;
endfunction

function handleExpGain(corpse)
	var nearby := ListMobilesNearLocationEx(corpse.x, corpse.y, LIST_IGNORE_Z, 16, LISTEX_FLAG_HIDDEN + LISTEX_FLAG_NORMAL, corpse.realm);
	var killers := array{};
	foreach killer in nearby
		if (killer.acctname)
			killers.append(killer);
		endif
	endforeach
	AwardXPFromKill(killers, corpse);
endfunction

function discrete_gump(who)
   // Definir ID único para o gump
   var gump_id := 0x1E7E1;
   // Fechar qualquer gump anterior com esse ID
   CloseGump(who, gump_id);
   // Criar novo gump
   var gump := GFCreateGump();
   if (!gump)

       return 1;
   endif
   
   GFSetId(gump, gump_id);
   GFClosable(gump, 1);
   GFDisposable(gump, 0);
   GFMovable(gump, 0);

   // Definir posição e tamanho
   var width := 200;
   var height := 100;
   // Posição atualizada conforme solicitado
   var x := 30;
   var y := 40;

   // Adicionar imagens personalizadas
   GFGumpPic(gump, x + 3, y + 7, 2520);
   GFGumpPic(gump, x + 41, y + 6, 2521);
   GFGumpPic(gump, x + 209, y + 6, 2522);
   GFGumpPic(gump, x + 42, y + 42, 2527);
   GFGumpPic(gump, x + 4, y + 41, 2526);
   GFGumpPic(gump, x + 210, y + 42, 2528);

   // Adicionar texto
   GFTextMid(gump, x + 45, y + 17, width, 1152, "LEVEL UP!");
   GFTextMid(gump, x + 45, y + 37, width, 1152, "Clique para distribuir pontos");

   // Adicionar botão que cobre toda a área
   GFAddButton(gump, x + 30, y + 17, 2117, 2118, GF_CLOSE_BTN, 1);
   
   // Enviar gump para o jogador
   var result := GFSendGump(who, gump);
   // Verificar se o botão foi clicado
   if (result[0] == 1)
      // Definir constantes necessárias
      var GUMP_WIDTH := 440;
      var GUMP_HEIGHT := 565;
      
      // Criar o gump e mostrar a página de habilidades diretamente
      var hab_gump := GFECreateGump("Menu de Habilidades", GUMP_WIDTH, GUMP_HEIGHT);
      ShowHabilidadesPage(hab_gump, who);
      AddNavigationBar(hab_gump, 2);
      
      // Enviar o gump para o jogador
      var hab_result := GFSendGump(who, hab_gump);
   endif
endfunction