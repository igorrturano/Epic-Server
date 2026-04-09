include "system/observer/models/Event";
include ":message:Model/Message";

class SysNotificationEvent(Event)
	var Message; // Objeto Message a ser enviado quando a notificação for acionada.
	var Targets := array{}; // array de mobRef de quem vai receber a notificação.
	
	function SysNotificationEvent(this, source, message, targets)
		Super(source);
		this.Message := message;
		this.Targets := targets;
	endfunction
endclass