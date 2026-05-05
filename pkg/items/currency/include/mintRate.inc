// Phase 4 — Real implementation, delegates to seignorage.inc.
// API permanece estável pra callers (moneyer, gemvendor):
//   GetMintRate(coin_type, qty) → multiplicador efetivo (bracket walk).

include ":currency:include/seignorage";
