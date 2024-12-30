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

function UpdateXPBar(who)
   if (!GetObjProperty(who, "chardata")) 
      return 0;
   endif
   
   CloseGump(who, 0xE7B);
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
      var gump := BuildXPGump(who, xpbar);
      var input := GFSendGump(who, gump);

      if (input[0] == 2) // Move gump button
         var new_pos := PositionGump(who, xpbar.pos_x, xpbar.pos_y);
         xpbar.pos_x := new_pos[1];
         xpbar.pos_y := new_pos[2];
         SetObjProperty(who, "xpbar", xpbar);
      endif

      sleepms(100);
      xpbar := GetObjProperty(who, "xpbar");
   endwhile
endfunction

function BuildXPGump(who, xpbar)
   var gump := GFCreateGump(0, 0);
   GFMovable(gump, 0);
   GFDisposable(gump, 0);
   GFClosable(gump, 0);
   GFSetId(gump, 0xE7B);

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
	if (!tier)
		tier := 1;
    endif

   var xp_gain := tier * 25;
   foreach killer in killers
	 xp_gain := Cint(xp_gain / killers.size());
   	 AddXP(killer, xp_gain);
   endforeach

   return 1;
endfunction

// Add XP and check for level up
function AddXP(who, amount)
   var curr_xp := CInt(GetObjProperty(who, "Experience"));
   var curr_level := CInt(GetObjProperty(who, "Level")) ? CInt(GetObjProperty(who, "Level")) : 0;
   
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
      SetObjProperty(scroll, "owner", who.serial);
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