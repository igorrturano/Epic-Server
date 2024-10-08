use uo;
use math;

//include "include/bank";
include ":datafile:include/datafile";
include ":unicos:item_template";
include ":uose:/include/item_template";
include ":timeutils:time";

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
//	var new_num := GetMerchantDataFile().keys().size();
//	new_num := GetMerchantDataFile().keys().size()+1;
	var new_num := FindAvailableNumber(data_elem);
	if ( new_num == error )
		var temp := error;
		temp.+errortext := "Error in AddItemForSale() - Unable to allocate a slot. 255 item limit reached.";
		return temp;
	endif
	
	var template := CreateItemTemplate(item);
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
	const MIN_VALUE := 0xFF;
	const MAX_VALUE := 0x14A;
	//Range = 100
	
	var new_num := CInt(data_elem.GetProp("LastNum"));
	if ( new_num < MIN_VALUE )	
		return MIN_VALUE;
	elseif ( new_num > MAX_VALUE )
		var i := MIN_VALUE;
		for ( i; i<=MAX_VALUE; i:=i+1 )
			if ( !data_elem.GetProp(CStr(i)) )
				return i;
			endif
			sleepms(2);
		endfor
		// For some reason... no new numbers are available.
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

//
// Removes items that have been on the merchant for too long.
//
function RemoveOldItemsForSale(merchant)
	var selling_list := GetItemsSoldList(merchant);
	var max_age := (SECONDS_IN_DAY*14);
	
	foreach key in selling_list
		var info := GetItemSoldInfo(merchant, key);
		var cost := info[1];
		
		if ( cost.time < max_age )
			RemoveItemForSale(merchant, key);
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
