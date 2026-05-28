use uo;
use os;

include ":mobileRescale:include/drawObjectExt";

// Força o cliente a re-renderizar a escala do mobile emitindo um packet 0x78.
// O hook de escala só dispara quando o objeto entra no campo de visão, então
// é preciso teleportar para fora do range e voltar. Tenta 4 direções para
// lidar com bordas de mapa onde +30,+30 pode ser inválido.
//
// BUG: quando o mobile é o próprio jogador, o teleporte envia 0x20
// (DrawGamePlayer) ao jogador — sem bytes de escala. Outros clientes recebem
// 0x78 corretamente. Fix: enviar DrawObject manual com escala ao próprio jogador.
function ForceScaleRerender(mobile)
    var ox    := mobile.x;
    var oy    := mobile.y;
    var oz    := mobile.z;
    var realm := mobile.realm;

    var offsets := array{
        array{ 30,  0 },
        array{  0, 30 },
        array{-30,  0 },
        array{  0,-30 }
    };

    var ok := 0;
    foreach off in offsets
        if (MoveObjectToLocation(mobile, ox + off[1], oy + off[2], oz, realm, MOVEOBJECT_FORCELOCATION))
            sleepms(100);
            MoveObjectToLocation(mobile, ox, oy, oz, realm, MOVEOBJECT_FORCELOCATION);
            ok := 1;
            break;
        endif
    endforeach

    if (mobile.connected)
        _SendSelfScaledDrawObject(mobile);
    endif

    return ok;
endfunction

// Envia um packet 0x78 (DrawObject) com bytes de escala diretamente ao jogador
// para o seu próprio mobile — necessário porque 0x20 não carrega escala.
//
// Usa formato moderno (CV_70331+, versão 7.0.60.47): cor SEMPRE presente (2 bytes)
// independente do valor; sem flag 0x8000 no graphic. Formato antigo causava
// desalinhamento do stream após o primeiro item sem cor, lendo bytes de escala
// da posição errada.
function _SendSelfScaledDrawObject(mobile)
    var g := mobile.graphic;

    var p2 := _ScalePadHex(mobile.serial, 8)
            + _ScalePadHex(g,             4)
            + _ScalePadHex(mobile.x,      4)
            + _ScalePadHex(mobile.y,      4)
            + _ScalePadHex(mobile.z,      2)
            + _ScalePadHex(mobile.facing, 2)
            + _ScalePadHex(mobile.color,  4)
            + "0000";

    if (g == 400 || g == 401 ||
        g == 1828 || g == 1829 || g == 1830 || g == 1831 ||
        g == 1832 || g == 1833 || g == 1834 || g == 1835)
        foreach item in ListEquippedItems(mobile)
            // Formato CV_70331+: graphic sem bit 0x8000; cor sempre presente (2 bytes)
            p2 := p2 + _ScalePadHex(item.serial,  8)
                     + _ScalePadHex(item.graphic,  4)
                     + _ScalePadHex(item.layer,    2)
                     + _ScalePadHex(item.color,    4);
        endforeach
    endif

    p2 := p2 + "00000000"              // terminador da lista de itens
            + GetScaleTraitsHex(mobile) // escala + traços raciais (6 bytes)
            + "03e803e8";               // mount scale 1000/1000 (4 bytes)

    var pktLen := (len(p2) + 2) / 2 + 2;
    SendPacket(mobile, "78" + _ScalePadHex(pktLen, 4) + p2);
endfunction
