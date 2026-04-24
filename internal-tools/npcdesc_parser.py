import csv
import re
from typing import Dict, List, Any
from pathlib import Path

class NPCTemplateParser:
    """Parser for Ultima Online POL NPC template .cfg files"""
    
    def __init__(self):
        self.npcs = []
        self.all_fields = set()
    
    def parse_file(self, filepath: str) -> List[Dict[str, Any]]:
        """Parse the entire NPC template file"""
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Split by NPCTemplate blocks
        template_pattern = r'NPCTemplate\s+(\w+)\s*\{([^}]+)\}'
        matches = re.finditer(template_pattern, content, re.DOTALL)
        
        for match in matches:
            template_id = match.group(1)
            template_body = match.group(2)
            npc_data = self._parse_template(template_id, template_body)
            self.npcs.append(npc_data)
            self.all_fields.update(npc_data.keys())
        
        return self.npcs
    
    def _parse_template(self, template_id: str, body: str) -> Dict[str, Any]:
        """Parse individual NPC template block"""
        npc = {'TemplateID': template_id}
        
        # Parse each line
        lines = body.strip().split('\n')
        for line in lines:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            
            # Split on whitespace, handling multi-value properties
            parts = line.split(None, 1)
            if len(parts) == 2:
                key, value = parts
                npc[key] = value.strip()
            elif len(parts) == 1:
                # Property with no value
                npc[parts[0]] = ''
        
        return npc
    
    def to_csv(self, output_filepath: str):
        """Export parsed NPCs to CSV"""
        if not self.npcs:
            raise ValueError("No NPCs parsed. Run parse_file() first.")
        
        # Sort fields for consistent column order
        fieldnames = ['TemplateID'] + sorted([f for f in self.all_fields if f != 'TemplateID'])
        
        with open(output_filepath, 'w', newline='', encoding='utf-8') as f:
            writer = csv.DictWriter(f, fieldnames=fieldnames)
            writer.writeheader()
            
            for npc in self.npcs:
                # Fill missing fields with empty strings
                row = {field: npc.get(field, '') for field in fieldnames}
                writer.writerow(row)
        
        print(f"✓ Exported {len(self.npcs)} NPCs to {output_filepath}")
        print(f"✓ Total columns: {len(fieldnames)}")
    
    def get_stats(self):
        """Print statistics about parsed data"""
        print(f"\n{'='*60}")
        print(f"NPC Template Parser Statistics")
        print(f"{'='*60}")
        print(f"Total NPCs parsed: {len(self.npcs)}")
        print(f"Total unique properties: {len(self.all_fields)}")
        print(f"\nAll properties found:")
        for field in sorted(self.all_fields):
            print(f"  - {field}")
        print(f"{'='*60}\n")


def main():
    """Main execution function"""
    # Configuration
    input_file = 'npcdesc.cfg'  # Change this to your input file path
    output_file = 'npc_templates.csv'
    
    # Parse and convert
    parser = NPCTemplateParser()
    
    print("Parsing NPC templates...")
    parser.parse_file(input_file)
    
    # Show statistics
    parser.get_stats()
    
    # Export to CSV
    print("Exporting to CSV...")
    parser.to_csv(output_file)
    
    # Display sample data
    print("\nSample of first NPC:")
    print("-" * 60)
    if parser.npcs:
        first_npc = parser.npcs[0]
        for key, value in sorted(first_npc.items()):
            print(f"{key:20s}: {value}")
    print("-" * 60)


if __name__ == '__main__':
    main()