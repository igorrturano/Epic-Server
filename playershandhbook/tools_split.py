#!/usr/bin/env python3
"""One-shot splitter: homebrewery.txt -> src/ chapter files.

Segments are (relative_path, start_line, end_line), 1-based inclusive,
and MUST be contiguous and cover every line so that
  cat $(find src -name '*.md' | sort)  ==  homebrewery.txt
byte for byte. The script asserts this before writing anything.
"""
import os
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(ROOT, "homebrewery.txt")

SEGMENTS = [
    ("00-frontmatter.md", 1, 71),
    ("01-indice.md", 72, 235),
    ("10-parte1-sistemas.md", 236, 503),
    ("11-parte2-como-jogar.md", 504, 563),
    ("12-parte3-interpretacao.md", 564, 737),
    ("20-parte4-ambientacao.md", 738, 785),
    ("21-povos/00-aiglanos.md", 786, 1010),
    ("21-povos/01-baduran.md", 1011, 1181),
    ("21-povos/02-bjorske.md", 1182, 1334),
    ("21-povos/03-haroloth.md", 1335, 1495),
    ("21-povos/04-aluriel.md", 1496, 1754),
    ("21-povos/05-gulthrak.md", 1755, 1907),
    ("21-povos/06-colhesiana.md", 1908, 2105),
    ("21-povos/07-povos-do-leste.md", 2106, 2291),
    ("22-racas.md", 2292, 2384),
    ("23-faccoes-guildas.md", 2385, 2561),
    ("30-caminhos-classes.md", 2562, 3334),
    ("40-caminho-armas/00-intro.md", 3335, 3419),
    ("40-caminho-armas/01-aklat-tur.md", 3420, 3531),
    ("40-caminho-armas/02-arqueiro.md", 3532, 3650),
    ("40-caminho-armas/03-barbaro.md", 3651, 3765),
    ("40-caminho-armas/04-cacador.md", 3766, 3873),
    ("40-caminho-armas/05-combatente.md", 3874, 3958),
    ("40-caminho-armas/06-defensor.md", 3959, 4075),
    ("40-caminho-armas/07-monge.md", 4076, 4189),
    ("40-caminho-armas/08-mutante.md", 4190, 4303),
    ("40-caminho-armas/09-paladino.md", 4304, 4417),
    ("40-caminho-armas/10-samurai.md", 4418, 4536),
    ("50-caminho-sabedoria/00-intro.md", 4537, 4623),
    ("50-caminho-sabedoria/01-arqueiro-arcano.md", 4624, 4744),
    ("50-caminho-sabedoria/02-clerigo.md", 4745, 4864),
    ("50-caminho-sabedoria/03-construtor-de-golems.md", 4865, 4980),
    ("50-caminho-sabedoria/04-deslocador-das-sombras.md", 4981, 5101),
    ("50-caminho-sabedoria/05-druida.md", 5102, 5211),
    ("50-caminho-sabedoria/06-granadeiro.md", 5212, 5324),
    ("50-caminho-sabedoria/07-lamina-arcana.md", 5325, 5441),
    ("50-caminho-sabedoria/08-magus.md", 5442, 5555),
    ("50-caminho-sabedoria/09-necromante.md", 5556, 5662),
    ("50-caminho-sabedoria/10-xama.md", 5663, 5783),
    ("60-caminho-subterfugio/00-intro.md", 5784, 5866),
    ("60-caminho-subterfugio/01-assassino.md", 5867, 6004),
    ("60-caminho-subterfugio/02-bardo.md", 6005, 6119),
    ("60-caminho-subterfugio/03-cacador-de-demonios.md", 6120, 6239),
    ("60-caminho-subterfugio/04-cacador-de-tesouros.md", 6240, 6355),
    ("60-caminho-subterfugio/05-dancarino-das-sombras.md", 6356, 6478),
    ("60-caminho-subterfugio/06-duelista.md", 6479, 6593),
    ("60-caminho-subterfugio/07-shinobi.md", 6594, 6705),
    ("60-caminho-subterfugio/08-malandro-arcano.md", 6706, 6820),
    ("60-caminho-subterfugio/09-ladrao.md", 6821, 6936),
    ("60-caminho-subterfugio/10-vivisseccionista.md", 6937, 7054),
    ("70-passados.md", 7055, 7211),
    ("71-caracteristicas.md", 7212, 7439),
    ("72-pericias.md", 7440, 7566),
    ("80-magias/00-intro.md", 7567, 7635),
    ("80-magias/10-truques.md", 7636, 8011),
    ("80-magias/20-canticos-bardo.md", 8012, 8229),
    ("80-magias/30-ninjutsu.md", 8230, 8369),
    ("80-magias/40-divinas-intro.md", 8370, 8386),
    ("80-magias/41-paladino.md", 8387, 8546),
    ("80-magias/42-cacador-de-demonios.md", 8547, 8701),
    ("80-magias/43-clerigo.md", 8702, 9026),
    ("80-magias/50-naturais-intro.md", 9027, 9044),
    ("80-magias/51-druida.md", 9045, 9321),
    ("80-magias/52-xama.md", 9322, 9592),
    ("80-magias/60-primordiais.md", 9593, 10764),
    ("99-backcover.md", 10765, 10797),
]


def main():
    with open(SRC, "r", encoding="utf-8", newline="") as fh:
        lines = fh.readlines()
    total = len(lines)

    # Contiguity + full coverage check before any write.
    expected = 1
    for path, start, end in SEGMENTS:
        if start != expected:
            sys.exit(f"GAP/OVERLAP at {path}: expected start {expected}, got {start}")
        if end < start:
            sys.exit(f"BAD RANGE at {path}: {start}-{end}")
        expected = end + 1
    if expected - 1 != total:
        sys.exit(f"COVERAGE: segments end at {expected - 1}, file has {total} lines")

    outdir = os.path.join(ROOT, "src")
    for path, start, end in SEGMENTS:
        dest = os.path.join(outdir, path)
        os.makedirs(os.path.dirname(dest), exist_ok=True)
        with open(dest, "w", encoding="utf-8", newline="") as fh:
            fh.writelines(lines[start - 1:end])

    print(f"wrote {len(SEGMENTS)} files covering {total} lines")


if __name__ == "__main__":
    main()
