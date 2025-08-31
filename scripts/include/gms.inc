use uo;
include "/include/epicPropNames";

enum CHAVES_PREFERENCIA_GM
	 PREFERENCIA_GM_IS_INDESTRUTIVEL_PADRAO := "isCreateIdestrutivelPadrao",
	 PREFERENCIA_GM_IS_INDISPENSAVEL_PADRAO := "isCreateIndispensavelPadrao"
endenum

function AtualizaPreferenciaGm(gm, preferencia,valor)
	var propPreferencias := GetObjProperty(gm, PROPNAME_PREFERENCIAS_GM);
	if(!propPreferencias )
		propPreferencias := dictionary{ preferencia -> valor};
	else
		propPreferencias.insert(preferencia, valor);
	endif

	SetObjProperty(gm, PROPNAME_PREFERENCIAS_GM, propPreferencias);
endfunction

function PegaPreferenciaGm(gm, preferencia)
	var propPreferencias := GetObjProperty(gm, PROPNAME_PREFERENCIAS_GM);

	if(!propPreferencias || TypeOfInt(propPreferencias[preferencia]) == OT_UNINIT )
		return _PegaPadroesPreferenciaGm(preferencia);
	endif
	return propPreferencias[preferencia];
endfunction

function RemovePreferenciaGm(gm, preferencia)
	var propPreferencias := GetObjProperty(gm, PROPNAME_PREFERENCIAS_GM);
	return propPreferencias.erase(preferencia);
endfunction

function LimpaPreferenciasGm(gm)
	EraseObjProperty(gm, PROPNAME_PREFERENCIAS_GM);
endfunction

function isGm( quem)
	return quem.cmdlevel >= 1;
endfunction 

function _PegaPadroesPreferenciaGm(preferencia)
	if(preferencia == PREFERENCIA_GM_IS_INDISPENSAVEL_PADRAO)
		return true;
	elseif(preferencia == PREFERENCIA_GM_IS_INDESTRUTIVEL_PADRAO)
		return false;
	else
		return  error { errortext := "Preferência não definida." };
	endif
endfunction