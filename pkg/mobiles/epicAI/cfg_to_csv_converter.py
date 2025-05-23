import csv
import re
import os

def parse_cfg_to_csv(cfg_filepath, csv_filepath):
    records = []
    header_set = set() 

    current_record = None
    in_block = False

    try:
        with open(cfg_filepath, 'r', encoding='utf-8') as f_cfg:
            for line_number, line_content in enumerate(f_cfg):
                line = line_content.strip()

                if not line or line.startswith("//"):
                    continue

                if line.startswith("NPCTemplate"):
                    if current_record: # Save previous record before starting a new one
                        records.append(current_record)
                        for key_in_rec in current_record.keys():
                            header_set.add(key_in_rec)
                    
                    current_record = {}
                    # Split into at most 3 parts: "NPCTemplate", "Name", "Optional Suffix"
                    parts = line.split(None, 2) 
                    
                    template_name_candidate = parts[1]
                    
                    # Handle cases like "NPCTemplate Name{"
                    if template_name_candidate.endswith("{") and len(parts) == 2:
                        current_record['TemplateName'] = template_name_candidate[:-1].strip()
                        in_block = True
                    else:
                        current_record['TemplateName'] = template_name_candidate
                    
                    header_set.add('TemplateName')

                    if len(parts) > 2: # Suffix exists
                        suffix_candidate = parts[2].strip()
                        if suffix_candidate == "{": # e.g. NPCTemplate Name {
                            in_block = True 
                            # No actual suffix string, just the brace
                        elif suffix_candidate.endswith("{"): # e.g. NPCTemplate Name Suffix {
                            current_record['TemplateSuffix'] = suffix_candidate[:-1].strip()
                            header_set.add('TemplateSuffix')
                            in_block = True
                        else: # e.g. NPCTemplate Name Suffix (brace on next line)
                            current_record['TemplateSuffix'] = suffix_candidate
                            header_set.add('TemplateSuffix')
                            # in_block remains False, expecting '{' on a new line
                    
                    # If after NPCTemplate line parsing, we are not in block, it means { is expected.
                    # If { was part of NPCTemplate line, in_block is true.
                    continue 

                if not in_block and line == "{":
                    if current_record is None:
                        # This means a '{' appeared outside an NPCTemplate context.
                        # print(f"Warning: Encountered '{{' at line {line_number+1} outside of an NPCTemplate block. Ignoring.")
                        continue
                    in_block = True
                    continue
                
                if in_block:
                    if current_record is None: 
                        # This state should ideally not be reached if logic is correct.
                        # print(f"Warning: In block at line {line_number+1} but no current record. Resetting state.")
                        in_block = False # Reset state
                        continue

                    if line == "}":
                        if current_record: 
                            records.append(current_record)
                            for key_in_rec in current_record.keys():
                                header_set.add(key_in_rec)
                        current_record = None # Reset for the next block
                        in_block = False
                    else:
                        # Parse key-value pair
                        # Regex: first non-whitespace group is key, the rest is value
                        match = re.match(r'([^\s]+)\s+(.*)', line)
                        if match:
                            key = match.group(1)
                            value = match.group(2).strip()
                            current_record[key] = value
                            header_set.add(key)
                        # else:
                            # Malformed line within a block, optionally log:
                            # print(f"Warning: Malformed key-value line {line_number+1}: '{line_content.strip()}' in record {current_record.get('TemplateName', 'N/A')}")
                            
        # Add the last record if the file ends while still processing a block
        if current_record:
            records.append(current_record)
            for key_in_rec in current_record.keys():
                header_set.add(key_in_rec)

    except FileNotFoundError:
        print(f"Error: Input file '{cfg_filepath}' not found.")
        return
    except Exception as e:
        print(f"An error occurred during parsing file '{cfg_filepath}': {e}")
        return

    if not records:
        print(f"No NPCTemplate data found or parsed from '{cfg_filepath}'. CSV file will not be created or will be empty.")
        if not header_set: # No headers means no data at all
             return


    # Define header order: TemplateName, TemplateSuffix first, then others alphabetically
    ordered_headers = []
    if 'TemplateName' in header_set: # Should always be true if records exist
        ordered_headers.append('TemplateName')
    if 'TemplateSuffix' in header_set:
        ordered_headers.append('TemplateSuffix')
    
    # Add remaining headers sorted alphabetically
    for header in sorted(list(header_set)):
        if header not in ordered_headers:
            ordered_headers.append(header)
    
    # Fallback if no predefined headers were found but records exist (should not happen with current logic)
    if not ordered_headers and records: 
        ordered_headers = sorted(list(records[0].keys()))


    try:
        with open(csv_filepath, 'w', newline='', encoding='utf-8') as f_csv:
            # Use restval='' to ensure missing keys in a record get an empty string in CSV
            writer = csv.DictWriter(f_csv, fieldnames=ordered_headers, extrasaction='ignore', restval='')
            writer.writeheader()
            for record in records:
                writer.writerow(record)
        print(f"Successfully converted '{cfg_filepath}' to '{csv_filepath}'")
    except IOError:
        print(f"Error: Could not write to CSV file '{csv_filepath}'. Check permissions or path.")
    except Exception as e:
        print(f"An error occurred during CSV writing for '{csv_filepath}': {e}")


if __name__ == '__main__':
    # Determine the workspace root and construct absolute paths for input and output files.
    # This assumes the script is run from a context where '/home/igorrturano/epic-shard' is the workspace.
    workspace_root = '/home/igorrturano/epic-shard/' # As per user's context
    
    cfg_relative_path = os.path.join('pkg', 'mobiles', 'epicAI', 'npcdesc.cfg')
    csv_relative_path = os.path.join('pkg', 'mobiles', 'epicAI', 'npcdesc.csv')

    cfg_file_path_abs = os.path.join(workspace_root, cfg_relative_path)
    csv_file_path_abs = os.path.join(workspace_root, csv_relative_path)

    print(f"Input CFG file: {cfg_file_path_abs}")
    print(f"Output CSV file: {csv_file_path_abs}")
    
    parse_cfg_to_csv(cfg_file_path_abs, csv_file_path_abs)

    # For generic use, consider using argparse to pass file paths via command line:
    # import argparse
    # parser = argparse.ArgumentParser(description="Convert NPCTemplate CFG file to CSV.")
    # parser.add_argument("input_cfg", help="Path to the input CFG file.")
    # parser.add_argument("output_csv", help="Path to the output CSV file.")
    # args = parser.parse_args()
    # parse_cfg_to_csv(args.input_cfg, args.output_csv)
