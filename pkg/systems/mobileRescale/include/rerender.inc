use uo;
use os;

// Força o cliente a re-renderizar a escala do mobile emitindo um packet 0x78.
// O hook de escala só dispara quando o objeto entra no campo de visão, então
// é preciso teleportar para fora do range e voltar. Tenta 4 direções para
// lidar com bordas de mapa onde +30,+30 pode ser inválido.
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

    foreach off in offsets
        if (MoveObjectToLocation(mobile, ox + off[1], oy + off[2], oz, realm, MOVEOBJECT_FORCELOCATION))
            sleepms(100);
            MoveObjectToLocation(mobile, ox, oy, oz, realm, MOVEOBJECT_FORCELOCATION);
            return 1;
        endif
    endforeach

    return 0;
endfunction
