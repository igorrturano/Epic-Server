use uo;
use os;
use cfgfile;
use util;

include "include/client";
include ":charactercreation:habilidades";
const UOBJECT_EMPTY_BOTTLE := 0x0f0e;

//TODO: Ver se futuramente precisa remover essa função, talvez tenha virado Dead Code
	//Analisando bem, talvez o arquivo inteiro tenha virado (conferir melhor)
function EsfriarLiga(user, item)
  var hasestanho, hascobre;
  var num := CINT(GetObjProperty(item, "numMetal"));
  if (num)
    if (num == 1)
      PlaySoundEffect(user, 0x23f);
      sleepms(1500);
      var amt := CINT(GetObjProperty(item, "amt1"));
      var ingot := GetObjProperty(item, "oreObj1");
      if (ingot == "carvao")
        SendSysMessage(user, "Voce apenas estragou carvao!", FONT_NORMAL, 40);
      else
        SendSysMessage(user, "Voce esfriou o metal derretido e fez as barras.", FONT_NORMAL, 90);
        CreateItemInBackpack(user, ingot, amt);
        SendSysMessage(user, ""+TemHabilidade(user, "Tesouro do Lixo"));
        if (TemHabilidade(user, "Tesouro do Lixo"))
          smeltaImpurezasRaras(user, amt);
        endif
      endif
      EraseObjProperty(item, "ingType");
      EraseObjProperty(item, "numMetal");
      EraseObjProperty(item, "amt1");
      EraseObjProperty(item, "oreObj1");
      item.color := 951;
    else
      var metals := {};
      var metal;
      var i := 1;
      while ( i<=num) //pega a lista de metais que tem no pot
		metal := GetObjProperty(item, "oreObj"+i);
		metals.append(metal);
		i := i +1;
      endwhile
      PlaySoundEffect(user, 0x23f);
      sleepms(1500);
      var amt := CINT(GetObjProperty(item, "amt1"));
      var ingot := DeterminaLiga(metals, user);

      if (ingot == "noIngot")
        deleteProp(item);
        item.color := 951;
        SendSysMessage(user, "Voce nao conseguiu fazer uma liga com esses metais", FONT_NORMAL, 40);
        return;
      endif

      SendSysMessage(user, "Voce esfriou a liga metalica derretida e fez as barras.", FONT_NORMAL, 90);
      item.color := 951;
      deleteProp(item);
      if (ingot == 0x6317) //metal liquido, nao foi implementado no shard ainda mas quandofor
                //eh so consertar isso
        var hasbottle:= 0;
        foreach item in EnumerateItemsInContainer(user.backpack)
          if(item.objtype == UOBJECT_EMPTY_BOTTLE)
          SubtractAmount(item, 1);
          hasbottle := 1;
          break;
          endif
        endforeach
        if (hasbottle)
          CreateItemInBackpack(user, ingot, amt);
        else
          SendSysMessage(user, "Voce deixou o metal liquido derramar no chao.", FONT_NORMAL, 40);
          return;
        endif
      else
        CreateItemInBackpack(user, ingot, amt);
        SendSysMessage(user, ""+TemHabilidade(user, "Tesouro do Lixo"));
        if (TemHabilidade(user, "Tesouro do Lixo"))
          smeltaImpurezasRaras(user, amt);
        endif
      endif
    endif
  endif
endfunction

function deleteProp(item)
  var num := CINT(GetObjProperty(item, "numMetal"));
  eraseobjproperty(item, "numMetal");
  var i := 1;
  while (i <= num)
        EraseObjProperty(item, "amt"+i);
        EraseObjProperty(item, "oreObj"+i);
        i := i +1;
  endwhile
endfunction

function DeterminaLiga(metals, who)
	var metais := array;
  foreach item in metals
     case(Cint(item))
        0x1bde: metais.append("carvao");
        0x6309: metais.append("ferro");
        0x62ff: metais.append("impuro");
        0x6465: metais.append("cobre");
        0x6991: metais.append("estanho");
        0x6460: metais.append("frigutium");
        0x6310: metais.append("galvorn");
        0x6311: metais.append("mithril");
        0x6315: metais.append("illyra");
        0x630b: metais.append("fulmetium");
        0x630c: metais.append("venetium");
        0x630d: metais.append("ignium");
        0x6993: metais.append("prata");
        0x630a: metais.append("litan");
        0x630e: metais.append("golden");
        0x6995: metais.append("aco");
     endcase
  endforeach
  print("metais: " + metais);
  if (metais.size() == 2)
    // if (("ferro"  in metais ) && ("carvao" in metais) ) // TODO: Verificar como fazer barra de ferro aceitavel aqui
    //   return 0x6995; //aco
    // elseif (("prata" in metais) && ("estanho" in metais))
    //   return 0x6603; //cristalith
    if (("cobre"  in metais ) && ("estanho" in metais))
      return 0x6994; //bronze
    elseif (("carvao"  in metais ) && ("impuro" in metais))
      return 0x6309; //ferro
    elseif (("carvao"  in metais ) && ("ferro" in metais))
      return 0x6995; //aco
    // elseif (("litan" in metais) && ("estanho" in metais))
    //   return 0x6601; //stalith
    // elseif (("galvorn" in metais) && ("fulmetium" in metais))
    //   return 0x6998; //teslium
    // elseif (("galvorn" in metais) && ("venetium" in metais))
    //   return 0x6999; //hissium
    // elseif (("galvorn" in metais) && ("ignium" in metais))
    //   return 0x6997; //solium
    // elseif (("galvorn" in metais) && ("frigutium" in metais))
    //   return 0x699A; //icelum
    // elseif (("prata" in metais) && ("fulmetium" in metais))
    //   return 0x6609; //lunium
    else
      return "noIngot";
    endif
  elseif (metais.size() == 3)
    if (("aco" in metais) && ("carvao" in metais)  && ("ignium" in metais))
      return 0x630d; //ignium
    elseif (("aco" in metais) && ("carvao" in metais)  && ("frigutium" in metais))
      return 0x6606; //frigutium
    elseif (("aco" in metais) && ("carvao" in metais)  && ("venetium" in metais))
      return 0x6607; //venetium
    elseif (("aco" in metais) && ("carvao" in metais)  && ("fulmetium" in metais))
      return 0x6608; //fulmetium
    // elseif (("aco" in metais) && ("carvao" in metais)  && ("ferro" in metais))
    //   return 0x6602; //acielith
    // elseif (("cobre" in metais) && ("prata" in metais)  && ("golden" in metais))
    //   return 0x6604; //platina
    else
      return "noIngot";
    endif
  elseif (metais.size() == 5)
    if(("mithril" in metais) && ("fulmetium" in metais) && ("venetium" in metais) && ("ignium" in metais) && ("frigutium" in metais))
      return 0x699B; //oricalcum
    else
      return "noIngot";
    endif
  else
    return "noIngot";
  endif
endfunction

function smeltaImpurezasRaras(who, amt)
  var achou := 0;
  if(amt >= 40) //VALORES SETADOS PARA 40 PARA EVITAR POSSÍVEL PROBLEMA - SETAR PARA 20 CASO DÊ TUDO CERTO!
    var rand := RandomFloat(200);
    if(rand<0.1)
      SendSysMessage(who, "Achou um pouco de Mithril em meio ao minerios.");
      CreateItemInBackPack(who, 0x6311, CInt(amt/40)); // return 95; //mithril
      achou := 1;
    elseif(rand<0.2)
      SendSysMessage(who, "Achou um pouco de Galvorn em meio ao minerios.");
      CreateItemInBackPack(who, 0x6310, CInt(amt/40)); // return 90; //galvorn
      achou := 1;
    elseif(rand<0.4)
      SendSysMessage(who, "Achou um pouco de Fulmetium  em meio ao minerios.");
      CreateItemInBackPack(who, 0x630b, CInt(amt/40)); // return 80; //fulmetium
      achou := 1;
    elseif(rand<0.6)
      SendSysMessage(who, "Achou um pouco de Venetium em meio ao minerios.");
      CreateItemInBackPack(who, 0x630c, CInt(amt/40)); // return 80; //venetium
      achou := 1;
    elseif(rand<0.8)
      SendSysMessage(who, "Achou um pouco de Ignium em meio ao minerios.");
      CreateItemInBackPack(who, 0x630d, CInt(amt/40)); // return 80; //ignium
      achou := 1;
    elseif(rand<1.0)
      SendSysMessage(who, "Achou um pouco de frigutium em meio ao minerios.");
      CreateItemInBackPack(who, 0x6460, CInt(amt/40)); // return 80; //frigutium
      achou := 1;
    elseif(rand<1.2)
      SendSysMessage(who, "Achou um pouco de litan em meio ao minerios.");
      return CreateItemInBackPack(who, 0x630a, CInt(amt/40)); // return 70; //litan
    elseif(rand<1.4)
      SendSysMessage(who, "Achou um pouco de Ouro em meio ao minerios.");
      CreateItemInBackPack(who, 0x630e, CInt(amt/40)); // return 40; //golden
      achou := 1;
    endif
  endif
  if(achou)
    SkillCheck(who, GATHERING, -1);
  endif
  return achou;
endfunction