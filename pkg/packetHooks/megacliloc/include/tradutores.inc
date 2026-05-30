function TranslateCritical(crit)
	if (crit == "damage3")
		return "Dano Triplo";
	elseif (crit == "damage2")
		return "Dano Duplo";
	elseif ( crit == "stun")
		return "Ataque Atordoante";
	elseif (Crit == "bleed")
		return "Alvejar Ponto Vital";
    elseif (Crit == "armorpierce")
        return "Ignorar Armadura";
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
        // Legacy + renamed skills (Smithing/Carpentry/etc. resolve here too
        // via cfg Alias, so we map both spellings just in case the raw new
        // name is passed in).
        "Metal_Working": return "Ferreiro";
        "Smithing":      return "Ferreiro";
        "Wood_Working":  return "Carpinteiro";
        "Carpentry":     return "Carpinteiro";
        "Sneak":         return "Furtividade";
        "Stealth":       return "Furtividade";
        "Pickpocketing": return "Punguista";
        "Theft":         return "Punguista";
        "Awareness":     return "Perceptivo";
        "Perception":    return "Perceptivo";

        "Alchemy":   return "Alquimista";
        "Magery":    return "Mago";
        "Survival":  return "Sobrevivencialista";
        "Medicine":  return "Médico";
        "Mechanics": return "Mecânico";
        "Theology":  return "Teologia";
        "Science":   return "Ciência";
        "Arcana":    return "Arcana";

        // Legacy split parents (kept during migration window)
        "Domestic_Work": return "Trabalhador Doméstico";
        "Gathering":     return "Coletor";
        "Handle_Animal": return "Adestrador de Animais";

        // Path-rework split skills
        "Tailoring":     return "Costureiro";
        "Provisioning":  return "Provedor";
        "Mining":        return "Minerador";
        "Lumberjacking": return "Lenhador";
        "Taming":        return "Domador";
        "Beastmastery":  return "Mestre de Bestas";

        default: return profissao;
    endcase
endfunction