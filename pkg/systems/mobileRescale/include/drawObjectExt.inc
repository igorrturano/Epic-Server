use uo;

// Retorna os 6 bytes de escala + traços raciais para ser anexado ao
// final de qualquer DrawObject manual (após o terminador 00000000).
// Estrutura esperada pelo cliente:
//   uint16 ScaleY  (height * 1000)
//   uint16 ScaleX  (width  * 1000)
//   uint8  HeightModifier
//   uint8  MuscleModifier
function GetScaleTraitsHex(kobj)
    // escala
    var scaleH := 1000;
    var scaleW := 1000;
    var scaleProp := GetObjProperty(kobj, "entityScale");
    if (scaleProp && scaleProp.height && scaleProp.width)
        var h := CInt(scaleProp.height);
        var w := CInt(scaleProp.width);
        if (h > 0 && h <= 60000) scaleH := h; endif
        if (w > 0 && w <= 60000) scaleW := w; endif
    endif

    // traços raciais (ALTURA_RACIAL::MEDIO = 2, PESO_RACIAL::MEDIO = 2)
    var altura := 2;
    var peso   := 2;
    var chardata := GetObjProperty(kobj, "chardata");
    if (chardata && chardata.caracteristicas_rp)
        var ch := CInt(chardata.caracteristicas_rp.Altura);
        var cw := CInt(chardata.caracteristicas_rp.Peso);
        if (ch >= 0 && ch <= 200) altura := ch; endif
        if (cw >= 0 && cw <= 200) peso   := cw; endif
    endif

    return _ScalePadHex(scaleH, 4)
         + _ScalePadHex(scaleW, 4)
         + _ScalePadHex(altura, 2)
         + _ScalePadHex(peso,   2);
endfunction

// Converte inteiro para hex com padding (nBytes = nº de bytes → nBytes*2 chars hex)
function _ScalePadHex(val, nChars)
    if (val < 0) val := (1 << (nChars * 4)) + val; endif
    var s := CStr(Hex(val));
    if (s[1,2] == "0x") s := s[3, len(s)]; endif
    while (len(s) < nChars) s := "0" + s; endwhile
    return s;
endfunction
