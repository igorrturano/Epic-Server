use uo;
use cfgfile;

class EquipWrapper()
	var _name; // String
	var _desc; // String
	var _graphic; // Int
	var _weight; // Int
	var _maxHP; // Int

	function EquipWrapper(this, item)
		var template, name, desc, weight, maxHP, graphic;

		if (item.isA(POLCLASS_EQUIPMENT)) // Caso o itemRef exista
			template := item.getCfgElem();

			this._name := template.name;
			this._desc := item.desc;
			this._weight := item.weight;
			this._maxHP := item.maxHP;
			this._graphic := item.graphic;
			
		else // Caso seja apenas um template
			template := FindConfigElem(ReadConfigFile(":*:itemdesc"), item);

			this._name := template.name;
			this._desc := template.desc;
			this._weight := template.weight;
			this._maxHP := template.maxHP;
			this._graphic := template.graphic;
		endif
	endfunction

	function GetName(this)
		return this._name;
	endfunction

	function GetDesc(this)
		return this._desc;
	endfunction

	function GetGraphic(this)
		return this._graphic;
	endfunction

	function GetWeight(this)
		return this._weight;
	endfunction

	function GetMaxHP(this)
		return this._maxHP;
	endfunction


	function ToString(this)
		return $"Equipment:
		Name: {this.GetName()},
		Description: {this.GetDesc()},
		Graphic: {this.GetGraphic()} ({Hex(this.GetGraphic())}),
		Weight: {this.GetWeight()},
		MaxHP: {this.GetMaxHP()}";
	endfunction
endclass