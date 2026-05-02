import re
import bs4

productions: list[str] = ['start → BoF procedures EoF','procedures → dcl SEMI procedures','procedures → dcl BECOMES expr1 SEMI procedures','procedures → structdef procedures','procedures → procedure procedures','procedures → main','procedure → ID COLON LPAREN params RPAREN ARROW type LCURLY statements RCURLY','main → MAIN COLON LPAREN RPAREN ARROW INT LCURLY statements RCURLY','structdef → STRUCT ID LCURLY dcls RCURLY SEMI','params → ε','params → dcls','dcls → dcl','dcls → dcl COMMA dcls','dcls → dcl SEMI dcls','dcl → type ID','statements → statements statement','statements → ε','statement → dcl SEMI','statement → dcl BECOMES expr1 SEMI','statement → expr1 BECOMES expr1 SEMI','statement → IF LPAREN expr1 RPAREN LCURLY statements RCURLY ifs','statement → WHILE LPAREN expr1 RPAREN LCURLY statements RCURLY','statement → FOR LPAREN forprologue SEMI expr1 SEMI forepilogue RPAREN LCURLY statements RCURLY','statement → BREAK SEMI','statement → expr1 SEMI','statement → NEW type LBRACK NUM RBRACK SEMI','statement → DELETE expr1 SEMI','statement → PRINT LPAREN args RPAREN SEMI','statement → RETURN expr1 SEMI','ifs → ε','ifs → ELIF LPAREN expr1 RPAREN LCURLY statements RCURLY ifs','ifs → ELSE LCURLY statements RCURLY','forprologue → dcl BECOMES expr1','forprologue → expr1 BECOMES expr1','forepilogue → expr1 BECOMES expr1','forepilogue → expr1','args → expr1','args → expr1 COMMA args','type → INT star','type → CHAR star','type → BOOL star','type → STRUCT ID star','star → ε','star → AT star','expr1 → expr1 OR expr2','expr1 → expr2','expr2 → expr2 AND expr3','expr2 → expr3','expr3 → expr3 BITOR expr4','expr3 → expr4','expr4 → expr4 BITXOR expr5','expr4 → expr5','expr5 → expr5 BITAND expr6','expr5 → expr6','expr6 → expr6 EQUALS expr7','expr6 → expr6 NEQ expr7','expr6 → expr7','expr7 → expr7 LT expr8','expr7 → expr7 LEQ expr8','expr7 → expr7 GT expr8','expr7 → expr7 GEQ expr8','expr7 → expr8','expr8 → expr8 LSHIFT expr9','expr8 → expr8 RSHIFT expr9','expr8 → expr9','expr9 → expr9 PLUS expr10','expr9 → expr9 SUB expr10','expr9 → expr10','expr10 → expr10 MULT expr11','expr10 → expr10 DIV expr11','expr10 → expr10 MOD expr11','expr10 → expr11','expr11 → AT expr11','expr11 → ADDR expr11','expr11 → NOT expr11','expr11 → BITNOT expr11','expr11 → INCR expr11','expr11 → DECR expr11','expr11 → SUB expr11','expr11 → PLUS expr11','expr11 → expr12','expr12 → expr13 EXP expr12','expr12 → expr13','expr13 → expr13 INCR','expr13 → expr13 DECR','expr13 → expr13 ARROW ID','expr13 → expr13 DOT ID','expr13 → expr14','expr14 → LPAREN expr1 RPAREN','expr14 → ID','expr14 → TRUE','expr14 → FALSE','expr14 → NIL','expr14 → NUM','expr14 → CHARLIT','expr14 → READ LPAREN RPAREN',]

productions_map: dict[str, int] = {productions[i] : i for i in range(len(productions))}

# for prod in productions:
#     print(f'{prod} : {productions_map[prod]}')

with open('table.html', 'r', encoding='utf-8') as file:
    soup = bs4.BeautifulSoup(file, 'html.parser')

headers = [th.text.strip() for th in soup.find('thead').find_all('th')][1:]
num_cols = len(headers) - 1

ordering: list[str] = ["MAIN","READ","PRINT","INT","CHAR","BOOL","STRUCT","TRUE","FALSE","NIL","NUM","CHARLIT","ID","RETURN","IF","ELIF","ELSE","FOR","WHILE","BREAK","DELETE","NEW","COLON","LPAREN","RPAREN","SEMI","LCURLY","RCURLY","COMMA","LBRACK","RBRACK","BECOMES","NOT","OR","AND","GEQ","GT","LEQ","LT","EQUALS","NEQ","PLUS","SUB","MULT","DIV","MOD","LSHIFT","RSHIFT","EXP","BITOR","BITXOR","BITAND","BITNOT","AT","ADDR","INCR","DECR","ARROW","DOT","BoF","EoF","start","procedures","procedure","main","type","star","params","dcls","dcl","statements","statement","ifs","forprologue","forepilogue","expr1","expr2","expr3","expr4","expr5","expr6","expr7","expr8","expr9","expr10","expr11","expr12","expr13","expr14","args","structdef","$",]

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

with open('parsing_table.hpp', 'w', encoding='utf-8') as f:
    f.write(cpp_output)
