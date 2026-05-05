// Phase 4 — Seignorage core.
//
// Pool: 3 buckets per coin type (cobre/prata/ouro), accumulator only.
// Pool injection happens via InjectToPool when moneyer/gemvendor pays.
// V1 não tem drainage — pool só acumula. Phase 5+ pode drenar (eventos GM, fake-coin compensation).
//
// Supply: total cobre-equivalente lido do datafile coinSupply (atualizado pelo cron).
// Bootstraps a 0 se datafile vazio (sistema novo / pre-launch).

use uo;
use os;
use cfgfile;
use datafile;
use polsys;

include "include/epicConstants";
include "include/epicEconomyAudit";

const SUPPLY_DATAFILE := ":currency:coinSupply";
const POOL_DATAFILE   := ":currency:coinPool";

// Peso em cobre-equivalente de cada moeda
function CoinWeight(coin_type)
	case (coin_type)
		SESTERTIUS:  return 1;
		DENARIUS:    return 10;
		AUREUS:      return 100;
	endcase
	return 0;
endfunction

function CoinPoolKey(coin_type)
	case (coin_type)
		SESTERTIUS:  return "cobre";
		DENARIUS:    return "prata";
		AUREUS:      return "ouro";
	endcase
	return "";
endfunction

// Lê total cobre-eq do snapshot mais recente (today se existir, senão último disponível).
function GetCurrentSupply()
	var df := OpenDataFile(SUPPLY_DATAFILE);
	if (!df || df == error)
		return 0;
	endif
	var keys := df.Keys();
	if (!keys.size())
		return 0;
	endif
	// Keys são datas DD-MM-YYYY. O cron escreve em ordem; pega a última.
	var latest := keys[keys.size()];
	var elem := df.FindElement(latest);
	if (!elem || elem.errortext)
		return 0;
	endif
	return CInt(elem.GetProp("TotalCobreEq"));
endfunction

// Brackets ordenados — leitura preguiçosa, cacheada no escopo do programa via static var.
function ReadMintConfig()
	var cfg := ReadConfigFile(":currency:config/mint");
	if (!cfg || cfg.errortext)
		return 0;
	endif
	return cfg;
endfunction

function GetSupplyTarget()
	var cfg := ReadMintConfig();
	if (!cfg)
		return 15000;
	endif
	var elem := FindConfigElem(cfg, "General");
	if (!elem || elem.errortext)
		return 15000;
	endif
	var t := CInt(elem.SupplyTarget);
	return (t > 0) ? t : 15000;
endfunction

function GetRefuseRatio()
	var cfg := ReadMintConfig();
	if (!cfg)
		return 3.0;
	endif
	var elem := FindConfigElem(cfg, "General");
	if (!elem || elem.errortext)
		return 3.0;
	endif
	var r := CDbl(elem.RefuseRatio);
	return (r > 0.0) ? r : 3.0;
endfunction

function GetRefuseMessage()
	var cfg := ReadMintConfig();
	if (!cfg)
		return "Já temos moedas suficientes em circulação.";
	endif
	var elem := FindConfigElem(cfg, "General");
	if (!elem || elem.errortext)
		return "Já temos moedas suficientes em circulação.";
	endif
	var msg := CStr(elem.RefuseMessage);
	if (!msg)
		return "Já temos moedas suficientes em circulação.";
	endif
	return msg;
endfunction

// Retorna array de brackets ordenados por Order ASC.
// Cada elem: struct{ name, order, upper, rate }
function GetBrackets()
	var cfg := ReadMintConfig();
	if (!cfg)
		return array{};
	endif
	var brackets := array{};
	foreach key in (GetConfigStringKeys(cfg))
		var elem := FindConfigElem(cfg, key);
		if (!elem || elem.errortext)
			continue;
		endif
		// Skip non-Bracket sections (General etc).
		if (CDbl(elem.Upper) <= 0.0)
			continue;
		endif
		var b := struct;
		b.+name := key;
		b.+order := CInt(elem.Order);
		b.+upper := CDbl(elem.Upper);
		b.+rate := CDbl(elem.Rate);
		brackets.append(b);
	endforeach
	// Sort by order ascending (insertion sort, brackets are <10 items)
	var i, j;
	for (i := 2; i <= brackets.size(); i := i + 1)
		var key := brackets[i];
		j := i - 1;
		while (j >= 1 && brackets[j].order > key.order)
			brackets[j + 1] := brackets[j];
			j := j - 1;
		endwhile
		brackets[j + 1] := key;
	endfor
	return brackets;
endfunction

// Verifica se a economia está em estado de RECUSA (ratio >= refuseRatio).
function IsMintRefused()
	var supply := GetCurrentSupply();
	var supply_target := GetSupplyTarget();
	if (supply_target <= 0)
		return 0;
	endif
	var ratio := CDbl(supply) / CDbl(supply_target);
	return (ratio >= GetRefuseRatio());
endfunction

// Verifica se um depósito específico ATRAVESSARIA o threshold de refuse.
// Diferente de IsMintRefused() (que checa estado pre), este antecipa o post.
function WouldCrossRefuse(coin_type, qty)
	if (qty <= 0)
		return 0;
	endif
	var weight := CoinWeight(coin_type);
	if (weight <= 0)
		return 0;
	endif
	var supply_pre := GetCurrentSupply();
	var supply_post := supply_pre + (qty * weight);
	var supply_target := GetSupplyTarget();
	if (supply_target <= 0)
		return 0;
	endif
	var refuse_supply := GetRefuseRatio() * CDbl(supply_target);
	return (CDbl(supply_post) > refuse_supply);
endfunction

// Walk pelos brackets entre supply_pre e supply_post (em cobre-eq).
// Retorna total de coin units (em cobre-eq) que o player recebe.
function WalkBrackets(supply_pre, supply_post, supply_target)
	var brackets := GetBrackets();
	if (!brackets.size())
		return supply_post - supply_pre;  // sem brackets = 1:1
	endif
	var coins_out := 0.0;
	var current := CDbl(supply_pre);
	var remaining := CDbl(supply_post - supply_pre);
	foreach b in brackets
		if (remaining <= 0.0)
			break;
		endif
		var bracket_upper_supply := b.upper * CDbl(supply_target);
		if (current >= bracket_upper_supply)
			continue;
		endif
		var capacity := bracket_upper_supply - current;
		var chunk := (remaining < capacity) ? remaining : capacity;
		coins_out := coins_out + (chunk * b.rate);
		current := current + chunk;
		remaining := remaining - chunk;
	endforeach
	// Sobra (acima do último bracket): cap em refuse — não deveria acontecer
	// se IsMintRefused foi checado antes, mas defensivo.
	return coins_out;
endfunction

// API principal: dado coin_type e qty (em unidades de coin_type), retorna effective rate.
// Phase 1 stub retornava 1.0 sempre. Agora faz bracket walk real.
function GetMintRate(coin_type := 0, qty := 0)
	if (!coin_type || qty <= 0)
		return 1.0;
	endif
	var weight := CoinWeight(coin_type);
	if (weight <= 0)
		return 1.0;
	endif
	var supply_impact := qty * weight;
	var supply_pre := GetCurrentSupply();
	var supply_post := supply_pre + supply_impact;
	var supply_target := GetSupplyTarget();

	var coins_out_eq := WalkBrackets(supply_pre, supply_post, supply_target);
	if (supply_impact <= 0)
		return 1.0;
	endif
	// effective_rate = coins_out / supply_impact (ambos em cobre-eq)
	return CDbl(coins_out_eq) / CDbl(supply_impact);
endfunction

// Adiciona ao pool do tipo de moeda. Pool[coin] += amount.
function InjectToPool(coin_type, amount)
	if (amount <= 0)
		return 0;
	endif
	var key := CoinPoolKey(coin_type);
	if (!key)
		return 0;
	endif
	var df := OpenDataFile(POOL_DATAFILE);
	if (!df || df == error)
		df := CreateDataFile(POOL_DATAFILE, 0);
	endif
	var elem := df.FindElement(key);
	if (!elem || elem.errortext)
		elem := df.CreateElement(key);
	endif
	var current := CInt(elem.GetProp("Amount"));
	elem.SetProp("Amount", current + CInt(amount));
	return 1;
endfunction

// Lê saldo atual do pool por moeda (debugging / GM cmd).
function GetPoolAmount(coin_type)
	var key := CoinPoolKey(coin_type);
	if (!key)
		return 0;
	endif
	var df := OpenDataFile(POOL_DATAFILE);
	if (!df || df == error)
		return 0;
	endif
	var elem := df.FindElement(key);
	if (!elem || elem.errortext)
		return 0;
	endif
	return CInt(elem.GetProp("Amount"));
endfunction

// Atualização do snapshot diário — chamada pelo cron.
function UpdateSupplySnapshot(today_key, total_cobre, total_prata, total_ouro)
	var df := OpenDataFile(SUPPLY_DATAFILE);
	if (!df || df == error)
		df := CreateDataFile(SUPPLY_DATAFILE, 0);
	endif
	var elem := df.FindElement(today_key);
	if (!elem || elem.errortext)
		elem := df.CreateElement(today_key);
	endif
	var total_eq := CInt(total_cobre) + CInt(total_prata) * 10 + CInt(total_ouro) * 100;
	elem.SetProp("TotalCobre", CInt(total_cobre));
	elem.SetProp("TotalPrata", CInt(total_prata));
	elem.SetProp("TotalOuro", CInt(total_ouro));
	elem.SetProp("TotalCobreEq", total_eq);
	return total_eq;
endfunction
