#!/usr/bin/env python3
import xml.etree.ElementTree as ET
import os
import sys

def parse_xml_to_md(xml_dir, output_file):
    output_dir = os.path.dirname(output_file)
    api_dir = os.path.join(output_dir, "api")
    if not os.path.exists(api_dir):
        os.makedirs(api_dir)

    index_path = os.path.join(xml_dir, 'index.xml')
    if not os.path.exists(index_path):
        print(f"Error: {index_path} not found!")
        return False

    tree = ET.parse(index_path)
    root = tree.getroot()

    classes = []
    for compound in root.findall('compound'):
        if compound.get('kind') in ['class', 'struct']:
            refid = compound.get('refid')
            name = compound.find('name').text
            classes.append((name, refid))
            
    classes.sort()

    categories = {}

    for name, refid in classes:
        class_file = os.path.join(xml_dir, f"{refid}.xml")
        if not os.path.exists(class_file):
            continue
            
        c_tree = ET.parse(class_file)
        c_root = c_tree.getroot()
        
        for compounddef in c_root.findall('compounddef'):
            kind = compounddef.get('kind')
            if kind not in ['class', 'struct']:
                continue
            
            # Determine category from location
            location = compounddef.find('location')
            category = "Core"
            if location is not None:
                file_path = location.get('file')
                if file_path:
                    parts = file_path.split('/')
                    if 'pluma' in parts:
                        idx = parts.index('pluma')
                        if idx + 2 < len(parts):
                            category = parts[idx + 1]
            
            if category not in categories:
                categories[category] = []
            categories[category].append(name)
            
            cat_dir = os.path.join(api_dir, category)
            if not os.path.exists(cat_dir):
                os.makedirs(cat_dir)
                
            md_content = f"# {kind.capitalize()} `{name}`\n\n"
            
            # Brief description
            brief = compounddef.find('briefdescription')
            if brief is not None:
                para = brief.find('para')
                if para is not None and para.text:
                    md_content += f"**{para.text.strip()}**\n\n"
                    
            # Detailed description
            deta = compounddef.find('detaileddescription')
            if deta is not None:
                para = deta.find('para')
                if para is not None and para.text:
                    md_content += f"{para.text.strip()}\n\n"
            
            # Public methods
            methods = []
            for sectiondef in compounddef.findall('sectiondef'):
                if sectiondef.get('kind') == 'public-func':
                    for memberdef in sectiondef.findall('memberdef'):
                        m_type = ""
                        type_el = memberdef.find('type')
                        if type_el is not None:
                            m_type = "".join(type_el.itertext()).strip()
                        m_name = memberdef.find('name').text
                        m_args = "".join(memberdef.find('argsstring').itertext()) if memberdef.find('argsstring') is not None else "()"
                        
                        if m_type and not m_type.endswith(" "):
                            m_type += " "
                        
                        m_brief = ""
                        m_brief_el = memberdef.find('briefdescription/para')
                        if m_brief_el is not None and m_brief_el.text:
                            m_brief = f" - *{m_brief_el.text.strip()}*"
                            
                        methods.append(f"- `{m_type}{m_name}{m_args}`{m_brief}")
            
            if methods:
                md_content += "## Public Methods\n"
                md_content += "\n".join(methods) + "\n\n"
                
            class_md_path = os.path.join(cat_dir, f"{name.replace('::', '_')}.md")
            with open(class_md_path, 'w') as f:
                f.write(md_content)

    # Generate Index
    index_md = "# Lib-Pluma API Reference\n\n"
    index_md += "Welcome to the **lib-pluma API Reference**. Below you will find the components organized by module.\n\n"
    
    for cat in sorted(categories.keys()):
        index_md += f"## {cat}\n\n"
        for cname in sorted(categories[cat]):
            safe_cname = cname.replace('::', '_')
            index_md += f"- [{cname}](api/{cat}/{safe_cname}.md)\n"
        index_md += "\n"

    with open(output_file, 'w') as f:
        f.write(index_md)
    
    return True

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: xml_to_md.py <xml_dir> <output_md>")
        sys.exit(1)
    
    xml_dir = sys.argv[1]
    output_md = sys.argv[2]
    
    if parse_xml_to_md(xml_dir, output_md):
        print(f"Markdown API reference generated at {output_md}")
    else:
        sys.exit(1)
