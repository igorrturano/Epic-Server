use os;
use uo;
use util;
use vitals;

include ":timedScripts:timedScripts";
include "include/client";
include ":ghaia:ghaia";
include "include/damage";

/* magias que os npcs podem castar:

  heal

  lightning
  fireball
  poisoning
  hallucination

  summonundead
  summonair
  summonfire
  summonearth
  fireexplosion
*/

const MOVING_EFFECT_FIREBALL  := 0x36d4;
const MOVING_EFFECT_MAGIC_ARROW  := 0x36e4;
const MOVING_EFFECT_HARM  := 0x36e4;
const MOVING_EFFECT_FLAMESTRIKE := 0x3709;
const MOVING_EFFECT_EBOLT       := 0x379f;
const EFFECT_SPARKLE  := 0x373a;
const EFFECT_RED_SPARKLE  := 0x374a;
const MOVING_EFFECT_TSUNAMI := 0x54ad;

const SOUND_EFFECT_HEAL := 0x1f3;
const SOUND_EFFECT_AGILITY := 0x1e8;
const SOUND_EFFECT_STRENGTH := 0x1ef;
const SOUND_EFFECT_REFLECT := 0x1ee;
const SOUND_EFFECT_POISON := 0x247;
const SOUND_EFFECT_MINDBLAST := 0x214;
const SOUND_EFFECT_FIREBALL  := 0x160;
const sound_effect_curse := 0x1fc;
const SOUND_EFFECT_MAGIC_ARROW	:= 0x01e6;
const SOUND_EFFECT_LIGHTNING  := 0x2a;
const SOUND_EFFECT_HARM := 0x1f2;
const SOUND_EFFECT_FLAMESTRIKE  := 0x209;
const SOUND_EFFECT_FEEBLEMIND := 0x1e5;
const SOUND_EFFECT_EBOLT        := 0x20b;
const SOUND_EFFECT_DISPEL  := 0x02;
const SOUND_EFFECT_CURE := 0x1e1;
const SOUND_EFFECT_CUNNING := 0x1ec;
const sound_effect_clumsy := 0x1e1;
const sound_effect_bless := 0x1eb;
const SOUND_EFFECT_WEAKEN := 0x1e7;


include ":tn:cooldown";

function SpellFight(opponent, spell_list ,alerted := 0)
  if((opponent.serial == me.serial))
    SetWarMode(0);
    opponent := 0;
    return;
  endif

    if (GetCooldown(me, "silence") > 0)
        PrintTextAbove(me, "*Silenciado*");
        return 1;
    endif

  var oldprio := set_priority(50);
  SetOpponent(opponent);
  DisableMainEvents();
  EnableCombatEvents();
  TurnToward(opponent);
  var loops := 0;
  var ev;
  var waittime := 0;

  if (!alerted) //se ele ja nao foi alertado por outro aliado
    if (alert_allies) //se esse npc eh do tipo que alerta
      AlertAllies(me, opponent);
    endif
  endif

   var speech := GetConfigStringArray( npccfgfile[mytemplate], "InitCombatSpeech" );
   if (speech.size() > 0)
	if (RandomInt(100)  < 30) //sempre 30% de chance de falar
		Say(speech[RandomInt(speech.size()+1)]);
	endif
   endif

  SetCooldown(me, "npcspell", 3);
  var dist;

  while(me)

    if  (me.dead)
	break;
    endif

    if (!opponent)
      break;
    endif

    if (opponent.dead)
	var temnego := 0;
	foreach npc in ListMobilesInLineOfSight(me, areasize + 2)
		if( CanFight(me, npc, align) )
			temnego := 1;
			break;
		endif
		sleepms(2);
	endforeach
	if (!temnego)
		var corpo := opponent.GetCorpse();
		var end := ReadGameClock() + 10;
		while (CoordinateDistance(me.x, me.y, corpo.x, corpo.y) < areasize + 5 && end > ReadGameClock())
			RunAwayFromLocation(corpo.x, corpo.y);
			sleepms(3);
		endwhile
	endif
	break;
    endif

    if (opponent.concealed)
      break;
    endif

    if (opponent.hidden)
	break;
    endif

    dist := CoordinateDistance(me.x, me.y, opponent.x, opponent.y);
    if ( dist > 20)
      break;
    endif

    if ( (dist > throwinfo.range) && (throwinfo.canthrow == 1) )
	throwItem(me, opponent, dist);
    endif

    if ( (dist > 6) && ( CInt(npccfgfile[mytemplate].teleport) ) )
	teleport(me, opponent);
    endif

	if(!CloseDistance(opponent))
	  loops := loops + 1;
	  waittime := 0;
	else
	  loops := 0;
	  waittime := 1;
	endif

    //se pode castar spell, manda v
  if (CheckLineOfSight(me, opponent))
    if (GetCooldown(me, "npcspell") == 0)
      if ( (opponent.weapon.attribute == "Archery") || (dist <= 8) )
         sleepms(500);
         CastSpellNPC(opponent, spell_list);
      endif
    endif
  endif

	  if (hasCombat(me))

    DoCombatSpecial(me); ////////////// SPECIALL

  endif

 if (hasAttack(me)) //SPECIAL ATTACK
        var chance := CInt(npccfgfile[mytemplate].SpecialAttackChance); //para mobs do npcdesc
        if (!chance)
          chance := 10;
        endif
        if ((RandomInt(500) <= chance) && (!TS_GetCooldown(me, "npc_special")) )// Acertou!
          DoCombatSpecialNotBoss (me, opponent);
        endif
      endif

	checkFlee(opponent);

	ev := wait_for_event(waittime);
	if(ev.type != SYSEVENT_SPEECH)
	  case (ev.type)
		SYSEVENT_DAMAGED:
      //if((ev.source.npctemplate) and (CheckLineOfSight(me, ev.source)))
       //   opponent := ev.source;
			 // 		        SetOpponent(opponent);
			//		        TurnToward(opponent);
			//se ele receber dano de alguem mais proximo que o oponente ou se receber dano de alguem e o oponente nao estiver acessivel
       if( /*(CoordinateDistance(me.x, me.y, ev.source.x, ev.source.y) < CoordinateDistance(me.x, me.y, opponent.x, opponent.y)) ||*/ (!CheckLineOfSight(me, opponent)) || (!opponent))
					if ( (opponent) && RandomInt(4) == 0 ) //se ainda existir o oponente so tem 25% de chance de mudar de alvo
                if (ev.source)
					        opponent := ev.source;
                  SetOpponent(opponent);
					        TurnToward(opponent);
                endif
           elseif ( (!opponent) || (!CheckLineOfSight(me, opponent))) //mas se nao tiver mais o oponent entao 100% de chance de mudar de alvo
                if (ev.source)
					        opponent := ev.source;
                  SetOpponent(opponent);
					        TurnToward(opponent);
                endif
					endif
        endif
		SYSEVENT_ENGAGED: if(ev.source)
						if(RandomInt(6) == 1)
							opponent := ev.source;
							SetOpponent(opponent);
							TurnToward(opponent);
						endif
			              endif
		EVID_TAUNT:
			if(ev.source)
				opponent := ev.source;
				SetOpponent(opponent);
			    TurnToward(opponent);
			endif
	  endcase
	endif
  endwhile

  DisableCombatEvents();
  EnableMainEvents();
  SetWarMode(0);
  SetOpponent(0);
  sleep(1);
endfunction


function Boss2Fight(opponent, spell_list,alerted := 0)

	if(opponent.serial == me.serial)
		SetWarMode(0);
		opponent := 0;
		return;
	endif

	var oldprio := set_priority(50);
	SetOpponent(opponent);
	DisableMainEvents();
	EnableCombatEvents();
	TurnToward(opponent);
	var loops := 0;
	var ev;
	var waittime := 0;

	var speech := GetConfigStringArray( npccfgfile[mytemplate], "InitCombatSpeech" );
	if (speech.size() > 0)
		if (RandomInt(100)  < 30) //sempre 30% de chance de falar
			Say(speech[RandomInt(speech.size()+1)]);
		endif
	endif

	var dist;
	while( me )

		if (me.dead)
			break;
		endif

		if (!opponent)
			break;
		endif

		if (opponent.dead)
			var temnego := 0;
			foreach npc in ListMobilesInLineOfSight(me, areasize + 2)
				if( CanFight(me, npc, align) )
					temnego := 1;
					break;
				endif
				sleepms(2);
			endforeach
			if (!temnego)
				var corpo := opponent.GetCorpse();
				var end := ReadGameClock() + 10;
				while (CoordinateDistance(me.x, me.y, corpo.x, corpo.y) < areasize + 5 && end > ReadGameClock())
					RunAwayFromLocation(corpo.x, corpo.y);
					sleepms(3);
				endwhile
			endif
			break;
		endif

		if (opponent.concealed)
			break;
		endif

		if (opponent.hidden)
			break;
		endif

		dist := CoordinateDistance(me.x, me.y, opponent.x, opponent.y);
		if ( dist > 20)
			if ( isBloodied(opponent) && TemHabilidade(opponent, "Baixo Limiar de Furia") )
				LogToFile(":fls_core:berserker.log", opponent.name+" ("+opponent.acctname+") peidou pra "+me.name+" em "+opponent.x+" "+opponent.y, LOG_DATETIME);
				var tipo := "thuriao";
				var region := GetRegionName(opponent);
				var coords := {opponent.x, opponent.y};
				var nome := GetObjProperty(opponent, "nome_inicial");
				Start_script(":DiscordWebhook:discord", {tipo, nome, opponent.acctname, me.name, region, coords});
				foreach x in EnumerateOnlineCharacters()
					if (x.cmdlevel > 0)
						SendSysMessageEx(x, opponent.name+" BERSERKER PEIDAO!", SSM_REQUEST);
					endif
				endforeach
			endif
			break;
		endif

		if (hasSpecial(me))
			DoSpecial(me);
		endif

    //se pode castar spell, manda v
  if (CheckLineOfSight(me, opponent))
    if (GetCooldown(me, "npcspell") == 0)
      if ( (opponent.weapon.attribute == "Archery") || (dist <= 8) )
         sleepms(500);
         CastSpellNPC(opponent, spell_list);
      endif
    endif
  endif

	if (!GetCooldown(me, "flamewave"))

      while (coordinatedistance(me.x, me.y, 4926, 1063 ) > 2)
        RunTowardLocation(4926, 1063);
        sleepms(100);
      endwhile

      me.facing := 0;
      Say("*fazendo gestos ritualisticos*");
      sleep(2);
      var mobs := array;
      foreach mob in (ListMobilesInLineOfSight(me, 10 ))
        if (!mob.npctemplate)
          if (!mob.hidden)
            mobs.append(mob);
          endif
        endif
      endforeach

      var walltype := randomint(3);
      var wall;
      if (walltype == 0)
        wall := 0x398c;
      elseif (walltype == 1)
        wall := 0x3915;
      elseif (walltype == 2)
        wall := 0x3967;
      endif

      var i := 0;
      for (i := 0; i < 15; i := i + 1)
        var fire := CreateITemAtLocation( 4919+i, 1058, 22, wall,1, me.realm);
				fire.SetDuration(10);
      endfor

      foreach mob in mobs
        if (mob == me.opponent)
          MoveObjectToLocation(mob,  4926, 1063, 22, me.realm);
        else
          MoveObjectToLocation(mob,  4919 + randomint(10)+1, 1049, 22, me.realm);
        endif
      endforeach

			SetCooldown(me,  "flamewave", 40);
//			say("burn");

    endif

		if(!CloseDistance(opponent))
			loops := loops + 1;
			waittime := 0;
		else
			loops := 0;
			waittime := 1;
		endif

		checkFlee(opponent);

		ev := wait_for_event(waittime);

		if(ev.type != SYSEVENT_SPEECH)
			case (ev.type)
				SYSEVENT_DAMAGED:

			//se ele receber dano de alguem mais proximo que o oponente ou se receber dano de alguem e o oponente nao estiver acessivel
				if( /*(CoordinateDistance(me.x, me.y, ev.source.x, ev.source.y) < CoordinateDistance(me.x, me.y, opponent.x, opponent.y)) ||*/ (!CheckLineOfSight(me, opponent)) || (!opponent))
					if ( (opponent) && RandomInt(4) == 0 ) //se ainda existir o oponente so tem 25% de chance de mudar de alvo
						if (ev.source)
							opponent := ev.source;
							SetOpponent(opponent);
							TurnToward(opponent);
						endif
					elseif ( (!opponent) || (!CheckLineOfSight(me, opponent))) //mas se nao tiver mais o oponent entao 100% de chance de mudar de alvo
						if (ev.source)
							opponent := ev.source;
							SetOpponent(opponent);
							TurnToward(opponent);
						endif
					endif
				endif

				SYSEVENT_ENGAGED:
					if(ev.source)
						if(RandomInt(6) == 1)
							opponent := ev.source;
							SetOpponent(opponent);
							TurnToward(opponent);
						endif
					endif

				EVID_TAUNT:
					if(ev.source)
						opponent := ev.source;
						SetOpponent(opponent);
						TurnToward(opponent);
					endif
				EVID_FLEE:
					if (ev.source)
						flee(ev.source);
					endif
			endcase
		endif
	endwhile

	DisableCombatEvents();
	EnableMainEvents();
	SetWarMode(0);
	SetOpponent(0);
	sleep(1);
endfunction



//essa funcao determina qual spell o npc vai castar
//define se cura ou ataca alguem

function CastSpellNPC(opponent, spell_list)

  var allies := GetAlliesNear(me);
  var enemies := GetEnemiesNear(me);

   var speech := GetConfigStringArray( npccfgfile[mytemplate], "CastCombatSpeech" );
   if (speech.size() > 0)
		Say(speech[RandomInt(speech.size()+1)]);
   endif

  //se sabe magia de heal
  if ( "heal" in spell_list)

    if ((AP_GetVital(me, HITS) /AP_GetVitalMaximumValue(me, HITS) < 0.2))
      SetCooldown(me, "npcspell", 5);
      CastHeal(me, me);
      return;
    endif

    if ( alert_allies == 1)
        foreach ally in allies
          if ((AP_GetVital(ally, HITS) /AP_GetVitalMaximumValue(ally, HITS) < 0.2))
            CastHeal(me, ally);
            SetCooldown(me, "npcspell", 5);
            return;
          endif
        endforeach
    endif

  endif

	//contra varios
	var correntes := "Correntes de dor"in spell_list;
	var sonho := "Sonho macabro"in spell_list;
	//gravemente ferido
	var drenarvida := "Drenar vida" in spell_list;
	//utilidades
	var envenenarespirito := "Envenenar espirito" in spell_list;
	var banimento := "Banimento arcano"in spell_list;
	//primeiro spell que sempre usa
	var retribuicao := "Retribuicao profana" in spell_list;
	//debuff
	var medo := "Medo" in spell_list;
	var enfraquecer := "Enfraquecer" in spell_list;
	//matar o oponente
	var ampliardor := "Ampliar dor" in spell_list;
	var envenenar := "Envenenar" in spell_list;
	var vinganca := "Vinganca recorrente" in spell_list;

	if (retribuicao)
		if (enemies.size() >= 1)
			if (GetCooldown(me, "retribuicao") == 0)
				start_script(":spells:mesum/retribuicao", array{me, me});
				SetCooldown(me, "npcspell", 5);
				SetCooldown(me, "retribuicao", cint(AP_GetSkill(me, MAGICLORE))*2);
			endif
		endif
	endif

	if ( correntes || sonho )
		if (enemies.size() > 3)
			if ( correntes && sonho )
				if (GetCooldown(me, "correntes") == 0 ) //talvez colocar um random
					var loc := struct;
					loc.x := me.x;
					loc.y := me.y;
					loc.z := me.z;
					start_script(":spells:mesum/correntedor", array{me, loc});
					SetCooldown(me, "correntes", cint(AP_GetSkill(me, MAGICLORE))+5);
					SetCooldown(me, "npcspell", 5);
				endif

				if (GetCooldown(me, "sonho") == 0)
					SetCooldown(me, "sonho", cint(AP_GetSkill(me, MAGICLORE))+5);
					start_script(":spells:mesum/sonhomacabro", array{me, enemies[RandomInt(enemies.size())+1]}); //escolher: melee maior dano
					SetCooldown(me, "npcspell", 5);
				endif

			elseif (correntes )
				if (GetCooldown(me, "correntes") == 0 )
					var loc := struct;
					loc.x := me.x;
					loc.y := me.y;
					loc.z := me.z;
					start_script(":spells:mesum/correntedor", array{me, loc});
					SetCooldown(me, "correntes", cint(AP_GetSkill(me, MAGICLORE))+5);
					SetCooldown(me, "npcspell", 5);
				endif
			elseif (sonho)
				if (GetCooldown(me, "sonho") == 0)
					SetCooldown(me, "sonho", cint(AP_GetSkill(me, MAGICLORE))+5);
					start_script(":spells:mesum/sonhomacabro", array{me, enemies[RandomInt(enemies.size())+1]}); //escolher: melee maior dano
					SetCooldown(me, "npcspell", 5);
				endif
			endif
		endif
	endif

	if (sonho && GetCooldown(me, "sonho") < 10)
		foreach mob in enemies
			if ( cint(GetObjProperty(mob, "#sleep")) )
				start_script(":spells:mesum/pesadelo", array{me, mob });
				SetCooldown(me, "npcspell", 5);
				break;
			endif
		endforeach
	endif

    if (GetCooldown(me.opponent, "spellthief"))
        me.opponent := me;
    endif
	if ( (AP_GetVital(me, HITS)/ AP_GetVitalMaximumValue(me, HITS) < 0.3) )
		if (drenarvida)
			start_script(":spells:mesum/drenarvida", array{me, me.opponent}); //escolher: melee mais perto do life maximo sugado
			SetCooldown(me, "npcspell", 5);
		endif
	endif

	//tem duas opcoes, ou ataca o opponent atual ou neutraliza alguem
	if ( (enfraquecer || medo) && (ampliardor || envenenar || vinganca ))
		if (randomint(2) == 0) //ataca oponente
			if (vinganca)
				if ( GetObjPRoperty(me.opponent, "#vinganca") != error ) //se o alvo nao ta sob o efeito ainda
					start_script(":spells:mesum/vinganca", array{me, me.opponent});
					setCooldown(me, "npcspell", 5);
				endif
			endif

			if (envenenar)
				var timers :=  TS_GetTimerNames(me.opponent);
				if ( !("defaultPoison" in timers)  ) //se o alvo nao ta envenenado
					start_script(":spells:mesum/envenenar", array{me, me.opponent});
					setCooldown(me, "npcspell", 5);
				endif
			endif

			if ( ampliardor)
				if ( GetObjProperty(me.opponent, "apodrecer") == error )
					start_script(":spells:mesum/ampliardor", array{me, me.opponent});
					setCooldown(me, "npcspell", 5);
				endif
			endif
			//se ele chegou aqui entao vai testar algum dos utilities spells
		else

			if (medo)
				var possiveis := array;
				foreach m in enemies
					if ( GetObjProperty(m, "#dmgreductionspell") == error )
                        if (GetCooldown(m, "spellthief"))
                            m := me;
                        endif
						possiveis.append(m);
					endif
				endforeach
				start_script(":spells:mesum/medo", array{me, possiveis[RandomInt(possiveis.size()+1)]});
				setCooldown(me, "npcspell", 5);
			endif

			if (enfraquecer)
				var possiveis := array;
				foreach m in enemies
					if ( GetObjProperty(m, "#WeakenMod") == error )
						possiveis.append(m);
					endif
				endforeach
				start_script(":spells:mesum/enfraquecer", array{me, possiveis[RandomInt(possiveis.size()+1)]});
				setCooldown(me, "npcspell", 5);

			endif

		endif
	endif


	if ( envenenarespirito)
		foreach mob in enemies
			if ( GetObjProperty(mob, "classe") == "Sabio" )
				if ( AP_GetVital(mob, MANA)/AP_GetVitalMaximumValue(mob, MANA) > 0.5 ) //pode acabar usando 2x no mesmo target
					start_script(":spells:mesum/envenenarespirito", array{me, mob});
					setCooldown(me, "npcspell", 5);
					break;
				endif
			endif
		endforeach
	endif

	if (banimento)
		foreach mob in enemies
			if (GetObjProperty(mob, "summoned") == 1)
				start_script(":spells:mesum/banimentoarcano", array{me, mob});
				setCooldown(me, "npcspell", 5);
				break;
			endif
		endforeach
	endif



  //se tem mtos inimigos perto casta magia de summon ou de area
if (RandomInt(4) == 0) // 25% de chance
  var area_spells := GetAreaSpells(spell_list);
  if (area_spells.size() >= 1)
    if ( enemies.size() >= (allies.size()+2) )
      var randomspell := area_spells[RandomInt(area_spells.size()) + 1];
      SetCooldown(me, "npcspell", 10);
      case (randomspell)
	"summonundead":
	"summonfire" :
	"summonwater" :
	"summonair" :
	"summonice" :
	"summonearth":  CastSummon(me, randomspell); break;
	"fireexplosion": CastArea(me, "fireexplosion", enemies); break;
      endcase
      return;
    endif
  endif
endif


  //se tem so um inimigo casta alguma de magia de ataque nele
  var directspells := GetDirectSpells(spell_list);

  if (directspells.size() >= 1)
    if (directspells.size() >= 1) //tem q testar 2x :(
        if (GetCooldown(opponent, "spellthief"))
            opponent := me;
        endif
      CastDirect(me, opponent, directspells[randomint(directspells.size()+1)]);
    endif
  endif

endfunction

function CastDirect(caster, cast_on, spellname)


  var dmg := CInt(AP_GetSkill(caster, MAGERY)/2);
  //SkillCheck(cast_on, RESISTING_SPELLS, -1);
  if (dmg < 10)
    dmg := 10;
  endif

  if (GetObjProperty(cast_on, "kit") == "abascanto")
	if (RandomInt(100) < 24)
		SendSysMessageEx(cast_on, "A magia falha em voce.", SSM_INFO);
		dmg := 0;
  endif
  endif

  if (spellname == "lightning")
      SetCooldown(me, "npcspell", 6);
      PlayLightningBoltEffect( cast_on );
      PlaySoundEffect( caster, SOUND_EFFECT_LIGHTNING );
      DamageFLS(cast_on, dmg, "Energy", me);
elseif (spellname == "flamestryke")
      SetCooldown(me, "npcspell", 6);
      PlayObjectCenteredEffect(cast_on, 0x3709, 0x0a, 0x1e );
      PlaySoundEffect( caster, SOUND_EFFECT_FLAMESTRIKE );
      DamageFLS(cast_on, dmg, "Fire", me);
  elseif ( spellname == "fireball")
      SetCooldown(me, "npcspell", 6);
      PlaySoundEffect( caster, SOUND_EFFECT_FIREBALL );
      PlayMovingEffect(caster, cast_on, GFX_LARGE_FIREBALL, 10, 10, 1);
      //PlayMovingEffectXYZ( caster.x, caster.y, caster.z , cast_on.x, cast_on.y, cast_on.z, 0x36d4, 0x18, 0x10, 0);
      DamageFLS(cast_on, dmg, "Fire", me);
  elseif (spellname == "poisoning" && !cast_on.poisoned)
      SetCooldown(me, "npcspell", 3);
      PlayObjectCenteredEffect( cast_on, EFFECT_SPARKLE, 10,10);
      PlaySoundEffect(cast_on, SOUND_EFFECT_POISON);
	  var poisonnivel := CInt(AP_GetSkill(caster, MAGERY)/20);
      TS_StartTimer(cast_on, "defaultPoison", dmg, poisonnivel, caster);
  elseif ( spellname == "hallucination")
      SetCooldown(me, "npcspell", 3);
      PlayObjectCenteredEffect( cast_on, EFFECT_SPARKLE, 10,10);
      PlaySoundEffect(cast_on, SOUND_EFFECT_POISON);
      TS_StartTimer(cast_on, "hallucination", 60, 2, caster);
  elseif ( spellname == "tsunami")
      SetCooldown(me, "npcspell", 6);
      PlaySoundEffect( caster, 0x20e );
      PlayMovingEffect(caster, cast_on, MOVING_EFFECT_TSUNAMI, 5, 10, 0);
      PlayObjectCenteredEffect( cast_on, 0x377a, 10,10);
	  //PlayMovingEffectXYZ( caster.x, caster.y, caster.z , cast_on.x, cast_on.y, cast_on.z, 0x54ad, 0x18, 0x10, 0);
      DamageFLS(cast_on, dmg, "Cold", me);
  endif

endfunction

function CastArea(me, spellname, enemies)

  if (spellname == "fireexplosion")
    PlaySoundEffect(me, 0x231);
    var i := 30;
    while(i > 0)
      var x := me.x + RandomInt(15) - 7;
      var y := me.y + RandomInt(15) - 7;
      var newtarg := CreateItemAtLocation(x, y, me.z, 0xeed, 1);
      newtarg.graphic := 1;
      PlayMovingEffectXYZ( newtarg.x, newtarg.y,(newtarg.z + 80), newtarg.x, newtarg.y, newtarg.z, 0x36d4, 0x18, 0x10, 0);
      i := i - 1;
      DestroyItem(newtarg);
      sleepms(200);
    endwhile
    RainDamage(enemies);
  endif

endfunction

function RainDamage(enemies)

    foreach targ in enemies
      PlayObjectCenteredEffect(targ, 0x36cb, 7, 0x10);
      PlaySoundEffect(targ, 0x228);
      DamageFLS(targ, 40, "Fire", me);
    endforeach

endfunction

function CastSummon(me, spell)
  if (spell == "summonundead")
    CreateNpcFromTemplate(":brainai:skeleton3", me.x, me.y, me.z, 0, me.realm);
    CreateNpcFromTemplate(":brainai:skeleton3", me.x, me.y, me.z, 0, me.realm);
    CreateNpcFromTemplate(":brainai:skeleton3", me.x, me.y, me.z, 0, me.realm);
  elseif ( spell == "summonfire" )
    CreateNpcFromTemplate(":brainai:fireelemental", me.x, me.y, me.z, 0, me.realm);
  elseif ( spell == "summonair")
    CreateNpcFromTemplate(":brainai:airelemental", me.x, me.y, me.z, 0, me.realm);
  elseif ( spell ==  "summonearth")
    CreateNpcFromTemplate(":brainai:earthelemental", me.x, me.y, me.z, 0, me.realm);
  elseif ( spell ==  "summonice")
	CreateNpcFromTemplate(":brainai:snowelemental", me.x, me.y, me.z, 0, me.realm);
  elseif( spell == "summonspider1")
    CreateNpcFromTemplate(":brainai:DreadGiantSpider", me.x, me.y, me.z, 0, me.realm);
    CreateNpcFromTemplate(":brainai:DreadGiantSpider", me.x, me.y, me.z, 0, me.realm);
  elseif( spell == "summonspider2")
    CreateNpcFromTemplate(":brainai:BlackHugeSpider", me.x, me.y, me.z, 0, me.realm);
    CreateNpcFromTemplate(":brainai:BlackHugeSpider", me.x, me.y, me.z, 0, me.realm);
    endif
endfunction

function CastHeal(me, cast_on)

    var healed := CInt(AP_GetSkill(me, MAGERY)/2);
    HealFLS(cast_on, healed);
    PlayObjectCenteredEffect( cast_on, EFFECT_SPARKLE, 7, 0x10 );
    PlaySoundEffect( cast_on, SOUND_EFFECT_HEAL );

endfunction

function GetAreaSpells(spell_list)

  var direct := array{};
  foreach spell in spell_list
    if (spell == "summonundead")
      direct.append(spell);
    elseif (spell == "summonfire");
      direct.append(spell);
    elseif (spell == "summonwater")
      direct.append(spell);
    elseif (spell == "summonair")
      direct.append(spell);
    elseif (spell == "summonearth")
      direct.append(spell);
    elseif (spell == "summonice")
      direct.append(spell);
    elseif (spell == "fireexplosion")
      direct.append(spell);
    endif
  endforeach


  return direct;

endfunction

function GetDirectSpells(spell_list)

  var direct := array{};
  foreach spell in spell_list
    if (spell == "poisoning")
      direct.append(spell);
    elseif (spell == "lightning");
      direct.append(spell);
    elseif (spell == "fireball")
      direct.append(spell);
    elseif (spell == "hallucination")
      direct.append(spell);
    elseif (spell == "flamestryke")
      direct.append(spell);
	elseif (spell == "tsunami")
      direct.append(spell);
    endif
  endforeach

  return direct;

endfunction
