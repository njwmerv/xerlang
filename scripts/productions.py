n: int = 0
with open('../xer/grammar', 'r') as file:
    for line in file:
        LINE = line.strip()
        if LINE == '': continue
        ROW = LINE.split(' ')
        n = max(len(ROW) - 3, n)
        rhs = '{'
        for sym in ROW[2:]:
            if sym == '.': break
            rhs += f'ParserSymbol::{sym},'
        rhs += '}'
        print('\t{' + f'ParserSymbol::{ROW[0]}, {len(ROW) - 3}, {rhs}' + '},')
print(n)