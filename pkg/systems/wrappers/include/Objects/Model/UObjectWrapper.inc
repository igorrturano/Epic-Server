use uo;

include ":wrappers:Objects/Exceptions/_all";
include "system/exception/FormatException/InvalidFormatException";

include "IUObjectWrapper";

include ":wrappers:Position/Model/PositionWrapper";
include "include/packets";
include ":text:Color";

class UObjectWrapper(IUObjectWrapper)
	var CfgFile;
	var Template;
	var ObjRef; // ObjRef
	var Serial := 0x0;
	var ObjType := 0x0;
	var Name := "";
	var Graphic := 0;
	var Color := 0;
	var Weight := 0;
	var Category := ""; // CATEGORY enum
	var CProps;

	function UObjectWrapper(this, byRef object)
		if (TypeOfInt(object) == OT_MOBILEREF || TypeOfInt(object) == OT_OFFLINEMOBILEREF || TypeOfInt(object) == OT_ITEMREF)
			this.ObjRef := object;
			this.Serial := object.serial;
			this.ObjType := object.objtype;
			this.Name := object.name;
			this.Graphic := object.graphic;
			this.Color := object.color;
			this.Weight := object.weight;
		elseif (TypeOfInt(object) == OT_CONFIGELEMREF)
			this.ObjType := GetObjtypeByName(object.name);
			this.CProps := object.CProps;
		elseif (object.isVendorTemplate)
			this.Template := GetObjtypeByName(object.name);
			this.ObjType := object.objtype;
			this.Name := object.name;
			this.Graphic := object.graphic;
			this.Color := object.color;
			this.Weight := object.weight;
			this.CProps := object.CProps;
		else
			var objRefBySerial := SystemFindObjectBySerial(object);
			if (objRefBySerial)
				object := objRefBySerial;
				this.ObjRef := object;
				this.Serial := object.serial;
				this.ObjType := object.objtype;
				this.Name := object.name;
				this.Graphic := object.graphic;
				this.Color := object.color;
				this.Weight := object.weight;
			else
				this.ObjType := object;
			endif
		endif
	endfunction

	function GetCfgFile(this)
		return this.CfgFile;
	endfunction
	
	function GetTemplate(this)
		return this.Template;
	endfunction
	
	function GetObjRef(this)
		return this.ObjRef;
	endfunction

	function GetSerial(this)
		return this.Serial;
	endfunction

	function GetObjType(this)
		return this.ObjType;
	endfunction

	function GetName(this)
		return this.Name;
	endfunction

	function GetGraphic(this)
		return this.Graphic;
	endfunction

	function GetColor(this)
		return this.Color;
	endfunction

	function GetWeight(this)
		return this.Weight;
	endfunction

	function SetWeight(this, weight)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		this.Weight := weight;

		// TODO: weight é um atributo R/O. Pra alterar vai ter que modificar o max_weight_mod subtraindo a diferença
		this.GetObjRef().weight := weight;

		return this;
	endfunction

	function GetPosition(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return PositionWrapper(
			this.GetObjRef().x,
			this.GetObjRef().y,
			this.GetObjRef().z,
			this.GetObjRef().realm
		);
	endfunction

	/**
	 * Obtém a categoria do objeto.
	 *
	 * @return String contendo a categoria do objeto (valor do enum CATEGORY das classes filhas)
	 */
	function GetCategory(this)
		return this.Category;
	endfunction

	function _SetName(this, name)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		this.Name := name;
		this.GetObjRef().name := name;

		return this;
	endfunction

	function SetGraphic(this, graphic)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		this.Graphic := graphic;
		this.GetObjRef().graphic := graphic;

		return this;
	endfunction

	function SetColor(this, color)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		this.Color := color;
		this.GetObjRef().color := color;

		return this;
	endfunction
	
	/**
	 * Obtém o mapa de propriedades personalizadas do template.
	 *
	 * @return Mapa contendo as propriedades personalizadas do template
	 */
	function GetCProps(this)
		return this.CProps;
	endfunction

	function GetProperty(this, propName)
		if (this.GetObjRef() is @Exception && !this.GetCProps())
			return this.GetObjRef();
		endif

		return this.GetObjRef() ? this.GetObjRef().GetProp(propName) : this.GetCProps()[propName];
	endfunction

	function SetProperty(this, propName, propValue)
		if (this.GetObjRef() is @Exception && !this.GetCProps())
			return this.GetObjRef();
		endif

		return this.GetObjRef() ? this.GetObjRef().SetProp(propName, propValue) : this.GetCProps().insert(propName, propValue);

		return this;
	endfunction

	function EraseProperty(this, propName)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		this.GetObjRef() ? this.GetObjRef().EraseProp(propName) :  this.GetCProps().erase(propName);

		return this;
	endfunction
	
	function GetMobilesInLineOfSight(this, range)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return ListMobilesInLineOfSight(this.GetObjRef(), CInt(range));
	endfunction
	
	/**
	 * Move o objeto para uma localização especificada
	 * 
	 * @param {Object} this - Referência ao objeto wrapper
	 * @param {Position} position - Objeto de posição contendo coordenadas X, Y, Z e Realm
	 * @param {Integer} flags - Flags de movimento (padrão: MOVEOBJECT_NORMAL)
	 * 
	 * @return {Integer} Retorna o resultado da operação de movimento
	 * 
	 * @example
	 * var resultado := obj.MoveToLocation(novaPosicao, MOVEOBJECT_FORCELOCATION);
	 */
	function MoveToLocation(this, position, flags := MOVEOBJECT_NORMAL)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (!(position is @PositionWrapper))
			return InvalidFormatException("The param position must be a PositionWrapper object!");
		endif

		return MoveObjectToLocation(this.GetObjRef(), position.GetX(), position.GetY(), position.GetZ(), position.GetRealm(), flags);
	endfunction

	/**
	 * Reproduz um efeito sonoro no objeto.
	 *
	 * @param this Referência à instância do objeto wrapper
	 * @param soundId ID do efeito sonoro a ser reproduzido
	 * @return Retorna a própria instância (this) para permitir encadeamento de métodos
	 */
	function PlaySound(this, soundId)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		PlaySoundEffect(this.GetObjRef(), soundId);

		return this;
	endfunction

	/**
	 * Reproduz um efeito sonoro privado para o objeto
	 * 
	 * @param this Referência ao objeto wrapper atual
	 * @param soundId ID do som a ser reproduzido
	 * @param soundSource Fonte do som (opcional). Se não fornecido, usa a referência do próprio objeto
	 * 
	 * @return Retorna a instância atual do objeto (this) para permitir encadeamento de métodos
	 */
	function PlaySoundPrivate(this, soundId, soundSource := false)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		PlaySoundEffectPrivate(this.GetObjRef(), soundId, soundSource ?: this.GetObjRef());

		return this;
	endfunction

	/**
	 * Reproduz um efeito visual animado que se move do objeto atual até um objeto alvo.
	 *
	 * @param targetWrapped    Objeto wrapper de destino para onde o efeito será direcionado
	 * @param effectId         ID do efeito visual a ser reproduzido
	 * @param speed            Velocidade do movimento do efeito (padrão: 10)
	 * @param loop             Define se o efeito deve repetir em loop (padrão: 0 - não repete)
	 * @param explode          Define se o efeito deve explodir ao atingir o alvo (padrão: 0 - não explode)
	 * @param color            Cor do efeito em formato numérico (padrão: 0)
	 * @param translucent      Define se o efeito é translúcido (padrão: 0 - não translúcido)
	 *
	 * @return this            Retorna a própria instância do objeto para permitir encadeamento de métodos
	 */
	function PlayEffectTo(this, targetWrapped, effectId, speed := 10, loop := 0, explode := false, color := 0, translucent := false)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (color)
			if (color is @Color)
				color := color.GetValue();
			else
				return InvalidFormatException("The param color must be a Color object!");
			endif
		endif

		PlayMovingEffectXYZHued(this.GetObjRef(), targetWrapped.GetObjRef(), effectId, speed, loop, explode, color, translucent);

		return this;
	endfunction

	/**
	 * Reproduz um efeito visual centralizado no objeto.
	 *
	 * @param effectId O identificador do efeito a ser reproduzido
	 * @param speed A velocidade de reprodução do efeito (padrão: 1)
	 * @param loop Define se o efeito deve repetir (0 = não repete, padrão: 0)
	 * @param color A cor do efeito (padrão: 0)
	 * @param translucent Define se o efeito é translúcido (padrão: false)
	 *
	 * @return Retorna o próprio objeto para encadeamento de métodos, ou uma exceção se a referência do objeto for inválida
	 */
	function PlayEffect(this, effectId, speed := 1, loop := 0, color := 0, translucent := false)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (color)
			if (color is @Color)
				color := color.GetValue();
			else
				return InvalidFormatException("The param color must be a Color object!");
			endif
		endif

		PlayObjectCenteredEffectEx(this.GetObjRef(), effectId, speed, loop, color, translucent);

		return this;
	endfunction

	function ToString(this)
		return "UObject: " +
			$"Serial: {this.Serial}, " +
			$"ObjType: {this.ObjType}, " +
			$"Name: {this.Name}, " +
			$"Graphic: {this.Graphic}, " +
			$"Color: {this.Color}";
	endfunction
endclass
