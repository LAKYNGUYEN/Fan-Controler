Import("env")


# Check if minify-html is installed, if not install it
try:
    import minify_html
except ImportError:
    env.Execute("$PYTHONEXE -m pip install minify-html")

# get html content

with open ('src/index.html', 'r', encoding='utf-8') as file:
    content = file.read()

# minify html content
minified = minify_html.minify(content,minify_css=True, remove_processing_instructions=True)
print(minified)

# save minified content to file
to_write = f"""
#ifndef MAIN_PAGE_H
#define MAIN_PAGE_H
const char *MAIN_PAGE = R"html({minified} )html";
#endif // MAIN_PAGE_H
"""

with open ('include/MAIN_PAGE.h', 'w', encoding='utf-8') as file:
    file.write(to_write)