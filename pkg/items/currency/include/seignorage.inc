// Phase 4 + 4.5 — Seignorage core.
//
// Pool: 3 buckets per coin type (cobre/prata/ouro). Counter virtual — não há
// coin físico armazenado. Tax destrói coin no momento do depósito; pool é
// budget que NPCs podem usar pra mintar drops bonus (até daily budget).
//
// InjectToPool incrementa o counter (representa tax acumulada).
// DrawFromPool decrementa e mintra coin nova (Phase 4.5 — drops bonus em mob).
// Excesso acima do daily budget fica "preso" no counter — destruição efetiva.
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

// Lê saldo atual do pool por moeda (debugging / GM cmd). Clampa negativos pra 0.
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
	var amount := CInt(elem.GetProp("Amount"));
	return (amount < 0) ? 0 : amount;
endfunction

// Phase 4.5 — Identificação de TrueBoss (multiplicador 5x na chance de drop).
function IsTrueBoss(npc)
	if (!npc || npc == error)
		return 0;
	endif
	var flag := GetObjProperty(npc, "true_boss");
	if (flag == error || !flag)
		return 0;
	endif
	return 1;
endfunction

// Phase 4.5 — Lê config do PoolDrop por moeda do mint.cfg.
// Retorna struct { chance, dice, min_tier, daily_budget } ou 0 se não configurado.
function GetPoolDropConfig(coin_type)
	var key := CoinPoolKey(coin_type);
	if (!key)
		return 0;
	endif
	var cfg := ReadMintConfig();
	if (!cfg)
		return 0;
	endif
	// Block name: "Cobre" / "Prata" / "Ouro" (capitalize first letter)
	var elem_name := Upper(key[1, 1]) + key[2, len(key)];
	var elem := FindConfigElem(cfg, elem_name);
	if (!elem || elem.errortext)
		return 0;
	endif
	var c := struct;
	c.+chance := CDbl(elem.Chance);
	c.+dice := CStr(elem.DiceRoll);
	c.+min_tier := CInt(elem.MinTier);
	c.+daily_budget := CInt(elem.DailyBudget);
	c.+trueboss_mult := CDbl(elem.TrueBossMultiplier);
	if (c.trueboss_mult <= 0.0)
		c.trueboss_mult := 5.0;
	endif
	return c;
endfunction

// Phase 4.5 — Drena do pool com 3 camadas de proteção contra valor negativo
// + daily budget cap (reset por data). Retorna quantidade efetivamente drenada.
function DrawFromPool(coin_type, max_amount)
	if (max_amount <= 0)
		return 0;
	endif
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

	// (1) Camada: pool atual (clamp negativos pra 0 antes de calcular).
	var current := CInt(elem.GetProp("Amount"));
	if (current <= 0)
		return 0;
	endif

	// Daily budget tracking: reset DrainedToday se virou dia.
	var today_key := StrFormatTime("%d-%m-%Y", POLCore().systime);
	var drain_day := CStr(elem.GetProp("DrainDay"));
	var drained_today := CInt(elem.GetProp("DrainedToday"));
	if (drain_day != today_key)
		drained_today := 0;
		elem.SetProp("DrainDay", today_key);
	endif

	var cfg := GetPoolDropConfig(coin_type);
	var daily_budget := 0;
	if (cfg)
		daily_budget := cfg.daily_budget;
	endif

	if (daily_budget > 0)
		var remaining_budget := daily_budget - drained_today;
		if (remaining_budget <= 0)
			return 0;
		endif
		if (max_amount > remaining_budget)
			max_amount := remaining_budget;
		endif
	endif

	// Drena: actual = min(max_amount, current).
	var actual := (max_amount < current) ? max_amount : current;
	var new_value := current - actual;
	// (3) Camada defensiva: clamp em 0 mesmo em race condition.
	if (new_value < 0)
		new_value := 0;
	endif
	elem.SetProp("Amount", new_value);
	elem.SetProp("DrainedToday", drained_today + actual);
	return actual;
endfunction

// Phase 4.5 — Lê quanto já foi drenado hoje (mintstate cmd).
function GetDrainedToday(coin_type)
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
	var today_key := StrFormatTime("%d-%m-%Y", POLCore().systime);
	var drain_day := CStr(elem.GetProp("DrainDay"));
	if (drain_day != today_key)
		return 0;
	endif
	return CInt(elem.GetProp("DrainedToday"));
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
