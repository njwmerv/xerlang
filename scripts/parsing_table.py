import re
import bs4

from constants import productions, ordering

productions_map: dict[str, int] = {productions[i] : i for i in range(len(productions))}

with open('table.html', 'r', encoding='utf-8') as file:
    soup = bs4.BeautifulSoup(file, 'html.parser')

headers = [th.text.strip() for th in soup.find('thead').find_all('th')][1:]
num_cols = len(headers) - 1

index_map = []
for col_name in ordering:
    if col_name in headers:
        index_map.append(headers.index(col_name))
    else: print(f'WTF {col_name} is NOT a token!?')


def parse_cell(cell_text: str) -> str:
    text: str = cell_text.strip()

    if not text: return 'n()' # '{ParsingTableEntry::Action::NIL,{.target_state=0}}'

    if text == 'accept': return 'a()'

    if text.startswith('shift'):
        state = re.search(r'\d+', text).group()
        return f's({state})'
        # return f'{{ParsingTableEntry::Action::SHIFT,{{.target_state={state}}}}}'

    if text.startswith('reduce'):
        rule = text.replace('reduce(', '').replace(')', '').replace('→', '→').strip()
        prod_id = productions_map.get(rule, -1)
        return f'r({prod_id})'
        # return f'{{ParsingTableEntry::Action::REDUCE,{{.production_id={prod_id}}}}}'

    if text.isdigit():
        return f'g({text})'
        # return f'{{ParsingTableEntry::Action::GOTO,{{.target_state={text}}}}}'

    return 'n()' # '{ParsingTableEntry::Action::NIL,{.target_state=0}}'

rows = soup.find('table').find_all('tr')[1:]
cpp_output = ''

for row in rows:
    cells = row.find_all('td')

    cpp_output += '\t{'
    row_entries = []
    for i in index_map: row_entries.append(parse_cell(cells[i].text))
    cpp_output += ','.join(row_entries)
    cpp_output += '},\n'

with open('parsing_table_output', 'w', encoding='utf-8') as f:
    f.write(cpp_output)
