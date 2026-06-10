use uo;
use math;
use cfgfile;

//include "include/bank";
include ":datafile:include/datafile";
include ":merchants:/include/item_template";
include ":timeutils:time";
include ":spells:SpellRegistry";

function GetPaymentsDataFile()
	var data_file := DFOpenDataFile(":merchants:payments", DF_CREATE);
	return data_file;
endfunction

function GetPaymentsDataElem(elem_name)
	var data_file := GetPaymentsDataFile();
	var data_elem := DFFindElement(data_file, elem_name, DF_CREATE);
	
	return data_elem;
endfunction

function SetMerchantGump(merchant, gump_text)
	var data_elem := GetPaymentsDataElem(Hex(merchant.serial));
   data_elem.SetProp("gump", gump_text);
endfunction

function GetMerchantGump(merchant)
	var data_elem := GetPaymentsDataElem(Hex(merchant.serial));
	var merchant_gump := data_elem.GetProp("gump");
   
   return merchant_gump;
endfunction

function AddPaymentForSeller(merchant, seller_serial, cost_amnt, guild_prop := 0)
	var data_elem := GetPaymentsDataElem(Hex(merchant.serial));
	var my_payments := data_elem.GetProp("my_payments");
	if ( !my_payments )
		my_payments := array{};
	endif
	
	var seller_account := data_elem.GetProp(Hex(seller_serial));
	if ( !seller_account )
		seller_account := array{};
	endif

	seller_account.Append(cost_amnt);

   if (guild_prop)
      var guild := guild_prop[1];
      var guild_money := Ceil(cost_amnt * guild_prop[2]);

      guild.SetProp("money", cint(guild.getProp("money")) + guild_money);
   endif
	
	cost_amnt := Cint(cost_amnt*0.1);
	if(cost_amnt > 0)
		my_payments.Append(cost_amnt);
	endif

	data_elem.SetProp("my_payments", my_payments);
	data_elem.SetProp(Hex(seller_serial), seller_account);
	// var money_stored := cint(data_elem.GetProp("money_stored"));
   
endfunction

function GetSellerAccount(merchant, seller_serial)
	var data_elem := GetPaymentsDataElem(Hex(merchant.serial));
	var account := data_elem.GetProp(Hex(seller_serial));
	if( !account )
		account := array{};
	endif

	return account;
endfunction

function GetMerchantPayments(merchant, seller_serial)
	var data_elem := GetPaymentsDataElem(Hex(merchant.serial));
	var payments := data_elem.GetProp("my_payments");
	if ( !payments )
		payments := array{};
	endif
	
	return payments;
endfunction

function RemoveMerchantPayment(merchant, seller_serial, payment_index)
	var data_elem := GetPaymentsDataElem(Hex(merchant.serial));
	var payments := data_elem.GetProp("my_payments");
	if ( !payments )
		payments := array{};
	endif
	
	payments.Erase(payment_index);
	
	if ( payments.size() == 0 )
		RemoveMerchantPayments(merchant, seller_serial);
	else
		data_elem.SetProp("my_payments", payments);
	endif
	
	return payments;
endfunction

function RemoveMerchantPayments(merchant, seller_serial)
	var data_file := GetPaymentsDataFile();
	var data_elem := GetPaymentsDataElem(Hex(merchant.serial));
	
	data_elem.EraseProp("my_payments");
	
	if ( DFGetElemProps(data_elem).size() == 0 )
		data_file.DeleteElement(Hex(merchant.serial));
	endif
endfunction
	

//---=[ Merchant NPC specific functions ]=-----------------------------
function GetMerchantDataFile()
	//Tracks merchant elems with props listing what they are selling
	//When a sale is made, the payment goes to the payments data elem.
	//If the seller no longer exists, the items just get destroyed.
	var data_file := DFOpenDataFile(":merchants:merchants", DF_CREATE);

	return data_file;
endfunction

function GetMerchantDataElem(elem_name)
	var data_file := GetMerchantDataFile();
	var data_elem := DFFindElement(data_file, elem_name, DF_CREATE);
	
	return data_elem;
endfunction

//
// Returns a list of numbers in reference to cprop keys and items being sold.
//
function GetItemsSoldList(merchant)
	var data_elem := GetMerchantDataElem(Hex(merchant.serial)+"-ItemsSelling");
	var prop_list :=  DFGetElemProps(data_elem);
	//Reversed so that LastNum is more likely to be found first
	//and makes newer items show up at the top, and older items further down.
	prop_list.Reverse();
	
	foreach key in prop_list
		if ( key == "LastNum" )
			prop_list.Erase(_key_iter);
			break;
		endif
		sleepms(4);
	endforeach
	return prop_list;
endfunction

//
// Returns information about a particular item being sold.
//
function GetItemSoldInfo(merchant, item_num)
	var data_elem := GetMerchantDataElem(Hex(merchant.serial)+"-ItemsSelling");
	
	return data_elem.GetProp(CStr(item_num));
endfunction

//
// Adds an item to the merchant's ItemsSelling list.
// Moves item to the merchant's GetSellingItemsBox()
//
function AddItemForSale(merchant, seller, item, cost_type, cost_name, cost_amt, desc, cost_lote, infiniteAmount := 0, specialProp := 0, commission:=0)
	var data_elem := GetMerchantDataElem(Hex(merchant.serial)+"-ItemsSelling");
	var new_num := FindAvailableNumber(data_elem);
	if ( new_num == error )
		var temp := error;
		temp.+errortext := "Error in AddItemForSale() - Unable to allocate a slot. 255 item limit reached.";
		return temp;
	endif
	
	// capture_contents=1: se for container, snapshot recursivo dos itens
	// dentro para que a venda preserve o conteúdo. Sem isso, DestroyItem(item)
	// abaixo apaga a bag E todos os filhos junto, deixando o comprador com
	// uma bag vazia (ou pior, perdendo itens).
	var template := CreateItemTemplate(item, 1);
	if ( template == error )
		return template;
	endif

	data_elem.SetProp("LastNum", new_num);
	new_num := CStr(new_num);

	var cost_info := struct;
	cost_info.+seller := CInt(seller.serial);
	cost_info.+cost_type := CInt(cost_type);
	cost_info.+cost_name := cost_name;
	cost_info.+cost_amt := CInt(cost_amt);
	cost_info.+commission := commission;
	cost_info.+desc := desc;
   cost_info.+lote := CInt(cost_lote);
	cost_info.+time := polcore().systime;
	cost_info.infinite := cint(infiniteAmount);
	if (specialProp)
		cost_info.+specialProp := specialProp;
	endif

	//sendsysmessage(seller, "cost_info" + cost_info);
	
	var sale_info := array{cost_info, template};
			
	data_elem.SetProp(new_num, sale_info);
	DestroyItem(item);
	
	return new_num;
endfunction

//
// Returns the next available item number in the elem.
//
function FindAvailableNumber(data_elem)
	const MIN_VALUE := 0xFF;   // 255
	const MAX_VALUE := 0x1C6;  // 454 — range real = 200 slots (era 0x14A = 76 com comentário mentindo "100")

	var new_num := CInt(data_elem.GetProp("LastNum"));
	if ( new_num < MIN_VALUE )
		return MIN_VALUE;
	elseif ( new_num >= MAX_VALUE )
		var i := MIN_VALUE;
		for ( i; i<=MAX_VALUE; i:=i+1 )
			if ( !data_elem.GetProp(CStr(i)) )
				return i;
			endif
			sleepms(2);
		endfor
		// Tudo ocupado mesmo após varrer o range — vendor lotado de verdade.
		return error;
	else
		return new_num+1;
	endif
endfunction

//
// Removes an item from the ItemsSelling list.
//
function RemoveItemForSale(merchant, item_num, amount := 0, confiscar := 0)
   var data_elem := GetMerchantDataElem(Hex(merchant.serial)+"-ItemsSelling");
	
   var selling_info := data_elem.GetProp(Cstr(item_num));  
   if ( cint(selling_info[1].infinite) == 1 && !confiscar)
      return 1;
   elseif (amount == 0 || selling_info[2].amount <= amount)
      return data_elem.EraseProp(CStr(item_num));
   endif
   
   selling_info[2].desc := StrReplace(selling_info[2].desc, "{}".format(selling_info[2].amount), "{}".format(selling_info[2].amount - amount));
   selling_info[2].amount -= amount;
   data_elem.SetProp(Cstr(item_num), selling_info);
   return 1;
endfunction

// =============================================================================
// Sistema de devolução de itens não-vendidos
// =============================================================================
// Itens parados por mais de 7 dias no vendor são arquivados (não destruídos).
// O template do item vai para o datafile :merchants:returns indexado pelo
// serial do seller. Na próxima vez que o seller logar, gump_returns notifica
// e devolve os itens ao backpack (ou no chão como fallback).

const MERCHANT_ITEM_MAX_AGE := SECONDS_IN_DAY * 7;

function GetReturnsDataFile()
	return DFOpenDataFile(":merchants:returns", DF_CREATE);
endfunction

function GetReturnsDataElem(seller_serial)
	var df := GetReturnsDataFile();
	return DFFindElement(df, Hex(CInt(seller_serial)), DF_CREATE);
endfunction

function GetPendingReturns(seller_serial)
	var elem := GetReturnsDataElem(seller_serial);
	var pending := elem.GetProp("pending");
	if ( !pending )
		return array{};
	endif
	return pending;
endfunction

function ClearPendingReturns(seller_serial)
	var elem := GetReturnsDataElem(seller_serial);
	elem.EraseProp("pending");
endfunction

// Move o slot `key` do merchant para a fila de devolução do seller. Não destrói.
// Retorna 1 em sucesso, 0 em falha (slot inválido ou seller sem serial).
function ArchiveItemForReturn(merchant, key)
	var info := GetItemSoldInfo(merchant, key);
	if ( !info || info == error )
		return 0;
	endif

	var cost := info[1];
	var template := info[2];
	var seller_serial := CInt(cost.seller);
	if ( !seller_serial )
		return 0;
	endif

	var elem := GetReturnsDataElem(seller_serial);
	var pending := elem.GetProp("pending");
	if ( !pending )
		pending := array{};
	endif

	pending.append(struct{
		"template"      := template,
		"npc_name"      := merchant.name,
		"npc_serial"    := CInt(merchant.serial),
		"archived_time" := polcore().systime
	});
	elem.SetProp("pending", pending);

	// Remove o slot no merchant.
	var data_elem := GetMerchantDataElem(Hex(merchant.serial)+"-ItemsSelling");
	data_elem.EraseProp(CStr(key));

	// Notifica o seller (online ou offline) via ObjProperty no mobile.
	var seller := SystemFindObjectBySerial(seller_serial, SYSFIND_SEARCH_OFFLINE_MOBILES);
	if ( seller && seller != error )
		SetObjProperty(seller, "notif_vendor_returns", 1);
	endif

	return 1;
endfunction

//
// Garbage collector: arquiva (não destrói) itens parados há mais de 7 dias.
// Itens com infinite=1 (lojas GM) nunca expiram.
//
function RemoveOldItemsForSale(merchant)
	var selling_list := GetItemsSoldList(merchant);
	var now := polcore().systime;

	foreach key in selling_list
		var info := GetItemSoldInfo(merchant, key);
		if ( !info || info == error )
			sleepms(2);
			continue;
		endif
		var cost := info[1];

		if ( CInt(cost.infinite) == 1 )
			sleepms(2);
			continue;
		endif

		if ( (now - CInt(cost.time)) > MERCHANT_ITEM_MAX_AGE )
			ArchiveItemForReturn(merchant, key);
		endif
		sleepms(2);
	endforeach
endfunction

function SetMerchantLog(merchant, buyer_name, item_name, seller_name, cost)
	var data_elem := GetPaymentsDataElem(Hex(merchant.serial));
	var my_sellings := data_elem.GetProp("sell_log");
   if (!my_sellings) my_sellings := array{}; endif
   my_sellings.Insert(1, "{} vendeu {} para {} por {} moedas".format(seller_name, item_name, buyer_name, cost));

   if (my_sellings.size() > 20)
      my_sellings.Erase(my_sellings.size());
   endif

   data_elem.SetProp("sell_log", my_sellings);
endfunction

function GetMerchantLog(merchant)
	var data_elem := GetPaymentsDataElem(Hex(merchant.serial));
	var my_sellings := data_elem.GetProp("sell_log");
   return my_sellings;
endfunction

// =============================================================================
// Resolução de label para o card do vendor
// =============================================================================
// Receitas e pergaminhos carregam um item.desc genérico ("instrucoes de
// fabricacao de metal", "Pergaminho de Magia") e guardam a identidade real
// (qual receita, qual feitiço) em CProps. Esta função consulta essas CProps
// e devolve um label legível, com fallback seguro para item.desc.
function ResolveSellerCardLabel(item)
	if ( !item || item == error )
		return "";
	endif

	var cprops := item.CProps;
	if ( cprops && cprops != error )
		// Receitas: CProp "recipe" guarda a key em :recipes:recipes.cfg
		var recipeKey := cprops["recipe"];
		if ( recipeKey )
			var recipeCfg := ReadConfigFile(":recipes:recipes");
			if ( recipeCfg )
				var recipeElem := FindConfigElem(recipeCfg, recipeKey);
				if ( recipeElem )
					var recipeName := GetConfigString(recipeElem, "name");
					if ( recipeName && recipeName != "" )
						return "Receita: " + recipeName;
					endif
				endif
			endif
		endif

		// Pergaminhos de magia: CProp "spellinfo" tem id que resolve no SpellRegistry
		var spellinfo := cprops["spellinfo"];
		if ( spellinfo && TypeOf(spellinfo) == "Struct" )
			var spellId := spellinfo.id;
			if ( spellId )
				var spellElem := ResolveSpell(spellId);
				if ( spellElem && spellElem != error )
					var spellName := GetConfigString(spellElem, "Name");
					if ( spellName && spellName != "" )
						return "Pergaminho: " + spellName;
					endif
				endif
			endif
		endif
	endif

	return item.desc;
endfunction

// Resume o conteúdo de um container vendido para anexar ao tooltip do card.
// Retorna "" se o template não tem __contents.
function FormatContainerContentsPreview(template)
	if ( !template || template == error ) return ""; endif
	var contents := template.__contents;
	if ( !contents || contents == error || contents.size() == 0 ) return ""; endif

	var s := "<br><BASEFONT COLOR=#c4d8e9>Dentro:</BASEFONT>";
	var mostrados := 0;
	foreach child in contents
		var nome := ResolveSellerCardLabel(child);
		if ( nome == "" || nome == error )
			nome := "(item)";
		endif
		s := s + "<br>- " + nome;
		if ( child.amount && CInt(child.amount) > 1 )
			s := s + " x" + CStr(child.amount);
		endif
		mostrados += 1;
		if ( mostrados >= 30 )
			s := s + "<br>- ... (e mais {} itens)".format(contents.size() - 30);
			break;
		endif
		sleepms(2);
	endforeach
	return s;
endfunction

//
// This function cleans everything up when a merchant dies.
//
function RemoveMerchant(serial)
	serial := CInt(serial);
	
	var fake_info := struct;
	fake_info.+name := "a dead merchant ("+Hex(serial)+")";
	fake_info.+serial := serial;
	
	var data_file := GetMerchantDataFile();
	data_file.DeleteElement(Hex(serial)+"-ItemsSelling");
	
	data_file := GetPaymentsDataFile();
	foreach elem in DFGetElemNames(data_file)
		RemoveMerchantPayments(fake_info, elem);
		sleepms(2);
	endforeach	
endfunction
