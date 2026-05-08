// Phase 4 (revisado) — Census on-demand.
// Substitui o daemon coinCensus.src. Chamado pelo moneyer no double-click
// e por .mintstate. Lazy refresh: só roda se virou novo dia desde último.
//
// Custo: ~1s na primeira chamada do dia. Gratuito nas seguintes.
// Race: 2 chamadas simultâneas no mesmo dia rodam 2x, last writer wins.
// Aceitável (1s desperdiçado raramente).

use uo;
use os;
use util;
use polsys;

include "include/epicEconomyAudit";
include ":currency:include/currency";
include ":currency:include/seignorage";
include ":containers:storageAreas";
include ":accounts:accounts";

const CENSUS_GLOBAL_LAST_DAY := "coin_census_last_day";

function CountCoinsInPlayer(p, byref total_cobre, byref total_prata, byref total_ouro)
	if (!p || p == error || p.acctname == error || !p.acctname)
		return 0;
	endif

	var bag := p.backpack;
	if (bag && bag != error)
		total_cobre := total_cobre + CInt(CountCoins(bag, EA_COIN_COBRE));
		total_prata := total_prata + CInt(CountCoins(bag, EA_COIN_PRATA));
		total_ouro  := total_ouro  + CInt(CountCoins(bag, EA_COIN_OURO));
	endif

	var bank := CP_GetStorageContainerForMobile(p, CP_DEFAULT_AREA, CP_NOCREATE);
	if (bank && bank != error)
		total_cobre := total_cobre + CInt(CountCoins(bank, EA_COIN_COBRE));
		total_prata := total_prata + CInt(CountCoins(bank, EA_COIN_PRATA));
		total_ouro  := total_ouro  + CInt(CountCoins(bank, EA_COIN_OURO));
	endif

	return 1;
endfunction

function RunCoinCensus()
	var total_cobre := 0;
	var total_prata := 0;
	var total_ouro  := 0;

	foreach acct_name in (ListAccounts())
		var acct := FindAccount(acct_name);
		if (!acct || acct == error)
			continue;
		endif
		if (acct.banned)
			continue;
		endif
		var i;
		for (i := 1; i <= 6; i := i + 1)
			var p := acct.GetCharacter(i);
			if (p && p != error)
				CountCoinsInPlayer(p, total_cobre, total_prata, total_ouro);
			endif
			sleepms(2);
		endfor
		sleepms(2);
	endforeach

	var today_key := StrFormatTime("%d-%m-%Y", POLCore().systime);
	return UpdateSupplySnapshot(today_key, total_cobre, total_prata, total_ouro);
endfunction

// API pública. Chamar antes de qualquer leitura de supply.
// No-op se census do dia já rodou.
function EnsureSupplyFresh()
	var today_key := StrFormatTime("%d-%m-%Y", POLCore().systime);
	var last_day := CStr(GetGlobalProperty(CENSUS_GLOBAL_LAST_DAY));
	if (last_day == today_key)
		return 1;
	endif
	RunCoinCensus();
	SetGlobalProperty(CENSUS_GLOBAL_LAST_DAY, today_key);
	return 1;
endfunction
