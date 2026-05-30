# Livro do Jogador — Epic! Shard

The handbook is a single Homebrewery (V3 / 5ePHB) brew. Editing a 10k-line
blob is unworkable, so the source lives split into one file per chapter under
`src/`, and `build.sh` reassembles the single document Homebrewery needs.

## Layout

```
homebrewery.txt          pre-split backup — DO NOT edit, kept to prove round-trip
src/                     edit here, one file per chapter
  00-frontmatter.md      metadata block + covers (the ```metadata fence must
                         stay the very first bytes of the built file)
  01-indice.md
  10/11/12-parte*.md     Partes I–III: sistemas, como jogar, interpretação
  20-parte4-ambientacao.md
  21-povos/              one file per povo (aiglanos … povos-do-leste)
  22-racas.md  23-faccoes-guildas.md
  30-caminhos-classes.md
  40-caminho-armas/      00-intro + one file per classe
  50-caminho-sabedoria/  00-intro + one file per classe
  60-caminho-subterfugio/ 00-intro + one file per classe
  70-passados.md  71-caracteristicas.md  72-pericias.md
  80-magias/             00-intro, truques, cânticos, ninjutsu,
                         divinas (paladino/caç.demônios/clérigo),
                         naturais (druida/xamã), primordiais
  99-backcover.md
build.sh                 src/ -> dist/handbook.brew.md
dist/handbook.brew.md    GENERATED. Paste THIS into Homebrewery. Never hand-edit.
tools_split.py           one-shot splitter + the authoritative segment map;
                         re-run only to re-derive src/ from the backup
```

## Workflow

1. Edit the relevant chapter file(s) in `src/`. A system reworked in
   `pkg/systems` maps to one chapter here — the git diff *is* the patch note.
2. `./build.sh`
3. Paste `dist/handbook.brew.md` into Homebrewery, or upload it.

Ordering is purely the numeric filename prefixes (`build.sh` does
`find src -name '*.md' | sort`). To insert a chapter, pick a prefix between
its neighbours (e.g. `45-…` between `40-` and `50-`); leave gaps when numbering
so you rarely have to renumber. New files need a `.md` extension to be picked
up.

## Invariants

- `src/00-frontmatter.md` must sort first and start with the ```` ```metadata ````
  fence — Homebrewery reads it from the top of the document.
- `dist/` is build output. `homebrewery.txt` is an immutable backup that lets
  `build.sh` confirm the pipeline is lossless until the first edit.
- The split is byte-exact: before any edits, the built file is `cmp`-identical
  to `homebrewery.txt`.
