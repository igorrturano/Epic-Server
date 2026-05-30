
use util;
include ":propagacao:include/propagacao";

class IMarcadorPropagacao()
	uninit function Iniciar(this);
	uninit function Propagar(this);
	uninit function EfetuarCiclo(this);
	uninit function EnviaEstimulo(this, valorEstimulo);
	uninit function RecebeEstimulo(this, valorEstimulo, isAcumulando);
	uninit function InjetaCombustivel(this, deveResetarEstado);
	uninit function GanchoAntesAtualizaEstado(this, estado);
	uninit function AtualizaComportamentoMarcador(this);
endclass