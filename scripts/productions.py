from constants import productions

i: int = 0
for prod in productions:
    PROD = prod.split(' ')
    # print(PROD)
    rhs = '{'
    count: int = 0

    if PROD[2] == 'ε':
        count = 0
        rhs = '{}'
    else:
        for sym in PROD[2:]:
            count += 1
            rhs += f'ParserSymbol::{sym},'
        rhs += '}'
    print('\t{' + f'{count}, {i}, ParserSymbol::{PROD[0]}, {rhs}' + '},')
    i += 1

i: int = 0
for prod in productions:
    PROD = prod.split(' ')
    rhs = ''

    if PROD[2] != 'ε':
        for sym in PROD[2:]:
            rhs += f'{sym}'
    print('#define ' + f'{PROD[0]}_{rhs} {i}')
    i += 1
