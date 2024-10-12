
function EstimulaAbdomen(npc, event)
   if(npc.graphic == 28 && !npc.warmode)
      if(getCooldown(npc, "produzTeia") == 0)
         PrintTextAbove(npc,"*Você estimula o abdomen da aranha e consegue extrair alguns fiapos de teia.*");
         var idleSound := PlaySoundEffect(npc, 388);
         CreateItemInBackpack(event.source, 0xe07, 10); // fiapos de teia
         setCooldown(npc,"produzTeia",30);
      else
         PrintTextAbove(npc,"*Você tenta extrair algo, mas só consegue causar desconforto no aracnídeo que se distância imediatamente.*");
         WalkAwayFrom(event.source);
      endif
      sleep(1);
      return 1;
   endif
endfunction