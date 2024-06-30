//include "include/bank";
include ":datafile:include/datafile";
include ":merchants:item_template";
include ":spells:swordmage";

//Constantes de ID de Botões de Gump
const BOTAO_ITENS_COLOCAR_A_VENDA := 0x1;
const BOTAO_ITENS_MEUS_A_VENDA := 0x2;
const BOTAO_ITENS_MINHA_CONTA := 0x3;
const BOTAO_ITENS_RETIRAR := 0x4;
const BOTAO_ITENS_TODOS_A_VENDA := 0x5;
const BOTAO_ITENS_CONFISCAR := 0x6;
const BOTAO_HISTORICO := 0x7;
const BOTAO_CONFISCADOS := 0x8;
const BOTAO_CONFIGURACAO := 0x9;
const BOTAO_MOVERNPC := 0x10;



//const SHOW_ITEMS_BOUGHT_BTN := 0x7;

const MERCHANT_SELLER := 0xA;
const MERCHANT_GM := 0xB;

//Constantes de ID para conteudo a ser exibido no Gump
const MENU_INTRO := 0x0;
const MENU_ITENS_A_VENDA := 0x1;
const MENU_ITENS_COMPRAR_CONFISCAR := 0x2;
const MENU_ITENS_MEUS_A_VENDA := 0x3;
const MENU_ITENS_RETIRAR_DA_VENDA := 0x4;
const MENU_ITENS_MEUS_GANHOS := 0x5;

const END_Y			:= 405;	  // nextpage

//Gump Constants
const BKGND_STONESLAB		:= 9200;  // Gray stone slab background
const BKGND_BLUESLAB		:= 3600;  // Raised edge (blocky)
const GF_STDCOLOR		:= 1890;

const MOEDAS_COBRE		:= 0xba63; //id das moedas de cobre
const MOEDAS_PRATA		:= 0xba64; //id das moedas de prata
const MOEDAS_OURO		:= 0xba65; //id das moedas de ouro
const GOLD_COIN			:= 0xeed;  //id das moedas de ouro (outro tipo válido)



function GetPaymentsDataFile()
	var data_file := DFOpenDataFile(":merchants:payments", DF_CREATE);
	return data_file;
endfunction

function GetPaymentsDataElem(elem_name)
	var data_file := GetPaymentsDataFile();
	var data_elem := DFFindElement(data_file, elem_name, DF_CREATE);

	return data_elem;
endfunction

function AddPaymentForSeller(merchant, seller_serial, item_template)
	var data_elem := GetPaymentsDataElem(Hex(merchant.serial));
	var my_payments := data_elem.GetProp("my_payments");
	if ( !my_payments )
		my_payments := array{};
	endif

	var seller_account := data_elem.GetProp(Hex(seller_serial));
	if ( !seller_account )
		seller_account := array{};
	endif

	var total_amount := item_template.amount;

	item_template.amount := Cint(total_amount);
	item_template.desc := (item_template.amount + " " + item_template.desc);
	seller_account.Append(item_template);

	item_template.amount := Cint(total_amount*0.1);
	item_template.desc := (item_template.amount + " " + item_template.desc);
	if(item_template.amount > 0)
		my_payments.Append(item_template);
	endif

	data_elem.SetProp("my_payments", my_payments);
	data_elem.SetProp(Hex(seller_serial), seller_account);
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
function GetItemsForSale(merchant)
	var data_elem := GetMerchantDataElem(Hex(merchant.serial)+"-ItemsSelling");
	var prop_list :=  DFGetElemProps(data_elem);
	//Reversed so that LastNum is more likely to be found first
	//and makes newer items show up at the top, and older items further down.
	prop_list.Reverse();

 //Não entendi porque ele procura o LastNum pra apagar
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
function GetItemsForSaleInfo(merchant, item_num)
	var data_elem := GetMerchantDataElem(Hex(merchant.serial)+"-ItemsSelling");

	return data_elem.GetProp(CStr(item_num));
endfunction

//
// Adds an item to the merchant's ItemsSelling list.
// Moves item to the merchant's GetSellingItemsBox()
//
function AddItemForSale(merchant, seller, item, cost_type, cost_name, cost_amt, desc, infiniteAmount := 0, specialProp := 0, commission:=0)
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
	cost_info.+cost_name := CStr(cost_name);
	cost_info.+cost_amt := CInt(cost_amt);
	cost_info.+commission := commission;
	cost_info.+desc := desc;
	cost_info.+time := polcore().systime;
	if (specialProp)
		cost_info.+specialProp := specialProp;
	endif

	//sendsysmessage(seller, "cost_info" + cost_info);

	var sale_info := array{};
	sale_info.Append(cost_info);
	sale_info.Append(template);
	sale_info.Append(infiniteAmount);

	data_elem.SetProp(new_num, sale_info);
  //data_elem.SetProp("infinite", infiniteAmount);

	DestroyItem(item);

	return new_num;
endfunction

//
// Returns the next available item number in the elem.
//
function FindAvailableNumber(data_elem)
	const MIN_VALUE := 0xFF;
	const MAX_VALUE := 0x1fe;
	//Range = 255

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
function RemoveItemForSale(merchant, item_num, removeInfinite :=0)
	var data_elem := GetMerchantDataElem(Hex(merchant.serial)+"-ItemsSelling");
	var selling_info := data_elem.GetProp(Cstr(item_num));
 	// printtextabove(merchant, " " + selling_info);
	if (selling_info[3] == 1 && !removeInfinite)
		return 1;
	else
	    return data_elem.EraseProp(CStr(item_num));
	endif
endfunction

//
// Removes items that have been on the merchant for too long.
//
function RemoveOldItemsForSale(merchant)
	var selling_list := GetItemsForSale(merchant);
	var max_age := (SECONDS_IN_DAY*14);

	foreach key in selling_list
		var info := GetItemsForSaleInfo(merchant, key);
		var cost := info[1];

		if ( cost.time < max_age )
			RemoveItemForSale(merchant, key);
		endif
		sleepms(2);
	endforeach
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

function GetMerchantLog(merchant)
	var data_elem := GetPaymentsDataElem(Hex(merchant.serial));
	var my_sellings := data_elem.GetProp("sell_log");
   return my_sellings;
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

function GetPlayerNameBySerial(serial)
   	var player := SystemFindObjectBySerial(serial, SYSFIND_SEARCH_OFFLINE_MOBILES);
   	if (!player) return 0; endif
   	var player_name := GetObjProperty(player, "nome_inicial"); //player.name;
   	if (GetObjProperty(player, "realname"))
    	player_name := GetObjProperty(player, "realname");
   	endif
	return player_name;
endfunction

function GetMerchantConfiscadosLog(merchant)
	var data_elem := GetPaymentsDataElem(Hex(merchant.serial));
	var my_sellings := data_elem.GetProp("ItensRevogados");
   return my_sellings;
endfunction

function SetMerchantConfiscadosLog(merchant, confiscador, item_name, cost)
	var data_elem := GetPaymentsDataElem(Hex(merchant.serial));
	var my_sellings := data_elem.GetProp("ItensRevogados");
   if (!my_sellings) my_sellings := array{}; endif
   my_sellings.Insert(1, "{} Confiscou {} no valor de {} moedas".format(confiscador, item_name, cost));

   if (my_sellings.size() > 20)
      my_sellings.Erase(my_sellings.size());
   endif

   data_elem.SetProp("ItensRevogados", my_sellings);
endfunction

function GetItemSoldInfo(merchant, item_num)
	var data_elem := GetMerchantDataElem(Hex(merchant.serial)+"-ItemsSelling");

	return data_elem.GetProp(CStr(item_num));
endfunction

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


function DoPurchase(npc, player, input)
	var info := GetItemsForSaleInfo(npc, input);
	var cost := info[1];
	var selling := info[2];
	var confiscar := 0;

//	foreach i in selling
//		SendSysMessage(player, _i_iter+" -> "+i);
//	endforeach

	if ( player.cmdlevel >= 3 || ( (GetObjProperty(npc, "faccao") != error) && (GetObjProperty(player, "faccao") == GetObjProperty(npc, "faccao"))))
		if ( YesNo(player, "Deseja CONFISCAR este objeto?"))
			confiscar := 1;
		endif
	endif

	var cost_type := cost.cost_type;
	var cost_desc := GetName(cost.cost_type);
	var cost_name := cost.cost_name;
	var cost_amt := cost.cost_amt;

	if ( !confiscar )
		info := array{}; // Cleanup

		if (cost_type == MOEDAS_PRATA)
			cost_amt := cost_amt*10;
		elseif (cost_type == MOEDAS_OURO || cost_type == GOLD_COIN )
			cost_amt := cost_amt*100;
		endif

		if (TemHabilidade(player,"Negociador Eximio"))
			cost_amt := CInt(cost_amt*0.7);
			SendSysMessage(player, "O custo final vai ser de " + cost_amt);
		endif

		var totalPago := 0;
		var faltaPagar := cost_amt;
		//print("Falta Pagar: "+ faltaPagar +" ");

		Set_Critical(1);
		var recursosDisponiveis := CalculaRecursosDisponiveis(player, cost_type);
		if (cost_type == MOEDAS_COBRE || cost_type == MOEDAS_PRATA || cost_type == MOEDAS_OURO || cost_type == GOLD_COIN)
			SendSysMessage(player, "Recurso inicial: " + recursosDisponiveis[5] + " moedas de cobre.");
		else
			SendSysMessage(player, "Recurso inicial: " + recursosDisponiveis[5] + " " + GetName(cost_type) + ".");
		endif


		var dicionarioAtual := Dictionary;
		var recursos := recursosDisponiveis[5];
		//print("recursos: "+ recursos +" ");
		var diferenca;
		var moeda;
		var item;

		if (recursos >= faltaPagar)
			if (cost_type == MOEDAS_COBRE || cost_type == MOEDAS_PRATA || cost_type == MOEDAS_OURO || cost_type == GOLD_COIN)
				if (faltaPagar > 0) //Tenta pagar com os cobres
					dicionarioAtual := recursosDisponiveis[1];
					foreach serial in dicionarioAtual //Verifica se da pra pagar só com moedas de cobre
						moeda := SystemFindObjectBySerial(_serial_iter);
						diferenca := faltaPagar - moeda.amount;
						if (diferenca > 0)
							totalPago += moeda.amount;
							faltaPagar -= moeda.amount;
							DestroyItem(moeda);
						else
							totalPago += faltaPagar;
							SubtractAmount(moeda, faltaPagar);
							faltaPagar := 0;
							AI_Speak(npc, "Aqui esta sua compra. Volte sempre.");
							break;
						endif
					endforeach
				endif
				if (faltaPagar > 0) //Tenta pagar com as pratas
					dicionarioAtual := recursosDisponiveis[2];
					foreach serial in dicionarioAtual //Verifica se da pra pagar só com moedas de cobre
						moeda := SystemFindObjectBySerial(_serial_iter);
						diferenca := faltaPagar - moeda.amount*10;
						if (diferenca > 0)
							totalPago += moeda.amount*10;
							faltaPagar -= moeda.amount*10;
							DestroyItem(moeda);
						else
							var troco := moeda.amount*10 - faltaPagar;
							DestroyItem(moeda);
							totalPago += faltaPagar;
							DarTroco(player, npc, troco);

							faltaPagar := 0;
							AI_Speak(npc, "Aqui esta sua compra. Volte sempre.");
							break;
						endif
					endforeach
				endif
				if (faltaPagar > 0) //Tenta pagar com os ouros
					dicionarioAtual := recursosDisponiveis[3];
					foreach serial in dicionarioAtual //Verifica se da pra pagar só com moedas de cobre
						moeda := SystemFindObjectBySerial(_serial_iter);
						//print("Serial: "+ _serial_iter +" Quantidade: "+ serial);
						//print("Moeda: "+ moeda.amount +" ");
						//print("Falta Pagar: "+ faltaPagar +" ");
						diferenca := faltaPagar - moeda.amount*100;
						if (diferenca > 0)
							totalPago += moeda.amount*100;
							faltaPagar -= moeda.amount*100;
							DestroyItem(moeda);
						else
							//print("Vai pagar com ouro");
							var troco := moeda.amount*100 - faltaPagar;
							DestroyItem(moeda);
							totalPago += faltaPagar;
							DarTroco(player, npc, troco);
							faltaPagar := 0;
							AI_Speak(npc, "Aqui esta sua compra. Volte sempre.");
							break;
						endif
					endforeach
				endif
			else
				dicionarioAtual := recursosDisponiveis[4];
				foreach serial in dicionarioAtual //Verifica se da pra pagar só com moedas de cobre
					item := SystemFindObjectBySerial(_serial_iter);
					diferenca := faltaPagar - item.amount;
					if (diferenca > 0)
						totalPago += item.amount;
						faltaPagar -= item.amount;
						DestroyItem(item);
					else
						totalPago += faltaPagar;
						SubtractAmount(item, faltaPagar);
						faltaPagar := 0;
						AI_Speak(npc, "Aqui esta sua compra. Volte sempre.");
						break;
					endif
				endforeach
			endif

			var template;
			var itemPagamento;
			//print("Pago em: "+cost_type);
			if (cost_type == MOEDAS_COBRE || cost_type == MOEDAS_PRATA || cost_type == MOEDAS_OURO || cost_type == GOLD_COIN)
				itemPagamento := CreateItemAtLocation(npc.x, npc.y, -100, MOEDAS_COBRE, 1);
				template := CreateItemTemplate(itemPagamento);
				template.amount := totalPago;
				template.desc := "Moedas de cobre ["+selling.desc+"]";
			else
				itemPagamento := CreateItemAtLocation(npc.x, npc.y, -100, cost_type, 1);
				template := CreateItemTemplate(itemPagamento);
				template.amount := totalPago;
				template.desc := GetName(cost_type, 1);
			endif
			DestroyItem(itemPagamento);
			AddPaymentForSeller(npc, cost.seller, template);
			SetMerchantLog(npc, GetRealName(player), selling.desc, GetPlayerNameBySerial(cost.seller), cost_amt);

			//Exibe quanto restou de recursos
			recursosDisponiveis := CalculaRecursosDisponiveis(player, cost_type);
			if (cost_type == MOEDAS_COBRE || cost_type == MOEDAS_PRATA || cost_type == MOEDAS_OURO || cost_type == GOLD_COIN)
				SendSysMessage(player, "Recurso final: " + recursosDisponiveis[5] + " moedas de cobre.");
			else
				SendSysMessage(player, "Recurso inicial: " + recursosDisponiveis[5] + " " + GetName(cost_type) + ".");
			endif
		else
			SendSysMessage(player, "Voce nao tem "+ GetName(cost_type, 1)+ " suficiente para comprar isto.");
			Set_Critical(0);
			return 0;
		endif
		Set_Critical(0);
	else
		/*var ItensRevogados := dictionary;

		var confiscado_info := ", {" + cost_name + " [" + cost_amt + " ";
		if(cost_type == MOEDAS_COBRE || cost_type == CInt(MOEDAS_COBRE))
			confiscado_info += "Moedas de Cobre";
		elseif(cost_type == MOEDAS_PRATA || cost_type == CInt(MOEDAS_PRATA))
			confiscado_info := "Modeas de Prata";
		else
			confiscado_info := "Moedas de Ouro";
		endif

		confiscado_info += "] [" + GetObjProperty(player, "faccao") + "]}";

		if ( GetObjProperty(npc, "ItensRevogados") )
			ItensRevogados := GetObjProperty(npc, "ItensRevogados");
			ItensRevogados.Insert(cost.seller, CStr(ItensRevogados) + ", "+ confiscado_info);
		else
			ItensRevogados.Insert(cost.seller, confiscado_info);
		endif
		SetObjProperty(npc, "ItensRevogados", ItensRevogados);*/

		SetMerchantConfiscadosLog(npc, GetRealName(player), selling.desc, cost_amt);
		AI_Speak(npc, "Aqui esta o objeto confiscado, avisarei o dono assim que ele aparecer por aqui.");
	endif

	Set_Critical(1);
	CreateItemInBackpackFromTemplate(player, selling);
	RemoveItemForSale(npc, input);
	Set_Critical(0);
endfunction

function ShowForSaleItemsInfo(byref gump, npc, player, input, special := 0)
	var sell_details:= GetItemsForSaleInfo(npc, input);

	GFResizePic(gump, 170, 15, BKGND_STONESLAB, 455, 450);
	var cost_info := sell_details[1];
	var selling := sell_details[2];

	var color := GF_STDCOLOR;
	if ( selling.color )
		color := selling.color-1;
	endif

	GFTextLine(gump, 200, 25, GF_STDCOLOR, "Nome");
	GFTextLine(gump, 400, 25, GF_STDCOLOR, "Quantidade");
	GFTextLine(gump, 175, 50, GF_STDCOLOR, "Produto");
	GFTextLine(gump, 420, 50, GF_STDCOLOR, selling.amount);
	if (!GetObjProperty(npc, "secret") || GetObjProperty(player, "secret"))
		GFTextLine(gump, 175, 95, GF_STDCOLOR, "Vendedor");
		var vendor := SystemFindObjectBySerial(cost_info.seller, SYSFIND_SEARCH_OFFLINE_MOBILES);
		if (vendor.cmdlevel && !player.cmdlevel)
			GFTextLine(gump, 205, 115, GF_STDCOLOR, "Desconhecido");
		else
			var nome := vendor.name;
			var npcfac := GetObjProperty(npc, "faccao");
			var playerfac := GetObjProperty(player, "faccao");
			//if (GetObjProperty(vendor, "faccao") && (GetObjProperty(npc, "faccao") == GetObjProperty(vendor, "faccao")))
			if ((playerfac != error) && (playerfac == npcfac))
				nome := GetRealName(vendor);
			else
				nome := "SEM AUTORIZACAO";
			endif
			GFTextLine(gump, 205, 115, GF_STDCOLOR, nome);
		endif
	endif
	GFTextLine(gump, 205, 70, color, selling.desc);
	GFResizePic(gump, 500, 25, 2620, 120, 100);
	GFTilePic(gump, 515, 40, selling.graphic);

	color := GF_STDCOLOR;
	var temp := item_cfg[cost_info.cost_type].color;
	if ( temp )
		color := temp-1;
	endif
	var cost_name;
	if (cost_info.objtype == MOEDAS_COBRE || cost_info.objtype == CInt(MOEDAS_COBRE) || cost_info.objtype == error)
		cost_name := "moedas de cobre";
	elseif (cost_info.objtype == MOEDAS_PRATA || cost_info.objtype == CInt(MOEDAS_PRATA))
		cost_name := "moedas de prata";
	elseif (cost_info.objtype == MOEDAS_OURO || cost_info.objtype == CInt(MOEDAS_OURO) || cost_info.objtype == GOLD_COIN || cost_info.objtype == CInt(GOLD_COIN))
		cost_name := "moedas de ouro";
	else
		if (cost_info.cost_name)
			cost_name := cost_info.cost_name;
		else
			cost_name := GetName(cost_info.cost_type);
		endif
	endif
	GFTextLine(gump, 175, 150, GF_STDCOLOR, "Preco");
	GFTextLine(gump, 420, 150, GF_STDCOLOR, cost_info.cost_amt);
	GFTextLine(gump, 205, 170, color, cost_name);
	GFResizePic(gump, 500, 125, 2620, 120, 100);
	GFTilePic(gump, 515, 140, GetGraphic(cost_info.cost_type));

	GFTextLine(gump, 175, 210, GF_STDCOLOR, "Descricao");
	GFResizePic(gump, 175, 230, 2620, 445, 200);
	var desc_lines := GFWordWrap(cost_info.desc, 455);
	var y_pos := 260;

	var tooltip := "Propaganda: ";
	foreach line in desc_lines
		tooltip := tooltip + line + "<br>";
	endforeach
	tooltip := tooltip + "<br>" + GetTooltipText(selling, player);
	GFHTMLArea(gump, 185, 235, 430, 185, tooltip, 0, 10);

	GFAddButton(gump, 200, 430, 2151, 2154, 1, input);
	if(!special)
		if ( (player.cmdlevel < 3) && ( (GetObjProperty(player, "faccao") == error) || (GetObjProperty(npc, "faccao") != GetObjProperty(player, "faccao"))) )
			GFTextLine(gump, 235, 433, GF_STDCOLOR, "Adquirir");
		elseif ( (player.cmdlevel < 3) && (GetObjProperty(npc, "faccao") == GetObjProperty(player, "faccao")))
			GFTextLine(gump, 235, 433, GF_STDCOLOR, "Adquirir ou Confiscar");
		else
			GFTextLine(gump, 235, 433, GF_STDCOLOR, "Adquirir ou Confiscar (GM)");
		endif
	else
		GFTextLine(gump, 235, 433, GF_STDCOLOR, "Remover");
	endif
endfunction

function GetRealName(byref who)
	var nome;
	if (GetObjProperty(who, "nome_inicial"))
		nome := GetObjProperty(who, "nome_inicial");
	endif
	if (GetObjProperty(who, "realname"))
		nome := GetObjProperty(who, "realname");
	endif
	return nome;
endfunction

function AddMyItemForSale(npc, player)

	var VendorProperty := GetObjProperty(npc, "onlygmsells");
	if (VendorProperty)
		if ((VendorProperty == 1 ) && (player.cmdlevel < 3 ))  //Quando o NPC está marcado para só permitir que GM coloque coisas à venda, encerra a função para qualquer um com cmdlevel menor que 3 (GM)
			SendSysMessage(player, "Voce nao pode colocar itens a venda nesse vendedor.");
			return;
		endif
	endif

	VendorProperty := GetObjProperty(npc, "oneplayersells");
	if (VendorProperty)
		if (VendorProperty != player.name) //Quando o NPC está marcado para só permitir que um player específico coloque coisas à venda, encerra a função para qualquer um que não seja esse player
			SendSysMessage(player, "Voce nao pode vender neste mercador");
			return;
		endif
	endif


	VendorProperty := GetObjProperty(npc, "onlyartifice");
	if (VendorProperty)
		if (getObjProperty(player, "kit") != "Artifice") //Quando o NPC está marcado para só permitir que um player com Kit Artifice coloque coisas à venda, encerra a função para qualquer um que não tenha o Kit Artifice
			SendSysMessage(player, "Apenas artifices podem por itens a venda nesse mercador.");
			return;
		endif
	endif

	SendSysMessage(player, "Que item deseja colocar para vender?");

	var targ := target(player); //Seleciona o item.

	var serialid := GetObjProperty(targ,"serialid"); //Armazena o serial do item selecionado

	if (serialid != error) //itens sem serial são inválidos e por isso a função é encerrada
		SendSysMessageEx(player,"Voce nao pode vender isto.",SSM_FAIL);
		return 0;
	endif

	if ( !targ.IsA(POLCLASS_ITEM) ) //Se for selecionado algo que não é um item, a função é encerrada
		SendSysMessage(player, "Cancelado.");
		return;
	elseif (!(targ in EnumerateItemsInContainer(player.backpack))) //Quando o item não está na mochila principal do personagem, não dá pra colocar ele à venda e por isso a função é encerrada
		SendSysMessage(player, "Apenas itens na mochila principal podem ser vendidos.");
		return;
	endif
	AddItem(npc, player, targ);

endfunction

function AddItem(npc, player, targ)

	var cost_amt;
	var cost_type;
	var cost_name;
	var item;
	var valorTaxaAnterior;
	var specialProp := struct;

	if (player.cmdlevel > 2) //Se é GM colocando à venda

		AI_Speak(npc, "O que voce gostaria que eu cobrasse por "+targ.desc+"?"); //GM pode colocar o NPC pra aceitar pagamentos que não sejam apenas moedas
		SendSysMessage(player, "Selecione um objeto para mostrar ao mercador o que voce deseja cobrar por este item.");

		item := target(player);
		if ( !item.IsA(POLCLASS_ITEM) )
			SendSysMessage(player, "Cancelado.");
			return;
		endif

		var cost_limit, limit_reason;
		if ( item.amount > 1 ) //Se o que que foi apontado como pagamento pelo item é stackable
			cost_limit := 100000;
			limit_reason := " (item empilhavel)";
		else
			cost_limit := 1;
			limit_reason := " (item nao empilhavel)";
		endif

		cost_name := item.desc; //Salva o nome do item escolhido
		if (item.amount > 1) //Quando o item escolhido é empilhável
			var RemoveNumber := splitwords(cost_name," ",1);  //Ajusta o nome removendo o numero que representa a quantidade do item escolhido
			cost_name := RemoveNumber[2]; //Atualiza o nome sem á quantidade como prefixo
		endif

		cost_type := item.objtype; //Salva o tipo de objeto a ser utilizado como pagamento

		AI_Speak(npc, "Que quantidade de '"+cost_name+"' devo cobrar?");
		cost_amt := RequestGump(player, "Que quantidade de '"+cost_name+"' devo cobrar?", "1 - "+cost_limit+limit_reason, "", 1);
		cost_amt := CInt(cost_amt);
		if ( !cost_amt || cost_amt > cost_limit || cost_amt <= 0 ) //Cancela a operação se forem inseridos valores inválidos
			SendSysMessage(player, "Cancelado");
			return;
		endif

		var specialItem := YesNo(player, "Esse item vai ter uma prop unica? Caso sim, use props do tipo String");
		if (specialItem)
			var specialName := RequestGump(player, "Escreva o nome da prop unica");
			var specialValue := RequestGump(player, "Escreva o valor da prop");
			if (specialName && specialValue)
				specialProp.+name := specialName;
				specialProp.+value := specialValue;
			endif
		endif

	else //Se não é GM colocando à venda

		var cost_limit := 100000;
		var limit_reason := "(item empilhavel)";
		cost_type := MOEDAS_COBRE;

		cost_amt := CInt(RequestGump(player, "Quantas moedas de cobre deseja cobrar por "+targ.desc+"?", "1 - "+cost_limit+limit_reason, "", 1));

		if ( !cost_amt || cost_amt <= 0 ) //Valor inválido ou não positivo encerra a função
			SendSysMessage(player, "Cancelado");
			return;
		elseif (cost_amt > cost_limit) //Valor acima do limite encerra a função
			SendSysMessage(player, "Voce ultrapassou o limite maximo de "+cost_limit+" moedas.");
			return;
		endif
	endif

	var desc := RequestGump(player, "Entre com uma pequena descricao do produto.", "Isso e opcional", "", 1);

	if ( !desc )
		desc := "";
	elseif ( Len(desc) > 300 )
		desc := desc[0,300]+"...";
	endif

	var infinite := 0;

	if (player.cmdlevel > 2) //Permite deixar itens permanentes a venda na loja se for GM<
		infinite := YesNo(player, "Voce deseja deixar esse item infinito?");
	endif

	Set_Critical(1);

	//Verifica se vai cobrar a taxa antes da venda/retirada ou só
	if (GetObjProperty(npc, "oneplayersells") == player.name)
		SendSysMessage(player, "Seu vendedor particular nao cobra taxas.");
	elseif (TemHabilidade(player, "Mercador Reconhecido"))
		SendSysMessage(player, "Vendedores descontam taxas de Mercadores Reconhecidos apenas depois de vender seus itens.");
	elseif (!GetObjProperty(npc, "taverneiro"))
		//Cobra taxas previstas naquele vendedor para quem tem que pagar pra colocar a venda
		var taxa := 0.0;
		if (cost_type == MOEDAS_COBRE || cost_type == MOEDAS_PRATA || cost_type == MOEDAS_OURO || cost_type == GOLD_COIN)
			if(GetObjProperty(npc, "taxap")) // 0.0 equivale a 0% e 1.0 equivale a 100%
				var pesoTaxa := CInt(getObjProperty(npc, "taxap"));
				if (cost_type == MOEDAS_COBRE)
					taxa := CInt(1.0 * cost_amt * (pesoTaxa * 0.01));
				elseif (cost_type == MOEDAS_PRATA)
					taxa := CInt(10.0 * cost_amt * (pesoTaxa * 0.01));
				elseif (cost_type == MOEDAS_OURO || cost_type == GOLD_COIN)
					taxa := CInt(100.0 * cost_amt * (pesoTaxa * 0.01));
				endif
			else
				if (cost_type == MOEDAS_COBRE)
					taxa := CInt((1.0 * cost_amt) * 0.1);
				elseif (cost_type == MOEDAS_PRATA)
					taxa := CInt((10.0 * cost_amt) * 0.1);
				elseif (cost_type == MOEDAS_OURO || cost_type == GOLD_COIN)
					taxa := CInt((100.0 * cost_amt) * 0.1);
				endif
			endif

			var recursosDisponiveis := CalculaRecursosDisponiveis(player, cost_type);
			if(taxa > recursosDisponiveis[5])
				SendSysMessage(player, "Voce precisa pagar " + taxa + " moedas de taxa, mas tem apenas " + recursosDisponiveis[5] + ".");
				SendSysMessage(player, "Cancelado.");
				return 0;
			elseif(taxa > 0)
				var totalPago := 0;
				var faltaPagar := taxa;
				valorTaxaAnterior := taxa;
				var dicionarioAtual := Dictionary;
				var recursos := recursosDisponiveis[5];
				//print("recursos: "+ recursos +" ");
				var diferenca;
				var moeda;
				var item;
				if (faltaPagar > 0) //Tenta pagar com os cobres
					dicionarioAtual := recursosDisponiveis[1];
					foreach serial in dicionarioAtual //Verifica se da pra pagar só com moedas de cobre
						moeda := SystemFindObjectBySerial(_serial_iter);
						diferenca := faltaPagar - moeda.amount;
						if (diferenca > 0)
							totalPago += moeda.amount;
							faltaPagar -= moeda.amount;
							DestroyItem(moeda);
						else
							totalPago += faltaPagar;
							SubtractAmount(moeda, faltaPagar);
							faltaPagar := 0;
							break;
						endif
					endforeach
				endif
				if (faltaPagar > 0) //Tenta pagar com as pratas
					dicionarioAtual := recursosDisponiveis[2];
					foreach serial in dicionarioAtual //Verifica se da pra pagar só com moedas de prata
						moeda := SystemFindObjectBySerial(_serial_iter);
						diferenca := faltaPagar - moeda.amount*10;
						if (diferenca > 0)
							totalPago += moeda.amount*10;
							faltaPagar -= moeda.amount*10;
							DestroyItem(moeda);
						else
							var troco := moeda.amount*10 - faltaPagar;
							DestroyItem(moeda);
							totalPago += faltaPagar;
							DarTroco(player, npc, troco);
							faltaPagar := 0;
							break;
						endif
					endforeach
				endif
				if (faltaPagar > 0) //Tenta pagar com os ouros
					dicionarioAtual := recursosDisponiveis[3];
					foreach serial in dicionarioAtual //Verifica se da pra pagar só com moedas de ouro
						moeda := SystemFindObjectBySerial(_serial_iter);
						//print("Serial: "+ _serial_iter +" Quantidade: "+ serial);
						//print("Moeda: "+ moeda.amount +" ");
						//print("Falta Pagar: "+ faltaPagar +" ");
						diferenca := faltaPagar - moeda.amount*100;
						if (diferenca > 0)
							totalPago += moeda.amount*100;
							faltaPagar -= moeda.amount*100;
							DestroyItem(moeda);
						else
							//print("Vai pagar com ouro");
							var troco := moeda.amount*100 - faltaPagar;
							DestroyItem(moeda);
							totalPago += faltaPagar;
							DarTroco(player, npc, troco);
							faltaPagar := 0;
							break;
						endif
					endforeach
				endif

				if (GetObjProperty(npc, "cofreid"))
					var cofser := getObjProperty(npc, "cofreid");
					var cofrereal := SystemFindObjectBySerial(cofser);
					CreateItemInContainer( cofrereal, "moedacobre", taxa);
				endif

				SendSysMessage(player, "Voce pagou " + taxa + " moedas de taxa. Restaram " + CStr(recursosDisponiveis[5] - taxa) + " moedas.");

			endif
		endif
	endif

	var result := AddItemForSale(npc, player, targ, cost_type, cost_name, cost_amt, desc, infinite, specialProp);
	Set_Critical(0);

	if (result != error)
		AI_Speak(npc, "Tudo certo. Colocarei seu item a venda.");
		SendSysMessage(player, "O item foi adicionado com sucesso.");
	else
		DarTroco(player, npc, valorTaxaAnterior); //A ideia é o npc devolva as moedas que cobrou
		AI_Speak(npc, "Me desculpe, mas nao poderei vender isto para voce e estou devolvendo suas moedas.");
	endif
endfunction

function ShowForSaleItems(byref gump, npc, player, special := 0)

	GFResizePic(gump, 170, 15, BKGND_STONESLAB, 455, 450);
	GFTextMid(gump, 170, 25, 455, GF_STDCOLOR, "Minhas vendas");
	GFPage(gump, 1);

	var items_a_venda := GetItemsForSale(npc);
	var ItensRevogados := dictionary;

	if ( GetObjProperty(npc, "ItensRevogados") )
		ItensRevogados := GetObjProperty(npc, "ItensRevogados");
	endif

	var y_pos := 45;
	foreach serial in items_a_venda
		var info := GetItemsForSaleInfo(npc, serial);
		var item := info[2];
		var cost_info := info[1];
		var donoDoItem := SystemFindObjectBySerial(cost_info.seller, SYSFIND_SEARCH_OFFLINE_MOBILES);
		//print( "array: " + items_a_venda + " item " + info);
		//print("serial "  + cost_info.seller  + " player " + player);
		if (!donoDoItem)
			RemoveItemForSale(npc, serial);
			//print("removendo item do vendor de char deletado.");
		endif
		info := info[1];

		//print("inso.seller " + info.seller + " player.serial " + player.serial);
		var cost_name;
		if( (!special) || (info.seller == CInt(player.serial)) ) // Quando a propriedade especial esta desabilitada ou o item não é seu, exibe eles na lsita de itens à venda
			if( y_pos > END_Y )
				GFAddButton(gump, 230, 25, 2224, 2224, GF_PAGE_BTN, gump.cur_page+1);
				y_pos := 45;
				GFPage(gump, gump.cur_page+1);

			endif
			if(gump.cur_page > 1)
				GFAddButton(gump, 200, 25, 2223, 2223, GF_PAGE_BTN, gump.cur_page-1);
			endif

			var color := GF_STDCOLOR;
			if ( item.color )
				color := item.color-1;
			endif

			if (cost_info.objtype == MOEDAS_COBRE || cost_info.objtype == error)
				cost_name := "moedas de cobre";
			elseif (cost_info.objtype == MOEDAS_PRATA)
				cost_name := "moedas de prata";
			elseif (cost_info.objtype == MOEDAS_OURO || cost_info.objtype == GOLD_COIN)
				cost_name := "moedas de ouro";
			else
				if (cost_info.cost_name)
					cost_name := cost_info.cost_name;
				else
					cost_name := GetName(cost_info.cost_type);
				endif
			endif

			var itemLAbel := item.desc + " -> " + cost_info.cost_amt + " " + cost_name;
			var itemLAbelSize := Len(itemLAbel);
			var tamanhoMaximo := 58;


			if( itemLAbelSize < tamanhoMaximo)
				//Adiciona os caracteres restamtes de descrição resumida na frente do nome do item à venda
				var desc_resumida := cost_info.desc;
				if ( (!desc_resumida) || (desc_resumida == error) || (desc_resumida == ""))
					itemLAbel += "";
				elseif ( Len(desc_resumida) > tamanhoMaximo - itemLAbelSize )
					itemLAbel += " ("+ desc_resumida[1,tamanhoMaximo - itemLAbelSize]+ "...)";
				else
					itemLAbel += " ("+desc_resumida+")";
				endif
			endif

			GFAddButton(gump, 175, y_pos+3, 2117, 2118, 1, serial);
			GFTextLine(gump, 195, y_pos, color, itemLAbel);
			//GFTextLine(gump, 195, y_pos, color, item.desc + " -> " + cost_info.cost_amt + " " + cost_name );
			y_pos := y_pos+20;
			sleepms(2);
		endif
	endforeach

	if (ItensRevogados.Exists(player.serial))
		var confiscados := ItensRevogados[player.serial];
		GumpPrompt(player, "Algumas de suas mercadorias foram confiscadas. procure saber os motivos com as autoridades comerciais locais." + CStr(confiscados));
		SendSysMessage(player, confiscados);
		ItensRevogados.Erase(player.serial);
		SetObjProperty(npc, "ItensRevogados", ItensRevogados);
	endif
endfunction

function RetrieveMyItem(npc, player, input)
	Set_Critical(1);
	var info := GetItemsForSaleInfo(npc, input);
	var cost := info[1];
	var selling := info[2];
	info := array{}; // Cleanup

	if (selling.amount > 1) //Quando o item escolhido é empilhável
		var RemoveNumber := splitwords(selling.desc," ",1);  //Ajusta o nome removendo o numero que representa a quantidade do item escolhido
		selling.desc := RemoveNumber[2]; //Atualiza o nome sem á quantidade como prefixo
	endif

	AI_Speak(npc, "Certo. Aqui esta seu produto.");
	RemoveItemForSale(npc, input, 1); //Terceira propriedade indica que o item inifito deve ser removido da lista
	var purchase := CreateItemInBackPack(player, selling, selling.amount);
	if ( purchase == error )
		purchase := CreateItemAtLocation(player.x, player.y, player.z, selling, selling.amount);
		if ( purchase == error )
			SendSysMessage(player, "Error - Unable to create item! ->"+purchase.errortext);
		else
			AI_Speak(npc, "Eu nao fui capaz de colocar o item na sua bolsa. Coloquei aos seus pes entao.");
		endif
	endif
	Set_Critical(0);
endfunction

function ShowMyProfit(byref gump, npc, player)
	GFResizePic(gump, 170, 15, BKGND_STONESLAB, 455, 450);
	GFTextMid(gump, 170, 25, 455, GF_STDCOLOR, "Meus Ganhos");
	GFPage(gump, 1);

	var items_inside := GetSellerAccount(npc, player.serial);

	var y_pos := 45;
	foreach template in items_inside
		var color := GF_STDCOLOR;
		if ( template.color )
			color := template.color-1;
		endif

		GFTextLine(gump, 175, y_pos, color, template.desc);
		y_pos := y_pos+20;
		sleepms(2);
	endforeach

	if( len(items_inside) > 0 )
		y_pos += 20;
		GFAddButton(gump, 175, y_pos+3, 2117, 2118, 1, BOTAO_ITENS_RETIRAR);
		GFTextLine(gump, 195, y_pos, GF_STDCOLOR, "Retirar tudo");
	endif
endfunction

function DoWithdraw(npc, player)
	Set_Critical(1);
	var data_elem := GetPaymentsDataElem(Hex(npc.serial));
	var account := data_elem.GetProp(Hex(player.serial));
	var moedas := 0;
	foreach item in account
		//print("Item: "+item);
		//print("Item (objType: "+item.objtype);
		if(item.objtype == MOEDAS_OURO || item.objtype == GOLD_COIN)
			moedas += item.amount * 100;
		elseif(item.objtype == MOEDAS_PRATA)
			moedas += item.amount * 10;
		elseif(item.objtype == MOEDAS_COBRE)
			moedas += item.amount;
		else
			var withdraw := CreateItemInBackPack(player, item, item.amount);
			if( withdraw == error )
				withdraw := CreateItemAtLocation(player.x, player.y, player.z, item, item.amount);
				if ( withdraw == error )
					SendSysMessage(player, "Error - Unable to create item! ->"+withdraw.errortext);
				else
					AI_Speak(npc, "Eu nao fui capaz de colocar o item na sua bolsa. Coloquei aos seus pes entao.");
				endif
			endif
		endif
		sleepms(2);
	endforeach
	DarTroco(player, npc, moedas);
	data_elem.EraseProp(Hex(player.serial));
	Set_Critical(0);
endfunction


function CalculaRecursosDisponiveis(player, cost_type)
	var recursosDisponiveis := Dictionary;
	recursosDisponiveis[1] := Dictionary; //Cobre
	recursosDisponiveis[2] := Dictionary; //Prata
	recursosDisponiveis[3] := Dictionary; //Ouro
	recursosDisponiveis[4] := Dictionary; //Outros
	recursosDisponiveis[5] := 0; //Total

	if (cost_type == MOEDAS_COBRE || cost_type == MOEDAS_PRATA || cost_type == MOEDAS_OURO || cost_type == GOLD_COIN)
		foreach item in EnumerateItemsInContainer(player.backpack)
			if (item.objtype == MOEDAS_COBRE)
				recursosDisponiveis[1][item.serial] := item.amount;
				recursosDisponiveis[5] += item.amount;
			elseif (item.objtype == MOEDAS_PRATA)
				recursosDisponiveis[2][item.serial] := item.amount;
				recursosDisponiveis[5] += item.amount*10;
			elseif (item.objtype == MOEDAS_OURO || item.objtype == GOLD_COIN)
				recursosDisponiveis[3][item.serial] := item.amount;
				recursosDisponiveis[5] += item.amount*100;
			endif
			//print("Valor Disponivel : "+ recursosDisponiveis[5] +" ");
		endforeach
	else
		foreach item in EnumerateItemsInContainer(player.backpack)
			if (item.objtype == cost_type)
				recursosDisponiveis[4][item.serial] := item.amount;
				recursosDisponiveis[5] += item.amount;
				//print("Valor Disponivel : "+ recursosDisponiveis[5] +" ");
			endif
		endforeach
	endif
	return recursosDisponiveis;
endfunction


function DarTroco(player, npc , ValorTroco)
	var faltaPagar := ValorTroco;
	if(ValorTroco > 0)
		SendSysMessage(player, "Recebeu:"); //Comentar quando for colocar em produção
	endif
	Set_Critical(1);
	var ouro := CInt(faltaPagar/100);
	if(ouro > 0)
		CreateItemInBackpack(player, MOEDAS_OURO, ouro);
		SendSysMessage(player, ouro + " moedas de ouro");
	endif
	faltaPagar -= CInt(ouro*100);


	var prata := CInt(faltaPagar/10);
	if(prata > 0)
		CreateItemInBackpack(player, MOEDAS_PRATA, prata);
		SendSysMessage(player, prata + " moedas de prata");
	endif
	faltaPagar -= CInt(prata*10);


	var cobre := CInt(faltaPagar);
	if(cobre > 0)
		CreateItemInBackpack(player, MOEDAS_COBRE, cobre);
		SendSysMessage(player, cobre + " moedas de cobre");
	endif
	Set_Critical(0);

	AI_Speak(npc, "Aqui estao suas moedas.");
	return 1;
endfunction

//FUNÇÃO PRA PEGAR DADOS DE TOOLTIP E JOGAR NO GUMP
function GetTooltipText(item_template, who)
	var white := "<BASEFONT COLOR=#FFFFFF>";
	var amareloclaro := "<BASEFONT COLOR=#f5d189>";
	var laranja :=  "<BASEFONT COLOR=#FFA500>";
	var roxo := "<BASEFONT COLOR=#d98cd9>";
	var softgreen := "<BASEFONT COLOR=#a1eca7>";
	var blueice := "<BASEFONT COLOR=#79b6c9 >";
	var redfire := "<BASEFONT COLOR=#e95c4b>";
	var lilas := "<BASEFONT COLOR=#d998a0>";
	var fisico := "<BASEFONT COLOR=#FFFFFF>";


	var Object_Name;

	var xDesc; //Será utilizado para juntar as informações do tooltip do item
	var desc; //Salva o campo de descrição do item ou personagem

	//Encontra o objeto que terá o tooltip preenchido

	if(item_template)

		var resource_config := ReadConfigFile(":crafting:config/resourceList");
		if(item_template.material)
			Object_Name := item_template.desc + " [" + resource_config[item_template.material].name + "]";
		else
			Object_Name := item_template.desc;
		endif

		xDesc := "";

		desc := item_template.description;
		if(desc == error)
			 desc := "";
		endif

		//EXIBE SE TEM PROPRIEDADE MAGICA
		if (item_template.magic)
			xDesc := xDesc + roxo + "Magico<br>";
		endif

		var descmods := item_template.descmod;

		//JUNTA AS DESCRIÇÕES DO DOS PROPS DO PERSONAGEM

		if(descmods != error)
			foreach descmod in descmods
				if(descmod)
					desc := desc + descmod;
				endif
			endforeach
		else
			desc := desc + descmods;
		endif

		//LIMITA O TAMANHO DAS DESCRIÇÕES
		if (len(desc) > 85)
			desc := desc[1, 85];
		endif

		//VERIFICA PROPRIEDADES DE IDIOMAS
		if (desc!=error)
			var linguagem := item_template.lingua;
			if (linguagem!=error)
				var linguas := GetObjProperty(who, "linguas");
				if (temHabilidade(who, "Arqueologo") || temHabilidade(who, "Formacao Academica") || linguagem in linguas)
					xDesc := xDesc + white + "" + desc + "<br>";
				else
					xDesc :=  xDesc + "*Inscrições em Língua Desconhecida*<br>";
				endif
			else
				xDesc := xDesc + white + "" + desc + "<br>";
			endif
		endif

		//ADICIONA DESCRIÇÕES DAS PROPRIEDADES COMUNS DE ARMAS E ARMADURAS
		var cfg := ReadConfigFile(":*:itemdesc");
		var elem := FindConfigElem(cfg, item_template.objtype);
		if (item_template.quality >= 1.1)
			xDesc := xDesc + white + getQualityTemplate(item_template.quality) + "<br>"; //exibe apenas dois digitos
		endif

		var cond := GetConditionTemplate(item_template);
		xDesc := xDesc + "HP: " + GetConditionTemplate(item_template) + "<br>";
		xDesc := xDesc + "Min Str: " + elem.strrequired + "<br>";

		if(elem.TipoAtaque)
			xDesc := xDesc + "Dano: " + GetDmgStringDesc(elem,  item_template) + "<br>";
			xDesc := xDesc + "Velocidade: " + GetVelStringDescTemplate(elem, item_template) + "<br>";
			xDesc := xDesc + GetTipoStringDescTemplate(elem, item_template) + "<br>";
			var crits := GetCriticalInfoTemplate(elem);
			if (crits != "")
				xDesc := xDesc + laranja + crits;
			endif

			var hits :=  GetHitsInfo(elem);
			if (hits != "")
				xDesc := xDesc + amareloclaro + hits;
			endif

			xDesc := xDesc + white + "Fisico " + (elem.PhysicalDamage + item_template.damage_physical_mod) + "<br>";

			if ( (elem.ColdDamage + item_template.damage_cold_mod != 0))
			xDesc := xDesc  + blueice + "Frio " + (elem.ColdDamage + item_template.damage_cold_mod) + "<br>";
			endif
			if ( (elem.FireDamage + item_template.damage_fire_mod != 0))
				xDesc := xDesc + redfire + "Fogo " + (elem.FireDamage + item_template.damage_fire_mod) + "<br>";
			endif
			if ( (elem.EnergyDamage + item_template.damage_energy_mod != 0))
				xDesc := xDesc  + lilas + "Energia " + (elem.EnergyDamage + item_template.damage_energy_mod) + "<br>";
			endif
			if ( (elem.PoisonDamage + item_template.damage_poison_mod != 0))
				xDesc := xDesc  + softgreen + "Veneno " + (elem.PoisonDamage + item_template.damage_poison_mod) + "<br>";
			endif

			//EXIBE AS GEMAS QUE TEM NA ARMA
			if ( getObjProperty(item_template, "swordmage") == who.serial)
				var gems := getobjproperty(item_template, "gems");
				foreach gem in (gems.keys())
					xDesc := xDesc + white + gem + " - "+ getSwordSpellName(gems[gem].id) + "<br>";
				endforeach
			endif
			if (item_template.dmg_mod > 0 )
				xDesc := xDesc + roxo + "Bônus Dano";
			endif
			if (item_template.hp_mod > 0)
				xDesc := xDesc + roxo + "Bônus item_template PVs";
			endif

			var hitsscripts := item_template.HitScripts;
			if (hitsscripts != error)
				foreach hits in hitsscripts
					if (hits == ":combat:weaponHitScripts/FireHit")
						xDesc := xDesc + roxo + "Dano de Fogo<br>";
					elseif ( hits == ":combat:weaponHitScripts/IceHit")
						xDesc := xDesc + roxo + "Dano de Gelo<br>";
					elseif ( hits == ":combat:weaponHitScripts/LightningHit")
						xDesc := xDesc + roxo + "Dano de Energia<br>";
					endif
				endforeach
			endif

			if (item_template.swordmage != error)
				xDesc := xDesc + white + item_template.swordmage + "<br>";
			endif
		elseif (elem.TipoProtecao)
			//props de armaduras
			//				if (cint(elem.strrequired) > 0)
			//					xDesc := xDesc + "Min Str: " + elem.strrequired + "<br>";
			//				endif
			if (elem.DexPenalty || item_template.penaltybonus)
				xDesc := xDesc + "Redutor Dex : " + getDexPenaltyTemplate(elem, item_template) + "<br>";
			endif
			if (elem.parrybonus || item_template.parrybonus)
				xDesc := xDesc + "Block Bonus: " + getParryBonusTemplate(elem, item_template) + "<br>";
			endif
			if (elem.parrydelay || item_template.parrydelay_mod)
				xDesc := xDesc + "Delay Bonus: " + getParryDelayTemplate(elem, item_template) + "<br>";
			endif
			xDesc := xDesc + fisico + "Fisico " + (elem.PhysicalResist + item_template.resist_physical_mod) + "<br>";

			if (elem.ColdResist + item_template.resist_cold_mod != 0)
				xDesc := xDesc  + blueice + "Frio " + (elem.ColdResist + item_template.resist_cold_mod) + "<br>";
			endif
			if ( elem.FireResist + item_template.resist_fire_mod != 0)
				xDesc := xDesc + redfire + "Fogo " + (elem.FireResist + item_template.resist_fire_mod) + "<br>";
			endif
			if ( elem.EnergyResist + item_template.resist_energy_mod != 0)
				xDesc := xDesc  + lilas + "Energia " + (elem.EnergyResist + item_template.resist_energy_mod) + "<br>";
			endif
			if (elem.PoisonResist + item_template.resist_poison_mod != 0)
				xDesc := xDesc  + softgreen + "Veneno " + (elem.PoisonResist + item_template.resist_poison_mod) + "<br>";
			endif
		endif
		var pplmage := item_template.magicwho; //magic lore
		if ( pplmage != error)
			if (who.serial in pplmage)
				if (item_template.pocaoname)
					xDesc := xDesc + white + item_template.pocaoname;
				endif
				var mods := item_template.equip;
				if (mods != error)
					foreach mod in (mods.keys())
						if (mod == "strmod")
							xDesc := xDesc + white + "Bônus For <br>";
						elseif (mod == "dexmod")
							xDesc := xDesc + white + "Bônus Des <br>";
						elseif (mod == "intmod")
							xDesc := xDesc + white + "Bônus Int <br>";
						elseif (mod == "hitsmod")
							xDesc := xDesc + white + "Bônus PVs<br>";
						elseif (mod == "manamod")
							xDesc := xDesc + white + "Bônus Mana <br>";
						elseif (mod == "stammod")
							xDesc := xDesc + white + "Bônus Stam <br>";
						elseif (mod == "PhysicalResistMod")
							xDesc := xDesc + white + "Resistencia Fisica <br>";
						elseif (mod == "ColdResistMod")
							xDesc := xDesc + blueice  + "Resistencia Frio <br>";
						elseif (mod == "FireResistMod")
							xDesc := xDesc + redfire + "Resistencia Fogo <br>";
						elseif (mod == "EnergyResistMod")
							xDesc := xDesc + lilas  + "Resistencia Energia <br>";
						elseif (mod == "PoisonResistMod")
							xDesc := xDesc + softgreen  + "Resistencia Veneno <br>";
						else
							xDesc := xDesc + roxo + "Bonus " + mod + "<br>";
						endif
					endforeach
				endif
			endif
		endif
		return Object_Name + "<br>" + xDesc;
	endif
	return "";
endfunction

//COPIA DO GERADOR DE TOOLTIP DO MEGACLILOC.SRC

function GetDmgStringDesc(elem, item)
	var dmg_mod := CInt(item.dmg_mod);
	var resultado := CSTR(elem.MinDam) + "-" + CSTR(elem.MaxDam);
	if (dmg_mod>0)
		resultado := resultado + " (+" + CSTR(dmg_mod) +")";
	elseif (dmg_mod<0)
		resultado := resultado + " (" + CSTR(dmg_mod) +")";
	endif
	return resultado;
	/*
	var avgdmg := cint(elem.MinDam + elem.MaxDam)/2;

	if(avgdmg >= 20)
		return "Letal";
	elseif(avgdmg >= 18)
		return "Muito";
	elseif(avgdmg >= 16)
		return "Bom";
	elseif(avgdmg >= 14)
		return "Mediano";
	elseif(avgdmg >= 12)
		return "Pouco";
	elseif(avgdmg >= 10)
		return "Pouquissimo ";
	else
		return "Inofensivo";
	endif
	*/

endfunction

function getQualityTemplate(num)
	var rank;
	if(num>2.8)
		rank := "Lendária";
	elseif(num>2.5)
		rank := "Épica";
	elseif(num>2.2)
		rank := "Rara";
	elseif(num>1.9)
		rank := "Excepcional";
	else
		rank := "Aprimorada";
	endif
	return rank;
endfunction

function GetConditionTemplate(item) //Durabilidade dos itens
	return CSTR(item.hp) + "/" + CSTR(item.maxhp);
	/*
	var c := cdbl(item.hp) / cdbl(item.maxhp);

	if (c > 0.9)
		return "Boa";
	elseif (c > 0.6)
		return "Pouco Danificada";
	elseif (c > 0.3)
		return "Bastante Danificada";
	else
		return "Quebradiça";
	endif
	*/
endfunction

function TranslateCriticalTemplate(crit)
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


function GetCriticalInfoTemplate(entry)

	var criticals := GetconfigStringArray(entry, "CriticalHitScripts");

	var critstr := "";

	var str;
	foreach crit in criticals
		str := ParseScriptStringTemplate(crit);
		critstr := critstr + TranslateCriticalTemplate(lower(str)) + "<br>";
	endforeach

	return critstr;

endfunction

function TranslateHitTemplate(crit)
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

function GetHitsInfo(entry)

	var criticals := GetconfigStringArray(entry, "HitScripts");

	var critstr := "";

	var str;
	foreach crit in criticals
		str := ParseScriptStringTemplate(crit);
		critstr := critstr + TranslateHitTemplate(lower(str)) + "<br>";
	endforeach

	return critstr;

endfunction

function GetTipoStringDescTemplate(elem, item) //Exibe o tipo de arma
	if (elem.TipoAtaque)
		return CStr(elem.TipoAtaque) + "<br>" + CSTR(elem.Attribute);
	elseif (elem.TipoProtecao)
		return CStr(elem.TipoProtecao) + "<br>";
	endif
endfunction


function GetVelStringDescTemplate(elem, item) //Velocidade da arma. Mostra diferença pra item base
	var speedI := CInt(item.CProps.attackspeedbonus);
	var speedE := Cint(elem.Speed);
	if (speedI != error)
		return CSTR(elem.speed) + " (+" + CSTR(speedI) + ")";
	elseif (speedI < 0)
		return CSTR(elem.speed) + " (" + CSTR(speedI) + ")";
	else
		return CSTR(elem.speed);
	endif
endfunction

function getParryBonusTemplate(elem, item)
	var parrybonusI := Cint(item.CProps.parrybonus);
	var parrybonusE := Cint(elem.parrybonus);
	if (parrybonusI > 0)
		return CSTR(parrybonusE) + " (+" + CSTR(parrybonusI) + ")";
	elseif (parrybonusI < 0)
		return CSTR(parrybonusE) + " (" + CSTR(parrybonusI) + ")";
	else
		return CSTR(parrybonusE);
	endif
endfunction

function getParryDelayTemplate(elem, item)
	var parrydelayI := Cint(item.CProps.parrydelay_mod);
	var parrydelayE := Cint(elem.parrydelay);

	if (parrydelayI > 0)
		return CSTR(parrydelayE) + " (-" + CSTR(parrydelayI) + ")";
	elseif (parrydelayI < 0)
		return CSTR(parrydelayE) + " (+" + CSTR(-parrydelayI) + ")";
	else
		return CSTR(parrydelayE);
	endif
endfunction

function getDexPenaltyTemplate(elem, item)
	var DexPenaltyI := Cint(item.CProps.penaltybonus);
	var DexPenaltyE := Cint(elem.DexPenalty);

	if (DexPenaltyI > 0)
		return CSTR(DexPenaltyE) + " (-" + CSTR(DexPenaltyI) + ")";
	elseif (DexPenaltyI < 0)
		return CSTR(DexPenaltyE) + " (+" + CSTR(-DexPenaltyI) + ")";
	else
		return CSTR(DexPenaltyE);
	endif
endfunction

function ParseScriptStringTemplate(crit)

	var str := "";
	var i := 1;

	while(crit[i] != "/")
		i := i + 1;
	endwhile
	i := i +1;
	while (crit[i])
		str := str + crit[i];
		i := i + 1;
	endwhile

	return str;

endfunction