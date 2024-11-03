import re
# Replace custom tags with HTML spans and apply classes
# https://raw.githubusercontent.com/rr-/TRCustoms/develop/frontend/src/components/markdown-composer/MarkdownButtons/index.tsx
def custom_markdown_parser(text):
    text = re.sub(r'\[o\](.*?)\[/o\]', r'<span class="object">\1</span>', text)  # blue text for objects
    text = re.sub(r'\[s\](.*?)\[/s\]', r'<span class="secret">\1</span>', text)   # secret styling
    text = re.sub(r'\[p\](.*?)\[/p\]', r'<span class="pickup">\1</span>', text)   # pickup styling
    text = re.sub(r'\[e\](.*?)\[/e\]', r'<span class="enemy">\1</span>', text)    # enemy styling
    text = re.sub(r'\[t\](.*?)\[/t\]', r'<span class="trap">\1</span>', text)     # trap styling
    text = re.sub(r'\[center\](.*?)\[/center\]', r'<div style="text-align: center;">\1</div>', text)  # center align

    return text

# Example usage
#description = """[center]**Tomb Raider: Pandora's Box**[/center]
#[s]Secret text[/s] [o]Object text[/o] [p]Pickup text[/p]"""

#parsed_description = custom_markdown_parser(description)
#print(parsed_description)


