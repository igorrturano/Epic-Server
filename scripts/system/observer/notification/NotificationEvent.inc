include "system/observer/models/Event";

class NotificationEvent(Event)
	var NotificationType;
	var Message;
	var Recipient;
	
	function NotificationEvent(this, source, payload)
		Super(source);
	endfunction

	function Notify(this)
		// Lógica para enviar a notificação ao destinatário
		Message(this.Message).SendSys(this.Recipient);
	endfunction
endclass