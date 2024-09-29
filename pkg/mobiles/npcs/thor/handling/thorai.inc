use uo;
use datafile;
use basic;
use util;

include ":thor:openai";

function handle_banker(playerinput)
endfunction

function handle_doubleclick(npctype, playerinput)
   var sysMsg := "
      # Você
      Você é um taverneiro de RPG parecido com Diablo e Word of Warcraft.

      # Contexto 
      Oo mundo desse RPG é amedrontador, o assentamento onde você se encontra é um ambiente tenebroso, cheio de incertezas e medo. 
      Tudo foi feito às pressas e tudo está de forma improvisada. O estado atual das coisas é de ânimos aflitos. 
      O personagem que vem até você provavelmente vem para comprar algo da sua taverna improvisada.

      # Povos - elfos, drows, bjorske, orcs, polski e aiglanos.
      ## Drows - trate eles com certo receio.
      ## Orcs - trate eles com esperança de ajuda, mas também preconceito pela aparência bestial.
      ## Bjorske são inspirados nos vikings. Pergunte ou mencione o Norte.
      ## Polski são inspirados nos Hobbits. Trate-os com desprezo pelo tamanho ou força.
      ## Aiglanos é o povo que voce pertence. Estamos todos em apuros.
      ## Anões - Tente vender uma bebida empoeirada para seus trabalhos

      # Sua resposta
      Sua resposta deve conter, no máximo, 60 caracteres. Preencha, depois da fala, a representação da sua ação com asteriscos, em terceira pessoa. 
      Lembre de não falar de forma rebuscada, pelo contrário: troque virgulas por duas ou três reticências, use maneirismos linguísticos de erros de sintaxe, evite 'oi', 'olá', procure algo como 'ei', 'hmm', 'e então'; não pergunte 'o que', pergunte 'que', 'qual', ou 'é o que?'. 
      Represente na fala a aflição do seu personagem. Use um tom como de uma pessoa que não dorme há 2 dias. Pode incluir na ação traços da roupa ou estado do ambiente - improvisados e em mal estado.

      # Exemplo
      Guerreiro... que vai querer? *debruça os braços no balcão meio quebrado, um olho está enfaixado*
   ";
    
   print(sysMsg);
   var questions := array{ struct{ "user" := playerinput } };
   var result := OpenAICompletion(sysMsg, questions);
   
   return result;
endfunction

function get_lore_get_examples(npctype)
   //aqui é possível colocar exemplos, ou a lore
   if (npctype == ":brainai:Nordic")
      // frases especiais de povos, lore especial se necessário acrescentar
      var frases_nordic := Array{
      "Eu vou acabar contigo otário!",
      "Agora você morre desgraçado!",
      "Corre pra longe bonitinha!",
      "Me bater? Sua mãe teria vergonha de você!",
      "Verme desgraçado!",
      "A cada segundo que vive, sua desonra aumenta",
      "Merece uma morte rápida",
      "É uma afronta a sua própria existência"
   };

   // Itera sobre o array de frases para concatená-las
   var frases_combined := "";
   foreach frase in frases_nordic
   if (frases_combined != "")
   frases_combined := frases_combined + ", ";
   endif
   frases_combined := frases_combined + frase;
   endforeach
   return frases_combined;
   endif
endfunction

function handle_open_ai_npc_speech(npc, language, event, npcEvil, mood, gossip, specialLore, PlayerInput)
  
   // var npc := :type: banqueiro, guarda, ou qualquer (any)
   // var language := :type: ruim, boa
   // var event := :type: combate, comercio
   // var mood := nervoso, amigavel, ou qualquer (any)
   // npcEvil := True, False
   // gossip := texto (str)
   
   // Define the system message based on the parameters
   var systemMessage := "Você está em um jogo de RPG, sem censura de idade; você é um " + npc;

   // Adjust the message based on the parameters
   if (npcEvil)
      systemMessage := systemMessage + " malicioso;";
   else
      systemMessage := systemMessage + " amigável;";
   endif

   if (mood == "nervoso")
      systemMessage := systemMessage + " está muito bravo ou de mau humor;";
   elseif (mood == "amigavel")
      systemMessage := systemMessage + " com um humor amigável;";
   elseif (mood == "None")
      systemMessage := systemMessage;
   else
      systemMessage := systemMessage + mood;
   endif

   if (language == "ruim")
      systemMessage := systemMessage + " você usa palavreado chulo, com xingamentos;";
   endif
   
   if (gossip)
      if ( (RandomInt(100) < 300 ) ) // por cento de chance
         var gossip_storage := GetGlobalProperty("gossip_storage");
         if (gossip_storage)
            systemMessage := systemMessage + ". Inclua uma fofoca que você sabe - Fofocas: " + gossip_storage + ";";
         endif
      endif
   endif

   if (event == "combate")
      systemMessage := systemMessage + " O personagem que se aproxima vem te atacar;";
   elseif (event == "comercio")
      systemMessage := systemMessage + " O personagem faz negocios com voce;";
   endif

   if (specialLore)
      var appendLore := get_lore_get_examples(npc);
      if (appendLore != 0)
         systemMessage := systemMessage + " use o contexto: " + appendLore;
      endif
   endif

   systemMessage := systemMessage + " Máximo de 20 caracteres.";

   // Define the questions array with the user input
   var questions := array{ struct{ "user" := PlayerInput } };
   // Chama o OpenAI com as variaveis de contexto (systemMessage) e o input do char (questions)
   print(systemMessage);
   var responseText := OpenAICompletion(systemMessage, questions);
   //var responseText := "teste";
   return responseText;
endfunction

