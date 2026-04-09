// extract-landtiles-detailed.js
const fs = require("fs");
const path = require("path");

// Parse command-line arguments
const args = process.argv.slice(2);
const modes = {
  namesOnly: args.includes("--names-only"),
  full: args.includes("--full") || !args.includes("--names-only"),
  saveFile: args.includes("--file"),
};

const landtilesPath = path.join(__dirname, "landtiles.cfg");
const content = fs.readFileSync(landtilesPath, "utf8");

// Split into individual landtile blocks
const landtileBlocks = content.match(/landtile\s+0x[0-9a-f]+\s*\{[^}]+\}/gi);

const terrainMap = {};

landtileBlocks.forEach((block) => {
  const hexMatch = block.match(/landtile\s+(0x[0-9a-f]+)/i);
  const nameMatch = block.match(/Name\s+(\S+)/i);

  if (hexMatch && nameMatch) {
    const hex = hexMatch[1];
    const name = nameMatch[1];

    if (!terrainMap[name]) {
      terrainMap[name] = [];
    }
    terrainMap[name].push(hex);
  }
});

// Function to output names only
function outputNamesOnly(toFile = false) {
  const names = Object.keys(terrainMap)
    .filter((name) => name !== "NoName")
    .sort();
  const output = names.join("\n");

  if (!toFile) {
    console.log("Terrain Names:");
    console.log("==============\n");
    console.log(output);
  }
  return output;
}

// Function to output full summary
function outputFullSummary(toFile = false) {
  let output = "Terrain Types with Hex IDs:\n";
  output += "===========================\n\n";

  Object.keys(terrainMap)
    .sort()
    .forEach((name) => {
      const hexIds = terrainMap[name];
      output += `${name.toUpperCase()}\n`;
      output += `  Count: ${hexIds.length}\n`;
      output += `  Hex IDs: ${hexIds.slice(0, 10).join(", ")}${
        hexIds.length > 10 ? "..." : ""
      }\n\n`;
    });

  if (!toFile) {
    console.log(output);
  }
  return output;
}

// Function to output config template
function outputConfigTemplate(toFile = false) {
  let output = "Generated Config Template:\n";
  output += "==========================\n\n";

  Object.keys(terrainMap)
    .sort()
    .forEach((name) => {
      if (name !== "NoName") {
        output += `TerrainType ${name}\n`;
        output += "{\n";
        terrainMap[name].forEach((hex) => {
          output += `    landtile ${hex}\n`;
        });
        output += `    element "${name}"\n`;
        output += `    templates // Add templates here\n`;
        output += "}\n\n";
      }
    });

  if (!toFile) {
    console.log(output);
  }
  return output;
}

// Main output logic
let consoleOutput = "";
let fileOutput = "";

if (modes.namesOnly && !modes.full) {
  // Names only mode
  consoleOutput = outputNamesOnly(false);
  fileOutput = outputNamesOnly(true);
} else if (modes.full) {
  // Full mode (default or explicit)
  consoleOutput = outputFullSummary(false);
  consoleOutput += "\n\n" + outputConfigTemplate(false);
  fileOutput = outputFullSummary(true) + "\n\n" + outputConfigTemplate(true);
}

// Save to file if requested
if (modes.saveFile) {
  const outputPath = path.join(__dirname, "landtiles-output.txt");
  fs.writeFileSync(outputPath, fileOutput);
  console.log(`\n✓ Output saved to ${outputPath}`);
}
