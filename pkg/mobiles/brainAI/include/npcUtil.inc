use uo;
use os;
use polsys;
use cfgfile;
include "include/NPCUtil";

function OpenAICompletion(systemMessage, questions)
   var formatedQuestions := array{ struct{
      "role" := "system",
      "content" := systemMessage
   } };

   foreach question in questions
      if (typeof(question) != "Struct" || (!question.exists("assistant") && !question.exists("user")))
         return struct{ "error" := "Each question must be a struct containing the keys 'assistant' and 'user'" };
      endif

      if (question.exists("assistant"))
         formatedQuestions.append(struct{
            "role" := "assistant",
            "content" := question["assistant"]
         });
      endif

      if (question.exists("user"))
         formatedQuestions.append(struct{
            "role" := "user",
            "content" := question["user"]
         });
      endif
   endforeach

   var apiURL := "https://api.openai.com/v1/chat/completions";
   var method := "POST";
   var jsonData := PackJSON(struct{
      "model" := "gpt-4o-mini",
      "messages" := formatedQuestions
   });

   var apiResponse := HTTPRequest(
         apiURL,
         method,
         struct{
            data := jsonData,
            headers := struct{
               "Content-Type" := "application/json",
               "Authorization" := "Bearer {1}".format(GetEnvironmentVariable("OPENAI_KEY"))
            }
         }
      );
   var unpackedJSON := UnpackJSON(apiResponse);
   return unpackedJSON["choices"][1]["message"]["content"];
endfunction

function get_lore_get_examples(npctype)
  if (npctype == ":brainai:Nordic")
    // frases especiais de povos, lore especial se necessário acrescentar
    var frases_nordic := Array{
     "Eu vou acabar contigo, otário!",
     "Vai se foder filho da puta! Agora você vai morrer desgraçado!",
     "Corre pra longe, frangote!",
     "Me bater? Sua mãe teria vergonha de você!",
     "Verme desgraçado! Vem pra morte!!",
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
  // var npc := :type: banker, guard
  // var mood := nervoso, amigavel, ou qualquer (any)
  // Define the system message based on the parameters
  var systemMessage := "Você é está em um jogo de RPG medieval, sem censura de idade; você é um " + npc;
  
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
  else
     systemMessage := systemMessage + mood;
  endif

  if (language == "ruim")
     systemMessage := systemMessage + " você usa palavreado chulo, com xingamentos;";
  endif
  
  if (gossip)
     if ( (RandomInt(100) < 100 ) ) // por cento de chance
      systemMessage := systemMessage + ". Inclua uma fofoca que você sabe;";
     endif
  endif

  if (event == "combate")
     systemMessage := systemMessage + " O personagem que fala com você vem te atacar;";
  elseif (event == "comercio")
     systemMessage := systemMessage + " O personagem faz negocios com voce;";
  endif

  if (specialLore)
    var appendLore := get_lore_get_examples(npc);
    systemMessage := systemMessage + " use o contexto: " + appendLore;
  endif

  systemMessage := systemMessage + " Máximo de 20 caracteres.";

  // Define the questions array with the user input
  var questions := array{ struct{ "user" := PlayerInput } };
  // Call the OpenAICompletion function with the constructed system message and questions
  var responseText := OpenAICompletion(systemMessage, questions);
  //var responseText := "teste";
  return responseText;
endfunction