include "Tooltip";

// TODO: Incorporar essa classe nas demais
	// Adicionar uma variável que receba esta classe, se ela existir, o tooltip é criado junto com o SelfCreate() da outra.

class TooltipText(Tooltip)
	var _text;

	function TooltipText(this, name, text)
		Super(name);
		this._text := text;
	endfunction

	function GetText(this)
		return this._text;
	endfunction

	function SetText(this, text)
		this._text := text;
	endfunction

	function SelfCreate(this, byRef gump)
		GFTooltipText(gump, this.GetText());
	endfunction

	function ToString(this)
		return "TooltipText(type=" + this.GetType()
			+ ", name=" + this._GetName()
			+ ", text=" + this.GetText()
			+ ")";
	endfunction
endclass