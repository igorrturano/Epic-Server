include "EquipWrapper";
include "IArmorWrapper";
include ":equipsys:Controller/ArmorController";
include ":combat:Model/ResistModCollection";
include ":combat:Model/ResistModFabricacao";
include ":combat:Model/ResistModRuna";

include ":combat:Model/ArmorResist";
include ":combat:Model/ShieldAttributes";

class ArmorWrapper(EquipWrapper, IArmorWrapper)
	var Coverage := array{};
	var TipoProtecao := "";

	var Resists; // ArmorResist class
	var ResistsMod; // ArmorResist class
	var ShieldAttributesVar; // ShieldAttributes class
	var ShieldAttributesMod; // ShieldAttributes class

	function ArmorWrapper(this, byRef item)
		Super(item);

		if (this.GetTemplate())
			this.Coverage := GetConfigStringArray(this.GetTemplate(), "Coverage");
			this.TipoProtecao := this.GetTemplate().TipoProtecao ?: error{ errortext := "The item have no Protection Type!" };
			this.Resists := this.__CalculateArmorResist();
			this.ResistsMod := ResistModCollection();
			this.SetResistsMod(ResistModFabricacao(this.GetObjRef() ? this.GetObjRef() : this.GetTemplate()));
			if (this.GetObjRef())
				var _runeType := GetObjProperty(this.GetObjRef(), "priestrune");
				var _runeCharges := CInt(GetObjProperty(this.GetObjRef(), "runecharges"));
				if (_runeType && _runeCharges > 0)
					this.SetResistsMod(ResistModRuna(this.GetObjRef()));
				endif
			endif

			this.WeightVar := this.__CalculateWeight();
			this.STRRequired := this.__CalculateSTRRequired();
			this.__InitModifiers();
			this.CriticalReductionMod := PegaValorFabricacao(this.GetObjRef(),CARACTERISTICA_FABRICACAO_REDUCAOCRITICO);
		else
			var err := InvalidTemplateException("The param must be an Armor ObjRef or a valid Template!");
			this.Coverage := err;
			this.TipoProtecao := err;
			this.Resists := err;
			this.ResistsMod := err;
			this.ShieldAttributesVar := err;
			this.ShieldAttributesMod := err;
			this.CriticalReductionMod := err;
		endif
	endfunction
	
	function GetTipoProtecao(this)
		return this.TipoProtecao;
	endfunction

	/**
	 * Obtém a região do corpo protegida pela armadura
	 *
	 * @return String contendo a área de cobertura da armadura
	 */
	function GetCoverage(this)
		return this.Coverage;
	endfunction

	/**
	 * Calcula e retorna as resistências totais da armadura.
	 * 
	 * Combina os valores de resistência base com os modificadores,
	 * somando cada tipo de resistência individualmente.
	 * 
	 * @return ArmorResist Objeto contendo todas as resistências calculadas (corte, perfuração, 
	 *                     contusão, fogo, gelo, energia e veneno), ou Exception em caso de erro
	 *                     ao obter as resistências base.
	 */
	function GetResists(this)
		var base := this.GetResistsBase();
		var mod := this.GetResistsMod();

		if (base is @Exception)
			return base;
		endif

		var combined := ArmorResist(
			CInt(base.GetSlashResist()) + CInt(mod.GetSlashResist()),
			CInt(base.GetPiercingResist()) + CInt(mod.GetPiercingResist()),
			CInt(base.GetBluntResist()) + CInt(mod.GetBluntResist()),
			CInt(base.GetFireResist()) + CInt(mod.GetFireResist()),
			CInt(base.GetColdResist()) + CInt(mod.GetColdResist()),
			CInt(base.GetPoisonResist()) + CInt(mod.GetPoisonResist()),
			CInt(base.GetEnergyResist()) + CInt(mod.GetEnergyResist())
		);

		return this._ApplyTypeCap(combined);
	endfunction

	/**
	 * Retorna os resistências base da armadura.
	 *
	 * @return ArmorResist Objeto contendo as resistências da armadura
	 */
	function GetResistsBase(this)
		return this.Resists;
	endfunction

	/**
	 * Retorna o modificador de resistências da armadura.
	 *
	 * @return ArmorResist Objeto contendo os modificadores de resistência da armadura
	 */
	function GetResistsMod(this)
		return this.ResistsMod.GetTotal();
	endfunction

	function GetCriticalReduction(this)
		return this.CriticalReductionMod;
	endfunction

	/**
	 * Obtém os atributos de escudo combinados (base + modificadores).
	 *
	 * Calcula e retorna os atributos finais do escudo somando os valores base
	 * com os modificadores aplicados. Se houver erro ao obter atributos base,
	 * retorna a exceção.
	 *
	 * @return ShieldAttributes Objeto contendo ParryBonus e ParryDelay calculados
	 * @return Exception Se houver erro ao obter atributos base
	 */
	function GetShieldAttributes(this)
		var base := this.GetShieldAttributesBase();
		var mod := this.GetShieldAttributesMod();

		if (base is @Exception)
			return base;
		endif

		return ShieldAttributes(
			CInt(base.GetParryBonus()) + CInt(mod.GetParryBonus()),
			CInt(base.GetParryDelay()) + CInt(mod.GetParryDelay())
		);
	endfunction
	
	/**
	 * Retorna os atributos base do escudo.
	 *
	 * @return Objeto ShieldAttributes contendo os atributos base do escudo
	 */
	function GetShieldAttributesBase(this)
		return this.ShieldAttributesVar;
	endfunction

	function GetSTRRequired(this)
		return this.STRRequired;
	endfunction

	/**
	 * Retorna as modificações dos atributos do escudo
	 *
	 * @return objeto ShieldAttributes contendo as modificações dos atributos do escudo
	 */
	function GetShieldAttributesMod(this)
		return this.ShieldAttributesMod;
	endfunction

	function SetResistsBase(this, resists)
		if (this.GetObjRef() is @Exception) return this.GetObjRef(); endif
		
		this.Resists := resists;

		return this;
	endfunction

	/**
	 * Define o modificador de resistências do objeto de armadura.
	 * 
	 * @param resists Objeto contendo os modificadores de resistência a serem aplicados
	 *
	 * @return Retorna a própria instância do objeto para encadeamento de métodos, ou um objeto Exception em caso de erro
	 */
	function SetResistsMod(this, resists)
		if (this.GetObjRef() is @Exception) return this.GetObjRef(); endif
		var ret := this.ResistsMod.InsertMod(resists);

		if(ret is @Exception) return this.ret; endif

		return this;
	endfunction

	function SetCriticalReductionMod(this, criticalReduction)
		this.CriticalReductionMod := criticalReduction;
		return this;
	endfunction

	/**
	 * Define os atributos base do escudo para o objeto.
	 *
	 * @param shieldAttrs Objeto ShieldAttributes contendo os atributos do escudo a serem definidos
	 *
	 * @return Retorna a própria instância do objeto em caso de sucesso, ou uma Exception em caso de erro
	 */
	function SetShieldAttributesBase(this, shieldAttrs)
		if (this.GetObjRef() is @Exception) return this.GetObjRef(); endif
		if (!(shieldAttrs is @ShieldAttributes)) return InvalidFormatException("Param must be a ShieldAttributes!"); endif

		this.ShieldAttributesVar := shieldAttrs;

		return this;
	endfunction

	/**
	 * Define os modificadores de atributos de escudo do objeto.
	 * 
	 * @param {ShieldAttributes} shieldAttrs - Os atributos de escudo a serem aplicados como modificadores
	 * @return {Object|Exception} Retorna o próprio objeto para encadeamento ou uma Exception em caso de erro
	 * @throws {Exception} Se a referência do objeto for inválida
	 * @throws {InvalidFormatException} Se shieldAttrs não for do tipo ShieldAttributes
	 */
	function SetShieldAttributesMod(this, shieldAttrs)
		if (this.GetObjRef() is @Exception) return this.GetObjRef(); endif
		if (!(shieldAttrs is @ShieldAttributes)) return InvalidFormatException("Param must be a ShieldAttributes!"); endif

		this.ShieldAttributesMod := shieldAttrs;

		return this;
	endfunction

	function IsShield(this)
		return Upper(SplitWords(this.TipoProtecao)[1]) == "ESCUDO";
	endfunction

	function Update(this)
		if (this.GetObjRef() is @Exception) return this.GetObjRef(); endif
		EquipWrapper::Update(this);
		
		var prop := this.GetEquipProp();
		prop.+SlashResist := this.Resists.GetSlashResist();
		prop.+PiercingResist := this.Resists.GetPiercingResist();
		prop.+BluntResist := this.Resists.GetBluntResist();
		prop.+FireResist := this.Resists.GetFireResist();
		prop.+ColdResist := this.Resists.GetColdResist();
		prop.+EnergyResist := this.Resists.GetEnergyResist();
		prop.+PoisonResist := this.Resists.GetPoisonResist();
		this.__InitModifiers();
		foreach modifier in (this.Modifiers)
			prop.Modifiers[modifier.GetAttribute()] := modifier.GetValue();
		endforeach
		this.SetEquipProp(prop);
		
		// Aqui seta as propriedades no ObjRef
	endfunction

	function ToString(this)
		var str := Super::ToString(this) + " Armor:" +
			$"\nCoverage: {this.GetCoverage()}, Protection Type: {this.GetTipoProtecao()}" +
			$"\nSlashResist: {this.Resists.GetSlashResist()} (+{this.ResistsMod.GetSlashResist()})" +
			$"\nPiercingResist: {this.Resists.GetPiercingResist()} (+{this.ResistsMod.GetPiercingResist()})" +
			$"\nBluntResist: {this.Resists.GetBluntResist()} (+{this.ResistsMod.GetBluntResist()})" +
			
			"\n\nModifiers:";

		foreach modifier in (this.Modifiers.values())
			str += $"\n{modifier.ToString()}, ";
		endforeach

		return str;
	endfunction

	// Private functions
	function __CalculateWeight(this)
		// TODO: Revisar para ver está calculando correto
		var weight := 1;
		
		if (this.IsShield()) // Se for escudo, peso é o DEX Penalty
			weight := -(this.Modifiers[DEXTERITY].GetValue()) ?: 1;
		else // Se não for, é a soma do maior valor de resistência + resistência por slot
			weight := Max(array{this.Resists.GetSlashResist(), this.Resists.GetPiercingResist(), this.Resists.GetBluntResist()});
			weight += CInt(getResistPhysicalBySlot(this.GetCoverage()[1]));
		endif

		weight *= GetWeightMultiplierByMaterial(this.GetMaterial());

		return weight;
	endfunction

	function __CalculateSTRRequired(this)
		return CInt(this.GetWeight() * 7.5);
	endfunction

	function __CalculateArmorResist(this)
		var templateResists := getResistPhysicalByProtection(this.GetTemplate().tipoProtecao);
		// var slotResists := this.GetTipoProtecao() ? getResistPhysicalBySlot(this.GetCoverage()) : ArmorResist();
		// Slot bonus removed — armor tiers + crafting provide sufficient defense

		// Floor por tipo de armadura (Light 10 / Medium 15 / Heavy 20). Aplicado
		// apenas em fisico (slash/pierce/blunt) — elementais vem de material
		// sub-effects, encantamento e runas. Ver design_combate.md.
		var floor := this._GetPhysicalFloor();

		return ArmorResist(
			templateResists.GetSlashResist() + floor,
			templateResists.GetPiercingResist() + floor,
			templateResists.GetBluntResist() + floor,
			this.GetTemplate().FireResist,
			this.GetTemplate().ColdResist,
			this.GetTemplate().PoisonResist,
			this.GetTemplate().EnergyResist
		);
	endfunction

	function _GetPhysicalFloor(this)
		var tp := this.GetTipoProtecao();
		if (!tp || tp is @Exception) return 0; endif
		if (tp["Armadura Leve"]) return 10; endif
		if (tp["Armadura Media"]) return 15; endif
		if (tp["Armadura Pesada"]) return 20; endif
		return 0;
	endfunction

	function _GetTypeCap(this)
		var tp := this.GetTipoProtecao();
		if (!tp || tp is @Exception) return 0; endif
		if (tp["Armadura Leve"]) return 50; endif
		if (tp["Armadura Media"]) return 60; endif
		if (tp["Armadura Pesada"]) return 70; endif
		return 0;
	endfunction

	function _ApplyTypeCap(this, byRef resists)
		var cap := this._GetTypeCap();
		if (cap <= 0) return resists; endif

		if (resists.GetSlashResist() > cap) resists.SetSlashResist(cap); endif
		if (resists.GetPiercingResist() > cap) resists.SetPiercingResist(cap); endif
		if (resists.GetBluntResist() > cap) resists.SetBluntResist(cap); endif
		if (resists.GetFireResist() > cap) resists.SetFireResist(cap); endif
		if (resists.GetColdResist() > cap) resists.SetColdResist(cap); endif
		if (resists.GetPoisonResist() > cap) resists.SetPoisonResist(cap); endif
		if (resists.GetEnergyResist() > cap) resists.SetEnergyResist(cap); endif

		return resists;
	endfunction

	function __InitModifiers(this)
		this.Modifiers := dictionary{};

		var storedMods := (!(this.EquipProp is @Exception)) ? this.EquipProp.Modifiers : 0;
		if (storedMods && storedMods.size() >= 1)
			foreach modifier in (storedMods.keys())
				this.SetModifier(AttributeModifier(modifier, storedMods[modifier]));
			endforeach
		else
			if (this.GetTipoProtecao())
				var modifiersSlot := GetModifiersBySlot(this.GetCoverage()[1]);
				var modifiersProt := GetModifiersByProtection(this.GetTipoProtecao());

				// Armadura Leve não penaliza DEX por slot — couro/tecido deveria ser ágil
				var slotDex := modifiersSlot[DEXTERITY].GetValue();
				if (this.GetTipoProtecao()["Armadura Leve"])
					slotDex := 0;
				endif

				this.SetModifier(AttributeModifier(DEXTERITY, slotDex + modifiersProt[DEXTERITY].GetValue()));
				this.SetModifier(AttributeModifier(SNEAK, modifiersSlot[SNEAK].GetValue() + modifiersProt[SNEAK].GetValue()));
				this.SetModifier(AttributeModifier(MECHANICS, modifiersSlot[MECHANICS].GetValue() + modifiersProt[MECHANICS].GetValue()));
				this.SetModifier(AttributeModifier(MAGERY, modifiersSlot[MAGERY].GetValue() + modifiersProt[MAGERY].GetValue()));
				var awarenessProtPenalty := (Upper(this.GetCoverage()[1]) == Upper(HEAD)) ? modifiersProt[AWARENESS].GetValue() : 0;
				this.SetModifier(AttributeModifier(AWARENESS, modifiersSlot[AWARENESS].GetValue() + awarenessProtPenalty));
			endif
		endif
	endfunction
endclass
