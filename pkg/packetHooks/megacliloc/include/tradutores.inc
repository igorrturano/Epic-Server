function TranslateCritical(crit)
	if (crit == "damage3")
		return "Dano Triplo";
	elseif (crit == "damage2")
		return "Dano Duplo";
	elseif (crit == "armorignore")
		return "Ignorar Armadura";
	elseif ( crit == "stun")
		return "Ataque Atordoante";
	elseif (Crit == "bleed")
		return "Alvejar Ponto Vital";
	endif

endfunction

function TranslateHit(crit)
	if (crit == "antimountedhit")
		return "Anti-montaria";
	elseif (crit == "chargehit")
		return "Carga";
	elseif (crit == "concussionhit")
		return "Concussão";
	elseif ( crit == "freearmorhit")
		return "Movimento Livre";
	elseif (crit == "heighthit")
		return "Diferença de Altura";
	elseif ( crit == "QueimaRoupa")
		return "Tiro a Queima Roupa";
	elseif ( crit == "pushback")
		return "Golpe Esmagador";
	elseif ( crit == "passivedisarm")
		return "Disarme Passivo";
	endif
endfunction

function TraduzirProfissao(profissao)
    case (profissao)
        "Metal_Working": return "Ferreiro";
        "Alchemy": return "Alquimista";
        "Magery": return "Mago";
        "Survival": return "Sobrevivencialista";
        "Medicine": return "Médico";
        "Domestic_Work": return "Trabalhador Doméstico";
        "Mechanics": return "Mecânico";
        "Wood_Working": return "Carpinteiro";
        "Gathering": return "Coletor";
        "Handle_Animal": return "Adestrador de Animais";
        default: return profissao;
    endcase
endfunction